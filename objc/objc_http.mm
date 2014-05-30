#include "objc_http.hpp"
using mx3::objc::ObjcHttp;

void
ObjcHttp::get(const string& url, function<void(HttpResponse)> done_fn) {
    string url_copy = url;
    // objc magic ...  done_fn(response_object);
}
