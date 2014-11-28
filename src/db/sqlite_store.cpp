#include "sqlite_store.hpp"

using mx3::SqliteStore;
using json11::Json;

SqliteStore::SqliteStore(const string& db_path) : m_db { mx3::sqlite::Db::open(db_path) } {
    _setup_db(m_db);
}

Json
SqliteStore::get(const string& key) {
    auto stmt = m_db->prepare("SELECT `value` FROM `kv` WHERE `key` = ?1;");
    stmt->bind(1, key);
    auto query = stmt->exec_query();

    if (query.has_next()) {
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

    bool did_update = false;
    {
    auto stmt = m_db->prepare("UPDATE `kv` SET `value` = ?2 WHERE `key` = ?1;");
    stmt->bind(1, key);
    stmt->bind(2, serialized);
    did_update = stmt->exec() > 0;
    }

    if (!did_update) {
        auto stmt = m_db->prepare("INSERT INTO `kv` (`key`, `value`) VALUES (?1, ?2);");
        stmt->bind(1, key);
        stmt->bind(2, serialized);
        stmt->exec();
    }
}

void
SqliteStore::_setup_db(shared_ptr<mx3::sqlite::Db> db) {
    vector<string> setup_commands  {
        "CREATE TABLE IF NOT EXISTS `kv` (`key` TEXT, `value` TEXT);"
    };

    for (const auto& cmd : setup_commands) {
        db->exec(cmd);
    }
}

