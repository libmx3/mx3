#pragma once
#include "stl.hpp"
#include <thread>
#include <deque>

namespace mx3 {

// an event loop system which is delegated to the host OS
class EventLoop {
  public:
    EventLoop();
    virtual ~EventLoop();
    virtual void post(const function<void()>& run_fn) = 0;
};

// an implementation of EventLoop implemented in pure c++
class NativeEventLoop : public mx3::EventLoop {
  public:
    NativeEventLoop();
    // todo(skabbes) make sure to cleanly shut down the event loop on destroy
    virtual ~NativeEventLoop() {}
    virtual void post(const function<void()>& run_fn) override;

  private:
    // the actual run loop runs here
    void _run_loop();

    // todo(skabbes) make the ability to stop this event loop
    // whether we should stop this loop
    // std::atomic_bool m_stop;

    std::mutex m_mutex;
    std::condition_variable m_cv;
    // todo(skabbes) maybe make this a priority queue? and give a post_priority method?
    std::deque<function<void()>> m_queue;
    std::thread m_thread;
};

}
