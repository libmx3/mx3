#include "api.hpp"
#include "stl.hpp"

using std::string;
using std::unique_ptr;
using mx3::Api;
using json11::Json;

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;


#include "sql_snapshot.hpp"

namespace {
    const string USERNAME_KEY = "username";
}

Api::Api(const string& root_path) :
    m_sqlite(),
    // todo this needs to use a fs/path abstraction (not yet built)
    m_ldb(_open_database(root_path + "/example_ldb"))
{
    // still needs fs abstraction :(
    string sqlite_path = root_path + "/example.sqlite";
    m_sqlite.open(sqlite_path.c_str());
    _setup_db();

    auto j_launch_number = _get_value("launch_number");
    size_t launch = 0;
    if (j_launch_number.is_number()) {
        launch = j_launch_number.number_value() + 1;
    }
    _set_value("launch_number", static_cast<double>(launch));
    _log_launch(launch);
}

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

unique_ptr<mx3::SqlSnapshot>
Api::get_launches() {
    auto stmt = m_sqlite.compileStatement("SELECT content FROM Data");
    auto query = stmt.execQuery();
    return std::unique_ptr<mx3::SqlSnapshot>( new mx3::SqlSnapshot(query) );
}

void
Api::_log_launch(size_t num) {
    string log_line = "Launch #" + std::to_string(num);
    auto stmt = m_sqlite.compileStatement("INSERT INTO `Data` (content) VALUES (?1)");
    stmt.bind(1, log_line.c_str());
    stmt.execDML();
}

void
Api::_setup_db() {
    vector<string> setup_commands  {
        "CREATE TABLE IF NOT EXISTS `Data` (content TEXT)"
    };
    for (const auto& cmd : setup_commands) {
        m_sqlite.execDML(cmd.c_str());
    }
}

json11::Json
Api::_get_value(const std::string& key) const {
    string value;
    auto status = m_ldb->Get({}, key, &value);
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
    auto status = m_ldb->Put({}, key, serialized.c_str());
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
    // we wrap this pointer in a unique_ptr and it will automatically close the db when it is unused.
    // if you try to use a unique_ptr, you'll likely run into problems. Look up "move semantics" to figure out why
    // if you just want a quick fix, change it to shared_ptr, and your code will start to work.
    return unique_ptr<leveldb::DB>{db};
}
