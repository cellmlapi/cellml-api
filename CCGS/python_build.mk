lib_LTLIBRARIES += \
	libccgs_python_bridge.la \
	python/CCGS.la

libccgs_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libannotools_python_bridge.la libmalaes_python_bridge.la libcuses_python_bridge.la libcevas_python_bridge.la libcellml_python_bridge.la
libccgs_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libccgs_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libccgs_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCCGS.cxx

python_CCGS_la_LIBADD = $(ALLPYTHONLIBADD) libccgs.la libannotools_python_bridge.la libmalaes_python_bridge.la libcuses_python_bridge.la libcevas_python_bridge.la libcellml_python_bridge.la libccgs_python_bridge.la
python_CCGS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CCGS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/CCGS/sources/
python_CCGS_la_SOURCES = $(top_builddir)/interfaces/Py2PCCGS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCCGS.cxx \
		 $(top_builddir)/interfaces/P2PyCCGS.hxx \
		 $(top_builddir)/interfaces/Py2PCCGS.cxx

PYTHON_BOOTSTRAPS += -WbincludeCCGS=CCGSBootstrap.hpp -Wbbootstrapcellml_services::CodeGeneratorBootstrap=CreateCodeGeneratorBootstrap
