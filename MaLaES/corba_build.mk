lib_LTLIBRARIES += \
  libmalaes_corba_bridge.la libmalaes_corba_stubs.la

libmalaes_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCIMaLaES.cxx \
  $(top_builddir)/interfaces/SCIMaLaES.cxx

libmalaes_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/MaLaESSK.cc

libmalaes_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libmalaes_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue

libmalaes_corba_bridge_la_LIBADD := \
  $(top_builddir)/libmalaes_corba_stubs.la \
  $(top_builddir)/libcuses_corba_bridge.la \
  $(top_builddir)/libcevas_corba_bridge.la \
  $(top_builddir)/libannotools_corba_bridge.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la $(STLLINK)

libmalaes_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libcevas_corba_stubs.la \
  $(top_builddir)/libcuses_corba_stubs.la \
  $(top_builddir)/libannotools_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCIMaLaES.cxx \
  $(top_builddir)/interfaces/SCIMaLaES.cxx \
  $(top_builddir)/interfaces/MaLaESSK.cc
