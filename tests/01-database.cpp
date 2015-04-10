#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <cstdlib>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "catch.hpp"
#include "database.h"
#include "options.h"
#include "statement.h"

namespace fs = boost::filesystem;
using namespace Pdfsearch;

template <typename T> void
findFiles(std::insert_iterator<T> iter, const std::string& dir,
        const boost::regex& pattern);

TEST_CASE("database constructor1", "[database]") {
    REQUIRE_NOTHROW(Database db);
}

TEST_CASE("database constructor2", "[database]") {
    std::string file("./testdb");

    REQUIRE_NOTHROW(Database db(file));
    REQUIRE(fs::exists(file));

    fs::remove(file);
}

TEST_CASE("database constructor fail", "[database]") {
    // Can't open a directory.
    REQUIRE_THROWS_AS(Database db("./pdfs"),
        DatabaseError);
} 

TEST_CASE("database open", "[database]") {
    Database db;
    std::string file("./testdb");

    REQUIRE_NOTHROW(db.open(file));
    REQUIRE(fs::exists(file));

    fs::remove(file);
}

TEST_CASE("database open existing database", "[database]") {
    Database db;

    REQUIRE_NOTHROW(db.open("./existing_testdb.sqlite"));
    REQUIRE_NOTHROW(db.databaseCreated());
}

// Don't know how to really test this.
TEST_CASE("database close", "[database]") {
    std::string file("./testdb");
    Database db(file);
    REQUIRE_NOTHROW(db.close());

    fs::remove(file);
}

TEST_CASE("database databaseCreated", "[database]") {
    std::string file("./testdb");
    Database db(file);

    REQUIRE(!db.databaseCreated());

    fs::remove(file);
}

TEST_CASE("database createDatabase", "[database]") {
    std::string file("./testdb");
    Database db(file);

    REQUIRE_NOTHROW(db.createDatabase());
    REQUIRE(db.databaseCreated());

    fs::remove(file);
}

TEST_CASE("database vacuum", "[database]") {
    std::string file("./testdb");
    Database db(file);
    db.createDatabase();

    /* TODO Is there a definitive method to check was the database vacuumed?
     * Now we rely on changed modification time. */
    const auto& t0 = fs::last_write_time(file);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    REQUIRE_NOTHROW(db.vacuum());
    REQUIRE(fs::last_write_time(file) > t0);

    fs::remove(file);
}

TEST_CASE("database index1", "[database]") {
    std::string dbFile("./testdb");
    fs::remove(dbFile);
    Database db(dbFile);
    db.createDatabase();
    std::vector<std::string> dirs{ "./pdfs/" };

    REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));

    SECTION("index all pdfs") {
        Statement s(db, "select file from pdfs;");
        std::set<std::string> dbFiles;
        for (auto it = s.begin(); it != s.end(); it++)
            dbFiles.insert(*(it.column<std::string>(0)));
        s.reset();

        boost::regex p("\\.pdf$", boost::regex::icase);
        decltype(dbFiles) fsFiles;
        findFiles(std::inserter(fsFiles, fsFiles.begin()), "./pdfs/good1", p);

        REQUIRE(std::equal(fsFiles.begin(), fsFiles.end(), dbFiles.begin()));
    }

    SECTION("update newer pdf") {
        Statement s(db,
            "select last_modified from pdfs where file = ?1;");
        fs::path pdf = fs::canonical("./pdfs/good1/CrashCourse_FR.PDF");
        s.bind(pdf.native(), 1);
        time_t lastModifiedOld = 0;
        for (auto it = s.begin(); it != s.end(); it++)
            lastModifiedOld = *(it.column<sqlite3_int64>(0));
        s.reset();

        fs::last_write_time(pdf, lastModifiedOld + 1);

        REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));

        s.bind(pdf.native(), 1);
        time_t lastModifiedNew = 0;
        for (auto it = s.begin(); it != s.end(); it++)
            lastModifiedNew = *(it.column<sqlite3_int64>(0));
        s.reset();

        REQUIRE(lastModifiedOld < lastModifiedNew);
    }

    SECTION("do nothing for unchanged pdfs") {
        Statement s(db,
            "select file, last_modified from pdfs where file <> ?1;");
        fs::path pdf = fs::canonical("./pdfs/good1/CrashCourse_FR.PDF");
        s.bind(pdf.native(), 1);
        std::set<std::tuple<std::string, sqlite3_int64>> unChangedBefore;
        for (auto it = s.begin(); it != s.end(); it++) {
            unChangedBefore.insert(std::make_tuple(*(it.column<std::string>(0)),
                *(it.column<sqlite3_int64>(1))));
        }
        s.reset();

        fs::last_write_time(pdf, ::time(nullptr));

        REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));

        s.bind(pdf.native(), 1);
        std::set<std::tuple<std::string, sqlite3_int64>> unChangedAfter;
        for (auto it = s.begin(); it != s.end(); it++) {
            unChangedAfter.insert(std::make_tuple(*(it.column<std::string>(0)),
                *(it.column<sqlite3_int64>(1))));
        }

        REQUIRE(std::equal(unChangedBefore.begin(), unChangedBefore.end(),
            unChangedAfter.begin()));
    }

    fs::remove(dbFile);
}

