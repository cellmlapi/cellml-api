DECLARE_EXTENSION(vacss)
DECLARE_IDL(VACSS)
DECLARE_IDL_DEPENDENCY(CUSES)
DECLARE_EXTENSION_END(vacss)

INCLUDE_DIRECTORIES(VACSS/sources)

ADD_LIBRARY(vacss
  VACSS/sources/VACSSImpl.cpp)
TARGET_LINK_LIBRARIES(vacss cuses cellml ${CMAKE_DL_LIBS})
SET_TARGET_PROPERTIES(vacss PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${VACSS_SOVERSION})
INSTALL(TARGETS vacss EXPORT libcellml-config DESTINATION lib)

DECLARE_BOOTSTRAP("VACSSBootstrap" "VACSS" "VACSService" "cellml_services" "createVACSService" "CreateVACSService" "VACSSBootstrap.hpp" "VACSS/sources" "vacss")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(ValidateCellML VACSS/tests/ValidateCellML.cpp)
  TARGET_LINK_LIBRARIES(ValidateCellML cellml cuses vacss annotools)
  ADD_TEST(CheckVACSS ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/tests/CheckVACSS)
  DECLARE_TEST_LIB(vacss)
  DECLARE_CPPUNIT_FILE(VACSS)
ENDIF()

DECLARE_TEST_LIB(vacss)
