lib_LTLIBRARIES += libccgs.la

libccgs_la_LIBADD = \
  libcellml.la \
  libmalaes.la \
  libcuses.la \
  libcevas.la \
  libannotools.la \
  $(STLLINK)

libccgs_la_SOURCES = \
  $(top_srcdir)/CCGS/sources/CCGSImplementation.cpp \
  $(top_srcdir)/CCGS/sources/CCGSGenerator.cpp

libccgs_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/AnnoTools/sources \
  -I$(top_srcdir)/MaLaES/sources \
  -I$(top_srcdir)/CeVAS/sources \
  -I$(top_srcdir)/CUSES/sources \
  -I$(top_srcdir)/sources/rdf \
  -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCCGS.hxx \
  $(top_srcdir)/CCGS/sources/CCGSBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCCGS.hxx

include $(top_srcdir)/CCGS/tests/build.mk
