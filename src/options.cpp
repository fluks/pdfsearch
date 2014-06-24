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
Pdfsearch::Options::getopt() {
    /* Read config option and parse config first. */
    const char* shortopts = "-c:";
    const struct option config_longopts[] = {
        { "config", 1, 0, 'c' },
        { 0, 0, 0, 0 }
    };

    opterr = 0;
    char c;
    while ((c = getopt_long(argc, argv, shortopts, config_longopts, 0)) != -1) {
        switch (c) {
            case 'c':
                config = optarg;
                goto break_config;
        }
    }
    break_config:
    parseConfig();
    optind = 1;

    shortopts = ":ac:d:f:him:q:r:v";
    const struct option longopts[] = {
        { "vacuum",      0, 0, 'a' },
        { "config",      1, 0, 'c' },
        { "database",    1, 0, 'd' },
        { "directories", 1, 0, 'f' },
        { "help",        0, 0, 'h' },
        { "index",       0, 0, 'i' },
        { "matches",     1, 0, 'm' },
        { "query",       1, 0, 'q' },
        { "recurse",     1, 0, 'r' },
        { "verbose",     0, 0, 'v' },
        { 0, 0, 0, 0 }
    };

    while ((c = getopt_long(argc, argv, shortopts, longopts, 0)) != -1) {
        std::ostringstream error;
        switch (c) {
            case 'a':
                vacuum = true;
                break;
            /* Config already handled. */
            case 'c': break;
            case 'd':
                database = optarg;
                break;
            case 'f':
                parseDirectories(optarg);
                break;
            case 'h':
                printHelp();
                help = true;
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
    std::ifstream file(config.c_str());
    std::ostringstream error;
    if (file.fail()) {
        error << "can't open file '" << config << "': " << strerror(errno);
        throw std::ios_base::failure(error.str());
    }

    std::string line;
    static boost::regex_constants::syntax_option_type flags =
        boost::regex::perl | boost::regex::icase;
    static const boost::regex r_database("^database\\s*=\\s*(.+)$", flags);
    static const boost::regex r_directories("^directories\\s*=\\s*(.+)$", flags);
    static const boost::regex r_matches("^matches\\s*=\\s*(\\d+)$", flags);
    static const boost::regex r_recursion("^recursion\\s*=\\s*(-?\\d+)$", flags);
    static const boost::regex r_verbose("^verbose\\s*=\\s*(yes|no)$", flags);
    while (std::getline(file, line).good()) {
        boost::smatch m;
        std::istringstream integer;
        if (boost::regex_match(line, m, r_database))
            database = m[1];
        else if (boost::regex_match(line, m, r_directories))
            parseDirectories(m[1].str().c_str());
        else if (boost::regex_match(line, m, r_matches)) {
            integer.str(m[1]);
            if ((integer >> matches).fail()) {
                error << "recursion option '" << integer.str() <<
                    "' doesn't fit to int";
                throw std::runtime_error(error.str());
            }
        }
        else if (boost::regex_match(line, m, r_recursion)) {
            integer.str(m[1]);
            if ((integer >> recursion).fail()) {
                error << "recursion option '" << integer.str() <<
                    "' doesn't fit to int";
                throw std::runtime_error(error.str());
            }
        }
        else if (boost::regex_match(line, m, r_verbose)) {
            verbose = m[1] == "yes" || m[1] == "YES";
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
Pdfsearch::Options::parseDirectories(const char* _directories) {
    std::string f;
    for (; *_directories != '\0'; _directories++) {
        if (*_directories == ',' &&
            !f.empty() && f.at(f.size() - 1) != '\\') {
            directories.push_back(f);
            f.clear();
        }
        else if (*_directories == ',' &&
                 !f.empty() && f.at(f.size() - 1) == '\\')
            f.replace(f.size() - 1, 1, 1, ',');
        else
            f.push_back(*_directories);
    }
    if (!f.empty())
        directories.push_back(f);
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