TEST_CASE("database index2", "[database]") {
    std::string dbFile("./testdb");
    fs::remove(dbFile);
    Database db(dbFile);
    db.createDatabase();

    SECTION("don't recurse into directories") {
        std::vector<std::string> dirs{ "./pdfs/" };

        REQUIRE_NOTHROW(db.index(dirs, 0));
        
        Statement s(db, "select id from pdfs;");
        int n = 0;
        for (auto it = s.begin(); it != s.end(); it++)
            n++;

        REQUIRE(n == 0);
    }

    SECTION("recurse one level deep into directories") {
        std::vector<std::string> dirs{ "./pdfs/" };

        REQUIRE_NOTHROW(db.index(dirs, 1));
        
        Statement s(db, "select file from pdfs;");
        std::set<std::string> dbFiles;
        for (auto it = s.begin(); it != s.end(); it++)
            dbFiles.insert(*(it.column<std::string>(0)));

        std::set<std::string> fsFiles;
        fsFiles.insert(fs::canonical("./pdfs/good1/CrashCourse_FR.PDF").
            native());
        fsFiles.insert(fs::canonical(
            "./pdfs/good1/TrueCrypt User Guide.pdf").native());

        REQUIRE(std::equal(dbFiles.begin(), dbFiles.end(), fsFiles.begin()));
    }

    fs::remove(dbFile);
}

TEST_CASE("database update", "[database]") {
    std::string dbFile("./testdb");
    fs::remove(dbFile);
    Database db(dbFile);
    db.createDatabase();
    std::vector<std::string> dirs { "./pdfs" };
    
    SECTION("updating non-modified pdfs doesn't do anything") {
        REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));

        Statement s(db, "select file, last_modified from pdfs;");
        std::set<std::tuple<std::string, sqlite3_int64>> pdfsBefore;
        for (auto it = s.begin(); it != s.end(); it++) {
            pdfsBefore.insert(std::make_tuple(*(it.column<std::string>(0)),
                *(it.column<sqlite3_int64>(1))));
        }
        s.reset();

        REQUIRE_NOTHROW(db.update());

        std::set<std::tuple<std::string, sqlite3_int64>> pdfsAfter;
        for (auto it = s.begin(); it != s.end(); it++) {
            pdfsAfter.insert(std::make_tuple(*(it.column<std::string>(0)),
                *(it.column<sqlite3_int64>(1))));
        }

        REQUIRE(std::equal(pdfsBefore.begin(), pdfsBefore.end(),
            pdfsAfter.begin()));
    }

    SECTION("updating after removing a pdf deletes pdf from database") {
        fs::path from("./pdfs/good1/CrashCourse_FR.PDF");
        fs::path to = fs::canonical("./pdfs/good1/") / fs::path("temp.pdf");
        fs::copy(from, to);

        REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));

        // New pdf and its pages are in database.
        Statement s1(db,
                "select pdfs_id, count(*) from plaintexts where pdfs_id = "
                "(select id from pdfs where file = ?1);");
        s1.bind(to.native(), 1);
        int id = 0;
        int pages = 0;
        for (auto it = s1.begin(); it != s1.end(); it++) {
            id = *(it.column<int>(0));
            pages = *(it.column<int>(1));
        }

        REQUIRE(pages > 0);

        fs::remove(to);

        REQUIRE_NOTHROW(db.update());

        // Pdf is deleted from Pdfs.
        Statement s2(db, "select id from pdfs where file = ?1;");
        s2.bind(to.native(), 1);
        int idAfter = 0;
        for (auto it = s2.begin(); it != s2.end(); it++)
            idAfter = *(it.column<int>(0));

        REQUIRE(idAfter == 0);

        // And pages of the deleted pdf are also deleted.
        Statement s3(db,
            "select count(*) from plaintexts where pdfs_id = ?1;");
        s3.bind(id, 1);
        int pagesAfter = 0;
        for (auto it = s3.begin(); it != s3.end(); it++)
            pagesAfter = *(it.column<int>(0));

        REQUIRE(pagesAfter == 0);
    }

    SECTION("changing a pdf updates it in database") {
        REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));
        
        // Pdf is updated. 
        Statement s(db,
            "select last_modified from pdfs where file = ?1;");
        fs::path pdf = fs::canonical("./pdfs/good1/CrashCourse_FR.PDF");
        s.bind(pdf.native(), 1);
        time_t lastModifiedOld = 0;
        for (auto it = s.begin(); it != s.end(); it++)
            lastModifiedOld = *(it.column<sqlite3_int64>(0));
        s.reset();

        fs::last_write_time(pdf, lastModifiedOld + 1);

        REQUIRE_NOTHROW(db.update());

        s.bind(pdf.native(), 1);
        time_t lastModifiedNew = 0;
        for (auto it = s.begin(); it != s.end(); it++)
            lastModifiedNew = *(it.column<sqlite3_int64>(0));

        REQUIRE(lastModifiedOld < lastModifiedNew);

        // Pages are also updated(not gone).
        Statement s2(db,
            "select count(*) from plaintexts where pdfs_id = "
                "(select id from pdfs where file = ?1);");
        s2.bind(pdf.native(), 1);
        int pages = 0;
        for (auto it = s2.begin(); it != s2.end(); it++)
            pages = *(it.column<int>(0));

        REQUIRE(pages > 0);
    }

    fs::remove(dbFile);
}

