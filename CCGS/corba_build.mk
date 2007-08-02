lib_LTLIBRARIES += libccgs_corba_bridge.la libccgs_corba_stubs.la libCCGSService.la

libccgs_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCICCGS.cxx \
  $(top_builddir)/interfaces/SCICCGS.cxx

libccgs_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/CCGSSK.cc

libCCGSService_la_SOURCES := \
  $(top_srcdir)/CCGS/sources/CCGSServiceRegistration.cpp

libCCGSService_la_LIBADD := \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libccgs_corba_bridge.la \
  $(top_builddir)/libccgs_corba_stubs.la

libCCGSService_la_LDFLAGS := -no-undefined -shared -module $(OMNILINK)

libCCGSService_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libccgs_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libccgs_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libccgs_corba_bridge_la_LIBADD := \
  $(top_builddir)/libannotools_corba_bridge.la \
  $(top_builddir)/libcuses_corba_bridge.la \
  $(top_builddir)/libmalaes_corba_bridge.la \
  $(top_builddir)/libccgs_corba_stubs.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la

libccgs_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libannotools_corba_stubs.la \
  $(top_builddir)/libcuses_corba_stubs.la \
  $(top_builddir)/libmalaes_corba_stubs.la \
  $(top_builddir)/libcellml_corba_stubs.la

libccgs_corba_bridge_la_LDFLAGS := \
  -no-undefined

libccgs_corba_stubs_la_LDFLAGS := \
  -no-undefined

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICCGS.cxx \
  $(top_builddir)/interfaces/SCICCGS.cxx \
  $(top_builddir)/interfaces/CCGSSK.cc
