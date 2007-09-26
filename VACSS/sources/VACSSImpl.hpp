#ifndef _VACSSImplementation_hpp
#define _VACSSImplementation_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "Utilities.hxx"
#include "IfaceVACSS.hxx"

class CDA_VACSService
  : public iface::cellml_services::VACSService
{
public:
  CDA_VACSService();

  CDA_IMPL_ID;  
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::VACSService);

  iface::cellml_services::CellMLValidityErrorSet* validateModel
    (iface::cellml_api::Model* aModel) throw ();
  uint32_t getPositionInXML(iface::dom::Node* aNode,
                            uint32_t aNodalOffset,
                            uint32_t* aColumn)
    throw ();
};

#endif // _VACSSImplementation_hpp
