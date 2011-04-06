#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AM_CXXFLAGS=

AC_INIT(CellMLAPI, 0.1, [[ak.miller@auckland.ac.nz]])
AC_PREREQ(2.59)

AC_CONFIG_MACRO_DIR([m4])

command -v omniidl >/dev/null
has_omniidl=$?
if test $has_omniidl != 0; then
   AC_MSG_ERROR(omniidl from omniORB is required to build the CellML API)
fi

AC_CONFIG_SRCDIR([sources/cellml/CellMLBootstrap.cpp])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADER([cda_config.h])

AM_INIT_AUTOMAKE([-Wno-portability])
PKG_PROG_PKG_CONFIG

LIBXML2_CONFIG=$(which xml2-config)

AC_ARG_WITH(libxml2,
[  --with-libxml2          xml2-config path and command line (will auto-detect if omitted)],
[LIBXML2_CONFIG=$withval], [])

LIBXML2_CFLAGS=$($LIBXML2_CONFIG --cflags)
CFLAGS="$LIBXML2_CFLAGS $CFLAGS"
AC_CHECK_HEADERS([libxml/tree.h], , [AC_MSG_ERROR(cannot find libxml2 headers - try using --with-libxml2)])

AC_ARG_ENABLE(examples,
[  --enable-examples Build example programs],
[case "${enableval}" in
  yes) enable_examples=true ;;
  no) enable_examples=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-examples) ;;
esac], [enable_examples=true])

AC_ARG_ENABLE(corba,
[  --enable-corba    Build CORBA glue],
[case "${enableval}" in
  yes) corba=true ;;
  no)  corba=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-corba) ;;
esac],[corba=false])
AM_CONDITIONAL(ENABLE_CORBA, test x$corba = xtrue)

AC_ARG_ENABLE(xpcom,
[  --enable-xpcom=path/to/mozilla/dist  Build XPCOM glue],
[case "${enableval}" in
  no) xpcom=false ;;
  *)
  xpcom=$enableval
  # CHECK_SHORT_WCHAR_OPTION
  # AM_CXXFLAGS="$AM_CXXFLAGS $short_wchar_option"
  AC_DEFINE(ENABLE_XPCOM, 1, [Is XPCOM enabled?])
  ;;
esac], [xpcom=false])
AM_CONDITIONAL(ENABLE_XPCOM, test x$xpcom != xfalse)

AC_ARG_ENABLE(java,
[  --enable-java  Build Java bindings],
[case "${enableval}" in
  no) java=false ;;
  *)
  java=$enableval
  # CHECK_SHORT_WCHAR_OPTION
  # AM_CXXFLAGS="$AM_CXXFLAGS $short_wchar_option"
  AC_DEFINE(ENABLE_JAVA, 1, [Is Java enabled?])
  ;;
esac], [java=false])
AM_CONDITIONAL(ENABLE_JAVA, test x$java != xfalse)

AC_CHECK_HEADERS(jni.h JavaVM/jni.h)

PYTHON_CFLAGS=
PYTHON_LDFLAGS=
AC_ARG_ENABLE(python,
[  --enable-python     Build Python bindings],
[case "${enableval}" in
  no) python=false ;;
  *) python=$enableval
  AC_DEFINE(ENABLE_PYTHON, 1, [Is Python enabled?])
  ;;
esac], [python=false])
AM_CONDITIONAL(ENABLE_PYTHON, test x$python != xfalse)

if test x$python != xfalse; then
  PYTHON_CFLAGS=$(python-config --cflags)
  PYTHON_LDFLAGS=$(python-config --ldflags)
fi

AC_ARG_ENABLE(context,
[  --enable-context    Build the CellML Context],
[case "${enableval}" in
  yes) context=true ;;
  no)  context=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-context) ;;
esac],[context=false])
AM_CONDITIONAL(ENABLE_CONTEXT, test x$context = xtrue)

AC_ARG_ENABLE(annotools,
[  --enable-annotools  Build the annotation tools],
[case "${enableval}" in
  yes) annotools=true ;;
  no)  annotools=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-annotools) ;;
esac], [annotools=false])
AM_CONDITIONAL(ENABLE_ANNOTOOLS, test x$annotools = xtrue)

