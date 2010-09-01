lib_LTLIBRARIES += \
	libcis_python_bridge.la \
	python/CIS.la

libcis_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libccgs_python_bridge.la libcellml_python_bridge.la
libcis_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libcis_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libcis_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCIS.cxx

python_CIS_la_LIBADD = $(ALLPYTHONLIBADD) libcis.la libccgs_python_bridge.la libcellml_python_bridge.la libcis_python_bridge.la
python_CIS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CIS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/CIS/sources/
python_CIS_la_SOURCES = $(top_builddir)/interfaces/Py2PCIS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCIS.cxx \
		 $(top_builddir)/interfaces/P2PyCIS.hxx \
		 $(top_builddir)/interfaces/Py2PCIS.cxx

PYTHON_BOOTSTRAPS += -WbincludeCIS=CISBootstrap.hpp -Wbbootstrapcellml_services::CellMLIntegrationService=CreateIntegrationService
