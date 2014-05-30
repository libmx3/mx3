#pragma once
#include "stl.hpp"
#include "types.hpp"
#include "../http.hpp"

namespace github {

class Client {
  public:
    Client(const shared_ptr<mx3::Http>& http_client);
    // todo error handling?
    void get_users(function<void(vector<github::User>)>);
  private:
    shared_ptr<mx3::Http> m_http;
};

}
