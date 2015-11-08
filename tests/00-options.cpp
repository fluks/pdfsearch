#include <cstring>
#include <limits>
#include <sstream>
#include "config.h"
#include "options.h"
#include "catch.hpp"

TEST_CASE("default options", "[options]") {
    const char* argv[] = { "" };
    Pdfsearch::Options o(1, const_cast<char**>(argv));

    REQUIRE(o.getConfig() == CONFIG_FILE);
    REQUIRE(o.getDatabase() == DATABASE_FILE);
    REQUIRE(o.getDirectories().at(0) == ".");
    REQUIRE(!o.getHelp());
    REQUIRE(!o.getIndex());
    REQUIRE(!o.getUpdate());
    REQUIRE(o.getMatches() == Pdfsearch::Options::UNLIMITED_MATCHES);
    REQUIRE(o.getQuery().empty());
    REQUIRE(o.getRecursion() == Pdfsearch::Options::RECURSE_INFINITELY);
    REQUIRE(!o.getVacuum());
    REQUIRE(!o.getVerbose());
}

TEST_CASE("reset - scan same argv twice", "[options]") {
    const char* argv[] = { "", "-a" };
    Pdfsearch::Options o1(1, const_cast<char**>(argv));
    o1.getopt();

    Pdfsearch::Options::reset();
    o1.getopt();
}

TEST_CASE("vacuum", "[options]") {
    const char* argv[] = { "", "-a" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();

    REQUIRE(o.getVacuum());
}

TEST_CASE("argv is copied", "[options]") {
    // Allocate and copy arguments.
    int argc = 2;
    char** argv = new char*[2];
    for (int i = 0; i < argc; i++)
        argv[i] = new char[3];
    ::strcpy(argv[0], "");
    ::strcpy(argv[1], "-h");

    Pdfsearch::Options o(argc, argv);
    argv[1][1] = 'a';
    o.getopt();

    REQUIRE(o.getHelp());
    REQUIRE(!o.getVacuum());

    for (int i = 0; i < argc; i++)
        delete[] argv[i];
    delete[] argv;
}

TEST_CASE("bundle options", "[options]") {
    const char* query = "unicode";
    const char* argv[] = { "", "-q", query, "-vr3", "-m10" };
    Pdfsearch::Options o(5, const_cast<char**>(argv));
    o.getopt();

    REQUIRE(o.getVerbose());
    REQUIRE(o.getQuery() == query);
    REQUIRE(o.getRecursion() == 3);
    REQUIRE(o.getMatches() == 10);
}

TEST_CASE("long options", "[options]") {
    const char* argv[] = { "", "--database=a", "--verbose", "--index" };
    Pdfsearch::Options o(4, const_cast<char**>(argv));
    o.getopt();

    REQUIRE(o.getDatabase() == "a");
    REQUIRE(o.getVerbose());
    REQUIRE(o.getIndex());
}

TEST_CASE("config", "[options]") {
    const char* argv[] = { "", "-ctest1.conf" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();
    const auto& dirs = o.getDirectories();
    decltype(dirs) expectedDirs{ "a,b", "c", "€öäå" };

    REQUIRE(o.getDatabase() == "t.sqlite");
    REQUIRE(dirs.size() == expectedDirs.size());
    REQUIRE(std::equal(dirs.begin(), dirs.end(), expectedDirs.begin(),
        std::equal_to<std::string>()));
    REQUIRE(o.getMatches() == 5);
    REQUIRE(o.getRecursion() == 3);
    REQUIRE(o.getVerbose());
}

TEST_CASE("command line options override config", "[options]") {
    const char* argv[] = { "", "-ctest1.conf", "--recursion=-1" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE(o.getRecursion() == -1);
}

TEST_CASE("invalid option", "[options]") {
    const char* argv[] = { "", "-w" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));

    REQUIRE_THROWS_AS(o.getopt(), std::invalid_argument);
}

TEST_CASE("matches is NaN", "[options]") {
    const char* argv[] = { "", "-ma" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));

    REQUIRE_THROWS_AS(o.getopt(), std::invalid_argument);
}

TEST_CASE("missing argument", "[options]") {
    const char* argv[] = { "", "-r" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));

    REQUIRE_THROWS_AS(o.getopt(), std::invalid_argument);
}

