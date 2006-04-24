# CORBA add-on to interfaces module...

lib_LTLIBRARIES += libcellml_corba.la
libcellml_corba_la_SOURCES := \
  interfaces/CCIDOM-APISPEC.cxx \
  interfaces/CCIDOM-events.cxx \
  interfaces/CCIxpcom.cxx \
  interfaces/CCIMathML-content-APISPEC.cxx \
  interfaces/CCICellML-APISPEC.cxx \
  interfaces/CCICellML-events.cxx \
  interfaces/SCIDOM-APISPEC.cxx \
  interfaces/SCIDOM-events.cxx \
  interfaces/SCIxpcom.cxx \
  interfaces/SCIMathML-content-APISPEC.cxx \
  interfaces/SCICellML-APISPEC.cxx \
  interfaces/SCICellML-events.cxx \
  interfaces/DOM-APISPECSK.cc \
  interfaces/DOM-eventsSK.cc \
  interfaces/xpcomSK.cc \
  interfaces/MathML-content-APISPECSK.cc \
  interfaces/CellML-APISPECSK.cc \
  interfaces/CellML-eventsSK.cc
libcellml_corba_la_LIBADD := \
  $(top_builddir)/libCORBASupport.la

libcellml_corba_la_CXXFLAGS := -I$(top_srcdir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue

# Force correct order of compilation...
$(top_builddir)/interfaces/CCI%.cxx: $(top_builddir)/interfaces/%SK.cc
$(top_builddir)/interfaces/SCI%.cxx: $(top_builddir)/interfaces/%SK.cc

%.hh %SK.cc: $(top_srcdir)/interfaces/%.idl
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	omniidl -bcxx $$SAVEDIR/$< && \
	cd $$SAVEDIR
