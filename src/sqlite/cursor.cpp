#include "cursor.hpp"
#include "stmt.hpp"
#include <sqlite3/sqlite3.h>

using mx3::sqlite::Stmt;
using mx3::sqlite::Cursor;
using mx3::sqlite::Value;

namespace {
    template<typename F>
    inline auto s_column_or_throw(F&& fn, sqlite3_stmt * stmt, int expected_type, int pos) -> decltype(fn(stmt, pos)) {
        if (sqlite3_column_type(stmt, pos) != expected_type) {
            throw std::runtime_error {"invalid type for column"};
        }
        return std::forward<F>(fn)(stmt, pos);
    }
}

Value
Cursor::value_at(int pos) const {
    sqlite3_stmt * stmt = m_raw_stmt.get();
    const auto type = sqlite3_column_type(stmt, pos);
    switch (type) {
        case SQLITE_NULL: {
            return Value {nullptr};
        }
        case SQLITE_TEXT: {
            auto data = sqlite3_column_text(stmt, pos);
            return Value { string { reinterpret_cast<const char *>(data) } };
        }
        case SQLITE_INTEGER: {
            return Value { static_cast<int64_t>( sqlite3_column_int64(stmt, pos) ) };
        }
        case SQLITE_FLOAT: {
            return Value { sqlite3_column_double(stmt, pos) };
        }
        case SQLITE_BLOB: {
            const auto len = sqlite3_column_bytes(stmt, pos);
            const uint8_t * data = static_cast<const uint8_t*>( sqlite3_column_blob(stmt, pos) );
            return Value { vector<uint8_t> {data, data + len} };
        }
    }
    return Value {nullptr};
}

vector<Value>
Cursor::values() const {
    vector<Value> values;
    const auto col_count = this->column_count();
    values.reserve(col_count);
    for (int i=0; i < col_count; i++) {
        values.push_back( this->value_at(i) );
    }
    return values;
}

std::map<string, Value>
Cursor::value_map() const {
    std::map<string, Value> all_values;
    const auto col_count = this->column_count();
    for (int i=0; i < col_count; i++) {
        all_values.emplace( this->column_name(i), this->value_at(i) );
    }
    return all_values;
}


void
Cursor::Resetter::operator() (sqlite3_stmt * stmt) {
    auto error_code = sqlite3_reset(stmt);
    if (error_code != SQLITE_OK) {
        // this is in the dtor, so don't throw
    }
}

Cursor::Cursor(shared_ptr<Stmt> stmt, bool is_valid)
    : m_stmt {std::move(stmt)}
    , m_raw_stmt {m_stmt->borrow_stmt()}
    , m_is_valid {is_valid}
{}

sqlite3_stmt *
Cursor::borrow_stmt() const {
    return m_raw_stmt.get();
}

sqlite3 *
Cursor::borrow_db() const {
    return m_stmt->borrow_db();
}

string
Cursor::column_name(int pos) const {
    auto name = sqlite3_column_name(m_raw_stmt.get(), pos);
    return string {name};
}

vector<string>
Cursor::column_names() const {
    vector<string> names;
    const auto col_count = this->column_count();
    names.reserve(col_count);
    for (int i=0; i < col_count; i++) {
        names.push_back( this->column_name(i) );
    }
    return names;
}

int
Cursor::column_count() const {
    return sqlite3_column_count(m_raw_stmt.get());
}

void
Cursor::next() {
    auto result = sqlite3_step(m_raw_stmt.get());
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

bool
Cursor::is_null(int pos) const {
    return sqlite3_column_type(m_raw_stmt.get(), pos) == SQLITE_NULL;
}

string
Cursor::string_value(int pos) const {
    auto data = s_column_or_throw(sqlite3_column_text, m_raw_stmt.get(), SQLITE_TEXT, pos);
    return string { reinterpret_cast<const char *>(data) };
}

int32_t
Cursor::int_value(int pos) const {
    return s_column_or_throw(sqlite3_column_int, m_raw_stmt.get(), SQLITE_INTEGER, pos);
}

int64_t
Cursor::int64_value(int pos) const {
    return s_column_or_throw(sqlite3_column_int64, m_raw_stmt.get(), SQLITE_INTEGER, pos);
}

double
Cursor::double_value(int pos) const {
    return s_column_or_throw(sqlite3_column_double, m_raw_stmt.get(), SQLITE_FLOAT, pos);
}

vector<uint8_t>
Cursor::blob_value(int pos) const {
    auto stmt = m_raw_stmt.get();
    const auto len = s_column_or_throw(sqlite3_column_bytes, stmt, SQLITE_BLOB, pos);
    const uint8_t * data = static_cast<const uint8_t*>( sqlite3_column_blob(stmt, pos) );
    return vector<uint8_t> {data, data + len};
}

