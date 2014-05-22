#include "sql_snapshot.hpp"
using mx3::SqlSnapshot;
using std::string;

SqlSnapshot::SqlSnapshot(CppSQLite3Query& query) {
    while (!query.eof()) {
        m_data.emplace_back(query.getStringField(0));
        query.nextRow();
    }
}

const string&
SqlSnapshot::at(size_t index) {
    return m_data[index];
}
