# CORBA support for CellML context...

lib_LTLIBRARIES += libcellml_context_corba.la

libcellml_context_corba_la_SOURCES := \
  $(top_builddir)/interfaces/CCICellML-Context.cxx \
  $(top_builddir)/interfaces/SCICellML-Context.cxx \
  $(top_builddir)/interfaces/CellML-ContextSK.cc

libcellml_context_corba_la_CXXFLAGS := \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue
