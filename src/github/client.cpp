#include "client.hpp"
#include <json11/json11.hpp>

using github::Client;
using github::UsersRequest;

using json11::Json;
#include <iostream>
using std::cout;
using std::endl;

namespace {
    const string BASE_URL = "https://api.github.com";
}

github::User
github::parse_user(const json11::Json& data) {
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

UsersRequest::UsersRequest(function<void(vector<github::User>)> users_cb) : m_success_fn {users_cb} {}

void
UsersRequest::on_network_error() {}

void
UsersRequest::on_success(const int16_t&, const string& data) {
    vector<github::User> users;

    string error;
    auto json_response = Json::parse(data, error);
    if (!error.empty()) {
        // there was an error
        // fail somehow
    } else {
        if (json_response.is_array()) {
            for (const auto& item : json_response.array_items()) {
                users.emplace_back( github::parse_user(item) );
            }
        }
    }
    m_success_fn(users);
}


Client::Client(const shared_ptr<mx3_gen::Http>& http_client) : m_http(http_client) {}

// todo error handling?
void
Client::get_users(shared_ptr<mx3_gen::Http> http, optional<uint64_t> since, function<void(vector<github::User>)> callback) {
    string url = BASE_URL + "/users";
    if (since) {
        url += "?since=" + std_patch::to_string(*since);
    }
    http->get(url, make_shared<github::UsersRequest>(callback));
}

void
Client::get_users(optional<uint64_t> since, function<void(vector<github::User>)> callback) {
    Client::get_users(m_http, since, callback);
}
