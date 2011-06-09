# The overall libcellml.la library...
lib_LTLIBRARIES += libcellml.la
noinst_LTLIBRARIES += libdomonly.la libdomdirectonly.la libmathmlonly.la libcellmlonly.la libxpathonly.la
libcellml_la_SOURCES = sources/Utilities.cpp
libcellml_la_LIBADD = \
  libcellmlonly.la \
  libmathmlonly.la \
  libdomonly.la \
  libxpathonly.la \
  libdomdirectonly.la -lxml2 $(STLLINK)
libcellml_la_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/sources \
  -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

if ENABLE_RDF
libcellml_la_LIBADD += librdf.la
endif

libdomdirectonly_la_SOURCES = \
  sources/dom_direct/DOMImplementation.cpp \
  sources/dom_direct/DOMLoader.cpp
libdomdirectonly_la_CXXFLAGS = \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/dom -I$(top_builddir)/interfaces \
  $(LIBXML_CFLAGS) $(AM_CXXFLAGS)

libdomonly_la_SOURCES = \
  sources/dom/DOMBootstrap.cpp \
  sources/dom/DOMWriter.cpp

libdomonly_la_CXXFLAGS = \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I $(top_srcdir)/sources/dom \
  -I $(top_srcdir)/sources/dom_direct -I$(top_builddir)/interfaces \
  $(GDOME_CFLAGS) $(GLIB_CFLAGS) $(AM_CXXFLAGS)

libmathmlonly_la_SOURCES = \
  sources/mathml/MathMLImplementation.cpp
libmathmlonly_la_CXXFLAGS = \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

include_HEADERS += \
  sources/cellml/CellMLBootstrap.hpp \
  cda_config.h \
  sources/cda_compiler_support.h \
  sources/cellml-api-cxx-support.hpp

libcellmlonly_la_SOURCES = \
  sources/cellml/CellMLImplementation.cpp \
  sources/cellml/CellMLBootstrap.cpp \
  sources/cellml/CellMLEvents.cpp
libcellmlonly_la_CXXFLAGS = \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/mathml -I$(top_builddir)/interfaces -I$(top_srcdir)/sources/rdf \
  $(AM_CXXFLAGS)

libxpathonly_la_SOURCES = \
  sources/xpath/XPathImpl.cpp
libxpathonly_la_CXXFLAGS = \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/xpath \
  -I$(top_builddir)/interfaces $(AM_CXXFLAGS)
