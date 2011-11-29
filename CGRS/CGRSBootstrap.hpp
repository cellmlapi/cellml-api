#include "cda_compiler_support.h"
#ifdef IN_CGRS_MODULE
#define CGRS_PUBLIC_PRE CDA_EXPORT_PRE
#define CGRS_PUBLIC_POST CDA_EXPORT_POST
#else
#define CGRS_PUBLIC_PRE CDA_IMPORT_PRE
#define CGRS_PUBLIC_POST CDA_IMPORT_POST
#endif

/**
 * Creates a new CGRS service.
 * This code is the external C++ interface which allows you to fetch the
 * implementation.
 */
CGRS_PUBLIC_PRE
  already_AddRefd<iface::CGRS::GenericsService>
  CreateGenericsService(void)
CGRS_PUBLIC_POST;
