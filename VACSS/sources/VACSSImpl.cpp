#include "VACSSImpl.hpp"
#include "VACSSBootstrap.hpp"
#include <string>

/* The code special cases this to match the CellML namespace
 * corresponding to the appropriate CellML version...
 */
#define MATCHING_CELLML_NS L"http://www.cellml.org/cellml/"
#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"
#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"
#define XLINK_NS L"http://www.w3.org/1999/xlink"

CDA_VACSService::CDA_VACSService()
  : _cda_refcount(1)
{
}

CDA_CellMLValidityErrorSet::CDA_CellMLValidityErrorSet()
  : _cda_refcount(1)
{
}

CDA_CellMLValidityErrorSet::~CDA_CellMLValidityErrorSet()
  throw()
{
  errorlist_t::iterator i;
  for (i = mErrors.begin(); i != mErrors.end(); i++)
    (*i)->release_ref();
}

uint32_t
CDA_CellMLValidityErrorSet::nValidityErrors()
  throw()
{
  return mErrors.size();
}

iface::cellml_services::CellMLValidityError*
CDA_CellMLValidityErrorSet::getValidityError(uint32_t aIndex)
  throw(std::exception&)
{
  if (aIndex >= mErrors.size())
    throw iface::cellml_api::CellMLException();
  
  iface::cellml_services::CellMLValidityError* e = mErrors[aIndex];
  
  if (e != NULL)
    e->add_ref();
  
  return e;
}

void
CDA_CellMLValidityErrorSet::adoptValidityError(iface::cellml_services::CellMLValidityError* ve)
{
  mErrors.push_back(ve);
}

CDA_CellMLValidityErrorBase::CDA_CellMLValidityErrorBase
(
 const std::wstring& aDescription,
 bool aIsWarning,
 iface::cellml_services::CellMLValidityError* aSupplement
) throw()
  : mDescription(aDescription),
    mIsWarning(aIsWarning),
    mSupplement(aSupplement)
{
}

CDA_CellMLValidityErrorBase::~CDA_CellMLValidityErrorBase()
  throw()
{
}

wchar_t*
CDA_CellMLValidityErrorBase::description()
  throw()
{
  return CDA_wcsdup(mDescription.c_str());
}

uint32_t
CDA_CellMLValidityErrorBase::nSupplements()
  throw()
{
  if (mSupplement)
    return 1;

  return 0;
}

iface::cellml_services::CellMLValidityError*
CDA_CellMLValidityErrorBase::getSupplement(uint32_t supplement)
  throw(std::exception&)
{
  if (supplement > 0)
    return NULL;

  return mSupplement;
}

bool
CDA_CellMLValidityErrorBase::isWarningOnly()
  throw()
{
  return mIsWarning;
}

CDA_CellMLValidityError::CDA_CellMLValidityError
(
 const std::wstring& aDescription,
 bool aIsWarning,
 iface::cellml_services::CellMLValidityError* aSupplement
) throw()
  : CDA_CellMLValidityErrorBase(aDescription, aIsWarning, aSupplement),
    _cda_refcount(1)
{
}

CDA_CellMLRepresentationValidityError::CDA_CellMLRepresentationValidityError
(
 const std::wstring& aDescription,
 iface::dom::Node* aNode,
 uint32_t errorNodalOffset,
 bool aIsWarning,
 iface::cellml_services::CellMLValidityError* aSupplement
) throw()
  : CDA_CellMLValidityErrorBase(aDescription, aIsWarning, aSupplement),
    _cda_refcount(1),
    mNode(aNode), mErrorNodalOffset(errorNodalOffset)
{
}

iface::dom::Node*
CDA_CellMLRepresentationValidityError::errorNode()
  throw()
{
  if (mNode)
    mNode->add_ref();

  return mNode;
}

uint32_t
CDA_CellMLRepresentationValidityError::errorNodalOffset()
  throw()
{
  return mErrorNodalOffset;
}

CDA_CellMLSemanticValidityError::CDA_CellMLSemanticValidityError
(
 const std::wstring& aDescription,
 iface::cellml_api::CellMLElement* aElement,
 bool aIsWarning,
 iface::cellml_services::CellMLValidityError* aSupplement
) throw()
  : CDA_CellMLValidityErrorBase(aDescription, aIsWarning, aSupplement),
    _cda_refcount(1),
    mElement(aElement)
{
}

iface::cellml_api::CellMLElement*
CDA_CellMLSemanticValidityError::errorElement()
  throw()
{
  if (mElement)
    mElement->add_ref();

  return mElement;
}

ModelValidation::ModelValidation(iface::cellml_api::Model* aModel)
  : mModel(aModel)
{
}

#define SEMANTIC_ERROR(message, node) \
  mErrors->adoptValidityError(new CDA_CellMLSemanticValidityError(message, node))
#define SEMANTIC_WARNING(message, node) \
  mErrors->adoptValidityError(new CDA_CellMLSemanticValidityError(message, node, true))
#define REPR_ERROR(message, node) \
  mErrors->adoptValidityError(new CDA_CellMLRepresentationValidityError(message, node))
#define REPR_WARNING(message, node) \
  mErrors->adoptValidityError(new CDA_CellMLRepresentationValidityError(message, node, 0, true))

