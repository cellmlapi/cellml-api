lib_LTLIBRARIES += libxpath.la

libxpath_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libxpath_la_SOURCES = \
  $(top_srcdir)/xpath/XPathImpl.cpp

libxpath_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/xpath -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/TeLICeMS/sources -I$(top_builddir)/interfaces \
  $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/Ifacexpath.hxx \
  $(top_srcdir)/xpath/XPathBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/Ifacexpath.hxx
