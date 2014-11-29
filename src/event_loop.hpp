#pragma once
#include "stl.hpp"
#include <thread>
#include <queue>
#include <atomic>

#include "interface/event_loop.hpp"
#include "interface/async_task.hpp"

namespace mx3 {

// a helper class to turn the `AsyncTask` api into a std::function based one
class FnTask final : public mx3_gen::AsyncTask {
  public:
    FnTask(function<void()> run_me);
    virtual void execute() override;
  private:
    function<void()> m_fn;
};

// an interface wrapper on top of the platform event loops
class EventLoopRef final {
  public:
    EventLoopRef(shared_ptr<mx3_gen::EventLoop> loop);
    void post(function<void()> run_fn);
  private:
    shared_ptr<mx3_gen::EventLoop> m_loop;
};

// an implementation of mx3_gen::EventLoop implemented in pure c++
class EventLoopCpp final : public mx3_gen::EventLoop {
  public:
    EventLoopCpp();
    ~EventLoopCpp();
    virtual void post(const shared_ptr<mx3_gen::AsyncTask>& task) override;

  private:
    // the actual run loop runs here
    void _run_loop();
    std::atomic<bool> m_stop;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<shared_ptr<mx3_gen::AsyncTask>> m_queue;
    std::thread m_thread;
};

}
