# Pre-initialise so we can append...
include_HEADERS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
noinst_PROGRAMS =
bin_PROGRAMS =
BUILT_SOURCES =
CLEANFILES =
EXTRA_LTLIBRARIES =
PYTHON_BOOTSTRAPS =

ACLOCAL_AMFLAGS = -I m4

NULLSTRING :=

CYGWIN_WRAPPER ?= @CYGWIN_WRAPPER@
OMNILINK ?= @OMNILINK@
STLLINK ?= @STLLINK@
LT_DLPREOPEN ?= @LT_DLPREOPEN@
THREADFLAGS ?= @THREADFLAGS@
LIBXML_CFLAGS ?= @LIBXML_CFLAGS@
PYTHON_CFLAGS ?= @PYTHON_CFLAGS@
PYTHON_LDFLAGS ?= @PYTHON_LDFLAGS@
MOZILLA_DIR ?= @XPCOM@
PRE_NON_AS_NEEDED := @PRE_NON_AS_NEEDED@
POST_NON_AS_NEEDED := @POST_NON_AS_NEEDED@
LIBADD_DL := @lt_cv_dlopen_libs@

FLEX = @FLEX@
LEX = work-around-unhelpful-automake-error
BISON = @BISON@
YACC = work-around-unhelpful-automake-error


# Avoid bogus error...
libcellml_java_bridge_la_SOURCES =
libcellml_java_bridge_la_LIBADD =
cellml_jar_java =
cellml_jar_classes =
pure_java =

# This saves repeating this everywhere...
XPCOM_LDFLAGS =  -no-undefined -module -L$(MOZILLA_DIR)/lib -L$(MOZILLA_DIR)/bin -lxpcomglue_s -lxpcom -lnspr4
XPCOM_LDFLAGS_NOTMOD =  -no-undefined -L$(MOZILLA_DIR)/lib -L$(MOZILLA_DIR)/bin -lxpcomglue_s -lxpcom -lnspr4
JAVA_LDFLAGS_NOTMOD = -no-undefined 
XPCOM_CXXFLAGS = -I$(MOZILLA_DIR)/include \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/rdf \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS)

# Parts needed to build the interface generates...
include $(top_srcdir)/simple_interface_generators/build.mk

# Parts needed to build and test the core...
include $(top_srcdir)/interfaces/build.mk
include $(top_srcdir)/sources/build.mk
include $(top_srcdir)/tests/build.mk

# These parts relate to CORBA...
if ENABLE_CORBA
include $(top_srcdir)/interfaces/corba_build.mk
endif

if ENABLE_XPCOM
include $(top_srcdir)/interfaces/xpcom_build.mk
endif

if ENABLE_JAVA
include $(top_srcdir)/interfaces/java_build.mk
endif

if ENABLE_PYTHON
include $(top_srcdir)/interfaces/python_build.mk
endif

# These parts relate to the CellML Context...
if ENABLE_CONTEXT
include $(top_srcdir)/sources/cellml_context/build.mk
endif

# CellML Context and also CORBA...
if ENABLE_CONTEXT_AND_CORBA
include $(top_srcdir)/sources/cellml_context/corba_build.mk
endif

if ENABLE_CONTEXT_AND_XPCOM
include $(top_srcdir)/sources/cellml_context/xpcom_build.mk
endif

if ENABLE_RDF
include $(top_srcdir)/sources/rdf/build.mk
endif

if ENABLE_ANNOTOOLS
include $(top_srcdir)/AnnoTools/build.mk
endif

if ENABLE_CUSES
include $(top_srcdir)/CUSES/build.mk
endif

if ENABLE_CEVAS
include $(top_srcdir)/CeVAS/build.mk
endif

if ENABLE_MALAES
include $(top_srcdir)/MaLaES/build.mk
endif

if ENABLE_CCGS
include $(top_srcdir)/CCGS/build.mk
endif

if ENABLE_CELEDS
include $(top_srcdir)/CeLEDS/build.mk
endif

if ENABLE_CELEDSEXPORT
include $(top_srcdir)/CeLEDSExporter/build.mk
endif

