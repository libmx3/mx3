#pragma once
#include "stl.hpp"
#include "value.hpp"

// forward delcare these structs to keep from having to include paths propigate
struct sqlite3;
struct sqlite3_stmt;

namespace mx3 { namespace sqlite {

class Stmt;

class Cursor final {
  public:
    sqlite3_stmt * borrow_stmt() const;
    sqlite3 * borrow_db() const;

    bool is_valid() const { return m_is_valid; }
    void next();

    int column_count() const;
    string column_name(int pos) const;
    vector<string> column_names() const;

    Value value_at(int pos) const;

    // returns a vector of values in the current row.
    // the size of the vector will exactly be the number of columns see `column_count`
    vector<Value> values() const;

    vector<vector<Value>> all_rows();

    // gives a map of column_name -> value
    // this is a convenience method, and combining `column_names` `values` will be more efficient
    // please note that if you have a query with repeated values, this will not capture them
    // 'SELECT a, a, b, c FROM data' <-- the duplicate 'a' will be squashed
    std::map<string, Value> value_map() const;

    bool is_null(int pos) const;
    int32_t int_value(int pos) const;
    int64_t int64_value(int pos) const;
    double double_value(int pos) const;
    string string_value(int pos) const;
    vector<uint8_t> blob_value(int pos) const;

  private:
    friend class Stmt;
    struct Resetter final {
        void operator() (sqlite3_stmt * stmt);
    };
    Cursor(const shared_ptr<Stmt>& stmt, bool is_valid);

    // this keeps the statement alive while we are executing the query
    shared_ptr<Stmt> m_stmt;
    // a helper used for RAII cursor resetting
    unique_ptr<sqlite3_stmt, Resetter> m_raw_stmt;
    bool m_is_valid;
};

} }
