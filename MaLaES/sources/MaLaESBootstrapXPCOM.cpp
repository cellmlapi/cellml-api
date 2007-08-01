#include "Utilities.hxx"
#include "IfaceCCGS.hxx"
#include "MaLaESBootstrap.hpp"
#include "x2pMaLaES.hxx"
#include <nsIGenericFactory.h>

#define MALAES_BOOTSTRAP_CID                         \
{ /* 7886c601-8d93-4552-9f46-ef77d396c2c3 */         \
    0x7886c601,                                      \
    0x8d93,                                          \
    0x4552,                                          \
    {0x9f, 0x46, 0xef, 0x77, 0xd3, 0x96, 0xc2, 0xc3} \
}

NS_IMETHODIMP
MaLaESBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::MaLaESBootstrap* bs = CreateMaLaESBootstrap();
  nsCOMPtr<cellml_servicesIMaLaESBootstrap> xbs =
    new ::x2p::cellml_services::MaLaESBootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "MaLaESBootstrap",
    MALAES_BOOTSTRAP_CID,
    "@cellml.org/malaes-bootstrap;1",
    MaLaESBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(MaLaESBootstrap, components)
