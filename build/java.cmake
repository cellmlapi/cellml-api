INCLUDE(FindJNI)
INCLUDE(FindJava)
INCLUDE_DIRECTORIES(simple_interface_generators/glue/java ${JNI_INCLUDE_DIRS})

IF (CHECK_BUILD AND NOT JNI_FOUND)
  MESSAGE(FATAL_ERROR "Java Native Interface libraries / includes were not found, but you have enabled Java support. To override the pre-build checks and manually fix any problems, pass -DCHECK_BUILD:BOOL=OFF to CMake.")
ENDIF()

SET(ALL_JAVA_FILES)
SET(ALL_JAVACLASS_FILES)

LIST(APPEND ALL_JAVA_FILES "${CMAKE_SOURCE_DIR}/simple_interface_generators/glue/java/pjm/XPCOMDerived.java")
LIST(APPEND ALL_JAVACLASS_FILES "${CMAKE_BINARY_DIR}/javacp/pjm/XPCOMDerived.class")
LIST(APPEND ALL_JAVA_FILES "${CMAKE_SOURCE_DIR}/simple_interface_generators/glue/java/pjm/Reference.java")
LIST(APPEND ALL_JAVACLASS_FILES "${CMAKE_BINARY_DIR}/javacp/pjm/Reference.class")

FOREACH(extension ${EXTENSION_LIST})
  SET(${extension}_java_bridge_files)
  SET(java_${extension}_lib_files)
  SET(${extension}_java_defines_lib)
  SET(${extension}_java_defines_module IN_JAVA_WRAPPER)

  FOREACH(idlname ${IDL_LIST_${extension}})
    SET(${idlname}_EXTENSION ${extension})
    SET(THESE_JAVA_FILES)
    FOREACH(iface ${${idlname}_INTERFACES})
      LIST(APPEND ALL_JAVA_FILES interfaces/${${idlname}_NAMESPACE}/${iface}.java)
      LIST(APPEND THESE_JAVA_FILES interfaces/${${idlname}_NAMESPACE}/${iface}.java)
      LIST(APPEND ALL_JAVACLASS_FILES "${CMAKE_BINARY_DIR}/javacp/${${idlname}_NAMESPACE}/${iface}.class")

      LIST(APPEND ALL_JAVA_FILES interfaces/pjm2pcm/${${idlname}_NAMESPACE}/${iface}.java)
      LIST(APPEND THESE_JAVA_FILES interfaces/pjm2pcm/${${idlname}_NAMESPACE}/${iface}.java)
      LIST(APPEND ALL_JAVACLASS_FILES "${CMAKE_BINARY_DIR}/javacp/pjm2pcm/${${idlname}_NAMESPACE}/${iface}.class")
    ENDFOREACH(iface)
    FOREACH(enum ${${idlname}_ENUMS})
      LIST(APPEND ALL_JAVA_FILES interfaces/${${idlname}_NAMESPACE}/${enum}.java)
      LIST(APPEND THESE_JAVA_FILES interfaces/${${idlname}_NAMESPACE}/${enum}.java)
      LIST(APPEND ALL_JAVACLASS_FILES "${CMAKE_BINARY_DIR}/javacp/${${idlname}_NAMESPACE}/${enum}.class")
    ENDFOREACH(enum)

    SET(idlpath "interfaces/${idlname}.idl")

    SET(dofirst)
    FOREACH(idldep ${IDL_DEPS_${extension}})
      LIST(APPEND dofirst "interfaces/p2j${idldep}.hxx")
    ENDFOREACH(idldep)

    ADD_CUSTOM_COMMAND(OUTPUT ${THESE_JAVA_FILES} interfaces/p2j${idlname}.cpp interfaces/p2j${idlname}.hxx
	interfaces/j2p${idlname}Mod.cpp interfaces/j2p${idlname}Sup.cpp interfaces/j2p${idlname}.hxx 
      COMMAND ${OMNIIDL} -bjava -Iinterfaces -p${CMAKE_SOURCE_DIR}/simple_interface_generators/omniidl_be ${CMAKE_SOURCE_DIR}/${idlpath}
      MAIN_DEPENDENCY ${idlpath} DEPENDS
      simple_interface_generators/omniidl_be/java/__init__.py
      simple_interface_generators/omniidl_be/java/interfacegen.py
      simple_interface_generators/omniidl_be/java/j2pcm.py
      simple_interface_generators/omniidl_be/java/jnutils.py
      simple_interface_generators/omniidl_be/java/nativeclassgen.py
      simple_interface_generators/omniidl_be/java/pcm2j.py
      ${dofirst}
      WORKING_DIRECTORY interfaces VERBATIM)
    LIST(APPEND ${extension}_java_bridge_files interfaces/p2j${idlname}.cpp interfaces/j2p${idlname}Sup.cpp)
    LIST(APPEND java_${extension}_lib_files interfaces/j2p${idlname}Mod.cpp)
    LIST(APPEND ${extension}_java_defines_lib MODULE_CONTAINS_P2J__${idlname})
    LIST(APPEND ${extension}_java_defines_lib IN_LIBRARY_J2P__${idlname})
    LIST(APPEND ${extension}_java_defines_module IN_MODULE_J2P__${idlname})
    INSTALL(FILES ${CMAKE_BINARY_DIR}/interfaces/p2j${idlname}.hxx ${CMAKE_BINARY_DIR}/interfaces/j2p${idlname}.hxx DESTINATION include)
  ENDFOREACH(idlname)
