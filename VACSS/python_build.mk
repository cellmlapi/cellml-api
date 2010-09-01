lib_LTLIBRARIES += \
	libvacss_python_bridge.la \
	python/VACSS.la

libvacss_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
libvacss_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libvacss_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libvacss_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyVACSS.cxx

python_VACSS_la_LIBADD = $(ALLPYTHONLIBADD) libvacss.la libcellml_python_bridge.la libvacss_python_bridge.la
python_VACSS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_VACSS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/VACSS/sources/
python_VACSS_la_SOURCES = $(top_builddir)/interfaces/Py2PVACSS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyVACSS.cxx \
		 $(top_builddir)/interfaces/P2PyVACSS.hxx \
		 $(top_builddir)/interfaces/Py2PVACSS.cxx

PYTHON_BOOTSTRAPS += -WbincludeVACSS=VACSSBootstrap.hpp -Wbbootstrapcellml_services::VACSSService=CreateVACSSService
