#include "api.hpp"
using std::string;
using std::unique_ptr;
using mx3::Api;
using json11::Json;

#include <iostream>
using std::cout;
using std::endl;

namespace {
    const string USERNAME_KEY = "username";
}

Api::Api(const string& path) : m_db(_open_database(path)) {}

bool
Api::has_user() const {
    return !_get_value(USERNAME_KEY).is_null();
}

string
Api::get_username() const {
    return _get_value(USERNAME_KEY).string_value();
}

void
Api::set_username(const string& username) {
    _set_value(USERNAME_KEY, username);
}

json11::Json
Api::_get_value(const std::string& key) const {
    string value;
    auto status = m_db->Get({}, key, &value);
    _throw_if_error(status);

    // an "expected" error is that that key isn't found, represent that as a null
    if (status.IsNotFound()) {
        return Json(nullptr);
    }

    string error;
    Json result = Json::parse(value, error);
    if (error.empty()) {
        return result;
    } else {
        return Json(nullptr);
    }
}

void
Api::_set_value(const std::string& key, const json11::Json& value) {
    auto serialized = value.dump();
    auto status = m_db->Put({}, key, serialized.c_str());
    _throw_if_error(status);
}

void
Api::_throw_if_error(const leveldb::Status& status) {
    if (status.ok() || status.IsNotFound()) {
        return;
    }

    auto what = status.ToString();
    throw std::runtime_error(what);
}

unique_ptr<leveldb::DB>
Api::_open_database(const std::string& db_path) {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB * db = nullptr;
    auto status = leveldb::DB::Open(options, db_path.c_str(), &db);
    if (!status.ok()) {
        throw std::runtime_error(status.ToString());
    }
    // we wrap this pointer in a shared_ptr and it will automatically close the db when it is unused.
    return unique_ptr<leveldb::DB>{db};
}
