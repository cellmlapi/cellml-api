DECLARE_EXTENSION(mofs)
DECLARE_IDL(MoFS)
DECLARE_IDL_DEPENDENCY(CellML_APISPEC)
DECLARE_EXTENSION_END(mofs)

INCLUDE_DIRECTORIES(MoFS)

ADD_LIBRARY(mofs MoFS/MoFSImpl.cpp)
TARGET_LINK_LIBRARIES(mofs cellml annotools cevas)
SET_TARGET_PROPERTIES(mofs PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${MOFS_SOVERSION})
target_link_libraries(libcellml INTERFACE mofs)
INSTALL(TARGETS mofs DESTINATION lib)

DECLARE_BOOTSTRAP("MoFSBootstrap" "MoFS" "ModelFlatteningService" "mofs" "createModelFlatteningService" "CreateModelFlatteningService" "MoFSBootstrap.hpp" "MoFS" "mofs")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(FlattenModel MoFS/tests/FlattenModel.cpp)
  TARGET_LINK_LIBRARIES(FlattenModel cellml cevas annotools mofs)
  # ADD_TEST(CheckSRuS ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/tests/RetryWrapper ${CMAKE_CURRENT_SOURCE_DIR}/tests/CheckSRuS)
  ADD_TEST(CheckFlatten ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/tests/RetryWrapper ${CMAKE_CURRENT_SOURCE_DIR}/tests/CheckFlatten)
ENDIF()
