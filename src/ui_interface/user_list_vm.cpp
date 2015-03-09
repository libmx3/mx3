#include "user_list_vm.hpp"
#include "../github/client.hpp"
#include "../sqlite/sqlite.hpp"
#include "../sqlite_query/query_diff.hpp"

#include <iostream>
#include <thread>
using mx3_gen::UserListVmObserver;
using mx3_gen::UserListVmCell;

namespace chrono {
    using namespace std::chrono;
}

namespace {
    const string s_list_stmt  { "SELECT `login`, `id` FROM `github_users` ORDER BY id;" };
}

namespace mx3 {

UserListVm::UserListVm(const vector<sqlite::Row>& rows, const std::weak_ptr<UserListVmHandle> & handle)
    : m_rows(rows)
    , m_handle(handle)
{}

int32_t
UserListVm::count() {
    return static_cast<int32_t>(m_rows.size());
}

void UserListVm::delete_row(int32_t index) {
    const string github_login = this->get(index)->name;
    const auto handle = m_handle.lock();
    if (handle) {
        handle->delete_login(github_login);
    }
}

optional<UserListVmCell>
UserListVm::get(int32_t index) {
    if (index < this->count()) {
        return UserListVmCell {index, m_rows[index][0].string_value()};
    }
    return nullopt;
}

UserListVmHandle::UserListVmHandle(
    shared_ptr<mx3::sqlite::Db> db,
    const mx3::Http& http,
    const shared_ptr<SingleThreadTaskRunner> & ui_thread,
    const shared_ptr<SingleThreadTaskRunner> & bg_thread
)
    : m_db(db)
    , m_monitor(sqlite::QueryMonitor::create_shared(m_db))
    , m_list_stmt(m_db->prepare(s_list_stmt))
    , m_http(http)
    , m_observer(nullptr)
    , m_ui_thread(ui_thread)
    , m_bg_thread(bg_thread)
{
    m_monitor->listen_to_changes([this] () {
        this->_notify_new_data();
    });
}

void
UserListVmHandle::start(const shared_ptr<UserListVmObserver>& observer) {
    auto db = m_db;
    auto ui_thread = m_ui_thread;
    m_observer = observer;

    github::get_users(m_http, nullopt, [db, ui_thread, observer] (vector<github::User> users) mutable {
        auto update_stmt = db->prepare("UPDATE `github_users` SET `login` = ?2 WHERE `id` = ?1;");
        auto insert_stmt = db->prepare("INSERT INTO `github_users` (`id`, `login`) VALUES (?1, ?2);");
        sqlite::TransactionStmts transaction_stmts {db};
        sqlite::WriteTransaction guard {transaction_stmts};

        for (const auto& user : users) {
            update_stmt->reset();
            update_stmt->bind(1, user.id);
            update_stmt->bind(2, user.login);
            if ( update_stmt->exec() == 0 ) {
                insert_stmt->reset();
                insert_stmt->bind(1, user.id);
                insert_stmt->bind(2, user.login);
                insert_stmt->exec();
            }
        }
        guard.commit();
    });
}

void
UserListVmHandle::delete_login(const string& github_login) {
    const auto self = shared_from_this();
    m_bg_thread->post([self, github_login] () {
        const auto delete_stmt = self->m_db->prepare("DELETE FROM `github_users` WHERE `login` = ?1");
        delete_stmt->bind(1, github_login);
        // This exec will automatically trigger the commit hook.
        delete_stmt->exec();
    });
}

void
UserListVmHandle::stop() {
    // this isn't implemented yet :(
}


void
UserListVmHandle::_notify_new_data() {
    // todo(kabbes) this isn't thread safe

    optional<vector<mx3_gen::ListChange>> diff;

    decltype(m_prev_rows) prev_rows;
    std::swap(m_prev_rows, prev_rows);
    auto new_rows = m_list_stmt->exec_query().all_rows();

    if (prev_rows) {
        constexpr const size_t ID_POS = 1;
        const auto is_same_entity = [] (const sqlite::Row& a, const sqlite::Row& b) {
            return a[ID_POS] == b[ID_POS];
        };
        const auto less_than = [] (const sqlite::Row& a, const sqlite::Row& b) {
            return a[ID_POS] < b[ID_POS];
        };
        const auto sql_diff = sqlite::calculate_diff(*prev_rows, new_rows, is_same_entity, less_than);
        vector<mx3_gen::ListChange> final_diff;
        final_diff.reserve(sql_diff.size());
        for (const auto& c : sql_diff) {
            final_diff.push_back({c.from_index, c.to_index});
        }
        diff = std::move(final_diff);
    }

    m_prev_rows = std::move(new_rows);
    const std::weak_ptr<UserListVmHandle> weak_self = shared_from_this();
    m_ui_thread->post([diff = std::move(diff), weak_self, observer = m_observer, new_rows = *m_prev_rows] () {
        // todo(kabbes) make sure to check if this has been stopped
        observer->on_update(diff, make_shared<UserListVm>(new_rows, weak_self));
    });
}
}
