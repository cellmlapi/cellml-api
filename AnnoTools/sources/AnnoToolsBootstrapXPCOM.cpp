#include "Utilities.hxx"
#include "IfaceCCGS.hxx"
#include "AnnoToolsBootstrap.hpp"
#include "x2pAnnoTools.hxx"
#include <nsIGenericFactory.h>

#define ANNOTOOLS_BOOTSTRAP_CID                      \
{ /* 77794d51-59dc-40b2-aaa0-ceebb78c5fca */         \
    0x77794d51,                                      \
    0x59dc,                                          \
    0x40b2,                                          \
    {0xaa, 0xa0, 0xce, 0xeb, 0xb7, 0x8c, 0x5f, 0xca} \
}

NS_IMETHODIMP
AnnoToolsBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::AnnotationToolService* bs = CreateAnnotationToolService();
  nsCOMPtr<cellml_servicesIAnnotationToolService> xbs =
    new ::x2p::cellml_services::AnnotationToolService(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "AnnoToolsBootstrap",
    ANNOTOOLS_BOOTSTRAP_CID,
    "@cellml.org/annotation-service;1",
    AnnoToolsBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(AnnoToolsBootstrap, components)
