#include "ServiceRegistration.hxx"
#include "CCGSImplementation.hpp"
#include "CCGSBootstrap.hpp"

#include "Utilities.hxx"

// This is a hack to force linking...
#include "SCICCGS.hxx"
#include "corba_support/WrapperRepository.hxx"
#include "CCICCGS.hxx"

iface::cellml_context::CellMLModule* gCodeGenerator;

int
do_registration(void* aContext, void* aModuleManager, void (*UnloadService)())
{
  RETURN_INTO_OBJREF(cgm, iface::cellml_services::CGenerator, CreateCGenerator());
  QUERY_INTERFACE(gCodeGenerator, cgm, cellml_context::CellMLModule);

  reinterpret_cast<iface::cellml_context::CellMLModuleManager*>(aModuleManager)
    ->registerModule(gCodeGenerator);

  // All unload requests are silently ignored until we find a better way to
  // sort out the linking issues on all platforms.
  // gCodeGenerator->SetUnloadCCGS(UnloadService);

  // Ugly hack to force linking...
  SCI::cellml_services::prodCCodeVariable();
  CCI::cellml_services::prodCCodeVariable();
  
  return 0;
}

void
do_deregistration(void* aModuleManager)
{
  reinterpret_cast<iface::cellml_context::CellMLModuleManager*>(aModuleManager)
    ->deregisterModule(gCodeGenerator);
  gCodeGenerator->release_ref();
}
