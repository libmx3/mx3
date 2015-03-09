#include "query_monitor.hpp"

namespace mx3 {
namespace sqlite {

class QueryMonitor::only_for_internal_use_by_make_shared_t {};

shared_ptr<QueryMonitor> QueryMonitor::create_shared(const shared_ptr<Db> & write_db) {
    return make_shared<QueryMonitor>(only_for_internal_use_by_make_shared_t{}, write_db);
}

QueryMonitor::QueryMonitor(
        only_for_internal_use_by_make_shared_t,
        const shared_ptr<Db> & write_db) : m_write_db(write_db)
{
    _install_hooks();
}


QueryMonitor::~QueryMonitor() {
    m_write_db->update_hook(nullptr);
    m_write_db->commit_hook(nullptr);
    m_write_db->rollback_hook(nullptr);
    m_write_db->wal_hook(nullptr);
}

void QueryMonitor::listen_to_changes(const function<void()> & fn) {
    m_listeners.push_back(fn);
}

void QueryMonitor::_install_hooks() {
    m_write_db->update_hook([this] (Db::Change change) {
        this->_on_update(std::move(change));
    });
    m_write_db->commit_hook([this] () {
        return this->_on_commit();
    });
    m_write_db->rollback_hook([this] () {
        this->_on_rollback();
    });
    m_write_db->wal_hook([this] (const string& db_name, int pages) {
        this->_on_wal_commit(db_name, pages);
    });
}


void QueryMonitor::_on_update(Db::Change) {}
bool QueryMonitor::_on_commit() {
    return true;
}
void QueryMonitor::_on_rollback() {}
bool QueryMonitor::_on_wal_commit(const string& db_name, int pages) {
    if (pages > 1000) {
        m_write_db->wal_checkpoint_v2(db_name, Checkpoint::PASSIVE);
    }
    // Grab size before iterating in case m_listeners is modified in a callback.
    // In actuality this should be posted to an event loop, but oh well  - its only a demo app
    const size_t total = m_listeners.size();
    for (size_t i=0; i < total; i++) {
        m_listeners[i]();
    }
    return true;
}

} }
