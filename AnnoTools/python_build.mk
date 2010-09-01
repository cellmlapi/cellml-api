lib_LTLIBRARIES += \
	libannotools_python_bridge.la \
	python/AnnoTools.la

libannotools_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD)
libannotools_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libannotools_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libannotools_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyAnnoTools.cxx

python_AnnoTools_la_LIBADD = $(ALLPYTHONLIBADD) libannotools.la libcellml_python_bridge.la libannotools_python_bridge.la
python_AnnoTools_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_AnnoTools_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/AnnoTools/sources/
python_AnnoTools_la_SOURCES = $(top_builddir)/interfaces/Py2PAnnoTools.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyAnnoTools.cxx \
		 $(top_builddir)/interfaces/P2PyAnnoTools.hxx \
		 $(top_builddir)/interfaces/Py2PAnnoTools.cxx

PYTHON_BOOTSTRAPS += -WbincludeAnnoTools=AnnoToolsBootstrap.hpp -Wbbootstrapcellml_services::AnnotationToolService=CreateAnnotationToolService
