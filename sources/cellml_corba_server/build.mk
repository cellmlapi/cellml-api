# CellML CORBA Server...
bin_PROGRAMS += cellml_corba_server
cellml_corba_server_SOURCES = $(top_srcdir)/sources/cellml_corba_server/Main.cpp
cellml_corba_server_CXXFLAGS = \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces \
  -I $(top_srcdir)/sources/cellml_context \
  -I$(top_srcdir)/libltdl $(AM_CXXFLAGS)
cellml_corba_server_LDADD = \
  $(top_builddir)/libcellml_context.la \
  $(top_builddir)/libcellml_context_corba_bridge.la \
  $(top_builddir)/libcellml_context_corba_stubs.la \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libcellml_corba_bridge.la \
  $(top_builddir)/libcellml_corba_stubs.la \
  $(top_builddir)/libCORBASupport.la \
  $(top_builddir)/libltdlc.la \
  -lxml2 $(OMNILINK) $(STLLINK)

lib_LTLIBRARIES += libcellml_corba_client.la
libcellml_corba_client_la_SOURCES = \
  $(top_srcdir)/sources/Utilities.cpp \
  $(top_srcdir)/sources/cellml_corba_server/Client.cpp
libcellml_corba_client_la_CXXFLAGS = \
  -I $(top_srcdir) \
  -I$(top_srcdir)/sources \
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)
libcellml_corba_client_la_LIBADD = \
  libcellml_context_corba_stubs.la \
  libcellml_context_corba_bridge.la \
  $(OMNILINK) $(STLLINK)
