#include "Utilities.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include "x2pCellML_APISPEC.hxx"
#include <nsIGenericFactory.h>

#define CELLML_BOOTSTRAP_CID                         \
{ /* 56849f69-c321-4be6-9191-1bb2ee0a76e0 */         \
    0x56849f69,                                      \
    0xc321,                                          \
    0x4be6,                                          \
    {0x91, 0x91, 0x1b, 0xb2, 0xee, 0x0a, 0x75, 0xe0} \
}

NS_IMETHODIMP
CellMLBootstrapConstructor(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_api::CellMLBootstrap* bs = CreateCellMLBootstrap();
  nsCOMPtr<cellml_apiICellMLBootstrap> xbs = new ::x2p::cellml_api::CellMLBootstrap(bs);
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CellMLBootstrap",
    CELLML_BOOTSTRAP_CID,
    "@cellml.org/cellml-bootstrap;1",
    CellMLBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CellMLBootstrap, components)
