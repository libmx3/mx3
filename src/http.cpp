#include "http.hpp"

namespace mx3 {

Http::Http(shared_ptr<mx3_gen::Http> http_impl, const shared_ptr<SingleThreadTaskRunner> & cb_thread)
    : m_cb_thread {std::move(cb_thread)}
    , m_http {std::move(http_impl)} {}

void
Http::get(const string& url, function<void(HttpResponse)> m_cb) {
    m_http->get(url, make_shared<Http::Request>(std::move(m_cb), m_cb_thread) );
}

Http::Request::Request(function<void(HttpResponse)> cb, const shared_ptr<SingleThreadTaskRunner> & on_thread)
    : m_cb_thread {on_thread}
    , m_cb {std::move(cb)} {}

void
Http::Request::on_network_error() {
    HttpResponse resp;
    resp.error = true;
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
    m_cb_thread->post([callback, shared_resp] () {
        callback(std::move(*shared_resp));
    });
}

}
