DECLARE_EXTENSION(vacss)
DECLARE_IDL(VACSS)
DECLARE_EXTENSION_END(vacss)

INCLUDE_DIRECTORIES(VACSS/sources)

ADD_LIBRARY(vacss
  VACSS/sources/VACSSImpl.cpp)

DECLARE_BOOTSTRAP("VACSSBootstrap" "VACSS" "VACSService" "cellml_services" "createVACSService" "CreateVACSService" "VACSSBootstrap.hpp")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(ValidateCellML VACSS/tests/ValidateCellML.cpp)
  TARGET_LINK_LIBRARIES(ValidateCellML cellml xml2 cuses vacss annotools)
  ADD_TEST(CheckVACSS tests/CheckVACSS)
  DECLARE_TEST_LIB(vacss)
  DECLARE_CPPUNIT_FILE(VACSS)
ENDIF()

DECLARE_TEST_LIB(vacss)
