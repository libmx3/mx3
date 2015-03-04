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

TEST(sqlite_value, tons_of_tests) {
    auto db  = Db::open_memory();
    vector<uint8_t> b {1,0,2,3};
    double d = 1.5;
    int i = 8;
    string s = "Steven";

    db->exec("CREATE TABLE abc (`s` TEXT, `b` BLOB, `d` DOUBLE, `i` INTEGER)");
    auto insert_stmt = db->prepare("INSERT INTO `abc` (s, b, d, i) VALUES (?, ?, ?, ?)");
    insert_stmt->bind(1, s);
    insert_stmt->bind(2, b);
    insert_stmt->bind(3, d);
    insert_stmt->bind(4, i);
    insert_stmt->exec();

    auto cursor = db->prepare("SELECT * from `abc`;")->exec_query();

    auto v0 = cursor.value_at(0);
    auto v1 = cursor.value_at(1);
    auto v2 = cursor.value_at(2);
    auto v3 = cursor.value_at(3);
    auto v_undefined = cursor.value_at(4);

    EXPECT_EQ(v0.type(), Value::Type::STRING);
    EXPECT_EQ(v1.type(), Value::Type::BLOB);
    EXPECT_EQ(v2.type(), Value::Type::DOUBLE);
    EXPECT_EQ(v3.type(), Value::Type::INT);
    EXPECT_EQ(v_undefined.type(), Value::Type::NUL);

    EXPECT_EQ(v0.string_value(), s);
    // access twice to make sure we don't move it out
    EXPECT_THROW(v0.blob_value(),   std::runtime_error);
    EXPECT_THROW(v0.blob_value(),   std::runtime_error);
    EXPECT_THROW(v0.double_value(), std::runtime_error);
    EXPECT_THROW(v0.int_value(),    std::runtime_error);
    EXPECT_THROW(v0.int64_value(),  std::runtime_error);

    EXPECT_THROW(v1.string_value(), std::runtime_error);
    // access twice to make sure we don't move it out
    EXPECT_EQ(v1.blob_value(), b);
    EXPECT_EQ(v1.blob_value(), b);
    EXPECT_THROW(v1.double_value(), std::runtime_error);
    EXPECT_THROW(v1.int_value(),    std::runtime_error);
    EXPECT_THROW(v1.int64_value(),  std::runtime_error);

    EXPECT_THROW(v2.string_value(), std::runtime_error);
    EXPECT_THROW(v2.blob_value(),   std::runtime_error);
    EXPECT_THROW(v2.blob_value(),   std::runtime_error);
    EXPECT_EQ(v2.double_value(), d);
    EXPECT_EQ(v2.int_value(), static_cast<int>(d));
    EXPECT_EQ(v2.int64_value(), static_cast<int64_t>(d));

    EXPECT_THROW(v3.string_value(), std::runtime_error);
    EXPECT_THROW(v3.blob_value(),   std::runtime_error);
    EXPECT_EQ(v3.double_value(), i);
    EXPECT_EQ(v3.int_value(),    i);
    EXPECT_EQ(v3.int64_value(),  i);

    EXPECT_THROW(v_undefined.blob_value(),   std::runtime_error);
    EXPECT_THROW(v_undefined.double_value(), std::runtime_error);
    EXPECT_THROW(v_undefined.int_value(),    std::runtime_error);
    EXPECT_THROW(v_undefined.int64_value(),  std::runtime_error);
    EXPECT_THROW(v_undefined.string_value(), std::runtime_error);

    auto v1_copy = v1;
    EXPECT_EQ(v1_copy.blob_value(),   b);
    EXPECT_EQ(v1_copy.blob_value(),   b);
    EXPECT_THROW(v1_copy.double_value(), std::runtime_error);
    EXPECT_THROW(v1_copy.int_value(),    std::runtime_error);
    EXPECT_THROW(v1_copy.int64_value(),  std::runtime_error);
    EXPECT_THROW(v1_copy.string_value(), std::runtime_error);

    EXPECT_EQ(v1_copy, v1);

    auto v0_copy = v0;
    EXPECT_EQ(v0, v0_copy);
    EXPECT_EQ(v0, v0);

    string s1 = v0_copy.move_string();
    string s2 = v0_copy.move_string();
    EXPECT_FALSE( s1 == s2 );
    EXPECT_EQ(s2, "");

    s1 = cursor.value_at(0).string_value();
    EXPECT_EQ(s1, s);

    int64_t ONE = 1;
    double ONE_FLOAT = 1.0;
    EXPECT_EQ( Value{ONE}, Value{ONE_FLOAT} );
    EXPECT_EQ( Value{ONE_FLOAT}, Value{ONE} );
    EXPECT_FALSE( Value{ONE} == Value{"1"} );
    EXPECT_FALSE( Value{"1"} == Value{ONE} );

    // can construct a map, set
    std::map<string, Value> row;
    std::map<Value, string> row_rev;
    std::set<Value> things;

    vector<Value> v { .7, static_cast<int64_t>(1), 1.5, static_cast<int64_t>(1000) };

    EXPECT_TRUE( v[0] < v[1] );
    EXPECT_TRUE( v[1] < v[2] );
    EXPECT_TRUE( v[2] < v[3] );
}

