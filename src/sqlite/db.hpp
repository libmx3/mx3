#pragma once
#include "stl.hpp"

// forward delcare these structs to keep from having to include paths propigate
struct sqlite3;
struct sqlite3_stmt;

namespace mx3 { namespace sqlite {

class Query final {
  public:
    bool has_next() { return !m_is_done; }
    void next();
    string get_string(int pos);
  private:
    friend class Stmt;
    struct NoopDelete final {
        void operator() (sqlite3_stmt * stmt) {}
    };
    Query(sqlite3_stmt * stmt, bool is_done);
    bool m_is_done;
    unique_ptr<sqlite3_stmt, NoopDelete> m_stmt;
};

class Stmt final {
  public:
    void bind(int pos, const vector<uint8_t>& value);
    void bind(int pos, double value);
    void bind(int pos, int32_t value);
    void bind(int pos, int64_t value);
    void bind(int pos, std::nullptr_t value);
    void bind(int pos, const string& value);
    int exec();
    Query exec_query();
  private:
    friend class Db;
    struct Deleter final {
        void operator() (sqlite3_stmt * stmt);
    };
    Stmt(sqlite3_stmt * stmt);
    unique_ptr<sqlite3_stmt, Deleter> m_stmt;
};

class Db final : public std::enable_shared_from_this<Db> {
  public:
    static shared_ptr<Db> open(const string& path);
    Db(const string& path);
    ~Db();
    void exec(const string& sql);
    Stmt prepare(const string& sql);
  private:
    sqlite3 * m_db;
};

} }
