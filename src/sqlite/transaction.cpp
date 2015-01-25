#include "transaction.hpp"
#include <sqlite3/sqlite3.h>

namespace mx3 { namespace sqlite {

TransactionStmts::TransactionStmts(const shared_ptr<Db>& db)
    : m_begin { db->prepare("BEGIN") }
    , m_commit { db->prepare("COMMIT") }
    , m_rollback { db->prepare("ROLLBACK") }
{}

void TransactionStmts::begin() { m_begin->exec(); }
void TransactionStmts::commit() { m_commit->exec(); }
void TransactionStmts::rollback() { m_rollback->exec(); }

TransactionGuard::TransactionGuard(TransactionStmts& stmts) : m_state{State::NONE}, m_stmts{stmts} {
    m_stmts.begin();
}

TransactionGuard::~TransactionGuard() {
    if (m_state == State::NONE) {
        m_state = State::ROLLBACK;
        m_stmts.rollback();
    }
}

void TransactionGuard::commit() {
    if (m_state != State::NONE) {
        throw std::runtime_error {"TransactionGuard usage error: only call commit/rollback once"};
    }
    m_state = State::COMMIT;
    m_stmts.commit();
}

void TransactionGuard::rollback() {
    if (m_state != State::NONE) {
        throw std::runtime_error {"TransactionGuard usage error: only call commit/rollback once"};
    }
    m_state = State::ROLLBACK;
    m_stmts.rollback();
}

} } // end namespace mx3::sqlite
