# Tests...
check_PROGRAMS=tests/RunTestBin

TESTS=tests/RunTestBin

if ENABLE_CCGS
TESTS+=tests/CheckCodeGenerator
TESTS+=tests/CheckCustomGen
endif
if ENABLE_CELEDS
TESTS+=tests/CheckCeLEDS
endif
if ENABLE_VACSS
TESTS += tests/CheckVACSS
endif
if ENABLE_CIS
TESTS += tests/CheckCIS
endif

tests_RunTestBin_SOURCES=\
	tests/TestOutput.cpp \
	tests/DOMTest.cpp \
	tests/MathMLTest.cpp \
	tests/CellMLTest.cpp \
	tests/CellMLEventsTest.cpp \
        tests/URITest.cpp

tests_RunTestBin_LDADD=-L$(top_builddir) -lcellml -lcppunit -lxml2
tests_RunTestBin_CXXFLAGS=\
  -Wall -I$(top_srcdir)/sources -I$(top_srcdir)/sources/dom \
  -I$(top_srcdir)/sources/mathml -I$(top_srcdir)/sources/cellml \
  -I$(top_builddir)/interfaces -I$(top_srcdir)/tests \
  $(AM_CXXFLAGS)

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

if ENABLE_RDF
tests_RunTestBin_SOURCES += \
	tests/RDFTest.cpp
tests_RunTestBin_CXXFLAGS += -I$(top_srcdir)/sources/rdf
endif

if ENABLE_CONTEXT
tests_RunTestBin_SOURCES += \
	tests/CellMLContextTest.cpp
tests_RunTestBin_CXXFLAGS += -I$(top_srcdir)/sources/cellml_context
tests_RunTestBin_LDADD += -lcellml_context
endif

if ENABLE_SPROS
tests_RunTestBin_SOURCES += \
	tests/SProSTest.cpp
tests_RunTestBin_CXXFLAGS += -I$(top_srcdir)/SProS/sources
tests_RunTestBin_LDADD += -lspros
endif

tests_RunTestBin_SOURCES += \
	tests/XPathTest.cpp
tests_RunTestBin_CXXFLAGS += -I$(top_srcdir)/xpath
tests_RunTestBin_LDADD += -lxpath
