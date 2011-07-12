DECLARE_EXTENSION(celeds)
DECLARE_IDL(CeLEDS)
DECLARE_EXTENSION_END(celeds)

INCLUDE_DIRECTORIES(CeLEDS/sources)

ADD_LIBRARY(celeds
  CeLEDS/sources/CeLEDSImpl.cpp)

DECLARE_BOOTSTRAP("CeLEDSBootstrap" "CeLEDS" "CeLEDSBootstrap" "cellml_services" "createCeLEDSBootstrap" "CreateCeLEDSBootstrap" "CeLEDSBootstrap.hpp")

