#pragma once
#include "stl.hpp"
#include "interface/http.hpp"
#include "interface/http_callback.hpp"
#include "single_thread_task_runner.hpp"

namespace mx3 {

struct HttpResponse {
    bool error;
    uint16_t http_code;
    string data;
};

class Http final {
  public:
    Http(shared_ptr<mx3_gen::Http> http_impl, const shared_ptr<SingleThreadTaskRunner> & runner);
    void get(const string& url, function<void(HttpResponse)>);
  private:
    class Request final : public mx3_gen::HttpCallback {
      public:
        Request(function<void(HttpResponse)> cb, const shared_ptr<SingleThreadTaskRunner> & on_thread);
        virtual void on_network_error();
        virtual void on_success(int16_t http_code, const string& data);
        void _cb_with(HttpResponse resp);
      private:
        const shared_ptr<SingleThreadTaskRunner> m_cb_thread;
        const function<void(HttpResponse)> m_cb;
    };
    const shared_ptr<SingleThreadTaskRunner> m_cb_thread;
    const shared_ptr<mx3_gen::Http> m_http;
};

}
