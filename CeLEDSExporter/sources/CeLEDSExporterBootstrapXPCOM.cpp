#include "Utilities.hxx"
#include "IfaceCeLEDSExporter.hxx"
#include "CeLEDSExporterBootstrap.hpp"
#include "x2pCeLEDSExporter.hxx"
#include <nsIGenericFactory.h>

#define CeLEDSExporter_BOOTSTRAP_CID                         \
{ /* 51b7958a-d36d-4477-bb8d-9ec22ce45a0e */         \
    0x51b7958a,                                      \
    0xd36d,                                          \
    0x4477,                                          \
    {0xbb, 0x8d, 0x9e, 0xc2, 0x2c, 0xe4, 0x5a, 0x0e} \
}

NS_IMETHODIMP
CeLEDSExporterBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_services::CeLEDSExporterBootstrap* bs = CreateCeLEDSExporterBootstrap();
  nsCOMPtr<cellml_servicesICeLEDSExporterBootstrap> xbs =
    new ::x2p::cellml_services::CeLEDSExporterBootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CeLEDSExporterBootstrap",
    CeLEDSExporter_BOOTSTRAP_CID,
    "@cellml.org/celedsexporter-bootstrap;1",
    CeLEDSExporterBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CeLEDSExporterBootstrap, components)

