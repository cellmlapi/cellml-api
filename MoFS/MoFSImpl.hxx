#include "IfaceMoFS.hxx"
#include "Utilities.hxx"

class CDA_ModelFlatteningService
  : public iface::mofs::ModelFlatteningService
{
public:
  CDA_ModelFlatteningService() {};
  CDA_IMPL_QI1(mofs::ModelFlatteningService);
  CDA_IMPL_OBJREF;
  CDA_IMPL_ID;

  iface::cellml_api::Model* flattenModel(iface::cellml_api::Model* aModel)
    : throw(iface::cellml_api::CellMLException);
};
