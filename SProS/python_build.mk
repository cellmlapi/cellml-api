lib_LTLIBRARIES += \
	libspros_python_bridge.la \
	python/SProS.la

libspros_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
libspros_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libspros_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libspros_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PySProS.cxx

python_SProS_la_LIBADD = $(ALLPYTHONLIBADD) libspros.la libcellml_python_bridge.la libspros_python_bridge.la
python_SProS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_SProS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/SProS/sources/
python_SProS_la_SOURCES = $(top_builddir)/interfaces/Py2PSProS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PySProS.cxx \
		 $(top_builddir)/interfaces/P2PySProS.hxx \
		 $(top_builddir)/interfaces/Py2PSProS.cxx

PYTHON_BOOTSTRAPS += -WbincludeSProS=SProSBootstrap.hpp -WbbootstrapSProS::Bootstrap=CreateSProSBootstrap
