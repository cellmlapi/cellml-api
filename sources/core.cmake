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

OPTION(USE_SYSTEM_LIBXML2 "Use a system or external libxml2 rather than the one included with the CellML API" ON)
MARK_AS_ADVANCED(USE_SYSTEM_LIBXML2)

if(USE_SYSTEM_LIBXML2)
  find_package(LibXml2 ${LIBXML2_VERSION} REQUIRED)
  #INCLUDE_DIRECTORIES(SYSTEM ${LIBXML2_INCLUDE_DIR})
  #SET(SYSTEM_XML2 ${LIBXML2_LIBRARIES})
  set(EXTDEP_SOURCES)
else()
  SET(SYSTEM_XML2)
  INCLUDE_DIRECTORIES(extdep/libxml)
  INCLUDE_DIRECTORIES(SYSTEM extdep/libxml/include)
  SET(EXTDEP_SOURCES
    extdep/libxml/catalog.c
    extdep/libxml/chvalid.c
    extdep/libxml/debugXML.c
    extdep/libxml/dict.c
    extdep/libxml/encoding.c
    extdep/libxml/entities.c
    extdep/libxml/error.c
    extdep/libxml/globals.c
    extdep/libxml/hash.c
    extdep/libxml/HTMLparser.c
    extdep/libxml/HTMLtree.c
    extdep/libxml/legacy.c
    extdep/libxml/list.c
    extdep/libxml/nanoftp.c
    extdep/libxml/nanohttp.c
    extdep/libxml/parser.c
    extdep/libxml/parserInternals.c
    extdep/libxml/pattern.c
    extdep/libxml/SAX2.c
    extdep/libxml/SAX.c
    extdep/libxml/schematron.c
    extdep/libxml/threads.c
    extdep/libxml/tree.c
    extdep/libxml/uri.c
    extdep/libxml/valid.c
    extdep/libxml/xinclude.c
    extdep/libxml/xlink.c
    extdep/libxml/xmlIO.c
    extdep/libxml/xmlmemory.c
    extdep/libxml/xmlreader.c
    extdep/libxml/xmlregexp.c
    extdep/libxml/xmlsave.c
    extdep/libxml/xmlschemas.c
    extdep/libxml/xmlschemastypes.c
    extdep/libxml/xmlstring.c
    extdep/libxml/xmlunicode.c
    extdep/libxml/xmlwriter.c
    extdep/libxml/xpath.c
    extdep/libxml/xpointer.c
    )
ENDIF()

ADD_LIBRARY(cellml
    sources/Utilities.cpp
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
    ${EXTDEP_SOURCES}
  )
SET(NETWORK_LIBS)
IF(WIN32)
    LIST(APPEND NETWORK_LIBS ws2_32)
ENDIF()
find_package(Threads)
target_link_libraries(cellml PUBLIC ${CMAKE_DL_LIBS} ${NETWORK_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (BUILD_SHARED_LIBS)
    target_compile_definitions(cellml PRIVATE cellml_EXPORTS)
else()
     target_compile_definitions(cellml 
        PUBLIC $<BUILD_INTERFACE:CELLML_STATIC>)
endif()
if (USE_SYSTEM_LIBXML2)
    target_link_libraries(cellml PUBLIC xml2)
endif()
set_target_properties(cellml PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${CELLML_SOVERSION} OUTPUT_NAME cellml-${LIBCELLML_VERSION})
INSTALL(TARGETS cellml
    EXPORT libcellml-config 
    DESTINATION lib
    INCLUDES DIRECTORY ${LIBCELLML_INCLUDE_DEST})        
INSTALL(FILES sources/cellml/CellMLBootstrap.hpp sources/cda_compiler_support.h
    ${CMAKE_BINARY_DIR}/cda_config.h sources/cellml-api-cxx-support.hpp
    sources/dom/DOMBootstrap.hxx
    DESTINATION ${LIBCELLML_INCLUDE_DEST})

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

DECLARE_TEST_LIB(cellml)
