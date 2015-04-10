SUBDIRS = src tests doc

AM_CTAGSFLAGS = -R -o tags --c++-kinds=+p --fields=+iaS --extra=+q \
		/usr/local/include/poppler/cpp /usr/include/sqlite3.h \
		/usr/include/boost/filesystem/ /usr/include/boost/regex/ \
		/usr/include/boost/system/

cppcheck:
	cppcheck --enable=warning,information --suppress=missingIncludeSystem \
		$(top_srcdir)/src

generate-doc:
	@doxygen $(top_srcdir)/doc/Doxyfile

generate-man:
	@pod2man --release --center 'General Commands Manual' \
		$(top_srcdir)/doc/pdfsearch.pod $(top_srcdir)/doc/pdfsearch.1

all-local:
	@# Save original config.h.
	[ ! -e "$(top_srcdir)/src/config.h.orig" ] && \
		cp $(top_srcdir)/src/config.h $(top_srcdir)/src/config.h.orig
	sed -i \
		-e 's/\(CONFIG_FILE =\).*/\1 "$(subst /,\/,$(HOME))\/.pdfsearch\/pdfsearch.conf";/' \
		-e 's/\(DATABASE_FILE =\).*/\1 "$(subst /,\/,$(HOME))\/.pdfsearch\/pdfsearch.sqlite";/' \
			$(top_srcdir)/src/config.h

link-new-poppler:
	make LDFLAGS='-L/usr/local/lib/ -Wl,--rpath,/usr/local/lib'

.PHONY: generate-doc generate-man cppcheck all-local link-new-poppler
