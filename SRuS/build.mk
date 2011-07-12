lib_LTLIBRARIES += libsrus.la

libsrus_la_LIBADD = \
  libcellml.la \
  libcis.la \
  libxpath.la \
  $(STLLINK)

libsrus_la_SOURCES = \
  $(top_srcdir)/SRuS/SRuSImpl.cpp

libsrus_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/SProS/sources \
  -I$(top_srcdir)/CIS/sources \
  -I$(top_srcdir)/SRuS/ -I$(top_builddir)/interfaces -I$(top_builddir)/xpath \
  $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceSProS.hxx \
  $(top_srcdir)/SRuS/SRuSBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceSRuS.hxx

include $(top_srcdir)/SRuS/tests/build.mk
