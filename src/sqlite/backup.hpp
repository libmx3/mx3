#pragma once
#include "stl.hpp"
#include "db.hpp"

struct sqlite3_backup;

namespace mx3 { namespace sqlite {

shared_ptr<Db> backup_to_memory(const shared_ptr<Db>& source);

class Backup final {
public:
    Backup(const shared_ptr<Db>& dest, const shared_ptr<Db>& source);
    Backup(const shared_ptr<Db>& dest,
           const string& dest_name,
           const shared_ptr<Db>& source,
           const string& source_name);
    void step(int n_pages);
    bool is_done();
    void finish();
    int remaining();
    int pagecount();
private:
    struct Closer final {
        void operator() (sqlite3_backup * backup) const;
    };
    unique_ptr<sqlite3_backup, Closer> m_backup;
    bool m_is_done;
};

} } // end namespace mx3::sqlite
