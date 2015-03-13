#include <cstdio>
#include <boost/filesystem.hpp>
#include "test.h"
#include "database.h"
#include "database_error.h"

START_TEST(defaultConstructor) {
    Pdfsearch::Database db;

    ck_assert_msg(dynamic_cast<Pdfsearch::Database*>(&db) != NULL,
        "default constructor creates an instance");
}
END_TEST

START_TEST(constructorWithParameter) {
    const char* file = "tests/_t.sqlite";
    Pdfsearch::Database db(file);

    ck_assert_msg(dynamic_cast<Pdfsearch::Database*>(&db) != NULL,
        "constructor with a parameter creates an instance");
    ck_assert_msg(boost::filesystem::exists(file), "create a database file");

    std::remove(file);
}
END_TEST

START_TEST(constructorThrowsWithUnreadableFile) {
    const char* file = "tests/_mode_000.sqlite";

    try {
        Pdfsearch::Database db(file);
        ck_abort_msg("constructor with parameter throws on unreadable file");
    }
    catch (Pdfsearch::DatabaseError& e) {
        ck_assert_msg(true,
            "constructor with parameter throws on unreadable file");
    }
}
END_TEST

START_TEST(openDatabaseConnection) {
    const char* file = "tests/_exists.sqlite";
    Pdfsearch::Database db;

    try {
        db.open(file);
        ck_assert_msg(true, "opens database connection");
    }
    catch (Pdfsearch::DatabaseError& e) {
        ck_abort_msg("opens database connection");
    }
}
END_TEST

START_TEST(createDatabase) {
    const char* file = "tests/_create.sqlite";

    try {
        Pdfsearch::Database db(file);

        ck_assert_msg(!db.databaseCreated(), "database not yet created");

        db.createDatabase();

        ck_assert_msg(db.databaseCreated(), "database created");
    }
    catch (Pdfsearch::DatabaseError& e) {
        ck_abort_msg("create database");
    }

    std::remove(file);
}
END_TEST

START_TEST(closeDatabase) {
    const char* file = "tests/_exists.sqlite";

    try {
        Pdfsearch::Database db(file);
        db.close();
        /* Fails because database connection closed. TODO Should it check if
         * connection is open? */
        db.createDatabase();

        ck_abort_msg("close database");
    }
    catch (std::exception& e) {
        ck_assert_msg(true, "close database");
    }
}
END_TEST

Suite*
suite_database() {
    Suite *suite = suite_create("database");
    TCase *tcase = tcase_create("Core");
    suite_add_tcase(suite, tcase);

    tcase_add_test(tcase, defaultConstructor);
    tcase_add_test(tcase, constructorWithParameter);
    tcase_add_test(tcase, constructorThrowsWithUnreadableFile);
    tcase_add_test(tcase, openDatabaseConnection);
    tcase_add_test(tcase, createDatabase);
    tcase_add_test(tcase, closeDatabase);

    return suite;
}
