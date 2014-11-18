#include "leveldb_store.hpp"

using mx3::LeveldbStore;
using json11::Json;

LeveldbStore::LeveldbStore(const string& db_path) {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB * db = nullptr;
    auto status = leveldb::DB::Open(options, db_path.c_str(), &db);
    if (!status.ok()) {
        throw std::runtime_error(status.ToString());
    }
    // we wrap this pointer in a unique_ptr and it will automatically close the db when it is unused.
    // if you try to use a unique_ptr, you'll likely run into problems. Look up "move semantics" to figure out why
    // if you just want a quick fix, change it to shared_ptr, and your code will start to work.
    m_db = unique_ptr<leveldb::DB>{db};
}

Json
LeveldbStore::get(const string& key) {
    string value;
    auto status = m_db->Get({}, key, &value);
    _throw_if_error(status);

    // an "expected" error is that that key isn't found, represent that as a null
    if (status.IsNotFound()) {
        return nullptr;
    }

    string error;
    Json result = Json::parse(value, error);
    if (error.empty()) {
        return result;
    } else {
        return nullptr;
    }
}

void
LeveldbStore::set(const string& key, const Json& value) {
    auto serialized = value.dump();
    auto status = m_db->Put({}, key, serialized.c_str());
    _throw_if_error(status);
}

void
LeveldbStore::_throw_if_error(const leveldb::Status& status) {
    if (status.ok() || status.IsNotFound()) {
        return;
    }

    auto what = status.ToString();
    throw std::runtime_error(what);
}

