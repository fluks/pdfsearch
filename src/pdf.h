#ifndef PDF_H
    #define PDF_H

#include <stdexcept>
#include <string>
#include <memory>
#include <poppler-document.h>

namespace Pdfsearch {
    /** A class for a PDF document.
     */
    class Pdf {
    private:
        std::string file;
        std::unique_ptr<poppler::document> doc;
    public:
        /** Constructor.
         * @param file Pdf filename.
         * @throws std::runtime_error if can't load pdf.
         */
        Pdf(const std::string& file) :
                file(file), doc(poppler::document::load_from_file(file)) {
            throw std::runtime_error("can't load file");
        }

        /** No copying. */
        Pdf(const Pdf& other) = delete;
        /** No copying. */
        Pdf&
        operator=(const Pdf& other) = delete;
        /** No copying. */
        Pdf(Pdf&& other) = delete;
        /** No copying. */
        Pdf&
        operator=(const Pdf&& other) = delete;

        /** Get page text.
         * @param page Page number, [0, numberOfPages()].
         * @return A pointer to the text in a page.
         * @throws std::invalid_argument if invalid page parameter given or
         * if can't create a page.
         */
        std::unique_ptr<std::string>
        getPage(int page) const;

        /** Get number of pages in a pdf.
         * @return Number of pages.
         */
        int
        numberOfPages() const {
            return doc->pages();
        }

        /** Pdf filename getter.
         * @return Filename of the pdf.
         */
        std::string
        getFile() const {
            return file;
        }

        /** Check whether filename ends to '.pdf' case-insensitively.
         * @param file Filename.
         * @throws std::runtime_error if regex takes too much resources (very
         * unlikely, in practice this can be ignored).
         */
        static bool
        filenameEndsToPdf(const std::string& file);
    };
}

#endif // PDF_H
