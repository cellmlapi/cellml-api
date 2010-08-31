lib_LTLIBRARIES += \
  libceleds_corba_bridge.la libceleds_corba_stubs.la

libceleds_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCICeLEDS.cxx \
  $(top_builddir)/interfaces/SCICeLEDS.cxx

libceleds_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/CeLEDSSK.cc

libceleds_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libceleds_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libceleds_corba_bridge_la_LIBADD := \
  $(top_builddir)/libceleds_corba_stubs.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libmalaes_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la $(STLLINK)

libceleds_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libmalaes_corba_stubs.la \
  $(top_builddir)/libcellml_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICeLEDS.cxx \
  $(top_builddir)/interfaces/SCICeLEDS.cxx \
  $(top_builddir)/interfaces/CeLEDSSK.cc

