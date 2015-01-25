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
        void operator() (vector<std::tuple<ChangeType, string, string, int64_t>> * changes) const {
            changes->clear();
        }
    };

    string s_select_by_rowid(const string& table_name, const vector<ColumnInfo>& columns) {
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
    shared_ptr<Db> s_open_wal(const string& path) {
        const auto db = Db::open(path, {
            OpenFlag::CREATE,
            OpenFlag::READWRITE,
            // multi-threaded mode
            OpenFlag::NOMUTEX,
            OpenFlag::PRIVATECACHE
        });
        db->enable_wal();
        return db;
    }
} // end anon namespace

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

vector<std::tuple<ChangeType, string, string, int64_t>>
collapse_by_rowid(vector<std::tuple<ChangeType, string, string, int64_t>> changes) {
    vector<std::tuple<ChangeType, string, string, int64_t>> new_changes;
    new_changes.reserve(changes.size());
    std::set<std::tuple<int64_t, string, string>> seen;
    const auto r_begin = changes.rbegin();
    const auto r_end   = changes.rend();
    for (auto it = r_begin; it != r_end; it++) {
        const bool did_insert = seen.emplace(
                std::get<3>(*it),
                std::get<2>(*it),
                std::get<1>(*it)).second;
        if (did_insert) {
            new_changes.push_back(std::move(*it));
        }
    }
    // reverse the final output to preserve the original ordering (as much as possible)
    std::reverse(new_changes.begin(), new_changes.end());
    return new_changes;
}

// Post process RowChanges to squash duplicate modifications (based on rowid)
vector<RowChange> collapse_by_rowid(vector<RowChange>&& changes) {
    // Iterate changes in reverse and push into the new vector if no duplicates have been
    // found yet. Then, reverse the output array to maintain stability.
    vector<RowChange> new_changes;
    new_changes.reserve(changes.size());
    std::unordered_set<int64_t> seen;
    seen.reserve(changes.size());

    // going backwards, mark latest
    const auto r_begin = changes.rbegin();
    const auto r_end   = changes.rend();
    for (auto it = r_begin; it != r_end; it++) {
        const bool did_insert = seen.emplace(it->rowid).second;
        // If there is an update that is (null -> null), then swallow it.
        // It was an insert followed by a delete in the same transaction.
        if (did_insert && (it->old_row || it->new_row)) {
            new_changes.push_back(std::move(*it));
        }
    }

    // reverse the final output to preserve the original ordering (as much as possible)
    std::reverse(new_changes.begin(), new_changes.end());
    return new_changes;
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
    m_write_conn.db->update_hook([this] (ChangeType type, string db_name, string table_name, int64_t rowid) {
        m_changes.emplace_back(type, std::move(db_name), std::move(table_name), rowid);
    });
}

ObservableDb::~ObservableDb() {
    m_write_conn.db->update_hook(nullptr);
}

void
ObservableDb::transaction(function<void(const shared_ptr<Db>&)> transaction_fn) {
    const unique_ptr<decltype(m_changes), ChangeClearer> clean_on_exit {&m_changes};
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

    DbChanges db_changes;
    decltype(m_changes) changes_copy;
    std::swap(m_changes, changes_copy);
    changes_copy = collapse_by_rowid(std::move(changes_copy));

    for (const auto& c : changes_copy) {
        // const ChangeType type    = std::get<0>(c);
        // const string& db_name    = std::get<1>(c);
        const string& table_name = std::get<2>(c);
        int64_t rowid            = std::get<3>(c);

        RowChange current_change;
        current_change.rowid = rowid;
        auto table_changes_it = db_changes.find(table_name);
        if (table_changes_it == db_changes.end()) {
            auto p = m_read_conn.read_by_id_with_cols(table_name, rowid);
            current_change.old_row = std::move(p.second);
            TableChanges table_changes;
            table_changes.column_names = std::move(p.first);
            table_changes_it = db_changes.emplace(table_name, std::move(table_changes)).first;
        } else {
            current_change.old_row = m_read_conn.read_by_id(table_name, rowid);
        }
        current_change.new_row = m_write_conn.read_by_id(table_name, rowid);
        if (current_change.old_row) {
            std::cout << "Change: " << current_change.old_row.value()[0] << std::endl;
        }
        table_changes_it->second.row_changes.emplace_back( std::move(current_change) );
    }
    write_guard.commit();
    read_guard.commit();
    m_listener->on_change(std::move(db_changes));
}

} } // end namespace mx3::sqlite
