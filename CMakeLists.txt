CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_BACKWARDS_COMPATIBILITY 2.8)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

INCLUDE(CTest)

OPTION(ENABLE_EVERYTHING "Turn on all optional features, except the commonly problematic ones (XPCOM, CORBA, and GSL integrators).")

OPTION(ENABLE_CORBA "Generate a two-way bridge between the C++ implementation and CORBA, allowing all parts of the API to be accessed from CORBA. Note: this functionality is not widely tested; if you find a bug, we are only interested if you also provide a working patch to fix the problem")
MARK_AS_ADVANCED(FORCE ENABLE_CORBA)
OPTION(ENABLE_XPCOM "Generate XPIDL interfaces and a two-way bridge between the C++ implementation and XPCOM, allowing all parts of the API to be accessed from XPCOM. Note: this functionality is not widely tested; if you find a bug, we are only interested if you also provide a working patch to fix the problem")
OPTION(ENABLE_JAVA_BUILD "Build Java interfaces and a bridge between the C++ implementation and Java, allowing all parts of the API to be accessed from Java.")
OPTION(ENABLE_PYTHON "Build Python interfaces and a bridge between the C++ implementation and Python, allowing all parts of the API to be accessed from Python.")
MARK_AS_ADVANCED(FORCE ENABLE_XPCOM)
SET(XULRUNNER_SDK_PATH "/usr/local/xulrunner-sdk" CACHE PATH "This property is only used when XPCOM support has been enabled. It determines the path to the xulrunner SDK used to build the XPCOM bridge")
MARK_AS_ADVANCED(FORCE XULRUNNER_SDK_PATH)

OPTION(ENABLE_ANNOTOOLS "Build AnnoTools, a service for handling sets of annotations of different types against parts of models.")
OPTION(ENABLE_CCGS "Build the CellML Code Generation Service (CCGS), allowing CellML models to be translated into code in a customisable procedural language")
OPTION(ENABLE_CELEDS "Build the CellML Language Export Definition Service (CeLEDS), allowing an XML description of a translation from MathML to a text-based format to be applied to MathML")
OPTION(ENABLE_CELEDS_EXPORTER "Build the CellML Language Export Definition Exporter Service (CeLEDSExporter), allowing an XML description of a translation from CellML to a text-based format to be applied to CellML to generate procedural code")
OPTION(ENABLE_CIS "Build the CellML Integration Service, which allows simulations to be run from CellML models")
OPTION(ENABLE_CUSES "Build the CellML Units Simplification and Expansion Service (CUSES), allowing the canonicalisation and compare units in CellML models")
OPTION(ENABLE_CEVAS "Build the CellML Variable Association Service (CeVAS), allowing information on which variables are connected to be easily obtained")
OPTION(ENABLE_CONTEXT "Build the CellML Context, a service for maintaining a global list of models that are open")
OPTION(ENABLE_EXAMPLES "Build the example programs in the example subdirectory")
OPTION(ENABLE_GSL_INTEGRATORS "Build GSL integrators in CCGS (note: binaries linked against GSL contain code that is only available under the GNU GPL license)")
OPTION(ENABLE_MALAES "Build the MathML to Language Expression Service (MaLaES), allowing the translation of a MathML expressions into text in a customised form")
OPTION(ENABLE_RDF "Enable the RDF library, which provides access to RDF as triples")
OPTION(ENABLE_SPROS "Enable SProS, the SED-ML Processing Service, for manipulating and creating SED-ML descriptions of simulation experiments")
OPTION(ENABLE_SRUS "Enable SRuS, the SED-ML Running Service, for running simulations from SED-ML simulation experiments")
OPTION(ENABLE_SPROS "Enable SProS, the SED-ML Processing Service, for manipulating and creating SED-ML descriptions of simulation experiments")
OPTION(ENABLE_TELICEMS "Enable the TeLICeM (Text-based Language for the Input of CellML Models) Service")
OPTION(ENABLE_VACSS "Build the Validation Against CellML Specification Service (VACSS), allowing CellML models to be validated to detect potential or actual problems with the model")

