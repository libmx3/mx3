#include "stl.hpp"
#include <atomic>
#include <gtest/gtest.h>

#include <mx3/mx3.hpp>
using namespace mx3;

TEST(NativeEventLoop, ctor_dtor) {
    NativeEventLoop loop;
}

TEST(NativeEventLoop, runs_functions) {
    NativeEventLoop loop;
    std::atomic_int count{41};
    semaphore ready_sem;
    loop.post( [&] () {
        count++;
        ready_sem.notify();
    });
    ready_sem.wait();
    EXPECT_EQ(count, 42);
}

TEST(NativeEventLoop, runs_multi_functions) {
    NativeEventLoop loop;
    std::atomic_bool a_ran{false};
    std::atomic_bool b_ran{false};

    semaphore ready_sem;
    loop.post( [&] () {
        EXPECT_EQ(a_ran, false);
        EXPECT_EQ(b_ran, false);
        a_ran = true;
    });
    loop.post( [&] () {
        EXPECT_EQ(a_ran, true);
        EXPECT_EQ(b_ran, false);
        b_ran = true;
        ready_sem.notify();
    });

    ready_sem.wait();
    EXPECT_EQ(a_ran, true);
    EXPECT_EQ(b_ran, true);
}
