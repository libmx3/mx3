#pragma once
#include "stl.hpp"

namespace mx3 {

// going against naming conventions here, since this class "feels" like an stl class
// therefore follow those conventions
class semaphore final {
  public:
    semaphore();
    void notify();
    void wait();
  private:
    size_t m_count;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

}
