DECLARE_EXTENSION(annotools)
DECLARE_IDL(AnnoTools)
DECLARE_EXTENSION_END(annotools)

INCLUDE_DIRECTORIES(AnnoTools/sources)

ADD_LIBRARY(annotools
  AnnoTools/sources/ATImplementation.cpp)

DECLARE_BOOTSTRAP("AnnoToolsBootstrap" "AnnoTools" "AnnotationToolService" "cellml_services" "createAnnotationToolService" "CreateAnnotationToolService" "AnnoToolsBootstrap.hpp")
DECLARE_CPPUNIT_FILE(AnnoTools)
DECLARE_TEST_LIB(annotools)
