#include <stdlib.h>
#include <fstream>
#include "test.h"
#include "config.h"

static void
createConfig();

int
main() {
    SRunner *sr = srunner_create(suite_options());
    //srunner_add_suite(sr, );

    /* Create default config, otherwise getopt() throws. */
    createConfig();

    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    exit(number_failed > 0 ? number_failed : EXIT_SUCCESS);
}

static void
createConfig() {
    std::ofstream config(Pdfsearch::Config::CONFIG_FILE);
    config.close();
}
