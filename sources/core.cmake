DECLARE_EXTENSION(cellml)
DECLARE_IDL(CellML_APISPEC)
DECLARE_IDL(CellML_events)
DECLARE_IDL(DOM_APISPEC)
DECLARE_IDL(DOM_events)
DECLARE_IDL(MathML_content_APISPEC)
IF (ENABLE_RDF)
  DECLARE_IDL(RDF_APISPEC)
ENDIF()
DECLARE_EXTENSION_END(cellml)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(sources)
INCLUDE_DIRECTORIES(sources/dom)
INCLUDE_DIRECTORIES(sources/dom_direct)
INCLUDE_DIRECTORIES(sources/cellml)
INCLUDE_DIRECTORIES(sources/mathml)

IF (ENABLE_RDF)
  SET(MAYBE_RDF_SOURCES sources/rdf/RDFImplementation.cpp)
ELSE()
  SET(MAYBE_RDF_SOURCES)
ENDIF()

ADD_LIBRARY(cellml
    sources/dom/DOMBootstrap.cpp
    sources/dom/DOMWriter.cpp
    sources/dom_direct/DOMImplementation.cpp
    sources/dom_direct/DOMLoader.cpp
    sources/cellml/CellMLBootstrap.cpp
    sources/cellml/CellMLImplementation.cpp
    sources/cellml/CellMLEvents.cpp
    sources/mathml/MathMLImplementation.cpp
    ${MAYBE_RDF_SOURCES}
  )

DECLARE_BOOTSTRAP("CellMLBootstrap" "CellML_APISPEC" "CellMLBootstrap" "cellml_api" "createCellMLBootstrap" "CreateCellMLBootstrap" "CellMLBootstrap.hpp")
