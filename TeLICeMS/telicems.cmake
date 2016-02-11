DECLARE_EXTENSION(telicems)
DECLARE_IDL(TeLICeMS)
DECLARE_IDL_DEPENDENCY(CellML_APISPEC)
DECLARE_EXTENSION_END(telicems)

INCLUDE_DIRECTORIES(TeLICeMS/sources)

INCLUDE(FindBISON)
INCLUDE(FindFLEX)

BISON_TARGET(TeLICeMParse TeLICeMS/sources/TeLICeMParse.yy ${CMAKE_BINARY_DIR}/TeLICeMParse.gen.cpp)
FLEX_TARGET(TeLICeMScan TeLICeMS/sources/TeLICeMScan.l ${CMAKE_BINARY_DIR}/TeLICeMScanner.cpp)

ADD_LIBRARY(telicems
  TeLICeMS/sources/TeLICeMSImpl.cpp
  ${CMAKE_BINARY_DIR}/TeLICeMScanner.cpp
  ${CMAKE_BINARY_DIR}/TeLICeMParse.genf.cpp)
TARGET_LINK_LIBRARIES(telicems cellml ${CMAKE_DL_LIBS})
SET_TARGET_PROPERTIES(telicems PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${TELICEMS_SOVERSION})
target_link_libraries(libcellml INTERFACE telicems)
INSTALL(TARGETS telicems DESTINATION lib)

DECLARE_BOOTSTRAP("TeLICeMSService" "TeLICeMS" "TeLICeMService" "cellml_services" "createTeLICeMService" "CreateTeLICeMService" "TeLICeMService.hpp" "TeLICeMS/sources" "telicems")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(TestTeLICeMParser TeLICeMS/tests/TestTeLICeMParser.cpp)
  ADD_EXECUTABLE(TestTeLICeMSerialiser TeLICeMS/tests/TestTeLICeMSerialiser.cpp)
  TARGET_LINK_LIBRARIES(TestTeLICeMParser cellml telicems)
  TARGET_LINK_LIBRARIES(TestTeLICeMSerialiser cellml telicems)
  ADD_TEST(CheckTeLICeMSParser ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/tests/CheckTeLICeMSParser)
ENDIF()


ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/TeLICeMParse.genf.cpp
                   COMMAND ${BASH} "${CMAKE_SOURCE_DIR}/TeLICeMS/FixBison" "${CMAKE_BINARY_DIR}/TeLICeMParse.gen.cpp" "${CMAKE_BINARY_DIR}/TeLICeMParse.genf.cpp"
                   DEPENDS ${CMAKE_BINARY_DIR}/TeLICeMParse.gen.cpp ${CMAKE_SOURCE_DIR}/TeLICeMS/FixBison)
