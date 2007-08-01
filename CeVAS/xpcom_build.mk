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

libcevas_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libcevas_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CeVAS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCeVAS.cpp \
  $(top_builddir)/interfaces/x2pCeVAS.cpp
