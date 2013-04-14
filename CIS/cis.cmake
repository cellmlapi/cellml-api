DECLARE_EXTENSION(cis)
DECLARE_IDL(CIS)
DECLARE_IDL_DEPENDENCY(CCGS)
DECLARE_EXTENSION_END(cis)

INCLUDE_DIRECTORIES(CIS/sources)

# Find clang and LLVM...
IF(LLVM_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LLVM_FIND_QUIETLY TRUE)
ENDIF(LLVM_INCLUDE_DIR)

FIND_PATH(LLVM_INCLUDE_DIR llvm/Constants.h
  /usr/local/include
  /usr/include
)
FIND_PATH(CLANG_INCLUDE_DIR clang/Basic/SourceLocation.h
  /usr/local/include
  /usr/include
)

SET(LLVM_LIBPATH /usr/local/lib /usr/lib)
MARK_AS_ADVANCED(LLVM_LIBPATH)

FIND_LIBRARY(LLVM_CORE_LIBRARY
  NAMES LLVMCore
  PATHS ${LLVM_LIBPATH}
)
FIND_LIBRARY(CLANG_BASIC_LIBRARY
  NAMES clangBasic
  PATHS ${LLVM_LIBPATH}
)

IF (LLVM_INCLUDE_DIR AND LLVM_CORE_LIBRARY AND CLANG_INCLUDE_DIR AND CLANG_BASIC_LIBRARY)
   SET(LLVM_FOUND TRUE)

   INCLUDE_DIRECTORIES(${LLVM_INCLUDE_DIR} ${CLANG_INCLUDE_DIR})

   IF (NOT LLVM_FIND_QUIETLY)
      MESSAGE(STATUS "Found LLVM: ${LLVM_CORE_LIBRARY}")
   ENDIF (NOT LLVM_FIND_QUIETLY)

   SET(LLVM_LIBRARIES)

   SET(LLVM_PLAT_LIBS)
   FOREACH(plat ARM CellSPU Hexagon MBlaze Mips MSP430 NVPTX PowerPC Sparc X86 XCore)
     LIST(APPEND LLVM_PLAT_LIBS LLVM${plat}Desc LLVM${plat}CodeGen LLVM${plat}Info LLVM${plat}AsmPrinter LLVM${plat}Utils)
   ENDFOREACH()

   FOREACH(extraLLVM clangARCMigrate clangASTMatchers clangCodeGen clangDriver clangFrontend clangFrontendTool clangLex clangParse clangRewriteCore clangRewriteFrontend clangSerialization clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangStaticAnalyzerFrontend clangTooling clangSema clangEdit clangAnalysis clangAST clangBasic ${LLVM_PLAT_LIBS} LLVMExecutionEngine LLVMArchive LLVMAsmPrinter LLVMBitReader LLVMBitWriter LLVMCodeGen LLVMCppBackendCodeGen LLVMCppBackendInfo LLVMDebugInfo LLVMInstCombine LLVMInstrumentation LLVMInterpreter LLVMJIT LLVMLinker LLVMMBlazeAsmParser LLVMMBlazeAsmPrinter LLVMMBlazeCodeGen LLVMMBlazeDesc LLVMMBlazeDisassembler LLVMMBlazeInfo LLVMMCDisassembler LLVMMCJIT LLVMObject LLVMRuntimeDyld LLVMScalarOpts LLVMSelectionDAG LLVMTableGen LLVMTarget LLVMTransformUtils LLVMVectorize LLVMXCoreCodeGen LLVMXCoreDesc LLVMXCoreInfo LLVMipa LLVMipo LLVMMC LLVMAnalysis LLVMMCParser LLVMAsmParser LLVMSupport)
     FIND_LIBRARY(LLVM_${extraLLVM}_LIBRARY
       NAMES ${extraLLVM}
       PATHS ${LLVM_LIBPATH}
       )
     IF (${LLVM_${extraLLVM}_LIBRARY} STREQUAL "LLVM_${extraLLVM}_LIBRARY-NOTFOUND")
       SET(${LLVM_${extraLLVM}_LIBRARY})
     ELSE()
       LIST(APPEND LLVM_LIBRARIES ${LLVM_${extraLLVM}_LIBRARY})
     ENDIF()
   ENDFOREACH()
   SET(LLVM_LIBRARIES ${LLVM_LIBRARIES} ${LLVM_CORE_LIBRARY})
ELSE(LLVM_INCLUDE_DIR AND LLVM_CORE_LIBRARY AND CLANG_INCLUDE_DIR AND CLANG_BASIC_LIBRARY)
   SET(LLVM_FOUND FALSE)
   SET(LLVM_LIBRARIES )
ENDIF(LLVM_INCLUDE_DIR AND LLVM_CORE_LIBRARY AND CLANG_INCLUDE_DIR AND CLANG_BASIC_LIBRARY)

MARK_AS_ADVANCED(
  CLANG_INCLUDE_DIR
  LLVM_INCLUDE_DIR
  )

OPTION(USE_SYSTEM_SUNDIALS "Use the SUNDIALS libraries found on the system, rather than the sources distributed with the API.")
MARK_AS_ADVANCED(USE_SYSTEM_SUNDIALS)

