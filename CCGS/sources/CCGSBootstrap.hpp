#include "cda_compiler_support.h"
#ifdef IN_CCGS_MODULE
#define CCGS_PUBLIC CDA_EXPORT
#else
#define CCGS_PUBLIC CDA_IMPORT
#endif

/**
 * Creates a new C generator.
 * This code is the external C++ interface which allows you to fetch the
 * implementation.
 */
iface::cellml_services::CGenerator* CreateCGenerator(void) CCGS_PUBLIC;
