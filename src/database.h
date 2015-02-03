#ifndef DATABASE_H
    #define DATABASE_H

#include <sqlite3.h>
#include <string>

namespace Pdfsearch {
    /** A database class. */
    class Database {
    private:
        std::string file;
        sqlite3* db;
        constexpr static const char* TABLE_NAME = u8"pdfs";
        constexpr static const char* ID_COL = u8"id";
        constexpr static const char* FILE_COL = u8"file";
        constexpr static const char* LAST_MODIFIED_COL = u8"last_modified";
        constexpr static const char* PLAIN_TEXT_COL = u8"plain_text";
        constexpr static const char* UPDATED_COL = u8"updated";
        /* A master table name each SQLite database has. */
        constexpr static const char* MASTER_TABLE = u8"sqlite_master";
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
        /** Create table.
         * @throws A Database::Error if can't create table.
         */
        void createTable() const;
        /** Check that is table created.
         * @return True if table exists, false otherwise.
         * @throws A Database::Error if can't query database.
         */
        bool tableExists() const;
        /** Vacuum the database.
         * @see http://www.sqlite.org/lang_vacuum.html
         * @throws A Database::Error if can't vacuum the database.
         */
        void vacuum() const;
        void update() const;
    };
}

#endif /* DATABASE_H */
