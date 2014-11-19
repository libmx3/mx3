#include "user_list_vm.hpp"
using mx3::UserListVm;
using mx3::UserListVmHandle;
using mx3_gen::UserListVmObserver;
using mx3_gen::UserListVmCell;

int32_t
UserListVm::count() {
    return 5;
}

optional<UserListVmCell>
UserListVm::get(const int32_t & index) {
    switch (index) {
        case 0:
            return UserListVmCell {index, "zero"};
        case 1:
            return UserListVmCell {index, "one"};
        case 2:
            return UserListVmCell {index, "two"};
        case 3:
            return UserListVmCell {index, "three"};
        case 4:
            return UserListVmCell {index, "four"};
        default:
            return nullopt;
    }
}

UserListVmHandle::UserListVmHandle(function<void(function<void()>)> ui_thread_post_fn)
    : m_stop(false)
    , m_observer(nullptr)
    , m_post_ui(ui_thread_post_fn) {}

void
UserListVmHandle::start(const shared_ptr<UserListVmObserver>& observer) {
    m_post_ui( [observer] () {
        observer->on_update( make_shared<UserListVm>() );
    });
}

void
UserListVmHandle::stop() {

}
