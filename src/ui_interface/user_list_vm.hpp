#pragma once
#include "../interface/user_list_vm.hpp"
#include "../interface/user_list_vm_cell.hpp"
#include "../interface/user_list_vm_handle.hpp"
#include "../interface/user_list_vm_observer.hpp"
#include "stl.hpp"
#include <atomic>
#include "../http.hpp"
#include "../sqlite/db.hpp"

namespace mx3 {

class UserListVm final : public mx3_gen::UserListVm {
  public:
    UserListVm(shared_ptr<sqlite::Db> db_conn);
    virtual int32_t count() override;
    virtual optional<mx3_gen::UserListVmCell> get(const int32_t & index) override;
  private:
    optional<int32_t> m_count;
    int32_t m_cursor_pos;
    vector<optional<mx3_gen::UserListVmCell>> m_row_cache;
    shared_ptr<sqlite::Db> m_db;
    sqlite::Stmt m_count_stmt;
    sqlite::Stmt m_list_stmt;
    sqlite::Cursor m_query;
};

class UserListVmHandle final : public mx3_gen::UserListVmHandle {
  public:
    UserListVmHandle(shared_ptr<sqlite::Db> db, shared_ptr<mx3::Http> http, function<void(function<void()>)> ui_thread_post_fn);
    virtual void start(const shared_ptr<mx3_gen::UserListVmObserver>& observer) override;
    virtual void stop() override;
  private:
    shared_ptr<sqlite::Db> m_db;
    shared_ptr<mx3::Http> m_http;
    std::atomic_bool m_stop;
    shared_ptr<mx3_gen::UserListVmObserver> m_observer;
    function<void(function<void()>)> m_post_ui;
};

} // namespace mx3
