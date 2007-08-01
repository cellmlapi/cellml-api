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

libcuses_xpcom_bridge_la_LDFLAGS = \
  -no-undefined

libcuses_xpcom_bridge_la_CXXFLAGS = \
  -I$(MOZILLA_DIR)/include/nspr -I$(MOZILLA_DIR)/include/xpcom \
  -I$(MOZILLA_DIR)/include/string -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_srcdir)/simple_interface_generators/glue/xpcom \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_CUSES

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2xCUSES.cpp \
  $(top_builddir)/interfaces/x2pCUSES.cpp
