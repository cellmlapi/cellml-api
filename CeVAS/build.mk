lib_LTLIBRARIES += libcevas.la

libcevas_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libcevas_la_SOURCES = \
  $(top_srcdir)/CeVAS/sources/CeVASImpl.cpp

libcevas_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/AnnoTools/sources $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCeVAS.hxx \
  $(top_srcdir)/CeVAS/sources/CeVASBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCeVAS.hxx
