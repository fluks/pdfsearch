#include "statement.h"

Pdfsearch::Statement::Statement(const Pdfsearch::Database& db,
        const std::string& sql) {
    int result = sqlite3_prepare_v2(db.db, sql.c_str(), sql.size(), &statement,
        nullptr);
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errstr(result));
}

Pdfsearch::Statement::~Statement() {
    try {
        finalize();
    }
    catch (DatabaseError& e) {
    }
}

void
Pdfsearch::Statement::step() const {
    int result = sqlite3_step(statement);
    if (result != SQLITE_DONE)
        throw DatabaseError(result, sqlite3_errstr(result));
}

void
Pdfsearch::Statement::reset() const {
    int result = sqlite3_reset(statement);
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errstr(result));
}

void
Pdfsearch::Statement::finalize() {
    int result = sqlite3_finalize(statement);
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errstr(result));
    statement = nullptr;
}
