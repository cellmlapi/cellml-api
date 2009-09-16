if ENABLE_EXAMPLES
bin_PROGRAMS += ValidateCellML
endif

ValidateCellML_SOURCES=$(top_srcdir)/VACSS/tests/ValidateCellML.cpp
ValidateCellML_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libvacss.la \
  -lxml2
ValidateCellML_LDFLAGS=-O0
ValidateCellML_CXXFLAGS=\
  -I$(top_srcdir)/sources \
  -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/mathml \
  -I$(top_srcdir)/sources/cellml \
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir)/VACSS/sources \
  -Wall -O0 $(AM_CXXFLAGS)
