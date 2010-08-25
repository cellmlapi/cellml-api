#include "Utilities.hxx"
#include "IfaceCUSES.hxx"
#include "CUSESBootstrap.hpp"
#include "x2pCUSES.hxx"
#include <nsIGenericFactory.h>

#define CUSES_BOOTSTRAP_CID                          \
{ /* 2cf2e47b-b492-4386-9a6a-973d1a962a63 */         \
    0x2cf2e47b,                                      \
    0xb492,                                          \
    0x4386,                                          \
    {0x9a, 0x6a, 0x97, 0x3d, 0x1a, 0x96, 0x2a, 0x63} \
}

NS_IMETHODIMP
CUSESBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::CUSESBootstrap* bs = CreateCUSESBootstrap();
  nsCOMPtr<cellml_servicesICUSESBootstrap> xbs =
    new ::x2p::cellml_services::CUSESBootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CUSESBootstrap",
    CUSES_BOOTSTRAP_CID,
    "@cellml.org/cuses-bootstrap;1",
    CUSESBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CUSESBootstrap, components)
