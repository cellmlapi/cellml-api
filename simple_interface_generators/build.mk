# CORBA Support glue for interface generators...
if ENABLE_CORBA
lib_LTLIBRARIES += libCORBASupport.la
libCORBASupport_la_SOURCES = $(top_srcdir)/simple_interface_generators/glue/corba_support/WrapperRepository.cpp
libCORBASupport_la_CXXFLAGS = -I$(top_builddir)/interfaces -I$(top_srcdir) -I$(top_srcdir)/sources $(AM_CXXFLAGS)
libCORBASupport_la_LIBADD = $(OMNILINK) $(STLLINK)
libCORBASupport_la_LDFLAGS = -lcellml
endif
if ENABLE_XPCOM
lib_LTLIBRARIES += libXPCOMSupport.la
libXPCOMSupport_la_SOURCES = \
  $(top_srcdir)/simple_interface_generators/glue/xpcom/p2xxpcom.cpp \
  $(top_srcdir)/simple_interface_generators/glue/xpcom/x2pxpcom.cpp \
  $(top_srcdir)/simple_interface_generators/glue/xpcom/WideCharSupport.cpp
libXPCOMSupport_la_CXXFLAGS = \
  -I$(top_builddir)/interfaces -I$(top_srcdir) -I$(top_srcdir)/sources \
  $(XPCOM_CXXFLAGS) \
  -I$(top_builddir)/simple_interface_generators/glue/xpcom -DIN_MODULE_XPCOMSupport \
  $(AM_CXXFLAGS)
libXPCOMSupport_la_LIBADD = $(STLLINK) -lcellml
libXPCOMSupport_la_LDFLAGS = $(XPCOM_LDFLAGS_NOTMOD) -lcellml

BUILT_SOURCES += \
  $(top_builddir)/simple_interface_generators/glue/xpcom/IWrappedPCM.h

$(top_builddir)/simple_interface_generators/glue/xpcom/IWrappedPCM.h: \
$(top_srcdir)/simple_interface_generators/glue/xpcom/IWrappedPCM.idl
	mkdir -p $(top_builddir)/simple_interface_generators/glue/xpcom
	$(CYGWIN_WRAPPER) $(MOZILLA_DIR)/bin/xpidl -m header -I$(MOZILLA_DIR)/idl -e $@ $<
	$(CYGWIN_WRAPPER) $(MOZILLA_DIR)/bin/xpidl -m typelib -I$(MOZILLA_DIR)/idl -e $(top_builddir)/simple_interface_generators/glue/xpcom/IWrappedPCM.xpt $<
endif

if ENABLE_JAVA
lib_LTLIBRARIES += libJavaSupport.la
libJavaSupport_la_SOURCES = \
  $(top_srcdir)/simple_interface_generators/glue/java/p2jxpcom.cpp

libJavaSupport_la_CXXFLAGS = \
  -I$(top_builddir)/interfaces -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_builddir)/simple_interface_generators/glue/java -DIN_MODULE_JavaSupport \
  $(AM_CXXFLAGS)
libJavaSupport_la_LDFLAGS = -lcellml
endif

if ENABLE_PYTHON
lib_LTLIBRARIES += libPythonSupport.la python/xpcom.la
libPythonSupport_la_SOURCES = \
  $(top_srcdir)/simple_interface_generators/glue/python/python_support.cxx

libPythonSupport_la_CXXFLAGS = \
  -I$(top_builddir)/interfaces -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_builddir)/simple_interface_generators/glue/python -DIN_MODULE_PythonSupport \
  $(AM_CXXFLAGS) $(PYTHON_CFLAGS)

libPythonSupport_la_LDFLAGS = -lcellml
python_xpcom_la_SOURCES = \
  $(top_srcdir)/simple_interface_generators/glue/python/xpcom.cxx
python_xpcom_la_LIBADD = $(ALLPYTHONLIBADD)
python_xpcom_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module -lcellml
python_xpcom_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
endif
