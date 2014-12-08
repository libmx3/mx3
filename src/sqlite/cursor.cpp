#include "db.hpp"
#include <sqlite3/sqlite3.h>

using mx3::sqlite::Stmt;
using mx3::sqlite::Cursor;

namespace {
    template<typename F>
    inline auto s_column_or_throw(F&& fn, sqlite3_stmt * stmt, int expected_type, int pos) -> decltype(fn(stmt, pos)) {
        if (sqlite3_column_type(stmt, pos) != expected_type) {
            throw std::runtime_error {"invalid type for column"};
        }
        return std::forward<F>(fn)(stmt, pos);
    }
}

Cursor::Cursor(shared_ptr<Stmt> stmt, bool is_valid) : m_stmt {stmt} , m_is_valid {is_valid} {}

sqlite3_stmt *
Cursor::borrow_stmt() const {
    return m_stmt->borrow_stmt();
}

string
Cursor::column_name(int pos) const {
    auto name = sqlite3_column_name(m_stmt->borrow_stmt(), pos);
    return string {name};
}

int
Cursor::column_count() const {
    return sqlite3_column_count(m_stmt->borrow_stmt());
}

void
Cursor::next() {
    auto result = sqlite3_step(m_stmt->borrow_stmt());
    switch (result) {
        case SQLITE_ROW:
            break;
        case SQLITE_DONE:
            m_is_valid = false;
            break;
        default: {
          throw std::runtime_error { "invalid query" };
          break;
        }
    }
}

string
Cursor::string_value(int pos) {
    auto data = s_column_or_throw(sqlite3_column_text, m_stmt->borrow_stmt(), SQLITE_TEXT, pos);
    return string { reinterpret_cast<const char *>(data) };
}

int32_t
Cursor::int_value(int pos) {
    return s_column_or_throw(sqlite3_column_int, m_stmt->borrow_stmt(), SQLITE_INTEGER, pos);
}

int64_t
Cursor::int64_value(int pos) {
    return s_column_or_throw(sqlite3_column_int64, m_stmt->borrow_stmt(), SQLITE_INTEGER, pos);
}

double
Cursor::double_value(int pos) {
    return s_column_or_throw(sqlite3_column_double, m_stmt->borrow_stmt(), SQLITE_FLOAT, pos);
}

vector<uint8_t>
Cursor::blob_value(int pos) {
    auto stmt = m_stmt->borrow_stmt();
    const auto len = s_column_or_throw(sqlite3_column_bytes, stmt, SQLITE_BLOB, pos);
    const uint8_t * data = static_cast<const uint8_t*>( sqlite3_column_blob(stmt, pos) );
    return {data, data + len};
}

