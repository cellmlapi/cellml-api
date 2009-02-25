# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libceledsexporter_xpcom_bridge.la

libceledsexporter_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCeLEDSExporter.cpp \
  $(top_builddir)/interfaces/x2pCeLEDSExporter.cpp \
  $(top_srcdir)/CeLEDSExporter/sources/CeLEDSExporterBootstrapXPCOM.cpp

libceledsexporter_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libceledsexporter.la \
  $(top_builddir)/libXPCOMSupport.la

libceledsexporter_xpcom_bridge_la_LDFLAGS = \
  -no-undefined -module

libceledsexporter_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CCGS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCeLEDSExporter.cpp \
  $(top_builddir)/interfaces/x2pCeLEDSExporter.cpp
