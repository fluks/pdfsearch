#ifndef OPTIONS_H
    #define OPTIONS_H

#include <vector>
#include <string>
#include "config.h"

namespace Pdfsearch {
    /** Class to read command line options.
     * Example usage:
     * @code
       Pdfsearch::Options o(argc, argv);
       try {
           o.getopt();
           o.validate();
           // Use getters.
           // ...
       @endcode
       @note The class is non-copyable.
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
        /* Index database. */
        bool index;
        /* Number of matches to return for query. [UNLIMITED_MATCHES, Inf]. */
        int matches;
        std::string query;
        /* Level of recursion to directory.
         * With a negative value recurses infinitely, 0 not at all, 1
         * directories in this directory, etc.
         * [-Inf, Inf]. */
        int recursion;
        /* Update database. */
        bool update;
        /* Vacuum the database. */
        bool vacuum;
        /* Print context for the match.
         * TODO Use -B -A like in grep, characters before and after match. */
        bool verbose;

        void
        parseDirectories(const char* directories);

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
            /** Recurse infinitely. */
            RECURSE_INFINITELY = -1
        };
        /** Create a new Options instance.
         * @param argc Number of command line arguments.
         * @param argv Command line arguments.
         * Sets defaults:
         *     config: Config::CONFIG_FILE
         *     database: Config::DATABASE_FILE
         *     directories: current directory('.')
         *     help: false
         *     index: false
         *     matches: Options::UNLIMITED_MATCHES
         *     query: empty string
         *     recursion: Options::RECURSE_INFINITELY
         *     update: false
         *     vacuum: false
         *     verbose: false
         * @note Copies argv.
         */
        Options(int argc, char** argv);
        /** Destructor.
         * Frees memory allocated for argv.
         */
        ~Options();

        /** Non-copyable. */
        Options(const Options& other) = delete;
        /** Non-copyable. */
        Options& operator=(const Options& other) = delete;
        /** Non-copyable. */
        Options(Options&& other) = delete;
        /** Non-copyable. */
        Options& operator=(Options&& other) = delete;

        /** Parse commandline options.
         * @throws std::ios_base::failure if fails to read config,
         * std::runtime_error on integer overflow or std::invalid_argument on
         * invalid command line argument.
         */
        void
        getopt();
        /** Validate options.
         * Check that mutually exclusive options are not given, either index,
         * query, update or vacuum is given and matches < UNLIMITED_MATCHES.
         * Other kind of option validation happens when option is used.
         * @throws std::invalid_argument if there's an invalid option.
         */
        void
        validate() const;
        /** Print help. */
        static void
        printHelp();
        /** %Config file option getter.
         * @return A path to config file.
         */
        std::string
        getConfig() const { return config; };
        /** %Database file option getter.
         * @return A path to database file.
         */
        std::string
        getDatabase() const { return database; };
        /** Help option getter.
         * @return True if help option was given as argument, false otherwise.
         */
        bool
        getHelp() const { return help; };
        /** Get directories for index.
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
         * @return Options::UNLIMITED_MATCHES to return all matches.
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
        /** Update option getter.
         * @return True if update option was given as argument, false otherwise.
         */
        bool
        getUpdate() const { return update; }
        /** Vacuum option getter.
         * @return True if vacuum option was given as argument, false otherwise.
         */
        bool
        getVacuum() const { return vacuum; };
        /** Verbosity option getter.
         * @return True if verbose option was given as argument, false otherwise.
         */
        bool
        getVerbose() const { return verbose; };
    };
}

#endif // OPTIONS_H
