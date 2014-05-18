// use this file to just try stuff out quickly
// just do "make play" to build and run

// generally, you'd have some kind of stl.hpp file that you could include that does all this stuff
// but just putting it inline here
#include <exception>
#include <iostream>
using std::cout;
using std::endl;
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

// open a database and create if it doesn't exist. Throw an exception in error situations
// (corrupted database errors and IO errors)
shared_ptr<leveldb::DB>
open_db(const string& path) {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB * db = nullptr;
    auto status = leveldb::DB::Open(options, path.c_str(), &db);
    if (!status.ok()) {
        throw std::runtime_error(status.ToString());
    }
    // we wrap this pointer in a shared_ptr and it will automatically close the db when it is unused.
    return shared_ptr<leveldb::DB>{db};
}

int main() {
    cout << "Hello, world" << endl;

    // since we are using a shared_ptr to the database, we don't need to close it!
    // this pattern is called RAII
    auto db = open_db("./test_db");
    return 0;
}
