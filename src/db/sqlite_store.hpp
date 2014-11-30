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
    shared_ptr<sqlite::Db> m_db;
    shared_ptr<sqlite::Stmt> m_select;
    shared_ptr<sqlite::Stmt> m_update;
    shared_ptr<sqlite::Stmt> m_insert;
};

}