ENDFOREACH(extension)

FOREACH(bootstrap ${BOOTSTRAP_LIST})
  IF(NOT EXISTS "${CMAKE_BINARY_DIR}/javagen/cellml_bootstrap/${bootstrap}.java")
    FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/javagen/cellml_bootstrap")
    FILE(WRITE "${CMAKE_BINARY_DIR}/javagen/cellml_bootstrap/${bootstrap}.java" "package cellml_bootstrap;\npublic class ${bootstrap}\n{\n  public static native ${BOOTSTRAP_${bootstrap}_IFACEMODULE}.${BOOTSTRAP_${bootstrap}_IFACE} ${BOOTSTRAP_${bootstrap}_METHOD}();\n};\n")
  ENDIF()
  STRING(REPLACE "_" "_1" IFACE_JESCAPE ${bootstrap})
  IF(NOT EXISTS "${CMAKE_BINARY_DIR}/javagen/${bootstrap}Java.cpp")
    FILE(WRITE "${CMAKE_BINARY_DIR}/javagen/${bootstrap}Java.cpp" "#include \"Utilities.hxx\"\n#include <exception>\n#include \"pick-jni.h\"\n#include \"j2p${BOOTSTRAP_${bootstrap}_IDL}.hxx\"\n#include \"${BOOTSTRAP_${bootstrap}_HEADER}\"\nextern \"C\" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_${IFACE_JESCAPE}_${BOOTSTRAP_${bootstrap}_METHOD}(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }\n\njobject\nJava_cellml_1bootstrap_${IFACE_JESCAPE}_${BOOTSTRAP_${bootstrap}_METHOD}(JNIEnv* env, jclass clazz)\n{\n  RETURN_INTO_OBJREF(b, iface::${BOOTSTRAP_${bootstrap}_IFACEMODULE}::${BOOTSTRAP_${bootstrap}_IFACE}, ${BOOTSTRAP_${bootstrap}_METHODCXX}());\n  return wrap_${BOOTSTRAP_${bootstrap}_IFACEMODULE}_${BOOTSTRAP_${bootstrap}_IFACE}(env, b);\n}\n")
  ENDIF()
  SET(bslib ${BOOTSTRAP_${bootstrap}_LIBASSOC})
  LIST(APPEND java_${bslib}_lib_files "${CMAKE_BINARY_DIR}/javagen/${bootstrap}Java.cpp")
  LIST(APPEND ALL_JAVA_FILES_NODEPEND "${CMAKE_BINARY_DIR}/javagen/cellml_bootstrap/${bootstrap}.java")
  LIST(APPEND ALL_JAVACLASS_FILES "${CMAKE_BINARY_DIR}/javacp/cellml_bootstrap/${bootstrap}.class")
ENDFOREACH(bootstrap)

