#include "Utilities.hxx"
#include "IfaceCCGS.hxx"
#include "CCGSBootstrap.hpp"
#include "x2pCCGS.hxx"
#include <nsIGenericFactory.h>

#define CCGS_BOOTSTRAP_CID                           \
{ /* 9a30a2f6-9010-4579-9fa8-f70606545e8b */         \
    0x9a30a2f6,                                      \
    0x9010,                                          \
    0x4579,                                          \
    {0x9f, 0xa8, 0xf7, 0x06, 0x06, 0x54, 0x5e, 0x8b} \
}

NS_IMETHODIMP
CCGSBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::CGenerator* bs = CreateCGenerator();
  nsCOMPtr<cellml_servicesICGenerator> xbs =
    new ::x2p::cellml_services::CGenerator(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CCGSBootstrap",
    CCGS_BOOTSTRAP_CID,
    "@cellml.org/ccgs-bootstrap;1",
    CCGSBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CCGSBootstrap, components)
