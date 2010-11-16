#include "Utilities.hxx"
#include "IfaceTeLICeMS.hxx"
#include "TeLICeMService.hpp"
#include "x2pTeLICeMS.hxx"
#include <nsIGenericFactory.h>

#define TELICEMS_BOOTSTRAP_CID                     \
{ /* f67bbeaa-56b8-416a-ba2b-4ed2e7c6c509 */       \
  0xf67bbeaa,                                      \
  0x56b8,                                          \
  0x416a,                                          \
  {0xba, 0x2b, 0x4e, 0xd2, 0xe7, 0xc6, 0xc5, 0x09} \
}

NS_IMETHODIMP
TeLICeMSBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::TeLICeMService* tcs = CreateTeLICeMService();
  nsCOMPtr<cellml_servicesITeLICeMService> xtcs =
    new ::x2p::cellml_services::TeLICeMService(tcs);
  tcs->release_ref();
  return xtcs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "TeLICeMService",
    TELICEMS_BOOTSTRAP_CID,
    "@cellml.org/telicems-bootstrap;1",
    TeLICeMSBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(TeLICeMService, components)
