#ifndef _VACSSImplementation_hpp
#define _VACSSImplementation_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include <vector>
#include <string>
#include "IfaceCellML_APISPEC.hxx"
#include "Utilities.hxx"
#include "IfaceVACSS.hxx"
#include "IfaceCUSES.hxx"

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

class CDA_CellMLValidityErrorSet
  : public iface::cellml_services::CellMLValidityErrorSet
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::CellMLValidityErrorSet);

  CDA_CellMLValidityErrorSet();
  virtual ~CDA_CellMLValidityErrorSet() throw();
  uint32_t nValidityErrors() throw();
  iface::cellml_services::CellMLValidityError* getValidityError(uint32_t aIndex)
    throw(std::exception&);

  // Does not follow PCM rules (takes ownership of the CellMLValidityError, to
  // make it easier to use locally by calling new.
  void adoptValidityError(iface::cellml_services::CellMLValidityError* ve);

private:
  typedef std::vector<iface::cellml_services::CellMLValidityError*> errorlist_t;
  errorlist_t mErrors;
};

class CDA_CellMLValidityErrorBase
  : public virtual iface::cellml_services::CellMLValidityError
{
public:
  /**
   * @note The ownership of aSupplement passes to the new object.
   */
  CDA_CellMLValidityErrorBase(const std::wstring& aDescription,
                          bool aIsWarning = false,
                          iface::cellml_services::CellMLValidityError* aSupplement = NULL)
    throw();
  virtual ~CDA_CellMLValidityErrorBase() throw();
  
  wchar_t* description() throw();
  uint32_t nSupplements() throw();
  iface::cellml_services::CellMLValidityError* getSupplement(uint32_t nSupplements)
    throw(std::exception&);
  bool isWarningOnly() throw();

private:
  std::wstring mDescription;
  bool mIsWarning;
  ObjRef<iface::cellml_services::CellMLValidityError> mSupplement;
};

class CDA_CellMLValidityError
  : public CDA_CellMLValidityErrorBase
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::CellMLValidityError);

  /**
   * @note The ownership of aSupplement passes to the new object.
   */
  CDA_CellMLValidityError(const std::wstring& aDescription,
                          bool aIsWarning = false,
                          iface::cellml_services::CellMLValidityError* aSupplement = NULL)
    throw();
  virtual ~CDA_CellMLValidityError() throw() {}
};

class CDA_CellMLRepresentationValidityError
  : public CDA_CellMLValidityErrorBase,
    public iface::cellml_services::CellMLRepresentationValidityError
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI2(cellml_services::CellMLValidityError,
               cellml_services::CellMLRepresentationValidityError);

  /**
   * @note The ownership of aSupplement passes to the new object.
   */
  CDA_CellMLRepresentationValidityError
  (
   const std::wstring& aDescription,
   iface::dom::Node* aNode,
   uint32_t errorNodalOffset = 0,
   bool aIsWarning = false,
   iface::cellml_services::CellMLValidityError* aSupplement = NULL
  )
    throw();
  virtual ~CDA_CellMLRepresentationValidityError() throw() {}

  iface::dom::Node* errorNode() throw();
  uint32_t errorNodalOffset() throw();

private:
  ObjRef<iface::dom::Node> mNode;
  uint32_t mErrorNodalOffset;
};

class CDA_CellMLSemanticValidityError
  : public CDA_CellMLValidityErrorBase,
    public iface::cellml_services::CellMLSemanticValidityError
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI2(cellml_services::CellMLValidityError,
               cellml_services::CellMLSemanticValidityError);

  /**
   * @note The ownership of aSupplement passes to the new object.
   */
  CDA_CellMLSemanticValidityError
  (
   const std::wstring& aDescription,
   iface::cellml_api::CellMLElement* aElement,
   bool aIsWarning = false,
   iface::cellml_services::CellMLValidityError* aSupplement = NULL
  )
    throw();
  virtual ~CDA_CellMLSemanticValidityError() throw() {}

  iface::cellml_api::CellMLElement* errorElement() throw();

