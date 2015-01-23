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

class TransactionDb final {
  public:
    TransactionDb(const string& path);
    optional<Row> read_by_id(const string& table_name, int32_t schema_ver, int64_t rowid);
    void begin();
    void commit();
    shared_ptr<Db> db;
  private:
    shared_ptr<Stmt> m_begin;
    shared_ptr<Stmt> m_commit;
    int32_t m_schema_version;
    std::map<string, shared_ptr<Stmt>> m_read_stmts;
};

class ObservableDb final {
  public:
    ObservableDb(const string& path);
    void transaction(function<void(const shared_ptr<Db>&)> t_fn);
  private:
    TransactionDb m_write;
    TransactionDb m_read;
    vector<std::tuple<ChangeType, string, string, int64_t>> m_changes;
};

} }