TEST(sqlite_value, basic_tests) {
    using TestType = std::tuple<Value, Value::Type, bool, bool>;
    using Vt = Value::Type;
    vector<TestType> tests {
        {Value {nullptr}, Vt::NUL, true, false},
        {Value {static_cast<int64_t>(15)}, Vt::INT, false, true},
        {Value {static_cast<int64_t>(14)}, Vt::INT, false, true},
        {Value {static_cast<double>(15.5)}, Vt::DOUBLE, false, true},
        {Value {static_cast<double>(16.5)}, Vt::DOUBLE, false, true},
        {Value {"Hello"}, Vt::STRING, false, false},
        {Value {"hello"}, Vt::STRING, false, false},
        {Value {vector<uint8_t>{3, 0, 1, 2}}, Vt::BLOB, false, false},
        {Value {vector<uint8_t>{0, 1, 2, 3}}, Vt::BLOB, false, false}
    };

    for (const auto& test : tests) {
        const auto& v = std::get<0>(test);
        EXPECT_EQ( v.type(),       std::get<1>(test) );
        EXPECT_EQ( v.is_null(),    std::get<2>(test) );
        EXPECT_EQ( v.is_numeric(), std::get<3>(test) );

        Value v_copy = v;
        EXPECT_EQ( v_copy.type(),       std::get<1>(test) );
        EXPECT_EQ( v_copy.is_null(),    std::get<2>(test) );
        EXPECT_EQ( v_copy.is_numeric(), std::get<3>(test) );
        EXPECT_TRUE( v == v_copy );
        EXPECT_TRUE( v_copy == v );

        Value v_move = std::move(v_copy);
        EXPECT_EQ( v_move.type(),       std::get<1>(test) );
        EXPECT_EQ( v_move.is_null(),    std::get<2>(test) );
        EXPECT_EQ( v_move.is_numeric(), std::get<3>(test) );
        EXPECT_TRUE( v == v_move);
        EXPECT_TRUE( v_move == v );

        // copy assign
        v_copy = v_move;
        EXPECT_EQ( v_copy.type(),       std::get<1>(test) );
        EXPECT_EQ( v_copy.is_null(),    std::get<2>(test) );
        EXPECT_EQ( v_copy.is_numeric(), std::get<3>(test) );
        EXPECT_TRUE( v == v_copy );
        EXPECT_TRUE( v_copy == v );

        // move assign
        v_move = Value {44.0};
        v_move = std::move(v_copy);
        EXPECT_EQ( v_move.type(),       std::get<1>(test) );
        EXPECT_EQ( v_move.is_null(),    std::get<2>(test) );
        EXPECT_EQ( v_move.is_numeric(), std::get<3>(test) );
        EXPECT_TRUE( v == v_move);
        EXPECT_TRUE( v_move == v );

        for (const auto& t2 : tests) {
            if (&t2 == &test) {
                EXPECT_TRUE( v == std::get<0>(t2) );
                EXPECT_TRUE( std::get<0>(t2) == v );
            } else {
                Value t_copy = std::get<0>(t2);
                t_copy = v;
                EXPECT_TRUE( t_copy == v );
                Value t_copy_2 = std::move(t_copy);
                EXPECT_TRUE( t_copy_2 == v );
                EXPECT_FALSE( v == std::get<0>(t2) );
                EXPECT_FALSE( std::get<0>(t2) == v );
            }
        }
    }
}
