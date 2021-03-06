#include <iostream>
#include <stdexcept>
#include <vector>
#include "options.h"
#include "database.h"

static void
printResults(const std::vector<Pdfsearch::QueryResult>& results, bool verbose);

int
main(int argc, char** argv) {
    Pdfsearch::Options options(argc, argv);
    try {
        options.getopt();
        if (options.getHelp()) {
            options.printHelp();
            // Using exit(), destructors of automatic variables aren't called.
            return EXIT_SUCCESS;
        }
        options.validate();

        Pdfsearch::Database db(options.getDatabase());
        if (!db.databaseCreated()) {
            if (options.getIndex())
                db.createDatabase();
            else {
                std::cerr << "database missing" << std::endl;
                return EXIT_FAILURE;
            }
        }

        std::string query = options.getQuery();
        if (!query.empty()) {
            auto results(
                db.query(query, options.getVerbose(), options.getMatches()));
            printResults(results, options.getVerbose());
        }
        else if (options.getIndex())
            db.index(options.getDirectories(), options.getRecursion());
        else if (options.getUpdate())
            db.update();
        else if (options.getVacuum())
            db.vacuum();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void
printResults(const std::vector<Pdfsearch::QueryResult>& results, bool verbose) {
    int i = 1;
    for (const auto& r : results) {
        if (verbose) {
            std::cout.width(3);
            std::cout << std::left << i++ << " " << r.file << " [" <<
                r.page << "/" << r.pages << "]: " << r.chunk << std::endl;
        }
        else
            std::cout << r.file << std::endl;
    }
}
