#pragma once
#include "stl.hpp"
#include <thread>
#include <queue>
#include <atomic>

#include "interface/event_loop.hpp"
#include "interface/async_task.hpp"
#include "interface/thread_launcher.hpp"

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
    EventLoopCpp(const shared_ptr<mx3_gen::ThreadLauncher> & launcher);
    ~EventLoopCpp();
    virtual void post(const shared_ptr<mx3_gen::AsyncTask>& task) override;

  private:
    // the actual run loop runs here
    void _run_loop();
    std::mutex m_task_mutex;
    std::condition_variable m_task_cv;
    bool m_stop;
    std::queue<shared_ptr<mx3_gen::AsyncTask>> m_queue;

    bool m_done;
    std::mutex m_done_mutex;
    std::condition_variable m_done_cv;
};

}
