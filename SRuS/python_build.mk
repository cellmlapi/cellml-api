lib_LTLIBRARIES += \
	libsrus_python_bridge.la \
	python/SRuS.la

libsrus_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la libspros_python_bridge.la
libsrus_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libsrus_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libsrus_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PySRuS.cxx

python_SRuS_la_LIBADD = $(ALLPYTHONLIBADD) libsrus.la libcellml_python_bridge.la libsrus_python_bridge.la
python_SRuS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_SRuS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/SRuS/
python_SRuS_la_SOURCES = $(top_builddir)/interfaces/Py2PSRuS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PySRuS.cxx \
		 $(top_builddir)/interfaces/P2PySRuS.hxx \
		 $(top_builddir)/interfaces/Py2PSRuS.cxx

PYTHON_BOOTSTRAPS += -WbincludeSRuS=SRuSBootstrap.hpp -WbbootstrapSRuS::Bootstrap=CreateSRuSBootstrap
