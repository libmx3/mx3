#include "value.hpp"
#include <ostream>

using mx3::sqlite::Value;

namespace {
    Value::Type s_affinity_type(Value::Type t) {
        switch (t) {
            case Value::Type::NUL:
            case Value::Type::STRING:
            case Value::Type::BLOB:
                return t;
            case Value::Type::INT:
            case Value::Type::DOUBLE:
                return Value::Type::INT;
        }
        // need to add this to make android compiler happy
        return t;
    }
}

Value::Value(const Value& other) : m_type {other.m_type} {
    switch (other.m_type) {
        case Type::NUL:
            break;
        case Type::STRING:
            new (&m_string) string {other.m_string};
            break;
        case Type::BLOB:
            new (&m_blob) vector<uint8_t> {other.m_blob};
            break;
        case Type::INT:
            m_int64 = other.m_int64;
            break;
        case Type::DOUBLE:
            m_double = other.m_double;
            break;
    }
}

Value&
Value::operator=(Value&& other) {
    this->~Value();
    m_type = other.m_type;
    switch (m_type) {
        case Type::NUL:
            break;
        case Type::STRING:
            new (&m_string) string {std::move(other.m_string)};
            break;
        case Type::BLOB:
            new (&m_blob) vector<uint8_t> {std::move(other.m_blob)};
            break;
        case Type::INT:
            m_int64 = other.m_int64;
            break;
        case Type::DOUBLE:
            m_double = other.m_double;
            break;
    }
    return *this;
}

// set m_type to null, so that when the dtor runs, it does nothing
Value::Value(Value&& other) : m_type {Type::NUL} {
    *this = std::move(other);
}

Value&
Value::operator=(const Value& other) {
    if (this != &other) {
        *this = Value {other};
    }
    return *this;
}

bool
Value::operator==(const Value& other) const {
    switch (m_type) {
        case Type::NUL:
            return other.m_type == Type::NUL;
        case Type::STRING:
            return other.m_type == Type::STRING && m_string == other.m_string;
        case Type::BLOB:
            return other.m_type == Type::BLOB && m_blob == other.m_blob;
        case Type::INT:
            return (other.m_type == Type::INT    && m_int64 == other.m_int64) ||
                   (other.m_type == Type::DOUBLE && m_int64 == other.m_double);
        case Type::DOUBLE:
            return (other.m_type == Type::DOUBLE && m_double == other.m_double) ||
                   (other.m_type == Type::INT    && m_double == other.m_int64);
    }
    // all cases handled above, but the android compiler complains about this
    __builtin_unreachable();
}

Value::Value()                  : m_type {Type::NUL} {}
Value::Value(std::nullptr_t)    : m_type {Type::NUL} {}
Value::Value(int64_t x)         : m_type {Type::INT},    m_int64  {x} {}
Value::Value(double x)          : m_type {Type::DOUBLE}, m_double {x} {}
Value::Value(string x)          : m_type {Type::STRING}  { new (&m_string) string {std::move(x)}; }
Value::Value(vector<uint8_t> x) : m_type {Type::BLOB}    { new (&m_blob) vector<uint8_t> {std::move(x)}; }

Value::~Value() {
    switch (m_type) {
        case Type::STRING:
            m_string.~string();
            break;
        case Type::BLOB:
            m_blob.~vector<uint8_t>();
            break;
        case Type::NUL:
        case Type::INT:
        case Type::DOUBLE:
            // do nothing
            break;
    }
}

string
Value::move_string() {
    if (m_type != Type::STRING) {
        throw std::runtime_error {"invalid type for column, string"};
    }
    string replacement;
    std::swap(m_string, replacement);
    return replacement;
}

string
Value::string_value() && {
    return this->move_string();
}

vector<uint8_t>
Value::move_blob() {
    if (m_type != Type::BLOB) {
        throw std::runtime_error {"invalid type for column, blob"};
    }
    vector<uint8_t> replacement;
    std::swap(replacement, m_blob);
    return replacement;
}

vector<uint8_t>
Value::blob_value() && {
    return this->move_blob();
}

int
Value::int_value() const {
    return static_cast<int>( this->int64_value() );
}

int64_t
Value::int64_value() const {
    if (m_type == Type::INT) {
        return m_int64;
    } else if (m_type == Type::DOUBLE) {
        return static_cast<int64_t>( m_double );
    } else {
        throw std::runtime_error {"invalid type for column, int"};
    }
}

double
Value::double_value() const {
    if (m_type == Type::DOUBLE) {
        return m_double;
    } else if (m_type == Type::INT) {
        return static_cast<double>(m_int64);
    } else {
        throw std::runtime_error {"invalid type for column, double"};
    }
}

const string&
Value::string_value() const& {
    if (m_type != Type::STRING) {
        throw std::runtime_error {"invalid type for column, string"};
    }
    return m_string;
}

const vector<uint8_t>&
Value::blob_value() const& {
    if (m_type != Type::BLOB) {
        throw std::runtime_error {"invalid type for column, blob"};
    }
    return m_blob;
}

bool
mx3::sqlite::operator<(const Value& l, const Value& r) {
    const auto l_type = l.type();
    const auto r_type = r.type();
    const auto l_aff_type = s_affinity_type( l_type );
    const auto r_aff_type = s_affinity_type( r_type );

    return l_aff_type < r_aff_type ||
        (l_aff_type == r_aff_type && l_type != Value::Type::NUL &&
            (
                (l_type == Value::Type::STRING && l.string_value() < r.string_value()) ||
                (l_type == Value::Type::BLOB   && l.blob_value()   < r.blob_value()  ) ||
                (l_type == r_type && r_type == Value::Type::INT    && l.int64_value()  < r.int64_value() ) ||
                (l_type == r_type && r_type == Value::Type::DOUBLE && l.double_value() < r.double_value()) ||
                (l_type == Value::Type::INT    && r_type == Value::Type::DOUBLE && l.int64_value() < r.double_value()) ||
                (l_type == Value::Type::DOUBLE && r_type == Value::Type::INT && l.double_value() < r.int64_value())
            )
        );
}

std::ostream&
mx3::sqlite::operator <<(std::ostream& os, const Value& v) {
    switch (v.type()) {
        case Value::Type::INT:
            return os << v.int64_value();
        case Value::Type::DOUBLE:
            return os << v.double_value();
        case Value::Type::STRING:
            return os << v.string_value();
        case Value::Type::BLOB:
            return os << "<BLOB " << v.blob_value().size() << " bytes>";
        case Value::Type::NUL:
        default:
            break;
    }
    return os << "null";
}
