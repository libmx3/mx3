#include "db.hpp"
#include <sqlite3/sqlite3.h>

using mx3::sqlite::Db;
using mx3::sqlite::Stmt;
using mx3::sqlite::Cursor;

namespace {
    // a helper funciton which throws when it encounters an error
    template<typename F, typename... Args>
    inline void s_throw_on_sqlite_error(F&& fn, sqlite3_stmt * stmt, Args... args) {
        auto error_code = fn(stmt, std::forward<Args>(args)...);
        if (error_code != SQLITE_OK) {
            throw std::runtime_error { sqlite3_errstr(error_code) };
        }
    }

    template<typename F>
    inline auto s_column_or_throw(F&& fn, sqlite3_stmt * stmt, int expected_type, int pos) -> decltype(fn(stmt, pos)) {
        if (sqlite3_column_type(stmt, pos) != expected_type) {
            throw std::runtime_error {"invalid type for column"};
        }
        return std::forward<F>(fn)(stmt, pos);
    }
}

shared_ptr<Db>
Db::open(const string& db_path) {
    return shared_ptr<Db> { new Db(db_path) };
}

Db::Db(const string& path) : m_db {nullptr} {
    constexpr const int flags =
        SQLITE_OPEN_READWRITE |
        SQLITE_OPEN_CREATE |
        // multi-threaded mode
        SQLITE_OPEN_NOMUTEX |
        SQLITE_OPEN_PRIVATECACHE;

    sqlite3 * db;
    auto error_code = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
    m_db = unique_ptr<sqlite3, Db::Closer>{db};
    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
}

void
Db::Closer::operator() (sqlite3 * db) {
    sqlite3_close_v2(db);
}

void
Db::exec(const string& sql) {
    char * error_msg = nullptr;
    auto result = sqlite3_exec(m_db.get(), sql.c_str(), nullptr, nullptr, &error_msg);
    if (result && error_msg) {
        if (error_msg) {
            throw std::runtime_error { string(error_msg) };
        } else {
            throw std::runtime_error { "Unknown error" };
        }
    }
}

Stmt
Db::prepare(const string& sql) {
    sqlite3_stmt * stmt = nullptr;
    const char * end_point = nullptr;
    auto error_code = sqlite3_prepare_v2(m_db.get(), sql.c_str(), static_cast<int>(sql.length()), &stmt, &end_point);
    Stmt wrapped_stmt {stmt};

    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    } else {
        return std::move(wrapped_stmt);
    }
}

Stmt::Stmt(sqlite3_stmt * stmt) : m_stmt{stmt} {}

void
Stmt::Deleter::operator() (sqlite3_stmt * stmt) {
    if (stmt) {
        /* unused error_code = */ sqlite3_finalize(stmt);
    }
}

int
Stmt::param_count() const {
    return sqlite3_bind_parameter_count(m_stmt.get());
}

optional<string>
Stmt::param_name(int pos) const {
    auto name = sqlite3_bind_parameter_name(m_stmt.get(), pos);
    if (name) {
        return string {name};
    } else {
        return nullopt;
    }
}

int
Stmt::param_index(const string& name) const {
    auto index = sqlite3_bind_parameter_index(m_stmt.get(), name.c_str());
    if (index == 0) {
        throw std::runtime_error { "Param `" + name + "` not found" };
    }
    return index;
}

void
Stmt::bind(int pos, const vector<uint8_t>& value) {
    s_throw_on_sqlite_error(sqlite3_bind_blob, m_stmt.get(), pos, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
}

void
Stmt::bind(int pos, double value) {
    s_throw_on_sqlite_error(sqlite3_bind_double, m_stmt.get(), pos, value);
}

void
Stmt::bind(int pos, int32_t value) {
    s_throw_on_sqlite_error(sqlite3_bind_int, m_stmt.get(), pos, value);
}

void
Stmt::bind(int pos, int64_t value) {
    s_throw_on_sqlite_error(sqlite3_bind_int64, m_stmt.get(), pos, value);
}

void
Stmt::bind(int pos, std::nullptr_t) {
    s_throw_on_sqlite_error(sqlite3_bind_null, m_stmt.get(), pos);
}

void
Stmt::bind(int pos, const string& value) {
    s_throw_on_sqlite_error(sqlite3_bind_text, m_stmt.get(), pos, value.c_str(), static_cast<int>(value.length()), SQLITE_TRANSIENT);
}

int
Stmt::exec() {
    auto cursor = this->exec_query();
    auto * db = sqlite3_db_handle(m_stmt.get());
    return sqlite3_changes(db);
}

Cursor
Stmt::exec_query() {
    sqlite3_stmt * stmt = m_stmt.get();
    this->reset();
    auto result_code = sqlite3_step(stmt);
    switch (result_code) {
        case SQLITE_ROW:
        case SQLITE_DONE:
            break;
        default: {
          throw std::runtime_error { sqlite3_errstr(result_code) };
          break;
        }
    }
    return Cursor {stmt, result_code == SQLITE_DONE};
}

void
Stmt::reset() {
    auto error_code = sqlite3_reset(m_stmt.get());
    if (error_code != SQLITE_OK) {
        throw std::runtime_error { sqlite3_errstr(error_code) };
    }
}

void
Stmt::clear_bindings() {
    sqlite3_clear_bindings(m_stmt.get());
}

Cursor::Cursor(sqlite3_stmt * stmt, bool is_done) : m_is_done{is_done}, m_stmt {stmt} {}

void
Cursor::Reset::operator() (sqlite3_stmt * stmt) {
    sqlite3_reset(stmt);
}

string
Cursor::column_name(int pos) const {
    auto name = sqlite3_column_name(m_stmt.get(), pos);
    return string {name};
}

int
Cursor::column_count() const {
    return sqlite3_column_count(m_stmt.get());
}

void
Cursor::next() {
    auto result = sqlite3_step(m_stmt.get());
    switch (result) {
        case SQLITE_ROW:
            break;
        case SQLITE_DONE:
            m_is_done = true;
            break;
        default: {
          throw std::runtime_error { "invalid query" };
          break;
        }
    }
}

string
Cursor::string_value(int pos) {
    auto data = s_column_or_throw(sqlite3_column_text, m_stmt.get(), SQLITE_TEXT, pos);
    return string { reinterpret_cast<const char *>(data) };
}

int32_t
Cursor::int_value(int pos) {
    return s_column_or_throw(sqlite3_column_int, m_stmt.get(), SQLITE_INTEGER, pos);
}

int64_t
Cursor::int64_value(int pos) {
    return s_column_or_throw(sqlite3_column_int64, m_stmt.get(), SQLITE_INTEGER, pos);
}

double
Cursor::double_value(int pos) {
    return s_column_or_throw(sqlite3_column_double, m_stmt.get(), SQLITE_FLOAT, pos);
}

vector<uint8_t>
Cursor::blob_value(int pos) {
    auto stmt = m_stmt.get();
    const auto len = s_column_or_throw(sqlite3_column_bytes, stmt, SQLITE_BLOB, pos);
    const uint8_t * data = static_cast<const uint8_t*>( sqlite3_column_blob(stmt, pos) );
    return {data, data + len};
}
