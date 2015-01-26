#include "observable_db.hpp"
#include <sqlite3/sqlite3.h>
#include <unordered_set>
#include <set>
#include <iostream>
#include <algorithm>

namespace mx3 {
namespace sqlite {

namespace {
    struct ChangeClearer final {
        void operator() (vector<Db::Change> * changes) const {
            changes->clear();
        }
    };
} // end anon namespace

static string s_select_by_rowid(const string& table_name, const vector<ColumnInfo>& columns) {
    string sql = "SELECT ";
    for (const auto& c : columns) {
        if (&c != &columns[0]) {
            sql += ", " + escape_column(c.name);
        } else {
            sql += escape_column(c.name);
        }
    }
    sql += " FROM " + escape_column(table_name) + " WHERE rowid = ?1";
    return sql;
}

static shared_ptr<Db> s_open_wal(const string& path) {
    const auto db = Db::open(path, {
        OpenFlag::READWRITE,
        // multi-threaded mode
        OpenFlag::NOMUTEX,
        OpenFlag::PRIVATECACHE
    });
    db->enable_wal();
    return db;
}

namespace detail {

vector<size_t> get_pk_pos(const TableInfo& table_info) {
    vector<std::pair<int32_t, size_t>> pks;
    size_t i = 0;
    for (const auto& col : table_info.columns) {
        if (col.is_pk()) {
            pks.emplace_back(col.pk, i);
        }
        i++;
    }
    // Sort the pks by their primary key 'importance'
    std::sort(pks.begin(), pks.end());
    vector<size_t> pk_idx;
    pk_idx.reserve(pks.size());
    for (const auto p : pks) {
        pk_idx.push_back(p.second);
    }
    return pk_idx;
}

std::map<string, vector<size_t>> get_pk_pos(const vector<TableInfo>& schema_info) {
    std::map<string, vector<size_t>> pks_by_table;
    for (const auto& table_info : schema_info) {
        pks_by_table.emplace(table_info.name, get_pk_pos(table_info));
    }
    return pks_by_table;
}

vector<Db::Change> collapse_by_rowid(vector<Db::Change> changes) {
    std::set<std::tuple<int64_t, string, string>> seen;
    std::reverse(changes.begin(), changes.end());
    const auto new_end = std::remove_if(changes.begin(), changes.end(), [&seen] (const Db::Change& c) -> bool {
        const bool did_insert = seen.emplace(c.rowid, c.table_name, c.db_name).second;
        // Remove this if we weren't the first instance of (rowid, table, db)
        // this means "last writer wins".
        return !did_insert;
    });
    changes.erase(new_end, changes.end());
    std::reverse(changes.begin(), changes.end());
    return changes;
}

optional<vector<Value>> extract_primary_key(const optional<vector<Value>>& row, const vector<size_t>& pk_positions) {
    if (row) {
        vector<Value> primary_key;
        primary_key.reserve(pk_positions.size());
        for (const auto pos : pk_positions) {
            if (pos >= row->size()) {
                throw std::runtime_error {"Bad state to extract primary key"};
            }
            primary_key.push_back((*row)[pos]);
        }
        return primary_key;
    }
    return nullopt;
}

vector<Value> extract_primary_key(const RowChange& change, const vector<size_t>& pk_positions) {
    if (!change.old_row && !change.new_row) {
        throw std::runtime_error {"Cannot extract primary of an empty update"};
    }
    const auto old_primary_key = extract_primary_key(change.old_row, pk_positions);
    const auto new_primary_key = extract_primary_key(change.new_row, pk_positions);
    if (old_primary_key && new_primary_key && old_primary_key != new_primary_key) {
        throw std::runtime_error {"Bad state same update classified different primary key"};
    }
    return old_primary_key ? old_primary_key.value() : new_primary_key.value();
}

TableChanges allow_first_change(TableChanges&& changes, const vector<size_t>& pk_positions) {
    std::set<std::vector<Value>> seen;
    auto& row_changes = changes.row_changes;
    const auto new_end = std::remove_if(row_changes.begin(), row_changes.end(), [&seen, &pk_positions] (const RowChange& c) {
        vector<Value> primary_key = extract_primary_key(c, pk_positions);
        const bool did_insert = seen.insert(std::move(primary_key)).second;
        return !did_insert;
    });
    row_changes.erase(new_end, row_changes.end());
    return changes;
}

DbChanges allow_first_change(DbChanges&& changes, const std::map<string, vector<size_t>>& pk_positions) {
    for (auto& table_mapping : changes) {
        const auto it = pk_positions.find(table_mapping.first);
        if (it == pk_positions.end()) {
            throw std::runtime_error {"table not found" + table_mapping.first};
        }
        // if the pk positions are empty, no update collapsing needs to happen
        if (!it->second.empty()) {
            table_mapping.second = allow_first_change( std::move(table_mapping.second), it->second );
        }
    }
    return changes;
}

} // end namespace detail

string escape_column(const string& column) {
    string result;
    result.reserve(column.size() + 3);
    result += '"';
    for (const char c : column) {
        if (c == '"') {
            result += "\"\"";
        } else {
            result += c;
        }
    }
    result += '"';
    return result;
}

ObserveConnection::ObserveConnection(const shared_ptr<Db>& set_db)
    : db{set_db}
    , transaction_stmts{db}
{}

optional<mx3::sqlite::Row>
ObserveConnection::read_by_id(const string& table_name, int64_t rowid) {
    Cursor c = _read_cursor(table_name, rowid);
    return c.is_valid() ? c.values() : optional<mx3::sqlite::Row>{};
}

std::pair<vector<string>, optional<mx3::sqlite::Row>>
ObserveConnection::read_by_id_with_cols(const string& table_name, int64_t rowid) {
    Cursor c = _read_cursor(table_name, rowid);
    return {c.column_names(), c.is_valid() ? c.values() : optional<mx3::sqlite::Row>{}};
}

mx3::sqlite::Cursor ObserveConnection::_read_cursor(const string& table_name, int64_t rowid) {
    auto it = m_fetch_stmts.find(table_name);
    if (it == m_fetch_stmts.end()) {
        const auto table_info = this->db->table_info(table_name);
        if (!table_info) {
            throw std::runtime_error {"table not found"};
        }
        const auto stmt = this->db->prepare( s_select_by_rowid(table_name, table_info->columns) );
        it = m_fetch_stmts.emplace(table_name, stmt).first;
    }

    const auto& stmt = it->second;
    stmt->reset();
    stmt->bind(1, rowid);
    return stmt->exec_query();
}

ObservableDb::ObservableDb(const string& path, const function<void(DbChanges)>& fn)
    : ObservableDb(path, make_shared<FnDbListener>(fn)) {}

ObservableDb::ObservableDb(const string& path, const shared_ptr<DbListener>& listener)
    : m_write_conn {s_open_wal(path)}
    , m_read_conn {s_open_wal(path)}
    // In order to actually create a read snapshot, you need to issue a BEGIN
    // _and_ an actual select statement.  This select statement will work for any database.
    , m_begin_read_snapshot {m_read_conn.db->prepare("SELECT 1 FROM sqlite_master LIMIT 1")}
    , m_schema_version {m_write_conn.db->schema_version()}
    , m_listener {listener}
{
    m_write_conn.db->update_hook([this] (Db::Change change) {
        m_changes.push_back(std::move(change));
    });
}

ObservableDb::~ObservableDb() {
    m_write_conn.db->update_hook(nullptr);
}

void
ObservableDb::transaction(function<void(const shared_ptr<Db>&)> transaction_fn) {
    unique_ptr<decltype(m_changes), ChangeClearer> clean_on_exit {&m_changes};
    TransactionGuard read_guard {m_read_conn.transaction_stmts};
    // Although this line appears to do nothing, sqlite defers opening a read transaction until
    // a select statement has been issued.
    m_begin_read_snapshot->exec_scalar();

    {
        TransactionGuard write_guard {m_write_conn.transaction_stmts};
        transaction_fn(m_write_conn.db);
        const int32_t new_schema_version = m_write_conn.db->schema_version();
        if (new_schema_version != m_schema_version) {
            throw std::runtime_error{"Transactions can't change the schema version"};
        }
        write_guard.commit();
    }

    // Reopen a transaction on the write database, to ensure a consistent view of the
    // rows that we will be requerying.
    TransactionGuard write_guard {m_write_conn.transaction_stmts};

    decltype(m_changes) changes_copy;
    std::swap(m_changes, changes_copy);
    DbChanges db_changes = _collect_changes(std::move(changes_copy));

    write_guard.commit();
    read_guard.commit();
    // need to release this before calling on_change, since the callback could modify m_changes
    clean_on_exit = nullptr;
    m_listener->on_change(std::move(db_changes));
}

DbChanges ObservableDb::_collect_changes(vector<Db::Change> changes) {
    changes = detail::collapse_by_rowid(std::move(changes));

    DbChanges db_changes;
    for (const auto& c : changes) {
        RowChange current_change;
        current_change.rowid = c.rowid;
        auto table_changes_it = db_changes.find(c.table_name);

        // todo(kabbes) can prevent some reads by inspecting the ChangeType
        if (table_changes_it == db_changes.end()) {
            auto p = m_read_conn.read_by_id_with_cols(c.table_name, c.rowid);
            current_change.old_row = std::move(p.second);
            TableChanges table_changes;
            table_changes.column_names = std::move(p.first);
            table_changes_it = db_changes.emplace(c.table_name, std::move(table_changes)).first;
        } else {
            current_change.old_row = m_read_conn.read_by_id(c.table_name, c.rowid);
        }
        current_change.new_row = m_write_conn.read_by_id(c.table_name, c.rowid);

        // If both are null, then we know that this entity has been added and deleted in
        // the same transaction, so we can safely ignore it.
        if (current_change.old_row || current_change.new_row) {
            table_changes_it->second.row_changes.emplace_back(std::move(current_change));
        }
    }

    // Flip the order of changes back again, since we collected them in reverse order.
    for (auto& mapping : db_changes) {
        vector<RowChange>& row_changes = mapping.second.row_changes;
        std::reverse(row_changes.begin(), row_changes.end());
    }

    return db_changes;
}

} } // end namespace mx3::sqlite
