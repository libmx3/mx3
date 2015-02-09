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
    // Begins a read transation for a WAL-mode database (issues both a BEGIN and a single select).
    void begin_read();
    void commit();
    void rollback();
private:
    const shared_ptr<Stmt> m_begin;
    // A read transaction needs to issue a SELECT statement to begin.
    const shared_ptr<Stmt> m_read_select;
    const shared_ptr<Stmt> m_commit;
    const shared_ptr<Stmt> m_rollback;
};

namespace detail {

/* This class is a RAII wrapper for a sqlite transaction (similar to std::lock_guard).
 * This class is not meant to be used directly, see `WriteTransaction` and `ReadTransaction`.
 *
 * Creating a TransactionGuard automatically begins a transaction.  That transaction
 * must be explicilty comitted.  If neither rollback or commit is called (or an exception
 * is throw), the transaction will be rolled back.
 */
class TransactionGuard {
public:
    TransactionGuard(TransactionStmts& t_data, bool is_read_trans);
    TransactionGuard(const TransactionGuard& other) = delete;
    TransactionGuard& operator=(const TransactionGuard& other) = delete;

    // Explicitly not virtual.  ReadTransaction and WriteTransaction are the only subclasses.
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

} // end namesapce detail

class WriteTransaction final : public detail::TransactionGuard {
public:
    WriteTransaction(TransactionStmts& t_data);
};

/* Establish a read transaction on a database.  This class issues both a BEGIN, and a single
 * SELECT statement to ensure the read transaction isn't deferred.
 *
 * This class is only useful for WAL mode databases.
 */
class ReadTransaction final : public detail::TransactionGuard {
public:
    ReadTransaction(TransactionStmts& t_data);
};

} }
