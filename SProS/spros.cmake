DECLARE_EXTENSION(spros)
DECLARE_IDL(SProS)
DECLARE_IDL_DEPENDENCY(MathML_content_APISPEC)
DECLARE_EXTENSION_END(spros)

INCLUDE_DIRECTORIES(SProS/sources)

ADD_LIBRARY(spros
  SProS/sources/SProSImpl.cpp)
TARGET_LINK_LIBRARIES(spros cellml)
INSTALL(TARGETS spros DESTINATION lib)

DECLARE_BOOTSTRAP("SProSBootstrap" "SProS" "Bootstrap" "SProS" "createSProSBootstrap" "CreateSProSBootstrap" "SProSBootstrap.hpp" "SProS/sources" "spros")

DECLARE_CPPUNIT_FILE(SProS)
DECLARE_TEST_LIB(spros)
