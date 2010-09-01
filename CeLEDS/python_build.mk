lib_LTLIBRARIES += \
	libceleds_python_bridge.la \
	python/CeLEDS.la

libceleds_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libmalaes_python_bridge.la libcellml_python_bridge.la
libceleds_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libceleds_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libceleds_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCeLEDS.cxx

python_CeLEDS_la_LIBADD = $(ALLPYTHONLIBADD) libceleds.la libcellml_python_bridge.la libmalaes_python_bridge.la libceleds_python_bridge.la
python_CeLEDS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CeLEDS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/CeLEDS/sources/
python_CeLEDS_la_SOURCES = $(top_builddir)/interfaces/Py2PCeLEDS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCeLEDS.cxx \
		 $(top_builddir)/interfaces/P2PyCeLEDS.hxx \
		 $(top_builddir)/interfaces/Py2PCeLEDS.cxx

PYTHON_BOOTSTRAPS += -WbincludeCeLEDS=CeLEDSBootstrap.hpp -Wbbootstrapcellml_services::CeLEDSBootstrap=CreateCeLEDSBootstrap