if ENABLE_VACSS
include $(top_srcdir)/VACSS/build.mk
endif

if ENABLE_TELICEMS
include $(top_srcdir)/TeLICeMS/build.mk
endif

if ENABLE_SPROS
include $(top_srcdir)/SProS/build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CCGS
include $(top_srcdir)/CCGS/corba_build.mk
endif

if ENABLE_ANNOTOOLS_AND_XPCOM
include $(top_srcdir)/AnnoTools/xpcom_build.mk
endif

if ENABLE_ANNOTOOLS_AND_JAVA
include $(top_srcdir)/AnnoTools/java_build.mk
endif

if ENABLE_ANNOTOOLS_AND_PYTHON
include $(top_srcdir)/AnnoTools/python_build.mk
endif

if ENABLE_CEVAS_AND_XPCOM
include $(top_srcdir)/CeVAS/xpcom_build.mk
endif

if ENABLE_CEVAS_AND_JAVA
include $(top_srcdir)/CeVAS/java_build.mk
endif

if ENABLE_CEVAS_AND_PYTHON
include $(top_srcdir)/CeVAS/python_build.mk
endif


if ENABLE_CUSES_AND_XPCOM
include $(top_srcdir)/CUSES/xpcom_build.mk
endif

if ENABLE_CUSES_AND_JAVA
include $(top_srcdir)/CUSES/java_build.mk
endif

if ENABLE_CUSES_AND_PYTHON
include $(top_srcdir)/CUSES/python_build.mk
endif

if ENABLE_MALAES_AND_XPCOM
include $(top_srcdir)/MaLaES/xpcom_build.mk
endif

if ENABLE_MALAES_AND_JAVA
include $(top_srcdir)/MaLaES/java_build.mk
endif

if ENABLE_MALAES_AND_PYTHON
include $(top_srcdir)/MaLaES/python_build.mk
endif

if ENABLE_VACSS_AND_XPCOM
include $(top_srcdir)/VACSS/xpcom_build.mk
endif

if ENABLE_VACSS_AND_JAVA
include $(top_srcdir)/VACSS/java_build.mk
endif

if ENABLE_VACSS_AND_PYTHON
include $(top_srcdir)/VACSS/python_build.mk
endif

if ENABLE_CCGS_AND_XPCOM
include $(top_srcdir)/CCGS/xpcom_build.mk
endif

if ENABLE_CCGS_AND_JAVA
include $(top_srcdir)/CCGS/java_build.mk
endif

if ENABLE_CCGS_AND_PYTHON
include $(top_srcdir)/CCGS/python_build.mk
endif

if ENABLE_CELEDS_AND_XPCOM
include $(top_srcdir)/CeLEDS/xpcom_build.mk
endif

if ENABLE_CELEDS_AND_JAVA
include $(top_srcdir)/CeLEDS/java_build.mk
endif

if ENABLE_CELEDS_AND_PYTHON
include $(top_srcdir)/CeLEDS/python_build.mk
endif

if ENABLE_CELEDSEXPORT_AND_XPCOM
include $(top_srcdir)/CeLEDSExporter/xpcom_build.mk
endif

if ENABLE_CELEDSEXPORT_AND_JAVA
include $(top_srcdir)/CeLEDSExporter/java_build.mk
endif

if ENABLE_CELEDSEXPORT_AND_PYTHON
include $(top_srcdir)/CeLEDSExporter/python_build.mk
endif

if ENABLE_CIS
include $(top_srcdir)/CIS/build.mk
if ENABLE_GSL_INTEGRATORS
libcis_la_LIBADD += -lgsl
RunCellML_LDADD += -lgsl
RunCellML_LDFLAGS += -$(PRE_NON_AS_NEEDED)gslcblas$(POST_NON_AS_NEEDED)
endif
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CIS
include $(top_srcdir)/CIS/corba_build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CUSES
include $(top_srcdir)/CUSES/corba_build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CEVAS
include $(top_srcdir)/CeVAS/corba_build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_MALAES
include $(top_srcdir)/MaLaES/corba_build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CELEDS
include $(top_srcdir)/CeLEDS/corba_build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_CELEDSEXPORT
include $(top_srcdir)/CeLEDSExporter/corba_build.mk
endif

