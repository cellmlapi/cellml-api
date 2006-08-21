#include "cda_compiler_support.h"
#ifdef IN_CIS_MODULE
#define CIS_PUBLIC CDA_EXPORT
#else
#define CIS_PUBLIC CDA_IMPORT
#endif

/**
 * Creates a new CIS integrator.
 * This code is the external C++ interface which allows you to fetch the
 * implementation.
 */
iface::cellml_services::CellMLIntegrationService*
CreateIntegrationService(void) CIS_PUBLIC;
