#pragma once
#include "stl.hpp"
#include "event_loop.hpp"

namespace mx3 {

template<typename T>
class QueryResult final {
  public:
    QueryResult(const shared_ptr<mx3::EventLoop>& main_loop) : m_loop(main_loop) {}

    // must be called from the main thread
    size_t count() const {
        return m_data.size();
    }

    // must be called from the main thread
    const T& get_item_at(size_t position) {
        return m_data.at(position);
    }

    // this needs to be called from the main thread, *change_fn* is guaranteed to be called from the main thread
    void on_change(const function<void()>& change_fn) { m_change_fn = change_fn; }

    // meant to be called by data provider (from any thread)
    void update_data(const vector<T>& data) {
        m_loop->post( [data,this] () {
            m_data = data;
            if (m_change_fn) {
                m_change_fn();
            }
        });
    }
  private:
    shared_ptr<mx3::EventLoop> m_loop;
    function<void()> m_change_fn;
    vector<T> m_data;
};

template<typename T>
using QueryResultPtr = shared_ptr<QueryResult<T>>;
}
