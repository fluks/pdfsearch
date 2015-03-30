#include <string>
#include <boost/filesystem.hpp>
#include "catch.hpp"
#include "statement.h"
#include "database.h"
#include "database_error.h"

namespace fs = boost::filesystem;
using namespace Pdfsearch;

TEST_CASE("statement constructor", "[statement]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    REQUIRE_NOTHROW(Statement s(db, "select * from pdfs;"));
    REQUIRE_THROWS_AS(Statement s(db, "invalid sql will throw"), DatabaseError);

    fs::remove(dbFile);
}

TEST_CASE("statement bind", "[statement]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    SECTION("ok") {
        Statement s(db, "insert into pdfs(file, last_modified) values(?1, ?2);");

        REQUIRE_NOTHROW(s.bind(std::string("a"), 1));
        REQUIRE_NOTHROW(s.bind(1, 2));
    }

    SECTION("wrong column") {
        Statement s(db, "insert into pdfs(file, last_modified) values(?1, ?2);");

        REQUIRE_THROWS_AS(s.bind(std::string("a"), 0), DatabaseError);
        REQUIRE_THROWS_AS(s.bind(1, 3), DatabaseError);
    }

    fs::remove(dbFile);
}

TEST_CASE("statement step", "[statement]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement s(db, "insert into pdfs(file, last_modified) values(?1, ?2);");
    s.bind(std::string("a"), 1);
    s.bind(1, 2);

    REQUIRE_NOTHROW(s.step());

    fs::remove(dbFile);
}

TEST_CASE("statement reset", "[statement]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement s(db, "insert into pdfs(file, last_modified) values(?1, ?2);");
    for (int i = 1; i < 10; i++) {
        s.bind(std::string('a', i), 1);
        s.bind(i, 2);
        s.step();

        REQUIRE_NOTHROW(s.reset());
    }

    fs::remove(dbFile);
}

TEST_CASE("statement begin and end", "[statement]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement s(db, "select * from pdfs;");
    
    REQUIRE_NOTHROW(auto it = s.begin());
    REQUIRE_NOTHROW(auto it = s.end());

    fs::remove(dbFile);
}
