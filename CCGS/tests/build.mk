noinst_PROGRAMS += CellML2C
CellML2C_SOURCES=$(top_srcdir)/CCGS/tests/CellML2C.cpp
CellML2C_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la -lxml2 -ldl
CellML2C_LDFLAGS=-O0
CellML2C_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom/ -I$(top_srcdir)/sources/mathml/ -I$(top_srcdir)/sources/cellml -I$(top_builddir)/interfaces -I$(top_srcdir)/CCGS/sources/ -Wall -ggdb -O0 $(AM_CXXFLAGS)
