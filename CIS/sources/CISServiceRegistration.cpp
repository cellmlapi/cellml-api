#include "ServiceRegistration.hxx"
#include "Utilities.hxx"
#include "CISImplementation.hxx"

CDA_CellMLIntegrationService* gIntegrationService;

int
do_registration(iface::cellml_context::CellMLContext* aContext,
                iface::cellml_context::CellMLModuleManager* aModuleManager)
{
  gIntegrationService = new CDA_CellMLIntegrationService();
  gIntegrationService->SetUnloadCIS(UnloadService);
  aModuleManager->registerModule(gIntegrationService);
}

void
do_deregistration(iface::cellml_context::CellMLModuleManager* aModuleManager)
{
  aModuleManager->deregisterModule(gIntegrationService);
  gIntegrationService->release_ref();
}
