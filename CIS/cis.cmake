DECLARE_EXTENSION(cis)
DECLARE_IDL(CIS)
DECLARE_IDL_DEPENDENCY(CCGS)
DECLARE_EXTENSION_END(cis)

INCLUDE_DIRECTORIES(CIS/sources)

ADD_LIBRARY(cis
  CIS/sources/CISImplementation.cxx
  CIS/sources/CISSolve.cxx
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
  CIS/sources/ida/ida_spgmr
  CIS/sources/levmar/Axb.c
  CIS/sources/levmar/lm.c
  CIS/sources/levmar/misc.c
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

TARGET_LINK_LIBRARIES(cis ccgs malaes cuses cevas cellml ${MAYBEGSL} ${THREADLIBRARY} ${CMAKE_DL_LIBS})

DECLARE_BOOTSTRAP("CISBootstrap" "CIS" "CellMLIntegrationService" "cellml_services" "createIntegrationService" "CreateIntegrationService" "CISBootstrap.hpp" "CIS/sources" "cis")

IF (BUILD_TESTING)
  ADD_EXECUTABLE(RunCellML CIS/tests/RunCellML.cpp)
  TARGET_LINK_LIBRARIES(RunCellML cellml ccgs xml2 cuses cevas malaes annotools cis)
  ADD_TEST(CheckCIS tests/CheckCIS)
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
