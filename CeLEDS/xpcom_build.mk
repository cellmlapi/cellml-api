# XPCOM add-on to interfaces module...

lib_LTLIBRARIES += libceleds_xpcom_bridge.la

libceleds_xpcom_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/p2xCeLEDS.cpp \
  $(top_builddir)/interfaces/x2pCeLEDS.cpp \
  $(top_srcdir)/CeLEDS/sources/CeLEDSBootstrapXPCOM.cpp

libceleds_xpcom_bridge_la_LIBADD = \
  $(STLLINK) \
  $(top_builddir)/libceleds.la \
  $(top_builddir)/libXPCOMSupport.la

libceleds_xpcom_bridge_la_LDFLAGS = \
  $(XPCOM_LDFLAGS)

libceleds_xpcom_bridge_la_CXXFLAGS = \
  $(XPCOM_CXXFLAGS) -DMODULE_CONTAINS_CCGS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCeLEDS.cpp \
  $(top_builddir)/interfaces/x2pCeLEDS.cpp
