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
    void s_print_row(const optional<mx3::sqlite::Row>& row) {
        if (row) {
            for (const auto& val : *row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "[null]" << std::endl;
        }
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
    return stmt->exec_one();
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
    if (m_state ==  TransactionGuard::State::NONE) {
        m_state =  TransactionGuard::State::COMMIT;
        m_db.m_commit->exec();
    }
}

void TransactionGuard::rollback() {
    if (m_state == TransactionGuard::State::NONE) {
        m_state = State::ROLLBACK;
        m_db.m_rollback->exec();
    }
}

ObservableDb::ObservableDb(const string& path)
    : m_write {path}
    , m_read {path}
    // In order to actually create a read snapshot, you need to issue a BEGIN
    // _and_ an actual select statement.  This select statement will work for any database.
    , m_begin_read_snapshot {m_read.db->prepare("SELECT 1 FROM sqlite_master LIMIT 1") }
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

    // group the changes by rowid
    for (const auto& c : m_changes) {
        const ChangeType type = std::get<0>(c);
        const string& db_name = std::get<1>(c);
        const string& table_name = std::get<2>(c);
        int64_t rowid = std::get<3>(c);

        std::cout << int(type) << " " << db_name << ", " << table_name << ", " << rowid << std::endl;

        const auto old_row = m_read.read_by_id(table_name, read_schema_version, rowid);
        const auto new_row = m_write.read_by_id(table_name, write_schema_version, rowid);

        s_print_row(old_row);
        s_print_row(new_row);
    }

    m_changes.clear();
    read_transaction.commit();
}
