#include <stdlib.h>
#include "test.h"

int
main() {
    SRunner *sr = srunner_create(suite_options());
    srunner_add_suite(sr, suite_database());

    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    exit(number_failed > 0 ? number_failed : EXIT_SUCCESS);
}
