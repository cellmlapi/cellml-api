lib_LTLIBRARIES += \
	libcuses_python_bridge.la \
	python/CUSES.la

libcuses_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
libcuses_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libcuses_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libcuses_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyCUSES.cxx

python_CUSES_la_LIBADD = $(ALLPYTHONLIBADD) libcuses.la libcellml_python_bridge.la libcuses_python_bridge.la
python_CUSES_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_CUSES_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/CUSES/sources/
python_CUSES_la_SOURCES = $(top_builddir)/interfaces/Py2PCUSES.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyCUSES.cxx \
		 $(top_builddir)/interfaces/P2PyCUSES.hxx \
		 $(top_builddir)/interfaces/Py2PCUSES.cxx

PYTHON_BOOTSTRAPS += -WbincludeCUSES=CUSESBootstrap.hpp -Wbbootstrapcellml_services::CUSESBootstrap=CreateCUSESBootstrap
