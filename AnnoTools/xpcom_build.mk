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

libannotools_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libannotools_xpcom_bridge_la_CXXFLAGS = \
  $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_AnnoTools

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xAnnoTools.cpp \
  $(top_builddir)/interfaces/x2pAnnoTools.cpp
