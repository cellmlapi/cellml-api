lib_LTLIBRARIES += \
	libmalaes_python_bridge.la \
	python/MaLaES.la

libmalaes_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
libmalaes_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libmalaes_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libmalaes_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyMaLaES.cxx

python_MaLaES_la_LIBADD = $(ALLPYTHONLIBADD) libmalaes.la libcellml_python_bridge.la libannotools_python_bridge.la libcuses_python_bridge.la libcevas_python_bridge.la libmalaes_python_bridge.la
python_MaLaES_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_MaLaES_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/MaLaES/sources/
python_MaLaES_la_SOURCES = $(top_builddir)/interfaces/Py2PMaLaES.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyMaLaES.cxx \
		 $(top_builddir)/interfaces/P2PyMaLaES.hxx \
		 $(top_builddir)/interfaces/Py2PMaLaES.cxx

PYTHON_BOOTSTRAPS += -WbincludeMaLaES=MaLaESBootstrap.hpp -Wbbootstrapcellml_services::MaLaESBootstrap=CreateMaLaESBootstrap
