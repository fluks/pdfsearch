#ifndef RESULTROWITERATOR_H
    #define RESULTROWITERATOR_H

#include <sqlite3.h>
#include <stdexcept>
#include "database_error.h"

namespace Pdfsearch {
     /** A class for a resultset of a SELECT SQL query.
     */
    class ResultRowIterator {
    private:
        sqlite3_stmt* statement;
        // Current number of row.
        sqlite3_int64 row;
        // Number of columns in current row.
        int columns;
        // Status of the statement. SQLITE_ROW if there's a row of results,
        // SQLITE_DONE if there's no more rows.
        int status;

        // Constructor is private, use end() and begin().
        // @param statement Prepared statement.
        // @param status 
        ResultRowIterator(sqlite3_stmt* statement, int status) :
            statement(statement), row(0), columns(0), status(status) {
        };

        // Validate column.
        // @param column A column number of a resultset, [0, getColumns()[.
        // @throws Pdfsearch::DatabaseError if there is no rows to return or
        // if column number is invalid.
        void validateCol(int column) {
            if (status != SQLITE_ROW)
                throw DatabaseError("resultset not ready");
            if (column < 0 || column >= columns)
                throw DatabaseError("invalid column number");
        }
    public:
        /** Get the iterator to resultset.
         * The first row is ready if there is such.
         * @param statement Prepared statement.
         * @return Resultset.
         * @throws Pdfsearch::DatabaseError if step to next row fails.
         */
        static ResultRowIterator begin(sqlite3_stmt* statement) {
            ResultRowIterator it(statement, SQLITE_OK);
            it++;

            return it;
        };

        /** The end of the resultset.
         * Used to check is there more results.
         * @param statement Prepared statement.
         * @return ResultRowIterator with no more results.
         */
        static ResultRowIterator end(sqlite3_stmt* statement) {
            return ResultRowIterator(statement, SQLITE_DONE);
        };

        /** Get the next row.
         * Prefix ++ operator.
         * @return This ResultRowIterator.
         * @throws Pdfsearch::DatabaseError if step to next row fails.
         */
        ResultRowIterator& operator++() {
            if (status == SQLITE_DONE)
                throw DatabaseError("out of resultset bounds");

            /*int result = sqlite3_reset(statement);*/
            /*if (result != SQLITE_OK)*/
            /*throw DatabaseError(result, sqlite3_errstr(result));*/

            status = sqlite3_step(statement);
            if (status != SQLITE_ROW && status != SQLITE_DONE)
                throw DatabaseError(status, sqlite3_errstr(status));

            columns = sqlite3_column_count(statement);
            if (status == SQLITE_ROW)
                row++;

            return *this;
        };

        /** Get the next row.
         * Postfix ++ operator.
         * @return Copy of this ResultRowIterator.
         * @throws Pdfsearch::DatabaseError if step to next row fails.
         */
        ResultRowIterator operator++(int) {
            ResultRowIterator result(*this);
            ++(*this);

            return result;
        };

        /** Equality operator.
         * @param rhs Other result.
         * @return True if resultsets are on the same row, false otherwise.
         * @throw if resultsets are for different statements.
         */
        bool operator==(const ResultRowIterator& rhs) const {
            if (statement != rhs.statement)
                throw std::runtime_error(
                    "can't compare resultsets of different statements");

            return status == SQLITE_DONE && rhs.status == SQLITE_DONE;
        };

        /** Unequality operator.
         * @param rhs Other result.
         * @return False if resultsets are on the same row, true otherwise.
         */
        bool operator!=(const ResultRowIterator& rhs) const {
            return !operator==(rhs);
        };

        /** Get value of a column in resultset.
         * Valid generic types are int, sqlite3_int64, double and std::string.
         * Example:
         * @code
         * std::unique_ptr<int> pages(it.column<int>(0));
         * std::unique_ptr<std::string> pages = std::move(it.column<std::string>(0));
         * @endcode
         * @param column Column index. 0 <= column < number of columns.
         * @return Value of a column in resultset as an unique_ptr. Can be
         * nullptr if database column is nullable. In that case user has to
         * check for it before use.
         * @throws DatabaseError if there is no row ready or if index is < 0 or
         * >= number of columns in resultset or if generic type is not the
         * same as column's type.
         */
        template<typename T>
        std::unique_ptr<T> column(int column);

        /** Get current row number.
         * @return Current row number.
         */
        sqlite3_int64 getRow() const {
            return row;
        };

        /** Get number of columns in the current row.
         * @return Number of columns in the current row.
         */
        int getColumns() const {
            return columns;
        };
    };

    template<> inline std::unique_ptr<int>
    ResultRowIterator::column<int>(int column) {
        validateCol(column);

        int type = sqlite3_column_type(statement, column);
        if (type == SQLITE_NULL)
            return std::unique_ptr<int>(nullptr);
        else if (type != SQLITE_INTEGER)
            throw DatabaseError("column's type is not int");

        return std::unique_ptr<int>(
            new int(sqlite3_column_int(statement, column)));
    }

    template<> inline std::unique_ptr<sqlite3_int64>
    ResultRowIterator::column<sqlite3_int64>(int column) {
        validateCol(column);

        int type = sqlite3_column_type(statement, column);
        if (type == SQLITE_NULL)
            return std::unique_ptr<sqlite3_int64>(nullptr);
        else if (type != SQLITE_INTEGER)
            throw DatabaseError("column's type is not int");

        return std::unique_ptr<sqlite3_int64>(
            new sqlite3_int64(sqlite3_column_int(statement, column)));
    }

    template<> inline std::unique_ptr<double>
    ResultRowIterator::column<double>(int column) {
        validateCol(column);

        int type = sqlite3_column_type(statement, column);
        if (type == SQLITE_NULL)
            return std::unique_ptr<double>(nullptr);
        else if (type != SQLITE_FLOAT)
            throw DatabaseError("column's type is not double");

        return std::unique_ptr<double>(
            new double(sqlite3_column_double(statement, column)));
    }

    template<> inline std::unique_ptr<std::string>
    ResultRowIterator::column<std::string>(int column) {
        validateCol(column);

        int type = sqlite3_column_type(statement, column);
        if (type == SQLITE_NULL)
            return std::unique_ptr<std::string>(nullptr);
        else if (type != SQLITE_TEXT)
            throw DatabaseError("column's type is not std::string");

        return std::unique_ptr<std::string>(new std::string(
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, column))));
    }
}

#endif // RESULTROWITERATOR_H
