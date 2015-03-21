#include <cassert>
#include <iostream>
#include <boost/regex.hpp>
#include "database.h"
#include "database_error.h"
#include "options.h"

static void
insertPages(const Pdfsearch::Pdf& doc, const Pdfsearch::Statement& s);

static int
numberOfRowsCb(void* rows, int columns, char** result, char** columnName);

Pdfsearch::Database::Database(const std::string& file) :
    file(file),
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
Pdfsearch::Database::open(const std::string& file) {
    this->file = file;
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

        u8"create table PlainTexts"
            u8"(plain_text    text default '',"
            u8" page          int not null,"
            u8" pdfs_id       integer not null references Pdfs(id)"
            u8"                   on delete cascade);"

        u8"create index last_modified_index on Pdfs(last_modified);"
        u8"create index pdfs_id_index       on PlainTexts(pdfs_id);";

    char* errmsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::string error(errmsg);
        sqlite3_free(errmsg);
        throw DatabaseError(error);
    }
}

static int
numberOfRowsCb(void* rows, int, char**, char**) {
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

static void
insertPages(const Pdfsearch::Pdf& doc, const Pdfsearch::Statement& s) {
    auto file(doc.getFile());
    for (int i = 0; i < doc.numberOfPages(); i++) {
        auto text(doc.getPage(i));

        s.bind(*text, 1);
        s.bind(i + 1, 2);
        s.bind(file, 3);
        s.step();
        s.reset();
    }
}

void
Pdfsearch::Database::update() const {
    assert(db != nullptr);

    begin();

    stmt_map statements;
    initStatements(statements);

    const auto& getAllPdfs = statements.at(statement_key::GET_ALL_PDFS1).get();
    const auto& deletePdf = statements.at(statement_key::DELETE_PDF).get();
    const auto& deletePages = statements.at(statement_key::DELETE_PAGES).get();
    const auto& insertPage = statements.at(statement_key::INSERT_PAGE).get();
    const auto& updatePdf = statements.at(statement_key::UPDATE_PDF).get();
    for (auto it = getAllPdfs->begin(); it != getAllPdfs->end(); it++) {
        try {
            const auto& id(it.column<int>(0));
            const auto& file(it.column<std::string>(1));
            const auto& lastModified(it.column<sqlite3_int64>(2));

            const boost::filesystem::path p(*file);
            if (boost::filesystem::exists(p)) {
                const auto& newLastModified =
                    boost::filesystem::last_write_time(p);
                if (newLastModified > *lastModified) {
                    updatePdf->bind<sqlite3_int64>(newLastModified, 1);
                    updatePdf->bind(*id, 2);
                    updatePdf->step();
                    updatePdf->reset();

                    deletePages->bind(*id, 1);
                    deletePages->step();
                    deletePages->reset();

                    Pdf doc(*file);
                    insertPages(doc, *insertPage);
                }
            }
            else {
                deletePdf->bind(*id, 1);
                deletePdf->step();
                deletePdf->reset();
            }
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    commit();
}

std::vector<Pdfsearch::QueryResult>
Pdfsearch::Database::query(const std::string& query, bool verbose, int matches)
        const {
    assert(db != nullptr);

    std::string q("%" + query + "%");

    stmt_map statements;
    initStatements(statements);
    const auto& getAllPdfs = statements.at(statement_key::GET_ALL_PDFS2);
    getAllPdfs->bind(q, 1);

    std::vector<QueryResult> results;

    boost::regex pattern("((?:\\s+\\S+){0,5}\\s*" + query + "\\s*(?:\\S+\\s+){0,5})",
        boost::regex::icase);
    for (auto it = getAllPdfs->begin();
        it != getAllPdfs->end() && (matches == Options::UNLIMITED_MATCHES ||
                          it.getRow() <= matches);
            it++) {
        QueryResult qr;
        qr.file = *(it.column<std::string>(0));

        if (verbose) {
            qr.page = *(it.column<int>(2));
            qr.pages = *(it.column<int>(3));

            boost::smatch m;
            const auto& text(it.column<std::string>(1));
            if (boost::regex_search(text->cbegin(), text->cend(), m, pattern))
                qr.chunk = std::string(m[1].first, m[1].second);
            else
                qr.chunk = query;
        }
        results.push_back(qr);
    }

    return results;
}

void
Pdfsearch::Database::initStatements(Pdfsearch::Database::stmt_map& m) const {
    m.insert(std::make_pair(statement_key::IS_PDF_IN_DB,
       std::unique_ptr<Statement>(new Statement(*this,
       "select id, last_modified from Pdfs where file = ?1;"))));
    m.insert(std::make_pair(statement_key::INSERT_PDF,
       std::unique_ptr<Statement>(new Statement(*this,
       "insert into Pdfs(file, last_modified) values(?1, ?2);"))));
    m.insert(std::make_pair(statement_key::INSERT_PAGE,
       std::unique_ptr<Statement>(new Statement(*this,
       "insert into PlainTexts(plain_text, page, pdfs_id)"
           "values(?1, ?2, (select id from Pdfs where file = ?3));"))));
    m.insert(std::make_pair(statement_key::DELETE_PAGES,
       std::unique_ptr<Statement>(new Statement(*this,
       "delete from PlainTexts where pdfs_id = ?1;"))));
    m.insert(std::make_pair(statement_key::DELETE_PDF,
       std::unique_ptr<Statement>(new Statement(*this,
       "delete from Pdfs where id = ?1;"))));
    m.insert(std::make_pair(statement_key::UPDATE_PDF,
       std::unique_ptr<Statement>(new Statement(*this,
       "update Pdfs set last_modified = ?1 where id = ?2;"))));
    m.insert(std::make_pair(statement_key::GET_ALL_PDFS1,
       std::unique_ptr<Statement>(new Statement(*this,
       "select id, file, last_modified from Pdfs;"))));
    m.insert(std::make_pair(statement_key::GET_ALL_PDFS2,
       std::unique_ptr<Statement>(new Statement(*this,
       "select (select file from Pdfs where id = pdfs_id),"
           " plain_text, page,"
           " (select count(*) from PlainTexts P1"
               " where P1.pdfs_id = P2.pdfs_id) from PlainTexts P2"
               " where plain_text like ?1;"))));
}

void
Pdfsearch::Database::index(const std::vector<std::string>& directories,
        const int MAX_DEPTH) const {
    assert(db != nullptr);

    begin();

    stmt_map statements;
    initStatements(statements);

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
    const Pdfsearch::Database::stmt_map& statements
        ) const {
    auto file(boost::filesystem::canonical(p).native());
    Pdf doc(file);

    const auto& isPdfInDB = statements.at(statement_key::IS_PDF_IN_DB).get();
    isPdfInDB->bind(file, 1);
    std::unique_ptr<sqlite3_int64> lastModified;
    std::unique_ptr<int> id;
    for (auto it = isPdfInDB->begin(); it != isPdfInDB->end(); it++) {
        id = std::move(it.column<int>(0));
        lastModified = std::move(it.column<sqlite3_int64>(1));
    }
    isPdfInDB->reset();

    const auto& newLastModified(boost::filesystem::last_write_time(p));
    const auto& insertPage = statements.at(statement_key::INSERT_PAGE).get();
    if (lastModified == nullptr) {
        const auto& insertPdf = statements.at(statement_key::INSERT_PDF).get();
        insertPdf->bind(file, 1);
        insertPdf->bind<sqlite3_int64>(newLastModified, 2);
        insertPdf->step();
        insertPdf->reset();

        insertPages(doc, *insertPage);
    }
    else if (*lastModified < newLastModified) {
        const auto& deletePages = statements.at(statement_key::DELETE_PAGES).get();
        deletePages->bind(*id, 1);
        deletePages->step();
        deletePages->reset();

        const auto& updatePdf = statements.at(statement_key::UPDATE_PDF).get();
        updatePdf->bind<sqlite3_int64>(newLastModified, 1);
        updatePdf->bind(*id, 2);
        updatePdf->step();
        updatePdf->reset();

        insertPages(doc, *insertPage);
    }
}

void
Pdfsearch::Database::iterateDirectory(const boost::filesystem::path& p,
    int depth, const int MAX_DEPTH,
    const Pdfsearch::Database::stmt_map& statements
        ) const {
    using namespace boost::filesystem;

    auto end = directory_iterator();
    for (auto it = directory_iterator(p); it != end; ++it) {
        try {
            if (is_directory(it->path()) && !is_symlink(it->path()) &&
                    (MAX_DEPTH == Options::RECURSE_INFINITELY || depth < MAX_DEPTH)) {
                iterateDirectory(it->path(), ++depth, MAX_DEPTH, statements);
            }
            else if (is_regular_file(it->path()) && !is_symlink(it->path()) &&
                    Pdf::filenameEndsToPdf(it->path().native())) {
                insertPdf(it->path(), statements);
            }
        }
        catch (std::exception& e) {
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
