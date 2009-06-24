# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libvacss_xpcom_bridge.la

libvacss_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xVACSS.cpp \
  $(top_builddir)/interfaces/x2pVACSS.cpp \
  $(top_srcdir)/VACSS/sources/VACSSBootstrapXPCOM.cpp

libvacss_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libvacss.la \
  $(top_builddir)/libXPCOMSupport.la

libvacss_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libvacss_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_VACSS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xVACSS.cpp \
  $(top_builddir)/interfaces/x2pVACSS.cpp
