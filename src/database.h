#ifndef DATABASE_H
    #define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include "statement.h"

namespace Pdfsearch {
    class Statement;

    /** A database class. */
    class Database {
        friend class Statement;
    public:
        enum class statement_key { IS_PDF_IN_DB, INSERT_PDF, INSERT_PAGE, DELETE_PAGES,
            UPDATE_PDF };
    private:
        std::string file;
        sqlite3* db;

        std::map<enum statement_key, const Statement&> initStatements() const;

        void iterateDirectory(const boost::filesystem::path& p, int depth,
            const int MAX_DEPTH,
            const std::map<enum statement_key, const Statement&>& statemenets) const;

        void insertPdf(const boost::filesystem::path& p,
            const std::map<enum statement_key, const Statement&>& statemenets) const;

        void begin() const;

        void commit() const;

        void rollback() const;

        void execute(const std::string& sql) const;
    public:
        /** Default constructor. */
        Database() : db(nullptr) {};
        /** Construct instance and open database.
         * @param _file Filepath to database.
         * @throws A Database::Error if can't open database.
         */
        Database(std::string _file);
        /** Destructor.
         * Tries to close the database.
         * */
        ~Database();
        /* Open database.
         * Database filepath needs to be initialized in the constructor
         * already.
         * @throws A Database::Error if can't open database.
         */
        void open();
        /* Open database.
         * @param Filepath to database.
         * @throws A Database::Error if can't open database.
         */
        void open(std::string _file);
        /** Close database.
         * @throws A Database::Error if can't close database.
         */
        void close();
        /** Create database.
         * @throws A Database::Error if can't create database.
         */
        void createDatabase() const;
        /** Check that database is created.
         * @return True if database exists, false otherwise.
         * @throws A Database::Error if can't query database.
         */
        bool databaseCreated() const;
        /** Vacuum the database.
         * @see http://www.sqlite.org/lang_vacuum.html
         * @throws A Database::Error if can't vacuum the database.
         */
        void vacuum() const;
        void update() const;
        void index(const std::vector<std::string>& directories,
            const int MAX_DEPTH) const;
        void query(const std::string& query, bool verbose, int matches) const;
    };
}

#endif /* DATABASE_H */
