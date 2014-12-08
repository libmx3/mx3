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
    auto db1  = Db::open_memory();
    db1->exec("CREATE TABLE IF NOT EXISTS abc (name TEXT);");
    db1->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    auto cursor = db1->prepare("SELECT * from `abc`;")->exec_query();
    EXPECT_EQ(cursor.is_valid(), true);
    }

    {
    auto db2  = Db::open_memory();
    db2->exec("CREATE TABLE IF NOT EXISTS abc (name TEXT);");
    auto cursor = db2->prepare("SELECT * from `abc`;")->exec_query();
    EXPECT_EQ(cursor.is_valid(), false);
    }
}

TEST(sqlite_db, can_exec) {
    auto db = Db::open_memory();
    db->exec("SELECT * from sqlite_master");
}

TEST(sqlite_db, can_exec_scalar) {
    auto db = Db::open_memory();

    auto result = db->exec_scalar("PRAGMA user_version;");
    EXPECT_EQ(result, 0);

    db->exec("CREATE TABLE IF NOT EXISTS abc (name TEXT);");
    result = db->exec_scalar("SELECT COUNT(*) FROM sqlite_master");
    EXPECT_EQ(result, 1);

    EXPECT_THROW( db->exec_scalar("SELCT * from sqlite_master'"), std::runtime_error );
}

TEST(sqlite_db, throw_on_bad_exec) {
    auto db = Db::open_memory();
    EXPECT_THROW( db->exec("SELCT * from'"), std::runtime_error );
}

TEST(sqlite_db, exec_works) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    auto r1 = db->prepare("UPDATE `abc` SET `name` = 'first' WHERE `name` = 'first'")->exec();
    EXPECT_EQ(r1, 0);

    auto r2 = db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    EXPECT_EQ(r2, 1);

    // should throw an error
    auto s2 = db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')");
    EXPECT_THROW(s2->exec(), std::runtime_error);
}

TEST(sqlite_db, update_hook_insert) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");

    size_t times_called = 0;
    db->update_hook([&] (ChangeType type, string db_name, string table_name, int64_t row_id) {
        times_called++;
        EXPECT_EQ(type, ChangeType::INSERT);
        EXPECT_EQ(table_name, "abc");
    });
    db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    EXPECT_EQ(times_called, 1);
}

TEST(sqlite_db, update_hook_delete) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    int64_t insert_row_id = db->last_insert_rowid();

    size_t times_called = 0;
    db->update_hook([&] (ChangeType type, string db_name, string table_name, int64_t row_id) {
        times_called++;
        EXPECT_EQ(insert_row_id, row_id);
        EXPECT_EQ(type, ChangeType::DELETE);
        EXPECT_EQ(table_name, "abc");
    });
    db->prepare("DELETE FROM `abc` WHERE 1")->exec();
    EXPECT_EQ(times_called, 1);
}

TEST(sqlite_db, update_hook_update) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    db->prepare("INSERT INTO `abc` (`name`) VALUES ('first')")->exec();
    int64_t insert_row_id = db->last_insert_rowid();

    size_t times_called = 0;
    db->update_hook([&] (ChangeType type, string db_name, string table_name, int64_t row_id) {
        times_called++;
        EXPECT_EQ(insert_row_id, row_id);
        EXPECT_EQ(type, ChangeType::UPDATE);
        EXPECT_EQ(table_name, "abc");
    });
    auto stmt = db->prepare("UPDATE `abc` SET `name` = 'second' WHERE rowid = ?1");
    stmt->bind(1, insert_row_id);
    stmt->exec();
    EXPECT_EQ(times_called, 1);
}

TEST(sqlite_stmt, param_count) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    auto stmt = db->prepare("SELECT * from `abc` WHERE name = ?1");
    EXPECT_EQ(stmt->param_count(), 1);

    stmt = db->prepare("SELECT * from `abc`");
    EXPECT_EQ(stmt->param_count(), 0);

    stmt = db->prepare("SELECT * from `abc` WHERE name = ?1 OR name = ?1 OR name = ?1");
    EXPECT_EQ(stmt->param_count(), 1);

    stmt = db->prepare("SELECT * from `abc` WHERE name = ?");
    EXPECT_EQ(stmt->param_count(), 1);

    stmt = db->prepare("SELECT * from `abc` WHERE name = ? OR name = ?2");
    EXPECT_EQ(stmt->param_count(), 2);
}

TEST(sqlite_stmt, param_name) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    auto stmt = db->prepare("SELECT * from `abc` WHERE name = ?1");
    EXPECT_EQ(stmt->param_name(1).value_or(""), "?1");

    stmt = db->prepare("SELECT * from `abc` WHERE name = ?");
    EXPECT_EQ(stmt->param_name(1), nullopt);

    stmt = db->prepare("SELECT * from `abc` WHERE name = :name");
    EXPECT_EQ(stmt->param_name(1).value_or(""), ":name");
}

TEST(sqlite_stmt, param_index) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (name TEXT, PRIMARY KEY(name) )");
    auto stmt = db->prepare("SELECT * from `abc` WHERE name = ?1");
    EXPECT_EQ(stmt->param_index("?1"), 1);
    EXPECT_THROW(stmt->param_index(":not_a_param"), std::runtime_error);

    stmt = db->prepare("SELECT * from `abc` WHERE name = :name");
    EXPECT_EQ(stmt->param_index(":name"), 1);
    EXPECT_THROW(stmt->param_index("?name"), std::runtime_error);
    EXPECT_THROW(stmt->param_index("name"), std::runtime_error);
}

