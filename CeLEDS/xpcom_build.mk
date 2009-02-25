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
  -no-undefined -module

libceleds_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CCGS

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCeLEDS.cpp \
  $(top_builddir)/interfaces/x2pCeLEDS.cpp
