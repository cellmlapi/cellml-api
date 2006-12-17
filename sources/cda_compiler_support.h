#ifndef compiler_support_h
#define compiler_support_h

#include "cda_config.h"

#ifdef __BORLANDC__
#define WIN32
#define HEADER_INLINE
#else
#define HEADER_INLINE inline
#endif

#ifdef HAVE_VISIBILITY_ATTRIBUTE
#define CDA_EXPORT(x) x __attribute__((visibility("default")))
#define WARN_IF_RETURN_UNUSED __attribute__((warn_unused_result))
#define CDA_IMPORT(x) x __attribute__((visibility("default")))
#elif defined(WIN32)
#define CDA_EXPORT(x) __declspec(dllexport) x
#define CDA_IMPORT(x) __declspec(dllimport) x
#define WARN_IF_RETURN_UNUSED
#else
#define CDA_EXPORT(x)
#define CDA_IMPORT(x)
#define WARN_IF_RETURN_UNUSED
#endif

#endif // compiler_support_h
