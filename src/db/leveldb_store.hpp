#pragma once
#include <leveldb/db.h>
#include "json_store.hpp"
#include "stl.hpp"

namespace mx3 {

class LeveldbStore final : public mx3::JsonStore {
  public:
    LeveldbStore(const string& db_path);
    virtual json11::Json get(const string& key) override;
    virtual void set(const string& key, const json11::Json& value) override;
  private:
    static void _throw_if_error(const leveldb::Status& status);
    std::unique_ptr<leveldb::DB> m_db;
};

}
