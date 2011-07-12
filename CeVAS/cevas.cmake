DECLARE_EXTENSION(cevas)
DECLARE_IDL(CeVAS)
DECLARE_EXTENSION_END(cevas)

INCLUDE_DIRECTORIES(CeVAS/sources)

ADD_LIBRARY(cevas
  CeVAS/sources/CeVASImpl.cpp)

DECLARE_BOOTSTRAP("CeVASBootstrap" "CeVAS" "CeVASBootstrap" "cellml_services" "createCeVASBootstrap" "CreateCeVASBootstrap" "CeVASBootstrap.hpp")

DECLARE_CPPUNIT_FILE(CeVAS)
DECLARE_TEST_LIB(cevas)
