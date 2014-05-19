#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <string>
using std::string;

#include <json11/json11.hpp>
using namespace json11;

TEST(json11, simple_parse_test) {
    const string simple_test =
        R"({"k1":"v1", "k2":42, "k3":["a",123,true,false,null]})";

    string err;
    auto json = Json::parse(simple_test, err);

    // check that we got what we expected
    EXPECT_EQ(json.is_object(), true);
    auto root = json.object_items();
    EXPECT_EQ(root.size(), 3);

    EXPECT_EQ(json["k1"].is_string(), true);
    EXPECT_EQ(json["k1"].string_value(), "v1");

    EXPECT_EQ(json["k2"].is_number(), true);
    EXPECT_EQ(json["k2"].int_value(), 42);
    EXPECT_EQ(json["k2"].number_value(), 42.0f);

    EXPECT_EQ(json["k3"].is_array(), true);
    auto items = json["k3"].array_items();
    EXPECT_EQ(items.size(), 5);
    EXPECT_EQ(items[0], Json{"a"});
    EXPECT_EQ(items[1], Json{123});
    EXPECT_EQ(items[2], Json{true});
    EXPECT_EQ(items[3], Json{false});
    EXPECT_EQ(items[4], Json{nullptr});
}

TEST(json11, fancy_constructors) {
    std::list<int>   a { 1, 2, 3 };
    std::vector<int> b { 1, 2, 3 };
    std::set<int>    c { 1, 2, 3 };

    EXPECT_EQ(Json(a), Json(b));
    EXPECT_EQ(Json(b), Json(c));
    EXPECT_EQ(Json(c), Json(a));

    std::map<string, string> m1 {
        { "k1", "v1" },
        { "k2", "v2" }
    };

    std::unordered_map<string, string> m2 {
        { "k1", "v1" },
        { "k2", "v2" }
    };
    EXPECT_EQ(Json(m1), Json(m2));
}

TEST(json11, implicit_serialize) {
    class Point {
    public:
        int x;
        int y;
        Point (int x, int y) : x(x), y(y) {}
        Json to_json() const { return Json::array { x, y }; }
    };

    std::vector<Point> points = {
        { 1, 2 },
        { 10, 20 },
        { 100, 200 }
    };

    Json expected_rep = Json::array {
        Json::array {1, 2},
        Json::array {10, 20},
        Json::array {100, 200}
    };
    EXPECT_EQ(Json(points), expected_rep);
}

