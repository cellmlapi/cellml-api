lib_LTLIBRARIES += libmalaes.la

libmalaes_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libmalaes_la_SOURCES = \
  $(top_srcdir)/MaLaES/sources/MaLaESImpl.cpp

libmalaes_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_builddir)/interfaces \
  -I$(top_srcdir)/AnnoTools/sources -I$(top_srcdir)/CUSES/sources \
  -I$(top_srcdir)/CeVAS/sources $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceMaLaES.hxx \
  $(top_srcdir)/MaLaES/sources/MaLaESBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceMaLaES.hxx
