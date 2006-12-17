#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_Context.hxx"

extern "C"
{
  // These must be provided by the service...
  int do_registration(void* aContext,
                      void* aModuleManager,
                      void (*UnloadService)()) CDA_EXPORT;
  void do_deregistration(void* aModuleManager) CDA_EXPORT;
};

