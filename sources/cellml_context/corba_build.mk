# CORBA support for CellML context...

lib_LTLIBRARIES += libcellml_context_corba.la
noinst_LTLIBRARIES += libcellml_context_corba_sk.la

libcellml_context_corba_la_LIBADD := \
  libcellml.la \
  libcellml_corba_bridge.la \
  libcellml_context_corba_sk.la $(STLLINK)

libcellml_context_corba_la_SOURCES := \
  $(top_builddir)/interfaces/CCICellML_Context.cxx \
  $(top_builddir)/interfaces/SCICellML_Context.cxx \
  $(top_srcdir)/sources/cellml_corba_server/Client.cpp

libcellml_context_corba_sk_la_SOURCES := \
  $(top_builddir)/interfaces/CellML_ContextSK.cc

libcellml_context_corba_la_CXXFLAGS := \
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
   -I$(top_srcdir)/simple_interface_generators/glue \
  -DMODULE_CONTAINS_CellMLContext
  $(AM_CXXFLAGS)

libcellml_context_corba_sk_la_CXXFLAGS := \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICellML_Context.cxx \
  $(top_builddir)/interfaces/SCICellML_Context.cxx \
  $(top_builddir)/interfaces/CellML_ContextSK.cc
