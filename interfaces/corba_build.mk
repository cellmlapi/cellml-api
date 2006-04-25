# CORBA add-on to interfaces module...

lib_LTLIBRARIES += libcellml_corba.la
libcellml_corba_la_SOURCES := \
  $(top_builddir)/interfaces/CCIDOM-APISPEC.cxx \
  $(top_builddir)/interfaces/CCIDOM-events.cxx \
  $(top_builddir)/interfaces/CCIxpcom.cxx \
  $(top_builddir)/interfaces/CCIMathML-content-APISPEC.cxx \
  $(top_builddir)/interfaces/CCICellML-APISPEC.cxx \
  $(top_builddir)/interfaces/CCICellML-events.cxx \
  $(top_builddir)/interfaces/SCIDOM-APISPEC.cxx \
  $(top_builddir)/interfaces/SCIDOM-events.cxx \
  $(top_builddir)/interfaces/SCIxpcom.cxx \
  $(top_builddir)/interfaces/SCIMathML-content-APISPEC.cxx \
  $(top_builddir)/interfaces/SCICellML-APISPEC.cxx \
  $(top_builddir)/interfaces/SCICellML-events.cxx \
  $(top_builddir)/interfaces/DOM-APISPECSK.cc \
  $(top_builddir)/interfaces/DOM-eventsSK.cc \
  $(top_builddir)/interfaces/xpcomSK.cc \
  $(top_builddir)/interfaces/MathML-content-APISPECSK.cc \
  $(top_builddir)/interfaces/CellML-APISPECSK.cc \
  $(top_builddir)/interfaces/CellML-eventsSK.cc
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
