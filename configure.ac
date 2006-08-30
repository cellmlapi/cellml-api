#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AM_CXXFLAGS=

AC_INIT(CellMLAPI, 0.1, [[ak.miller@auckland.ac.nz]])
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR([sources/cellml/CellMLBootstrap.cpp])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADER([cda_config.h])

AM_INIT_AUTOMAKE
PKG_PROG_PKG_CONFIG

# Set up libtool...
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CXX
AC_LANG(C)
AC_LANG(C++)
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST([LIBTOOL_DEPS])

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

AC_ARG_ENABLE(cis,
[  --enable-cis    Build the CellML Integration Service],
[case "${enableval}" in
  yes)
    if test x$ccgs = xfalse; then
      AC_MSG_ERROR(You need --enable-ccgs to support --enable-cis)
    fi
    cis=true
    ;;
  no)
    cis=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-cis) ;;
esac],[cis=false])
AM_CONDITIONAL(ENABLE_CIS, test x$cis = xtrue)

# This is verbose but it should (I hope) be portable...
contextandcorba=false
if test x$context = xtrue; then
  AC_DEFINE(ENABLE_CONTEXT, 1, [Is the CellML Context enabled?])
  if test x$corba = xtrue; then
    contextandcorba=true
    if test x$ccgs = xtrue; then
      contextandcorbaandccgs=true
    fi
    if test x$cis = xtrue; then
      contextandcorbaandcis=true
    fi
  fi
fi
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA, test x$contextandcorba = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CCGS,
               test x$contextandcorbaandccgs = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CIS,
               test x$contextandcorbaandcis = xtrue)

# Checks for header files.
AC_CHECK_HEADERS([strings.h wchar.h])

THREADFLAGS=
AC_CHECK_LIB([pthread], [pthread_create], [THREADFLAGS=-lpthread])

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
     abssrcdir=$srcdir
     testdir=$srcdir/tests
     ;;
  *)
     abssrcdir=`pwd`/$srcdir
     testdir=`pwd`/$srcdir/tests ;;
esac

case "$host" in
*-cygwin*|*-mingw*|*-msvc*|*-mks*|*w32*)
    testdir=`$srcdir/build/cygwin-wrapper $testdir`
    ;;
esac

VISIBILITY_FLAGS=
dnl Borrowed from glibc configure.in
dnl ===============================================================
if test "$GCC" = "yes"; then
  AC_CACHE_CHECK(for visibility(hidden) attribute,
                 ac_cv_visibility_hidden,
                 [cat > conftest.c <<EOF
                  int foo __attribute__ ((visibility ("hidden"))) = 1;
EOF
                  ac_cv_visibility_hidden=no
                  if ${CC-cc} -Werror -S conftest.c -o conftest.s >/dev/null 2>&1; then
                    if grep '\.hidden.*foo' conftest.s >/dev/null; then
                      ac_cv_visibility_hidden=yes
                    fi
                  fi
                  rm -f conftest.[cs]
                 ])
  if test "$ac_cv_visibility_hidden" = "yes"; then
    AC_DEFINE([HAVE_VISIBILITY_ATTRIBUTE], [], [Define if gcc visibility attributes supported])
    VISIBILITY_FLAGS=-fvisibility=hidden
  fi
fi

AM_CXXFLAGS="$AM_CXXFLAGS $VISIBILITY_FLAGS"

# Host specific peculiarities...
CYGWIN_WRAPPER=
OMNILINK="-lomniORB4 -lomnithread"
STLLINK=
case "$host" in
*cygwin*|*mks*|*msvc*|*wince|*mingw*)
        STLLINK="-lstlport.5.0 -no-undefined"
        CYGWIN_WRAPPER="${abssrcdir}/build/cygwin-wrapper"
        OMNILINK="-lomniORB407_rt -lomnithread32_rt -lws2_32"
        ;;
esac
AC_SUBST([CYGWIN_WRAPPER])
AC_SUBST([OMNILINK])
AC_SUBST([STLLINK])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([THREADFLAGS])

AC_DEFINE_UNQUOTED(TESTDIR, L"$testdir",
                   [Define TESTDIR to path of test sources.])

AC_CONFIG_FILES([
                 Makefile
                ])
AC_OUTPUT
