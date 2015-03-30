#include <vector>
#include <tuple>
#include <string>
#include "catch.hpp"
#include "resultrowiterator.h"
#include "database.h"
#include "statement.h"
#include "database_error.h"

namespace fs = boost::filesystem;
using namespace Pdfsearch;

TEST_CASE("resultrowiterator operator==", "[resultrowiterator]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement s(db, "select * from pdfs;");

    auto begin = s.begin();
    auto beginCopy = begin;
    auto beginCopyCtor(begin);
    auto end = s.end();

    REQUIRE(begin == beginCopy);
    REQUIRE(begin == beginCopyCtor);
    REQUIRE(begin == end);

    fs::remove(dbFile);
}

TEST_CASE("resultrowiterator operator!=", "[resultrowiterator]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement insert(db, "insert into pdfs(file, last_modified) values(?1, ?2);");
    insert.bind<std::string>("a", 1);
    insert.bind(1, 2);
    insert.step();

    Statement s(db, "select file from pdfs;");
    auto begin = s.begin();
    auto end = s.end();

    REQUIRE(begin != end);

    fs::remove(dbFile);
}

TEST_CASE("resultrowiterator getRow", "[resultrowiterator]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement insert(db, "insert into pdfs(file, last_modified) values(?1, 1);");
    for (int i = 1; i <= 3; i++) {
        insert.bind(std::string('a', i), 1);
        insert.step();
        insert.reset();
    }

    Statement s(db, "select file from pdfs;");
    int row = 1;
    for (auto it = s.begin(); it != s.end(); ++it, row++)
        REQUIRE(it.getRow() == row);

    fs::remove(dbFile);
}

TEST_CASE("resultrowiterator getColumns", "[resultrowiterator]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement insert(db, "insert into pdfs(file, last_modified) values(?1, 1);");
    insert.bind(std::string("a"), 1);
    insert.step();

    Statement s1(db, "select file from pdfs;");
    for (auto it = s1.begin(); it != s1.end(); ++it)
        REQUIRE(it.getColumns() == 1);

    Statement s2(db, "select file, last_modified from pdfs;");
    for (auto it = s2.begin(); it != s2.end(); ++it)
        REQUIRE(it.getColumns() == 2);

    fs::remove(dbFile);
}

TEST_CASE("resultrowiterator column ok", "[resultrowiterator]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    std::vector<std::tuple<std::string, int>> input {
         { std::make_tuple("a", 1) },
         { std::make_tuple("aa", 2) },
         { std::make_tuple("aaa", 3) } };
    Statement insert(db, "insert into pdfs(file, last_modified) values(?1, ?2);");
    for (int i = 0; i < 3; i++) {
        insert.bind(std::get<0>(input.at(i)), 1);
        insert.bind(std::get<1>(input.at(i)), 2);
        insert.step();
        insert.reset();
    }

    Statement s(db, "select file, last_modified from pdfs;");
    std::vector<std::tuple<std::string, int>> output;
    for (auto it = s.begin(); it != s.end(); ++it) {
        std::string f;
        int lm;
        
        REQUIRE_NOTHROW(f = *(it.column<std::string>(0)));
        REQUIRE_NOTHROW(lm = *(it.column<int>(1)));
        
        output.push_back(std::make_tuple(f, lm));
    }

    REQUIRE(std::equal(input.begin(), input.end(), output.begin()));

    fs::remove(dbFile);
}

TEST_CASE("resultrowiterator column fail", "[resultrowiterator]") {
    std::string dbFile("./tests/testdb");
    Database db(dbFile);
    db.createDatabase();

    Statement insert(db, "insert into pdfs(file, last_modified) values(?1, 1);");
    insert.bind<std::string>("a", 1);
    insert.step();

    Statement s1(db, "select file from pdfs;");
    for (auto it = s1.begin(); it != s1.end();) {
        // Column number too high.
        REQUIRE_THROWS_AS(it.column<std::string>(2), DatabaseError);
        // Column number too low.
        REQUIRE_THROWS_AS(it.column<std::string>(-1), DatabaseError);

        it++;

        // No rows left.
        REQUIRE_THROWS_AS(it.column<int>(1), DatabaseError);
    }

    Statement s2(db, "select file from pdfs where file = 'no file';");
    auto it = s2.begin();

    // No rows.
    REQUIRE_THROWS_AS(it.column<std::string>(0), DatabaseError);

    /* Can use same select statement many times.
     * TODO Put this in statement's test file? */
    Statement s3(db, "select last_modified from pdfs;");
    for (auto it = s3.begin(); it != s3.end(); it++)
        ;

    for (auto it = s3.begin(); it != s3.end(); it++) {
        int lm = 0;
        
        REQUIRE_NOTHROW(lm = *(it.column<int>(0)));
        REQUIRE(lm == 1);
    }

    fs::remove(dbFile);
}
