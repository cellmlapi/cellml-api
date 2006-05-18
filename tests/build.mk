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
