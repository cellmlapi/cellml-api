DECLARE_EXTENSION(celedsexporter)
DECLARE_IDL(CeLEDSExporter)
DECLARE_IDL_DEPENDENCY(CeLEDS)
DECLARE_IDL_DEPENDENCY(CCGS)
DECLARE_EXTENSION_END(celedsexporter)

INCLUDE_DIRECTORIES(CeLEDSExporter/sources)

ADD_LIBRARY(celedsexporter
  CeLEDSExporter/sources/CeLEDSExporterImpl.cpp)

DECLARE_BOOTSTRAP("CeLEDSExporterBootstrap" "CeLEDSExporter" "CeLEDSExporterBootstrap" "cellml_services" "createCeLEDSExporterBootstrap" "CreateCeLEDSExporterBootstrap" "CeLEDSExporterBootstrap.hpp")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(testCeLEDS CeLEDSExporter/tests/testCeLEDS.cpp)
  TARGET_LINK_LIBRARIES(testCeLEDS cellml xml2 cuses vacss annotools celeds celedsexporter malaes ccgs cevas)
  ADD_TEST(CheckCeLEDS tests/CheckCeLEDS)
  DECLARE_TEST_LIB(celeds)
  DECLARE_TEST_LIB(celedsexporter)
ENDIF()
