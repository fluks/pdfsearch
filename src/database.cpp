#include <cassert>
#include <sstream>
#include <iostream>
#include <memory>
#include <boost/regex.hpp>
#include <poppler-document.h>
#include <poppler-page.h>
#include "database.h"
#include "database_error.h"
#include "options.h"

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
Pdfsearch::Database::createDatabase() const {
    assert(db != nullptr);

    const char* sql =
        u8"create table Pdfs"
            u8"(id            integer primary key asc,"
            u8" file          text unique not null,"
            u8" last_modified int not null);"

        u8"create table Plain_texts"
            u8"(plain_text    text default '',"
            u8" page          int not null,"
            u8" pdfs_id       integer not null references Pdfs(id)"
            u8"                   on delete cascade);"

        u8"create index last_modified_index on Pdfs(last_modified);"
        u8"create index pdfs_id_index       on Plain_texts(pdfs_id);";

    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
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
Pdfsearch::Database::databaseCreated() const {
    assert(db != nullptr);

    const char* sql =
        u8"select 1 from sqlite_master "
            u8"where type='table' and name='Pdfs';";

    int rows = 0;
    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql, numberOfRowsCb, &rows, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }

    return rows > 0;
}

void
Pdfsearch::Database::vacuum() const {
    execute("vacuum;");
}

void
Pdfsearch::Database::update() const {
    assert(db != nullptr);

    const char* sql = "select * from Pdfs;";

    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
}

void
Pdfsearch::Database::query(const std::string& query, bool verbose, int matches)
        const {
    assert(db != nullptr);
}

bool
isPdf(const boost::filesystem::path& p) {
    static const boost::regex pattern(".*\\.pdf$", boost::regex::icase);
    return boost::regex_match(p.filename().generic_string(), pattern);
}

std::map<enum Pdfsearch::Database::statement_key, const Pdfsearch::Statement&>
Pdfsearch::Database::initStatements() const {
    return std::map<enum statement_key, const Statement&>({
        { statement_key::IS_PDF_IN_DB,
            Statement(*this, "select id, last_modified from Pdfs where file = ?1;") },
        { statement_key::INSERT_PDF,
            Statement(*this, "insert into Pdfs(file, last_modified) values(?1, ?2);") },
        { statement_key::INSERT_PAGE,
            Statement(*this, "insert into Plain_texts(plain_text, page, pdfs_id)"
                                  "values(?1, ?2, (select id from Pdfs where file = ?3));") },
        { statement_key::DELETE_PAGES,
            Statement(*this, "delete from Plain_texts where pdfs_id = ?1;") },
        { statement_key::UPDATE_PDF,
            Statement(*this, "update Pdfs set last_modified = ?1 where id = ?2;") } });
}

void
Pdfsearch::Database::index(const std::vector<std::string>& directories,
        const int MAX_DEPTH) const {
    assert(db != nullptr);

    begin();

    //std::map<enum statement_key, const Statement&> statements(initStatements());
    std::map<enum statement_key, const Statement&> statements({
        { statement_key::IS_PDF_IN_DB,
            Statement(*this, "select id, last_modified from Pdfs where file = ?1;") },
        { statement_key::INSERT_PDF,
            Statement(*this, "insert into Pdfs(file, last_modified) values(?1, ?2);") },
        { statement_key::INSERT_PAGE,
            Statement(*this, "insert into Plain_texts(plain_text, page, pdfs_id)"
                                  "values(?1, ?2, (select id from Pdfs where file = ?3));") },
        { statement_key::DELETE_PAGES,
            Statement(*this, "delete from Plain_texts where pdfs_id = ?1;") },
        { statement_key::UPDATE_PDF,
            Statement(*this, "update Pdfs set last_modified = ?1 where id = ?2;") } });


    for (const auto& d : directories) {
        try {
            int depth = 0;
            iterateDirectory(d, depth, MAX_DEPTH, statements);
        }
        catch (boost::filesystem::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    commit();
}

void
Pdfsearch::Database::insertPdf(const boost::filesystem::path& p,
    const std::map<enum Pdfsearch::Database::statement_key, const Pdfsearch::Statement&>& statements
        ) const {
    std::unique_ptr<poppler::document> doc(
        poppler::document::load_from_file(p.native()));
    if (doc == nullptr) {
        std::cerr << "file: " << boost::filesystem::canonical(p).native() <<
            " is not a pdf file" << std::endl;
        return;
    }

    const auto& s1 = statements.at(statement_key::IS_PDF_IN_DB);
    s1.bind(boost::filesystem::canonical(p).native(), 1);

    std::unique_ptr<int> last_modified;
    for (auto it = s1.begin(); it != s1.end(); it++)
        last_modified = std::move(it.column<int>(0));

    if (last_modified == nullptr) {
        const auto& s2 = statements.at(statement_key::INSERT_PDF);
        s2.bind(boost::filesystem::canonical(p).native(), 1);
        s2.bind(static_cast<int>(boost::filesystem::last_write_time(p)), 2);
        s2.step();

        const auto& s3 = statements.at(statement_key::INSERT_PAGE);
        for (int i = 0; i < doc->pages(); i++) {
            std::unique_ptr<poppler::page> page(doc->create_page(i));
            std::vector<char> chars(page->text().to_utf8());
            std::string text(chars.begin(), chars.end());

            s3.bind(text, 1);
            s3.bind(i + 1, 2);
            s3.step();
            s3.reset();
        }
    }
}

void
Pdfsearch::Database::iterateDirectory(const boost::filesystem::path& p,
    int depth, const int MAX_DEPTH,
    const std::map<enum Pdfsearch::Database::statement_key, const Pdfsearch::Statement&>& statements
        ) const {
    using namespace boost::filesystem;

    auto end = directory_iterator();
    for (auto it = directory_iterator(p); it != end; ++it) {
        try {
            if (is_directory(it->path()) && !is_symlink(it->path()) &&
                    (depth == Options::RECURSE_INFINITELY || depth < MAX_DEPTH))
                iterateDirectory(it->path(), ++depth, MAX_DEPTH, statements);
            else if (is_regular_file(it->path()) && !is_symlink(it->path()) &&
                    isPdf(it->path()))
                insertPdf(it->path(), statements);
        }
        catch (filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void
Pdfsearch::Database::execute(const std::string& sql) const {
    assert(db != nullptr);

    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
}

void
Pdfsearch::Database::begin() const {
    execute("begin;");
}

void
Pdfsearch::Database::commit() const {
    execute("commit;");
}

void
Pdfsearch::Database::rollback() const {
    execute("rollback;");
}
