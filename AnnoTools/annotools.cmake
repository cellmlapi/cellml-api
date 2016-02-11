DECLARE_EXTENSION(annotools)
DECLARE_IDL(AnnoTools)
DECLARE_IDL_DEPENDENCY(DOM_APISPEC)
DECLARE_IDL_DEPENDENCY(MathML_content_APISPEC)
DECLARE_IDL_DEPENDENCY(CellML_APISPEC)
DECLARE_EXTENSION_END(annotools)

INCLUDE_DIRECTORIES(AnnoTools/sources)

ADD_LIBRARY(annotools
  AnnoTools/sources/ATImplementation.cpp)
TARGET_LINK_LIBRARIES(annotools cellml ${CMAKE_DL_LIBS})
target_link_libraries(libcellml INTERFACE annotools)
SET_TARGET_PROPERTIES(annotools PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${ANNOTOOLS_SOVERSION})
INSTALL(TARGETS annotools
    EXPORT libcellml-config
    DESTINATION lib
    INCLUDES DESTINATION ${LIBCELLML_INCLUDE_DEST})
INSTALL(FILES AnnoTools/sources/AnnoToolsBootstrap.hpp DESTINATION ${LIBCELLML_INCLUDE_DEST})    

DECLARE_BOOTSTRAP("AnnoToolsBootstrap" "AnnoTools" "AnnotationToolService" "cellml_services" "createAnnotationToolService" "CreateAnnotationToolService" "AnnoToolsBootstrap.hpp" "AnnoTools/sources" "annotools")
DECLARE_CPPUNIT_FILE(AnnoTools)
DECLARE_TEST_LIB(annotools)
