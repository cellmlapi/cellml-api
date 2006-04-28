# CORBA add-on to interfaces module...

lib_LTLIBRARIES += libcellml_corba.la
libcellml_corba_la_SOURCES := \
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
libcellml_corba_la_LIBADD := \
  $(top_builddir)/libCORBASupport.la

libcellml_corba_la_CXXFLAGS := \
  -I$(top_srcdir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue

# Force correct order of compilation...
$(top_builddir)/interfaces/CCI%.cxx: $(top_builddir)/interfaces/%SK.cc
$(top_builddir)/interfaces/SCI%.cxx: $(top_builddir)/interfaces/%SK.cc

$(top_builddir)/interfaces/%.hh \
$(top_builddir)/interfaces/%SK.cc: \
  $(top_srcdir)/interfaces/%.idl
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	omniidl -bcxx $$SAVEDIR/$< && \
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
