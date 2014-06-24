#include <functional>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <limits>
#include <sstream>
#include "test.h"
#include "options.h"

static const char* configFile("t.conf");
static const std::unordered_map<std::string, std::string> configs({
    { "database",    "t.sqlite" },
    { "DIRECTORIES", "a\\,b,c,€" },
    { "matches",     "5" },
    { "recursion",   "3" },
    { "verbose",     "no" }
});

static void
createConfig(const char* file,
    const std::unordered_map<std::string, std::string>& configs);

START_TEST(defaults) {
    const char* argv[] = { "" };
    int argc = 1;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));
    auto f1 = o.getDirectories();
    decltype(f1) f2;

    ck_assert_msg(
        o.getConfig() == std::string(Pdfsearch::Config::CONFIG_FILE) &&
        o.getDatabase() == std::string(Pdfsearch::Config::DATABASE_FILE) &&
        f1.size() == f2.size() &&
        std::equal(f1.begin(), f1.end(), f2.begin(),
            std::equal_to<std::string>()) &&
        !o.getHelp() &&
        !o.getIndex() &&
        o.getMatches() == 0 &&
        o.getQuery() == std::string("") &&
        o.getRecursion() == -1 &&
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
            o.getQuery() == std::string(queryParam) &&
            o.getRecursion() == 3 &&
            o.getMatches() == 10,
            "verbose == true && query == %s && recursion == 3 && "
                "matches == 10", queryParam
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
            o.getDatabase() == std::string("a") &&
            o.getVerbose() &&
            o.getIndex(),
            "database == %c && verbose && index", 'a'
        );
    }
    catch (std::exception& e) {
        ck_abort_msg("getopt() throws: %s", e.what());
    }
}
END_TEST

START_TEST(config) {
    const char* argv[] = { "", "-c", configFile, "--verbose",  "-m10" };
    int argc = 5;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        createConfig(configFile, configs);

        o.getopt();
        std::vector<std::string> directories(o.getDirectories());
        decltype(directories) expectedDirectories{ "a,b", "c", "€" };
        ck_assert_msg(
            o.getDatabase() == configs.at("database") &&
            directories.size() == expectedDirectories.size() &&
            std::equal(directories.begin(), directories.end(),
                expectedDirectories.begin(), std::equal_to<std::string>()) &&
            o.getMatches() == 10 &&
            o.getRecursion() ==
                std::stoi(configs.at("recursion"), nullptr, 10) &&
            o.getVerbose(),
            "read options from config"
        );
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
        ck_abort_msg("invalid option didn't throw");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "invalid option did throw");
    }
}
END_TEST

START_TEST(NaN) {
    const char* argv[] = { "", "-ma" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_abort_msg("NaN for matches option didn't throw");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "NaN for matches option did throw");
    }
}
END_TEST

START_TEST(missingArgument) {
    const char* argv[] = { "", "-r" };
    int argc = 2;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        o.getopt();
        ck_abort_msg("no argument for recursion option didn't throw");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "no argument for recursion option did throw");
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
        ck_abort_msg("INT_MAX + 1 for recursion option didn't throw");
    }
    catch (std::invalid_argument& e) {
        ck_assert_msg(true, "INT_MAX + 1 for recursion option did throw");
    }
}
END_TEST

START_TEST(invalidConfig) {
    const char* argv[] = { "", "--config", configFile };
    int argc = 3;
    Pdfsearch::Options o(argc, const_cast<char**>(argv));

    try {
        const std::unordered_map<std::string, std::string> configs({
            { "invalid", "option" }
        });
        createConfig(configFile, configs);

        o.getopt();
        ck_abort_msg("parsing invalid config didn't throw");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "parsing invalid config did throw");
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
        ck_abort_msg("validate() didn't throw with no command line arguments");
    }
    catch (std::exception& e) {
        ck_assert_msg(true,
            "validate() did throw with no command line arguments");
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
        ck_abort_msg("validate() didn't throw with -ia");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "validate() did throw with -ia");
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
        ck_abort_msg("validate() didn't throw with -i -qpera");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "validate() did throw with -i -qpera");
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
        ck_abort_msg("validate() didn't throw with -qkalle -a");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "validate() did throw with -qkalle -a");
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
        ck_abort_msg("validate() didn't throw with -im-1");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "validate() did throw with -im-1");
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
        ck_assert_msg(true, "validate() didn't throw with -i");
    }
    catch (std::exception& e) {
        ck_abort_msg("validate() didn throw with -i");
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
    tcase_add_test(tcase, invalidOption);
    tcase_add_test(tcase, NaN);
    tcase_add_test(tcase, missingArgument);
    tcase_add_test(tcase, integerOverflow);
    tcase_add_test(tcase, invalidConfig);
    tcase_add_test(tcase, validate1);
    tcase_add_test(tcase, validate2);
    tcase_add_test(tcase, validate3);
    tcase_add_test(tcase, validate4);
    tcase_add_test(tcase, validate5);
    tcase_add_test(tcase, validate6);

    return suite;
}

static void
createConfig(const char* file,
    const std::unordered_map<std::string, std::string>& configs) {
    std::ofstream os(file);
    /* Using fail() here calls check's fail(), not ofstream's. */
    if (!os.good()) {
        std::string error("can't open file: ");
        error.append(file).append(" for writing");
        throw std::ios_base::failure(error);
    }
    for (const auto& e : configs) {
        os << e.first << " = " << e.second << std::endl;
        if ((os.rdstate() &
            (std::ofstream::failbit | std::ofstream::badbit)) != 0)
            throw std::ios_base::failure("error writing to config");
    }

    os.close();
}
