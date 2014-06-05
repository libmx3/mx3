#pragma once
#include "stl.hpp"
#include "types.hpp"
#include "../http.hpp"
#include <json11/json11.hpp>

namespace github {

class Client final {
  public:
    Client(const shared_ptr<mx3::Http>& http_client);
    // todo error handling?
    void get_users(function<void(vector<github::User>)>);
  private:
    static github::User _parse_user(const json11::Json& json);
    shared_ptr<mx3::Http> m_http;
};

}
