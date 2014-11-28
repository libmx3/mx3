#pragma once
#include "stl.hpp"
#include <json11/json11.hpp>
#include "types.hpp"
#include "../interface/http.hpp"
#include "../interface/http_callback.hpp"

namespace github {

github::User parse_user(const json11::Json& json);

class UsersRequest final : public mx3_gen::HttpCallback {
  public:
    UsersRequest(function<void(vector<github::User>)> users_cb);
    virtual void on_network_error();
    virtual void on_success(const int16_t& http_code, const string& data);
  private:
    function<void(vector<github::User>)> m_success_fn;
};

class Client final {
  public:
    Client(const shared_ptr<mx3_gen::Http>& http_client);
    void get_users(optional<uint64_t> since, function<void(vector<github::User>)>);
    static void get_users(shared_ptr<mx3_gen::Http> http, optional<uint64_t> since, function<void(vector<github::User>)>);
  private:
    shared_ptr<mx3_gen::Http> m_http;
};

}
