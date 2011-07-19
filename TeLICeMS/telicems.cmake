DECLARE_EXTENSION(telicems)
DECLARE_IDL(TeLICeMS)
DECLARE_IDL_DEPENDENCY(CellML_APISPEC)
DECLARE_EXTENSION_END(telicems)

INCLUDE_DIRECTORIES(TeLICeMS/sources)

INCLUDE(FindBISON)
INCLUDE(FindFLEX)

BISON_TARGET(TeLICeMParse TeLICeMS/sources/TeLICeMParse.yy TeLICeMParse.gen.cpp)
FLEX_TARGET(TeLICeMScan TeLICeMS/sources/TeLICeMScan.l TeLICeMScanner.cpp)

ADD_LIBRARY(telicems
  TeLICeMS/sources/TeLICeMSImpl.cpp
  TeLICeMScanner.cpp
  TeLICeMParse.gen.cpp)
TARGET_LINK_LIBRARIES(telicems cellml)
INSTALL(TARGETS telicems DESTINATION lib)

DECLARE_BOOTSTRAP("TeLICeMSService" "TeLICeMS" "TeLICeMService" "cellml_services" "createTeLICeMService" "CreateTeLICeMService" "TeLICeMService.hpp" "TeLICeMS/sources" "telicems")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(TestTeLICeMParser TeLICeMS/tests/TestTeLICeMParser.cpp)
  ADD_EXECUTABLE(TestTeLICeMSerialiser TeLICeMS/tests/TestTeLICeMSerialiser.cpp)
  TARGET_LINK_LIBRARIES(TestTeLICeMParser cellml xml2 telicems)
  TARGET_LINK_LIBRARIES(TestTeLICeMSerialiser cellml xml2 telicems)
  ADD_TEST(CheckTeLICeMSParser tests/CheckTeLICeMSParser)
ENDIF()
