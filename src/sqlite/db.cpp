#include "db.hpp"
#include <algorithm>
#include <sqlite3/sqlite3.h>

namespace mx3 { namespace sqlite {

namespace {
    struct Sqlite3Free final {
        void operator() (char * sql) const {
            sqlite3_free(sql);
        }
    };

    inline char ascii_upper(const char c) {
        return c < 'a' ? c : c > 'z' ? c : (c - 'a' + 'A');
    }

    inline bool nocase_eq(const char a, const char b) {
        return ascii_upper(a) == ascii_upper(b);
    }

    bool matches_nocase(const string& haystack, const string& needle) {
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), nocase_eq) != haystack.end();
    }
}

// see https://www.sqlite.org/datatype3.html
Affinity ColumnInfo::type_affinity() const {
    static const string INT {"INT"};
    static const string CHAR {"CHAR"};
    static const string CLOB {"CLOB"};
    static const string TEXT {"TEXT"};
    static const string BLOB {"BLOB"};
    static const string REAL {"REAL"};
    static const string FLO {"FLO"};
    static const string DOUB {"DOUB"};

    if (matches_nocase(type, INT)) {
        return Affinity::INTEGER;
    } else if (matches_nocase(type, CHAR) ||
               matches_nocase(type, CLOB) ||
               matches_nocase(type, TEXT)) {
        return Affinity::TEXT;
    } else if (matches_nocase(type, BLOB) || type.empty()) {
        return Affinity::NONE;
    } else if (matches_nocase(type, REAL) ||
               matches_nocase(type, FLO) ||
               matches_nocase(type, DOUB)) {
        return Affinity::REAL;
    } else {
        return Affinity::NUMERIC;
    }
}

string
mprintf(const char * format, const string& data) {
    unique_ptr<char, Sqlite3Free> raw_str {
        sqlite3_mprintf(format, data.c_str())
    };
    return string {raw_str.get()};
}

string
mprintf(const char * format, int64_t data) {
    unique_ptr<char, Sqlite3Free> raw_str {
        sqlite3_mprintf(format, data)
    };
    return string {raw_str.get()};
}

string libversion() {
    return {sqlite3_libversion()};
}

string sourceid() {
    return {sqlite3_sourceid()};
}

int libversion_number() {
    return sqlite3_libversion_number();
}

struct Db::only_for_internal_make_shared_t {};

shared_ptr<Db>
Db::open(const string& db_path, const std::set<OpenFlag>& flags, const optional<string>& vfs_name) {
    int sqlite_flags = 0;
    for (const auto flag : flags) {
        switch (flag) {
            case OpenFlag::READONLY:
                sqlite_flags |= SQLITE_OPEN_READONLY;
                break;
            case OpenFlag::READWRITE:
                sqlite_flags |= SQLITE_OPEN_READWRITE;
                break;
            case OpenFlag::CREATE:
                sqlite_flags |= SQLITE_OPEN_CREATE;
                break;
            case OpenFlag::URI:
                sqlite_flags |= SQLITE_OPEN_URI;
                break;
            case OpenFlag::MEMORY:
                sqlite_flags |= SQLITE_OPEN_MEMORY;
                break;
            case OpenFlag::NOMUTEX:
                sqlite_flags |= SQLITE_OPEN_NOMUTEX;
                break;
            case OpenFlag::FULLMUTEX:
                sqlite_flags |= SQLITE_OPEN_FULLMUTEX;
                break;
            case OpenFlag::SHAREDCACHE:
                sqlite_flags |= SQLITE_OPEN_SHAREDCACHE;
                break;
            case OpenFlag::PRIVATECACHE:
                sqlite_flags |= SQLITE_OPEN_PRIVATECACHE;
                break;
        }
    }

    sqlite3 * db = nullptr;
    const char * vfs_p = vfs_name ? vfs_name->c_str() : nullptr;
    const auto error_code = sqlite3_open_v2(db_path.c_str(), &db, sqlite_flags, vfs_p);
    auto temp_db = unique_ptr<sqlite3, Db::Closer> {db};

    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
    return make_shared<Db>(only_for_internal_make_shared_t{}, std::move(temp_db));
}

shared_ptr<Db>
Db::open(const string& db_path) {
    return Db::open(db_path, {
        OpenFlag::READWRITE,
        OpenFlag::CREATE,
        // multi-threaded mode
        OpenFlag::NOMUTEX,
        OpenFlag::PRIVATECACHE
    });
}


shared_ptr<Db>
Db::open_memory() {
    return Db::open(":memory:");
}

shared_ptr<Db>
Db::inherit_db(sqlite3 * db) {
    return make_shared<Db>(only_for_internal_make_shared_t{}, unique_ptr<sqlite3, Db::Closer> {db});
}

Db::Db(only_for_internal_make_shared_t, unique_ptr<sqlite3, Closer> db) : m_db { std::move(db) } {}

