CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
project(LIBCELLML VERSION 2.0 LANGUAGES C CXX)

INCLUDE(build/soversions.cmake)
INCLUDE(build/packing.cmake)
INCLUDE(CTest)

OPTION(ENABLE_EVERYTHING "Turn on all optional features, except the commonly problematic ones (XPCOM, CORBA, and GSL integrators)." OFF)

OPTION(ENABLE_CORBA "Generate a two-way bridge between the C++ implementation and CORBA, allowing all parts of the API to be accessed from CORBA. Note: this functionality is not widely tested; if you find a bug, we are only interested if you also provide a working patch to fix the problem" OFF)
MARK_AS_ADVANCED(FORCE ENABLE_CORBA)
OPTION(ENABLE_XPCOM "Generate XPIDL interfaces and a two-way bridge between the C++ implementation and XPCOM, allowing all parts of the API to be accessed from XPCOM. Note: this functionality is not widely tested; if you find a bug, we are only interested if you also provide a working patch to fix the problem" OFF)
OPTION(ENABLE_JAVA_BUILD "Build Java interfaces and a bridge between the C++ implementation and Java, allowing all parts of the API to be accessed from Java." OFF)

MARK_AS_ADVANCED(FORCE ENABLE_XPCOM)
SET(XULRUNNER_SDK_PATH "/usr/local/xulrunner-sdk" CACHE PATH "This property is only used when XPCOM support has been enabled. It determines the path to the xulrunner SDK used to build the XPCOM bridge")
MARK_AS_ADVANCED(FORCE XULRUNNER_SDK_PATH)

OPTION(ENABLE_ANNOTOOLS "Build AnnoTools, a service for handling sets of annotations of different types against parts of models." ON)
OPTION(ENABLE_CCGS "Build the CellML Code Generation Service (CCGS), allowing CellML models to be translated into code in a customisable procedural language" ON)
OPTION(ENABLE_CELEDS "Build the CellML Language Export Definition Service (CeLEDS), allowing an XML description of a translation from MathML to a text-based format to be applied to MathML" OFF)
OPTION(ENABLE_CELEDS_EXPORTER "Build the CellML Language Export Definition Exporter Service (CeLEDSExporter), allowing an XML description of a translation from CellML to a text-based format to be applied to CellML to generate procedural code" OFF)
OPTION(ENABLE_CIS "Build the CellML Integration Service, which allows simulations to be run from CellML models" OFF)
OPTION(ENABLE_CUSES "Build the CellML Units Simplification and Expansion Service (CUSES), allowing the canonicalisation and compare units in CellML models" ON)
OPTION(ENABLE_CEVAS "Build the CellML Variable Association Service (CeVAS), allowing information on which variables are connected to be easily obtained" OFF)
OPTION(ENABLE_CONTEXT "Build the CellML Context, a service for maintaining a global list of models that are open" ON)
OPTION(ENABLE_EXAMPLES "Build the example programs in the example subdirectory" ON)
OPTION(ENABLE_GSL_INTEGRATORS "Build GSL integrators in CCGS (note: binaries linked against GSL contain code that is only available under the GNU GPL license)" OFF)
OPTION(ENABLE_MALAES "Build the MathML to Language Expression Service (MaLaES), allowing the translation of a MathML expressions into text in a customised form" ON)
OPTION(ENABLE_MOFS "Build the Model Flattening Service (MoFS) for flattening CellML 1.1 models into CellML 1.0 models" OFF)
OPTION(ENABLE_RDF "Enable the RDF library, which provides access to RDF as triples" OFF)
OPTION(ENABLE_SPROS "Enable SProS, the SED-ML Processing Service, for manipulating and creating SED-ML descriptions of simulation experiments" OFF)
OPTION(ENABLE_SRUS "Enable SRuS, the SED-ML Running Service, for running simulations from SED-ML simulation experiments" OFF)
OPTION(ENABLE_SPROS "Enable SProS, the SED-ML Processing Service, for manipulating and creating SED-ML descriptions of simulation experiments" OFF)
OPTION(ENABLE_TELICEMS "Enable the TeLICeM (Text-based Language for the Input of CellML Models) Service" OFF)
OPTION(ENABLE_VACSS "Build the Validation Against CellML Specification Service (VACSS), allowing CellML models to be validated to detect potential or actual problems with the model" ON)
OPTION(ENABLE_CGRS "Build the CellML Generics and Reflection Service (CGRS), providing access to the CellML API through a generic interface that allows each operation and attribute in the API to be discovered programmatically." OFF)

