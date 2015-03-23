#include <gtest/gtest.h>

#include "stl.hpp"
#include "src/sqlite/db.hpp"
#include "src/sqlite_query/observable_db.hpp"
#include <cstdio>
#include <iostream>
using std::cout;
using std::endl;

using namespace mx3::sqlite;

template<typename FromType, typename F>
auto map(const vector<FromType>& data, F&& map_fn) -> vector<decltype(map_fn(*data.begin()))> {
    using ToType  = decltype(map_fn(*data.begin()));
    vector<ToType> result;
    result.reserve(data.size());
    for (const auto& d : data) {
        result.push_back(map_fn(d));
    }
    return result;
}

static void s_print_row(const optional<mx3::sqlite::Row>& row) {
    if (row) {
        for (const auto& val : *row) {
            cout << val << " ";
        }
        cout << endl;
    } else {
        cout << "[null]" << endl;
    }
}

static vector<Value> trivial_row(const int x) {
    return vector<Value> {{x}};
}

TEST(sqlite_db, full_test) {
    const string filename = "test.db";
    {
    // database set up must be done outside of the observable databse
    std::remove(filename.c_str());
    const auto db = Db::open(filename);
    db->enable_wal();
    db->exec("CREATE TABLE test_table (id INTEGER PRIMARY KEY, data TEXT, sort INTEGER)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (1, 'one', 3)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (2, 'two', 3)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (3, 'three', 5)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (4, 'four', 4)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (5, 'five', 4)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (6, 'six', 3)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (7, 'seven', 5)");
    db->exec("INSERT INTO test_table (id, data, sort) VALUES (14, 'fourteen', 8)");
    }

    const auto write_db = Db::open(filename);
    const auto read_db = Db::open(filename);

    auto db = make_unique<experimental::ObservableDb>(write_db, read_db, [] (DbChanges db_changes) {
        (void)db_changes;
    });
}

TEST(sqlite_db, can_observe) {
    const string filename{"test.db"};

    {
    // database set up must be done outside of the observable databse
    std::remove(filename.c_str());
    const auto db = Db::open(filename);
    db->enable_wal();
    db->exec("CREATE TABLE fake_table (table_id TEXT, name TEXT NOT NULL, `data` BLOB, price FLOAT DEFAULT 1.4, PRIMARY KEY (table_id))");
    }

    const auto write_db = Db::open(filename);
    const auto read_db = Db::open(filename);

    auto db = make_unique<experimental::ObservableDb>(write_db, read_db, [] (DbChanges db_changes) {
        for (const auto& table : db_changes) {
            cout << table.first << " had " << table.second.row_changes.size() << " changes" << endl;
            for (const auto& col : table.second.column_names) {
                cout << col << ", ";
            }
            cout << endl;
            cout << "==============================================" << endl;
            size_t change_num = 0;
            for (const auto& row_change : table.second.row_changes) {
                cout << "Change " << change_num << endl;
                cout << "    ";
                s_print_row(row_change.old_row);
                cout << "    ";
                s_print_row(row_change.new_row);
                change_num++;
            }
        }
    });

    TransactionStmts write_stmts {write_db};
    {
        WriteTransaction write_trans {write_stmts};
        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (5, \"hello5\");");
        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (1, \"hello1\");");
        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (6, \"hello6\");");
        write_trans.commit();
    }

    {
        WriteTransaction write_trans {write_stmts};
        write_db->exec("UPDATE fake_table SET name = \"hello2\" WHERE table_id != 5;");
        write_db->exec("UPDATE fake_table SET price = 1.234 WHERE table_id == 1;");

        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (7, \"i_should_be_delete\");");
        write_db->exec("DELETE FROM fake_table WHERE table_id = 7");
        write_trans.commit();
    }

    {
        WriteTransaction write_trans {write_stmts};
        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (15, \"fifteen\");");
        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (16, \"sixteen\");");
        write_db->exec("UPDATE fake_table SET name = \"fifteen_6\"  WHERE table_id == 15;");
        write_db->exec("DELETE FROM fake_table WHERE table_id = 15");
        write_db->exec("INSERT INTO fake_table (table_id, name) VALUES (15, \"fifteen_2\");");
        write_db->exec("UPDATE fake_table SET name = \"fifteen_3\"  WHERE table_id == 15;");
        write_trans.commit();
    }

    // close the database
    db = nullptr;
    std::remove(filename.c_str());
}
