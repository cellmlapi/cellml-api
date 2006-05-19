# CellML CORBA Server...
bin_PROGRAMS := cellml_corba_server
cellml_corba_server_SOURCES := $(top_srcdir)/sources/cellml_corba_server/Main.cpp
cellml_corba_server_CXXFLAGS := \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces \
  -I $(top_srcdir)/sources/cellml_context
cellml_corba_server_LDADD := \
  $(top_builddir)/libcellml_context.la \
  $(top_builddir)/libcellml_context_corba.la \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libcellml_corba.la \
  $(top_builddir)/libCORBASupport.la -lxml2

cellml_corba_server_LDFLAGS := -static -lomniORB4
