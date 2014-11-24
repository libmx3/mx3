#pragma once
#include "stl.hpp"
#include "json_store.hpp"
#include "../sqlite/db.hpp"

namespace mx3 {

class SqliteStore final : public mx3::JsonStore {
  public:
    SqliteStore(const string& db_path);
    virtual json11::Json get(const string& key) override;
    virtual void set(const string& key, const json11::Json& value) override;
  private:
    static void _setup_db(sqlite::Db& db);
    sqlite::Db m_db;
};

}
