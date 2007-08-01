lib_LTLIBRARIES += \
  libcuses_corba_bridge.la libcuses_corba_stubs.la

libcuses_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCICUSES.cxx \
  $(top_builddir)/interfaces/SCICUSES.cxx

libcuses_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/CUSESSK.cc

libcuses_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libcuses_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue

libcuses_corba_bridge_la_LIBADD := \
  $(top_builddir)/libcuses_corba_stubs.la \
  $(top_builddir)/libccgs_corba_bridge.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la $(STLLINK)

libcuses_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libcellml_corba_stubs.la \
  $(top_builddir)/libccgs_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICUSES.cxx \
  $(top_builddir)/interfaces/SCICUSES.cxx \
  $(top_builddir)/interfaces/CUSESSK.cc