LIST(APPEND cellml_java_bridge_files simple_interface_generators/glue/java/p2jxpcom.cpp)
LIST(APPEND cellml_java_defines_lib IN_MODULE_JavaSupport)
FOREACH(extension ${EXTENSION_LIST})
  ADD_LIBRARY(java_${extension} MODULE ${java_${extension}_lib_files})
  IF (WIN32) # Otherwise the filename is wrong on MingW.
    SET_PROPERTY(TARGET java_${extension} PROPERTY OUTPUT_NAME "java_${extension}")
  ELSEIF (APPLE)
    SET_PROPERTY(TARGET java_${extension} PROPERTIES PREFIX "lib" SUFFIX ".jnilib")
  ENDIF ()
  SET_PROPERTY(TARGET java_${extension} PROPERTY COMPILE_DEFINITIONS ${${extension}_java_defines_module})
  ADD_LIBRARY(${extension}_java_bridge ${${extension}_java_bridge_files})
  SET_PROPERTY(TARGET ${extension}_java_bridge PROPERTY COMPILE_DEFINITIONS ${${extension}_java_defines_lib})
  INSTALL(TARGETS java_${extension} DESTINATION lib)
  INSTALL(TARGETS ${extension}_java_bridge DESTINATION lib)
  SET(deplibs)
  FOREACH(dep ${IDL_DEPS_${extension}})
    SET(depextn ${${dep}_EXTENSION})
    LIST(APPEND deplibs ${depextn}_java_bridge)
  ENDFOREACH(dep)
  IF (NOT "${deplibs}" STREQUAL "")
    LIST(REMOVE_DUPLICATES deplibs)
  ENDIF()
  TARGET_LINK_LIBRARIES(${extension}_java_bridge ${deplibs} cellml ${extension})
  TARGET_LINK_LIBRARIES(java_${extension} ${deplibs} ${extension}_java_bridge cellml ${extension})
ENDFOREACH(extension)

FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/javacp")
ADD_CUSTOM_COMMAND(OUTPUT ${ALL_JAVACLASS_FILES}
  COMMAND ${Java_JAVAC_EXECUTABLE} ${ALL_JAVA_FILES} ${ALL_JAVA_FILES_NODEPEND} -d "${CMAKE_BINARY_DIR}/javacp"
  DEPENDS ${ALL_JAVA_FILES})

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/cellml.jar COMMAND ${Java_JAR_EXECUTABLE} cf ${CMAKE_BINARY_DIR}/cellml.jar -C "${CMAKE_BINARY_DIR}/javacp" .
  DEPENDS ${ALL_JAVACLASS_FILES} VERBATIM)
ADD_CUSTOM_TARGET(JAVA_BUILD ALL DEPENDS cellml.jar)
INSTALL(FILES ${CMAKE_BINARY_DIR}/cellml.jar DESTINATION lib)

IF (BUILD_TESTING)
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/javatestcp")
SET(JAVA_TEST_NAMES "TestMain")
SET(JAVA_TEST_JAVAFILE)
SET(JAVA_TEST_CLASSFILE)
FOREACH(test ${JAVA_TEST_NAMES})
  LIST(APPEND JAVA_TEST_JAVAFILE "${CMAKE_SOURCE_DIR}/tests/java/${test}.java")
  LIST(APPEND JAVA_TEST_CLASSFILE "${CMAKE_BINARY_DIR}/javatestcp/${test}.class")
ENDFOREACH(test)
ADD_CUSTOM_COMMAND(OUTPUT ${JAVA_TEST_CLASSFILE}
  COMMAND ${Java_JAVAC_EXECUTABLE} ${JAVA_TEST_JAVAFILE} -d ${CMAKE_BINARY_DIR}/javatestcp/ -cp ${CMAKE_BINARY_DIR}/javacp/
  DEPENDS ${JAVA_TEST_JAVAFILE})
IF (WIN32)
SET(PATHSEP ";")
ELSE()
SET(PATHSEP ":")
ENDIF()
ADD_CUSTOM_TARGET(testclasses ALL DEPENDS ${JAVA_TEST_CLASSFILE})
ADD_TEST(JavaTest ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR} -cp "${CMAKE_BINARY_DIR}/javatestcp/${PATHSEP}${CMAKE_BINARY_DIR}/javacp/" TestMain DEPENDS ${JAVA_TEST_CLASSFILE})
ENDIF(BUILD_TESTING)
