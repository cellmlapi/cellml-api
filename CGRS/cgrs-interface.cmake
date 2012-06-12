# This file describes how to build the implementation of the CGRS interface,
# but not the individual generics modules.

DECLARE_EXTENSION(cgrs)
DECLARE_IDL(CGRS)
DECLARE_IDL_DEPENDENCY(CellML_APISPEC)
DECLARE_EXTENSION_END(cgrs)

INCLUDE_DIRECTORIES(CGRS)

ADD_LIBRARY(cgrs
  CGRS/CGRSImplementation.cpp)
TARGET_LINK_LIBRARIES(cgrs ${CMAKE_DL_LIBS} cellml)
SET_TARGET_PROPERTIES(cgrs PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${CGRS_SOVERSION})
INSTALL(TARGETS cgrs DESTINATION lib)

DECLARE_BOOTSTRAP("CGRSBootstrap" "CGRS" "GenericsService" "CGRS" "createGenericsService" "CreateGenericsService" "CGRSBootstrap.hpp" "CGRS" "cgrs")

IF (BUILD_TESTING)
  # DECLARE_CPPUNIT_FILE(CGRS)
  DECLARE_TEST_LIB(cgrs)
ENDIF()
