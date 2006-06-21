lib_LTLIBRARIES += libccgs_corba.la
bin_PROGRAMS += CCGSService

libccgs_corba_la_SOURCES := \
  $(top_builddir)/interfaces/CCICCGS.cxx \
  $(top_builddir)/interfaces/SCICCGS.cxx \
  $(top_builddir)/interfaces/CCGSSK.cc

CCGSService_SOURCES := \
  $(top_srcdir)/CCGS/sources/CCGSServiceRegistration.cpp

CCGSService_LDADD := \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libccgs_corba.la \
  $(top_builddir)/libcellml_corba_client.la \
  $(top_builddir)/libcellml_context_corba.la \
  $(top_builddir)/libcellml_corba.la \
  $(top_builddir)/libCORBASupport.la

CCGSService_LDFLAGS := -static -lomniORB4

CCGSService_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources/ \
	-I$(top_srcdir)/simple_interface_generators/glue

libccgs_corba_la_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources/ \
	-I$(top_srcdir)/simple_interface_generators/glue

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICCGS.cxx \
  $(top_builddir)/interfaces/SCICCGS.cxx \
  $(top_builddir)/interfaces/CCGSSK.cc
