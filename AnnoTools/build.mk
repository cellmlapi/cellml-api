lib_LTLIBRARIES += libannotools.la

libannotools_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libannotools_la_SOURCES = \
  $(top_srcdir)/AnnoTools/sources/ATImplementation.cpp

libannotools_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceAnnoTools.hxx \
  $(top_srcdir)/AnnoTools/sources/AnnoToolsBootstrap.hpp

BUILT_SOURCES += \
  $(top_builddir)/interfaces/IfaceAnnoTools.hxx
