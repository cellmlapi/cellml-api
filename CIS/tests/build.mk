bin_PROGRAMS += RunCellML
RunCellML_SOURCES=$(top_srcdir)/CIS/tests/RunCellML.cpp
RunCellML_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libcis.la \
  -lxml2 -lgsl $(THREADFLAGS)
RunCellML_LDFLAGS=-O0 -$(PRE_NON_AS_NEEDED)gslcblas$(POST_NON_AS_NEEDED)
RunCellML_CXXFLAGS=\
  -I$(top_srcdir)/sources \
  -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/mathml \
  -I$(top_srcdir)/sources/cellml \
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir)/CCGS/sources \
  -I$(top_srcdir)/CIS/sources \
  -Wall -O0 $(AM_CXXFLAGS)
