# Please note that RunSEDML is included as a testing program to help test the
# API, and not as a general purpose tool. Please carefully consider whether
# adding a new program is justified on the grounds of testing the API
# more extensively before doing so.

if ENABLE_EXAMPLES
bin_PROGRAMS += RunSEDML
endif

RunSEDML_SOURCES=$(top_srcdir)/SRuS/tests/RunSEDML.cpp
RunSEDML_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libccgs.la \
  $(top_builddir)/libspros.la \
  $(top_builddir)/libsrus.la
RunSEDML_LDFLAGS=-O0
RunSEDML_CXXFLAGS=-I$(top_srcdir)/sources \
  -I$(top_srcdir)/SProS/sources -I$(top_srcdir)/SRuS -I$(top_builddir)/interfaces
