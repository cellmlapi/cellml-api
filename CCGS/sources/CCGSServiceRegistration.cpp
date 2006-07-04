#include "ServiceRegistration.hxx"
#include "CCGSImplementation.hpp"

// This is a hack to force linking...
#include "SCICCGS.hxx"
#include "corba_support/WrapperRepository.hxx"
#include "CCICCGS.hxx"

CDA_CGenerator* gCodeGenerator;

int
do_registration(iface::cellml_context::CellMLContext* aContext,
                iface::cellml_context::CellMLModuleManager* aModuleManager)
{
  gCodeGenerator = new CDA_CGenerator();
  gCodeGenerator->SetUnloadCCGS(UnloadService);
  aModuleManager->registerModule(gCodeGenerator);

  // Ugly hack to force linking...
  SCI::cellml_services::prodCCodeVariable();
  CCI::cellml_services::prodCCodeVariable();
  
  return 0;
}

void
do_deregistration(iface::cellml_context::CellMLModuleManager* aModuleManager)
{
  aModuleManager->deregisterModule(gCodeGenerator);
  gCodeGenerator->release_ref();
}
