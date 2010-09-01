# Python add-on to interfaces module...

ALLPYTHONLIBADD=\
  $(STLLINK) \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libPythonSupport.la
ALLPYTHONLDFLAGS=-no-undefined $(PYTHON_LDFLAGS)
ALLPYTHONCXXFLAGS=\
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/rdf \
  -I$(top_srcdir)/simple_interface_generators/glue/python \
  $(PYTHON_CFLAGS) $(AM_CXXFLAGS)

lib_LTLIBRARIES += \
	libcellml_python_bridge.la \
	python/CellML_APISPEC.la \
	python/CellML_events.la \
	python/DOM_APISPEC.la python/DOM_events.la \
	python/MathML_content_APISPEC.la python/RDF_APISPEC.la

libcellml_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD)
libcellml_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libcellml_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libcellml_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCellML_APISPEC.cxx \
  $(top_builddir)/interfaces/P2PyCellML_events.cxx \
  $(top_builddir)/interfaces/P2PyDOM_APISPEC.cxx \
  $(top_builddir)/interfaces/P2PyDOM_events.cxx \
  $(top_builddir)/interfaces/P2PyMathML_content_APISPEC.cxx \
  $(top_builddir)/interfaces/P2PyRDF_APISPEC.cxx

python_CellML_APISPEC_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
python_CellML_APISPEC_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CellML_APISPEC_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
python_CellML_APISPEC_la_SOURCES = \
  $(top_builddir)/interfaces/Py2PCellML_APISPEC.cxx

python_CellML_events_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
python_CellML_events_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CellML_events_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
python_CellML_events_la_SOURCES = \
  $(top_builddir)/interfaces/Py2PCellML_events.cxx

python_DOM_APISPEC_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
python_DOM_APISPEC_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_DOM_APISPEC_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
python_DOM_APISPEC_la_SOURCES = \
  $(top_builddir)/interfaces/Py2PDOM_APISPEC.cxx

python_DOM_events_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
python_DOM_events_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_DOM_events_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
python_DOM_events_la_SOURCES = \
  $(top_builddir)/interfaces/Py2PDOM_events.cxx

python_MathML_content_APISPEC_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
python_MathML_content_APISPEC_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_MathML_content_APISPEC_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
python_MathML_content_APISPEC_la_SOURCES = \
  $(top_builddir)/interfaces/Py2PMathML_content_APISPEC.cxx

python_RDF_APISPEC_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
python_RDF_APISPEC_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_RDF_APISPEC_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
python_RDF_APISPEC_la_SOURCES = \
  $(top_builddir)/interfaces/Py2PRDF_APISPEC.cxx

PYTHON_BOOTSTRAPS += -WbincludeCellML_APISPEC=CellMLBootstrap.hpp \
                     -Wbbootstrapcellml_api::CellMLBootstrap=CreateCellMLBootstrap

$(top_builddir)/interfaces/P2Py%.cxx \
$(top_builddir)/interfaces/P2Py%.hxx \
$(top_builddir)/interfaces/Py2P%.cxx: \
$(top_srcdir)/interfaces/%.idl $(top_srcdir)/interfaces/Iface%.hxx \
$(top_srcdir)/simple_interface_generators/omniidl_be/python/__init__.py \
$(top_srcdir)/simple_interface_generators/omniidl_be/python/CToPythonWalker.py \
$(top_srcdir)/simple_interface_generators/omniidl_be/python/PythonToCWalker.py \
$(top_srcdir)/simple_interface_generators/omniidl_be/python/typeinfo.py
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -p$$SAVEDIR/$(top_srcdir)/simple_interface_generators/omniidl_be \
	-bpython $(PYTHON_BOOTSTRAPS) -DIN_CELLML_MODULE $$SAVEDIR/$< && \
	cd $$SAVEDIR

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCellML_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyCellML_APISPEC.hxx \
		$(top_builddir)/interfaces/Py2PCellML_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyCellML_events.cxx \
		$(top_builddir)/interfaces/P2PyCellML_events.hxx \
		$(top_builddir)/interfaces/Py2PCellML_events.cxx \
		$(top_builddir)/interfaces/P2PyCellML_Services.cxx \
		$(top_builddir)/interfaces/P2PyCellML_Services.hxx \
		$(top_builddir)/interfaces/Py2PCellML_Services.cxx \
		$(top_builddir)/interfaces/P2PyDOM_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyDOM_APISPEC.hxx \
		$(top_builddir)/interfaces/Py2PDOM_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyDOM_events.cxx \
		$(top_builddir)/interfaces/P2PyDOM_events.hxx \
		$(top_builddir)/interfaces/Py2PDOM_events.cxx \
		$(top_builddir)/interfaces/P2PyMathML_content_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyMathML_content_APISPEC.hxx \
		$(top_builddir)/interfaces/Py2PMathML_content_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyRDF_APISPEC.cxx \
		$(top_builddir)/interfaces/P2PyRDF_APISPEC.hxx \
		$(top_builddir)/interfaces/Py2PRDF_APISPEC.cxx