AC_ARG_ENABLE(cuses,
[  --enable-cuses     Build the CellML Units Simplification / Expansion Service],
[case "${enableval}" in
  yes) cuses=true ;;
  no)  cuses=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-cuses) ;;
esac], [cuses=false])
AM_CONDITIONAL(ENABLE_CUSES, test x$cuses = xtrue)

if test x$annotools = xfalse -a x$cuses = xtrue; then
   AC_MSG_ERROR(You must enable AnnoTools if you want CUSES)
fi

AC_ARG_ENABLE(cevas,
[  --enable-cevas     Build the CellML Variable Association Service],
[case "${enableval}" in
  yes) cevas=true ;;
  no)  cevas=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-cevas) ;;
esac], [cevas=false])
AM_CONDITIONAL(ENABLE_CEVAS, test x$cevas = xtrue)

if test x$annotools = xfalse -a x$cevas = xtrue; then
   AC_MSG_ERROR(You must enable AnnoTools if you want CeVAS)
fi

AC_ARG_ENABLE(vacss,
[  --enable-vacss     Build the Validate Against CellML Specification Service.
                      **This option is NOT officially supported** in this
                      version and does not yet perform full validation],
[case "${enableval}" in
  yes) vacss=true ;;
  no) vacss=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-vacss) ;;
esac], [vacss=false])
AM_CONDITIONAL(ENABLE_VACSS, test x$vacss = xtrue)

AC_ARG_ENABLE(malaes,
[  --enable-malaes   Build the MathML to Language Expression Service],
[case "${enableval}" in
  yes) malaes=true ;;
  no) malaes=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-malaes) ;;
esac], [malaes=false])
AM_CONDITIONAL(ENABLE_MALAES, test x$malaes = xtrue)

if test x$cuses = xfalse -a x$malaes = xtrue; then
   AC_MSG_ERROR(You must enable CUSES if you want MaLaES)
fi
if test x$cevas = xfalse -a x$malaes = xtrue; then
   AC_MSG_ERROR(You must enable CeVAS if you want MaLaES)
fi

AC_ARG_ENABLE(ccgs,
[  --enable-ccgs    Build the CellML C Code Generation Service],
[case "${enableval}" in
  yes) ccgs=true ;;
  no)  ccgs=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-ccgs) ;;
esac],[ccgs=false])
AM_CONDITIONAL(ENABLE_CCGS, test x$ccgs = xtrue)

AC_ARG_ENABLE(celeds,
[  --enable-celeds    Build the CellML Language Export Definition Service],
[case "${enableval}" in
  yes) celeds=true ;;
  no)  celeds=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-celeds) ;;
esac],[celeds=false])
AM_CONDITIONAL(ENABLE_CELEDS, test x$celeds = xtrue)

AC_ARG_ENABLE(rdf,
[  --enable-rdf    Build the CellML low level RDF Service],
[case "${enableval}" in
  yes) rdf=true ;;
  no)  rdf=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-rdf) ;;
esac],[rdf=false])
if test x$rdf = xtrue; then
  AC_DEFINE(ENABLE_RDF, 1, [Is RDF enabled?])
fi
AM_CONDITIONAL(ENABLE_RDF, test x$rdf = xtrue)

if test x$malaes = xfalse -a x$ccgs = xtrue; then
   AC_MSG_ERROR(You must enable MaLaES if you want CCGS)
fi

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

AC_ARG_ENABLE(gsl-integrators,
[  --enable-gsl-integrators Build integrators that use GSL (incompatible with non-GPL code)],
[case "${enableval}" in
  no) gslintegrators=false ;;
  *) gslintegrators=true
  AC_DEFINE(ENABLE_GSL_INTEGRATORS, 1, [Are the GSL integrators enabled?])
  ;;
esac], [gslintegrators=false])
AM_CONDITIONAL(ENABLE_GSL_INTEGRATORS, test x$gslintegrators = xtrue -a x$cis = xtrue)

if test x$ccgs = xfalse -a x$cis = xtrue; then
   AC_MSG_ERROR(You must enable CCGS if you want CIS)
fi

AC_ARG_ENABLE(telicems,
[  --enable-telicems Enable the TeLiCeM (Text-based Language for the Input of CellML Models) Service],
[case "${enableval}" in
  no) telicems=false ;;
  *) telicems=true
     AC_DEFINE(ENABLE_TELICEMS, 1, [Is TeLiCeMs enabled?])
  ;;
