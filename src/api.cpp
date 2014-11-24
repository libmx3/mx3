#include "api.hpp"
#include "stl.hpp"
#include "github/client.hpp"
#include "github/types.hpp"
#include "db/sqlite_store.hpp"
#include "ui_interface/user_list_vm.hpp"
#include <iostream>

using mx3::Api;
using json11::Json;

#include "sql_snapshot.hpp"

namespace {
    const string USERNAME_KEY      = "username";
    const string LAUNCH_NUMBER_KEY = "launch_number";
}

Api::Api(const string& root_path, const shared_ptr<mx3::EventLoop>& main_thread, const shared_ptr<mx3::Http>& http_client) :
    m_sqlite(root_path + "/example.sqlite"),
    m_github_client(http_client),
    // todo this needs to use a fs/path abstraction (not yet built)
    m_db( std_patch::make_unique<mx3::SqliteStore>(root_path + "/kv.sqlite") ),
    m_main_thread(main_thread),
    m_bg_thread( make_shared<mx3::NativeEventLoop>() )
{
    _setup_db();

    auto j_launch_number = m_db->get(LAUNCH_NUMBER_KEY);
    size_t launch = 0;
    if (j_launch_number.is_number()) {
        launch = j_launch_number.number_value() + 1;
    }
    m_db->set("launch_number", static_cast<double>(launch));
    _log_launch(launch);
}

bool
Api::has_user() {
    return !m_db->get(USERNAME_KEY).is_null();
}

string
Api::get_username() {
    return m_db->get(USERNAME_KEY).string_value();
}

void
Api::set_username(const string& username) {
    m_db->set(USERNAME_KEY, username);
}

shared_ptr<mx3_gen::UserListVmHandle>
Api::observer_user_list() {
    std::weak_ptr<mx3::EventLoop> ui_thread_weak = m_main_thread;
    auto ui_post = [ui_thread_weak] (function<void()> run_fn) {
        auto ui_thread = ui_thread_weak.lock();
        if (ui_thread) {
            ui_thread->post(run_fn);
        }
    };
    return make_shared<mx3::UserListVmHandle>(ui_post);
}

unique_ptr<mx3::SqlSnapshot>
Api::get_launches() {
    auto stmt = m_sqlite.prepare("SELECT content FROM Data");
    auto query = stmt.exec_query();
    return std::unique_ptr<mx3::SqlSnapshot>( new mx3::SqlSnapshot(query) );
}

mx3::QueryResultPtr<github::User>
Api::get_github_users() {
    auto result = make_shared<mx3::QueryResult<github::User>>(m_main_thread);
    std::weak_ptr<mx3::QueryResult<github::User>> result_ref = result;
    m_github_client.get_users( [=](vector<github::User> users) {
        auto sp = result_ref.lock();
        if (sp) {
            sp->update_data(users);
        }
    });
    return result;
}

void
Api::_log_launch(size_t num) {
    string log_line = "Launch #" + to_string(num);
    auto stmt = m_sqlite.prepare("INSERT INTO `Data` (content) VALUES (?1)");
    stmt.bind(1, log_line);
    stmt.exec();
}

void
Api::_setup_db() {
    vector<string> setup_commands  {
        "CREATE TABLE IF NOT EXISTS `Data` (content TEXT)"
    };
    for (const auto& cmd : setup_commands) {
        m_sqlite.exec(cmd);
    }
}