iface::cellml_services::CellMLValidityErrorSet*
ModelValidation::validate()
{
  mErrors = already_AddRefd<CDA_CellMLValidityErrorSet>
    (new CDA_CellMLValidityErrorSet());

  // Get the top-level element...
  DECLARE_QUERY_INTERFACE_OBJREF(mModelDE, mModel,
                                 cellml_api::CellMLDOMElement);
  if (mModelDE != NULL)
  {
    RETURN_INTO_OBJREF(topEl, iface::dom::Element, mModelDE->domElement());
    validateRepresentation(topEl);
  }
  else
    SEMANTIC_WARNING(L"Can't perform representational validation because you are "
                     L"using a model implementation which doesn't allow the DOM "
                     L"to be fetched; this may cause an invalid model to be "
                     L"reported as valid.",
                     mModel);

  validateSemantics();

  if (mErrors != NULL)
    mErrors->add_ref();
  return mErrors;
}

static const wchar_t* kCellMLNamespaces[] =
  {
    L"http://www.cellml.org/cellml/1.0#",
    L"http://www.cellml.org/cellml/1.1#",
  };

#define INTERNAL_ERROR_MESSAGE_M(l) L"Internal error (line " #l L" in " \
  __FILE__ L"): this should never happen, please report a bug."
#define INTERNAL_ERROR_MESSAGE INTERNAL_ERROR_MESSAGE_M(__LINE__)

