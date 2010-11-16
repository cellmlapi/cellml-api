lib_LTLIBRARIES += \
	libtelicems_python_bridge.la \
	python/TeLICeMS.la

libtelicems_python_bridge_la_LIBADD = $(ALLPYTHONLIBADD) libcellml_python_bridge.la
libtelicems_python_bridge_la_LDFLAGS = $(ALLPYTHONLDFLAGS)
libtelicems_python_bridge_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS)
libtelicems_python_bridge_la_SOURCES = \
  $(top_builddir)/interfaces/P2PyTeLICeMS.cxx

python_TeLICeMS_la_LIBADD = $(ALLPYTHONLIBADD) libtelicems.la libcellml_python_bridge.la libtelicems_python_bridge.la
python_TeLICeMS_la_LDFLAGS = $(ALLPYTHONLDFLAGS) -module
python_TeLICeMS_la_CXXFLAGS = $(ALLPYTHONCXXFLAGS) -I$(top_srcdir)/TeLICeMS/sources/
python_TeLICeMS_la_SOURCES = $(top_builddir)/interfaces/Py2PTeLICeMS.cxx

BUILT_SOURCES += $(top_builddir)/interfaces/P2PyTeLICeMS.cxx \
		 $(top_builddir)/interfaces/P2PyTeLICeMS.hxx \
		 $(top_builddir)/interfaces/Py2PTeLICeMS.cxx

PYTHON_BOOTSTRAPS += -WbincludeTeLICeMS=TeLICeMService.hpp -Wbbootstrapcellml_services::TeLICeMService=CreateTeLICeMService
