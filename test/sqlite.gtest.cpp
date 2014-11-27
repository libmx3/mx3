#include <gtest/gtest.h>
#include "stl.hpp"
#include "src/sqlite/db.hpp"

using namespace mx3::sqlite;

TEST(sqlite_db, can_open_close) {
    auto db = Db::open(":memory:");
    Db stack_db {":memory:"};
}

TEST(sqlite_db, can_exec) {
    Db db {":memory:"};
    db.exec("SELECT * from sqlite_master");
}

TEST(sqlite_db, throw_on_bad_exec) {
    Db db {":memory:"};
    EXPECT_THROW( db.exec("SELCT * from'"), std::runtime_error );
}

TEST(sqlite_db, exec_works) {
    Db db {":memory:"};
    db.exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    auto r1 = db.prepare("UPDATE `abc` SET `name` = 'first' WHERE `name` = 'first'").exec();
    EXPECT_EQ(r1, 0);

    auto r2 = db.prepare("INSERT INTO `abc` (`name`) VALUES ('first')").exec();
    EXPECT_EQ(r2, 1);

    // should throw an error
    auto s2 = db.prepare("INSERT INTO `abc` (`name`) VALUES ('first')");
    EXPECT_THROW(s2.exec(), std::runtime_error);
}
