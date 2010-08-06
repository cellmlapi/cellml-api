#include "Utilities.hxx"
#include "IfaceCellML_Context.hxx"
#include "CellMLContextBootstrap.hxx"
#include "x2pCellML_Context.hxx"
#include <nsIGenericFactory.h>

#define CELLML_CONTEXT_BOOTSTRAP_CID                 \
{ /* 988c6513-73b6-4900-b354-4a9be12ef4ed */         \
    0x988c6513,                                      \
    0x73b6,                                          \
    0x4900,                                          \
    {0xb3, 0x54, 0x4a, 0x9b, 0xe1, 0x2e, 0xf4, 0xed} \
}

NS_IMETHODIMP
CellMLContextBootstrapConstructor
(
 nsISupports *aOuter,
 REFNSIID aIID,
 void **aResult
)
{
  if (aOuter != nsnull)
    return NS_ERROR_NO_AGGREGATION;
  iface::cellml_context::CellMLContext* bs = CreateCellMLContext();
  nsCOMPtr<cellml_contextICellMLContext> xbs =
    new ::x2p::cellml_context::CellMLContext(bs);
  bs->release_ref();
  return xbs->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  {
    "CellMLContextBootstrap",
    CELLML_CONTEXT_BOOTSTRAP_CID,
    "@cellml.org/cellml-context-bootstrap;1",
    CellMLContextBootstrapConstructor
  }
};

NS_IMPL_NSGETMODULE(CellMLContextBootstrap, components)
