#include "http.hpp"

using mx3::Http;
using mx3::HttpResponse;
using mx3::EventLoopRef;

Http::Http(shared_ptr<mx3_gen::Http> http_impl, EventLoopRef cb_thread)
    : m_cb_thread {std::move(cb_thread)}
    , m_http {std::move(http_impl)} {}

void
Http::get(const string& url, function<void(HttpResponse)> m_cb) {
    m_http->get(url, make_shared<Http::Request>(std::move(m_cb), m_cb_thread) );
}

Http::Request::Request(function<void(HttpResponse)> cb, const EventLoopRef& on_thread)
    : m_cb_thread {on_thread}
    , m_cb {std::move(cb)} {}

void
Http::Request::on_network_error(int16_t code) {
    HttpResponse resp;
    resp.error = code;
    _cb_with(std::move(resp));
}

void
Http::Request::on_success(int16_t http_code, const string& data) {
    HttpResponse resp;
    resp.error = false;
    resp.http_code = http_code;
    resp.data = data;
    _cb_with(std::move(resp));
}

void
Http::Request::_cb_with(HttpResponse resp) {
    auto callback = m_cb;
    auto shared_resp = make_shared<HttpResponse>(std::move(resp));
    m_cb_thread.post([callback, shared_resp] () {
        callback(std::move(*shared_resp));
    });
}

