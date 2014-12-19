#pragma once
#include <set>
#include "stl.hpp"
#include "stmt.hpp"

namespace mx3 { namespace sqlite {

// single param helpers for sqlite3_mprintf
string mprintf(const char * format, const string& data);
string mprintf(const char * format, int64_t data);

enum class ChangeType {
    INSERT,
    UPDATE,
    DELETE
};

enum class OpenFlag {
    READONLY,
    READWRITE,
    CREATE,
    URI,
    MEMORY,
    NOMUTEX,
    FULLMUTEX,
    SHAREDCACHE,
    PRIVATECACHE
};

struct ColumnInfo final {
    int64_t cid;
    string name;
    string type;
    bool notnull;
    optional<string> dflt_value;
    int32_t pk;
    bool is_pk() const { return pk != 0; }
};

struct TableInfo final {
    string name;
    string sql;
    int64_t rootpage;
    vector<ColumnInfo> columns;
};


class Db final : public std::enable_shared_from_this<Db> {
  public:
    using UpdateHookFn   = function<void(ChangeType, string, string, int64_t)>;
    using CommitHookFn   = function<bool()>;
    using RollbackHookFn = function<void()>;

    // use this constructor if you want to simply open the database with default settings
    static shared_ptr<Db> open(const string& path);

    // use this constructor if you want to simply open an in memory database with the default flags
    static shared_ptr<Db> open_memory();

    // the most general Db constructor, directly mirrors sqlite3_open_v2
    static shared_ptr<Db> open(const string& path, const std::set<OpenFlag>& flags, const optional<string>& vfs_name = nullopt);

    // use this constructor if you want to do anything custom to set up your database
    static shared_ptr<Db> inherit_db(sqlite3 * db);
    ~Db();

    void update_hook(UpdateHookFn update_fn);
    void commit_hook(CommitHookFn commit_fn);
    void rollback_hook(RollbackHookFn rollback_fn);
    int64_t last_insert_rowid();

    int32_t schema_version();

    vector<TableInfo> schema_info();
    vector<ColumnInfo> column_info(const string& table_name);

    int32_t user_version();
    void set_user_version(int32_t user_ver);

    // give the ability to fetch the raw db pointer (in case you need to do anything special)
    sqlite3 * borrow_db();
    shared_ptr<Stmt> prepare(const string& sql);
    void exec(const string& sql);
    int64_t exec_scalar(const string& sql);
    void enable_wal();
    void close();
  private:
    struct Closer final {
        void operator() (sqlite3 * db) const;
    };
    Db(unique_ptr<sqlite3, Closer> db);
    unique_ptr<sqlite3, Closer> m_db;
};

} }
