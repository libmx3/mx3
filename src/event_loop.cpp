#include "event_loop.hpp"

using mx3::NativeEventLoop;

NativeEventLoop::NativeEventLoop() : m_stop(false), m_thread(&NativeEventLoop::_run_loop, this) {}

NativeEventLoop::~NativeEventLoop() {
    m_stop = true;
    m_cv.notify_one();
    m_thread.join();
}

void
NativeEventLoop::post(const function<void()>& run_fn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.emplace(run_fn);
    m_cv.notify_one();
}

void
NativeEventLoop::_run_loop() {
    while (true) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [this] {
            return m_stop == true || !m_queue.empty();
        });
        if (m_stop == true) {
            return;
        }

        // copy the function off, so we can run it without holding the lock
        auto fn = m_queue.front();
        m_queue.pop();
        lk.unlock();
        fn();
    }
}


