bin_PROGRAMS += testCeLEDS
testCeLEDS_SOURCES=$(top_srcdir)/CeLEDSExporter/tests/testCeLEDS.cpp
testCeLEDS_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libceledsexporter.la -lxml2
testCeLEDS_LDFLAGS=-O0
testCeLEDS_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_builddir)/interfaces -I$(top_srcdir)/CeLEDSExporter/sources -Wall -O0 $(AM_CXXFLAGS)
