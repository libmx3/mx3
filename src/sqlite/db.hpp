#pragma once
#include <set>
#include <chrono>
#include "stl.hpp"
#include "stmt.hpp"

namespace mx3 { namespace sqlite {

// single param helpers for sqlite3_mprintf
string mprintf(const char * format, const string& data);
string mprintf(const char * format, int64_t data);
string libversion();
string sourceid();
int libversion_number();

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

enum class Checkpoint {
    PASSIVE,
    FULL,
    RESTART
};

enum class Affinity {
    TEXT,
    NUMERIC,
    INTEGER,
    REAL,
    NONE
};

struct ColumnInfo final {
    int64_t cid;
    string name;
    string type;
    Affinity type_affinity() const;
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
    struct Change final {
        ChangeType type;
        string db_name;
        string table_name;
        int64_t rowid;
    };
    using UpdateHookFn = function<void(Change)>;
    using CommitHookFn = function<bool()>;
    using RollbackHookFn = function<void()>;
    using WalHookFn = function<void(const string&, int)>;

    // use this constructor if you want to simply open the database with default settings
    static shared_ptr<Db> open(const string& path);

    // use this constructor if you want to simply open an in memory database with the default flags
    static shared_ptr<Db> open_memory();

    // the most general Db constructor, directly mirrors sqlite3_open_v2
    static shared_ptr<Db> open(const string& path, const std::set<OpenFlag>& flags, const optional<string>& vfs_name = nullopt);

    // use this constructor if you want to do anything custom to set up your database
    static shared_ptr<Db> inherit_db(sqlite3 * db);
    ~Db();

    void update_hook(const UpdateHookFn& update_fn);
    void commit_hook(const CommitHookFn& commit_fn);
    void rollback_hook(const RollbackHookFn& rollback_fn);
    void wal_hook(const WalHookFn& wal_fn);
    std::pair<int, int> wal_checkpoint_v2(const optional<string>& db_name, Checkpoint mode);

    string journal_mode();
    int64_t last_insert_rowid();
    int32_t schema_version();

    void busy_timeout(nullopt_t);
    void busy_timeout(std::chrono::system_clock::duration timeout);

    vector<TableInfo> schema_info();
    optional<TableInfo> table_info(const string& table_name);
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
    struct only_for_internal_make_shared_t;
  public:
    // make_shared constructor
    Db(only_for_internal_make_shared_t flag, unique_ptr<sqlite3, Closer> db);
  private:
    unique_ptr<sqlite3, Closer> m_db;
    // To ensure proper memory management, these hooks are owned by the database.
    unique_ptr<UpdateHookFn> m_update_hook;
    unique_ptr<CommitHookFn> m_commit_hook;
    unique_ptr<RollbackHookFn> m_rollback_hook;
    unique_ptr<WalHookFn> m_wal_hook;
};

} }
