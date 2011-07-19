DECLARE_EXTENSION(cellml)
DECLARE_IDL(CellML_APISPEC)
DECLARE_IDL(CellML_events)
DECLARE_IDL(DOM_APISPEC)
DECLARE_IDL(DOM_events)
DECLARE_IDL(MathML_content_APISPEC)
IF (ENABLE_RDF)
  DECLARE_IDL(RDF_APISPEC)
ENDIF()
IF (ENABLE_CONTEXT)
  DECLARE_IDL(CellML_Context)
ENDIF()
DECLARE_EXTENSION_END(cellml)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(sources)
INCLUDE_DIRECTORIES(sources/dom)
INCLUDE_DIRECTORIES(sources/dom_direct)
INCLUDE_DIRECTORIES(sources/cellml)
INCLUDE_DIRECTORIES(sources/mathml)

IF (ENABLE_RDF)
  INCLUDE_DIRECTORIES(sources/rdf)
  SET(MAYBE_RDF_SOURCES sources/rdf/RDFImplementation.cpp)
ELSE()
  SET(MAYBE_RDF_SOURCES)
ENDIF()

IF (ENABLE_CONTEXT)
  INCLUDE_DIRECTORIES(sources/cellml_context)
  SET(MAYBE_CONTEXT_SOURCES sources/cellml_context/CellMLContextImplementation.cpp)
ELSE()
  SET(MAYBE_CONTEXT_SOURCES)
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
    ${MAYBE_CONTEXT_SOURCES}
  )
TARGET_LINK_LIBRARIES(cellml xml2)
INSTALL(TARGETS cellml DESTINATION lib)

DECLARE_BOOTSTRAP("CellMLBootstrap" "CellML_APISPEC" "CellMLBootstrap" "cellml_api" "createCellMLBootstrap" "CreateCellMLBootstrap" "CellMLBootstrap.hpp" "sources/cellml" "cellml")
DECLARE_CPPUNIT_FILE(DOM)
DECLARE_CPPUNIT_FILE(MathML)
DECLARE_CPPUNIT_FILE(CellML)
DECLARE_CPPUNIT_FILE(CellMLEvents)

IF(ENABLE_RDF)
  DECLARE_CPPUNIT_FILE(RDF)
ENDIF()

IF(ENABLE_CONTEXT)
  DECLARE_CPPUNIT_FILE(CellMLContext)
ENDIF()

INCLUDE(FindLibXml2)
IF (CHECK_BUILD AND NOT LIBXML2_FOUND)
  MESSAGE(FATAL_ERROR "libxml2 libraries / includes were not found. To override the pre-build checks and manually fix any problems, pass -DCHECK_BUILD:BOOL=OFF to CMake.")
ENDIF()
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

DECLARE_TEST_LIB(xml2)
DECLARE_TEST_LIB(cellml)
