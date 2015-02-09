#include "transaction.hpp"
#include <sqlite3/sqlite3.h>

namespace mx3 { namespace sqlite {

TransactionStmts::TransactionStmts(const shared_ptr<Db>& db)
    : m_begin { db->prepare("BEGIN") }
    , m_read_select { db->prepare("SELECT 1 FROM sqlite_master LIMIT 1") }
    , m_commit { db->prepare("COMMIT") }
    , m_rollback { db->prepare("ROLLBACK") }
{}

void TransactionStmts::begin() { m_begin->exec(); }
void TransactionStmts::begin_read() { m_begin->exec(); m_read_select->exec_scalar(); }
void TransactionStmts::commit() { m_commit->exec(); }
void TransactionStmts::rollback() { m_rollback->exec(); }

namespace detail {

TransactionGuard::TransactionGuard(TransactionStmts& stmts, bool is_read_trans)
    : m_state{State::NONE}, m_stmts{stmts}
{
    if (is_read_trans) {
        m_stmts.begin_read();
    } else {
        m_stmts.begin();
    }
}

TransactionGuard::~TransactionGuard() {
    if (m_state == State::NONE) {
        m_state = State::ROLLBACK;
        m_stmts.rollback();
    }
}

void TransactionGuard::commit() {
    auto prev_state = State::COMMIT;
    std::swap(m_state, prev_state);
    if (prev_state != State::NONE) {
        throw std::runtime_error {"TransactionGuard usage error: only call commit/rollback once"};
    }
    m_stmts.commit();
}

void TransactionGuard::rollback() {
    auto prev_state = State::ROLLBACK;
    std::swap(m_state, prev_state);
    if (prev_state != State::NONE) {
        throw std::runtime_error {"TransactionGuard usage error: only call commit/rollback once"};
    }
    m_stmts.rollback();
}

} // end namespace detail

ReadTransaction::ReadTransaction(TransactionStmts& t_data)
    : detail::TransactionGuard(t_data, /* is_read_trans */ true) {}

WriteTransaction::WriteTransaction(TransactionStmts& t_data)
    : detail::TransactionGuard(t_data, /* is_read_trans */ false) {}

} } // end namespace mx3::sqlite
