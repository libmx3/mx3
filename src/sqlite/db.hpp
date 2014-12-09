#pragma once
#include <set>
#include "stl.hpp"
#include "stmt.hpp"

namespace mx3 { namespace sqlite {

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

    void update_hook(UpdateHookFn update_fn);
    void commit_hook(CommitHookFn commit_fn);
    void rollback_hook(RollbackHookFn rollback_fn);
    int64_t last_insert_rowid();

    // give the ability to fetch the raw db pointer (in case you need to do anything special)
    sqlite3 * borrow_db();
    shared_ptr<Stmt> prepare(const string& sql);
    void exec(const string& sql);
    int64_t exec_scalar(const string& sql);
    void close();
  private:
    struct Closer final {
        void operator() (sqlite3 * db);
    };
    Db(unique_ptr<sqlite3, Closer> db);
    unique_ptr<sqlite3, Closer> m_db;

    UpdateHookFn m_update_hook;
    CommitHookFn m_commit_hook;
    RollbackHookFn m_rollback_hook;
};

} }
