lib_LTLIBRARIES += libvacss.la

libvaccs_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libvacss_la_SOURCES = \
  $(top_srcdir)/VACSS/sources/VACSSImpl.cpp

libvacss_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_builddir)/interfaces \
  $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceVACSS.hxx \
  $(top_srcdir)/VACSS/sources/VACSSBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceVACSS.hxx