if ENABLE_CONTEXT_AND_CORBA_AND_ANNOTOOLS
include $(top_srcdir)/AnnoTools/corba_build.mk
endif

if ENABLE_CIS_AND_XPCOM
include $(top_srcdir)/CIS/xpcom_build.mk
endif

if ENABLE_CIS_AND_JAVA
include $(top_srcdir)/CIS/java_build.mk
endif

if ENABLE_CIS_AND_PYTHON
include $(top_srcdir)/CIS/python_build.mk
endif

if ENABLE_TELICEMS_AND_XPCOM
include $(top_srcdir)/TeLICeMS/xpcom_build.mk
endif

if ENABLE_TELICEMS_AND_JAVA
include $(top_srcdir)/TeLICeMS/java_build.mk
endif

if ENABLE_TELICEMS_AND_PYTHON
include $(top_srcdir)/TeLICeMS/python_build.mk
endif

if ENABLE_SPROS_AND_XPCOM
include $(top_srcdir)/SProS/xpcom_build.mk
endif

if ENABLE_SPROS_AND_JAVA
include $(top_srcdir)/SProS/java_build.mk
endif

if ENABLE_SPROS_AND_PYTHON
include $(top_srcdir)/SProS/python_build.mk
endif

CLEANFILES += $(top_builddir)/interfaces/*.cxx $(top_builddir)/interfaces/*.cpp $(top_builddir)/interfaces/*.hxx $(top_builddir)/interfaces/*.cc $(top_builddir)/interfaces/*.hh

nonexistent-filename:
$(top_builddir)/cellml-sdk.tar.bz2: nonexistent-filename
	mkdir -p $(top_builddir)/cellml-sdk/lib
	mkdir -p $(top_builddir)/cellml-sdk/include
	cp $(top_srcdir)/sources/cda_compiler_support.h $(top_builddir)/cellml-sdk/include
	cp $(top_srcdir)/sources/Utilities.hxx $(top_builddir)/cellml-sdk/include
	cp $(top_builddir)/cda_config.h $(top_builddir)/cellml-sdk/include
	cp $(top_builddir)/interfaces/Iface*.hxx $(top_builddir)/cellml-sdk/include
	cp `find $(top_builddir) -iname "*Bootstrap.h??" -and -not -path "*sdk*"` $(top_builddir)/cellml-sdk/include
	cp -R $(top_builddir)/.libs/*$(shrext_cmds)* $(top_builddir)/cellml-sdk/lib
	if [[ `uname` == "Darwin" ]]; then \
		cp -R $(top_builddir)/.libs/*.so $(top_builddir)/cellml-sdk/lib; \
	fi
	LIBFILES=`find "$(top_builddir)/.libs/" -name "*.$(libext)" -and -not -path "*.hg*" -and -not -name "*rdf\.*" -and -not -path "*sdk*" -and -not -name "*only*"`; \
	if test -n "$$LIBFILES"; \
        then \
	  cp $$LIBFILES $(top_builddir)/cellml-sdk/lib/ ; \
	fi
	JARFILES=`find "$(top_builddir)/" -name "*.jar" -and -not -path "*.hg*" -and -not -path "*sdk*"`; \
	if test -n "$$JARFILES"; \
        then \
	  cp $$JARFILES $(top_builddir)/cellml-sdk/lib/ ; \
	fi
	for FN in $(top_builddir)/cellml-sdk/lib/lib*_java_bridge.so; do \
          DIRNAME=$(dirname $FN) ; \
	  BASENAME=$(basename $FN .so) ; \
	  ln -s $BASENAME.so $DIRNAME/$BASENAME.jnilib ; \
	done
	tar -cjf $(top_builddir)/cellml-sdk.tar.bz2 -C $(top_builddir) cellml-sdk
	rm -fr cellml-sdk
