#include "client.hpp"
#include <json11/json11.hpp>
using github::Client;
using json11::Json;

namespace {
    const string BASE_URL = "https://api.github.com";
}

Client::Client(const shared_ptr<mx3::Http>& http_client) : m_http(http_client) {}

// todo error handling?
void
Client::get_users(function<void(vector<github::User>)> callback) {
    m_http->get(BASE_URL + "/users", [callback] (mx3::HttpResponse response) {
        vector<github::User> users;

        string error;
        auto json_response = Json::parse(response.data, error);
        if (!error.empty()) {
            // there was an error
            // fail somehow
        }

        // parse response.data
        // github::User current_user;
        // current_user.login = "blah"
        // users.push_back(current_user);
        callback(users);
    });
}
