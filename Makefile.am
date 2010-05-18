# Pre-initialise so we can append...
include_HEADERS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
noinst_PROGRAMS =
bin_PROGRAMS =
BUILT_SOURCES =
CLEANFILES =
EXTRA_LTLIBRARIES =

NULLSTRING :=

CYGWIN_WRAPPER ?= @CYGWIN_WRAPPER@
OMNILINK ?= @OMNILINK@
STLLINK ?= @STLLINK@
LT_DLPREOPEN ?= @LT_DLPREOPEN@
THREADFLAGS ?= @THREADFLAGS@
LIBXML_CFLAGS ?= @LIBXML_CFLAGS@
MOZILLA_DIR ?= @XPCOM@
PRE_NON_AS_NEEDED := @PRE_NON_AS_NEEDED@
POST_NON_AS_NEEDED := @POST_NON_AS_NEEDED@
LIBADD_DL := @lt_cv_dlopen_libs@

# Avoid bogus error...
libcellml_java_bridge_la_SOURCES =
libcellml_java_bridge_la_LIBADD =
cellml_jar_java =
cellml_jar_classes =
pure_java =

# This saves repeating this everywhere...
XPCOM_LDFLAGS =  -no-undefined -module -L$(MOZILLA_DIR)/lib -L$(MOZILLA_DIR)/bin -lxpcomglue_s -lxpcom -lnspr4
XPCOM_LDFLAGS_NOTMOD =  -no-undefined -L$(MOZILLA_DIR)/lib -L$(MOZILLA_DIR)/bin -lxpcomglue_s -lxpcom -lnspr4
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

if ENABLE_CELLML_CORBA_SERVER
include $(top_srcdir)/libltdl/build.mk
include $(top_srcdir)/sources/cellml_corba_server/build.mk
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

if ENABLE_CONTEXT_AND_CORBA_AND_CCGS
include $(top_srcdir)/CCGS/corba_build.mk
endif

if ENABLE_ANNOTOOLS_AND_XPCOM
include $(top_srcdir)/AnnoTools/xpcom_build.mk
endif

if ENABLE_ANNOTOOLS_AND_JAVA
include $(top_srcdir)/AnnoTools/java_build.mk
endif

if ENABLE_CEVAS_AND_XPCOM
include $(top_srcdir)/CeVAS/xpcom_build.mk
endif

if ENABLE_CEVAS_AND_JAVA
include $(top_srcdir)/CeVAS/java_build.mk
endif

if ENABLE_CUSES_AND_XPCOM
include $(top_srcdir)/CUSES/xpcom_build.mk
endif

if ENABLE_CUSES_AND_JAVA
include $(top_srcdir)/CUSES/java_build.mk
endif

if ENABLE_MALAES_AND_XPCOM
include $(top_srcdir)/MaLaES/xpcom_build.mk
endif

if ENABLE_MALAES_AND_JAVA
include $(top_srcdir)/MaLaES/java_build.mk
endif

if ENABLE_VACSS_AND_XPCOM
include $(top_srcdir)/VACSS/xpcom_build.mk
endif

if ENABLE_VACSS_AND_JAVA
include $(top_srcdir)/VACSS/java_build.mk
endif

if ENABLE_CCGS_AND_XPCOM
include $(top_srcdir)/CCGS/xpcom_build.mk
endif

if ENABLE_CCGS_AND_JAVA
include $(top_srcdir)/CCGS/java_build.mk
endif

if ENABLE_CELEDS_AND_XPCOM
include $(top_srcdir)/CeLEDS/xpcom_build.mk
endif

if ENABLE_CELEDS_AND_JAVA
include $(top_srcdir)/CeLEDS/java_build.mk
endif

if ENABLE_CELEDSEXPORT_AND_XPCOM
include $(top_srcdir)/CeLEDSExporter/xpcom_build.mk
endif

if ENABLE_CELEDSEXPORT_AND_JAVA
include $(top_srcdir)/CeLEDSExporter/java_build.mk
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

CLEANFILES += $(top_builddir)/interfaces/*.cxx $(top_builddir)/interfaces/*.cpp $(top_builddir)/interfaces/*.hxx $(top_builddir)/interfaces/*.cc $(top_builddir)/interfaces/*.hh
