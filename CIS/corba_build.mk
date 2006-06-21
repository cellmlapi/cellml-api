bin_PROGRAMS += CISService
CISService_SOURCES := \
  $(top_srcdir)/CIS/sources/CISServiceRegistration.cpp \
  $(top_builddir)/interfaces/CCICIS.cxx \
  $(top_builddir)/interfaces/SCICIS.cxx \
  $(top_builddir)/interfaces/CISSK.cc

CISService_LDADD := \
  $(top_builddir)/libccgs_corba.la \
  $(top_builddir)/libcis.la \
  $(top_builddir)/libcellml_corba_client.la \
  $(top_builddir)/libcellml_context_corba.la \
  $(top_builddir)/libcellml_corba.la \
  $(top_builddir)/libCORBASupport.la -lgsl -ldl -lcblas

CISService_LDFLAGS := -static -lomniORB4

CISService_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources/ \
	-I$(top_srcdir)/simple_interface_generators/glue

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCICIS.cxx \
  $(top_builddir)/interfaces/SCICIS.cxx \
  $(top_builddir)/interfaces/CISSK.cc
