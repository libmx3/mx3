#include "sqlite_store.hpp"

using mx3::SqliteStore;
using json11::Json;

SqliteStore::SqliteStore(const string& db_path) : m_db()
{
    m_db.open(db_path.c_str());
    _setup_db(m_db);
}

Json
SqliteStore::get(const string& key) {
    auto stmt = m_db.compileStatement("SELECT `value` FROM `kv` WHERE `key` = ?1;");
    stmt.bind(1, key.c_str());
    auto query = stmt.execQuery();

    if (!query.eof()) {
        string value = query.getStringField(0);
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
    auto stmt = m_db.compileStatement("UPDATE `kv` SET `value` = ?2 WHERE `key` = ?1;");
    stmt.bind(1, key.c_str());
    stmt.bind(2, serialized.c_str());
    did_update = stmt.execDML() > 0;
    }

    if (!did_update) {
        auto stmt = m_db.compileStatement("INSERT INTO `kv` (`key`, `value`) VALUES (?1, ?2);");
        stmt.bind(1, key.c_str());
        stmt.bind(2, serialized.c_str());
        stmt.execDML();
    }
}

void
SqliteStore::_setup_db(CppSQLite3DB& db) {
    vector<string> setup_commands  {
        "CREATE TABLE IF NOT EXISTS `kv` (`key` TEXT, `value` TEXT);"
    };
    for (const auto& cmd : setup_commands) {
        db.execDML(cmd.c_str());
    }
}