OPTION(CHECK_BUILD "Run some basic checks on whether the build is likely to succeed before actually building" ON)
OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Option Dependencies...
# These must be ordered so modules come before their dependencies.
IF (ENABLE_EVERYTHING)
  SET(ENABLE_CELEDS_EXPORTER ON)
  SET(ENABLE_SRUS ON)
  SET(ENABLE_VACSS ON)
  SET(ENABLE_TELICEMS ON)
  SET(ENABLE_JAVA_BUILD ON)
  SET(ENABLE_PYTHON ON)
  SET(ENABLE_RDF ON)
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

# Context is included with core when enabled.

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

# Run the tests...
FIND_LIBRARY(CPPUNIT cppunit)
IF(CPPUNIT STREQUAL "CPPUNIT-NOTFOUND")
  SET(HAS_CPPUNIT OFF)
ELSE()
  SET(HAS_CPPUNIT ON)
ENDIF()
INCLUDE(build/test.cmake)

# Do all the language specific bits...

INCLUDE(build/cxx.cmake)

IF (ENABLE_CORBA)
  INCLUDE(build/corba.cmake)
ENDIF()

IF (ENABLE_XPCOM)
  INCLUDE(build/xpcom.cmake)
ENDIF()

IF (ENABLE_JAVA_BUILD)
  INCLUDE(build/java.cmake)
ENDIF()

IF (ENABLE_PYTHON)
  INCLUDE(build/python.cmake)
ENDIF()

IF (ENABLE_EXAMPLES)
  # To do: build and install examples.
ENDIF()

# Finally, compiler setup...
INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCCompilerFlag)
CHECK_TYPE_SIZE(wchar_t* SIZEOF_WCHAR_TP)
CHECK_INCLUDE_FILE(inttypes.h HAVE_HEADER_INTTYPES_H)
CHECK_INCLUDE_FILE(JavaVM/jni.h HAVE_JAVAVM_JNI_H)
CHECK_INCLUDE_FILE(jni.h HAVE_JNI_H)
LIST(APPEND CMAKE_EXTRA_INCLUDE_FILES "inttypes.h")
CHECK_TYPE_SIZE(int64_t SIZE_INT64_T)
CHECK_TYPE_SIZE(uint64_t SIZE_UINT64_T)
CHECK_TYPE_SIZE(int32_t SIZE_INT32_T)
CHECK_TYPE_SIZE(uint32_t SIZE_UINT32_T)
CHECK_TYPE_SIZE(int16_t SIZE_INT16_T)
CHECK_TYPE_SIZE(uint16_t SIZE_UINT16_T)
CHECK_TYPE_SIZE(int8_t SIZE_INT16_T)
CHECK_TYPE_SIZE(uint8_t SIZE_UINT16_T)
CHECK_C_COMPILER_FLAG("-fvisibility=hidden" HAVE_VISIBILITY_ATTRIBUTE)
IF(HAVE_NO_UNDEFINED)
  ADD_DEFINITIONS("-fvisibility=hidden")
ENDIF()

INCLUDE(CheckCSourceRuns)
# The standard CheckCSourceCompiles, except actually run it to pick up linker problems.
MACRO (CHECK_LINKER_FLAG _FLAG _RESULT)
   SET(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
   SET(CMAKE_REQUIRED_DEFINITIONS "${_FLAG}")
   CHECK_C_SOURCE_RUNS("int main(void) { return 0; }" ${_RESULT}
     # Some compilers do not fail with a bad flag
     FAIL_REGEX "unrecognized .*option"                     # GNU
     FAIL_REGEX "ignoring unknown option"                   # MSVC
     FAIL_REGEX "warning D9002"                             # MSVC, any lang
     FAIL_REGEX "[Uu]nknown option"                         # HP
     FAIL_REGEX "[Ww]arning: [Oo]ption"                     # SunPro
     FAIL_REGEX "command option .* is not recognized"       # XL
     )
   SET (CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
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

IF(MSVC)
  ADD_DEFINITIONS("/wd4290 /wd4091 /wd4250")
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

CONFIGURE_FILE(cda_config.h.in cda_config.h)
