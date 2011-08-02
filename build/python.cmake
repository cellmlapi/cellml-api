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

FOREACH(extension ${EXTENSION_LIST})
  FOREACH(idlname ${IDL_LIST_${extension}})
    SET(${idlname}_EXTENSION ${extension})
  ENDFOREACH(idlname)
ENDFOREACH(extension)

FOREACH(extension ${EXTENSION_LIST})
  SET(P2PYTHON_BRIDGE_LIST)
  SET(PYTHON2P_BRIDGE_LIST)
  SET(deplibs)
  SET(${extension}_python_defines_module)
  SET(${extension}_python_defines_lib)
  FOREACH(idlfile ${IDL_LIST_${extension}})
    SET(${extension}_python_defines_module IN_PYTHON_MODULE_${idlfile})
    SET(${extension}_python_defines_lib IN_PYTHON_LIB_${idlfile})
    SET(idlpath "interfaces/${idlfile}.idl")
    SET(p2pypath "interfaces/P2Py${idlfile}.cxx")
    SET(p2pyhpath "interfaces/P2Py${idlfile}.hxx")
    SET(py2ppath "interfaces/Py2P${idlfile}.cxx")
    SET(dofirst)
    FOREACH(idldep ${IDL_DEPS_${extension}})
      LIST(APPEND dofirst "interfaces/P2Py${idldep}.hxx")
      LIST(APPEND deplibs "${${idldep}_EXTENSION}_python_bridge")
    ENDFOREACH(idldep)

    ADD_CUSTOM_COMMAND(OUTPUT ${p2pypath} ${py2ppath} ${p2pyhpath} 
      COMMAND ${OMNIIDL} -bpcmpy ${PYOMNIOPTS} -Iinterfaces -p../simple_interface_generators/omniidl_be ../${idlpath}
      MAIN_DEPENDENCY ${idlpath} DEPENDS
              simple_interface_generators/omniidl_be/pcmpy/__init__.py
              simple_interface_generators/omniidl_be/pcmpy/CToPythonWalker.py
              simple_interface_generators/omniidl_be/pcmpy/PythonToCWalker.py
              simple_interface_generators/omniidl_be/pcmpy/identifier.py
              simple_interface_generators/omniidl_be/pcmpy/typeinfo.py
              ${dofirst}
      WORKING_DIRECTORY interfaces VERBATIM)
    INSTALL(FILES ${p2pyhpath} DESTINATION include)

    LIST(APPEND P2PYTHON_BRIDGE_LIST ${p2pypath})
    LIST(APPEND PYTHON2P_BRIDGE_LIST ${py2ppath})
  ENDFOREACH(idlfile)
  ADD_LIBRARY(${extension}_python_bridge ${P2PYTHON_BRIDGE_LIST})
  SET_PROPERTY(TARGET ${extension}_python_bridge PROPERTY COMPILE_DEFINITIONS ${${extension}_python_defines_lib})
  TARGET_LINK_LIBRARIES(${extension}_python_bridge PythonSupport ${deplibs} ${PYTHON_LIBRARIES} ${extension})
  INSTALL(TARGETS ${extension}_python_bridge DESTINATION lib)
  ADD_LIBRARY(python_${extension} MODULE ${PYTHON2P_BRIDGE_LIST})
  SET_PROPERTY(TARGET python_${extension} PROPERTY COMPILE_DEFINITIONS ${${extension}_python_defines_module})
  TARGET_LINK_LIBRARIES(python_${extension} PythonSupport ${deplibs} ${PYTHON_LIBRARIES} ${extension} ${extension}_python_bridge)
  SET_PROPERTY(TARGET python_${extension} PROPERTY LIBRARY_OUTPUT_NAME ${extension})
  SET_PROPERTY(TARGET python_${extension} PROPERTY LIBRARY_OUTPUT_DIRECTORY python)
  INSTALL(TARGETS python_${extension} DESTINATION lib/python)
ENDFOREACH(extension)
