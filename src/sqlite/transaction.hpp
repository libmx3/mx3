#pragma once
#include <set>
#include "stl.hpp"
#include "stmt.hpp"
#include "db.hpp"

namespace mx3 { namespace sqlite {

/* This class holds prepared statements used for transactions.
 *
 * You can reuse this class with a `TransactionGuard` to prevent re-compiling transaction
 * statements every time you need to use them.
 */
class TransactionStmts final {
public:
    TransactionStmts(const shared_ptr<Db>& db);
    TransactionStmts(const TransactionStmts& other) = delete;
    TransactionStmts& operator=(const TransactionStmts& other) = delete;
    void begin();
    void commit();
    void rollback();
private:
    const shared_ptr<Stmt> m_begin;
    const shared_ptr<Stmt> m_commit;
    const shared_ptr<Stmt> m_rollback;
};

/* This class is a RAII wrapper for a sqlite transaction (similar to std::lock_guard)
 *
 * Creating a TransactionGuard automatically begins a transaction.  That transaction
 * must be explicilty comitted.  If neither rollback or commit is called (or an exception
 * is throw), the transaction will be rolled back.
 */
class TransactionGuard final {
public:
    TransactionGuard(TransactionStmts& t_data);
    TransactionGuard(const TransactionGuard& other) = delete;
    TransactionGuard& operator=(const TransactionGuard& other) = delete;
    ~TransactionGuard();
    void commit();
    void rollback();
private:
    enum class State {
        NONE,
        COMMIT,
        ROLLBACK
    };
    State m_state;
    TransactionStmts& m_stmts;
};

} }
