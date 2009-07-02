# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcis_xpcom_bridge.la

libcis_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCIS.cpp \
  $(top_builddir)/interfaces/x2pCIS.cpp \
  $(top_srcdir)/CIS/sources/CISBootstrapXPCOM.cpp

libcis_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcis.la \
  $(top_builddir)/libXPCOMSupport.la

libcis_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libcis_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CIS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCIS.cpp \
  $(top_builddir)/interfaces/x2pCIS.cpp
