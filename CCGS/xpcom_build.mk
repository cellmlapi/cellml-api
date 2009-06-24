# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libccgs_xpcom_bridge.la

libccgs_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCCGS.cpp \
  $(top_builddir)/interfaces/x2pCCGS.cpp \
  $(top_srcdir)/CCGS/sources/CCGSBootstrapXPCOM.cpp

libccgs_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libXPCOMSupport.la

libccgs_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libccgs_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CCGS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCCGS.cpp \
  $(top_builddir)/interfaces/x2pCCGS.cpp
