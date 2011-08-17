INCLUDE(FindPythonLibs)
INCLUDE_DIRECTORIES(simple_interface_generators/glue/python ${PYTHON_INCLUDE_PATH})

IF (CHECK_BUILD AND NOT PYTHONLIBS_FOUND)
  MESSAGE(FATAL_ERROR "Python libraries / includes were not found, but you have enabled Python support. To override the pre-build checks and manually fix any problems, pass -DCHECK_BUILD:BOOL=OFF to CMake.")
ENDIF()

SET(PYOMNIOPTS)
FOREACH(bootstrap ${BOOTSTRAP_LIST})
  LIST(APPEND PYOMNIOPTS -Wbinclude${BOOTSTRAP_${bootstrap}_IDL}=${BOOTSTRAP_${bootstrap}_HEADER} -Wbbootstrap${BOOTSTRAP_${bootstrap}_IFACEMODULE}::${BOOTSTRAP_${bootstrap}_IFACE}=${BOOTSTRAP_${bootstrap}_METHODCXX})
ENDFOREACH(bootstrap)

FIND_LIBRARY(LIBUTIL util)
IF(LIBUTIL STREQUAL "LIBUTIL-NOTFOUND")
  SET(LINK_LIBUTIL)
ELSE()
  SET(LINK_LIBUTIL ${LIBUTIL})
ENDIF()

ADD_LIBRARY(PythonSupport simple_interface_generators/glue/python/python_support.cxx)
TARGET_LINK_LIBRARIES(PythonSupport ${PYTHON_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${LINK_LIBUTIL} cellml)
INSTALL(TARGETS PythonSupport DESTINATION lib)

ADD_LIBRARY(python_xpcom MODULE simple_interface_generators/glue/python/xpcom.cxx)
TARGET_LINK_LIBRARIES(python_xpcom ${PYTHON_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${LINK_LIBUTIL} cellml)
SET_PROPERTY(TARGET python_xpcom PROPERTY PREFIX "")
SET_PROPERTY(TARGET python_xpcom PROPERTY LIBRARY_OUTPUT_NAME xpcom)
SET_PROPERTY(TARGET python_xpcom PROPERTY LIBRARY_OUTPUT_DIRECTORY python/cellml_api)
INSTALL(TARGETS python_xpcom DESTINATION lib/python/cellml_api)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/python/cellml_api/__init__.py DESTINATION lib/python/cellml_api)

FOREACH(extension ${EXTENSION_LIST})
  FOREACH(idlname ${IDL_LIST_${extension}})
    SET(${idlname}_EXTENSION ${extension})
  ENDFOREACH(idlname)
ENDFOREACH(extension)

FOREACH(extension ${EXTENSION_LIST})
  SET(P2PYTHON_BRIDGE_LIST)
  SET(deplibs)
  SET(${extension}_python_defines_lib)
  FOREACH(idldep ${IDL_DEPS_${extension}})
    LIST(APPEND dofirst "interfaces/P2Py${idldep}.hxx")
    LIST(APPEND deplibs "${${idldep}_EXTENSION}_python_bridge")
  ENDFOREACH(idldep)
  IF (NOT "${deplibs}" STREQUAL "")
    LIST(REMOVE_DUPLICATES deplibs)
  ENDIF()
  FOREACH(idlfile ${IDL_LIST_${extension}})
    LIST(APPEND ${extension}_python_defines_lib IN_PYTHON_LIB_${idlfile})
    SET(idlpath "${CMAKE_SOURCE_DIR}/interfaces/${idlfile}.idl")
    SET(p2pypath "${CMAKE_BINARY_DIR}/interfaces/P2Py${idlfile}.cxx")
    SET(p2pyhpath "${CMAKE_BINARY_DIR}/interfaces/P2Py${idlfile}.hxx")
    SET(py2ppath "${CMAKE_BINARY_DIR}/interfaces/Py2P${idlfile}.cxx")
    SET(dofirst)

    ADD_CUSTOM_COMMAND(OUTPUT ${p2pypath} ${py2ppath} ${p2pyhpath} 
      COMMAND ${OMNIIDL} -bpcmpy ${PYOMNIOPTS} -Wbinclude${BOOTSTRAP_${extension}_IFACE}=${BOOTSTRAP_${extension}_HEADER} -Wbbootstrap${BOOTSTRAP_${extension}_IFACE}=${BOOTSTRAP_${extension}_METHODCXX} -Wbmoduledir=cellml_api -Wbmodulename=${idlfile} -Iinterfaces -p${CMAKE_SOURCE_DIR}/simple_interface_generators/omniidl_be ${idlpath}
      MAIN_DEPENDENCY ${idlpath} DEPENDS
              simple_interface_generators/omniidl_be/pcmpy/__init__.py
              simple_interface_generators/omniidl_be/pcmpy/CToPythonWalker.py
              simple_interface_generators/omniidl_be/pcmpy/PythonToCWalker.py
              simple_interface_generators/omniidl_be/pcmpy/identifier.py
              simple_interface_generators/omniidl_be/pcmpy/typeinfo.py
              ${dofirst}
      WORKING_DIRECTORY interfaces VERBATIM)
    INSTALL(FILES ${p2pyhpath} DESTINATION include)

    ADD_LIBRARY(python_${idlfile} MODULE ${py2ppath})
    SET_PROPERTY(TARGET python_${idlfile} PROPERTY COMPILE_DEFINITIONS IN_PYTHON_MODULE_${idlfile})
    TARGET_LINK_LIBRARIES(python_${idlfile} PythonSupport ${deplibs} ${PYTHON_LIBRARIES} ${extension} ${extension}_python_bridge)
    SET_PROPERTY(TARGET python_${idlfile} PROPERTY PREFIX "")
    SET_PROPERTY(TARGET python_${idlfile} PROPERTY LIBRARY_OUTPUT_NAME ${idlfile})
    SET_PROPERTY(TARGET python_${idlfile} PROPERTY LIBRARY_OUTPUT_DIRECTORY python/cellml_api)
    INSTALL(TARGETS python_${idlfile} DESTINATION lib/python/cellml_api)

    LIST(APPEND P2PYTHON_BRIDGE_LIST ${p2pypath})
  ENDFOREACH(idlfile)
  ADD_LIBRARY(${extension}_python_bridge ${P2PYTHON_BRIDGE_LIST})
  SET_PROPERTY(TARGET ${extension}_python_bridge PROPERTY COMPILE_DEFINITIONS ${${extension}_python_defines_lib})
  TARGET_LINK_LIBRARIES(${extension}_python_bridge PythonSupport ${deplibs} ${PYTHON_LIBRARIES} ${extension})
  INSTALL(TARGETS ${extension}_python_bridge DESTINATION lib)
ENDFOREACH(extension)