private:
  ObjRef<iface::cellml_api::CellMLElement> mElement;
};

class ModelValidation
{
public:
  ModelValidation(iface::cellml_api::Model* aModel);
  
  iface::cellml_services::CellMLValidityErrorSet* validate();
private:
  iface::cellml_api::Model * mModel;
  ObjRef<CDA_CellMLValidityErrorSet> mErrors;
  static const uint32_t kCellML_1_0 = 0;
  static const uint32_t kCellML_1_1 = 1;

  uint32_t mCellMLVersion;

  void validateRepresentation(iface::dom::Element* aTop);
  void validateSemantics();
  void validatePerModel(iface::cellml_api::Model* aModel);
  void validatePerImport(iface::cellml_api::CellMLImport* aImport);
  void validateNameUniqueness(iface::cellml_api::Model* aModel);
  void validateComponentRefs(iface::cellml_api::CellMLImport* aImport);
  void validateUnitsRefs(iface::cellml_api::CellMLImport* aImport);
  void validateExtensionElement(iface::dom::Element* aEl);
  void validatePerComponent(iface::cellml_api::CellMLComponent* aComponent);
  void validatePerUnits(iface::cellml_api::Units* aUnits);
  void validatePerImportComponent(iface::cellml_api::ImportComponent*
                                  aComponent);
  void validatePerImportUnits(iface::cellml_api::ImportUnits* aUnits);

  struct ReprValidationAttribute;
  struct ReprValidationElement
  {
    const wchar_t* namespace_name;
    const wchar_t* name;
    const ReprValidationAttribute** attributes;
    const ReprValidationElement** children;
    uint32_t minVersion;
    uint32_t maxVersion;
    uint32_t minInParent;
    const wchar_t* tooFewMessage;
    uint32_t maxInParent;
    const wchar_t* tooManyMessage;
    void (ModelValidation::* textValidator)(iface::dom::Node* aContext,
                                            const std::wstring&);
    typedef enum {
      NOTHING_FURTHER,
      EXTRANEOUS_ELEMENTS,
      EXTRANEOUS_ATTRIBUTES,
      EXTRANEOUS_ELEMENTS_ATTRIBUTES
    } ElementValidationLevel;
    ElementValidationLevel (ModelValidation::* customValidator)(iface::dom::Element*);
  };

  void validateElementRepresentation(iface::dom::Element* aEl,
                                     const ReprValidationElement& aSpec);

  static const ReprValidationElement* sNoChildren[];
  static const ReprValidationElement sModelVE;
  static const ReprValidationElement* sModelChildren[];
  static const ReprValidationElement sImportVE;
  static const ReprValidationElement* sImportChildren[];
  static const ReprValidationElement sModelUnitsVE;
  static const ReprValidationElement* sModelUnitsChildren[];
  static const ReprValidationElement sImportUnitsVE;
  static const ReprValidationElement sModelComponentVE;
  static const ReprValidationElement* sModelComponentChildren[];
  static const ReprValidationElement sImportComponentVE;
  static const ReprValidationElement sGroupVE;
  static const ReprValidationElement* sGroupChildren[];
  static const ReprValidationElement sConnectionVE;
  static const ReprValidationElement* sConnectionChildren[];
  static const ReprValidationElement sVariableVE;
  static const ReprValidationElement sMapComponentsVE;
  static const ReprValidationElement sMapVariablesVE;
  static const ReprValidationElement sUnitVE;
  static const ReprValidationElement sRelationshipRefVE;
  static const ReprValidationElement sComponentRefVE;
  static const ReprValidationElement sSubComponentRefVE;
  static const ReprValidationElement* sComponentRefChildren[];
  static const ReprValidationElement sReactionVE;
  static const ReprValidationElement* sReactionChildren[];
  static const ReprValidationElement sVariableRefVE;
  static const ReprValidationElement* sVariableRefChildren[];
  static const ReprValidationElement sRoleVE;
  static const ReprValidationElement* sRoleChildren[];
  static const ReprValidationElement sMathVE;