esac], [telicems=false])
AM_CONDITIONAL(ENABLE_TELICEMS, test x$telicems = xtrue)

AC_ARG_ENABLE(spros,
[  --enable-spros    Enable the SED-ML Processing Service (SProS)],
[case "${enableval}" in
  no) spros=false ;;
  *) spros=true
     AC_DEFINE(ENABLE_SPROS, 1, [Is SProS enabled?])
  ;;
esac], [spros=false])
AM_CONDITIONAL(ENABLE_SPROS, test x$spros = xtrue)

celedsexport=false
if test x$ccgs = xtrue; then
  if test x$celeds = xtrue; then
    celedsexport=true
  fi
fi
AM_CONDITIONAL(ENABLE_CELEDSEXPORT, test x$celedsexport = xtrue)

# This is verbose but it should (I hope) be portable...
contextandcorba=false
contextandxpcom=false
contextandjava=false
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
    if test x$annotools = xtrue; then
      contextandcorbaandannotools=true
    fi
    if test x$cevas = xtrue; then
      contextandcorbaandcevas=true
    fi
    if test x$cuses = xtrue; then
      contextandcorbaandcuses=true
    fi
    if test x$malaes = xtrue; then
      contextandcorbaandmalaes=true
    fi
    if test x$celeds = xtrue; then
      contextandcorbaandceleds=true
    fi
    if test x$celedsexport = xtrue; then
      contextandcorbaandceledsexport=true
    fi
    if test x$vacss = xtrue; then
      contextandcorbaandvacss=true
    fi
    if test x$rdf = xtrue; then
      contextandcorbaandrdf=true
    fi
  fi
  if test x$xpcom != xfalse; then
    contextandxpcom=true
  fi
  if test x$java != xfalse; then
    contextandjava=true
  fi
fi

annotoolsandxpcom=false
cevasandxpcom=false
cusesandxpcom=false
malaesandxpcom=false
ccgsandxpcom=false
celedsandxpcom=false
celedsexportandxpcom=false
cisandxpcom=false
vacssandxpcom=false
rdfandxpcom=false

annotoolsandjava=false
cevasandjava=false
cusesandjava=false
malaesandjava=false
ccgsandjava=false
celedsandjava=false
celedexportsandjava=false
cisandjava=false
vacssandjava=false
rdfandjava=false

if test x$xpcom != xfalse; then
   if test x$annotools = xtrue; then
      annotoolsandxpcom=true
   fi

   if test x$cevas = xtrue; then
      cevasandxpcom=true
   fi

   if test x$cuses = xtrue; then
      cusesandxpcom=true
   fi

   if test x$malaes = xtrue; then
      malaesandxpcom=true
   fi

   if test x$ccgs = xtrue; then
      ccgsandxpcom=true
   fi

   if test x$celeds = xtrue; then
      celedsandxpcom=true
   fi

   if test x$celedsexport = xtrue; then
      celedsexportandxpcom=true
   fi

   if test x$cis = xtrue; then
      cisandxpcom=true
   fi

   if test x$vacss = xtrue; then
      vacssandxpcom=true
   fi
   if test x$rdf = xtrue; then
      rdfandxpcom=true
   fi
   if test x$telicems = xtrue; then
      telicemsandxpcom=true
   fi
   if test x$spros = xtrue; then
      sprosandxpcom=true
   fi
fi

AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA, test x$contextandcorba = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_XPCOM, test x$contextandxpcom = xtrue)

AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CCGS,
               test x$contextandcorbaandccgs = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CIS,
               test x$contextandcorbaandcis = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_ANNOTOOLS,
               test x$contextandcorbaandannotools = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CEVAS,
               test x$contextandcorbaandcevas = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CUSES,
               test x$contextandcorbaandcuses = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_MALAES,
               test x$contextandcorbaandmalaes = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CELEDS,
               test x$contextandcorbaandceleds = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_CELEDSEXPORT,
               test x$contextandcorbaandceledsexport = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_VACSS,
               test x$contextandcorbaandvacss = xtrue)
AM_CONDITIONAL(ENABLE_CONTEXT_AND_CORBA_AND_RDF,
               test x$contextandcorbaandrdf = xtrue)

