#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(CellMLAPI, 0.1, [[ak.miller@auckland.ac.nz]])
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR([sources/cellml/CellMLBootstrap.cpp])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE
PKG_PROG_PKG_CONFIG

# Checks for programs.
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_LIBTOOL

# Checks for libraries.
# PKG_CHECK_MODULES([GLIB], [glib-2.0])
# PKG_CHECK_MODULES([GDOME], [gdome2])


# Checks for header files.
AC_CHECK_HEADERS([strings.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_HEADER_STDC

# Check for size of wchar_t...
AC_CHECK_SIZEOF(wchar_t*)

case $srcdir in
  [\\/]* | ?:[\\/]* ) # Absolute
     testdir=$srcdir/tests
     ;;
  *)
     testdir=`pwd`/$srcdir/tests ;;
esac

AC_DEFINE_UNQUOTED(TESTDIR, L"$testdir",
                   [Define TESTDIR to path of test sources.])

AC_CONFIG_FILES([
                 Makefile
                 interfaces/Makefile
                 sources/Makefile
                 sources/dom/Makefile
                 sources/dom_direct/Makefile
                 sources/mathml/Makefile
                 sources/cellml/Makefile
                 tests/Makefile])
AC_OUTPUT
