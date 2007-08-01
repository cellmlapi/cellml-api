#include "Utilities.hxx"
#include "IfaceCeVAS.hxx"
#include "CeVASBootstrap.hpp"
#include "x2pCeVAS.hxx"
#include <nsIGenericFactory.h>

#define CEVAS_BOOTSTRAP_CID                          \
{ /* 91ffb3cd-839a-4abc-90ef-4c90d45de464 */         \
    0x91ffb3cd,                                      \
    0x839a,                                          \
    0x4abc,                                          \
    {0x90, 0xef, 0x4c, 0x90, 0xd4, 0x5d, 0xe4, 0x64} \
}

NS_IMETHODIMP
CeVASBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::CeVASBootstrap* bs = CreateCeVASBootstrap();
  nsCOMPtr<cellml_servicesICeVASBootstrap> xbs =
    new ::x2p::cellml_services::CeVASBootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CeVASBootstrap",
    CEVAS_BOOTSTRAP_CID,
    "@cellml.org/cevas-bootstrap;1",
    CeVASBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CeVASBootstrap, components)
