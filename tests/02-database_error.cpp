#include <string>
#include "test.h"
#include "database_error.h"

START_TEST(createInstance1) {
    try {
        throw Pdfsearch::DatabaseError("1");
        ck_abort_msg("create instance");
    }
    catch (Pdfsearch::DatabaseError& e) {
        ck_assert_msg(e.what() == std::string("1"), "create instance");
    }
}
END_TEST

START_TEST(createInstance2) {
    try {
        throw Pdfsearch::DatabaseError(1, "1");
        ck_abort_msg("create instance");
    }
    catch (Pdfsearch::DatabaseError& e) {
        ck_assert_msg(
            e.getCode() == 1 &&
            e.what() == std::string("1"),
            "create instance");
    }
}
END_TEST

START_TEST(baseClass) {
    try {
        throw Pdfsearch::DatabaseError("a");
        ck_abort_msg("base class is std::runtime_error");
    }
    catch (std::runtime_error& e) {
        ck_assert_msg(true, "base class is std::runtime_error");
    }
}
END_TEST

Suite*
suite_database_error() {
    Suite *suite = suite_create("database_error");
    TCase *tcase = tcase_create("Core");
    suite_add_tcase(suite, tcase);

    tcase_add_test(tcase, createInstance1);
    tcase_add_test(tcase, createInstance2);
    tcase_add_test(tcase, baseClass);

    return suite;
}
