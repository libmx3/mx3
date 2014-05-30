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

        if (json_response.is_object()) {
            github::User user;
            // if json_response["login"].is_string();
            user.login     = json_response["login"].string_value();
            bool b_value   = json_response["bool_key_name"].bool_value();
            int int_value  = json_response["int_key_name"].int_value();
            double d_value = json_response["double_key_name"].number_value();
            users.push_back(user);

            // shut up compiler!
            (void)b_value;
            (void)int_value;
            (void)d_value;
        }

        if (json_response.is_array()) {
            for (const auto& item : json_response.array_items()) {
                // don't make the compiler yell
                (void)item;
            }
        }

        callback(users);
    });
}
