DECLARE_EXTENSION(annotools)
DECLARE_IDL(AnnoTools)
DECLARE_IDL_DEPENDENCY(DOM_APISPEC)
DECLARE_IDL_DEPENDENCY(MathML_content_APISPEC)
DECLARE_IDL_DEPENDENCY(CellML_APISPEC)
DECLARE_EXTENSION_END(annotools)

INCLUDE_DIRECTORIES(AnnoTools/sources)

ADD_LIBRARY(annotools
  AnnoTools/sources/ATImplementation.cpp)
INSTALL(TARGETS annotools DESTINATION lib)

DECLARE_BOOTSTRAP("AnnoToolsBootstrap" "AnnoTools" "AnnotationToolService" "cellml_services" "createAnnotationToolService" "CreateAnnotationToolService" "AnnoToolsBootstrap.hpp" "AnnoTools/sources" "annotools")
DECLARE_CPPUNIT_FILE(AnnoTools)
DECLARE_TEST_LIB(annotools)
