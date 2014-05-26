#include "semaphore.hpp"
using mx3::semaphore;

semaphore::semaphore() : m_count(0) {}

void
semaphore::notify() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_count++;
    m_cv.notify_one();
}

void
semaphore::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this]() { return m_count > 0; });
    --m_count;
}

