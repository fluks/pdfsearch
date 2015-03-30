#include <string>
#include <type_traits>
#include "catch.hpp"
#include "database_error.h"

TEST_CASE("constructor1", "[database_error]") {
    Pdfsearch::DatabaseError e("aaa");

    REQUIRE(e.what() == std::string("aaa"));
    REQUIRE(e.getCode() == 0);
}

TEST_CASE("constructor2", "[database_error]") {
    Pdfsearch::DatabaseError e(10, "aaa");

    REQUIRE(e.what() == std::string("aaa"));
    REQUIRE(e.getCode() == 10);
}

TEST_CASE("base class", "[database_error]") {
    bool v = std::is_base_of<std::runtime_error, Pdfsearch::DatabaseError>::value;
    REQUIRE(v);
}

TEST_CASE("throw", "[database_error]") {
    try {
        throw Pdfsearch::DatabaseError("bla");
        REQUIRE(false);
    }
    catch (const Pdfsearch::DatabaseError& e) {
        REQUIRE(true);
    }
}
