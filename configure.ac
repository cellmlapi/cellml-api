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

AC_ARG_ENABLE(corba,
[  --enable-corba    Build CORBA glue],
[case "${enableval}" in
  yes) corba=true ;;
  no)  corba=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-corba) ;;
esac],[corba=false])
AM_CONDITIONAL(ENABLE_CORBA, test x$corba = xtrue)
AC_ARG_ENABLE(context,
[  --enable-context    Build the CellML Context],
[case "${enableval}" in
  yes) context=true ;;
  no)  context=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-context) ;;
esac],[context=false])
AM_CONDITIONAL(ENABLE_CONTEXT, test x$context = xtrue)
# This is verbose but it should (I hope) be portable...
contextandcorba=false
if test x$context = xtrue; then
  if test x$corba = xtrue; then
    contextandcorba=true
  fi
fi
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA, test x$contextandcorba = xtrue)
AC_ARG_ENABLE(server,
[  --enable-server    Build the CellML CORBA Server (requires CORBA and Context)],
[case "${enableval}" in
  yes)
    server=true
    if test x$contextandcorba = xfalse; then
      AC_MSG_ERROR(You need --enable-context and --enable-corba to support --enable-server)
    fi
  ;;
  no)  server=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-server) ;;
esac],[server=false])
AM_CONDITIONAL(ENABLE_CELLML_CORBA_SERVER, test x$server = xtrue)

AC_ARG_ENABLE(ccgs,
[  --enable-ccgs    Build the CellML C Code Generation Service],
[case "${enableval}" in
  yes) ccgs=true ;;
  no)  ccgs=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-ccgs) ;;
esac],[ccgs=false])
AM_CONDITIONAL(ENABLE_CCGS, test x$ccgs = xtrue)

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
                ])
AC_OUTPUT
