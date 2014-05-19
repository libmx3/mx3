// use this file to just try stuff out quickly
// just do "make play" to build and run

// generally, you'd have some kind of stl.hpp file that you could include that does all this stuff
// but just putting it inline here
#include <exception>
#include <iostream>
using std::cout;
using std::endl;
using std::cin;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <memory>
using std::shared_ptr;

#include <leveldb/db.h>
using namespace leveldb;

#include <mx3/mx3.hpp>
using namespace mx3;

#include <json11/json11.hpp>
using json11::Json;

#include <sqlite3/sqlite3.h>

// caller is responsible for closing the db pointer
sqlite3 *
open_sqlite(const string& path) {
    sqlite3 * db = nullptr;
    auto error = sqlite3_open(path.c_str(), &db);
    if (error != SQLITE_OK) {
        throw std::runtime_error("Could not open database at path: " + path);
    }
    return db;
}

int main() {
    mx3::Api api("./test_ldb");
    if (!api.has_user()) {
        string name;
        cout << "Please enter your username: ";
        cin >> name;
        api.set_username(name);
    }
    cout << "Hello, " << api.get_username() << endl;

    return 0;
}
