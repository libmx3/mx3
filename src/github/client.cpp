#include "client.hpp"
#include <json11/json11.hpp>
using github::Client;
using json11::Json;
#include <iostream>
using std::cout;
using std::endl;

namespace {
    const string BASE_URL = "https://api.github.com";
}

Client::Client(const shared_ptr<mx3::Http>& http_client) : m_http(http_client) {}

// todo error handling?
void
Client::get_users(shared_ptr<mx3::Http> http, optional<uint64_t> since, function<void(vector<github::User>)> callback) {
    string url = BASE_URL + "/users";
    if (since) {
        url += "?since=" + std::to_string(*since);
    }
    http->get(url, [callback] (mx3::HttpResponse response) {
        vector<github::User> users;

        string error;
        auto json_response = Json::parse(response.data, error);
        if (!error.empty()) {
            // there was an error
            // fail somehow
        } else {
            if (json_response.is_array()) {
                for (const auto& item : json_response.array_items()) {
                    users.emplace_back( _parse_user(item) );
                }
            }
        }

        callback(users);
    });
}

void
Client::get_users(optional<uint64_t> since, function<void(vector<github::User>)> callback) {
    Client::get_users(m_http, since, callback);
}

github::User
Client::_parse_user(const json11::Json& data) {
    github::User user;
    user.login               = data["login"].string_value();
    user.id                  = data["id"].number_value();
    user.avatar_url          = data["avatar_url"].string_value();
    user.gravatar_id         = data["gravatar_id"].string_value();
    user.url                 = data["url"].string_value();
    user.html_url            = data["html_url"].string_value();
    user.followers_url       = data["followers_url"].string_value();
    user.following_url       = data["following_url"].string_value();
    user.gists_url           = data["gists_url"].string_value();
    user.starred_url         = data["starred_url"].string_value();
    user.subscriptions_url   = data["subscriptions_url"].string_value();
    user.organizations_url   = data["organizations_url"].string_value();
    user.repos_url           = data["repos_url"].string_value();
    user.events_url          = data["events_url"].string_value();
    user.received_events_url = data["received_events_url"].string_value();
    user.type                = data["type"].string_value();
    user.site_admin          = data["site_admin"].bool_value();
    return user;
}
