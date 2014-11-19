#pragma once
#include "../interface/user_list_vm.hpp"
#include "../interface/user_list_vm_cell.hpp"
#include "../interface/user_list_vm_handle.hpp"
#include "../interface/user_list_vm_observer.hpp"
#include "stl.hpp"
#include <atomic>

namespace mx3 {

class UserListVm final : public mx3_gen::UserListVm {
  public:
    virtual int32_t count() override;
    virtual optional<mx3_gen::UserListVmCell> get(const int32_t & index) override;
};

class UserListVmHandle final : public mx3_gen::UserListVmHandle {
  public:
    UserListVmHandle(function<void(function<void()>)> ui_thread_post_fn);
    virtual void start(const shared_ptr<mx3_gen::UserListVmObserver>& observer) override;
    virtual void stop() override;
  private:
    std::atomic_bool m_stop;
    shared_ptr<mx3_gen::UserListVmObserver> m_observer;
    function<void(function<void()>)> m_post_ui;
};

} // namespace mx3
