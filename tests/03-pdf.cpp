#include <string>
#include "pdf.h"
#include "catch.hpp"

TEST_CASE("load pdf", "[pdf]") {
    REQUIRE_NOTHROW(Pdfsearch::Pdf p("tests/pdfs/good1/CrashCourse_FR.PDF"));
}

TEST_CASE("load not a pdf", "[pdf]") {
    REQUIRE_THROWS_AS(Pdfsearch::Pdf p("notapdf"), std::runtime_error);
}

TEST_CASE("pages", "[pdf]") {
    Pdfsearch::Pdf p("tests/pdfs/good1/good2/unicodeexample.pdf");

    SECTION("has a certain number of pages") {
        REQUIRE(p.numberOfPages() == 6);
    }

    SECTION("a page has certain text in it") {
        const auto& page(p.getPage(0));
        REQUIRE(page->find("Unicode") != std::string::npos);
    }

    SECTION("getting a not existing page throws") {
        REQUIRE_THROWS_AS(p.getPage(6), std::invalid_argument);
        REQUIRE_THROWS_AS(p.getPage(-1), std::invalid_argument);
    }
}

TEST_CASE("pdf extension", "[pdf]") {
    REQUIRE(Pdfsearch::Pdf::filenameEndsToPdf("a.pdf"));
    REQUIRE(Pdfsearch::Pdf::filenameEndsToPdf("a.Pdf"));
    REQUIRE(Pdfsearch::Pdf::filenameEndsToPdf("€öäå.PDF"));
    REQUIRE(!Pdfsearch::Pdf::filenameEndsToPdf("€öäå.PD"));
    REQUIRE(!Pdfsearch::Pdf::filenameEndsToPdf("pdf"));
}
