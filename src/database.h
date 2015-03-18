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

    /** A database class. */
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
        typedef std::map<enum statement_key, std::unique_ptr<Pdfsearch::Statement>> stmt_map;
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
         * @param _file Filepath to database.
         * @throws A DatabaseError if can't open database.
         */
        Database(std::string _file);
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
         * @param Filepath to database.
         * @throws A DatabaseError if can't open database.
         */
        void
        open(std::string _file);
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
         * @param verbose If false, print only filenames where the phrase is
         * found, otherwise, print more information.
         * @param matches Limit the number of matches.
         * Options::UNLIMITED_MATCHES to return all matches.
         */
        void
        query(const std::string& query, bool verbose, int matches) const;
    };
}

#endif /* DATABASE_H */
