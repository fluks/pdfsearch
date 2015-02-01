#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <boost/regex.hpp>
#include "options.h"

Pdfsearch::Options::Options(int _argc, char** _argv) :
    argc(_argc),
    argv(_argv),
    config(Pdfsearch::Config::CONFIG_FILE),
    database(Pdfsearch::Config::DATABASE_FILE),
    help(false),
    index(false),
    matches(UNLIMITED_MATCHES),
    query(""),
    recursion(RECURSE_INFINITELY),
    vacuum(false),
    verbose(false) {
}

void
Pdfsearch::Options::parseConfigOption() {
    const char* shortopts = "-c:";
    const struct option longopts[] = {
        { "config", 1, 0, 'c' },
        { 0, 0, 0, 0 }
    };

    char c;
    while ((c = getopt_long(argc, argv, shortopts, longopts, 0)) != -1) {
        if (c == 'c') {
            config = optarg;
            return;
        }
    }
}

void
Pdfsearch::Options::getopt() {
    /* Read config option first and then parse config file. */
    opterr = 0;
    parseConfigOption();
    parseConfig();
    optind = 1;

    const char* shortopts = ":ac:d:f:him:q:r:v";
    const struct option longopts[] = {
        { "vacuum",      0, 0, 'a' },
        { "config",      1, 0, 'c' },
        { "database",    1, 0, 'd' },
        { "directories", 1, 0, 'f' },
        { "help",        0, 0, 'h' },
        { "index",       0, 0, 'i' },
        { "matches",     1, 0, 'm' },
        { "query",       1, 0, 'q' },
        { "recursion",   1, 0, 'r' },
        { "verbose",     0, 0, 'v' },
        { 0, 0, 0, 0 }
    };

    char c;
    while ((c = getopt_long(argc, argv, shortopts, longopts, 0)) != -1) {
        std::ostringstream error;
        switch (c) {
            case 'a':
                vacuum = true;
                break;
            /* Config already handled, but the option still exists in argv,
             * don't remove. */
            case 'c': break;
            case 'd':
                database = optarg;
                break;
            case 'f':
                parseDirectories(optarg);
                break;
            case 'h':
                help = true;
                /* Ignore other options. */
                return;
            case 'i':
                index = true;
                break;
            case 'm':
                matches = readInt(optarg, "matches");
                break;
            case 'q':
                query = optarg;
                break;
            case 'r':
                recursion = readInt(optarg, "recursion");
                break;
            case 'v':
                verbose = true;
                break;
            case '?':
                error << "invalid option '" << static_cast<char>(optopt) << "'";
                throw std::invalid_argument(error.str());
                break;
            case ':':
                error << "option '" << static_cast<char>(optopt) <<
                    "', requires an argument";
                throw std::invalid_argument(error.str());
                break;
        }
    }
}

void
Pdfsearch::Options::validate() const {
    if (!index && query.empty() && !vacuum) {
        throw std::invalid_argument("either index, query or vacuum option "
            "must be selected");
    }

    if (vacuum && index)
        throw std::invalid_argument("vacuum and index options are mutually "
            "exclusive");
    if (vacuum && !query.empty())
        throw std::invalid_argument("vacuum and query options are mutually "
            "exclusive");
    if (index && !query.empty())
        throw std::invalid_argument("query and index options are mutually "
            "exclusive");

    if (matches < UNLIMITED_MATCHES)
        throw std::invalid_argument("matches argument is negative");
}