Db::~Db() {
    // setting the hooks to nullptr
    this->update_hook(nullptr);
    this->commit_hook(nullptr);
    this->rollback_hook(nullptr);
    this->wal_hook(nullptr);
}

sqlite3 *
Db::borrow_db() {
    return m_db.get();
}

string
Db::journal_mode() {
    auto stmt = this->prepare("PRAGMA journal_mode;");
    return stmt->exec_query().string_value(0);
}

void
Db::update_hook(const UpdateHookFn& update_fn) {
    unique_ptr<UpdateHookFn> new_hook = nullptr;
    if (update_fn) {
        new_hook = make_unique<UpdateHookFn>(update_fn);
    }
    std::swap(new_hook, m_update_hook);

    sqlite3_update_hook(m_db.get(), [] (void * self, int change_type, const char * db_name, const char * table_name, sqlite3_int64 row_id) {
        UpdateHookFn * current_hook = static_cast<UpdateHookFn*>(self);
        if (current_hook) {
            auto update_hook = *current_hook;
            optional<ChangeType> type = nullopt;
            switch (change_type) {
                case SQLITE_INSERT: {
                    type = ChangeType::INSERT;
                    break;
                }
                case SQLITE_UPDATE: {
                    type = ChangeType::UPDATE;
                    break;
                }
                case SQLITE_DELETE: {
                    type = ChangeType::DELETE;
                    break;
                }
            }
            if (!type) {
                throw std::runtime_error {"Unexpected update type from sqlite"};
            }
            update_hook({*type, string {db_name}, string {table_name}, static_cast<int64_t>(row_id)});
        }
    }, m_update_hook.get());
}

void
Db::commit_hook(const CommitHookFn& commit_fn) {
    unique_ptr<CommitHookFn> new_hook = nullptr;
    if (commit_fn) {
        // move this function to the heap, so we can insert it directly into the db
        new_hook = make_unique<CommitHookFn>(commit_fn);
    }
    std::swap(new_hook, m_commit_hook);

    sqlite3_commit_hook(m_db.get(), [] (void * self) -> int {
        auto current_hook = static_cast<CommitHookFn*>(self);
        if (current_hook) {
            auto commit_hook = *current_hook;
            bool result = commit_hook();
            return result ? 0 : 1;
        }
        return 0;
    }, m_commit_hook.get());
}

void
Db::rollback_hook(const RollbackHookFn& rollback_fn) {
    unique_ptr<RollbackHookFn> new_hook = nullptr;
    if (rollback_fn) {
        new_hook = make_unique<RollbackHookFn>(rollback_fn);
    }
    std::swap(new_hook, m_rollback_hook);

    sqlite3_rollback_hook(m_db.get(), [] (void * self) {
        auto current_hook = static_cast<RollbackHookFn*>(self);
        if (current_hook) {
            auto rollback_hook = *current_hook;
            rollback_hook();
        }
    }, m_rollback_hook.get());
}

void
Db::wal_hook(const WalHookFn& wal_fn) {
    unique_ptr<WalHookFn> new_hook = nullptr;
    if (wal_fn) {
        new_hook = make_unique<WalHookFn>(wal_fn);
    }
    std::swap(m_wal_hook, new_hook);

    sqlite3_wal_hook(m_db.get(), [] (void * self, sqlite3*, const char* db_name, int pages) -> int {
        const auto current_hook = static_cast<WalHookFn*>(self);
        if (current_hook) {
            // Copy to the stack before calling, saves a lot of headaches.
            const auto wal_hook = *current_hook;
            const string name {db_name};
            wal_hook(name, pages);
        }
        return SQLITE_OK;
    }, m_wal_hook.get());
}

std::pair<int, int>
Db::wal_checkpoint_v2(const optional<string>& db_name, Checkpoint mode) {
    int sqlite_mode = 0;
    switch (mode) {
    case Checkpoint::PASSIVE:
        sqlite_mode = SQLITE_CHECKPOINT_PASSIVE;
        break;
    case Checkpoint::FULL:
        sqlite_mode = SQLITE_CHECKPOINT_FULL;
        break;
    case Checkpoint::RESTART:
        sqlite_mode = SQLITE_CHECKPOINT_RESTART;
        break;
    }
    std::pair<int, int> result;
    const char * sqlite_db_name = db_name ? db_name->data() : nullptr;
    const int error_code = sqlite3_wal_checkpoint_v2(m_db.get(), sqlite_db_name, sqlite_mode, &result.first, &result.second);
    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
    return result;
}

int64_t
Db::last_insert_rowid() {
    return sqlite3_last_insert_rowid(m_db.get());
}

int32_t
Db::schema_version() {
    return static_cast<int32_t>( this->prepare("PRAGMA schema_version;")->exec_scalar() );
}

void
Db::busy_timeout(nullopt_t) {
    this->busy_timeout(std::chrono::milliseconds{-1});
}

