lib_LTLIBRARIES += \
  libcis_corba_bridge.la libcis_corba_stubs.la libCISService.la

libcis_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCICIS.cxx \
  $(top_builddir)/interfaces/SCICIS.cxx

libcis_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/CISSK.cc

libCISService_la_SOURCES := \
  $(top_srcdir)/CIS/sources/CISServiceRegistration.cpp

libCISService_la_LDFLAGS := -shared -module -no-undefined $(OMNILINK) -lgsl -lgslcblas

libCISService_la_LIBADD := \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libcellml_corba_stubs.la \
  $(top_builddir)/libcis.la \
  $(top_builddir)/libcis_corba_bridge.la \
  $(top_builddir)/libcis_corba_stubs.la \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libccgs_corba_bridge.la \
  $(top_builddir)/libccgs_corba_stubs.la

libCISService_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libcis_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libcis_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue \
        $(AM_CXXFLAGS)

libcis_corba_bridge_la_LIBADD := \
  $(top_builddir)/libcis_corba_stubs.la \
  $(top_builddir)/libccgs_corba_bridge.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la $(STLLINK)

libcis_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libcellml_corba_stubs.la \
  $(top_builddir)/libccgs_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICIS.cxx \
  $(top_builddir)/interfaces/SCICIS.cxx \
  $(top_builddir)/interfaces/CISSK.cc
