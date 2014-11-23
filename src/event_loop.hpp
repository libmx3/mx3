#pragma once
#include "stl.hpp"
#include <thread>
#include <queue>
#include <atomic>

namespace mx3 {

// an event loop system which is delegated to the host OS
class EventLoop {
  public:
    virtual ~EventLoop() {}
    virtual void post(const function<void()>& run_fn) = 0;
};

// an implementation of EventLoop implemented in pure c++
class NativeEventLoop : public mx3::EventLoop {
  public:
    NativeEventLoop();
    // todo(skabbes) make sure to cleanly shut down the event loop on destroy
    virtual ~NativeEventLoop();
    virtual void post(const function<void()>& run_fn) override;

  private:
    // the actual run loop runs here
    void _run_loop();

    std::atomic<bool> m_stop;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<function<void()>> m_queue;
    std::thread m_thread;
};

}
