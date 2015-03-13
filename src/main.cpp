#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include "options.h"
#include "database.h"

int
main(int argc, char** argv) {
    Pdfsearch::Options options(argc, argv);
    try {
        options.getopt();
        if (options.getHelp()) {
            options.printHelp();
            exit(EXIT_SUCCESS);
        }
        options.validate();

        Pdfsearch::Database db(options.getDatabase());
        if (!db.databaseCreated())
            db.createDatabase();

        std::string query = options.getQuery();
        if (!query.empty())
            db.query(query, options.getVerbose(), options.getMatches());
        else if (options.getIndex()) {
            std::vector<std::string> dirs = options.getDirectories();
            if (dirs.empty())
                db.update();
            else
                db.index(dirs, options.getRecursion());
        }
        else if (options.getVacuum())
            db.vacuum();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
