#include "backup.hpp"
#include <sqlite3/sqlite3.h>

namespace mx3 { namespace sqlite {

shared_ptr<Db> backup_to_memory(const shared_ptr<Db>& source_db) {
    const auto mem_db = Db::open_memory();
    Backup b {mem_db, source_db};
    b.step(-1);
    b.finish();
    return mem_db;
}

Backup::Backup(const shared_ptr<Db>& dest, const shared_ptr<Db>& source)
    : Backup(dest, "main", source, "main") {}

Backup::Backup(
        const shared_ptr<Db>& dest,
        const string& dest_name,
        const shared_ptr<Db>& source,
        const string& source_name)
    : m_is_done {false}
{
    const auto backup = sqlite3_backup_init(dest->borrow_db(), dest_name.c_str(),
                                           source->borrow_db(), source_name.c_str());
    if (backup == nullptr) {
        throw std::runtime_error {sqlite3_errmsg(dest->borrow_db())};
    }
    m_backup = unique_ptr<sqlite3_backup, Closer> {backup};
}

void Backup::step(int n_pages) {
    const auto err_code = sqlite3_backup_step(m_backup.get(), n_pages);
    if (err_code != SQLITE_OK && err_code != SQLITE_DONE) {
        throw std::runtime_error {sqlite3_errstr(err_code)};
    }
    m_is_done = (err_code == SQLITE_DONE);
}

bool Backup::is_done() {
    return m_is_done;
}

void Backup::finish() {
    const auto err_code = sqlite3_backup_finish(m_backup.release());
    if (err_code != SQLITE_OK) {
        throw std::runtime_error {sqlite3_errstr(err_code)};
    }
}

int Backup::remaining() {
    return sqlite3_backup_remaining(m_backup.get());
}

int Backup::pagecount() {
    return sqlite3_backup_pagecount(m_backup.get());
}

void Backup::Closer::operator() (sqlite3_backup * backup) const {
    const auto err = sqlite3_backup_finish(backup);
    if (err != SQLITE_OK) {
        // ignore errors in destruction
    }
}

} } // end namespace mx3::sqlite
