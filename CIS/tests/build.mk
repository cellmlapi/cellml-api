noinst_PROGRAMS += RunCellML
RunCellML_SOURCES=$(top_srcdir)/CIS/tests/RunCellML.cpp
RunCellML_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libcis.la \
  -lxml2 -lgsl -lgslcblas -lpthread
RunCellML_LDFLAGS=-O0
RunCellML_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_builddir)/interfaces -I$(top_srcdir)/CCGS/sources -I$(top_srcdir)/CIS/sources -Wall -ggdb -O0
