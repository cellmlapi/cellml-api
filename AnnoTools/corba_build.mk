lib_LTLIBRARIES += \
  libannotools_corba_bridge.la libannotools_corba_stubs.la

libannotools_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCIAnnoTools.cxx \
  $(top_builddir)/interfaces/SCIAnnoTools.cxx

libannotools_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/AnnoToolsSK.cc

libannotools_corba_bridge_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libannotools_corba_stubs_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue \
	$(AM_CXXFLAGS)

libannotools_corba_bridge_la_LIBADD := \
  $(top_builddir)/libannotools_corba_stubs.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libCORBASupport.la $(STLLINK)

libannotools_corba_stubs_la_LIBADD := \
  $(OMNILINK) \
  $(STLLINK) \
  $(top_builddir)/libcellml_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCIAnnoTools.cxx \
  $(top_builddir)/interfaces/SCIAnnoTools.cxx \
  $(top_builddir)/interfaces/AnnoToolsSK.cc
