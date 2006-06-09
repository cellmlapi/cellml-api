bin_PROGRAMS += CCGSService
CCGSService_SOURCES := $(top_srcdir)/CCGS/sources/CCGSServiceRegistration.cpp
CCGSService_LDADD := \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libcellml_corba_client.la \
  $(top_builddir)/libcellml_context_corba.la \
  $(top_builddir)/libcellml_corba.la \
  $(top_builddir)/libCORBASupport.la

CCGSService_LDFLAGS := -static -lomniORB4

CCGSService_CXXFLAGS := \
	-I$(top_builddir)/interfaces \
	-I$(top_srcdir) \
	-I$(top_srcdir)/sources/cellml_corba_server \
        -I$(top_srcdir)/sources/
