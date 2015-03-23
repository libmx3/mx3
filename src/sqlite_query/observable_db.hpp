#pragma once
#include <set>
#include "stl.hpp"
#include "../sqlite/sqlite.hpp"

namespace mx3 { namespace sqlite {

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

namespace detail {
vector<Db::Change> collapse_by_rowid(vector<Db::Change> changes);
// Get the primary key positions for a single table.
vector<size_t> get_pk_pos(const TableInfo& table_info);
// Get all the primary key positions for the given database schema.
std::map<string, vector<size_t>> get_pk_pos(const vector<TableInfo>& schema_info);

optional<vector<Value>> extract_primary_key(const optional<vector<Value>>& row, const vector<size_t>& pk_positions);
vector<Value> extract_primary_key(const RowChange& change, const vector<size_t>& pk_positions);
TableChanges allow_first_change(TableChanges&& changes, const vector<size_t>& pk_positions);
DbChanges allow_first_change(DbChanges&& changes, const std::map<string, vector<size_t>>& pk_positions);

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

}

namespace experimental {

class ObservableDb final {
public:
    ObservableDb(
        const shared_ptr<Db>& write_db,
        const shared_ptr<Db>& read_db,
        const function<void(DbChanges)>& fn);
    ObservableDb(
        const shared_ptr<Db>& write_db,
        const shared_ptr<Db>& read_db,
        const shared_ptr<DbListener>& listener);
    ~ObservableDb();
private:
    void _on_update(Db::Change change);
    bool _on_commit();
    void _on_rollback();
    void _on_wal(const string& db_name, int pages);

    DbChanges _collect_changes(vector<Db::Change> changes);
    const shared_ptr<Db> m_write_db;
    detail::ObserveConnection m_write_conn;
    detail::ObserveConnection m_read_conn;
    unique_ptr<ReadTransaction> m_read_snapshot;
    const int32_t m_schema_version;
    vector<Db::Change> m_changes;
    shared_ptr<DbListener> m_listener;
};

} // end namespace experimental

} } // end namespace mx3::sqlite
