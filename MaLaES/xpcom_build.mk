# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libmalaes_xpcom_bridge.la

libmalaes_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xMaLaES.cpp \
  $(top_builddir)/interfaces/x2pMaLaES.cpp \
  $(top_srcdir)/MaLaES/sources/MaLaESBootstrapXPCOM.cpp

libmalaes_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libcellml_xpcom_bridge.la \
  $(top_builddir)/libmalaes.la \
  $(top_builddir)/libXPCOMSupport.la

libmalaes_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libmalaes_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_MaLaES

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xMaLaES.cpp \
  $(top_builddir)/interfaces/x2pMaLaES.cpp
