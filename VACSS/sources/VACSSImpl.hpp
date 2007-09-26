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

private:
  /**
   * Advances the row / column pair through the node until either there is no
   * more data, or we find the node aUntil.
   * @param aNode in The DOM node to advance through.
   * @param aRow in/out The row counter.
   * @param aCol in/out The column counter.
   * @param aUntil in The node to stop at.
   * @param aUntilOffset The offset in aUntil to stop at.
   * @return true if & only if aUntil was encountered.
   */
  bool advanceCursorThroughNodeUntil(iface::dom::Node* aNode,
                                     uint32_t& aRow,
                                     uint32_t& aCol,
                                     iface::dom::Node* aUntil,
                                     uint32_t aUntilOffset);

  /**
   * Advances through a string.
   */
  void advanceCursorThroughString(const std::wstring& aStr,
                                  uint32_t& aRow,
                                  uint32_t& aCol,
                                  bool aStopHere,
                                  uint32_t aStopOffset
                                 );
};

#endif // _VACSSImplementation_hpp
