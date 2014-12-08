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

Stmt::Stmt(sqlite3_stmt * stmt, shared_ptr<Db> db) : m_db {db}, m_stmt {stmt} {}

sqlite3_stmt *
Stmt::borrow_stmt() {
    return m_stmt.get();
}

void
Stmt::Finalizer::operator() (sqlite3_stmt * stmt) {
    auto result_code = sqlite3_finalize(stmt);
    if (result_code != SQLITE_OK) {
        // warn about usage
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
    return Cursor {this->shared_from_this(), result_code != SQLITE_DONE};
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
