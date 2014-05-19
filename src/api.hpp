#pragma once
#include <string>
#include <memory>
#include <leveldb/db.h>
#include <json11/json11.hpp>

namespace mx3 {

class Api final {
  public:
    Api(const std::string& db_path);
    // whether a user already exists
    bool has_user() const;
    // get the current username, or "" if none exists
    std::string get_username() const;
    void set_username(const std::string& name);

  private:
    static std::unique_ptr<leveldb::DB> _open_database(const std::string& db_path);
    static void _throw_if_error(const leveldb::Status& status);

    // persistent getters and setters for NSUserDefatuls style stuff
    json11::Json _get_value(const std::string& key) const;
    void _set_value(const std::string& key, const json11::Json& value);

    std::unique_ptr<leveldb::DB> m_db;
};

}