void
Pdfsearch::Options::parseConfig() {
    std::ifstream file;
    file.exceptions(std::fstream::failbit | std::fstream::badbit);
    file.open(config);

    using namespace boost;
    static regex_constants::syntax_option_type flags =
        regex::perl | regex::icase;
    static const regex databasePattern("^database\\s*=\\s*(.+)$",       flags);
    static const regex directoriesPattern("^directories\\s*=\\s*(.+)$", flags);
    static const regex matchesPattern("^matches\\s*=\\s*(\\d+)$",       flags);
    static const regex recursionPattern("^recursion\\s*=\\s*(-?\\d+)$", flags);
    static const regex verbosePattern("^verbose\\s*=\\s*(yes|no)$",     flags);

    std::string line;
    /* If failtbit is set, might throw on eof. */
    file.exceptions(std::fstream::badbit);
    while (std::getline(file, line).good()) {
        smatch m;
        std::istringstream integer;
        std::ostringstream error;

        if (regex_match(line, m, databasePattern))
            database = m[1];
        else if (regex_match(line, m, directoriesPattern))
            parseDirectories(m[1].str().c_str());
        else if (regex_match(line, m, matchesPattern)) {
            integer.str(m[1]);
            if ((integer >> matches).fail()) {
                error << "matches option '" << integer.str() <<
                    "' doesn't fit to int";
                throw std::runtime_error(error.str());
            }
        }
        else if (regex_match(line, m, recursionPattern)) {
            integer.str(m[1]);
            if ((integer >> recursion).fail()) {
                error << "recursion option '" << integer.str() <<
                    "' doesn't fit to int";
                throw std::runtime_error(error.str());
            }
        }
        else if (regex_match(line, m, verbosePattern)) {
            std::string lowercaseM(m[1].str());
            std::transform(lowercaseM.begin(), lowercaseM.end(),
                lowercaseM.begin(), ::tolower);
            verbose = lowercaseM == "yes";
        }
        else {
            error << "invalid option in configuration file: '" << line << "'";
            throw std::runtime_error(error.str());
        }
    }

    file.close();
}

void
Pdfsearch::Options::printHelp() {
    using namespace std;
    ostringstream help;
    help <<
        Config::PROGRAM_NAME << " version " << Config::VERSION    << endl <<
        "Usage: " << Config::PROGRAM_NAME << " [OPTIONS] ..."     << endl <<
        "       " << Config::PROGRAM_NAME << " -q <STRING>"       << endl <<
        "       " << Config::PROGRAM_NAME << " -i"                << endl <<
        "   -a, --vacuum              vacuum database"            << endl <<
        "   -c, --config=FILE         configuration file"         << endl <<
        "   -d, --database=FILE       database file"              << endl <<
        "   -f, --directories=DIR,... search pdfs from DIRs"      << endl <<
        "   -h, --help"                                           << endl <<
        "   -i, --index               index and update database"  << endl <<
        "   -m, --matches=N           find N matches for query"   << endl <<
        "   -q, --query=STRING        query the database"         << endl <<
        "   -r, --recursion=N         recurse N directories deep" << endl <<
        "   -v, --verbose             print query context"        << endl;
    cout << help.str();
}

void
Pdfsearch::Options::parseDirectories(const char* _dirs) {
    std::string dir;
    for (; *_dirs != '\0'; _dirs++) {
        if (*_dirs == '\\') {
            auto next_char = *(_dirs + 1);
            if (next_char != '\0') {
                if (next_char == ',' || next_char == '\\')
                    dir.push_back(next_char);
                else {
                    dir.push_back('\\');
                    dir.push_back(next_char);
                }
                ++_dirs;
            }
            else
                dir.push_back('\\');
        }
        else if (*_dirs == ',') {
            directories.push_back(dir);
            dir.clear();
        }
        else
            dir.push_back(*_dirs);
    }
    if (!dir.empty())
        directories.push_back(dir);
}

int
Pdfsearch::Options::readInt(const char* s, const char* optionName) {
    std::ostringstream error;
    try {
        return std::stoi(s, nullptr, 10);
    }
    catch (std::invalid_argument& e) {
        error << optionName << " option '" << s << "' is NaN";
        throw std::invalid_argument(error.str());
    }
    catch (std::out_of_range& e) {
        error << optionName << " option '" << s << "' doesn't fit to int";
        throw std::invalid_argument(error.str());
    }
}
