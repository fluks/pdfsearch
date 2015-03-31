#include "statement.h"

sqlite3_stmt*
Pdfsearch::Statement::allocateStatement(sqlite3* db, const std::string& sql) {
    sqlite3_stmt* statement;
    int result = sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &statement,
        nullptr);
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errstr(result));

    return statement;
}

void
Pdfsearch::Statement::deleteStatement(sqlite3_stmt* statement) {
    sqlite3_finalize(statement);
}

Pdfsearch::Statement::Statement(const Pdfsearch::Database& db,
        const std::string& sql) :
    statement(allocateStatement(db.db, sql), deleteStatement) {
}

void
Pdfsearch::Statement::step() const {
    int result = sqlite3_step(statement.get());
    if (result != SQLITE_DONE)
        throw DatabaseError(result, sqlite3_errstr(result));
}

void
Pdfsearch::Statement::reset() const {
    int result = sqlite3_reset(statement.get());
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errstr(result));
}
