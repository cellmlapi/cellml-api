lib_LTLIBRARIES += \
  libcevas_corba_bridge.la libcevas_corba_stubs.la

libcevas_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCICeVAS.cxx \
  $(top_builddir)/interfaces/SCICeVAS.cxx

libcevas_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/CeVASSK.cc

libcevas_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libcevas_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue

libcevas_corba_bridge_la_LIBADD := \
  $(top_builddir)/libcevas_corba_stubs.la \
  $(top_builddir)/libccgs_corba_bridge.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la $(STLLINK)

libcevas_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libcellml_corba_stubs.la \
  $(top_builddir)/libccgs_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICeVAS.cxx \
  $(top_builddir)/interfaces/SCICeVAS.cxx \
  $(top_builddir)/interfaces/CeVASSK.cc
