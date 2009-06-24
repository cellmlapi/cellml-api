# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcellml_context_xpcom_bridge.la

libcellml_context_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCellML_Context.cpp \
  $(top_builddir)/interfaces/x2pCellML_Context.cpp \
  $(top_srcdir)/sources/cellml_context/CellMLContextBootstrapXPCOM.cpp

libcellml_context_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcellml_context.la \
  $(top_builddir)/libXPCOMSupport.la

libcellml_context_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libcellml_context_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CellMLContext

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCellML_Context.cpp \
  $(top_builddir)/interfaces/x2pCellML_Context.cpp
