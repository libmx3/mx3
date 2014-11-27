#include <gtest/gtest.h>
#include "stl.hpp"
#include "src/sqlite/db.hpp"
#include <iostream>
using std::cout;
using std::endl;

using namespace mx3::sqlite;

TEST(sqlite_db, can_open_close) {
    auto db = Db::open(":memory:");
}

// this test relies on the fact that closing a in memory database loses all of its data
TEST(sqlite_db, dtor_closes_db) {
    {
    auto db1  = Db::open(":memory:");
    db1->exec("CREATE TABLE IF NOT EXISTS abc (name TEXT);");
    db1->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    auto cursor = db1->prepare("SELECT * from `abc`;")->exec_query();
    EXPECT_EQ(cursor.has_next(), true);
    }

    {
    auto db2  = Db::open(":memory:");
    db2->exec("CREATE TABLE IF NOT EXISTS abc (name TEXT);");
    auto cursor = db2->prepare("SELECT * from `abc`;")->exec_query();
    EXPECT_EQ(cursor.has_next(), false);
    }
}

TEST(sqlite_db, can_exec) {
    auto db = Db::open(":memory:");
    db->exec("SELECT * from sqlite_master");
}

TEST(sqlite_db, throw_on_bad_exec) {
    auto db = Db::open(":memory:");
    EXPECT_THROW( db->exec("SELCT * from'"), std::runtime_error );
}

TEST(sqlite_db, exec_works) {
    auto db  = Db::open(":memory:");
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    auto r1 = db->prepare("UPDATE `abc` SET `name` = 'first' WHERE `name` = 'first'")->exec();
    EXPECT_EQ(r1, 0);

    auto r2 = db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    EXPECT_EQ(r2, 1);

    // should throw an error
    auto s2 = db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')");
    EXPECT_THROW(s2->exec(), std::runtime_error);
}
