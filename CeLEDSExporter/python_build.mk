lib_LTLIBRARIES += \
	libceledsexporter_python_bridge.la \
	python/CeLEDSExporter.la

libceledsexporter_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libmalaes_python_bridge.la libceleds_python_bridge.la libccgs_python_bridge.la libcellml_python_bridge.la
libceledsexporter_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libceledsexporter_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libceledsexporter_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCeLEDSExporter.cxx

python_CeLEDSExporter_la_LIBADD = $(ALLPYTHONLIBADD) libceledsexporter.la libmalaes_python_bridge.la libceleds_python_bridge.la libccgs_python_bridge.la libcellml_python_bridge.la libceledsexporter_python_bridge.la
python_CeLEDSExporter_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CeLEDSExporter_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/CeLEDSExporter/sources/
python_CeLEDSExporter_la_SOURCES = $(top_builddir)/interfaces/Py2PCeLEDSExporter.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCeLEDSExporter.cxx \
		 $(top_builddir)/interfaces/P2PyCeLEDSExporter.hxx \
		 $(top_builddir)/interfaces/Py2PCeLEDSExporter.cxx

PYTHON_BOOTSTRAPS += -WbincludeCeLEDSExporter=CeLEDSExporterBootstrap.hpp -Wbbootstrapcellml_services::CeLEDSExporterBootstrap=CreateCeLEDSExporterBootstrap
