#include "cda_compiler_support.h"
#include <inttypes.h>
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

