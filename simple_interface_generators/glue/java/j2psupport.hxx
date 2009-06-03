#ifndef J2PSupport_hxx
#define J2PSupport_hxx

#include "cda_compiler_support.h"

#ifdef IN_JAVA_WRAPPER
#define JWRAP_PUBLIC_PRE CDA_EXPORT_PRE
#define JWRAP_PUBLIC_POST CDA_EXPORT_POST
#else
#define JWRAP_PUBLIC_PRE CDA_IMPORT_PRE
#define JWRAP_PUBLIC_POST CDA_IMPORT_POST
#endif

#endif // J2PSupport_hxx
