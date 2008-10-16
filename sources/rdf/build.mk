# Interfaces for Low level RDF Library...

include_HEADERS += \
  $(top_builddir)/interfaces/IfaceRDF_APISPEC.hxx

noinst_LTLIBRARIES += librdf.la

librdf_la_SOURCES = \
  sources/rdf/RDFImplementation.cpp
librdf_la_CXXFLAGS = \
  -Wall -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/cellml \
  -I $(top_srcdir)/sources/dom -I $(top_srcdir)/sources/dom_direct \
  -I $(top_srcdir)/sources/rdf -I$(top_builddir)/interfaces $(AM_CXXFLAGS)

BUILT_SOURCES += $(top_builddir)/interfaces/IfaceRDF_APISPEC.hxx
