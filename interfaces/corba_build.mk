# CORBA add-on to interfaces module...

lib_LTLIBRARIES += libcellml_corba_stubs.la libcellml_corba_bridge.la
# noinst_LTLIBRARIES += libcellml_corba_sk.la
libcellml_corba_bridge_la_SOURCES := \
  $(top_builddir)/interfaces/CCIDOM_APISPEC.cxx \
  $(top_builddir)/interfaces/CCIDOM_events.cxx \
  $(top_builddir)/interfaces/CCIxpcom.cxx \
  $(top_builddir)/interfaces/CCIMathML_content_APISPEC.cxx \
  $(top_builddir)/interfaces/CCICellML_APISPEC.cxx \
  $(top_builddir)/interfaces/CCICellML_events.cxx \
  $(top_builddir)/interfaces/SCIDOM_APISPEC.cxx \
  $(top_builddir)/interfaces/SCIDOM_events.cxx \
  $(top_builddir)/interfaces/SCIxpcom.cxx \
  $(top_builddir)/interfaces/SCIMathML_content_APISPEC.cxx \
  $(top_builddir)/interfaces/SCICellML_APISPEC.cxx \
  $(top_builddir)/interfaces/SCICellML_events.cxx

libcellml_corba_stubs_la_SOURCES := \
  $(top_builddir)/interfaces/DOM_APISPECSK.cc \
  $(top_builddir)/interfaces/DOM_eventsSK.cc \
  $(top_builddir)/interfaces/xpcomSK.cc \
  $(top_builddir)/interfaces/MathML_content_APISPECSK.cc \
  $(top_builddir)/interfaces/CellML_APISPECSK.cc \
  $(top_builddir)/interfaces/CellML_eventsSK.cc

libcellml_corba_stubs_LIBADD := $(STLLINK) $(OMNILINK)

libcellml_corba_bridge_la_CXXFLAGS := \
  -I$(top_builddir)/interfaces -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/simple_interface_generators/glue $(AM_CXXFLAGS)

libcellml_corba_stubs_la_CXXFLAGS := \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue -no-undefined

libcellml_corba_bridge_la_LIBADD := libcellml_corba_stubs.la $(STLLINK) $(OMNILINK)

# Force correct order of compilation...
$(top_builddir)/interfaces/CCI%.cxx: $(top_builddir)/interfaces/%SK.cc
$(top_builddir)/interfaces/SCI%.cxx: $(top_builddir)/interfaces/%SK.cc

$(top_builddir)/interfaces/%.hh \
$(top_builddir)/interfaces/%SK.cc: \
  $(top_srcdir)/interfaces/%.idl
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -bcxx $$SAVEDIR/$< && \
	cd $$SAVEDIR

BUILT_SOURCES += \
  $(top_builddir)/interfaces/CCIDOM_APISPEC.cxx \
  $(top_builddir)/interfaces/CCIDOM_events.cxx \
  $(top_builddir)/interfaces/CCIxpcom.cxx \
  $(top_builddir)/interfaces/CCIMathML_content_APISPEC.cxx \
  $(top_builddir)/interfaces/CCICellML_APISPEC.cxx \
  $(top_builddir)/interfaces/CCICellML_events.cxx \
  $(top_builddir)/interfaces/SCIDOM_APISPEC.cxx \
  $(top_builddir)/interfaces/SCIDOM_events.cxx \
  $(top_builddir)/interfaces/SCIxpcom.cxx \
  $(top_builddir)/interfaces/SCIMathML_content_APISPEC.cxx \
  $(top_builddir)/interfaces/SCICellML_APISPEC.cxx \
  $(top_builddir)/interfaces/SCICellML_events.cxx \
  $(top_builddir)/interfaces/DOM_APISPECSK.cc \
  $(top_builddir)/interfaces/DOM_eventsSK.cc \
  $(top_builddir)/interfaces/xpcomSK.cc \
  $(top_builddir)/interfaces/MathML_content_APISPECSK.cc \
  $(top_builddir)/interfaces/CellML_APISPECSK.cc \
  $(top_builddir)/interfaces/CellML_eventsSK.cc
