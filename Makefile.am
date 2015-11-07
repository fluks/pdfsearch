SUBDIRS = src tests doc

AM_CTAGSFLAGS = -R -o tags --c++-kinds=+p --fields=+iaS --extra=+q \
		/usr/local/include/poppler/cpp /usr/include/sqlite3.h \
		/usr/include/boost/filesystem/ /usr/include/boost/regex/ \
		/usr/include/boost/system/

cppcheck:
	@cppcheck --enable=warning,information --suppress=missingIncludeSystem \
		$(top_srcdir)/src

generate-doc:
	@doxygen $(top_srcdir)/doc/Doxyfile

generate-man:
	@pod2man --release --center 'General Commands Manual' \
		$(top_srcdir)/doc/pdfsearch.pod $(top_srcdir)/doc/pdfsearch.1

.PHONY: generate-doc generate-man cppcheck
