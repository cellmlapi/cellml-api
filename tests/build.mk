# Tests...
check_PROGRAMS=tests/RunTestBin$(EXEEXT)
TESTS=tests/RunTestBin tests/CheckCodeGenerator
tests_RunTestBin_SOURCES=\
	tests/TestOutput.cpp \
	tests/DOMTest.cpp \
	tests/MathMLTest.cpp \
	tests/CellMLTest.cpp \
	tests/CellMLEventsTest.cpp

tests_RunTestBin_LDADD=-L$(top_builddir) -lcellml -lcppunit -lxml2
tests_RunTestBin_CXXFLAGS=\
  -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/tests $(AM_CXXFLAGS)

if ENABLE_ANNOTOOLS
tests_RunTestBin_SOURCES += \
	tests/AnnoToolsTest.cpp
tests_RunTestBin_CXXFLAGS +=  -I$(top_srcdir)/AnnoTools/sources
tests_RunTestBin_LDADD += -lannotools
endif

if ENABLE_CUSES
tests_RunTestBin_SOURCES += \
	tests/CUSESTest.cpp
tests_RunTestBin_CXXFLAGS +=  -I$(top_srcdir)/CUSES/sources
tests_RunTestBin_LDADD += -lcuses
endif

if ENABLE_CEVAS
tests_RunTestBin_SOURCES += \
	tests/CeVASTest.cpp
tests_RunTestBin_CXXFLAGS +=  -I$(top_srcdir)/CeVAS/sources
tests_RunTestBin_LDADD += -lcevas
endif

if ENABLE_MALAES
tests_RunTestBin_SOURCES += \
	tests/MaLaESTest.cpp
tests_RunTestBin_CXXFLAGS +=  -I$(top_srcdir)/MaLaES/sources
tests_RunTestBin_LDADD += -lmalaes
endif

if ENABLE_VACSS
tests_RunTestBin_SOURCES += \
	tests/VACSSTest.cpp
tests_RunTestBin_CXXFLAGS +=  -I$(top_srcdir)/VACSS/sources
tests_RunTestBin_LDADD += -lvacss
endif