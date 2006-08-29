# Tests...
check_PROGRAMS=tests/RunTestBin
TESTS=tests/RunTestBin tests/CheckCodeGenerator
tests_RunTestBin_SOURCES=\
	tests/TestOutput.cpp \
	tests/MathMLTest.cpp \
	tests/DOMTest.cpp \
	tests/CellMLTest.cpp \
	tests/CellMLEventsTest.cpp

tests_RunTestBin_LDADD=-L$(top_builddir) -lcellml -lcppunit -lxml2
tests_RunTestBin_CXXFLAGS=-I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom/ -I$(top_srcdir)/sources/mathml/ -I$(top_srcdir)/sources/cellml -I$(top_builddir)/interfaces -I$(top_srcdir)/tests
