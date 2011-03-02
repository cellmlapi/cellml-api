lib_LTLIBRARIES += libspros.la

libspros_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libspros_la_SOURCES = \
  $(top_srcdir)/SProS/sources/SProSImpl.cpp

libspros_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/SProS/sources -I$(top_builddir)/interfaces \
  $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceSProS.hxx \
  $(top_srcdir)/SProS/sources/SProSBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceSProS.hxx
