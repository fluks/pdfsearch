#include <functional>
#include <iostream>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <limits>
#include <sstream>
#include "test.h"
#include "options.h"
#include "config.h"

START_TEST(defaults) {
    const char* argv[] = { "" };
    int argc = 1;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    ck_assert_msg(
        o.getConfig() == Pdfsearch::Config::CONFIG_FILE &&
        o.getDatabase() == Pdfsearch::Config::DATABASE_FILE &&
        o.getDirectories().empty() &&
        !o.getHelp() &&
        !o.getIndex() &&
        o.getMatches() == Pdfsearch::Options::UNLIMITED_MATCHES &&
        o.getQuery().empty() &&
        o.getRecursion() == Pdfsearch::Options::RECURSE_INFINITELY &&
        !o.getVacuum() &&
        !o.getVerbose(),
        "Constructor sets defaults"
    );
}
END_TEST

START_TEST(vacuum) {
    const char* argv[] = { "", "-a" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_assert_msg(o.getVacuum(), "vacuum set");
    }
    catch (std::exception& e) {
        ck_abort_msg("getopt() throws: %s", e.what());
    }
}
END_TEST

START_TEST(bundle) {
    const char* queryParam =  "'/.*pdf.*/'";
    const char* argv[] = { "", "-q", queryParam, "-vr3", "-m10" };
    int argc = 5;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_assert_msg(
            o.getVerbose() &&
            o.getQuery() == queryParam &&
            o.getRecursion() == 3 &&
            o.getMatches() == 10,
            "options can be bundled"
        );
    }
    catch (std::exception& e) {
        ck_abort_msg("getopt() throws: %s", e.what());
    }
}
END_TEST

START_TEST(longOptions) {
    const char* argv[] = { "", "--database=a", "--verbose", "--index" };
    int argc = 4;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();

        ck_assert_msg(
            o.getDatabase() == "a" &&
            o.getVerbose() &&
            o.getIndex(),
            "long options can be used"
        );
    }
    catch (std::exception& e) {
        ck_abort_msg("getopt() throws: %s", e.what());
    }
}
END_TEST

START_TEST(config) {
    const char* argv[] = { "", "-ctests/test1.conf" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        auto dirs = o.getDirectories();
        decltype(dirs) expectedDirs({ "a,b", "c", "€öäå"});

        ck_assert_msg(
            o.getDatabase() == "t.sqlite" &&
            dirs.size() == expectedDirs.size() &&
            std::equal(dirs.begin(), dirs.end(), expectedDirs.begin(),
                std::equal_to<std::string>()) &&
            o.getMatches() == 5 &&
            o.getRecursion() == 3 &&
            o.getVerbose(),
            "read options from config"
        );
    }
    catch (std::exception& e) {
        ck_abort_msg("getopt() throws: %s", e.what());
    }
}
END_TEST

START_TEST(commandlineOptionsOverrideConfig) {
    const char* argv[] = { "", "-ctests/test1.conf", "--recursion=-1" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();

        ck_assert_msg(o.getRecursion() == -1,
            "commandline options override options in config");
    }
    catch (std::exception& e) {
        ck_abort_msg("getopt() throws: %s", e.what());
    }
}
END_TEST

START_TEST(invalidOption) {
    const char* argv[] = { "", "-w" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_abort_msg("invalid option");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "invalid option");
    }
}
END_TEST

START_TEST(matchesIsNaN) {
    const char* argv[] = { "", "-ma" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_abort_msg("matches option is NaN fails");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "matches option is NaN fails");
    }
}
END_TEST

START_TEST(missingArgument) {
    const char* argv[] = { "", "-r" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_abort_msg("recursion needs an argument");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "recursion needs an argument");
    }
}
END_TEST

START_TEST(integerOverflow) {
    long long l = std::numeric_limits<int>::max() + 1LL;
    std::ostringstream oss;
    oss << l;
    /* For some reason if oss.str().c_str() is used to initialize argv,
     * getopt() throws something unexpected. */
    const std::string s(oss.str());
    const char* argv[] = { "", "-r", s.c_str() };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_abort_msg("recursion's integer overflows");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "recursion's integer overflows");
    }
}
END_TEST

START_TEST(invalidConfig) {
    const char* argv[] = { "", "--config=tests/invalid.conf" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();

        ck_abort_msg("invalid option in config");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "invalid option in config");
    }
}
END_TEST

START_TEST(validate1) {
    const char* argv[] = { "" };
    int argc = 1;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        o.validate();
        ck_abort_msg("no command line options doesn't validate");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "no command line options doesn't validate");
    }
}
END_TEST

START_TEST(validate2) {
    const char* argv[] = { "", "-ia" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        o.validate();
        ck_abort_msg("index and vacuum options are mutually exclusive");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "index and vacuum options are mutually exclusive");
    }
}
END_TEST

