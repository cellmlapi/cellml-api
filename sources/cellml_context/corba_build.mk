# CORBA support for CellML context...

lib_LTLIBRARIES += libcellml_context_corba_bridge.la libcellml_context_corba_stubs.la

libcellml_context_corba_bridge_la_LIBADD = \
  libcellml_corba_bridge.la \
  libcellml_context_corba_stubs.la $(STLLINK)

libcellml_context_corba_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/CCICellML_Context.cxx \
  $(top_builddir)/interfaces/SCICellML_Context.cxx

libcellml_context_corba_stubs_la_SOURCES = \
  $(top_builddir)/interfaces/CellML_ContextSK.cc

libcellml_context_corba_bridge_la_CXXFLAGS = \
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
   -I$(top_srcdir)/simple_interface_generators/glue \
  -DMODULE_CONTAINS_CellMLContext \
  $(AM_CXXFLAGS)

libcellml_context_corba_stubs_la_CXXFLAGS = \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue

libcellml_context_corba_stubs_la_LIBADD = \
  $(OMNILINK) $(STLLINK) libcellml_corba_stubs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICellML_Context.cxx \
  $(top_builddir)/interfaces/SCICellML_Context.cxx \
  $(top_builddir)/interfaces/CellML_ContextSK.cc
