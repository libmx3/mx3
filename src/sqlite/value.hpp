#pragma once
#include "stl.hpp"
#include <iosfwd>

namespace mx3 { namespace sqlite {

class Value final {
  public:
    Value();
    Value(std::nullptr_t x);
    Value(int x);
    Value(int64_t x);
    Value(double x);
    Value(const char * x);
    Value(string x);
    Value(vector<uint8_t> x);
    Value(Value&& other);
    Value(const Value& other);
    ~Value();
    Value& operator=(const Value& other);
    Value& operator=(Value&& other);
    bool operator==(const Value& other) const;

    enum class Type {
        // Do not change the orders of these here, since they dictate ordering.
        NUL,
        INT,
        DOUBLE,
        STRING,
        BLOB
    };
    Value::Type type() const { return m_type; }
    bool is_null() const { return m_type == Type::NUL; }
    bool is_numeric() const { return m_type == Type::INT || m_type == Type::DOUBLE; }

    string move_string();
    const string& string_value() const;

    vector<uint8_t> move_blob();
    const vector<uint8_t>& blob_value() const;

    int int_value() const;
    int64_t int64_value() const;
    double double_value() const;
  private:
    Value::Type m_type;
    union {
        int64_t m_int64;
        double m_double;
        string m_string;
        vector<uint8_t> m_blob;
    };
};

using Row = vector<Value>;

// a comparison operator that also compares double and int values correctly
bool operator<(const Value& l, const Value& r);
std::ostream& operator <<(std::ostream& os, const mx3::sqlite::Value& v);
std::ostream& operator <<(std::ostream& os, const vector<mx3::sqlite::Value>& v);
std::ostream& operator <<(std::ostream& os, Value::Type t);

} } // end mx3::sqlite


