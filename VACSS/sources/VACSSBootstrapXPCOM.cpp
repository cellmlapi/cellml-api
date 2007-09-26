#include "Utilities.hxx"
#include "IfaceVACSS.hxx"
#include "VACSSBootstrap.hpp"
#include "x2pVACSS.hxx"
#include <nsIGenericFactory.h>

#define VACSS_BOOTSTRAP_CID                          \
{ /* b600e7a3-a302-4244-98fb-ee5c7bd45592 */         \
    0xb600e7a3,                                      \
    0xa302,                                          \
    0x4244,                                          \
    {0x98, 0xfb, 0xee, 0x5c, 0x7b, 0xd4, 0x55, 0x92} \
}

NS_IMETHODIMP
VACSSBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::VACSService* vs = CreateVACSService();
  nsCOMPtr<cellml_servicesIVACSService> xvs =
    new ::x2p::cellml_services::VACSService(vs);
  vs->release_ref();
  return xvs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "VACSService",
    VACSS_BOOTSTRAP_CID,
    "@cellml.org/vacs-service;1",
    VACSSBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(VACSSBootstrap, components)
