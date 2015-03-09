#pragma once
#include "../interface/user_list_vm.hpp"
#include "../interface/user_list_vm_cell.hpp"
#include "../interface/user_list_vm_handle.hpp"
#include "../interface/user_list_vm_observer.hpp"
#include "../http.hpp"
#include "../event_loop.hpp"
#include "stl.hpp"
#include <atomic>
#include "../sqlite/sqlite.hpp"
#include "../sqlite_query/query_monitor.hpp"

namespace mx3 {

class UserListVmHandle;

class UserListVm final : public mx3_gen::UserListVm {
  public:
    UserListVm(const vector<sqlite::Row>& rows, const std::weak_ptr<UserListVmHandle> & handle);
    virtual int32_t count() override;
    virtual optional<mx3_gen::UserListVmCell> get(int32_t index) override;
    virtual void delete_row(int32_t index) override;
  private:
    const vector<sqlite::Row> m_rows;
    const std::weak_ptr<UserListVmHandle> m_handle;
};

class UserListVmHandle final : public mx3_gen::UserListVmHandle,
                               public std::enable_shared_from_this<UserListVmHandle> {
  public:
    UserListVmHandle(
        shared_ptr<sqlite::Db> db,
        const mx3::Http& http,
        mx3::EventLoopRef ui_thread
    );
    virtual void start(const shared_ptr<mx3_gen::UserListVmObserver>& observer) override;
    virtual void stop() override;

    void delete_login(const string& github_id);
  private:
    void _notify_new_data();

    const shared_ptr<sqlite::Db> m_db;
    const shared_ptr<sqlite::QueryMonitor> m_monitor;
    const shared_ptr<sqlite::Stmt> m_list_stmt;
    optional<vector<sqlite::Row>> m_prev_rows;
    mx3::Http m_http;
    shared_ptr<mx3_gen::UserListVmObserver> m_observer;
    mx3::EventLoopRef m_ui_thread;
};

} // namespace mx3
