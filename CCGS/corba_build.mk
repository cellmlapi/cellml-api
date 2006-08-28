lib_LTLIBRARIES += libccgs_corba.la libCCGSService.la
noinst_LTLIBRARIES += libccgs_corba_sk.la

libccgs_corba_la_SOURCES := \
  $(top_builddir)/interfaces/CCICCGS.cxx \
  $(top_builddir)/interfaces/SCICCGS.cxx

libccgs_corba_sk_la_SOURCES := \
  $(top_builddir)/interfaces/CCGSSK.cc

libCCGSService_la_SOURCES := \
  $(top_srcdir)/CCGS/sources/CCGSServiceRegistration.cpp

libCCGSService_la_LIBADD := \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libccgs_corba.la
libCCGSService_la_LDFLAGS := -no-undefined -shared -module $(OMNILINK)

libCCGSService_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libccgs_corba_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources \
	-I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libccgs_corba_sk_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir)/simple_interface_generators/glue

libccgs_corba_la_LIBADD := \
  $(top_builddir)/libccgs_corba_sk.la

libccgs_corba_la_LDFLAGS := \
  -no-undefined

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICCGS.cxx \
  $(top_builddir)/interfaces/SCICCGS.cxx \
  $(top_builddir)/interfaces/CCGSSK.cc