void
Db::busy_timeout(std::chrono::system_clock::duration timeout) {
    const int ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
    const auto error_code = sqlite3_busy_timeout(m_db.get(), ms);
    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
}

vector<ColumnInfo>
Db::column_info(const string& table_name) {
    auto cursor = this->prepare(
        mprintf("PRAGMA table_info(%Q);", table_name)
    )->exec_query();

    vector<string> names = cursor.column_names();
    std::map<string, int32_t> pos_map;
    for (size_t i = 0; i < names.size(); i++) {
        pos_map[names[i]] = static_cast<int32_t>(i);
    }

    const auto cid_pos         = pos_map["cid"];
    const auto name_pos        = pos_map["name"];
    const auto type_pos        = pos_map["type"];
    const auto notnull_pos     = pos_map["notnull"];
    const auto dflt_value_pos  = pos_map["dflt_value"];
    const auto pk_pos          = pos_map["pk"];

    vector<ColumnInfo> columns;
    while (cursor.is_valid()) {
        ColumnInfo col;
        col.cid = cursor.int64_value(cid_pos);
        col.name = cursor.string_value(name_pos);
        col.type = cursor.string_value(type_pos);
        col.notnull = cursor.int64_value(notnull_pos) == 1;
        auto dflt_value = cursor.value_at(dflt_value_pos);
        col.dflt_value  = dflt_value.is_null()
                        ? optional<string> {nullopt}
                        : optional<string> {dflt_value.string_value()};
        col.pk = static_cast<int32_t>(cursor.int64_value(pk_pos));
        columns.push_back( std::move(col) );
        cursor.next();
    }
    // sort the columns by cid (which is the natural order of the table)
    std::sort(columns.begin(), columns.end(), [] (const ColumnInfo& c1, const ColumnInfo& c2) -> bool {
        return c1.cid < c2.cid;
    });
    return columns;
}

vector<TableInfo>
Db::schema_info() {
    vector<TableInfo> tables;
    auto table_cursor = this->prepare("SELECT name from 'sqlite_master' WHERE type = 'table'")->exec_query();
    while (table_cursor.is_valid()) {
        const string name = table_cursor.string_value(0);
        auto info = this->table_info(name);
        if (info) {
            tables.push_back(std::move(*info));
        }
        table_cursor.next();
    }
    return tables;
}

optional<TableInfo>
Db::table_info(const string& table_name) {
    auto table_stmt = this->prepare("SELECT name, rootpage, sql from 'sqlite_master' WHERE type = 'table' AND name = ?1");
    table_stmt->bind(1, table_name);
    auto cursor = table_stmt->exec_query();
    if (cursor.is_valid()) {
        TableInfo info;
        info.name     = cursor.string_value(0);
        info.rootpage = cursor.int64_value(1);
        info.sql      = cursor.string_value(2);
        info.columns  = this->column_info(table_name);
        return info;
    }
    return nullopt;
}

int32_t
Db::user_version() {
    return static_cast<int32_t>( this->prepare("PRAGMA user_version;")->exec_scalar() );
}

void
Db::set_user_version(int32_t user_ver) {
    this->exec(
        mprintf("PRAGMA user_version=%lld;", user_ver)
    );
}

void
Db::Closer::operator() (sqlite3 * db) const {
    // we use the sqlite3_close_v2 call since we want to prevent misuse
    auto error_code = sqlite3_close_v2(db);
    if (error_code != SQLITE_OK) {
        // warn about API misuse
    }
}

void
Db::close() {
    const auto error_code = sqlite3_close(m_db.get());
    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
}

void
Db::enable_wal() {
    this->exec("PRAGMA journal_mode=WAL");
}

void
Db::exec(const string& sql) {
    char * error_msg = nullptr;
    const auto result = sqlite3_exec(m_db.get(), sql.c_str(), nullptr, nullptr, &error_msg);
    const unique_ptr<char, Sqlite3Free> raw_msg { error_msg };
    if (result != SQLITE_OK) {
        string message;
        if (raw_msg) {
            message = string {raw_msg.get()};
        } else {
            message = "Unknown error";
        }
        throw std::runtime_error { message };
    }
}

int64_t
Db::exec_scalar(const string& sql) {
    const auto cursor = this->prepare(sql)->exec_query();
    if (cursor.column_count() == 1) {
        return cursor.int64_value(0);
    }
    throw std::runtime_error {"Cant call exec_scalar on query with multiple rows"};
}

shared_ptr<Stmt>
Db::prepare(const string& sql) {
    sqlite3_stmt * stmt = nullptr;
    const char * end_point = nullptr;

    const auto error_code = sqlite3_prepare_v2(m_db.get(), sql.c_str(), static_cast<int>(sql.length()), &stmt, &end_point);
    const shared_ptr<Stmt> wrapped_stmt = Stmt::create(stmt, shared_from_this());

    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    } else {
        return wrapped_stmt;
    }
}

} } // end namespace mx3::sqlite
