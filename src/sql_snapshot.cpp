#include "sql_snapshot.hpp"
#include "sqlite/db.hpp"
using mx3::SqlSnapshot;
using std::string;

SqlSnapshot::SqlSnapshot(mx3::sqlite::Cursor& cursor) {
    while (cursor.has_next()) {
        m_data.emplace_back(cursor.string_value(0));
        cursor.next();
    }
}

const string&
SqlSnapshot::at(size_t index) {
    return m_data[index];
}
