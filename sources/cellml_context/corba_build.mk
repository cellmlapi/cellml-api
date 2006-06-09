# CORBA support for CellML context...

lib_LTLIBRARIES += libcellml_context_corba.la

libcellml_context_corba_la_SOURCES := \
  $(top_builddir)/interfaces/CCICellML_Context.cxx \
  $(top_builddir)/interfaces/SCICellML_Context.cxx \
  $(top_builddir)/interfaces/CellML_ContextSK.cc \
  $(top_srcdir)/sources/cellml_corba_server/Client.cpp

libcellml_context_corba_la_CXXFLAGS := \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICellML_Context.cxx \
  $(top_builddir)/interfaces/SCICellML_Context.cxx \
  $(top_builddir)/interfaces/CellML_ContextSK.cc
