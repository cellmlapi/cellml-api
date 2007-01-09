# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcis_xpcom_bridge.la

libcis_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCIS.cpp \
  $(top_builddir)/interfaces/x2pCIS.cpp \
  $(top_srcdir)/CIS/sources/CISBootstrapXPCOM.cpp

libcis_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libcellml_xpcom_bridge.la \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libccgs_xpcom_bridge.la \
  $(top_builddir)/libcis.la \
  $(top_builddir)/libXPCOMSupport.la

libcis_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libcis_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CIS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCIS.cpp \
  $(top_builddir)/interfaces/x2pCIS.cpp
