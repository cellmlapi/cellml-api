# Interfaces module...
include_HEADERS := \
  interfaces/IfaceDOM-APISPEC.hxx \
  interfaces/IfaceDOM-events.hxx \
  interfaces/Ifacexpcom.hxx \
  interfaces/IfaceMathML-content-APISPEC.hxx \
  interfaces/IfaceCellML-APISPEC.hxx \
  interfaces/IfaceCellML-events.hxx

Iface%.hxx CCI%.hxx SCI%.hxx CCI%.cxx SCI%.cxx: $(top_srcdir)/interfaces/%.idl
	omniidl -p $(top_srcdir)/simple_interface_generators/omniidl_be -bsimple_cpp $<

# CORBA add-on to interfaces module...

lib_LTLIBRARIES := libcellml_corba.la
libcellml_corba_la_SOURCES := \
  interfaces/CCIDOM-APISPEC.cxx \
  interfaces/CCIDOM-events.cxx \
  interfaces/CCIxpcom.cxx \
  interfaces/CCIMathML-content-APISPEC.cxx \
  interfaces/CCICellML-APISPEC.cxx \
  interfaces/CCICellML-events.cxx \
  interfaces/SCIDOM-APISPEC.cxx \
  interfaces/SCIDOM-events.cxx \
  interfaces/SCIxpcom.cxx \
  interfaces/SCIMathML-content-APISPEC.cxx \
  interfaces/SCICellML-APISPEC.cxx \
  interfaces/SCICellML-events.cxx \
  interfaces/DOM-APISPECSK.cc \
  interfaces/DOM-eventsSK.cc \
  interfaces/xpcomSK.cc \
  interfaces/MathML-content-APISPECSK.cc \
  interfaces/CellML-APISPECSK.cc \
  interfaces/CellML-eventsSK.cc
libcellml_corba_la_LIBADD := \
  $(top_builddir)/libCORBASupport.la

libcellml_corba_la_CXXFLAGS := -I$(top_srcdir)/interfaces -I$(top_srcdir)/simple_interface_generators/glue

# Force correct order of compilation...
CCI%.cxx: %SK.cc
SCI%.cxx: %SK.cc

# CORBA Support glue for interface generators...
noinst_LTLIBRARIES=libCORBASupport.la
libCORBASupport_la_SOURCES := simple_interface_generators/glue/corba_support/WrapperRepository.cpp
libCORBASupport_la_CXXFLAGS := -I$(top_builddir)/interfaces

%.hh %SK.cc: $(top_srcdir)/interfaces/%.idl
	omniidl -bcxx $<

# The overall libcellml.la library...
lib_LTLIBRARIES += libcellml.la
noinst_LTLIBRARIES += libdomonly.la libdomdirectonly.la libmathmlonly.la libcellmlonly.la
libcellml_la_SOURCES := sources/Utilities.cpp
libcellml_la_LIBADD := \
  libcellmlonly.la \
  libmathmlonly.la \
  libdomonly.la \
  libdomdirectonly.la
libcellml_la_CXXFLAGS := -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_builddir)/interfaces

libdomdirectonly_la_SOURCES := \
  sources/dom_direct/DOMImplementation.cpp \
  sources/dom_direct/DOMLoader.cpp
libdomdirectonly_la_CXXFLAGS := \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/dom -I$(top_builddir)/interfaces \
  `xml2-config --cflags`

libdomonly_la_SOURCES := \
  sources/dom/DOMBootstrap.cpp \
  sources/dom/DOMWriter.cpp

libdomonly_la_CXXFLAGS := \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I $(top_srcdir)/sources/dom \
  -I $(top_srcdir)/sources/dom_direct -I$(top_builddir)/interfaces \
  $(GDOME_CFLAGS) $(GLIB_CFLAGS)

libmathmlonly_la_SOURCES := \
  sources/mathml/MathMLImplementation.cpp
libmathmlonly_la_CXXFLAGS := \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces

include_HEADERS += sources/cellml/CellMLBootstrap.hpp
libcellmlonly_la_SOURCES := \
  sources/cellml/CellMLImplementation.cpp \
  sources/cellml/CellMLBootstrap.cpp \
  sources/cellml/CellMLEvents.cpp
libcellmlonly_la_CXXFLAGS := \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces

# CellML CORBA Server...
bin_PROGRAMS := cellml_corba_server
cellml_corba_server_SOURCES := sources/cellml_corba_server/Main.cpp
cellml_corba_server_CXXFLAGS := \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces
cellml_corba_server_LDADD := \
  $(top_builddir)/sources/libcellml.la \
  $(top_builddir)/interfaces/libcellml_corba.la
cellml_corba_server_LDFLAGS := -lomniORB4 -lxml2

# Tests...
check_PROGRAMS=tests/RunTestBin
TESTS=tests/RunTestBin
tests_RunTestBin_SOURCES=tests/TestOutput.cpp \
			 tests/DOMTest.cpp \
                         tests/MathMLTest.cpp \
                         tests/CellMLTest.cpp \
                         tests/CellMLEventsTest.cpp
tests_RunTestBin_LDADD=-L$(top_builddir)/sources -lcellml -lcppunit -lxml2 -ldl
tests_RunTestBin_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom/ -I$(top_srcdir)/sources/mathml/ -I$(top_srcdir)/sources/cellml -I$(top_builddir)/interfaces -I$(top_srcdir)/tests $(GLIB_CFLAGS) $(GDOME_CFLAGS)
