#include "stmt.hpp"
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

struct Stmt::only_for_internal_make_shared_t {};

Stmt::Stmt(only_for_internal_make_shared_t, sqlite3_stmt * stmt, const shared_ptr<Db>& db)
    : m_db {db}, m_stmt {stmt} {}

shared_ptr<Stmt> Stmt::create(sqlite3_stmt * stmt, const shared_ptr<Db>& db) {
    return make_shared<Stmt>(only_for_internal_make_shared_t{}, stmt, db);
}

sqlite3_stmt *
Stmt::borrow_stmt() const {
    return m_stmt.get();
}

sqlite3 *
Stmt::borrow_db() const {
    return m_db->borrow_db();
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
Stmt::bind(int pos, const Value& value) {
    switch (value.type()) {
    case Value::Type::NUL: {
        s_throw_on_sqlite_error(sqlite3_bind_null, m_stmt.get(), pos);
        break;
    }
    case Value::Type::INT: {
        s_throw_on_sqlite_error(sqlite3_bind_int64, m_stmt.get(), pos, value.int64_value());
        break;
    }
    case Value::Type::DOUBLE: {
        s_throw_on_sqlite_error(sqlite3_bind_double, m_stmt.get(), pos, value.double_value());
        break;
    }
    case Value::Type::STRING: {
        const string& str_value = value.string_value();
        s_throw_on_sqlite_error(sqlite3_bind_text, m_stmt.get(), pos, str_value.c_str(),
                                static_cast<int>(str_value.length()), SQLITE_TRANSIENT);
        break;
    }
    case Value::Type::BLOB: {
        const vector<uint8_t>& blob_value = value.blob_value();
        s_throw_on_sqlite_error(sqlite3_bind_blob, m_stmt.get(), pos, blob_value.data(),
                                static_cast<int>(blob_value.size()), SQLITE_TRANSIENT);
        break;
    }
    }
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

int64_t
Stmt::exec_scalar() {
    this->reset();
    auto cursor = this->exec_query();
    if (!cursor.is_valid()) {
        throw std::runtime_error { "not a scalar query" };
    }
    return cursor.int64_value(0);
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
