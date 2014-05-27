#include "stl.hpp"
#include <atomic>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>

#include <mx3/mx3.hpp>
using namespace mx3;

TEST(NativeEventLoop, ctor_dtor) {
    NativeEventLoop loop;
}

TEST(NativeEventLoop, dtor_when_blocked) {
    auto loop = make_unique<NativeEventLoop>();
    semaphore ready_sem;
    std::atomic_bool finished {false};
    loop->post( [&] () {
        ready_sem.notify();
        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        finished = true;
    });
    ready_sem.wait();
    // call dtor by assigning to null
    loop = nullptr;
    EXPECT_EQ(finished, true);
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

TEST(NativeEventLoop, runs_nested_functions) {
    // a slightly tricky example, because an implementation
    // might accidentally hold the lock while calling the fn
    NativeEventLoop loop;
    std::atomic_int count{41};
    semaphore ready_sem;
    loop.post( [&] () {
        loop.post( [&] () {
            count++;
            EXPECT_EQ(count, 43);
            ready_sem.notify();
        });
        count++;
        EXPECT_EQ(count, 42);
    });
    ready_sem.wait();
    EXPECT_EQ(count, 43);
}
