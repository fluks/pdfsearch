#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <new>
#include <boost/regex.hpp>
#include "config.h"
#include "options.h"

Pdfsearch::Options::Options(int argc, char** argv) :
        argc(argc),
        argv(nullptr),
        config(CONFIG_FILE),
        database(DATABASE_FILE),
        directories({ "." }),
        help(false),
        index(false),
        matches(UNLIMITED_MATCHES),
        query(""),
        recursion(RECURSE_INFINITELY),
        update(false),
        vacuum(false),
        verbose(false) {
    this->argv = new char*[argc];
    size_t i = 0;
    try {
        for (; i < static_cast<size_t>(argc); i++) {
            size_t len = ::strlen(argv[i]) + 1;
            this->argv[i] = new char[len];
            ::strcpy(this->argv[i], argv[i]);
        }
    }
    catch (const std::bad_alloc& e) {
        for (size_t j = 0; j < i; j++)
            delete[] this->argv[j];
        delete[] this->argv;

        throw e;
    }
}

Pdfsearch::Options::~Options() {
    reset();

    for (size_t i = 0; i < static_cast<size_t>(argc); i++)
        delete[] argv[i];
    delete[] argv;
}

void
Pdfsearch::Options::reset() {
    optind = 0;
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

    const char* shortopts = ":ac:d:hi::m:q:r:uv";
    const struct option longopts[] = {
        { "vacuum",      0, 0, 'a' },
        { "config",      1, 0, 'c' },
        { "database",    1, 0, 'd' },
        { "help",        0, 0, 'h' },
        { "index",       2, 0, 'i' },
        { "matches",     1, 0, 'm' },
        { "query",       1, 0, 'q' },
        { "recursion",   1, 0, 'r' },
        { "update",      0, 0, 'u' },
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
            case 'h':
                help = true;
                /* Ignore other options. */
                return;
            case 'i':
                index = true;
                /* There's an argument for index, but not right after option,
                 * i.e. not: -idir or --index=dir, but: -i dir or --index dir.
                 * This is needed only when an option has an optional argument.
                 * XXX Is this correct? */
                if (!optarg && optind < argc &&
                        std::string(argv[optind]).substr(0, 1) != "-") {
                    optarg = argv[optind];
                }
                parseDirectories(optarg);
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
            case 'u':
                update = true;
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
        throw std::invalid_argument("either index, query, update or vacuum "
            "option must be selected");
    }

    if (vacuum && index)
        throw std::invalid_argument("vacuum and index options are mutually "
            "exclusive");
    if (vacuum && update)
        throw std::invalid_argument("vacuum and update options are mutually "
            "exclusive");
    if (vacuum && !query.empty())
        throw std::invalid_argument("vacuum and query options are mutually "
            "exclusive");
    if (index && !query.empty())
        throw std::invalid_argument("query and index options are mutually "
            "exclusive");
    if (update && !query.empty())
        throw std::invalid_argument("query and update options are mutually "
            "exclusive");
    if (update && index)
        throw std::invalid_argument("index and update options are mutually "
            "exclusive");

    if (matches < UNLIMITED_MATCHES)
        throw std::invalid_argument("matches argument is negative");
}

void
Pdfsearch::Options::parseConfig() {
    using namespace boost;

    std::ifstream file;
    file.exceptions(std::fstream::failbit | std::fstream::badbit);
    file.open(config);

    static regex_constants::syntax_option_type flags =
        regex::perl | regex::icase;
    static const regex databasePattern("^database\\s*=\\s*(.+)$",       flags);
    static const regex directoriesPattern("^directories\\s*=\\s*(.+)$", flags);
    static const regex matchesPattern("^matches\\s*=\\s*(\\d+)$",       flags);
    static const regex recursionPattern("^recursion\\s*=\\s*(-?\\d+)$", flags);
    static const regex verbosePattern("^verbose\\s*=\\s*(yes|no)$",     flags);
    static const regex ignorePattern("^#.*|\\s*$",                      flags);

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
        else if (regex_match(line, ignorePattern))
            ;
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
        PACKAGE << " version " << VERSION                         << endl <<
        "Usage: " << PACKAGE << " [OPTIONS] ..."                  << endl <<
        "       " << PACKAGE << " -q <STRING>"                    << endl <<
        "       " << PACKAGE << " -i<DIR>,..."                    << endl <<
        "   -a, --vacuum              vacuum database"            << endl <<
        "   -c, --config=FILE         configuration file"         << endl <<
        "   -d, --database=FILE       database file"              << endl <<
        "   -h, --help"                                           << endl <<
        "   -i, --index=[DIR],...     index database searching"   << endl <<
        "                             pdfs from DIRs"             << endl <<
        "   -m, --matches=N           find N matches for query"   << endl <<
        "   -q, --query=STRING        query the database"         << endl <<
        "   -r, --recursion=N         recurse N directories deep" << endl <<
        "   -u, --update              update the database"        << endl <<
        "   -v, --verbose             print query context"        << endl;
    cout << help.str();
}

void
Pdfsearch::Options::parseDirectories(const char* dirs) {
    // No directories passed, scan default directory.
    if (!dirs)
        return;

    directories.clear();
    std::string d;
    for (; *dirs != '\0'; dirs++) {
        if (*dirs == '\\') {
            auto next_char = *(dirs + 1);
            if (next_char != '\0') {
                if (next_char == ',' || next_char == '\\')
                    d.push_back(next_char);
                else {
                    d.push_back('\\');
                    d.push_back(next_char);
                }
                ++dirs;
            }
            else
                d.push_back('\\');
        }
        else if (*dirs == ',') {
            directories.push_back(d);
            d.clear();
        }
        else
            d.push_back(*dirs);
    }
    if (!d.empty())
        directories.push_back(d);
}

int
Pdfsearch::Options::readInt(const char* s, const char* optionName) {
    std::ostringstream error;
    try {
        return std::stoi(s, nullptr, 10);
    }
    catch (const std::invalid_argument& e) {
        error << optionName << " option '" << s << "' is NaN";
        throw std::invalid_argument(error.str());
    }
    catch (const std::out_of_range& e) {
        error << optionName << " option '" << s << "' doesn't fit to int";
        throw std::invalid_argument(error.str());
    }
}
