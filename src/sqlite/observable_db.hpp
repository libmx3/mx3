#pragma once
#include <set>
#include "stl.hpp"
#include "db.hpp"
#include "transaction.hpp"

namespace mx3 { namespace sqlite {

using Row = vector<Value>;

struct RowChange final {
    int64_t rowid;
    optional<Row> old_row;
    optional<Row> new_row;
    bool operator==(const RowChange& other) const {
        return rowid == other.rowid && old_row == other.old_row && new_row == other.new_row;
    }
};

struct TableChanges final {
    vector<string> column_names;
    vector<RowChange> row_changes;
};

vector<RowChange> collapse_by_rowid(vector<RowChange>&& data);
vector<RowChange> collapse_by_pk(vector<RowChange>&& data, const vector<size_t>& pks);

vector<std::tuple<ChangeType, string, string, int64_t>>
collapse_by_rowid(vector<std::tuple<ChangeType, string, string, int64_t>> changes);

// a map of table to rows changed
using DbChanges = std::map<string, TableChanges>;

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


class ObserveConnection final {
public:
    ObserveConnection(const shared_ptr<Db>& db);
    optional<Row> read_by_id(const string& table_name, int64_t rowid);
    std::pair<vector<string>, optional<Row>> read_by_id_with_cols(const string& table_name, int64_t rowid);
    shared_ptr<Db> db;
    TransactionStmts transaction_stmts;
private:
    std::map<string, shared_ptr<Stmt>> m_fetch_stmts;
    Cursor _read_cursor(const string& table_name, int64_t rowid);
};

class ObservableDb final {
  public:
    ObservableDb(const string& path, const function<void(DbChanges)>& fn);
    ObservableDb(const string& path, const shared_ptr<DbListener>& listener);
    ~ObservableDb();
    void transaction(function<void(const shared_ptr<Db>&)> t_fn);
  private:
    ObserveConnection m_write_conn;
    ObserveConnection m_read_conn;
    shared_ptr<Stmt> m_begin_read_snapshot;
    const int32_t m_schema_version;

    vector<std::tuple<ChangeType, string, string, int64_t>> m_changes;
    shared_ptr<DbListener> m_listener;
};

} } // end namespace mx3::sqlite
