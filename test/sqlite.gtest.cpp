#include <gtest/gtest.h>
#include <chrono>

#include "stl.hpp"
#include "src/sqlite/db.hpp"
#include <iostream>
using std::cout;
using std::endl;

using namespace mx3::sqlite;

TEST(sqlite_lib, can_query_version_info) {
    EXPECT_EQ(libversion(), "3.8.4.3");
    EXPECT_EQ(sourceid(), "2014-04-03 16:53:12 a611fa96c4a848614efe899130359c9f6fb889c3");
    EXPECT_EQ(libversion_number(), 3008004);
}

TEST(sqlite_db, can_open_close) {
    const auto db = Db::open(":memory:");
}

TEST(sqlite_db, affinity) {
    const vector<std::pair<string, Affinity>> test_cases {
        {"INT", Affinity::INTEGER},
        {"INTEGER", Affinity::INTEGER},
        {"TINYINT", Affinity::INTEGER},
        {"SMALLINT", Affinity::INTEGER},
        {"MEDIUMINT", Affinity::INTEGER},
        {"BIGINT", Affinity::INTEGER},
        {"UNSIGNED BIG INT", Affinity::INTEGER},
        {"INT2", Affinity::INTEGER},
        {"INT8", Affinity::INTEGER},

        {"CHARACTER(20)", Affinity::TEXT},
        {"VARCHAR(255)", Affinity::TEXT},
        {"VARYING CHARACTER(255)", Affinity::TEXT},
        {"NCHAR(55)", Affinity::TEXT},
        {"NATIVE CHARACTER(70)", Affinity::TEXT},
        {"NVARCHAR(100)", Affinity::TEXT},
        {"TEXT", Affinity::TEXT},
        {"CLOB", Affinity::TEXT},

        {"BLOB", Affinity::NONE},
        {"", Affinity::NONE},

        {"REAL", Affinity::REAL},
        {"DOUBLE", Affinity::REAL},
        {"DOUBLE PRECISION", Affinity::REAL},
        {"FLOAT", Affinity::REAL},

        {"NUMERIC", Affinity::NUMERIC},
        {"DECIMAL(10,5)", Affinity::NUMERIC},
        {"BOOLEAN", Affinity::NUMERIC},
        {"DATE", Affinity::NUMERIC},
        {"DATETIME", Affinity::NUMERIC},

        // Not a recognized type, but `int` matches `point`.
        {"FLOATING POINT", Affinity::INTEGER},
        // String doesn't match any of the sqlite types.
        {"STRING", Affinity::NUMERIC}
    };

    for (const auto& tc : test_cases) {
        const auto db = Db::open(":memory:");
        db->exec("CREATE TABLE `aff_test` (`test_col` " + tc.first + ")");
        const auto info = db->table_info("aff_test").value();
        EXPECT_EQ(info.columns[0].name, "test_col");
        EXPECT_EQ(info.columns[0].type, tc.first);
        EXPECT_EQ(info.columns[0].type_affinity(), tc.second);
    }
}

TEST(sqlite_db, can_do_busy_timeout) {
    auto db = Db::open(":memory:");
    db->busy_timeout(nullopt);
    db->busy_timeout(std::chrono::seconds {5});
}

TEST(sqlite_db, wal_hook) {
    auto db = Db::open(":memory:");
    db->wal_hook([] (const string&, int) {

    });
    db = nullptr;
}

