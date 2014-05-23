#pragma once
#include "stl.hpp"
#include <mx3/mx3.hpp>
#include <dispatch/dispatch.h>

namespace mx3 { namespace objc {

class ObjcEventLoop final : public mx3::EventLoop {
  public:
    ObjcEventLoop(dispatch_queue_t q) : m_queue(q) {}
    virtual ~ObjcEventLoop() {}
    virtual void post(const function<void()>& run_fn) override;
  private:
    dispatch_queue_t m_queue;
};

} } // end namesapce mx3::objc
