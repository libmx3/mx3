#pragma once
#include <set>
#include "stl.hpp"
#include "db.hpp"

namespace mx3 { namespace sqlite {

using Row = vector<Value>;

struct RowChange final {
    optional<Row> old_row;
    optional<Row> new_row;
};

class TableChanges final {
  public:
    vector<string> column_names;
    vector<RowChange> row_changes;
};

// a map of table to rows changed
using DbChanges = std::map<string, TableChanges>;

namespace detail {

class TransactionDb final {
public:
    TransactionDb(const string& path);
    optional<Row> read_by_id(const string& table_name, int32_t schema_ver, int64_t rowid);
    std::pair<vector<string>, optional<Row>> read_by_id_with_cols(const string& table_name, int32_t schema_ver, int64_t rowid);
    void begin();
    void commit();
    void rollback();
    shared_ptr<Db> db;
private:
    mx3::sqlite::Cursor _read_cursor(const string& table_name, int32_t schema_ver, int64_t rowid);
    friend class TransactionGuard;
    shared_ptr<Stmt> m_begin;
    shared_ptr<Stmt> m_commit;
    shared_ptr<Stmt> m_rollback;
    int32_t m_schema_version;
    std::map<string, shared_ptr<Stmt>> m_read_stmts;
};

class TransactionGuard final {
public:
    TransactionGuard(TransactionDb& db);
    TransactionGuard(const TransactionGuard& other) = delete;
    TransactionGuard& operator=(const TransactionGuard& other) = delete;
    ~TransactionGuard();
    void commit();
    void rollback();
private:
    enum class State {
        NONE,
        COMMIT,
        ROLLBACK
    };
    TransactionDb& m_db;
    State m_state;
};

} // end namespace detail

class DbListener {
public:
    virtual void on_change(DbChanges changes) = 0;
};

// a class which allows you to use a lambda as DbListener
class FnDbListener final : public DbListener {
public:
    FnDbListener(const std::function<void(DbChanges)>& fn) : m_fn{fn} {}
    virtual void on_change(DbChanges changes) override { m_fn(std::move(changes)); }
private:
    function<void(DbChanges)> m_fn;
};

class ObservableDb final {
  public:
    ObservableDb(const string& path, const function<void(DbChanges)>& fn);
    ObservableDb(const string& path, const shared_ptr<DbListener>& listener);
    void transaction(function<void(const shared_ptr<Db>&)> t_fn);
  private:
    detail::TransactionDb m_write;
    detail::TransactionDb m_read;
    shared_ptr<Stmt> m_begin_read_snapshot;
    vector<std::tuple<ChangeType, string, string, int64_t>> m_changes;
    shared_ptr<DbListener> m_listener;
};

} } // end namespace mx3::sqlite
