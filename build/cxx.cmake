SET(CXX_IFACE_HEADER_LIST)
SET(IDL_LIST_CXX ${IDL_LIST})
LIST(APPEND IDL_LIST_CXX xpcom)
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/interfaces")
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/interfaces)
FOREACH(idlfile ${IDL_LIST_CXX})
  SET(idlpath "${CMAKE_SOURCE_DIR}/interfaces/${idlfile}.idl")
  SET(headerpath "${CMAKE_BINARY_DIR}/interfaces/Iface${idlfile}.hxx")
  ADD_CUSTOM_COMMAND(OUTPUT ${headerpath}
                     COMMAND ${OMNIIDL} -bsimple_cpp -Iinterfaces -p${CMAKE_SOURCE_DIR}/simple_interface_generators/omniidl_be ${idlpath}
                     MAIN_DEPENDENCY ${idlpath}
                     DEPENDS simple_interface_generators/omniidl_be/simple_cpp/cxxheadergen.py
                             simple_interface_generators/omniidl_be/simple_cpp/__init__.py
                             simple_interface_generators/omniidl_be/simple_cpp/simplecxx.py
                             simple_interface_generators/omniidl_be/simple_cpp/identifier.py
                     WORKING_DIRECTORY interfaces
                     VERBATIM)
  LIST(APPEND CXX_IFACE_HEADER_LIST ${headerpath})
  INSTALL(FILES ${headerpath} DESTINATION include)
ENDFOREACH(idlfile)

ADD_CUSTOM_TARGET(CXX_HEADERS_FOR_INTERFACES ALL DEPENDS ${CXX_IFACE_HEADER_LIST})

FOREACH(name ${BOOTSTRAP_LIST})
  INSTALL(FILES ${BOOTSTRAP_${name}_HEADERPATH}/${BOOTSTRAP_${name}_HEADER} DESTINATION include)
ENDFOREACH(name)
