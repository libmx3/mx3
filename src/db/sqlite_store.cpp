#include "sqlite_store.hpp"

using json11::Json;

namespace mx3 {

namespace {
    shared_ptr<sqlite::Db> s_setup_db(const string& db_path) {
        const auto db = sqlite::Db::open(db_path);
        sqlite::TransactionStmts stmts {db};
        sqlite::WriteTransaction guard {stmts};
        vector<string> setup_commands  {
            "CREATE TABLE IF NOT EXISTS `kv` (`key` TEXT, `value` TEXT);"
        };

        for (const auto& cmd : setup_commands) {
            db->exec(cmd);
        }
        guard.commit();
        return db;
    }
} // end anon namespace

SqliteStore::SqliteStore(const string& db_path)
    : m_db { s_setup_db(db_path) }
    , m_stmts { m_db }
    , m_insert { m_db->prepare("INSERT INTO `kv` (`key`, `value`) VALUES (?1, ?2);") }
    , m_update { m_db->prepare("UPDATE `kv` SET `value` = ?2 WHERE `key` = ?1;") }
    , m_select { m_db->prepare("SELECT `value` FROM `kv` WHERE `key` = ?1;") } {}

Json
SqliteStore::get(const string& key) {
    auto& stmt = m_select;
    stmt->reset();
    stmt->bind(1, key);
    auto query = stmt->exec_query();

    if (query.is_valid()) {
        string value = query.string_value(0);
        string error;
        return Json::parse(value, error);
    } else {
        return nullptr;
    }
}

void
SqliteStore::set(const string& key, const Json& value) {
    auto serialized = value.dump();
    sqlite::WriteTransaction guard {m_stmts};

    bool did_update = false;
    {
    auto& stmt = m_update;
    stmt->reset();
    stmt->bind(1, key);
    stmt->bind(2, serialized);
    did_update = stmt->exec() > 0;
    }

    if (!did_update) {
        auto& stmt = m_insert;
        stmt->reset();
        stmt->bind(1, key);
        stmt->bind(2, serialized);
        stmt->exec();
    }
    guard.commit();
}

} // end namespace mx3