TEST(sqlite_db, schema_info) {
    auto db = Db::open(":memory:");
    auto schema_info = db->schema_info();
    EXPECT_EQ(schema_info.size(), 0);

    string name = "my_table";
    string sql = "CREATE TABLE " + name + " (table_id INTEGER, name TEXT NOT NULL, data BLOB, price FLOAT DEFAULT 1.4, PRIMARY KEY (table_id))";
    db->exec(sql);

    schema_info = db->schema_info();
    auto t = schema_info[0];

    EXPECT_EQ(t.name, name);
    EXPECT_EQ(t.sql, sql);
    EXPECT_EQ(t.columns.size(), 4);

    const auto& c = t.columns;
    EXPECT_EQ(c[0].name, "table_id");
    EXPECT_EQ(c[0].type, "INTEGER");
    EXPECT_EQ(c[0].notnull, false);
    EXPECT_EQ(c[0].dflt_value, nullopt);
    EXPECT_EQ(c[0].is_pk(), true);

    EXPECT_EQ(c[1].name, "name");
    EXPECT_EQ(c[1].type, "TEXT");
    EXPECT_EQ(c[1].notnull, true);
    EXPECT_EQ(c[1].dflt_value, nullopt);
    EXPECT_EQ(c[1].is_pk(), false);

    EXPECT_EQ(c[2].name, "data");
    EXPECT_EQ(c[2].type, "BLOB");
    EXPECT_EQ(c[2].notnull, false);
    EXPECT_EQ(c[2].dflt_value, nullopt);
    EXPECT_EQ(c[2].is_pk(), false);

    EXPECT_EQ(c[3].name, "price");
    EXPECT_EQ(c[3].type, "FLOAT");
    EXPECT_EQ(c[3].notnull, false);
    EXPECT_EQ(c[3].dflt_value, string {"1.4"});
    EXPECT_EQ(c[3].is_pk(), false);

}

TEST(sqlite_db, get_set_user_version) {
    auto db = Db::open_memory();
    int32_t version = db->user_version();
    EXPECT_EQ(version, 0);
    db->set_user_version(493);
    version = db->user_version();
    EXPECT_EQ(version, 493);
    version = db->user_version();
    EXPECT_EQ(version, 493);
    db->set_user_version(77);
    version = db->user_version();
    EXPECT_EQ(version, 77);
}

TEST(sqlite_db, get_schema_version) {
    auto db = Db::open_memory();
    int32_t v1 = db->schema_version();
    db->exec("CREATE TABLE IF NOT EXISTS abc (name TEXT);");
    int32_t v2 = db->schema_version();
    EXPECT_FALSE( v1 == v2 );
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
    db->update_hook([&] (Db::Change change) {
        times_called++;
        EXPECT_EQ(change.type, ChangeType::INSERT);
        EXPECT_EQ(change.table_name, "abc");
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
    db->update_hook([&] (Db::Change change) {
        times_called++;
        EXPECT_EQ(insert_row_id, change.rowid);
        EXPECT_EQ(change.type, ChangeType::DELETE);
        EXPECT_EQ(change.table_name, "abc");
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
    db->update_hook([&] (Db::Change change) {
        times_called++;
        EXPECT_EQ(insert_row_id, change.rowid);
        EXPECT_EQ(change.type, ChangeType::UPDATE);
        EXPECT_EQ(change.table_name, "abc");
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

TEST(sqlite_stmt, get_values) {
    auto db  = Db::open_memory();
    db->exec("CREATE TABLE abc (`s` TEXT, `b` BLOB, `d` DOUBLE, `i` INTEGER)");
    auto insert_stmt = db->prepare("INSERT INTO `abc` (s, b, d, i) VALUES (?, ?, ?, ?)");

    vector<uint8_t> b {1,0,2,3};
    double d = 1.5;
    int64_t i = 8;
    string s = "Steven";
    insert_stmt->bind(1, s);
    insert_stmt->bind(2, b);
    insert_stmt->bind(3, d);
    insert_stmt->bind(4, i);
    insert_stmt->exec();

    auto cursor = db->prepare("SELECT b, d, i, s from `abc`")->exec_query();
    vector<Value> expected_values { b, d, i, s };
    auto real_values = cursor.values();
    EXPECT_EQ(real_values, expected_values);

    vector<string> expected_names { "b", "d", "i", "s" };
    auto real_names = cursor.column_names();
    EXPECT_EQ(expected_names, real_names);

    std::map<string, Value> expected_value_map {
        {"b", b},
        {"d", d},
        {"i", i},
        {"s", s}
    };
    auto real_map = cursor.value_map();
    EXPECT_EQ(real_map, expected_value_map);
}
