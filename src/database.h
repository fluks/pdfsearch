#ifndef DATABASE_H
    #define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/filesystem.hpp>
#include "statement.h"
#include "pdf.h"

namespace Pdfsearch {
    class Statement;

    /** Return type for Database#query(const std::string&, bool, int) const. */
    struct QueryResult {
        std::string file;
        /** Surrounding string of a match. */
        std::string chunk;
        /** Page number of a match. */
        int page;
        /** Number of pages in matching pdf. */
        int pages;
    };

    /** A database class.
     * Example usage:
     * @code
       try {
           Pdfsearch::Database db(file);
           if (!db.databaseCreated())
               db.createDatabase();

           db.index(directories, recursionDepth);

           auto results(db.query(text, verbose, matches));
           // ...
       }
       catch (Pdfsearch::DatabaseError& e) {
           // ...
       @endcode
     * @note The class is non-copyable.
     */
    class Database {
        friend class Statement;
    public:
        /** Keys to stmt_map returned by private function
         * initStatements(stmt_map&).
         */
        enum class statement_key { IS_PDF_IN_DB, INSERT_PDF, INSERT_PAGE,
            DELETE_PAGES, UPDATE_PDF, GET_ALL_PDFS1, GET_ALL_PDFS2,
            DELETE_PDF };

        /** Return value of a private funtion initStatements(stmt_map&). */
        typedef std::map<enum statement_key, std::unique_ptr<Statement>> stmt_map;
    private:
        std::string file;
        sqlite3* db;

        void
        initStatements(stmt_map& statements) const;

        void
        iterateDirectory(const boost::filesystem::path& p, int depth,
            const int MAX_DEPTH, const stmt_map& statements) const;

        void
        insertPdf(const boost::filesystem::path& p,
            const stmt_map& statements) const;

        void
        begin() const;

        void
        commit() const;

        void
        rollback() const;

        void
        execute(const std::string& sql) const;
    public:
        /** Default constructor. */
        Database() : db(nullptr) {};
        /** Construct instance and open database.
         * @param file Filepath to database.
         * @throws A DatabaseError if can't open database.
         */
        Database(const std::string& file);
        /** Non-copyable. */
        Database(const Database& other) = delete;
        /** Non-copyable. */
        Database& operator=(const Database& other) = delete;
        /** Non-copyable. */
        Database(Database&& other) = delete;
        /** Non-copyable. */
        Database& operator=(Database&& other) = delete;
        /** Destructor.
         * Closes the database.
         */
        ~Database();
        /** Open database.
         * Database filepath needs to be initialized in the constructor
         * already.
         * @throws A DatabaseError if can't open database.
         */
        void
        open();
        /** Open database.
         * @param file Filepath to database.
         * @throws A DatabaseError if can't open database.
         */
        void
        open(const std::string& file);
        /** Close database.
         * @throws A DatabaseError if can't close database.
         */
        void
        close();
        /** Create database.
         * @throws A DatabaseError if can't create database.
         */
        void
        createDatabase() const;
        /** Check that database is created.
         * @return True if database exists, false otherwise.
         * @throws A DatabaseError if can't query database.
         */
        bool
        databaseCreated() const;
        /** Vacuum the database.
         * @throws A DatabaseError if can't vacuum the database.
         * @see http://www.sqlite.org/lang_vacuum.html
         */
        void
        vacuum() const;
        /** Update the database.
         * If a pdf isn't on the filesystem anymore, it's removed from the
         * database. If pdf is newer on the filesystem than in the database,
         * pdf in the database is updated.
         */
        void
        update() const;
        /** Index pdfs.
         * Find pdfs on the filesystem and insert them to the database.
         * @param directories Directories where to look for pdfs.
         * @param MAX_DEPTH A maximum depth to recurse in a directory.
         * Options::RECURSE_INFINITELY to recurse indefinitely, 0 to
         * not to recurse at all.
         */
        void
        index(const std::vector<std::string>& directories,
            const int MAX_DEPTH) const;
        /** Find text from pdfs.
         * @param query The phrase to search.
         * @param verbose If false, only QueryResult::file member is set in the
         * return value, otherwise all members are set.
         * @param matches Limit the number of matches.
         * Options::UNLIMITED_MATCHES to return all matches.
         * @return Information about matching pages.
         */
        std::vector<QueryResult>
        query(const std::string& query, bool verbose, int matches) const;
    };
}

#endif /* DATABASE_H */
