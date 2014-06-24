#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "options.h"

int
main(int argc, char** argv) {
    Pdfsearch::Options options(argc, argv);
    try {
        options.getopt();
        if (options.getHelp())
            exit(EXIT_SUCCESS);
        options.validate();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
