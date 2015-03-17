#ifndef STATEMENT_H
    #define STATEMENT_H

#include <sqlite3.h>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <memory>
#include "database.h"
#include "database_error.h"
#include "resultrowiterator.h"

namespace Pdfsearch {
    class Database;

    class Statement {
    private:
        // Prepared statement.
        std::shared_ptr<sqlite3_stmt> statement;

        static sqlite3_stmt*
        allocatePointer(const Database& db, const std::string& sql);

        static void
        deletePointer(sqlite3_stmt* statement);
    public:
        /** Constractor.
         * @param db Database handle.
         * @param sql SQL command to prepare.
         * @throws DatabaseError if fails to prepare statement.
         */
        Statement(const Database& db, const std::string& sql);

        /** Copy constractor deleted, always pass by reference. */
        Statement(const Statement& _statement) = delete;

        /** Copy assignment operator deleted, always pass by reference. */
        Statement& operator =(const Statement& statement) = delete;

        /** Move constractor deleted, always pass by reference. */
        Statement(Statement&& _statement) = delete;

        /** Move assignment operator deleted, always pass by reference. */
        Statement& operator =(Statement&& statement) = delete;

        /** Destructor. Finalizes prepared statement. */
        ~Statement();

        /** Bind a value to a parameter.
         * Example:
         * @code
         * stmt.bind(10, 1);             // T is int.
         * stmt.bind<void*>(nullptr, 2); // void* needs template argument.
         * stmt.bind(3.4, 3);            // T is double.
         * stmt.bind("bla", 4);          // T is std::string.
         * @endcode
         * @param value A value to bind.
         * @param column Number of parameter to bind a value to.
         * @throws DatabaseError if can't bind value.
         */
        template<typename T>
        void bind(T value, int column) const;

        /** Get iterator to results.
         * @return ResultRowIterator
         * @throws DatabaseError if fails to get first row.
         */
        ResultRowIterator begin() const {
            return ResultRowIterator::begin(statement);
        };

        /** Get the end of results.
         * @return ResultRowIterator
         */
        ResultRowIterator end() const {
            return ResultRowIterator::end(statement);
        };

        /** Evaluate the statement.
         * @throws DatabaseError if evaluating the statement fails.
         */
        void step() const;

        /** Reset statement.
         * @throws DatabaseError if last call to step() failed.
         */
        void reset() const;

        /** Finalizes statement, use this if you want catch error message.
         * @throws DatabaseError if the most recent evaluation of the statement
         * failed.
         */
        void finalize();
    };

    template<> inline void
    Statement::bind<int>(int value, int column) const {
        int result = sqlite3_bind_int(statement, column, value);
        if (result != SQLITE_OK)
            throw DatabaseError(result, sqlite3_errstr(result));
    }

    template<> inline void
    Statement::bind<double>(double value, int column) const {
        int result = sqlite3_bind_double(statement, column, value);
        if (result != SQLITE_OK)
            throw DatabaseError(result, sqlite3_errstr(result));
    }

    template<> inline void
    Statement::bind<std::string>(std::string value, int column) const {
        int result = sqlite3_bind_text(statement, column, value.c_str(),
            value.size(), SQLITE_STATIC);
        if (result != SQLITE_OK)
            throw DatabaseError(result, sqlite3_errstr(result));
    }

    template<> inline void
    Statement::bind<void*>(void* value, int column) const {
        int result = sqlite3_bind_null(statement, column);
        if (result != SQLITE_OK)
            throw DatabaseError(result, sqlite3_errstr(result));
    }
}

#endif // STATEMENT_H