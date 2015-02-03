#ifndef DATABASE_ERROR_H
    #define DATABASE_ERROR_H

#include <stdexcept>
#include <string>

namespace Pdfsearch {
    /** A class for database exceptions. */
    class DatabaseError : public std::runtime_error {
    private:
        /** A return value that some SQLite functions return. */
        int code;
    public:
        /**
         * @param _code SQLite error code.
         * @param message DatabaseError message.
         */
        DatabaseError(int _code, const std::string& message) :
            std::runtime_error(message) {
            code = _code;
        };
        /**
         * @param message DatabaseError message.
         */
        DatabaseError(const std::string& message) :
            std::runtime_error(message) {};
        /** Get an SQLite error code.
         * @return An SQLite error code.
         */
        int getCode() const { return code; };
    };
}

#endif /* DATABASE_ERROR_H */
