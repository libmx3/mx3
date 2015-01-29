#include "stl.hpp"
#include <atomic>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>

#include <mx3/mx3.hpp>
using namespace mx3;

class CppThreadLauncher : public mx3_gen::ThreadLauncher {
public:
    virtual void start_thread(const optional<string> &, const shared_ptr<mx3_gen::AsyncTask> & run_fn) {
        std::thread t([run_fn]() {
            run_fn->execute();
        });
        t.detach();
    }
};

shared_ptr<mx3_gen::EventLoop> make_loop() {
    return make_shared<EventLoopCpp>( make_shared<CppThreadLauncher>() );
}

TEST(EventLoopCpp, ctor_dtor) {
    auto loop = make_loop();
}

TEST(EventLoopCpp, dtor_when_blocked) {
    auto loop = new mx3::EventLoopRef { make_loop() };
    semaphore ready_sem;
    std::atomic_bool finished {false};
    loop->post( [&] () {
        ready_sem.notify();
        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        finished = true;
    });
    ready_sem.wait();
    delete loop;
    EXPECT_EQ(finished, true);
}

TEST(EventLoopCpp, runs_functions) {
    auto loop = mx3::EventLoopRef { make_loop() };
    std::atomic_int count{41};
    semaphore ready_sem;
    loop.post( [&] () {
        count++;
        ready_sem.notify();
    });
    ready_sem.wait();
    EXPECT_EQ(count, 42);
}

TEST(EventLoopCpp, runs_multi_functions) {
    auto loop = mx3::EventLoopRef { make_loop() };
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

TEST(EventLoopCpp, runs_nested_functions) {
    // a slightly tricky example, because an implementation
    // might accidentally hold the lock while calling the fn
    auto loop = mx3::EventLoopRef { make_loop() };
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