AM_CONDITIONAL(ENABLE_ANNOTOOLS_AND_XPCOM, test x$annotoolsandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_CEVAS_AND_XPCOM, test x$cevasandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_CUSES_AND_XPCOM, test x$cusesandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_MALAES_AND_XPCOM, test x$malaesandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_CCGS_AND_XPCOM, test x$ccgsandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_CELEDS_AND_XPCOM, test x$celedsandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_CELEDSEXPORT_AND_XPCOM, test x$celedsexportandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_CIS_AND_XPCOM, test x$cisandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_VACSS_AND_XPCOM, test x$vacssandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_RDF_AND_XPCOM, test x$rdfandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_TELICEMS_AND_XPCOM, test x$telicemsandxpcom = xtrue)
AM_CONDITIONAL(ENABLE_SPROS_AND_XPCOM, test x$sprosandxpcom = xtrue)

if test x$java != xfalse; then
   if test x$annotools = xtrue; then
      annotoolsandjava=true
   fi

   if test x$cevas = xtrue; then
      cevasandjava=true
   fi

   if test x$cuses = xtrue; then
      cusesandjava=true
   fi

   if test x$malaes = xtrue; then
      malaesandjava=true
   fi

   if test x$ccgs = xtrue; then
      ccgsandjava=true
   fi

   if test x$celeds = xtrue; then
      celedsandjava=true
   fi

   if test x$celedsexport = xtrue; then
      celedsexportandjava=true
   fi

   if test x$cis = xtrue; then
      cisandjava=true
   fi

   if test x$vacss = xtrue; then
      vacssandjava=true
   fi

   if test x$rdf = xtrue; then
      rdfandjava=true
   fi

   if test x$telicems = xtrue; then
      telicemsandjava=true
   fi

   if test x$spros = xtrue; then
      sprosandjava=true
   fi
fi

AM_CONDITIONAL(ENABLE_CONTEXT_AND_JAVA, test x$contextandjava = xtrue)
AM_CONDITIONAL(ENABLE_ANNOTOOLS_AND_JAVA, test x$annotoolsandjava = xtrue)
AM_CONDITIONAL(ENABLE_CEVAS_AND_JAVA, test x$cevasandjava = xtrue)
AM_CONDITIONAL(ENABLE_CUSES_AND_JAVA, test x$cusesandjava = xtrue)
AM_CONDITIONAL(ENABLE_MALAES_AND_JAVA, test x$malaesandjava = xtrue)
AM_CONDITIONAL(ENABLE_CCGS_AND_JAVA, test x$ccgsandjava = xtrue)
AM_CONDITIONAL(ENABLE_CELEDS_AND_JAVA, test x$celedsandjava = xtrue)
AM_CONDITIONAL(ENABLE_CELEDSEXPORT_AND_JAVA, test x$celedsexportandjava = xtrue)
AM_CONDITIONAL(ENABLE_CIS_AND_JAVA, test x$cisandjava = xtrue)
AM_CONDITIONAL(ENABLE_VACSS_AND_JAVA, test x$vacssandjava = xtrue)
AM_CONDITIONAL(ENABLE_RDF_AND_JAVA, test x$rdfandjava = xtrue)
AM_CONDITIONAL(ENABLE_TELICEMS_AND_JAVA, test x$telicemsandjava = xtrue)
AM_CONDITIONAL(ENABLE_SPROS_AND_JAVA, test x$sprosandjava = xtrue)

if test x$python != xfalse; then
   if test x$annotools = xtrue; then
      annotoolsandpython=true
   fi

   if test x$cevas = xtrue; then
      cevasandpython=true
   fi

   if test x$cuses = xtrue; then
      cusesandpython=true
   fi

   if test x$malaes = xtrue; then
      malaesandpython=true
   fi

   if test x$ccgs = xtrue; then
      ccgsandpython=true
   fi

   if test x$celeds = xtrue; then
      celedsandpython=true
   fi

   if test x$celedsexport = xtrue; then
      celedsexportandpython=true
   fi

   if test x$cis = xtrue; then
      cisandpython=true
   fi

   if test x$vacss = xtrue; then
      vacssandpython=true
   fi

   if test x$rdf = xtrue; then
      rdfandpython=true
   fi

   if test x$telicems = xtrue; then
      telicemsandpython=true
   fi

   if test x$spros = xtrue; then
      sprosandpython=true
   fi