OPTION(CHECK_BUILD "Run some basic checks on whether the build is likely to succeed before actually building" ON)

# Option Dependencies...
# These must be ordered so modules come before their dependencies.
IF (ENABLE_EVERYTHING)
  SET(ENABLE_CELEDS_EXPORTER ON)
  SET(ENABLE_SRUS ON)
  SET(ENABLE_VACSS ON)
  SET(ENABLE_TELICEMS ON)
  SET(ENABLE_JAVA_BUILD ON)
  SET(ENABLE_RDF ON)
  SET(ENABLE_CGRS ON)
  SET(ENABLE_MOFS ON)
ENDIF()
IF (ENABLE_CELEDS_EXPORTER)
  SET(ENABLE_CELEDS ON)
  SET(ENABLE_CCGS ON)
ENDIF()
IF (ENABLE_CELEDS)
  SET(ENABLE_MALAES ON)
ENDIF()
IF (ENABLE_SRUS)
  SET(ENABLE_SPROS ON)
  SET(ENABLE_CIS ON)
ENDIF()
IF (ENABLE_GSL_INTEGRATORS)
  SET(ENABLE_CIS ON)
ENDIF()
IF (ENABLE_CIS)
  SET(ENABLE_CCGS ON)
ENDIF()
IF (ENABLE_MOFS)
  SET(ENABLE_ANNOTOOLS ON)
  SET(ENABLE_CEVAS ON)
ENDIF()
IF (ENABLE_MALAES)
  SET(ENABLE_CEVAS ON)
ENDIF()
IF (ENABLE_VACSS)
  SET(ENABLE_CUSES ON)
ENDIF()
IF (ENABLE_CUSES)
  SET(ENABLE_ANNOTOOLS ON)
ENDIF()

INCLUDE(FindThreads)

INCLUDE(build/idl.cmake)
INCLUDE(build/bootstraps.cmake)
INCLUDE(build/test_init.cmake)

FIND_PROGRAM(BASH bash)

# Now we do each module in turn (if enabled)
INCLUDE(sources/core.cmake)

IF(ENABLE_ANNOTOOLS)
  INCLUDE(AnnoTools/annotools.cmake)
ENDIF()

IF(ENABLE_CCGS)
  INCLUDE(CCGS/ccgs.cmake)
ENDIF()

IF (ENABLE_CELEDS)
  INCLUDE(CeLEDS/celeds.cmake)
ENDIF()

IF (ENABLE_CELEDS_EXPORTER)
  INCLUDE(CeLEDSExporter/celeds_exporter.cmake)
ENDIF()

IF (ENABLE_CIS)
  INCLUDE(CIS/cis.cmake)
ENDIF()

IF (ENABLE_CUSES)
  INCLUDE(CUSES/cuses.cmake)
ENDIF()

IF (ENABLE_CEVAS)
  INCLUDE(CeVAS/cevas.cmake)
ENDIF()

IF (ENABLE_MOFS)
  INCLUDE(MoFS/mofs.cmake)
ENDIF()

IF (ENABLE_MALAES)
  INCLUDE(MaLaES/malaes.cmake)
ENDIF()

# RDF is included with core when enabled.

IF (ENABLE_SPROS)
  INCLUDE(SProS/spros.cmake)
ENDIF()

IF (ENABLE_SRUS)
  INCLUDE(SRuS/srus.cmake)
ENDIF()

IF (ENABLE_TELICEMS)
  INCLUDE(TeLICeMS/telicems.cmake)
ENDIF()

IF (ENABLE_VACSS)
  INCLUDE(VACSS/vacss.cmake)
ENDIF()

IF (ENABLE_CGRS)
  INCLUDE(CGRS/cgrs-interface.cmake)
ENDIF()

# Run the tests...
INCLUDE(build/test.cmake)

# Do all the language specific bits...

#INCLUDE(build/cxx.cmake)

IF (ENABLE_CGRS)
  INCLUDE(CGRS/cgrs-modules.cmake)
ENDIF()

IF (ENABLE_CORBA)
  INCLUDE(build/corba.cmake)
