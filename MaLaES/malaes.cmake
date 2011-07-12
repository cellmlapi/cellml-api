DECLARE_EXTENSION(malaes)
DECLARE_IDL(MaLaES)
DECLARE_EXTENSION_END(malaes)

INCLUDE_DIRECTORIES(MaLaES/sources)

ADD_LIBRARY(malaes
  MaLaES/sources/MaLaESImpl.cpp)

DECLARE_BOOTSTRAP("MaLaESBootstrap" "MaLaES" "MaLaESBootstrap" "cellml_services" "createMaLaESBootstrap" "CreateMaLaESBootstrap" "MaLaESBootstrap.hpp")

DECLARE_CPPUNIT_FILE(MaLaES)
DECLARE_TEST_LIB(malaes)
