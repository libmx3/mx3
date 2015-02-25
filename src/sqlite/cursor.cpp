#include "cursor.hpp"
#include "stmt.hpp"
#include <sqlite3/sqlite3.h>

using mx3::sqlite::Stmt;
using mx3::sqlite::Cursor;
using mx3::sqlite::Value;

Value
Cursor::value_at(int pos) const {
    sqlite3_stmt * stmt = m_raw_stmt.get();
    sqlite3_value * value = sqlite3_column_value(stmt, pos);
    const auto type = sqlite3_value_type(value);

    switch (type) {
        case SQLITE_NULL: {
            return Value {nullptr};
        }
        case SQLITE_TEXT: {
            auto data = sqlite3_value_text(value);
            return Value { string { reinterpret_cast<const char *>(data) } };
        }
        case SQLITE_INTEGER: {
            return Value { static_cast<int64_t>( sqlite3_value_int64(value) ) };
        }
        case SQLITE_FLOAT: {
            return Value { sqlite3_value_double(value) };
        }
        case SQLITE_BLOB: {
            const auto len = sqlite3_value_bytes(value);
            const uint8_t * data = static_cast<const uint8_t*>( sqlite3_value_blob(value) );
            return Value { vector<uint8_t> {data, data + len} };
        }
    }
    return Value {nullptr};
}

vector<Value>
Cursor::values() const {
    vector<Value> values;
    const int col_count = this->column_count();
    values.reserve(col_count);
    for (int i=0; i < col_count; i++) {
        values.push_back( this->value_at(i) );
    }
    return values;
}

vector<vector<Value>>
Cursor::all_rows() {
    vector<vector<Value>> rows;
    while (this->is_valid()) {
        rows.push_back(this->values());
        this->next();
    }
    return rows;
}

std::map<string, Value>
Cursor::value_map() const {
    std::map<string, Value> all_values;
    const int col_count = this->column_count();
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

Cursor::Cursor(const shared_ptr<Stmt>& stmt, bool is_valid)
    : m_stmt {stmt}
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
    const char * name = sqlite3_column_name(m_raw_stmt.get(), pos);
    return string {name};
}

vector<string>
Cursor::column_names() const {
    vector<string> names;
    const int col_count = this->column_count();
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
    return this->value_at(pos).is_null();
}

string
Cursor::string_value(int pos) const {
    return this->value_at(pos).string_value();
}

int32_t
Cursor::int_value(int pos) const {
    return this->value_at(pos).int_value();
}

int64_t
Cursor::int64_value(int pos) const {
    return this->value_at(pos).int64_value();
}

double
Cursor::double_value(int pos) const {
    return this->value_at(pos).double_value();
}

vector<uint8_t>
Cursor::blob_value(int pos) const {
    return this->value_at(pos).blob_value();
}

