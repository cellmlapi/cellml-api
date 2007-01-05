# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libccgs_xpcom_bridge.la

libccgs_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCCGS.cpp \
  $(top_builddir)/interfaces/x2pCCGS.cpp \
  $(top_srcdir)/CCGS/sources/CCGSBootstrapXPCOM.cpp

libccgs_xpcom_bridge_la_LIBADD = $(STLLINK)
libccgs_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libccgs_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CCGS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCCGS.cpp \
  $(top_builddir)/interfaces/x2pCCGS.cpp
