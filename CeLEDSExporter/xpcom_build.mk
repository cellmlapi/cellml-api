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
  $(XPCOM_LDFLAGS)

libceledsexporter_xpcom_bridge_la_CXXFLAGS = \
  $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CCGS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCeLEDSExporter.cpp \
  $(top_builddir)/interfaces/x2pCeLEDSExporter.cpp
