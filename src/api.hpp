#pragma once
#include "stl.hpp"
#include <json11/json11.hpp>
#include "sql_snapshot.hpp"
#include "event_loop.hpp"
#include "http.hpp"
#include "github/client.hpp"
#include "github/types.hpp"
#include "query_result.hpp"
#include "db/json_store.hpp"
#include "interface/api.hpp"

namespace mx3 {

// the "api" of how the UI is allowed to talk to the c++ code
class Api final : public mx3_gen::Api {
  public:
    Api(
        const std::string& root_dir,
        const shared_ptr<mx3::EventLoop>& main_thread,
        const shared_ptr<mx3::Http>& http_client
    );
    // whether a user already exists
    virtual bool has_user() override;
    // get the current username, or "" if none exists
    virtual std::string get_username() override;
    virtual void set_username(const std::string& name) override;

    virtual shared_ptr<mx3_gen::UserListVmHandle> observer_user_list() override;

    // a _very_ simplistic query api
    std::unique_ptr<mx3::SqlSnapshot> get_launches();

    mx3::QueryResultPtr<github::User> get_github_users();

  private:
    // set up the database
    void _setup_db();

    // log to sqlite that the app has been launched
    void _log_launch(size_t num);

    sqlite::Db m_sqlite;
    github::Client m_github_client;
    std::unique_ptr<mx3::JsonStore> m_db;
    std::shared_ptr<mx3::EventLoop> m_main_thread;
    std::shared_ptr<mx3::EventLoop> m_bg_thread;
};

}
