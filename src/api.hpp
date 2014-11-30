#pragma once
#include "stl.hpp"
#include <json11/json11.hpp>
#include "event_loop.hpp"
#include "db/json_store.hpp"
#include "sqlite/sqlite.hpp"

#include "interface/api.hpp"

namespace mx3 {

// the "api" of how the UI is allowed to talk to the c++ code
class Api final : public mx3_gen::Api {
  public:
    Api(
        const string & root_path,
        const shared_ptr<mx3_gen::EventLoop>& main_thread,
        const shared_ptr<mx3_gen::Http> & http_impl
    );

    // whether a user already exists
    virtual bool has_user() override;
    // get the current username, or "" if none exists
    virtual string get_username() override;
    virtual void set_username(const string& name) override;

    virtual shared_ptr<mx3_gen::UserListVmHandle> observer_user_list() override;

  private:
    // set up the database
    void _setup_db();

    shared_ptr<mx3_gen::Http> m_http;
    shared_ptr<sqlite::Db> m_sqlite;
    shared_ptr<sqlite::Db> m_read_db;

    unique_ptr<mx3::JsonStore> m_db;
    mx3::EventLoopRef m_ui_thread;
    mx3::EventLoopRef m_bg_thread;
};

}
