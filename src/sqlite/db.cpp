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
        auto error_code = sqlite3_finalize(stmt);
        // check error code, issue warning
    }
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
    // todo check result code
    sqlite3_reset(stmt);
    auto result = sqlite3_step(stmt);
    switch (result) {
        case SQLITE_ROW:
        case SQLITE_DONE:
            break;
        default: {
          throw std::runtime_error { "invalid query" };
          break;
        }
    }
    return Cursor {stmt, result == SQLITE_DONE};
}

Cursor::Cursor(sqlite3_stmt * stmt, bool is_done) : m_is_done{is_done}, m_stmt {stmt} {}

void
Cursor::NoopDelete::operator() (sqlite3_stmt * stmt) {
    sqlite3_reset(stmt);
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
Cursor::get_string(int pos) {
    auto stmt = m_stmt.get();
    if (sqlite3_column_type(stmt, pos) != SQLITE_TEXT) {
        throw std::runtime_error {"invalid type for column"};
    }
    return string { reinterpret_cast<const char *>( sqlite3_column_text(stmt, pos) ) };
}

int32_t
Cursor::get_int(int pos) {
    auto stmt = m_stmt.get();
    if (sqlite3_column_type(stmt, pos) != SQLITE_INTEGER) {
        throw std::runtime_error {"invalid type for column"};
    }
    return sqlite3_column_int(stmt, pos);
}
