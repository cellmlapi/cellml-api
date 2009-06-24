# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcevas_xpcom_bridge.la

libcevas_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCeVAS.cpp \
  $(top_builddir)/interfaces/x2pCeVAS.cpp \
  $(top_srcdir)/CeVAS/sources/CeVASBootstrapXPCOM.cpp

libcevas_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcevas.la \
  $(top_builddir)/libXPCOMSupport.la

libcevas_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libcevas_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CeVAS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCeVAS.cpp \
  $(top_builddir)/interfaces/x2pCeVAS.cpp
