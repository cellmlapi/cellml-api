#include "Utilities.hxx"
#include "IfaceCeLEDS.hxx"
#include "CeLEDSBootstrap.hpp"
#include "x2pCeLEDS.hxx"
#include <nsIGenericFactory.h>

#define CeLEDS_BOOTSTRAP_CID                         \
{ /* 5921f65f-811b-1471-d929-86ee1cedde65 */         \
    0x5921f65f,                                      \
    0x811b,                                          \
    0x1471,                                          \
    {0xd9, 0x29, 0x86, 0xee, 0x1c, 0xed, 0xde, 0x65} \
}

NS_IMETHODIMP
CeLEDSBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::CeLEDSBootstrap* bs = CreateCeLEDSBootstrap();
  nsCOMPtr<cellml_servicesICeLEDSBootstrap> xbs =
    new ::x2p::cellml_services::CeLEDSBootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CeLEDSBootstrap",
    CeLEDS_BOOTSTRAP_CID,
    "@cellml.org/celeds-bootstrap;1",
    CeLEDSBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CeLEDSBootstrap, components)

