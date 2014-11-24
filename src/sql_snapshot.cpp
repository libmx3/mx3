#include "sql_snapshot.hpp"
#include "sqlite/db.hpp"
using mx3::SqlSnapshot;
using std::string;

SqlSnapshot::SqlSnapshot(mx3::sqlite::Query& query) {
    while (query.has_next()) {
        m_data.emplace_back(query.get_string(0));
        query.next();
    }
}

const string&
SqlSnapshot::at(size_t index) {
    return m_data[index];
}
