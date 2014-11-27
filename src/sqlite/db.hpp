#pragma once
#include "stl.hpp"

// forward delcare these structs to keep from having to include paths propigate
struct sqlite3;
struct sqlite3_stmt;

namespace mx3 { namespace sqlite {

class Stmt;
class Db;

class Cursor final {
  public:
    bool has_next() { return !m_is_done; }
    void next();

    string column_name(int pos) const;
    int column_count() const;

    int32_t int_value(int pos);
    int64_t int64_value(int pos);
    double double_value(int pos);
    string string_value(int pos);
    vector<uint8_t> blob_value(int pos);

  private:
    friend class Stmt;
    Cursor(shared_ptr<Stmt> stmt, bool is_done);

    // this keeps the statement alive while we are executing the query
    shared_ptr<Stmt> m_stmt;
    bool m_is_done;
    unique_ptr<bool> m_dont_let_anyone_copy_me;
};

class Stmt final : public std::enable_shared_from_this<Stmt> {
  public:
    sqlite3_stmt * borrow_stmt();

    // this is how many parameters this statment expects
    int param_count() const;
    // the name of the indexed parameter.  Some parameters are nameless and therefore
    // will return nullopt
    optional<string> param_name(int pos) const;
    // the index of the named parameter
    int param_index(const string& name) const;

    void bind(int pos, const vector<uint8_t>& value);
    void bind(int pos, double value);
    void bind(int pos, int32_t value);
    void bind(int pos, int64_t value);
    void bind(int pos, std::nullptr_t value);
    void bind(int pos, const string& value);

    // allow optional types to be bound
    template<typename WrappedType>
    void bind(int pos, const optional<WrappedType>& value) {
        if (value) {
            this->bind(pos, *value);
        } else {
            this->bind(pos, nullptr);
        }
    }

    int exec();
    Cursor exec_query();
    void reset();
    void clear_bindings();
  private:
    friend class Db;
    struct Finalizer final {
        void operator() (sqlite3_stmt * stmt);
    };
    Stmt(sqlite3_stmt * stmt, shared_ptr<Db> db);
    shared_ptr<Db> m_db;
    unique_ptr<sqlite3_stmt, Finalizer> m_stmt;
};

class Db final : public std::enable_shared_from_this<Db> {
  public:
    static shared_ptr<Db> open(const string& path);
    sqlite3 * borrow_db();
    shared_ptr<Stmt> prepare(const string& sql);
    void exec(const string& sql);
  private:
    Db(const string& path);
    struct Closer final {
        void operator() (sqlite3 * db);
    };
    unique_ptr<sqlite3, Closer> m_db;
};

} }
