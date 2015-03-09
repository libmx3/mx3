#pragma once
#include "stl.hpp"

namespace mx3 {

// An interface to hide the details of how tasks are executed.
class SingleThreadTaskRunner {
public:
    using Task = function<void()>;
    virtual void post(const Task & task) = 0;
};

}
