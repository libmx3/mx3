#pragma once
#include <CppSQLite/CppSQLite3.h>
#include "json_store.hpp"
#include "stl.hpp"

namespace mx3 {

class SqliteStore final : public mx3::JsonStore {
  public:
    SqliteStore(const string& db_path);
    virtual json11::Json get(const string& key) override;
    virtual void set(const string& key, const json11::Json& value) override;
  private:
    static void _setup_db(CppSQLite3DB& db);
    CppSQLite3DB m_db;
};

}
