lib_LTLIBRARIES += libvacss.la

libvacss_la_LIBADD = \
  libcuses.la \
  libcellml.la \
  $(STLLINK)

libvacss_la_SOURCES = \
  $(top_srcdir)/VACSS/sources/VACSSImpl.cpp

libvacss_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_builddir)/interfaces -I$(top_srcdir)/CUSES/sources \
  $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceVACSS.hxx \
  $(top_srcdir)/VACSS/sources/VACSSBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceVACSS.hxx

include $(top_srcdir)/VACSS/tests/build.mk
