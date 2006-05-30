lib_LTLIBRARIES += libccgs.la

libccgs_la_SOURCES := \
  $(top_srcdir)/CCGS/sources/CCGSImplementation.cpp \
  $(top_srcdir)/CCGS/sources/CCGSGenerator.cpp \
  $(top_srcdir)/CCGS/sources/Units.cxx \
  $(top_srcdir)/CCGS/sources/Equality.cxx

libccgs_la_CXXFLAGS := \
  -Wall -ggdb -I$(top_srcdir)/sources -I$(top_builddir)/interfaces

include_HEADERS += $(top_builddir)/interfaces/IfaceCCGS.hxx
BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCCGS.hxx
