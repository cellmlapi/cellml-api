DECLARE_EXTENSION(spros)
DECLARE_IDL(SProS)
DECLARE_EXTENSION_END(spros)

INCLUDE_DIRECTORIES(SProS/sources)

ADD_LIBRARY(spros
  SProS/sources/SProSImpl.cpp)

DECLARE_BOOTSTRAP("SProSBootstrap" "SProS" "Bootstrap" "SProS" "createSProSBootstrap" "CreateSProSBootstrap" "SProSBootstrap.hpp")

DECLARE_CPPUNIT_FILE(SProS)
DECLARE_TEST_LIB(spros)