TEST_CASE("database query1", "[database]") {
    std::string dbFile("./testdb");
    Database db(dbFile);
    db.createDatabase();
    std::vector<std::string> dirs{ "./pdfs/" };

    REQUIRE_NOTHROW(db.index(dirs, Options::RECURSE_INFINITELY));

    std::vector<QueryResult> r;
    
    REQUIRE_NOTHROW(r = db.query("%", false, Options::UNLIMITED_MATCHES));

    // Not verbose.
    REQUIRE(r.at(0).chunk.empty());

    std::set<std::string> dbPdfs;
    std::for_each(r.begin(), r.end(), [&](const QueryResult& x) {
        dbPdfs.insert(x.file);
    });

    std::set<std::string> fsPdfs;
    boost::regex p("\\.pdf$", boost::regex::icase);
    findFiles(std::inserter(fsPdfs, fsPdfs.begin()), "./pdfs/good1", p);

    // Query returned pages of all indexed pdfs.
    REQUIRE(std::equal(dbPdfs.begin(), dbPdfs.end(), fsPdfs.begin()));

    // Queries are case-insensitive.
    REQUIRE_NOTHROW(r =
        db.query(u8"UnIcOdE", false, Options::UNLIMITED_MATCHES));
    REQUIRE(r.size() > 0);

    // Matches parameter can limit number of results.
    REQUIRE_NOTHROW(r = db.query("%", false, 1));
    REQUIRE(r.size() == 1);

    // Can query with an unicode string.
    REQUIRE_NOTHROW(r = db.query(u8"ﺕﺎﺟﻴﻜﺴﺘﺎن", false, 1));
    REQUIRE(r.size() == 1);

    // Returns every page where the query matches.
    REQUIRE_NOTHROW(r =
        db.query(u8"unicode", false, Options::UNLIMITED_MATCHES));
    REQUIRE(r.size() == 6);

    // Verbose query sets QueryResult's members.
    REQUIRE_NOTHROW(r = db.query("french", true, 1));
    REQUIRE(!r.at(0).chunk.empty());
    REQUIRE(r.at(0).page > 0);
    REQUIRE(r.at(0).pages > 0);

    // There's surrounding text around the match.
    REQUIRE_NOTHROW(r = db.query("utf-8", true, 1));

    p = boost::regex(".+utf-8.+", boost::regex::icase);

    REQUIRE(boost::regex_match(r.at(0).chunk, p));

    fs::remove(dbFile);
}

TEST_CASE("database constraints", "[database]") {
    std::string dbFile("./testdb");
    fs::remove(dbFile);
    Database db(dbFile);
    db.createDatabase();

    SECTION("file is unique") {
        Statement s(db, "insert into pdfs(file, last_modified) values(?1, ?2);");
        s.bind(std::string("a"), 1);
        s.bind(0, 2);
        s.step();
        s.reset();

        s.bind(std::string("a"), 1);
        s.bind(1, 2);

        REQUIRE_THROWS_AS(s.step(), DatabaseError);
    }

    fs::remove(dbFile);
}

/* Helper functions */

template <typename T> void
findFiles(std::insert_iterator<T> iter, const std::string& dir,
        const boost::regex& pattern) {
    fs::recursive_directory_iterator end;
    for (auto it = fs::recursive_directory_iterator(dir); it != end; it++) {
        if (fs::is_regular(it->path()) &&
                 boost::regex_search(it->path().string(), pattern)) {
            iter = fs::canonical(it->path()).native();
        }
    }
}