fi

AM_CONDITIONAL(ENABLE_CONTEXT_AND_PYTHON, test x$contextandpython = xtrue)
AM_CONDITIONAL(ENABLE_ANNOTOOLS_AND_PYTHON, test x$annotoolsandpython = xtrue)
AM_CONDITIONAL(ENABLE_CEVAS_AND_PYTHON, test x$cevasandpython = xtrue)
AM_CONDITIONAL(ENABLE_CUSES_AND_PYTHON, test x$cusesandpython = xtrue)
AM_CONDITIONAL(ENABLE_MALAES_AND_PYTHON, test x$malaesandpython = xtrue)
AM_CONDITIONAL(ENABLE_CCGS_AND_PYTHON, test x$ccgsandpython = xtrue)
AM_CONDITIONAL(ENABLE_CELEDS_AND_PYTHON, test x$celedsandpython = xtrue)
AM_CONDITIONAL(ENABLE_CELEDSEXPORT_AND_PYTHON, test x$celedsexportandpython = xtrue)
AM_CONDITIONAL(ENABLE_CIS_AND_PYTHON, test x$cisandpython = xtrue)
AM_CONDITIONAL(ENABLE_VACSS_AND_PYTHON, test x$vacssandpython = xtrue)
AM_CONDITIONAL(ENABLE_RDF_AND_PYTHON, test x$rdfandpython = xtrue)
AM_CONDITIONAL(ENABLE_TELICEMS_AND_PYTHON, test x$telicemsandpython = xtrue)
AM_CONDITIONAL(ENABLE_SPROS_AND_PYTHON, test x$sprosandpython = xtrue)

AM_CONDITIONAL(ENABLE_EXAMPLES, test x$enable_examples = xtrue)

# Checks for programs.
AC_PROG_CXX
AC_LANG(C)
AC_LANG(C++)

# Set up libtool...
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_LIBTOOL_CXX
library_names_spec=$(echo $library_names_spec | tr " " "\n" | egrep -v \.lib$ | xargs echo)

AC_SUBST([LIBTOOL_DEPS])

# Checks for types...
AC_TYPE_INT8_T
AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for libraries.
# PKG_CHECK_MODULES([GLIB], [glib-2.0])
# PKG_CHECK_MODULES([GDOME], [gdome2])

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

case "$build" in
*-cygwin*|*-mingw*|*-msvc*|*-mks*|*w32*)
    testdir=`$srcdir/build/cygwin-wrapper echo $testdir`
    ;;
esac

VISIBILITY_FLAGS=
EXTRA_OPTIMISATION=
dnl Borrowed from glibc configure.in
dnl ===============================================================
if test "$GCC" = "yes"; then
  AC_CACHE_CHECK(for visibility(hidden) attribute,
                 ac_cv_visibility_hidden,
                 [cat > conftest.cpp <<EOF
		  class MyClass;
		  class __attribute__((visibility("default"))) MyClass
		  {
		    MyClass() {}
		    virtual ~MyClass() {}
		  };
                  int foo __attribute__ ((visibility ("hidden"))) = 1;
EOF
                  ac_cv_visibility_hidden=no
                  if ${CXX} -Werror -S conftest.cpp -o conftest.s >/dev/null 2>&1; then
                    if grep '\.hidden.*foo' conftest.s >/dev/null; then
                      ac_cv_visibility_hidden=yes
                    fi
                  fi
                  rm -f conftest.cpp conftest.s
                 ])
  if test "$ac_cv_visibility_hidden" = "yes"; then
    AC_DEFINE([HAVE_VISIBILITY_ATTRIBUTE], [], [Define if gcc visibility attributes supported])
    # Visibility and CORBA don't work together correctly.
    if test x$corba != xfalse; then
      VISIBILITY_FLAGS=
    else
      VISIBILITY_FLAGS=-fvisibility=hidden
    fi
  fi
  EXTRA_OPTIMISATION="-O2 -ffast-math"
fi

AM_CXXFLAGS="$AM_CXXFLAGS $VISIBILITY_FLAGS"

CXXFLAGS="$CXXFLAGS $EXTRA_OPTIMISATION"
CFLAGS="$CFLAGS $EXTRA_OPTIMISATION"

