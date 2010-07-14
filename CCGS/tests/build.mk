# Please note that CellML2C is included as a testing program to help test the
# API, and not as a general purpose tool. Please carefully consider whether
# adding a new CellML2* program is justified on the grounds of testing the API
# more extensively before doing so; if your only reason is you want to provide
# a command line tool for another language, please do so in a separate package
# instead. See also:
#  [1] https://tracker.physiomeproject.org/show_bug.cgi?id=1279
#  [2] https://tracker.physiomeproject.org/show_bug.cgi?id=1514#c16
#  [3] http://www.cellml.org/pipermail/cellml-tools-developers/2009-January/000140.html

if ENABLE_EXAMPLES
bin_PROGRAMS += CellML2C CellML2Python CellML2Matlab CustomGen
endif

CellML2C_SOURCES=$(top_srcdir)/CCGS/tests/CellML2C.cpp
CellML2C_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la -lxml2
CellML2C_LDFLAGS=-O0
CellML2C_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/AnnoTools/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/CCGS/sources -Wall -O0 $(AM_CXXFLAGS)

CustomGen_SOURCES=$(top_srcdir)/CCGS/tests/CustomGen.cpp
CustomGen_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la -lxml2
CustomGen_LDFLAGS=-O0
CustomGen_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/AnnoTools/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/CCGS/sources -Wall -O0 $(AM_CXXFLAGS)

CellML2Python_SOURCES=$(top_srcdir)/CCGS/tests/CellML2Python.cpp
CellML2Python_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la -lxml2
CellML2Python_LDFLAGS=-O0
CellML2Python_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/AnnoTools/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/CCGS/sources -I$(top_srcdir)/MaLaES/sources -Wall -O0 $(AM_CXXFLAGS)
CellML2Matlab_SOURCES=$(top_srcdir)/CCGS/tests/CellML2Matlab.cpp
CellML2Matlab_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la -lxml2
CellML2Matlab_LDFLAGS=-O0
CellML2Matlab_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/AnnoTools/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/CCGS/sources -I$(top_srcdir)/MaLaES/sources -Wall -O0 $(AM_CXXFLAGS)
