# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libtelicems_xpcom_bridge.la

libtelicems_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xTeLICeMS.cpp \
  $(top_builddir)/interfaces/x2pTeLICeMS.cpp \
  $(top_srcdir)/TeLICeMS/sources/TeLICeMServiceXPCOM.cpp

libtelicems_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libtelicems.la \
  $(top_builddir)/libXPCOMSupport.la

libtelicems_xpcom_bridge_la_LDFLAGS = $(XPCOM_LDFLAGS)

libtelicems_xpcom_bridge_la_CXXFLAGS = $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_TELICEMS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xTeLICeMS.cpp \
  $(top_builddir)/interfaces/x2pTeLICeMS.cpp
