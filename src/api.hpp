#pragma once
#include "stl.hpp"
#include <json11/json11.hpp>
#include <CppSQLite/CppSQLite3.h>
#include "sql_snapshot.hpp"
#include "event_loop.hpp"
#include "http.hpp"
#include "github/client.hpp"
#include "github/types.hpp"
#include "query_result.hpp"
#include "db/json_store.hpp"

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
    // set up the database
    void _setup_db();

    // log to sqlite that the app has been launched
    void _log_launch(size_t num);

    CppSQLite3DB m_sqlite;
    github::Client m_github_client;
    std::unique_ptr<mx3::JsonStore> m_db;
    std::shared_ptr<mx3::EventLoop> m_main_thread;
    std::shared_ptr<mx3::EventLoop> m_bg_thread;
};

}
