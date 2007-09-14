# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libannotools_xpcom_bridge.la

libannotools_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xAnnoTools.cpp \
  $(top_builddir)/interfaces/x2pAnnoTools.cpp \
  $(top_srcdir)/AnnoTools/sources/AnnoToolsBootstrapXPCOM.cpp

libannotools_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libannotools.la \
  $(top_builddir)/libXPCOMSupport.la

libannotools_xpcom_bridge_la_LDFLAGS = \
  -no-undefined -module

libannotools_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_AnnoTools

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xAnnoTools.cpp \
  $(top_builddir)/interfaces/x2pAnnoTools.cpp
