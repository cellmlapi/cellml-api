lib_LTLIBRARIES += libcis.la
libcis_la_SOURCES := \
  $(top_srcdir)/CIS/sources/CISImplementation.cxx \
  $(top_srcdir)/CIS/sources/CISSolve.cxx

libcis_la_LIBADD := \
  $(top_builddir)/libccgs.la $(STLLINK)

libcis_la_CXXFLAGS := \
  -Wall -ggdb -I$(top_srcdir)/sources -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceCIS.hxx \
  $(top_srcdir)/CIS/sources/CISBootstrap.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceCIS.hxx

include $(top_srcdir)/CIS/tests/build.mk
