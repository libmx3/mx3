#include "event_loop.hpp"

namespace mx3 {

FnTask::FnTask(function<void()> run_me) : m_fn {std::move(run_me)} {}

void
FnTask::execute() {
    m_fn();
}

EventLoopRef::EventLoopRef(shared_ptr<mx3_gen::EventLoop> loop) : m_loop {std::move(loop)} {}

void
EventLoopRef::post(const SingleThreadTaskRunner::Task & task) {
    m_loop->post(make_shared<FnTask>(task));
}

EventLoopCpp::EventLoopCpp(const shared_ptr<mx3_gen::ThreadLauncher> & launcher) : m_stop(false), m_done(false) {
    auto task = make_shared<FnTask>([this](){
        _run_loop();
    });
    launcher->start_thread(string{"background_event_loop"}, task);
}

EventLoopCpp::~EventLoopCpp() {
    {
    std::lock_guard<std::mutex> task_lk(m_task_mutex);
    m_stop = true;
    }
    m_task_cv.notify_one();

    std::unique_lock<std::mutex> done_lk(m_done_mutex);
    m_done_cv.wait(done_lk, [this] () { return m_done; });
}

void EventLoopCpp::post(const SingleThreadTaskRunner::Task & task) {
    {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    m_queue.emplace(task);
    }
    m_task_cv.notify_one();
}

void
EventLoopCpp::_run_loop() {
    while (true) {
        std::unique_lock<std::mutex> lk(m_task_mutex);
        m_task_cv.wait(lk, [this] {
            return m_stop == true || !m_queue.empty();
        });
        if (m_stop == true) {
            break;
        }

        // copy the function off, so we can run it without holding the lock
        auto task = std::move(m_queue.front());
        m_queue.pop();
        lk.unlock();
        task();
    }

    {
    std::lock_guard<std::mutex> lock(m_done_mutex);
    m_done = true;
    }
    m_done_cv.notify_one();
}

}
