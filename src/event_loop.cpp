#include "event_loop.hpp"

using mx3::EventLoopCpp;
using mx3::EventLoopRef;

EventLoopRef::Task::Task(function<void()> run_me) : m_fn {std::move(run_me)} {}

void
EventLoopRef::Task::execute() {
    m_fn();
}

EventLoopRef::EventLoopRef(shared_ptr<mx3_gen::EventLoop> loop) : m_loop {std::move(loop)} {}

void
EventLoopRef::post(function<void()> run_fn) {
    m_loop->post( make_shared<EventLoopRef::Task>(std::move(run_fn)) );
}

EventLoopCpp::EventLoopCpp() : m_stop(false), m_thread(&EventLoopCpp::_run_loop, this) {}

EventLoopCpp::~EventLoopCpp() {
    m_stop = true;
    m_cv.notify_one();
    m_thread.join();
}

void
EventLoopCpp::post(const shared_ptr<mx3_gen::AsyncTask>& task) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.emplace(task);
    m_cv.notify_one();
}

void
EventLoopCpp::_run_loop() {
    while (true) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [this] {
            return m_stop == true || !m_queue.empty();
        });
        if (m_stop == true) {
            return;
        }

        // copy the function off, so we can run it without holding the lock
        auto task = std::move(m_queue.front());
        m_queue.pop();
        lk.unlock();
        task->execute();
    }
}


