lib_LTLIBRARIES += \
	libcevas_python_bridge.la \
	python/CeVAS.la

libcevas_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
libcevas_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libcevas_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libcevas_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCeVAS.cxx

python_CeVAS_la_LIBADD = $(ALLPYTHONLIBADD) libcevas.la libcellml_python_bridge.la libcevas_python_bridge.la
python_CeVAS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CeVAS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/CeVAS/sources/
python_CeVAS_la_SOURCES = $(top_builddir)/interfaces/Py2PCeVAS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCeVAS.cxx \
		 $(top_builddir)/interfaces/P2PyCeVAS.hxx \
		 $(top_builddir)/interfaces/Py2PCeVAS.cxx

PYTHON_BOOTSTRAPS += -WbincludeCeVAS=CeVASBootstrap.hpp -Wbbootstrapcellml_services::CeVASBootstrap=CreateCeVASBootstrap
