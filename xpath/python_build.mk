lib_LTLIBRARIES += \
	libxpath_python_bridge.la \
	python/xpath.la

libxpath_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libxpath_python_bridge.la
libxpath_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libxpath_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libxpath_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2Pyxpath.cxx

python_xpath_la_LIBADD = $(ALLPYTHONLIBADD) libxpath.la libcellml_python_bridge.la libxpath_python_bridge.la
python_xpath_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_xpath_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/xpath/
python_xpath_la_SOURCES = $(top_builddir)/interfaces/Py2Pxpath.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2Pyxpath.cxx \
		 $(top_builddir)/interfaces/P2Pyxpath.hxx \
		 $(top_builddir)/interfaces/Py2Pxpath.cxx

PYTHON_BOOTSTRAPS += -Wbincludexpath=XPathBootstrap.hpp -Wbbootstrapxpath::XPathEvaluator=CreateXPathEvaluator
