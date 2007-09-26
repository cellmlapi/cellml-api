#include "VACSSImpl.hpp"
#include "VACSSBootstrap.hpp"

CDA_VACSService::CDA_VACSService()
{
}

iface::cellml_services::CellMLValidityErrorSet*
CDA_VACSService::validateModel(iface::cellml_api::Model* aModel)
  throw()
{
  return NULL;
}

uint32_t
CDA_VACSService::getPositionInXML
(
 iface::dom::Node* aNode,
 uint32_t aNodalOffset,
 uint32_t* aColumn
)
  throw()
{
  *aColumn = 0;
  return 0;
}

iface::cellml_services::VACSService*
CreateVACSService(void)
{
  return new CDA_VACSService();
}
