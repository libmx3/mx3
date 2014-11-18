#include <gtest/gtest.h>
#include "stl.hpp"
#include "src/db/sqlite_store.hpp"
using json11::Json;

TEST(json_store, can_get_and_set) {
    shared_ptr<mx3::JsonStore> db = make_shared<mx3::SqliteStore>("./test.sqlite");
    Json value = db->get("nothing");
    EXPECT_EQ(value.is_null(), true) << "should return null for non-existent values";

    db->set("A", 15);
    Json v = db->get("A");
    EXPECT_EQ(v.int_value(), 15);
}
