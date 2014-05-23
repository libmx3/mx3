#include "objc_event_loop.hpp"
using mx3::objc::ObjcEventLoop;

void
ObjcEventLoop::post(const std::function<void()>& run_fn) {
    // todo(skabbes) make sure that the block copies the function
    std::function<void()> run_fn_copy(run_fn);
    dispatch_async(m_queue, ^{
        run_fn_copy();
    });
}
