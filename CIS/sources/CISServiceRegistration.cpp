#include "ServiceRegistration.hxx"
#include "Utilities.hxx"
#include "CISImplementation.hxx"

CDA_CellMLIntegrationService* gIntegrationService;

int
do_registration(void* aContext, void* aModuleManager, void (*UnloadService)())
{
  gIntegrationService = new CDA_CellMLIntegrationService();
  gIntegrationService->SetUnloadCIS(UnloadService);
  reinterpret_cast<iface::cellml_context::CellMLModuleManager*>(aModuleManager)
    ->registerModule(gIntegrationService);
}

void
do_deregistration(void* aModuleManager)
{
  reinterpret_cast<iface::cellml_context::CellMLModuleManager*>(aModuleManager)
    ->deregisterModule(gIntegrationService);
  gIntegrationService->release_ref();
}
