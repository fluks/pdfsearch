include $(top_srcdir)/common.mk

catch_CPPFLAGS = -I$(top_srcdir)/src $(AM_CPPFLAGS)

TESTS = catch
check_PROGRAMS = catch
catch_SOURCES = 00-options.cpp \
				01-database.cpp \
				02-database_error.cpp \
				03-pdf.cpp \
				04-statement.cpp \
				05-resultrowiterator.cpp \
				catch.cpp \
				catch.hpp
catch_LDADD = $(top_builddir)/src/options.o \
				$(top_builddir)/src/database.o \
				$(top_builddir)/src/pdf.o \
				$(top_builddir)/src/statement.o
EXTRA_DIST = existing_testdb.sqlite \
			 invalid.conf \
			 test1.conf \
			 pdfs