TEST_CASE("integer overflow", "[options]") {
    long long l = std::numeric_limits<int>::max() + 1LL;
    std::ostringstream oss;
    oss << l;
    const char* argv[] = { "", "-r", oss.str().c_str() };
    Pdfsearch::Options o(3, const_cast<char**>(argv));

    REQUIRE_THROWS_AS(o.getopt(), std::invalid_argument);
}

TEST_CASE("invalid config", "[options]") {
    const char* argv[] = { "", "--config=invalid.conf" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));

    REQUIRE_THROWS_AS(o.getopt(), std::runtime_error);
}

TEST_CASE("validate - no options", "[options]") {
    const char* argv[] = { "" };
    Pdfsearch::Options o(1, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - mutually exclusive1", "[options]") {
    const char* argv[] = { "", "-i", "-a" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - mutually exclusive2", "[options]") {
    const char* argv[] = { "", "-qpera", "--index" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - mutually exclusive3", "[options]") {
    const char* argv[] = { "", "-u", "--index" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - mutually exclusive4", "[options]") {
    const char* argv[] = { "", "-qkalle", "--vacuum" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - mutually exclusive5", "[options]") {
    const char* argv[] = { "", "--update", "--vacuum" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - mutually exclusive6", "[options]") {
    const char* argv[] = { "", "--update", "--query", "bbb" };
    Pdfsearch::Options o(4, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - negative matches", "[options]") {
    const char* argv[] = { "", "-qaa", "-m-1" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_THROWS_AS(o.validate(), std::invalid_argument);
}

TEST_CASE("validate - ok1", "[options]") {
    const char* argv[] = { "", "-i" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();

    REQUIRE_NOTHROW(o.validate());
}

TEST_CASE("directories1", "[options]") {
    const char* argv[] = { "", "-ia,b,c" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();
    const auto& dirs = o.getDirectories();
    decltype(dirs) expectedDirs { "a", "b", "c" };

    REQUIRE(dirs.size() == 3);
    REQUIRE(std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()));
}

TEST_CASE("directories2", "[options]") {
    // Why this doesn't work?
    //const char* argv[] = { "", u8"-index€,öäå,a b c" };
    const char* argv[] = { "", u8"--index", "€,öäå,a b c" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));
    o.getopt();
    const auto& dirs = o.getDirectories();
    decltype(dirs) expectedDirs { u8"€", u8"öäå", u8"a b c" };

    REQUIRE(dirs.size() == expectedDirs.size());
    REQUIRE(std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()));
}

TEST_CASE("directories3", "[options]") {
    const char* argv[] = { "", "-ikal\\,le,toinen" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();
    const auto& dirs = o.getDirectories();
    decltype(dirs) expectedDirs { "kal,le", "toinen" };

    REQUIRE(dirs.size() == expectedDirs.size());
    REQUIRE(std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()));
}

TEST_CASE("directories4", "[options]") {
    const char* argv[] = { "", "--index=\\\\,\\" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();
    const auto& dirs = o.getDirectories();
    decltype(dirs) expectedDirs { "\\", "\\" };

    REQUIRE(dirs.size() == expectedDirs.size());
    REQUIRE(std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()));
}

TEST_CASE("directories5", "[options]") {
    const char* argv[] = { "", "-ia\\b,c" };
    Pdfsearch::Options o(2, const_cast<char**>(argv));
    o.getopt();
    const auto& dirs = o.getDirectories();
    decltype(dirs) expectedDirs { "a\\b", "c" };

    REQUIRE(dirs.size() == expectedDirs.size());
    REQUIRE(std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()));
}

TEST_CASE("config file not found", "[options]") {
    const char* argv[] = { "", "-i", "--config=does_not_exist.conf" };
    Pdfsearch::Options o(3, const_cast<char**>(argv));

    REQUIRE_THROWS_AS(o.getopt(), std::ios_base::failure);
}
