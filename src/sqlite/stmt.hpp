#pragma once
#include "stl.hpp"
#include "value.hpp"
#include "cursor.hpp"

namespace mx3 { namespace sqlite {

class Db;

class Stmt final : public std::enable_shared_from_this<Stmt> {
  public:
    sqlite3_stmt * borrow_stmt() const;
    sqlite3 * borrow_db() const;

    // this is how many parameters this statment expects
    int param_count() const;
    // the name of the indexed parameter.  Some parameters are nameless and therefore
    // will return nullopt
    optional<string> param_name(int pos) const;
    // the index of the named parameter
    int param_index(const string& name) const;

    int exec();
    Cursor exec_query();
    int64_t exec_scalar();
    void reset();
    void clear_bindings();

    void bind(int pos, const Value& value);

    // allow optional types to be bound
    template<typename WrappedType>
    void bind(int pos, const optional<WrappedType>& value) {
        if (value) {
            this->bind(pos, *value);
        } else {
            this->bind(pos, nullptr);
        }
    }
  private:
    friend class Db;
    struct Finalizer final {
        void operator() (sqlite3_stmt * stmt);
    };
    struct only_for_internal_make_shared_t;
  public:
    // make_shared ctor
    Stmt(only_for_internal_make_shared_t flag, sqlite3_stmt * stmt, const shared_ptr<Db>& db);
  private:
    static shared_ptr<Stmt> create(sqlite3_stmt * stmt, const shared_ptr<Db>& db);
    Stmt(sqlite3_stmt * stmt, shared_ptr<Db> db);
    shared_ptr<Db> m_db;
    unique_ptr<sqlite3_stmt, Finalizer> m_stmt;
};

} }
