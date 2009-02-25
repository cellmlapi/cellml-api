lib_LTLIBRARIES += libceledsexporter.la

libceledsexporter_la_LIBADD = \
  libcellml.la \
  libccgs.la \
  libceleds.la \
  $(STLLINK)

libceledsexporter_la_SOURCES = \
  $(top_srcdir)/CeLEDSExporter/sources/CeLEDSExporterImpl.cpp

libceledsexporter_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/cellml \
  -I$(top_srcdir)/CeLEDS/sources \
  -I$(top_srcdir)/CCGS/sources \
  -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCeLEDSExporter.hxx \
  $(top_srcdir)/CeLEDSExporter/sources/CeLEDSExporterBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCeLEDSExporter.hxx

include $(top_srcdir)/CeLEDSExporter/tests/build.mk