ENDIF()

IF (ENABLE_XPCOM)
  INCLUDE(build/xpcom.cmake)
ENDIF()

IF (ENABLE_JAVA_BUILD)
  INCLUDE(build/java.cmake)
ENDIF()

IF (ENABLE_EXAMPLES)
  # To do: build and install examples.
ENDIF()

# Finally, compiler setup...
INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCCompilerFlag)

CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_HEADER_INTTYPES_H)
IF (HAVE_HEADER_INTTYPES_H)
   LIST(APPEND CMAKE_EXTRA_INCLUDE_FILES "inttypes.h")
ENDIF()
IF (HAVE_SYS_TYPES_H)
   LIST(APPEND CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
ENDIF()

CHECK_TYPE_SIZE(wchar_t SIZEOF_WCHAR_TP)
SET(HAVE_INTTYPES_H ${HAVE_HEADER_INTTYPES_H})
CHECK_INCLUDE_FILE(JavaVM/jni.h HAVE_JAVAVM_JNI_H)
CHECK_INCLUDE_FILE(jni.h HAVE_JNI_H)
CHECK_TYPE_SIZE(int64_t SIZE_INT64_T)
CHECK_TYPE_SIZE(uint64_t SIZE_UINT64_T)
CHECK_TYPE_SIZE(int32_t SIZE_INT32_T)
CHECK_TYPE_SIZE(uint32_t SIZE_UINT32_T)
CHECK_TYPE_SIZE(int16_t SIZE_INT16_T)
CHECK_TYPE_SIZE(uint16_t SIZE_UINT16_T)
CHECK_TYPE_SIZE(int8_t SIZE_INT16_T)
CHECK_TYPE_SIZE(uint8_t SIZE_UINT16_T)

INCLUDE(CheckCXXSourceCompiles)
IF(NOT WIN32)
  SET(SAFE_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fvisibility=hidden")
  CHECK_CXX_SOURCE_COMPILES("class __attribute__((visibility(\"default\"))) MyClass { MyClass() __attribute__((visibility(\"default\"))); }; int main(int,char**) __attribute__((visibility(\"default\"))); int main(int argc, char** argv) { return 0; }"
                          HAVE_VISIBILITY_ATTRIBUTE)
  SET(CMAKE_REQUIRED_FLAGS "${SAFE_CMAKE_REQUIRED_FLAGS}")
  IF(HAVE_VISIBILITY_ATTRIBUTE)
    ADD_DEFINITIONS("-fvisibility=hidden")
  ENDIF()
ENDIF()

INCLUDE(CheckCSourceRuns)
# The standard CheckCSourceCompiles, except actually run it to pick up linker problems.
MACRO (CHECK_LINKER_FLAG _FLAG _RESULT)
   SET(SAFE_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
   SET(CMAKE_REQUIRED_FLAGS "${_FLAG}")
   CHECK_C_SOURCE_RUNS("int main(void) { return 0; }" ${_RESULT}
     # Some compilers do not fail with a bad flag
     FAIL_REGEX "unknown .*flag"                            # Darwin
     FAIL_REGEX "unrecognized .*option"                     # GNU
     FAIL_REGEX "ignoring unknown option"                   # MSVC
     FAIL_REGEX "warning D9002"                             # MSVC, any lang
     FAIL_REGEX "[Uu]nknown option"                         # HP
     FAIL_REGEX "[Ww]arning: [Oo]ption"                     # SunPro
     FAIL_REGEX "command option .* is not recognized"       # XL
     )
   SET (CMAKE_REQUIRED_FLAGS "${SAFE_CMAKE_REQUIRED_FLAGS}")
ENDMACRO (CHECK_LINKER_FLAG)

CHECK_LINKER_FLAG("-Wl,--no-undefined" HAVE_NO_UNDEFINED)
IF(HAVE_NO_UNDEFINED)
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_EXE_LINKER_FLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
  SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}")
ENDIF()

CHECK_LINKER_FLAG("-Wl,--allow-shlib-undefined" HAVE_ALLOW_SHLIB_UNDEFINED)
IF(HAVE_ALLOW_SHLIB_UNDEFINED)
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--allow-shlib-undefined ${CMAKE_EXE_LINKER_FLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--allow-shlib-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
  SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--allow-shlib-undefined ${CMAKE_MODULE_LINKER_FLAGS}")
ENDIF()

SET(NET_HEADERS)
SET(MATH_HEADERS)
SET(TIME_HEADERS)
SET(STRING_HEADERS)

INCLUDE(CheckIncludeFile)
# Determine whether your compiler supports codecvt.

check_cxx_source_compiles("
  #include <codecvt>
  int main() { 
    std::codecvt_utf8_utf16<wchar_t> x; 
    return 0; 
  }
" HAVE_STD_CODECVT)
IF(HAVE_STD_CODECVT)
add_definitions(-DHAVE_STD_CODECVT)
ENDIF()

CHECK_INCLUDE_FILE(ansidecl.h HAVE_ANSIDECL_H)
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
IF(HAVE_ARPA_INET_H)
LIST(APPEND NET_HEADERS "arpa/inet.h")
ENDIF()
CHECK_INCLUDE_FILE(arpa/nameser.h HAVE_ARPA_NAMESER_H)
IF(HAVE_ARPA_NAMESER_H)
LIST(APPEND NET_HEADERS "arpa/nameser.h")
ENDIF()
CHECK_INCLUDE_FILE(ctype.h HAVE_CTYPE_H)
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(errno.h HAVE_ERRNO_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
IF(HAVE_FLOAT_H)
LIST(APPEND MATH_HEADERS "float.h")
ENDIF()
CHECK_INCLUDE_FILE(fp_class.h HAVE_FP_CLASS_H)
IF(HAVE_FP_CLASS_H)
LIST(APPEND MATH_HEADERS "fp_class.h")
ENDIF()
CHECK_INCLUDE_FILE(ieeefp.h HAVE_IEEEFP_H)
IF(HAVE_IEEEFP_H)
LIST(APPEND MATH_HEADERS "ieeefp.h")
ENDIF()
CHECK_INCLUDE_FILE(inttypes.h.h HAVE_INTTYPES_H_H)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILE(malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)
IF(HAVE_MATH_H)
LIST(APPEND MATH_HEADERS "math.h")
ENDIF()
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE(nan.h HAVE_NAN_H)
IF(HAVE_NAN_H)
LIST(APPEND MATH_HEADERS "nan.h")
ENDIF()
CHECK_INCLUDE_FILE(ndir.h HAVE_NDIR_H)
CHECK_INCLUDE_FILE(netdb.h HAVE_NETDB_H)
IF(HAVE_NETDB_H)
LIST(APPEND NET_HEADERS "netdb.h")
ENDIF()
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_IN_H)
IF(HAVE_NETINET_IN_H)
LIST(APPEND NET_HEADERS "netinet/in.h")
ENDIF()
CHECK_INCLUDE_FILE(poll.h HAVE_POLL_H)
CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILE(resolv.h HAVE_RESOLV_H)
CHECK_INCLUDE_FILE(signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE(stdarg.h HAVE_STDARG_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
IF(HAVE_STRINGS_H)
LIST(APPEND STRING_HEADERS "strings.h")
ENDIF()
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
IF(HAVE_STRING_H)
LIST(APPEND STRING_HEADERS "string.h")
ENDIF()
CHECK_INCLUDE_FILE(sys/dir.h HAVE_SYS_DIR_H)
CHECK_INCLUDE_FILE(sys/mman.h HAVE_SYS_MMAN_H)
CHECK_INCLUDE_FILE(sys/ndir.h HAVE_SYS_NDIR_H)
CHECK_INCLUDE_FILE(sys/select.h HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
IF(HAVE_SYS_SOCKET_H)
LIST(APPEND NET_HEADERS "sys/socket.h")
ENDIF()
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/timeb.h HAVE_SYS_TIMEB_H)
IF(HAVE_SYS_TIMEB_H)
LIST(APPEND TIME_HEADERS "sys/timeb.h")
ENDIF()
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)
IF(HAVE_SYS_TIME_H)
LIST(APPEND TIME_HEADERS "sys/time.h")
ENDIF()
CHECK_INCLUDE_FILE(time.h HAVE_TIME_H)
IF(HAVE_TIME_H)
LIST(APPEND TIME_HEADERS "time.h")
ENDIF()
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(winsock.h HAVE_WINSOCK_H)

INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(AF_INET6 "${NET_HEADERS}" SUPPORT_IP6)
CHECK_SYMBOL_EXISTS(getaddrinfo "${NET_HEADERS}" HAVE_GETADDRINFO)
CHECK_SYMBOL_EXISTS(finite "${MATH_HEADERS}" HAVE_FINITE)
CHECK_SYMBOL_EXISTS(fpclass "${MATH_HEADERS}" HAVE_FPCLASS)
CHECK_SYMBOL_EXISTS(fprintf stdio.h HAVE_FPRINTF)
CHECK_SYMBOL_EXISTS(fp_class "${MATH_HEADERS}" HAVE_FP_CLASS)
CHECK_SYMBOL_EXISTS(ftime "${TIME_HEADERS}" HAVE_FTIME)
CHECK_SYMBOL_EXISTS(gettimeofday "${TIME_HEADERS}" HAVE_GETTIMEOFDAY)
CHECK_SYMBOL_EXISTS(isinf "${MATH_HEADERS}" HAVE_ISINF)
CHECK_SYMBOL_EXISTS(isnan "${MATH_HEADERS}" HAVE_ISNAN)
CHECK_SYMBOL_EXISTS(isnand "${MATH_HEADERS}" HAVE_ISNAND)
CHECK_SYMBOL_EXISTS(localtime "${TIME_HEADERS}" HAVE_LOCALTIME)
CHECK_SYMBOL_EXISTS(printf stdio HAVE_PRINTF)
CHECK_SYMBOL_EXISTS(signal signal.h HAVE_SIGNAL)
CHECK_SYMBOL_EXISTS(snprintf stdio.h HAVE_SNPRINTF)
CHECK_SYMBOL_EXISTS(sprintf stdio.h HAVE_SPRINTF)
CHECK_SYMBOL_EXISTS(sscanf stdio.h HAVE_SSCANF)
CHECK_SYMBOL_EXISTS(stat sys/stat.h HAVE_STAT)
CHECK_SYMBOL_EXISTS(strdup "${STRING_HEADERS}" HAVE_STRDUP)
CHECK_SYMBOL_EXISTS(strerror "${STRING_HEADERS}" HAVE_STRERROR)
CHECK_SYMBOL_EXISTS(strftime "${TIME_HEADERS}" HAVE_STRFTIME)
CHECK_SYMBOL_EXISTS(strndup "${STRING_HEADERS}" HAVE_STRNDUP)
CHECK_SYMBOL_EXISTS(va_copy stdarg.h HAVE_VA_COPY)
CHECK_SYMBOL_EXISTS(vfprintf stdio.h HAVE_VFPRINTF)
CHECK_SYMBOL_EXISTS(vsnprintf stdio.h HAVE_VSNPRINTF)
CHECK_SYMBOL_EXISTS(vsprintf stdio.h HAVE_VSPRINTF)
CHECK_SYMBOL_EXISTS(_stat sys/stat.h HAVE__STAT)
CHECK_SYMBOL_EXISTS(__va_copy stdarg.h HAVE__VA_COPY)
CHECK_SYMBOL_EXISTS(socklen_t "${NET_HEADERS};sys/types.h" HAVE_SOCKLEN_T)

IF(CMAKE_HOST_APPLE)
  ADD_DEFINITIONS("-fno-common")
ENDIF()
IF(MSVC)
  ADD_DEFINITIONS("/wd4290 /wd4091 /wd4250 /wd4355 /wd4101 /wd4996 /wd4800")
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

CONFIGURE_FILE(cda_config.h.in cda_config.h)
CONFIGURE_FILE(extdep/libxml/xml2_config.h.in extdep/libxml/xml2_config.h)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/extdep/libxml/)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/COPYING DESTINATION share/doc/cellml-api/)

# Use one-time-generated hxx headers
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
# export them to the cellml include directory
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/cellml)

install(EXPORT libcellml-config DESTINATION lib/cmake)
include(CMakePackageConfigHelpers)
WRITE_BASIC_PACKAGE_VERSION_FILE(${CMAKE_CURRENT_BINARY_DIR}/libcellml-config-version.cmake COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libcellml-config-version.cmake 
    DESTINATION lib/cmake)
    
# uninstall target
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

INCLUDE(build/python.cmake)
INCLUDE(build/docs.cmake)