# Host specific peculiarities...
CYGWIN_WRAPPER=
OMNILINK="-lomniORB4 -lomnithread"
STLLINK=
LIBXML_CFLAGS=

PRE_NON_AS_NEEDED="l"
POST_NON_AS_NEEDED=""
NO_AS_NEEDED=""

if test "$with_gnu_ld" = yes; then
  # Catch unresolved symbols on all platforms, because they will break the
  # build when we try on Win32.
  STLLINK="$STLLINK -Wl,--unresolved-symbols=ignore-in-shared-libs"

  NO_AS_NEEDED="-Wl,-no-as-needed"
  PRE_NON_AS_NEEDED="Wl,-no-as-needed -Wl,-l"
  POST_NON_AS_NEEDED="\$(NULLSTRING) -Wl,-as-needed"
fi

case "$host-$CXX" in
*msvc*|*wince*)
	STLLINK=-no-undefined
	OMNILINK="-lomniORB410_rt -lomnithread32_rt -lws2_32"
        CYGWIN_WRAPPER="\"${abssrcdir}/build/cygwin-wrapper\""
	;;
*cygwin*|*mks*|*mingw*)
        case "$build" in
        *cygwin*|*mks*|*msvc*|*wince|*mingw*)
                STLLINK="-lstlport.5.0 -no-undefined"
                CYGWIN_WRAPPER="${abssrcdir}/build/cygwin-wrapper"
                OMNILINK="-lomniORB407_rt -lomnithread32_rt -lws2_32"
                ;;
        *)
                STLLINK=-no-undefined
                OMNILINK="-lomniORB407_rt -lomnithread32_rt -lws2_32"
                ;;
        esac
        ;;
*darwin*)
        LIBXML_CFLAGS=`$LIBXML2_CONFIG --cflags`
        if [[ x$xpcom != xfalse ]]; then
                STLLINK="$STLLINK -L$xpcom/lib -L$xpcom/bin -Wl,-executable_path,$xpcom/bin"
        fi
        ;;
*linux*)
        LIBXML_CFLAGS=`$LIBXML2_CONFIG --cflags`
        STLLINK="$STLLINK -lpthread"
        ;;
*)
        LIBXML_CFLAGS=`$LIBXML2_CONFIG --cflags`
esac

if test x$cis = xtrue -a x$gslintegrators = xtrue; then
     LIBS=
     AC_CHECK_LIB([gslcblas],[cblas_dgemm], [], [AC_MSG_ERROR(GSL CBLAS library required)])
     LIBS="$NO_AS_NEEDED -lgslcblas"
     AC_CHECK_LIB([gsl],[gsl_blas_dgemm], [], [AC_MSG_ERROR(GSL library required)])
     LIBS=
fi

AC_CHECK_TOOL([FLEX], [flex], [no])
if test x$FLEX = xno; then
   AC_MSG_ERROR(Flex is required to build the CellML API)
fi
AC_CHECK_TOOL([BISON], [bison], [no])
if test x$BISON = xno; then
   AC_MSG_ERROR(Bison is required to build the CellML API)
fi

AC_ARG_VAR([FLEX], [The Flex lexer generator to use])
AC_ARG_VAR([BISON], [The Bison parser generator to use])

XPCOM="$xpcom"
AC_SUBST([XPCOM])
AC_SUBST([CYGWIN_WRAPPER])
AC_SUBST([OMNILINK])
AC_SUBST([STLLINK])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([THREADFLAGS])
AC_SUBST([LIBXML_CFLAGS])
AC_SUBST([PYTHON_CFLAGS])
AC_SUBST([PYTHON_LDFLAGS])
AC_SUBST([PRE_NON_AS_NEEDED])
AC_SUBST([POST_NON_AS_NEEDED])
AC_SUBST([lt_cv_dlopen_libs])
AC_SUBST([libext])
AC_SUBST([shrext_cmds])

AC_DEFINE_UNQUOTED(TESTDIR, L"$testdir",
                   [Define TESTDIR to path of test sources.])
AC_DEFINE_UNQUOTED(TESTDIR8, "$testdir",
                   [Define TESTDIR to path of test sources.])

AC_CONFIG_FILES([
                 Makefile
                ])
AC_OUTPUT
