#ifndef compiler_support_h
#define compiler_support_h

#include "cda_config.h"
#ifdef HAVE_VISIBILITY_ATTRIBUTE
#define CDA_EXPORT __attribute__((visibility("default")))
#define CDA_IMPORT __attribute__((visibility("default")))
#elif defined(WIN32)
#define CDA_EXPORT __declspec(dllexport)
#define CDA_IMPORT __declspec(dllimport)
#else
#define CDA_EXPORT
#define CDA_IMPORT
#endif

#endif // compiler_support_h
