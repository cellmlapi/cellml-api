# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcellml_context_xpcom_bridge.la

libcellml_context_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCellML_Context.cpp \
  $(top_builddir)/interfaces/x2pCellML_Context.cpp \
  $(top_srcdir)/sources/cellml_context/CellMLContextBootstrapXPCOM.cpp

libcellml_context_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcellml_context.la \
  $(top_builddir)/libcellml_xpcom_bridge.la \
  $(top_builddir)/libXPCOMSupport.la

libcellml_context_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libcellml_context_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CellMLContext

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCellML_Context.cpp \
  $(top_builddir)/interfaces/x2pCellML_Context.cpp
