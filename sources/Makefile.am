SUBDIRS=cellml dom dom_direct mathml

lib_LTLIBRARIES = libcellml.la
libcellml_la_SOURCES = Utilities.cpp
libcellml_la_LIBADD = \
  cellml/libcellmlonly.la \
  mathml/libmathmlonly.la \
  dom/libdomonly.la \
  dom_direct/libdomdirectonly.la

AM_CXXFLAGS=-I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_builddir)/interfaces
