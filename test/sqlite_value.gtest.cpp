#include <gtest/gtest.h>
#include <chrono>

#include "stl.hpp"
#include <algorithm>
#include "src/sqlite/db.hpp"
#include "src/sqlite/value.hpp"
#include <iostream>
using std::cout;
using std::endl;

using namespace mx3::sqlite;

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

TEST(sqlite_value, null_ordering) {
    EXPECT_TRUE(Value{} < Value{5});
    EXPECT_TRUE(Value{nullptr} < Value{5});
    EXPECT_FALSE(Value{5} < Value{});
    EXPECT_FALSE(Value{5} < Value{nullptr});

    EXPECT_FALSE(Value{} < Value{nullptr});
    EXPECT_FALSE(Value{nullptr} < Value{});
}

TEST(sqlite_value, numeric_ordering_integration) {
    const vector<Value> interesting_values {
        {},
        nullptr,

        static_cast<int>(5),
        static_cast<int>(50),
        static_cast<int>(499),

        static_cast<int64_t>(4),
        static_cast<int64_t>(40),
        static_cast<int64_t>(399),

        static_cast<double>(4.01),
        static_cast<double>(40.01),
        static_cast<double>(399.01),

        vector<uint8_t> {{1, 2, 3}},
        vector<uint8_t> {{0, 2, 3}},
        // No braced init, since it is ambiguous.
        vector<uint8_t> ({1}),
        vector<uint8_t> {{1, 2, 3, 4}},

        "A",
        "B",
        "C",
        "AA",
        "null"
    };

    vector<string> sql_types {
        "",
        "FLOAT",
        "INTEGER",
        "BLOB",
        "TEXT"
    };

    const auto db = Db::open(":memory:");
    for (const auto& sql_type : sql_types) {
        db->exec("CREATE TABLE `ordering_table` (data " + sql_type + ")");
        const auto insert_stmt = db->prepare("INSERT INTO `ordering_table` (data) VALUES (?1)");
        const auto delete_stmt = db->prepare("DELETE FROM `ordering_table`");
        const auto select_stmt = db->prepare("SELECT data FROM `ordering_table` ORDER BY data");
        const Affinity affinity = db->table_info("ordering_table").value().columns.at(0).type_affinity();
        for (const auto& a : interesting_values) {
            for (const auto& b : interesting_values) {
                if (&a == &b) {
                    continue;
                }
                delete_stmt->exec();

                insert_stmt->bind(1, a);
                insert_stmt->exec();
                insert_stmt->reset();
                insert_stmt->clear_bindings();
                insert_stmt->bind(1, b);
                insert_stmt->exec();
                insert_stmt->reset();
                insert_stmt->clear_bindings();

                select_stmt->reset();
                const auto actual_rows = select_stmt->exec_query().all_rows();

                Value coerced_a = a;
                Value coerced_b = b;
                // Since TEXT affinity columns change the storage class, we perform the same
                // operation before determining the expected values.
                if (affinity == Affinity::TEXT) {
                    std::stringstream s1;
                    if (a.type() == Value::Type::DOUBLE) {
                        s1 << a.double_value();
                        coerced_a = s1.str();
                    } else if (a.type() == Value::Type::INT) {
                        s1 << a.int_value();
                        coerced_a = s1.str();
                    }

                    std::stringstream s2;
                    if (b.type() == Value::Type::DOUBLE) {
                        s2 << b.double_value();
                        coerced_b = s2.str();
                    } else if (b.type() == Value::Type::INT) {
                        s2 << b.int_value();
                        coerced_b = s2.str();
                    }
                }

                if (coerced_a < coerced_b) {
                    EXPECT_EQ(actual_rows[0][0], coerced_a);
                    EXPECT_EQ(actual_rows[1][0], coerced_b);
                } else {
                    EXPECT_EQ(actual_rows[0][0], coerced_b);
                    EXPECT_EQ(actual_rows[1][0], coerced_a);
                }
            }
        }
        db->exec("DROP TABLE `ordering_table`;");
    }
}
