lib_LTLIBRARIES += libceleds.la

libceleds_la_LIBADD = \
  libcellml.la \
  libmalaes.la \
  $(STLLINK)

libceleds_la_SOURCES = \
  $(top_srcdir)/CeLEDS/sources/CeLEDSImpl.cpp

libceleds_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/cellml \
  -I$(top_srcdir)/MaLaES/sources \
  -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCeLEDS.hxx \
  $(top_srcdir)/CeLEDS/sources/CeLEDSBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCeLEDS.hxx
