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

libvacss_xpcom_bridge_la_LDFLAGS = \
  -no-undefined -module

libvacss_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_VACSS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xVACSS.cpp \
  $(top_builddir)/interfaces/x2pVACSS.cpp
