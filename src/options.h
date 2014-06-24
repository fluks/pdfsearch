#ifndef OPTIONS_H
    #define OPTIONS_H

#include <vector>
#include <string>
#include "config.h"

namespace Pdfsearch {
    /** Class to read command line options.
     */
    class Options {
    private:
        int argc;
        char** argv;
        /** Path to config file. */
        std::string config;
        /** Path to database file. */
        std::string database;
        /** Directories to search for pdfs. */
        std::vector<std::string> directories;
        bool help;
        /** Index or update database. */
        bool index;
        /** Number of matches to return for query. [UNLIMITED_MATCHES, Inf]. */
        int matches;
        /** Query string. */
        std::string query;
        /** Level of recursion to directory.
         * With a negative value recurses infinitely, 0 not at all, 1
         * directories in this directory, etc.
         * [-Inf, Inf]. */
        int recursion;
        /** Vacuum the database. */
        bool vacuum;
        /** Print context for the match.
         * TODO Use -B -A like in grep, characters before and after match. */
        bool verbose;
        enum {
            /** Return all the matches. */
            UNLIMITED_MATCHES = 0,
            /** Recurse infinitely. A negative value will do. */
            RECURSE_INFINITELY = -1
        };

        void parseDirectories(const char* _directories);
        /** Parse config file. */
        void parseConfig();

        /** Read an int and throw if fails. */
        static int readInt(const char* s, const char* optionName);
    public:
        /** Create a new Options instance.
         * @param _argc Number of command line arguments.
         * @param _argv Command line arguments.
         * Sets defaults:
         *     config: Pdfsearch::Config::CONFIG_FILE
         *     database: Pdfsearch::Config::DATABASE_FILE
         *     directories: empty
         *     help: false
         *     index: false
         *     matches: UNLIMITED_MATCHES
         *     query: empty string
         *     recursion: RECURSE_INFINITELY
         *     vacuum: false
         *     verbose: false
         */
        Options(int _argc, char** _argv);
        /** Parse options. */
        void getopt();
        /** Validate options.
         * Check that mutually exclusive options are not given, either index,
         * vacuum or query is given and matches < UNLIMITED_MATCHES. Other
         * kind of option validation happens when option is used.
         * @throws std::invalid_argument if there's an invalid option.
         * */
        void validate() const;
        /** Print help. */
        static void printHelp();

        std::string getConfig() const { return config; };
        std::string getDatabase() const { return database; };
        bool getHelp() const { return help; };
        std::vector<std::string> getDirectories() const { return directories; };
        bool getIndex() const { return index; };
        int getMatches() const { return matches; };
        std::string getQuery() const { return query; };
        int getRecursion() const { return recursion; };
        bool getVacuum() const { return vacuum; };
        bool getVerbose() const { return verbose; };
    };
}

#endif // OPTIONS_H
