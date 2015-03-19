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
        std::string config;
        std::string database;
        /* Directories to search for pdfs. */
        std::vector<std::string> directories;
        bool help;
        /* Index or update database. */
        bool index;
        /* Number of matches to return for query. [UNLIMITED_MATCHES, Inf]. */
        int matches;
        std::string query;
        /* Level of recursion to directory.
         * With a negative value recurses infinitely, 0 not at all, 1
         * directories in this directory, etc.
         * [-Inf, Inf]. */
        int recursion;
        /* Vacuum the database. */
        bool vacuum;
        /* Print context for the match.
         * TODO Use -B -A like in grep, characters before and after match. */
        bool verbose;

        void
        parseDirectories(const char* _directories);

        void
        parseConfig();

        void
        parseConfigOption();

        static int
        readInt(const char* s, const char* optionName);
    public:
        enum {
            /** Return all the matches. */
            UNLIMITED_MATCHES = 0,
            /** Recurse infinitely. A negative value will do. */
            RECURSE_INFINITELY = -1
        };
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
        /** Parse options.

        /** Non-copyable. */
        Options(const Options& other) = delete;
        /** Non-copyable. */
        Options& operator=(const Options& other) = delete;
        /** Non-copyable. */
        Options(Options&& other) = delete;
        /** Non-copyable. */
        Options& operator=(Options&& other) = delete;

         * @throws std::ios_base::failure if fails to read config,
         * std::runtime_error on integer overflow or std::invalid_argument on
         * invalid command line argument. */
        void
        getopt();
        /** Validate options.
         * Check that mutually exclusive options are not given, either index,
         * vacuum or query is given and matches < UNLIMITED_MATCHES. Other
         * kind of option validation happens when option is used.
         * @throws std::invalid_argument if there's an invalid option.
         * */
        void
        validate() const;
        /** Print help. */
        static void
        printHelp();
        /** Config file option getter.
         * @return A path to config file.
         */
        std::string
        getConfig() const { return config; };
        /** Database file option getter.
         * @return A path to database file.
         */
        std::string
        getDatabase() const { return database; };
        /** Help option getter.
         * @return True if help option was given as argument, false otherwise.
         */
        bool
        getHelp() const { return help; };
        /** Directories option getter.
         * @return Directories to search pdfs from.
         */
        std::vector<std::string>
        getDirectories() const { return directories; };
        /** Index option getter.
         * @return True if index option was given as argument, false otherwise.
         */
        bool
        getIndex() const { return index; };
        /** Mathes option getter.
         * @return Pdfsearch::Options::UNLIMITED_MATCHES to return all matches.
         */
        int
        getMatches() const { return matches; };
        /** Query option getter.
         * @return
         */
        std::string
        getQuery() const { return query; };
        /** Recursion option getter.
         * @return
         */
        int
        getRecursion() const { return recursion; };
        /** Vacuum option getter.
         * @return
         */
        bool
        getVacuum() const { return vacuum; };
        /** Verbosity option getter.
         * @return
         */
        bool
        getVerbose() const { return verbose; };
    };
}

#endif // OPTIONS_H
