#include "pdf.h"
#include <poppler-page.h>
#include <boost/regex.hpp>

std::unique_ptr<std::string>
Pdfsearch::Pdf::getPage(int i) const {
    if (i < 0 || i >= numberOfPages())
        throw std::invalid_argument("invalid page number");

    std::unique_ptr<poppler::page> page(doc->create_page(i));
    if (!page)
        throw std::runtime_error("can't create page");

    std::vector<char> chars(page->text().to_utf8());

    return std::unique_ptr<std::string>(
        new std::string(chars.begin(), chars.end()));
}

bool
Pdfsearch::Pdf::filenameEndsToPdf(const std::string& file) {
    static const boost::regex re("\\.pdf$", boost::regex::icase);
    return boost::regex_search(file, re);
}