IF (USE_SYSTEM_SUNDIALS)
  SET(SUNDIALS_SOURCES)
  SET(SYSTEM_SUNDIALS sundials_cvode sundials_ida sundials_kinsol sundials_nvecserial)
  FOREACH(CURLIB ${SUNDIALS_SOURCES})
    SET(CURLIBFOUND)
    FIND_LIBRARY(CURLIBFOUND ${CURLIB})
    IF(${CURLIBFOUND} STREQUAL "CURLIB-NOTFOUND")
      MESSAGE(SEND_ERROR "USE_SYSTEM_SUNDIALS is enabled but can't find ${CURLIB}")
    ENDIF()
  ENDFOREACH()
ELSE()
  SET(SYSTEM_SUNDIALS)
  SET(SUNDIALS_SOURCES
    CIS/sources/cvode/cvode_band.c
    CIS/sources/cvode/cvode_bandpre.c
    CIS/sources/cvode/cvode_bbdpre.c
    CIS/sources/cvode/cvode.c
    CIS/sources/cvode/cvode_dense.c
    CIS/sources/cvode/cvode_direct.c
    CIS/sources/cvode/cvode_diag.c
    CIS/sources/cvode/cvode_io.c
    CIS/sources/cvode/cvode_spbcgs.c
    CIS/sources/cvode/cvode_spgmr.c
    CIS/sources/cvode/cvode_spils.c
    CIS/sources/cvode/cvode_sptfqmr.c
    CIS/sources/nvec_ser/nvector_serial.c
    CIS/sources/sundials/sundials_band.c
    CIS/sources/sundials/sundials_dense.c
    CIS/sources/sundials/sundials_direct.c
    CIS/sources/sundials/sundials_iterative.c
    CIS/sources/sundials/sundials_math.c
    CIS/sources/sundials/sundials_nvector.c
    CIS/sources/sundials/sundials_spbcgs.c
    CIS/sources/sundials/sundials_spgmr.c
    CIS/sources/sundials/sundials_sptfqmr.c
    CIS/sources/ida/ida.c
    CIS/sources/ida/ida_dense.c
    CIS/sources/ida/ida_ic.c
    CIS/sources/ida/ida_bbdpre.c
    CIS/sources/ida/ida_direct.c
    CIS/sources/ida/ida_spbcgs.c
    CIS/sources/ida/ida_sptfqmr.c
    CIS/sources/ida/ida_io.c
    CIS/sources/ida/ida_band.c
    CIS/sources/ida/ida_spils.c
    CIS/sources/ida/ida_spgmr.c
    CIS/sources/kinsol/kinsol.c
    CIS/sources/kinsol/kinsol_io.c
    CIS/sources/kinsol/kinsol_spils.c
    CIS/sources/kinsol/kinsol_spgmr.c
  )
ENDIF()

ADD_LIBRARY(cis
  CIS/sources/CISImplementation.cxx
  CIS/sources/CISSolve.cxx
  ${SUNDIALS_SOURCES}
  )
INSTALL(TARGETS cis DESTINATION lib)

IF(ENABLE_GSL_INTEGRATORS)
  SET(MAYBEGSL gsl gslcblas)
ELSE()
  SET(MAYBEGSL)
ENDIF()

FIND_LIBRARY(PTHREAD pthread)
FIND_LIBRARY(PTHREADS pthreads)
SET(THREADLIBRARY)
IF(NOT PTHREAD STREQUAL "PTHREAD-NOTFOUND")
  LIST(APPEND THREADLIBRARY pthread)
ENDIF()
IF(NOT PTHREADS STREQUAL "PTHREADS-NOTFOUND")
  LIST(APPEND THREADLIBRARY pthreads)
ENDIF()

TARGET_LINK_LIBRARIES(cis ccgs malaes cuses cevas cellml ${MAYBEGSL} ${THREADLIBRARY} ${CMAKE_DL_LIBS} ${SYSTEM_SUNDIALS} ${CLANG_LIBRARIES} ${LLVM_LIBRARIES})
SET_TARGET_PROPERTIES(cis PROPERTIES VERSION ${GLOBAL_VERSION} SOVERSION ${CIS_SOVERSION})

DECLARE_BOOTSTRAP("CISBootstrap" "CIS" "CellMLIntegrationService" "cellml_services" "createIntegrationService" "CreateIntegrationService" "CISBootstrap.hpp" "CIS/sources" "cis")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(RunCellML CIS/tests/RunCellML.cpp)
  TARGET_LINK_LIBRARIES(RunCellML cellml ccgs cuses cevas malaes annotools cis)
  ADD_TEST(CheckCIS ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/tests/RetryWrapper ${CMAKE_CURRENT_SOURCE_DIR}/tests/CheckCIS)
  DECLARE_TEST_LIB(cis)
ENDIF()

IF(ENABLE_GSL_INTEGRATORS)
  CHECK_LIBRARY_EXISTS(gsl gsl_permutation_alloc "" HAVE_LIBGSL)
  IF (CHECK_BUILD AND NOT HAVE_LIBGSL)
    MESSAGE(FATAL_ERROR "GSL libraries were not found. To override the pre-build checks and manually fix any problems, pass -DCHECK_BUILD:BOOL=OFF to CMake.")
  ENDIF()
  CHECK_LIBRARY_EXISTS(gslcblas cblas_dtrmm "" HAVE_LIBGSLCBLAS)
  IF (CHECK_BUILD AND NOT HAVE_LIBGSLCBLAS)
    MESSAGE(FATAL_ERROR "GSLCBLAS libraries were not found. To override the pre-build checks and manually fix any problems, pass -DCHECK_BUILD:BOOL=OFF to CMake.")
  ENDIF()
ENDIF(ENABLE_GSL_INTEGRATORS)
