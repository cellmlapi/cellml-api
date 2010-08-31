lib_LTLIBRARIES += libcuses.la

libcuses_la_LIBADD = \
  libcellml.la \
  libannotools.la \
  $(STLLINK)

libcuses_la_SOURCES = \
  $(top_srcdir)/CUSES/sources/CUSESImpl.cpp

libcuses_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/AnnoTools/sources $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCUSES.hxx \
  $(top_srcdir)/CUSES/sources/CUSESBootstrap.hpp

BUILT_SOURCES += \
  $(top_builddir)/interfaces/IfaceCUSES.hxx
