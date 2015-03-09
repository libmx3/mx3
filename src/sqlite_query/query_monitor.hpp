#pragma once
#include "../sqlite/sqlite.hpp"

namespace mx3 {
namespace sqlite {

/* This class listens to changes in WAL mode databases.
 * And will notify listeners when changes are made.
 */
class QueryMonitor final {
public:
    static shared_ptr<QueryMonitor> create_shared(const shared_ptr<Db> & write_db);
    ~QueryMonitor();
    void listen_to_changes(const function<void()> & fn);
private:
    class only_for_internal_use_by_make_shared_t;
public:
    // make_shared constructor
    QueryMonitor(only_for_internal_use_by_make_shared_t flag, const shared_ptr<Db> & write_db);
private:
    void _install_hooks();
    void _on_update(Db::Change);
    bool _on_commit();
    void _on_rollback();
    bool _on_wal_commit(const string& db_name, int pages);
    const shared_ptr<Db> m_write_db;
    vector<function<void()>> m_listeners;
};

} }