START_TEST(validate3) {
    const char* argv[] = { "", "-i", "-qpera" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        o.validate();
        ck_abort_msg("index and query options are mutually exclusive");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "index and query options are mutually exclusive");
    }
}
END_TEST

START_TEST(validate4) {
    const char* argv[] = { "", "-qkalle", "-a" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        o.validate();
        ck_abort_msg("query and vacuum options are mutually exclusive");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "query and vacuum options are mutually exclusive");
    }
}
END_TEST

START_TEST(validate5) {
    const char* argv[] = { "", "-im-1" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        o.validate();
        ck_abort_msg("matches option is negative");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "matches option is negative");
    }
}
END_TEST

START_TEST(validate6) {
    const char* argv[] = { "", "-i" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        o.validate();
        ck_assert_msg(true, "index option ok");
    }
    catch (std::exception& e) {
        ck_abort_msg("index option ok");
    }
}
END_TEST

START_TEST(directories1) {
    /* "-i" option is there only to validate() not throw. */
    const char* argv[] = { "", "-i", "-fa,b,c" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        auto dirs = o.getDirectories();
        decltype(dirs) expectedDirs { "a", "b", "c" };

        ck_assert_msg(
            dirs.size() == 3 &&
            std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()),
            "directories1 ok");
    }
    catch (std::exception& e) {
        ck_abort_msg("directories1 ok");
    }
}
END_TEST

START_TEST(directories2) {
    /* This wouldn't work on the command line, the spaces in the last directory
     * name should be escaped or whole argument single quoted. */
    const char* argv[] = { "", "-i", u8"-f€,öäå,a b c" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        auto dirs = o.getDirectories();
        decltype(dirs) expectedDirs { u8"€", u8"öäå", u8"a b c" };

        ck_assert_msg(
            dirs.size() == expectedDirs.size() &&
            std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()),
            "directories2 ok");
    }
    catch (std::exception& e) {
        ck_abort_msg("directories2 ok");
    }
}
END_TEST

START_TEST(directories3) {
    const char* argv[] = { "", "-i", "-fkal\\,le,toinen" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        auto dirs = o.getDirectories();
        decltype(dirs) expectedDirs { "kal,le", "toinen" };

        ck_assert_msg(
            dirs.size() == expectedDirs.size() &&
            std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()),
            "directories3 ok");
    }
    catch (std::exception& e) {
        ck_abort_msg("directories3 ok");
    }
}
END_TEST

START_TEST(directories4) {
    const char* argv[] = { "", "-i", "-f\\\\,\\" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        auto dirs = o.getDirectories();
        decltype(dirs) expectedDirs { "\\", "\\" };

        ck_assert_msg(
            dirs.size() == expectedDirs.size() &&
            std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()),
            "directories4 ok");
    }
    catch (std::exception& e) {
        ck_abort_msg("directories4 ok");
    }
}
END_TEST

START_TEST(directories5) {
    const char* argv[] = { "", "-i", "-fa\\b,c" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        auto dirs = o.getDirectories();
        decltype(dirs) expectedDirs { "a\\b", "c" };

        ck_assert_msg(
            dirs.size() == expectedDirs.size() &&
            std::equal(dirs.begin(), dirs.end(), expectedDirs.begin()),
            "directories5 ok");
    }
    catch (std::exception& e) {
        ck_abort_msg("directories5 ok");
    }
}
END_TEST

START_TEST(configFileNotFound) {
    const char* argv[] = { "", "-i", "--config=does_not_exist.conf" };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();

        ck_abort_msg("config file not found throws");
    }
    catch (std::ios_base::failure& e) {
        ck_assert_msg(true, "config file not found throws");
    }
}
END_TEST


Suite*
suite_options() {
    Suite *suite = suite_create("options");
    TCase *tcase = tcase_create("Core");
    suite_add_tcase(suite, tcase);

    tcase_add_test(tcase, defaults);
    tcase_add_test(tcase, vacuum);
    tcase_add_test(tcase, bundle);
    tcase_add_test(tcase, longOptions);
    tcase_add_test(tcase, config);
    tcase_add_test(tcase, commandlineOptionsOverrideConfig);
    tcase_add_test(tcase, invalidOption);
    tcase_add_test(tcase, matchesIsNaN);
    tcase_add_test(tcase, missingArgument);
    tcase_add_test(tcase, integerOverflow);
    tcase_add_test(tcase, invalidConfig);
    tcase_add_test(tcase, validate1);
    tcase_add_test(tcase, validate2);
    tcase_add_test(tcase, validate3);
    tcase_add_test(tcase, validate4);
    tcase_add_test(tcase, validate5);
    tcase_add_test(tcase, validate6);
    tcase_add_test(tcase, directories1);
    tcase_add_test(tcase, directories2);
    tcase_add_test(tcase, directories3);
    tcase_add_test(tcase, directories4);
    tcase_add_test(tcase, directories5);
    tcase_add_test(tcase, configFileNotFound);

    return suite;
}
