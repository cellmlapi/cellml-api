DECLARE_EXTENSION(celedsexporter)
DECLARE_IDL(CeLEDSExporter)
DECLARE_IDL_DEPENDENCY(CeLEDS)
DECLARE_IDL_DEPENDENCY(CCGS)
DECLARE_EXTENSION_END(celedsexporter)

INCLUDE_DIRECTORIES(CeLEDSExporter/sources)

ADD_LIBRARY(celedsexporter
  CeLEDSExporter/sources/CeLEDSExporterImpl.cpp)
TARGET_LINK_LIBRARIES(celedsexporter celeds ccgs ${CMAKE_DL_LIBS})
SET_TARGET_PROPERTIES(celedsexporter PROPERTIES SOVERSION ${GLOBAL_SOVERSION})
INSTALL(TARGETS celedsexporter DESTINATION lib)

DECLARE_BOOTSTRAP("CeLEDSExporterBootstrap" "CeLEDSExporter" "CeLEDSExporterBootstrap" "cellml_services" "createCeLEDSExporterBootstrap" "CreateCeLEDSExporterBootstrap" "CeLEDSExporterBootstrap.hpp" "CeLEDSExporter/sources" "celeds_exporter")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(testCeLEDS CeLEDSExporter/tests/testCeLEDS.cpp)
  TARGET_LINK_LIBRARIES(testCeLEDS cellml cuses vacss annotools celeds celedsexporter malaes ccgs cevas)
  ADD_TEST(CheckCeLEDS ${BASH} tests/CheckCeLEDS)
  DECLARE_TEST_LIB(celeds)
  DECLARE_TEST_LIB(celedsexporter)
ENDIF()
