if ENABLE_EXAMPLES
bin_PROGRAMS += TestTeLICeMParser TestTeLICeMSerialiser
endif

TestTeLICeMParser_SOURCES=$(top_srcdir)/TeLICeMS/tests/TestTeLICeMParser.cpp
TestTeLICeMParser_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libtelicems.la -lxml2
TestTeLICeMParser_LDFLAGS=-O0
TestTeLICeMParser_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_builddir) -I$(top_builddir)/interfaces -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/TeLICeMS/sources -Wall -O0 $(AM_CXXFLAGS)

TestTeLICeMSerialiser_SOURCES=$(top_srcdir)/TeLICeMS/tests/TestTeLICeMSerialiser.cpp
TestTeLICeMSerialiser_LDADD=\
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libtelicems.la -lxml2
TestTeLICeMSerialiser_LDFLAGS=-O0
TestTeLICeMSerialiser_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_builddir) -I$(top_builddir)/interfaces -I$(top_srcdir)/sources/dom -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml -I$(top_srcdir)/TeLICeMS/sources -Wall -O0 $(AM_CXXFLAGS)


TESTS+=tests/CheckTeLICeMSParser
