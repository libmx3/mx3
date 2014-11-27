#pragma once
#include "stl.hpp"

// forward delcare these structs to keep from having to include paths propigate
struct sqlite3;
struct sqlite3_stmt;

namespace mx3 { namespace sqlite {

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
    struct Reset final {
        void operator() (sqlite3_stmt * stmt);
    };
    Cursor(sqlite3_stmt * stmt, bool is_done);
    bool m_is_done;
    unique_ptr<sqlite3_stmt, Reset> m_stmt;
};

class Stmt final {
  public:
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
    struct Deleter final {
        void operator() (sqlite3_stmt * stmt);
    };
    Stmt(sqlite3_stmt * stmt);
    unique_ptr<sqlite3_stmt, Deleter> m_stmt;
};

class Db final {
  public:
    static shared_ptr<Db> open(const string& path);
    Db(const string& path);
    void exec(const string& sql);
    Stmt prepare(const string& sql);
  private:
    struct Closer final {
        void operator() (sqlite3 * db);
    };
    unique_ptr<sqlite3, Closer> m_db;
};

} }
