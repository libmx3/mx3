#include "db.hpp"
#include <sqlite3/sqlite3.h>

using mx3::sqlite::Db;
using mx3::sqlite::Stmt;
using mx3::sqlite::ChangeType;
using mx3::sqlite::OpenFlag;

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
    auto error_code = sqlite3_open_v2(db_path.c_str(), &db, sqlite_flags, vfs_p);
    auto temp_db = unique_ptr<sqlite3, Db::Closer> {db};

    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
    return shared_ptr<Db> { new Db{std::move(temp_db)} };
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
    auto temp_db = unique_ptr<sqlite3, Db::Closer> {db};
    return shared_ptr<Db> { new Db{std::move(temp_db)} };
}

Db::Db(unique_ptr<sqlite3, Closer> db) : m_db { std::move(db) } {}

Db::~Db() {
    // setting the hooks to nullptr
    this->update_hook(nullptr);
    this->commit_hook(nullptr);
    this->rollback_hook(nullptr);
}

sqlite3 *
Db::borrow_db() {
    return m_db.get();
}

void
Db::update_hook(UpdateHookFn update_fn) {
    unique_ptr<UpdateHookFn> new_hook = nullptr;
    if (update_fn) {
        new_hook = make_unique<UpdateHookFn>( std::move(update_fn) );
    }

    unique_ptr<UpdateHookFn> old_update_hook {
        static_cast<UpdateHookFn*>(
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
                    update_hook(*type, string {db_name}, string {table_name}, static_cast<int64_t>(row_id));
                }
            }, new_hook.release())
        )
    };

}

void
Db::commit_hook(function<bool()> commit_fn) {
    unique_ptr<CommitHookFn> new_hook = nullptr;
    if (commit_fn) {
        // move this function to the heap, so we can insert it directly into the db
        new_hook = make_unique<CommitHookFn>( std::move(commit_fn) );
    }

    unique_ptr<CommitHookFn> old_commit_hook {
        static_cast<CommitHookFn*>(
            sqlite3_commit_hook(m_db.get(), [] (void * self) -> int {
                auto current_hook = static_cast<CommitHookFn*>(self);
                if (current_hook) {
                    auto commit_hook = *current_hook;
                    bool result = commit_hook();
                    return result ? 0 : 1;
                }
                return 0;
            }, new_hook.release())
        )
    };

}

void
Db::rollback_hook(function<void()> rollback_fn) {
    unique_ptr<RollbackHookFn> new_hook = nullptr;
    if (rollback_fn) {
        new_hook = make_unique<RollbackHookFn>( std::move(rollback_fn) );
    }

    unique_ptr<RollbackHookFn> old_rollback_hook {
        static_cast<RollbackHookFn*>(
            sqlite3_rollback_hook(m_db.get(), [] (void * self) {
                auto current_hook = static_cast<RollbackHookFn*>(self);
                if (current_hook) {
                    auto rollback_hook = *current_hook;
                    rollback_hook();
                }
            } , new_hook.release())
        )
    };

}

int64_t
Db::last_insert_rowid() {
    return sqlite3_last_insert_rowid(m_db.get());
}

void
Db::Closer::operator() (sqlite3 * db) {
    // we use the sqlite3_close_v2 call since we want to prevent misuse
    auto error_code = sqlite3_close_v2(db);
    if (error_code != SQLITE_OK) {
        // warn about API misuse
    }
}

void
Db::close() {
    auto error_code = sqlite3_close(m_db.get());
    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
}

void
Db::exec(const string& sql) {
    char * error_msg = nullptr;
    auto result = sqlite3_exec(m_db.get(), sql.c_str(), nullptr, nullptr, &error_msg);
    if (result != SQLITE_OK) {
        string message;
        if (error_msg) {
            message = string(error_msg);
            sqlite3_free(error_msg);
            error_msg = nullptr;
        } else {
            message = "Unknown error";
        }
        throw std::runtime_error { message };
    }
}

int64_t
Db::exec_scalar(const string& sql) {
    auto cursor = this->prepare(sql)->exec_query();
    if (cursor.column_count() == 1) {
        return cursor.int64_value(0);
    }
    throw std::runtime_error {"Cant call exec_scalar on query with multiple rows"};
}

shared_ptr<Stmt>
Db::prepare(const string& sql) {
    sqlite3_stmt * stmt = nullptr;
    const char * end_point = nullptr;
    auto error_code = sqlite3_prepare_v2(m_db.get(), sql.c_str(), static_cast<int>(sql.length()), &stmt, &end_point);

    // we don't use make_shared here since we want to be able hide the ctor
    auto raw_stmt = new Stmt {stmt, this->shared_from_this()};
    shared_ptr<Stmt> wrapped_stmt {raw_stmt};

    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    } else {
        return wrapped_stmt;
    }
}
