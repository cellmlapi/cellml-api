lib_LTLIBRARIES += libtelicems.la

libtelicems_la_LIBADD = \
  libcellml.la \
  $(STLLINK)

libtelicems_la_SOURCES = \
  $(top_srcdir)/TeLICeMS/sources/TeLICeMSImpl.cpp \
  $(top_builddir)/TeLICeMParse.tab.cc \
  $(top_builddir)/TeLICeMScanner.cpp

$(top_builddir)/%.tab.cc $(top_builddir)/%.tab.hh: $(top_srcdir)/TeLICeMS/sources/%.yy
	$(BISON) $^

$(top_builddir)/TeLICeMScanner.cpp: $(top_srcdir)/TeLICeMS/sources/TeLICeMScan.l
	$(FLEX) $^

libtelicems_la_CXXFLAGS = \
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/TeLICeMS/sources -I$(top_builddir)/interfaces \
  $(AM_CXXFLAGS)

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceTeLICeMS.hxx \
  $(top_srcdir)/TeLICeMS/sources/TeLICeMService.hpp

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceTeLICeMS.hxx \
	$(top_builddir)/TeLICeMParse.tab.cc \
	$(top_builddir)/TeLICeMParse.tab.hh \
	$(top_builddir)/TeLICeMScanner.cpp

# include $(top_srcdir)/TeLICeMS/tests/build.mk
