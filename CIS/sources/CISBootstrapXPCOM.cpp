#include "Utilities.hxx"
#include "IfaceCIS.hxx"
#include "CISBootstrap.hpp"
#include "x2pCIS.hxx"
#include <nsIGenericFactory.h>

#define CIS_BOOTSTRAP_CID                            \
{ /* d6c35cc3-a610-475a-b70a-bfef7102813d */         \
    0xd6c35cc3,                                      \
    0xa610,                                          \
    0x475a,                                          \
    {0xb7, 0x0a, 0xbf, 0xef, 0x71, 0x02, 0x81, 0x3d} \
}

NS_IMETHODIMP
CISBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::CellMLIntegrationService* bs =
    CreateIntegrationService();
  nsCOMPtr<cellml_servicesICellMLIntegrationService> xbs =
    new ::x2p::cellml_services::CellMLIntegrationService(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CISBootstrap",
    CIS_BOOTSTRAP_CID,
    "@cellml.org/cis-bootstrap;1",
    CISBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CISBootstrap, components)
