#pragma once
#include "stl.hpp"
#include <json11/json11.hpp>
#include "types.hpp"
#include "../http.hpp"

namespace github {

github::User parse_user(const json11::Json& json);
void get_users(mx3::Http http, optional<uint64_t> since, function<void(vector<github::User>)>);

class Client final {
  public:
    Client(mx3::Http http_client);
    void get_users(optional<uint64_t> since, function<void(vector<github::User>)>);
  private:
    mx3::Http m_http;
};

}