  struct ReprValidationAttribute
  {
    const wchar_t* namespace_name;
    const wchar_t* name;
    const wchar_t* missingMessage;
    void (ModelValidation::* contentValidator)(iface::dom::Node* aContext,
                                               const std::wstring&);
  };
  static const ReprValidationAttribute sMandatoryName;
  static const ReprValidationAttribute sOptionalName;
  static const ReprValidationAttribute sXlinkHref;
  static const ReprValidationAttribute sBaseUnits;
  static const ReprValidationAttribute sUnitsRef;
  static const ReprValidationAttribute sComponentRef;
  static const ReprValidationAttribute sPublicInterface;
  static const ReprValidationAttribute sPrivateInterface;
  static const ReprValidationAttribute sInitialValue;
  static const ReprValidationAttribute sComponent1Attr;
  static const ReprValidationAttribute sComponent2Attr;
  static const ReprValidationAttribute sVariable1Attr;
  static const ReprValidationAttribute sVariable2Attr;
  static const ReprValidationAttribute sPrefixAttr;
  static const ReprValidationAttribute sExponentAttr;
  static const ReprValidationAttribute sMultiplierAttr;
  static const ReprValidationAttribute sOffsetAttr;
  static const ReprValidationAttribute sComponentAttr;
  static const ReprValidationAttribute sReversibleAttr;
  static const ReprValidationAttribute sVariableAttr;
  static const ReprValidationAttribute sRoleAttr;
  static const ReprValidationAttribute sDeltaVariableAttr;
  static const ReprValidationAttribute sDirectionAttr;
  static const ReprValidationAttribute sStoichiometryAttr;
  static const ReprValidationAttribute sComponentRefAttr;
  static const ReprValidationAttribute sUnitsAttr;

  static const ReprValidationAttribute* sModelAttrs[];
  static const ReprValidationAttribute* sImportAttrs[];
  static const ReprValidationAttribute* sModelUnitsAttrs[];
  static const ReprValidationAttribute* sImportUnitsAttrs[];
  static const ReprValidationAttribute* sModelComponentAttrs[];
  static const ReprValidationAttribute* sImportComponentAttrs[];
  static const ReprValidationAttribute* sNoAttrs[];
  static const ReprValidationAttribute* sVariableAttrs[];
  static const ReprValidationAttribute* sMapComponentsAttrs[];
  static const ReprValidationAttribute* sMapVariablesAttrs[];
  static const ReprValidationAttribute* sUnitAttrs[];
  static const ReprValidationAttribute* sRelationshipRefAttrs[];
  static const ReprValidationAttribute* sComponentRefAttrs[];
  static const ReprValidationAttribute* sReactionAttrs[];
  static const ReprValidationAttribute* sVariableRefAttrs[];
  static const ReprValidationAttribute* sRoleAttrs[];

  void whitespaceOnlyValidatorCellML(iface::dom::Node* aContext,
                                     const std::wstring& aText);
  ReprValidationElement::ElementValidationLevel
    validateRelationshipRef(iface::dom::Element* aRR);
  void validateCellMLIdentifier(iface::dom::Node* aContext,
                                const std::wstring& aIdent);
  void validateBaseUnits(iface::dom::Node* aContext,
                         const std::wstring& aBU);
  void validateInterface(iface::dom::Node* aContext,
                         const std::wstring& aIntf);
  void validateInitialValue(iface::dom::Node* aContext,
                            const std::wstring& aInitial);
  void validatePrefix(iface::dom::Node* aContext,
                      const std::wstring& aPrefix);
  void validateFloatingPoint(iface::dom::Node* aContext,
                             const std::wstring& aFP);
  void validateReversible(iface::dom::Node* aContext,
                          const std::wstring& aReversible);
  void validateRole(iface::dom::Node* aContext,
                    const std::wstring& aRole);
  void validateDirection(iface::dom::Node* aContext,
                         const std::wstring& aDirection);
  ReprValidationElement::ElementValidationLevel
    validateMaths(iface::dom::Element* aRR);

  iface::cellml_services::CUSES* mCUSES;
};


#endif // _VACSSImplementation_hpp
