# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libmalaes_xpcom_bridge.la

libmalaes_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xMaLaES.cpp \
  $(top_builddir)/interfaces/x2pMaLaES.cpp \
  $(top_srcdir)/MaLaES/sources/MaLaESBootstrapXPCOM.cpp

libmalaes_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libmalaes.la \
  $(top_builddir)/libXPCOMSupport.la

libmalaes_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libmalaes_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_MaLaES

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xMaLaES.cpp \
  $(top_builddir)/interfaces/x2pMaLaES.cpp
