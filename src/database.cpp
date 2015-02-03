#include <cassert>
#include <sstream>
#include "database.h"
#include "database_error.h"

Pdfsearch::Database::Database(std::string _file) :
    file(_file),
    db(nullptr) {
    open();
}

Pdfsearch::Database::~Database() {
    /* Can't throw from destructor. */
    try {
        close();
    }
    catch (const DatabaseError& e) {
    }
}

void
Pdfsearch::Database::open() {
    assert(file != nullptr);

    int result = sqlite3_open(file.c_str(), &db);
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errmsg(db));

    char* errmsg = nullptr;
    result = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr,
        &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
}

void
Pdfsearch::Database::open(std::string _file) {
    file = _file;
    open();
}

void
Pdfsearch::Database::close() {
    int result = sqlite3_close(db);
    if (result != SQLITE_OK)
        throw DatabaseError(result, sqlite3_errmsg(db));
    db = nullptr;
}

void
Pdfsearch::Database::createTable() const {
    assert(db != nullptr);

    std::ostringstream sql;
    sql << u8"CREATE TABLE " << TABLE_NAME                 <<
        u8" (" << ID_COL << u8" INTEGER PRIMARY KEY ASC,"  <<
                  FILE_COL <<          u8" TEXT UNIQUE,"   <<
                  LAST_MODIFIED_COL << u8" INT,"           <<
                  PLAIN_TEXT_COL <<    u8" TEXT NOT NULL," <<
                  UPDATED_COL <<       u8" INT DEFAULT 1"  <<
        u8");";
    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql.str().c_str(), nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
}

static int
numberOfRowsCb(void* rows, int columns, char** result, char** columnName) {
    int *r = static_cast<int*>(rows);
    (*r)++;
    return 0;
}

bool
Pdfsearch::Database::tableExists() const {
    assert(db != nullptr);

    std::ostringstream sql;
    sql << u8"SELECT 1 FROM " << MASTER_TABLE <<
        u8" WHERE type='table' AND name='" << TABLE_NAME << "';";
    int rows = 0;
    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql.str().c_str(), numberOfRowsCb, &rows,
        &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }

    return rows > 0;
}

void
Pdfsearch::Database::vacuum() const {
    assert(db != nullptr);

    char* errmsg = nullptr;
    int result = sqlite3_exec(db, "VACUUM;", nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
}

void
Pdfsearch::Database::update() const {
    assert(db != nullptr);

    std::ostringstream sql;
    sql << "SELECT * FROM " << TABLE_NAME << ";";
    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql.str().c_str(), nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
        
}
