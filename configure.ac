#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([pdfsearch++], [0.01], [fluks.github@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lboost_filesystem':
AC_CHECK_LIB([boost_filesystem], [main])
# FIXME: Replace `main' with a function in `-lboost_regex':
AC_CHECK_LIB([boost_regex], [main])
# FIXME: Replace `main' with a function in `-lboost_system':
AC_CHECK_LIB([boost_system], [main])
# FIXME: Replace `main' with a function in `-lpoppler-cpp':
AC_CHECK_LIB([poppler-cpp], [main])
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [main])

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

AS_MKDIR_P(["${HOME}/.${PACKAGE_NAME}"])
# Point default values to user's $HOME.
AC_CONFIG_COMMANDS_POST([sed -i \
    -e 's%\(CONFIG_FILE =\).*%\1 "'"${HOME}/.${PACKAGE_NAME}/${PACKAGE_NAME}.conf"'";%' \
    -e 's%\(DATABASE_FILE =\).*%\1 "'"${HOME}/.${PACKAGE_NAME}/${PACKAGE_NAME}.sqlite"'";%' \
         "${srcdir}"/src/config.h])
# Create empty config, fails if not found.
AC_CONFIG_COMMANDS_POST([touch "${HOME}/.${PACKAGE_NAME}/${PACKAGE_NAME}.conf"])

AC_OUTPUT
