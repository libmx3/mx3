#include "observable_db.hpp"
#include <sqlite3/sqlite3.h>
#include <iostream>

using mx3::sqlite::Db;
using mx3::sqlite::detail::TransactionDb;
using mx3::sqlite::detail::TransactionGuard;
using mx3::sqlite::ColumnInfo;
using mx3::sqlite::ObservableDb;
using mx3::sqlite::Stmt;
using mx3::sqlite::ChangeType;
using mx3::sqlite::OpenFlag;

namespace {
    string s_select_by_rowid(const string& table_name, const vector<ColumnInfo>& columns) {
        string sql = "SELECT ";
        for (const auto& c : columns) {
            if (&c != &columns[0]) {
                sql += ", \"" + c.name + "\"";
            } else {
                sql += "\"" + c.name + "\"";
            }
        }
        sql += " FROM \"" + table_name + "\" WHERE rowid = ?1";
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
}

TransactionDb::TransactionDb(const string& path)
    : db { s_open_wal(path) }
    , m_begin { db->prepare("BEGIN TRANSACTION") }
    , m_commit { db->prepare("COMMIT TRANSACTION") }
    , m_rollback { db->prepare("ROLLBACK TRANSACTION") }
    , m_schema_version { db->schema_version() }
{}

optional<mx3::sqlite::Row>
TransactionDb::read_by_id(const string& table_name, int32_t schema_ver, int64_t rowid) {
    Cursor c = _read_cursor(table_name, schema_ver, rowid);
    return c.is_valid() ? c.values() : optional<mx3::sqlite::Row>{};
}

std::pair<vector<string>, optional<mx3::sqlite::Row>>
TransactionDb::read_by_id_with_cols(const string& table_name, int32_t schema_ver, int64_t rowid) {
    Cursor c = _read_cursor(table_name, schema_ver, rowid);
    return {c.column_names(), c.is_valid() ? c.values() : optional<mx3::sqlite::Row>{}};
}

mx3::sqlite::Cursor TransactionDb::_read_cursor(const string& table_name, int32_t schema_ver, int64_t rowid) {
    if (schema_ver != m_schema_version) {
        throw std::runtime_error {"schema must be stable"};
    }

    auto it = m_read_stmts.find(table_name);
    if (it == m_read_stmts.end()) {
        const auto table_info = this->db->table_info(table_name);
        if (!table_info) {
            throw std::runtime_error {"table not found"};
        }
        const auto stmt = this->db->prepare( s_select_by_rowid(table_name, table_info->columns) );
        it = m_read_stmts.emplace(table_name, stmt).first;
    }

    const auto& stmt = it->second;
    stmt->reset();
    stmt->bind(1, rowid);
    return stmt->exec_query();
}

TransactionGuard::TransactionGuard(TransactionDb& db)
    : m_db{db}
    , m_state{State::NONE}
{
    m_db.m_begin->exec();
}

TransactionGuard::~TransactionGuard() {
    this->rollback();
}

void TransactionGuard::commit() {
    if (m_state == TransactionGuard::State::NONE) {
        m_state = TransactionGuard::State::COMMIT;
        m_db.m_commit->exec();
    }
}

void TransactionGuard::rollback() {
    if (m_state == TransactionGuard::State::NONE) {
        m_state = State::ROLLBACK;
        m_db.m_rollback->exec();
    }
}

ObservableDb::ObservableDb(const string& path, const function<void(DbChanges)>& fn)
    : ObservableDb(path, make_shared<FnDbListener>(fn)) {}

ObservableDb::ObservableDb(const string& path, const shared_ptr<DbListener>& listener)
    : m_write {path}
    , m_read {path}
    // In order to actually create a read snapshot, you need to issue a BEGIN
    // _and_ an actual select statement.  This select statement will work for any database.
    , m_begin_read_snapshot {m_read.db->prepare("SELECT 1 FROM sqlite_master LIMIT 1")}
    , m_listener {listener}
{
    m_write.db->update_hook([this] (ChangeType type, string db_name, string table_name, int64_t rowid) {
        m_changes.emplace_back(type, std::move(db_name), std::move(table_name), rowid);
    });
}

void
ObservableDb::transaction(function<void(const shared_ptr<Db>&)> t_fn) {
    detail::TransactionGuard write_transaction {m_write};
    detail::TransactionGuard read_transaction {m_read};
    // Although this line appears to do nothing, sqlite defers opening a read transaction until
    // a select statement has been issued.
    m_begin_read_snapshot->exec_scalar();

    t_fn(m_write.db);
    write_transaction.commit();

    const auto read_schema_version = m_read.db->schema_version();
    const auto write_schema_version = m_write.db->schema_version();

    DbChanges db_changes;

    decltype(m_changes) changes_copy;
    std::swap(m_changes, changes_copy);

    for (const auto& c : changes_copy) {
        const ChangeType type    = std::get<0>(c);
        const string& db_name    = std::get<1>(c);
        const string& table_name = std::get<2>(c);
        int64_t rowid            = std::get<3>(c);
        (void)db_name;
        (void)type;

        RowChange current_change;
        auto table_changes_it = db_changes.find(table_name);
        if (table_changes_it == db_changes.end()) {
            auto p = m_read.read_by_id_with_cols(table_name, read_schema_version, rowid);
            current_change.old_row = std::move(p.second);
            TableChanges table_changes;
            table_changes.column_names = std::move(p.first);
            table_changes_it = db_changes.emplace(table_name, std::move(table_changes)).first;
        } else {
            current_change.old_row = m_read.read_by_id(table_name, read_schema_version, rowid);
        }
        current_change.new_row = m_write.read_by_id(table_name, write_schema_version, rowid);
        table_changes_it->second.row_changes.emplace_back( std::move(current_change) );
    }
    read_transaction.commit();
    m_listener->on_change(std::move(db_changes));
}
