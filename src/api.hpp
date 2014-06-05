#pragma once
#include "stl.hpp"
#include <leveldb/db.h>
#include <json11/json11.hpp>
#include <CppSQLite/CppSQLite3.h>
#include "sql_snapshot.hpp"
#include "event_loop.hpp"
#include "http.hpp"
#include "github/client.hpp"
#include "github/types.hpp"
#include "query_result.hpp"

namespace mx3 {

// the "api" of how the UI is allowed to talk to the c++ code
class Api final {
  public:
    Api(
        const std::string& root_dir,
        const shared_ptr<mx3::EventLoop>& main_thread,
        const shared_ptr<mx3::Http>& http_client
    );
    // whether a user already exists
    bool has_user() const;
    // get the current username, or "" if none exists
    std::string get_username() const;
    void set_username(const std::string& name);

    // a _very_ simplistic query api
    std::unique_ptr<mx3::SqlSnapshot> get_launches();

    mx3::QueryResultPtr<github::User> get_github_users();

  private:
    static std::unique_ptr<leveldb::DB> _open_database(const std::string& db_path);
    static void _throw_if_error(const leveldb::Status& status);

    // set up the database
    void _setup_db();

    // log to sqlite that the app has been launched
    void _log_launch(size_t num);

    // persistent getters and setters for NSUserDefatuls style stuff
    json11::Json _get_value(const std::string& key) const;
    void _set_value(const std::string& key, const json11::Json& value);

    CppSQLite3DB m_sqlite;
    github::Client m_github_client;
    std::unique_ptr<leveldb::DB> m_ldb;
    std::shared_ptr<mx3::EventLoop> m_main_thread;
    std::shared_ptr<mx3::EventLoop> m_bg_thread;
};

}
