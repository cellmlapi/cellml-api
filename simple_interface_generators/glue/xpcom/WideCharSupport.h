// Major hack: nsStringAPI tries to assert that it is compiled with
// -fshort-wchar if Mozilla is configured that way, but we are a bridge between
// short-wchar and normal code and deliberately not compiled with that flag. So
// to get away with it, we pretend that prlog.h was already included by defining
// the guard include, and set PR_STATIC_ASSERT to do nothing.
#define prlog_h___
#define PR_STATIC_ASSERT(x)
#include <nsStringAPI.h>
#include "Utilities.hxx"

#ifdef IN_MODULE_XPCOMSupport
#define XPCOMSupport_PUBLIC_PRE CDA_EXPORT_PRE
#define XPCOMSupport_PUBLIC_POST CDA_EXPORT_POST
#else
#define XPCOMSupport_PUBLIC_PRE CDA_EXPORT_PRE
#define XPCOMSupport_PUBLIC_POST CDA_EXPORT_POST
#endif

XPCOMSupport_PUBLIC_PRE void
ConvertWcharToAString
(
 const wchar_t* aSource,
 nsAString& aDest
) XPCOMSupport_PUBLIC_POST;

XPCOMSupport_PUBLIC_PRE void
ConvertAStringToWchar
(
 const nsAString& aSource,
 wchar_t** aDest
) XPCOMSupport_PUBLIC_POST;
