#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([pdfsearch], [0.01], [https://github.com/fluks/pdfsearch/issues], [],\
    [https://github.com/fluks/pdfsearch])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_LANG_CPLUSPLUS
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Disable assertions with --disable-assert.
AC_HEADER_ASSERT

AX_BOOST_BASE
AX_BOOST_FILESYSTEM
AX_BOOST_SYSTEM
AX_BOOST_REGEX
LIBS="$BOOST_FILESYSTEM_LIB $BOOST_SYSTEM_LIB $BOOST_REGEX_LIB"

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset])

AC_CONFIG_FILES([Makefile \
                src/Makefile \
                tests/Makefile \
                doc/Makefile])

AC_DEFINE_UNQUOTED([CONFIG_FILE],
    ["${HOME}/.${PACKAGE_NAME}/${PACKAGE_NAME}.conf"], [Configuration file.])
AC_DEFINE_UNQUOTED([DATABASE_FILE],
    ["${HOME}/.${PACKAGE_NAME}/${PACKAGE_NAME}.sqlite"], [Database file.])

AS_MKDIR_P(["${HOME}/.${PACKAGE_NAME}"])
# Create empty config, fails if not found.
AC_CONFIG_COMMANDS_POST([touch "${HOME}/.${PACKAGE_NAME}/${PACKAGE_NAME}.conf"])

PKG_CHECK_MODULES([POPPLER], [poppler], ,
    AC_MSG_ERROR([poppler not found], ))
PKG_CHECK_MODULES([POPPLERCPP], [poppler-cpp], ,
    AC_MSG_ERROR([poppler-cpp not found], ))
PKG_CHECK_MODULES([SQLITE], [sqlite3], ,
    AC_MSG_ERROR([sqlite not found], ))

LIBS="$LIBS $POPPLER_LIBS $POPPLERCPP_LIBS $SQLITE_LIBS"
AC_SUBST([AM_CPPFLAGS],
    ['$(POPPLER_CFLAGS) $(POPPLERCPP_CFLAGS) $(SQLITE_CFLAGS)'])

AC_OUTPUT
