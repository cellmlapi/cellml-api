# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libcuses_xpcom_bridge.la

libcuses_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCUSES.cpp \
  $(top_builddir)/interfaces/x2pCUSES.cpp \
  $(top_srcdir)/CUSES/sources/CUSESBootstrapXPCOM.cpp

libcuses_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libcuses.la \
  $(top_builddir)/libXPCOMSupport.la

libcuses_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libcuses_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CUSES

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCUSES.cpp \
  $(top_builddir)/interfaces/x2pCUSES.cpp
