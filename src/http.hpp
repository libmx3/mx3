#pragma once
#include "stl.hpp"

namespace mx3 {

struct HttpResponse {
    bool error;
    uint16_t http_code;
    string data;
};

class Http {
  public:
    virtual ~Http() {}
    virtual void get(const string& url, function<void(HttpResponse)>) = 0;
};

}
