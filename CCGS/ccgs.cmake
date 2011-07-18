DECLARE_EXTENSION(ccgs)
DECLARE_IDL(CCGS)
DECLARE_IDL_DEPENDENCY(MaLaES)
DECLARE_IDL_DEPENDENCY(CUSES)
DECLARE_IDL_DEPENDENCY(CeVAS)
DECLARE_EXTENSION_END(ccgs)

INCLUDE_DIRECTORIES(CCGS/sources)

ADD_LIBRARY(ccgs
  CCGS/sources/CCGSImplementation.cpp
  CCGS/sources/CCGSGenerator.cpp)
INSTALL(TARGETS ccgs DESTINATION lib)

DECLARE_BOOTSTRAP("CCGSBootstrap" "CCGS" "CodeGeneratorBootstrap" "cellml_services" "createCodeGeneratorBootstrap" "CreateCodeGeneratorBootstrap" "CCGSBootstrap.hpp" "CCGS/sources")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(CellML2C CCGS/tests/CellML2C.cpp)
  TARGET_LINK_LIBRARIES(CellML2C cellml ccgs xml2 cuses cevas malaes annotools)
  ADD_TEST(CheckCodeGenerator tests/CheckCodeGenerator)
  DECLARE_TEST_LIB(ccgs)
ENDIF()
