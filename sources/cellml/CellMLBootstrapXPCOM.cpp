#include "Utilities.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include "x2pCellML_APISPEC.hxx"

#if ENABLE_RDF
#include "IfaceRDF_APISPEC.hxx"
#include "RDFBootstrap.hpp"
#include "x2pRDF_APISPEC.hxx"
#endif

#include <nsIGenericFactory.h>

#define CELLML_BOOTSTRAP_CID                         \
{ /* 56849f69-c321-4be6-9191-1bb2ee0a76e0 */         \
    0x56849f69,                                      \
    0xc321,                                          \
    0x4be6,                                          \
    {0x91, 0x91, 0x1b, 0xb2, 0xee, 0x0a, 0x75, 0xe0} \
}

#if ENABLE_RDF
#define RDF_BOOTSTRAP_CID                            \
{ /* f4edbb5d-949e-4cdf-919e-0eece06f2a68 */         \
    0xf4edbb5d,                                      \
    0x949e,                                          \
    0x4cdf,                                          \
    {0x91, 0x9e, 0x0e, 0xec, 0xe0, 0x6f, 0x2a, 0x68} \
}
#endif

NS_IMETHODIMP
CellMLBootstrapConstructor(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_api::CellMLBootstrap* bs = CreateCellMLBootstrap();
  nsCOMPtr<cellml_apiICellMLBootstrap> xbs = new ::x2p::cellml_api::CellMLBootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

#if ENABLE_RDF
NS_IMETHODIMP
RDFBootstrapConstructor(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::rdf_api::Bootstrap* bs = CreateRDFBootstrap();
  nsCOMPtr<rdf_apiIBootstrap> xbs = new ::x2p::rdf_api::Bootstrap(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}
#endif

static const nsModuleComponentInfo components[] =
{
  {
    "CellMLBootstrap",
    CELLML_BOOTSTRAP_CID,
    "@cellml.org/cellml-bootstrap;1",
    CellMLBootstrapConstructor
  }
#if ENABLE_RDF
  ,{
    "RDFBootstrap",
    RDF_BOOTSTRAP_CID,
    "@cellml.org/rdf-bootstrap;1",
    RDFBootstrapConstructor
  }
#endif
};

NS_IMPL_NSGETMODULE(CellMLBootstrap, components)
