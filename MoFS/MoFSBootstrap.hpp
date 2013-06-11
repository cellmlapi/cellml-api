#include "cda_compiler_support.h"
#include "IfaceMoFS.hxx"
#ifdef IN_MOFS_MODULE
#define MOFS_PUBLIC_PRE CDA_EXPORT_PRE
#define MOFS_PUBLIC_POST CDA_EXPORT_POST
#else
#define MOFS_PUBLIC_PRE CDA_IMPORT_PRE
#define MOFS_PUBLIC_POST CDA_IMPORT_POST
#endif

/**
 * Creates a new ModelFlatteningService.
 * This code is the external C++ interface which allows you to fetch the
 * implementation.
 */
MOFS_PUBLIC_PRE already_AddRefd<iface::mofs::ModelFlatteningService>
CreateModelFlatteningService(void) MOFS_PUBLIC_POST;
