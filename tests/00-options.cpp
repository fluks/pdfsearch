#include <cstring>
#include "options.h"
#include "config.h"
#include "catch.hpp"

TEST_CASE("default options", "[options]") {
    const char* argv[] = { "" };
    Pdfsearch::Options o(1, const_cast<char**>(argv));

    REQUIRE(o.getConfig() == Pdfsearch::Config::CONFIG_FILE);
    REQUIRE(o.getDatabase() == Pdfsearch::Config::DATABASE_FILE);
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
    const char* argv[] = { "", "-ctests/test1.conf" };
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
