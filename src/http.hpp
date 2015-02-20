#pragma once
#include "stl.hpp"
#include "interface/http.hpp"
#include "interface/http_callback.hpp"
#include "event_loop.hpp"

namespace mx3 {

struct HttpResponse {
    int16_t  error;
    uint16_t http_code;
    string data;
};

class Http final {
  public:
    Http(shared_ptr<mx3_gen::Http> http_impl, EventLoopRef on_thread);
    void get(const string& url, function<void(HttpResponse)>);
  private:
    class Request final : public mx3_gen::HttpCallback {
      public:
        Request(function<void(HttpResponse)> cb, const EventLoopRef& on_thread);
        virtual void on_network_error(int16_t code);
        virtual void on_success(int16_t http_code, const string& data);
        void _cb_with(HttpResponse resp);
      private:
        mx3::EventLoopRef m_cb_thread;
        function<void(HttpResponse)> m_cb;
    };
    mx3::EventLoopRef m_cb_thread;
    shared_ptr<mx3_gen::Http> m_http;
};

}