const ModelValidation::ReprValidationElement
ModelValidation::sModelVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"model",
    /* attributes */sModelAttrs,
    /* children */sModelChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sModelChildren[] =
  {
    &sImportVE,
    &sModelUnitsVE,
    &sModelComponentVE,
    &sGroupVE,
    &sConnectionVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sModelAttrs[] =
  {
    &sMandatoryName,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sImportVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"import",
    /* attributes */sImportAttrs,
    /* children */sImportChildren,
    /* minVersion */kCellML_1_1,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sImportChildren[] =
  {
    &sImportUnitsVE,
    &sImportComponentVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sImportAttrs[] =
  {
    &sXlinkHref,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sModelUnitsVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"units",
    /* attributes */sModelUnitsAttrs,
    /* children */sModelUnitsChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sModelUnitsChildren[] =
  {
    &sUnitVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sModelUnitsAttrs[] =
  {
    &sMandatoryName,
    &sBaseUnits,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sImportUnitsVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"units",
    /* attributes */sImportUnitsAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_1,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sNoChildren[] =
  {
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sImportUnitsAttrs[] =
  {
    &sMandatoryName,
    &sUnitsRef,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sModelComponentVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"component",
    /* attributes */sModelComponentAttrs,
    /* children */sModelComponentChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sModelComponentChildren[] =
  {
    &sVariableVE,
    &sReactionVE,
    &sModelUnitsVE,
    &sMathVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sModelComponentAttrs[] =
  {
    &sMandatoryName,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sImportComponentVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"component",
    /* attributes */sImportComponentAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_1,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sImportComponentAttrs[] =
  {
    &sMandatoryName,
    &sComponentRefAttr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sGroupVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"group",
    /* attributes */sNoAttrs,
    /* children */sGroupChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sGroupChildren[] =
  {
    &sRelationshipRefVE,
    &sComponentRefVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sNoAttrs[] =
  {
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sConnectionVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"connection",
    /* attributes */sNoAttrs,
    /* children */sConnectionChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sConnectionChildren[] =
  {
    &sMapComponentsVE,
    &sMapVariablesVE,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sVariableVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"variable",
    /* attributes */sVariableAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sVariableAttrs[] =
  {
    &sMandatoryName,
    &sUnitsAttr,
    &sPublicInterface,
    &sPrivateInterface,
    &sInitialValue,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sMapComponentsVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"map_components",
    /* attributes */sMapComponentsAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */1,
    /* tooFewMessage */L"Each <connection> element MUST contain exactly one "
                       L"<map_components> element (section 3.4.4.1)",
    /* maxInParent */1,
    /* tooManyMessage */L"Each <connection> element MUST contain exactly one "
                        L"<map_components> element (section 3.4.4.1)",
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sMapComponentsAttrs[] =
  {
    &sComponent1Attr,
    &sComponent2Attr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sMapVariablesVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"map_variables",
    /* attributes */sMapVariablesAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */1,
    /* tooFewMessage */L"Each <connection> element MUST contain at least "
                       L"one <map_variables> element (section 3.4.4.1)",
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sMapVariablesAttrs[] =
  {
    &sVariable1Attr,
    &sVariable2Attr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sUnitVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"unit",
    /* attributes */sUnitAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sUnitAttrs[] =
  {
    &sUnitsAttr,
    &sPrefixAttr,
    &sExponentAttr,
    &sMultiplierAttr,
    &sOffsetAttr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sRelationshipRefVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"relationship_ref",
    /* attributes */sRelationshipRefAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */1,
    /* tooFewMessage */L"A <group> element MUST contain at least one "
                       L"<relationship_ref> element (section 6.4.1.1)",
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */&ModelValidation::validateRelationshipRef
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sRelationshipRefAttrs[] =
  {
    &sOptionalName,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sComponentRefVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"component_ref",
    /* attributes */sComponentRefAttrs,
    /* children */sComponentRefChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */1,
    /* tooFewMessage */L"A <group> element MUST contain at least one "
                       L"<component_ref> element (section 6.4.1.1)",
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sSubComponentRefVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"component_ref",
    /* attributes */sComponentRefAttrs,
    /* children */sComponentRefChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sComponentRefChildren[] =
  {
    &sSubComponentRefVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sComponentRefAttrs[] =
  {
    &sComponentAttr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sReactionVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"reaction",
    /* attributes */sReactionAttrs,
    /* children */sReactionChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */INTERNAL_ERROR_MESSAGE,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sReactionChildren[] =
  {
    &sVariableRefVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sReactionAttrs[] =
  {
    &sReversibleAttr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sVariableRefVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"variable_ref",
    /* attributes */sVariableRefAttrs,
    /* children */sVariableRefChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */1,
    /* tooFewMessage */L"Each <reaction> element must contain at least "
                       L"one <variable_ref> element (section 7.4.1.1)",
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sVariableRefChildren[] =
  {
    &sRoleVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sVariableRefAttrs[] =
  {
    &sVariableAttr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sRoleVE =
  { /* namespace */MATCHING_CELLML_NS,
    /* name */ L"role",
    /* attributes */sRoleAttrs,
    /* children */sRoleChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */1,
    /* tooFewMessage */L"Each <variable_ref> element must contain at least "
                       L"one <role> element (section 7.4.2.1)",
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */NULL
  };

const ModelValidation::ReprValidationElement*
ModelValidation::sRoleChildren[] =
  {
    &sMathVE,
    NULL
  };

const ModelValidation::ReprValidationAttribute*
ModelValidation::sRoleAttrs[] =
  {
    &sRoleAttr,
    &sDeltaVariableAttr,
    &sDirectionAttr,
    &sStoichiometryAttr,
    NULL
  };

const ModelValidation::ReprValidationElement
ModelValidation::sMathVE =
  { /* namespace */MATHML_NS,
    /* name */ L"math",
    /* attributes */sNoAttrs,
    /* children */sNoChildren,
    /* minVersion */kCellML_1_0,
    /* maxVersion */kCellML_1_1,
    /* minInParent */0,
    /* tooFewMessage */NULL,
    /* maxInParent */0,
    /* tooManyMessage */INTERNAL_ERROR_MESSAGE,
    /* textValidator */&ModelValidation::whitespaceOnlyValidatorCellML,
    /* customValidator */&ModelValidation::validateMaths
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sMandatoryName =
  {
    /* namespace */NULL,
    /* name */L"name",
    /* missingMessage */L"The CellML specification says the name attribute "
                        L"is required here",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sOptionalName =
  {
    /* namespace */NULL,
    /* name */L"name",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sXlinkHref =
  {
    /* namespace */XLINK_NS,
    /* name */L"href",
    /* missingMessage */L"The CellML specification says the xlink:href attribute "
                        L"is required here (section 9.4.1.1)",
    /* contentValidator */NULL
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sBaseUnits =
  {
    /* namespace */NULL,
    /* name */L"base_units",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateBaseUnits
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sUnitsRef =
  {
    /* namespace */NULL,
    /* name */L"units_ref",
    /* missingMessage */L"Each <units> element appearing as the child of an "
                        L"<import> element MUST also define a units_ref "
                        L"attribute (section 5.4.1.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sPublicInterface =
  {
    /* namespace */NULL,
    /* name */L"public_interface",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateInterface
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sPrivateInterface =
  {
    /* namespace */NULL,
    /* name */L"private_interface",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateInterface
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sInitialValue =
  {
    /* namespace */NULL,
    /* name */L"initial_value",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateInitialValue
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sComponent1Attr =
  {
    /* namespace */NULL,
    /* name */L"component_1",
    /* missingMessage */L"Each <map_components> element MUST define a "
                        L"component_1 attribute (section 3.4.5.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sComponent2Attr =
  {
    /* namespace */NULL,
    /* name */L"component_2",
    /* missingMessage */L"Each <map_components> element MUST define a "
                        L"component_2 attribute (section 3.4.5.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sVariable1Attr =
  {
    /* namespace */NULL,
    /* name */L"variable_1",
    /* missingMessage */L"Each <map_variables> element MUST define a variable_1 "
                        L"attribute (section 3.4.6.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sVariable2Attr =
  {
    /* namespace */NULL,
    /* name */L"variable_2",
    /* missingMessage */L"Each <map_variables> element MUST define a variable_2 "
                        L"attribute (section 3.4.6.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sPrefixAttr =
  {
    /* namespace */NULL,
    /* name */L"prefix",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validatePrefix
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sExponentAttr =
  {
    /* namespace */NULL,
    /* name */L"exponent",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateFloatingPoint
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sMultiplierAttr =
  {
    /* namespace */NULL,
    /* name */L"multiplier",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateFloatingPoint
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sOffsetAttr =
  {
    /* namespace */NULL,
    /* name */L"offset",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateFloatingPoint
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sComponentAttr =
  {
    /* namespace */NULL,
    /* name */L"component",
    /* missingMessage */L"A <component_ref> element must define a component "
                        L"attribute (section 6.4.3.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sReversibleAttr =
  {
    /* namespace */NULL,
    /* name */L"reversible",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateReversible
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sVariableAttr =
  {
    /* namespace */NULL,
    /* name */L"variable",
    /* missingMessage */L"Each <variable_ref> element must define a "
                        L"variable attribute (section 7.4.2.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sRoleAttr =
  {
    /* namespace */NULL,
    /* name */L"role",
    /* missingMessage */L"Each <role> element must define a role attribute "
                        L"(section 7.4.3.1)",
    /* contentValidator */&ModelValidation::validateRole
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sDeltaVariableAttr =
  {
    /* namespace */NULL,
    /* name */L"delta_variable",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sDirectionAttr =
  {
    /* namespace */NULL,
    /* name */L"direction",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateDirection
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sStoichiometryAttr =
  {
    /* namespace */NULL,
    /* name */L"stoichiometry",
    /* missingMessage */NULL,
    /* contentValidator */&ModelValidation::validateFloatingPoint
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sComponentRefAttr =
  {
    /* namespace */NULL,
    /* name */L"component_ref",
    /* missingMessage */L"Each <component> element appearing as the child "
                        L"of an <import> element MUST additionally define a "
                        L"component_ref attribute (section 3.4.2.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

const ModelValidation::ReprValidationAttribute
ModelValidation::sUnitsAttr =
  {
    /* namespace */NULL,
    /* name */L"units",
    /* missingMessage */L"Each <variable> / <unit> element MUST define a "
                        L"a units attribute (section 3.4.3.1 / 5.4.3.1)",
    /* contentValidator */&ModelValidation::validateCellMLIdentifier
  };

void
ModelValidation::validateRepresentation(iface::dom::Element* aTop)
{
  RETURN_INTO_WSTRING(ns, aTop->namespaceURI());

  if (ns == kCellMLNamespaces[0])
    mCellMLVersion = kCellML_1_0;
  else if (ns == kCellMLNamespaces[1])
    mCellMLVersion = kCellML_1_1;
  else
  {
    REPR_ERROR(L"The top-level element is in an unrecognised namespace. No "
               L"further errors can be displayed because the version to "
               L"validate against could not be determined.",
               aTop);
    return;
  }

  RETURN_INTO_WSTRING(ln, aTop->localName());
  if (ln == L"model")
    {
    }
  else
  {
    REPR_ERROR(L"The top-level element is not <model>. No "
               L"further errors can be displayed because of this.",
               aTop);
    return;
  }

  validateElementRepresentation(aTop, sModelVE);
}

void
ModelValidation::validateElementRepresentation
(
 iface::dom::Element* aEl,
 const ReprValidationElement& aSpec
)
{
  uint32_t seenAttrMasks = 0, seenAttrWrongNSMasks = 0;

  if (aSpec.minVersion > mCellMLVersion ||
      aSpec.maxVersion < mCellMLVersion)
  {
    std::wstring msg(L"Element ");
    msg += aSpec.name;
    msg += L" is invalid in this version of CellML.";
    REPR_ERROR(msg, aEl);
  }

  ReprValidationElement::ElementValidationLevel evl
    (ReprValidationElement::EXTRANEOUS_ELEMENTS_ATTRIBUTES);
  if (aSpec.customValidator)
    evl = (this->*(aSpec.customValidator))(aEl);
  bool extraAtt = (evl == ReprValidationElement::EXTRANEOUS_ATTRIBUTES) ||
    (evl == ReprValidationElement::EXTRANEOUS_ELEMENTS_ATTRIBUTES);
  bool extraEl = (evl == ReprValidationElement::EXTRANEOUS_ELEMENTS) ||
    (evl == ReprValidationElement::EXTRANEOUS_ELEMENTS_ATTRIBUTES);

  RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap,
                     aEl->attributes());
  uint32_t i, l = nnm->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(atn, iface::dom::Node, nnm->item(i));
    RETURN_INTO_WSTRING(ns, atn->namespaceURI());
    RETURN_INTO_WSTRING(ln, atn->localName());
    if (ln == L"")
    {
      wchar_t* tmp = atn->nodeName();
      ln = tmp;
      free(tmp);
    }

    if (ns == ((mCellMLVersion == kCellML_1_0) ?
               CELLML_1_1_NS : CELLML_1_0_NS))
    {
      REPR_ERROR(L"It is not valid to mix the CellML 1.0 and 1.1 namespaces "
                 L"in the same model document", atn);
      ns = (mCellMLVersion == kCellML_1_0) ? CELLML_1_0_NS : CELLML_1_1_NS;
    }

    const ReprValidationAttribute** p = aSpec.attributes;
    uint32_t mask = 1;
    for (; *p != NULL; p++, mask <<= 1)
    {
      if (ln != (*p)->name)
        continue;

      const wchar_t* match_ns = (*p)->namespace_name;
      if (match_ns == NULL)
      {
        // Kludge for the brokenness in the CellML specification, which says
        // that an attribute in the empty namespace is really in the CellML
        // namespace...
        if (ns == L"")
          match_ns = L"";
        else
          match_ns = MATCHING_CELLML_NS;
      }

      // Pointer comparison deliberate...
      if (match_ns == MATCHING_CELLML_NS)
        match_ns = (mCellMLVersion == kCellML_1_0) ?
          CELLML_1_0_NS : CELLML_1_1_NS;

      if (ns != match_ns)
      {
        // So we can get a more useful error message if someone gets the
        // namespace wrong...
        seenAttrWrongNSMasks |= mask;
        continue;
      }

      seenAttrMasks |= mask;

      if ((*p)->contentValidator != NULL)
      {
        RETURN_INTO_WSTRING(val, atn->nodeValue());
        ((this->*((*p)->contentValidator))(atn, val));
      }

      break;
    }

    if (extraAtt && (*p) == NULL)
    {
      if (ns == CELLML_1_0_NS || ns == CELLML_1_1_NS || ns == MATHML_NS ||
          ns == XLINK_NS || ns == L"")
      {
        std::wstring msg(L"Unexpected attribute ");
        msg += ln;
        msg += L" found - not valid here";
        REPR_ERROR(msg, atn);
      }
    }
  }

  const ReprValidationAttribute** p = aSpec.attributes;
  uint32_t mask = 1;
  for (; *p != NULL; p++, mask <<= 1)
  {
    if ((*p)->missingMessage == NULL)
      continue;

    if (seenAttrMasks & mask)
      continue;

    std::wstring missingMessage((*p)->missingMessage);
    if (seenAttrWrongNSMasks & mask)
      missingMessage += L". Note that an element with a matching name was "
        L"seen in a different namespace";

    REPR_ERROR(missingMessage, aEl);
  }

  const ReprValidationElement** e;

  // hard-coded 32 because we use a 32 bit mask above. This is a size limit on
  // the table which drives this code and not on the data so it is not worth
  // sacrificing the time needed to allocate here...
  uint32_t counts[32];
  memset(counts, 0, 32 * sizeof(uint32_t));

  RETURN_INTO_OBJREF(child, iface::dom::Node, aEl->firstChild());
  std::wstring textData;
  for (; child != NULL;
       child = already_AddRefd<iface::dom::Node>(child->nextSibling()))
  {
    uint16_t nt = child->nodeType();
    switch (nt)
    {
    case iface::dom::Node::ELEMENT_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(cel, child, dom::Element);
        RETURN_INTO_WSTRING(ns, cel->namespaceURI());
        RETURN_INTO_WSTRING(ln, cel->localName());
        if (ln == L"")
        {
          wchar_t* tmp = cel->nodeName();
          ln = tmp;
          free(tmp);
        }

        if (ns == ((mCellMLVersion == kCellML_1_0) ?
                   CELLML_1_1_NS : CELLML_1_0_NS))
        {
          REPR_ERROR(L"It is not valid to mix the CellML 1.0 and 1.1 namespaces "
                     L"in the same model document", cel);
          ns = (mCellMLVersion == kCellML_1_0) ? CELLML_1_0_NS : CELLML_1_1_NS;
        }

        for (e = aSpec.children, i = 0; *e != NULL; e++, i++)
        {
          if (ln != (*e)->name)
            continue;

          const wchar_t* match_ns = (*e)->namespace_name;
          // Pointer comparison deliberate...
          if (match_ns == MATCHING_CELLML_NS)
            match_ns = (mCellMLVersion == kCellML_1_0) ?
              CELLML_1_0_NS : CELLML_1_1_NS;

          if (ns != match_ns)
            continue;

          counts[i]++;
          // We only give the error once even if there are more than one too
          // many of the elements...
          if ((*e)->maxInParent != 0 &&
              counts[i] == (*e)->maxInParent + 1)
            REPR_ERROR((*e)->tooManyMessage, cel);

          validateElementRepresentation(cel, **e);
          break;
        }
        
        if (extraEl && *e == NULL)
        {
          if (ns == CELLML_1_0_NS || ns == CELLML_1_1_NS || ns == MATHML_NS ||
              ns == XLINK_NS)
          {
            std::wstring msg(L"Unexpected element ");
            msg += ln;
            msg += L" found - not valid here";
            REPR_ERROR(msg, cel);
          }
        }
        break;
      }

    case iface::dom::Node::TEXT_NODE:
    case iface::dom::Node::CDATA_SECTION_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tn, child, dom::Text);
        wchar_t* tmp = tn->data();
        textData += tmp;
        free(tmp);
      }
      break;

    case iface::dom::Node::ENTITY_REFERENCE_NODE:
      // XXX we should handle this better.
    default:
      continue;
    }
  }

  for (i = 0, e = aSpec.children; *e != NULL; e++, i++)
  {
    if (counts[i] < (*e)->minInParent)
    {
      REPR_ERROR((*e)->tooFewMessage, aEl);
    }
  }

  if (aSpec.textValidator)
    (this->*(aSpec.textValidator))(aEl, textData);
}

void
ModelValidation::whitespaceOnlyValidatorCellML
(
 iface::dom::Node* aContext,
 const std::wstring& aText
)
{
#define NON_WHITESPACE_CELLML_NAMESPACE \
  L"Per section 2.4.4 of the CellML specification, any characters that " \
  L"occur immediately within elements in the CellML namespace must be "\
  L"either space (#x20) characters, carriage returns (#xA), line feeds "\
  L"(#xD), or tabs (#x9)."

  std::wstring::const_iterator p;
  for (p = aText.begin(); p != aText.end(); p++)
  {
    wchar_t c = *p;
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
      continue;

    REPR_ERROR(NON_WHITESPACE_CELLML_NAMESPACE, aContext);

    // The general rule followed by VACSS is to return all errors, not just
    // the first one, but returning one error for every non-whitespace
    // character would probably be taking that too far...
    return;
  }
}

ModelValidation::ReprValidationElement::ElementValidationLevel
ModelValidation::validateRelationshipRef(iface::dom::Element* aRR)
{
  RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap,
                     aRR->attributes());
  uint32_t i, l = nnm->length();

  bool seenRelationship = false;
  bool seenName = false;
  bool seenEncapsulation = false;
  
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));
    RETURN_INTO_WSTRING(ln, n->localName());
    if (ln == L"")
    {
      wchar_t* str = n->nodeName();
      ln = str;
      free(str);
    }
    RETURN_INTO_WSTRING(ns, n->namespaceURI());

    if (ln == L"relationship")
    {
      if (seenRelationship)
      {
        REPR_ERROR(L"relationship_ref element has more than one relationship "
                   L"across several namespaces (section 6.4.1.1)", n);
      }
      seenRelationship = true;

      RETURN_INTO_WSTRING(val, n->nodeValue());
      if (ns == L"")
      {
        if (val == L"encapsulation")
          seenEncapsulation = true;
        else if (val != L"containment")
        {
          REPR_ERROR(L"The value of a relationship attribute in the CellML "
                     L"namespace must be \"containment\" or \"encapsulation\" "
                     L"(section 6.4.2.2)",
                     n);
        }
      }
      continue;
    }
    else if (ns != L"")
      continue;

    if (ln != L"name")
    {
      REPR_ERROR(ln + L" attribute not allowed here", n);
    }
    else
    {
      seenName = true;
      RETURN_INTO_WSTRING(val, n->nodeValue());
      validateCellMLIdentifier(n, val);
    }
  }

  if (!seenRelationship)
  {
    REPR_ERROR(L"relationship attribute is mandatory on relationship_ref"
               L" (section 6.4.1.1)", aRR);
  }
  if (seenEncapsulation && seenName)
  {
    REPR_ERROR(L"A name attribute must not be defined on a <relationship_ref>"
               L" element with a relationship attribute value of "
               L"\"encapsulation\" (section 6.4.2.4)", aRR);
  }

  // Disable check for extraneous attributes, we did it already.
  return ModelValidation::ReprValidationElement::EXTRANEOUS_ELEMENTS;
}

void
ModelValidation::validateCellMLIdentifier
(
 iface::dom::Node* aContext,
 const std::wstring& aIdent
)
{
#define IDENT_MUST_CONTAIN_LETTER \
  L"A valid CellML identifier must contain at least one letter (section 2.4.1)"
#define IDENT_MUST_NOT_START_NUMBER \
  L"A valid CellML identifier must not start with a number (section 2.4.1)"
#define IDENT_INVALID_CHARACTER \
  L"A valid CellML identifier must only contain alphanumeric characters " \
  L"from the US-ASCII character set and the underscore character " \
  L"(section 2.4.1)"

  std::wstring::const_iterator i = aIdent.begin();
  if (i == aIdent.end())
  {
    REPR_ERROR(IDENT_MUST_CONTAIN_LETTER, aContext);
    return;
  }

  wchar_t c = *i;

  // The 'can't start with a number' rule was introduced in CellML 1.1.
  if ((mCellMLVersion > kCellML_1_0) && (c >= '0' || c <= '9'))
  {
    REPR_ERROR(IDENT_MUST_NOT_START_NUMBER, aContext);
  }

  bool sawLetter = false;

  for (; i != aIdent.end(); i++)
  {
    c = *i;
    if ((c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z'))
      sawLetter = true;
    else if (!((c >= L'0' && c <= L'9') || c == L'_'))
    {
      REPR_ERROR(IDENT_INVALID_CHARACTER, aContext);
    }
  }

  if (!sawLetter)
  {
    REPR_ERROR(IDENT_MUST_CONTAIN_LETTER, aContext);
  }
}

void
ModelValidation::validateBaseUnits
(
 iface::dom::Node* aContext,
 const std::wstring& aBU
)
{
  if (aBU == L"yes" || aBU == L"no")
    return;

  REPR_ERROR(L"If present, the value of the base_units attribute MUST "
             L"be \"yes\" or \"no\" (section 5.4.1.3)", aContext);
}

void
ModelValidation::validateInterface
(
 iface::dom::Node* aContext,
 const std::wstring& aIntf
)
{
  if (aIntf == L"in" || aIntf == L"out" || aIntf == L"none")
    return;

  REPR_ERROR(L"If present, the value of the public_interface / "
             L"private_interface attribute MUST be \"in\", \"out\", or "
             L"\"none\" (section 3.4.3.4 / 3.4.3.5)", aContext);
}

void
ModelValidation::validateInitialValue
(
 iface::dom::Node* aContext,
 const std::wstring& aInitial
)
{
  if (mCellMLVersion == kCellML_1_0)
  {
    // In CellML 1.0, initial_value must be a real number...
    validateFloatingPoint(aContext, aInitial);
    return;
  }

  if (aInitial.length() == 0)
  {
    REPR_ERROR(L"If present, the value of the initial_value attribute MAY be "
               L"a real number or the value of the name attribute of a "
               L"<variable> element declared in the current component "
               "(section 3.4.3.7)", aContext);
    return;
  }

  wchar_t c = aInitial[0];
  if ((c >= L'0' && c <= L'9') || c == L'-' || c == L'.' || c == L'+')
    validateFloatingPoint(aContext, aInitial);
  else
    validateCellMLIdentifier(aContext, aInitial);
}

void
ModelValidation::validatePrefix
(
 iface::dom::Node* aContext,
 const std::wstring& aPrefix
)
{
  if (aPrefix.length() == 0)
  {
    REPR_ERROR(L"If present, the value of the prefix attribute MUST be an "
               L"integer or a name taken from one of the SI prefixes "
               L"(section 5.4.3.3)", aContext);
    return;
  }

  wchar_t c = aPrefix[0];
  if ((c >= L'0' && c <= L'9') || c == L'-' || c == L'.' || c == L'+')
  {
    validateFloatingPoint(aContext, aPrefix);
    return;
  }

  bool matched = false;
  if (c >= L'm')
  {
    if (c >= L't')
    {
      if (c == L'z')
        matched = (aPrefix == L"zepto" || aPrefix == L"zetta");
      else if (c == L'y')
        matched = (aPrefix == L"yocto" || aPrefix == L"yotta");
      else if (c == L't')
        matched = (aPrefix == L"tera");
    }
    else
    {
      if (c == L'p')
        matched = (aPrefix == L"peta" || aPrefix == L"pico");
      else if (c == L'n')
        matched = (aPrefix == L"nano");
      else if (c == L'm')
        matched = (aPrefix == L"mega" || aPrefix == L"micro" ||
                   aPrefix == L"milli");
    }
  }
  else
  {
    if (c >= L'f')
    {
      if (c == L'f')
        matched = (aPrefix == L"femto");
      else if (c == L'g')
        matched = (aPrefix == L"giga");
      else if (c == L'h')
        matched = (aPrefix == L"hecto");
      else if (c == L'k')
        matched = (aPrefix == L"kilo");
    }
    else
    {
      if (c == L'd')
        matched = (aPrefix == L"deci" || aPrefix == L"deka");
      else if (c == L'a')
        matched = (aPrefix == L"atto");
      else if (c == L'c')
        matched = (aPrefix == L"centi");
      else if (c == L'e')
        matched = (aPrefix == L"exa");
    }
  }

  if (!matched)
  {
    REPR_ERROR(L"If present, the value of the prefix attribute MUST be an "
               L"integer or a name taken from one of the SI prefixes "
               L"(section 5.4.3.3)", aContext);
  }
}

void
ModelValidation::validateFloatingPoint
(
 iface::dom::Node* aContext,
 const std::wstring& aFP
)
{
  const wchar_t* p = aFP.c_str();
  const wchar_t* e = p + aFP.length();

#define INVALID_REAL_NUMBER L"Expected a real number, but didn't get one in "\
  L"a valid format"

  if (p == e)
  {
    REPR_ERROR(INVALID_REAL_NUMBER, aContext);
    return;
  }

  if (*p == L'+' || *p == L'-')
  {
    p++;

    if (p == e)
    {
      REPR_ERROR(INVALID_REAL_NUMBER, aContext);
      return;
    }
  }

  bool seenDot = false, seenExp = false;
  while (p != e)
  {
    if (*p == L'.')
    {
      if (seenDot)
      {
        REPR_ERROR(INVALID_REAL_NUMBER, aContext);
        return;
      }
      seenDot = true;
      p++;
      continue;
    }

    if (*p >= L'0' && *p <= L'9')
    {
      p++;
      continue;
    }

    if (*p == L'E' || *p == L'e')
    {
      if (seenExp)
      {
        REPR_ERROR(INVALID_REAL_NUMBER, aContext);
        return;
      }
      seenExp = true;
      // No . after the exponent...
      seenDot = true;
      p++;

      if (p != e && (*p == L'-' || *p == L'+'))
        p++;

      continue;
    }

    REPR_ERROR(INVALID_REAL_NUMBER, aContext);
    return;
  }
}

void
ModelValidation::validateReversible
(
 iface::dom::Node* aContext,
 const std::wstring& aReversible
)
{
  if (aReversible == L"yes" || aReversible == L"no")
    return;

  REPR_ERROR(L"If present, the reversible attribute must have a value "
             L"of \"yes\" or \"no\" (section 7.4.1.2)", aContext);
}

void
ModelValidation::validateRole
(
 iface::dom::Node* aContext,
 const std::wstring& aRole
)
{
  wchar_t c = aRole[0];
  bool matched = false;
  if (c <= L'm')
  {
    if (c <= L'c')
    {
      if (c == L'a')
        matched = (aRole == L"activator");
      else if (c == L'c')
        matched = (aRole == L"catalyst");
    }
    else
    {
      if (c == L'i')
        matched = (aRole == L"inhibitor");
      else if (c == L'm')
        matched = (aRole == L"modifier");
    }
  }
  else
  {
    if (c == L'r')
      matched = (aRole == L"rate" || aRole == L"reactant");
    else if (c == L'p')
      matched = (aRole == L"product");
  }

  if (matched)
    return;

  REPR_ERROR(L"The role attribute must take one of the following seven values:"
             L" reactant, product, catalyst, activator, inhibitor, modifier, "
             L"rate (section 7.4.3.2)", aContext);
}

void
ModelValidation::validateDirection
(
 iface::dom::Node* aContext,
 const std::wstring& aDirection
)
{
  if (aDirection == L"forward" || aDirection == L"reverse" ||
      aDirection == L"both")
    return;

  REPR_ERROR(L"If present, the direction attribute must take one of the "
             L"following three values: forward, reverse, both"
             L" (section 7.4.3.4)", aContext);
}

ModelValidation::ReprValidationElement::ElementValidationLevel
ModelValidation::validateMaths(iface::dom::Element* aRR)
{
  return ModelValidation::ReprValidationElement::NOTHING_FURTHER;
}

void
ModelValidation::validateSemantics()
{
}

iface::cellml_services::CellMLValidityErrorSet*
CDA_VACSService::validateModel(iface::cellml_api::Model* aModel)
  throw()
{
  ModelValidation mv(aModel);
  return mv.validate();
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
  uint32_t aRow = 1;

  *aColumn = 1;

  RETURN_INTO_OBJREF(doc, iface::dom::Document, aNode->ownerDocument());
  advanceCursorThroughNodeUntil(doc, aRow, *aColumn, aNode, aNodalOffset);

  return aRow;
}

bool
CDA_VACSService::advanceCursorThroughNodeUntil
(
 iface::dom::Node* aNode,
 uint32_t& aRow,
 uint32_t& aCol,
 iface::dom::Node* aUntil,
 uint32_t aUntilOffset
)
{
  bool visitChildren = false;
  uint32_t trailingCharacters = 0;

  bool isUntilNode = (CDA_objcmp(aNode, aUntil) == 0);

  switch (aNode->nodeType())
  {
  case iface::dom::Node::ELEMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el, aNode, dom::Element);
      RETURN_INTO_WSTRING(name, el->nodeName());

      if (isUntilNode)
      {
        aCol += 1 + aUntilOffset;
        return true;
      }

      RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap,
                         el->attributes());
      uint32_t nnml = nnm->length();
      uint32_t i;
      for (i = 0; i < nnml; i++)
      {
        // XXX a single space between attributes is a canonical form, but we
        //     don't really know how it is actually laid out.
        if (i != 0)
          aCol++;
        RETURN_INTO_OBJREF(nn, iface::dom::Node, nnm->item(i));
        bool ret =
          advanceCursorThroughNodeUntil(nn, aRow, aCol, aUntil, aUntilOffset);
        if (ret)
          return true;
      }

      RETURN_INTO_OBJREF(elcnl, iface::dom::NodeList, el->childNodes());
      uint32_t elcnll = elcnl->length();

      if (elcnll)
      {
        aCol++; // >
        trailingCharacters = 3 + name.length(); // </name>
        visitChildren = true;
      }
      else
        aCol += 2; // />
    }
    break;
  case iface::dom::Node::ATTRIBUTE_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(attr, aNode, dom::Attr);

      RETURN_INTO_WSTRING(attrName, attr->name());

      // name="
      aCol += attrName.length() + 2;

      RETURN_INTO_WSTRING(attrValue, attr->nodeValue());
      
      advanceCursorThroughString(attrValue, aRow, aCol, isUntilNode,
                                 aUntilOffset);

      if (isUntilNode)
        return true;
      
      // Closing quote...
      aCol += 1;
    }
    break;
  case iface::dom::Node::TEXT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(txt, aNode, dom::Text);
      RETURN_INTO_WSTRING(value, txt->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::CDATA_SECTION_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(cds, aNode, dom::CDATASection);
      // [CDATA[
      aCol += 7;

      RETURN_INTO_WSTRING(value, cds->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::ENTITY_REFERENCE_NODE:
    {
      if (isUntilNode)
        return true;

      RETURN_INTO_WSTRING(value, aNode->nodeName());

      aCol += 1 + value.length();
    }
    break;
  case iface::dom::Node::ENTITY_NODE:
    {
      if (isUntilNode)
        return true;

      // XXX this could be better implemented.
      aCol = 1;
      aRow++;
    }
    break;
  case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(pi, aNode, dom::ProcessingInstruction);

      // <?
      aCol += 2;
      RETURN_INTO_WSTRING(targ, pi->target());
      aCol += targ.length() + 1;
      RETURN_INTO_WSTRING(value, pi->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::COMMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(comment, aNode, dom::Comment);
      // <!--
      aCol += 4;

      RETURN_INTO_WSTRING(value, comment->data());
      advanceCursorThroughString(value, aRow, aCol, isUntilNode,
                                 aUntilOffset);
      if (isUntilNode)
        return true;

      // -->
      aCol += 3;
    }
    break;
  case iface::dom::Node::DOCUMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(doc, aNode, dom::Document);

      if (isUntilNode)
        return true;

      aCol = 1;
      aRow++;

      visitChildren = true;
    }
    break;
  case iface::dom::Node::DOCUMENT_TYPE_NODE:
    {
      // XXX TODO...
      // DECLARE_QUERY_INTERFACE_OBJREF(doctype, aNode, dom::DocumentType);

      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::DOCUMENT_FRAGMENT_NODE:
    {
      DECLARE_QUERY_INTERFACE_OBJREF(docfrag, aNode, dom::DocumentFragment);
      visitChildren = true;

      if (isUntilNode)
        return true;
    }
    break;
  case iface::dom::Node::NOTATION_NODE:
    {
      // XXX TODO...
      // DECLARE_QUERY_INTERFACE_OBJREF(notat, aNode, dom::Notation);

      if (isUntilNode)
        return true;
    }
    break;
  }

  if (visitChildren)
  {
    ObjRef<iface::dom::Node> child;
    for (child = already_AddRefd<iface::dom::Node>(aNode->firstChild());
         child != NULL;
         child = already_AddRefd<iface::dom::Node>(child->nextSibling()))
    {
      bool ret = advanceCursorThroughNodeUntil(child, aRow, aCol, aUntil,
                                               aUntilOffset);
      if (ret)
        return true;
    }
  }

  aCol += trailingCharacters;

  return false;
}

void
CDA_VACSService::advanceCursorThroughString
(
 const std::wstring& aStr,
 uint32_t& aRow,
 uint32_t& aCol,
 bool aStopHere,
 uint32_t aStopOffset
)
{
  uint32_t i;
  for (i = 0; i < aStr.length(); i++)
  {
    if (aStopHere && aStopOffset-- == 0)
        return;

    wchar_t c = aStr[i];
    switch (c)
    {
    case L'\r':
      // Ignore \r, only consider \n...
      continue;
    case L'\n':
      aCol = 1;
      aRow++;
      continue;
    case L'<':
      aCol += 4; // &lt;
      continue;
    case L'"':
      aCol += 6; // &quot;
      continue;
    default:
      aCol++;
      continue;
    }
  }
}

iface::cellml_services::VACSService*
CreateVACSService(void)
{
  return new CDA_VACSService();
}
