#include "user_list_vm.hpp"
#include "../github/client.hpp"
#include <iostream>
#include <thread>
using mx3::UserListVm;
using mx3::UserListVmHandle;
using mx3_gen::UserListVmObserver;
using mx3_gen::UserListVmCell;

namespace chrono {
    using namespace std::chrono;
}

namespace {
    const string s_count_stmt { "SELECT COUNT(1) FROM `github_users`;" };
    const string s_list_stmt  { "SELECT `login` FROM `github_users`;" };
}

UserListVm::UserListVm(shared_ptr<mx3::sqlite::Db> db)
    : m_count{nullopt}
    , m_cursor_pos {0}
    , m_db {db}
    , m_count_stmt { m_db->prepare(s_count_stmt) }
    , m_list_stmt  { m_db->prepare(s_list_stmt) }
    , m_query { m_list_stmt.exec_query() }
{}

int32_t
UserListVm::count() {
    if (m_count) {
        return *m_count;
    }
    m_count = m_count_stmt.exec_query().int_value(0);
    std::cout << "Count: " << *m_count << std::endl;
    return *m_count;
}

optional<UserListVmCell>
UserListVm::get(const int32_t & index) {
    auto start = chrono::steady_clock::now();

    if (index < m_row_cache.size()) {
        return m_row_cache[index];
    }

    m_row_cache.resize(index + 1);
    while (m_query.has_next() && m_cursor_pos <= index) {
        m_row_cache[m_cursor_pos] = UserListVmCell {m_cursor_pos, m_query.string_value(0)};
        m_query.next();
        m_cursor_pos++;
    }

    auto end = chrono::steady_clock::now();

    if (m_cursor_pos == index + 1) {
        double millis = chrono::duration_cast<chrono::nanoseconds>( end - start ).count() / 1000000.0;
        std::cout << millis << " milliseconds" << std::endl;
        return *(m_row_cache[index]);
    } else {
        return nullopt;
    }
}

UserListVmHandle::UserListVmHandle(
    shared_ptr<mx3::sqlite::Db> db,
    shared_ptr<mx3::Http> http,
    function<void(function<void()>)> ui_thread_post_fn
)
    : m_db(db)
    , m_http(http)
    , m_stop(false)
    , m_observer(nullptr)
    , m_post_ui(ui_thread_post_fn) {}

void
UserListVmHandle::start(const shared_ptr<UserListVmObserver>& observer) {

    auto db = m_db;
    auto post_ui = m_post_ui;

    github::Client::get_users(m_http, [db, post_ui, observer] (vector<github::User> users) {
        auto stmt = db->prepare("INSERT INTO `github_users` (`login`) VALUES (?1);");
        db->exec("BEGIN TRANSACTION");
        for (const auto& user : users) {
            // todo(kabbes) insert all fields, check for duplicates
            stmt.bind(1, user.login);
            stmt.exec();
        }
        db->exec("COMMIT TRANSACTION");
        post_ui([db, observer] () {
            // todo(kabbes) make sure to check if this has been stopped
            observer->on_update( make_shared<UserListVm>(db) );
        });
    });
}

void
UserListVmHandle::stop() {
    // this isn't implemented yet :(

}
