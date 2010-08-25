#define IN_VACSS_MODULE
#include "VACSSImpl.hpp"
#include "VACSSBootstrap.hpp"
#include "CUSESBootstrap.hpp"
#include <cstdio>
#include <string>
#include <set>
#include <assert.h>
#include <iterator>

/* The code special cases this to match the CellML namespace
 * corresponding to the appropriate CellML version...
 * This has to be a static const for MSVC compatibility (because we do pointer
 * comparisons, and it seems MSVC doesn't always coalesce matching constant
 * strings to a single address).
 */
static const wchar_t* MATCHING_CELLML_NS = L"http://www.cellml.org/cellml/";
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

const ModelValidation::ApplyOperatorInformation
ModelValidation::mApplyOperators[] =
{
  {L"quotient",   AI_IGNORE,         AR_CUSTOM,        AA_BINARY},
  {L"factorial",  AI_MATCH,          AR_DIMENSIONLESS, AA_UNARY},
  {L"divide",     AI_IGNORE,         AR_CUSTOM,        AA_BINARY},
  {L"max",        AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"min",        AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"minus",      AI_MATCH,          AR_INPUT,         AA_BINARY},
  {L"plus",       AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"power",      AI_IGNORE,         AR_CUSTOM,        AA_BINARY},
  {L"rem",        AI_MATCH,          AR_INPUT,         AA_BINARY},
  {L"times",      AI_IGNORE,         AR_CUSTOM,        AA_NARY},
  {L"root",       AI_MATCH,          AR_CUSTOM,        AA_UNARY},
  {L"gcd",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_NARY},
  {L"and",        AI_BOOLEAN,        AR_BOOLEAN,       AA_NARY},
  {L"or",         AI_BOOLEAN,        AR_BOOLEAN,       AA_NARY},
  {L"xor",        AI_BOOLEAN,        AR_BOOLEAN,       AA_NARY},
  {L"not",        AI_BOOLEAN,        AR_BOOLEAN,       AA_UNARY},
  {L"implies",    AI_BOOLEAN,        AR_BOOLEAN,       AA_BINARY},
  {L"abs",        AI_MATCH,          AR_INPUT,         AA_UNARY},
  {L"lcm",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_NARY},
  {L"floor",      AI_MATCH,          AR_INPUT,         AA_UNARY},
  {L"ceiling",    AI_MATCH,          AR_INPUT,         AA_UNARY},
  {L"eq",         AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"neq",        AI_MATCH,          AR_BOOLEAN,       AA_BINARY},
  {L"gt",         AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"lt",         AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"geq",        AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"leq",        AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"equivalent", AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"approx",     AI_MATCH,          AR_BOOLEAN,       AA_NARY},
  {L"factorof",   AI_MATCH,          AR_BOOLEAN,       AA_BINARY},
  {L"exp",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"ln",         AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"log",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"mean",       AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"sdev",       AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"variance",   AI_MATCH,          AR_INPUT_SQ,      AA_NARY},
  {L"median",     AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"mode",       AI_MATCH,          AR_INPUT,         AA_NARY},
  {L"sin",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"cos",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"tan",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"sec",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"csc",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"cot",        AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"sinh",       AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"cosh",       AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"tanh",       AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"sech",       AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"csch",       AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"coth",       AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arcsin",     AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arccos",     AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arctan",     AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arccosh",    AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arccot",     AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arccoth",    AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arccsc",     AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arccsch",    AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arcsec",     AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arcsech",    AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arcsinh",    AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY},
  {L"arctanh",    AI_DIMENSIONLESS,  AR_DIMENSIONLESS, AA_UNARY}
};

ModelValidation::ModelValidation(iface::cellml_api::Model* aModel)
  : mModel(aModel)
{
  const wchar_t* reservedUnits[] =
    {
      L"ampere",  L"farad",         L"katal",     L"lux",
      L"pascal",  L"tesla",         L"becquerel", L"gram",
      L"kelvin",  L"meter",         L"radian",    L"volt",
      L"candela", L"gray",          L"kilogram",  L"metre",
      L"second",  L"watt",          L"celsius",   L"henry",
      L"liter",   L"mole",          L"siemens",   L"weber",
      L"coulomb", L"hertz",         L"litre",     L"newton",
      L"sievert", L"dimensionless", L"joule",     L"lumen",
      L"ohm",     L"steradian"
    };

  for (uint32_t i = 0; i < (sizeof(reservedUnits) / sizeof(reservedUnits[0]));
       i++)
  {
    mReservedUnits.insert(reservedUnits[i]);
  }

  for (uint32_t i = 0; i < (sizeof(mApplyOperators) / sizeof(mApplyOperators[0]));
       i++)
  {
    mApplyOperatorMap[mApplyOperators[i].mName] = mApplyOperators + i;
  }
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
                     L"reported as valid",
                     mModel);

  RETURN_INTO_OBJREF(cb, iface::cellml_services::CUSESBootstrap,
                     CreateCUSESBootstrap());
  mStrictCUSES = already_AddRefd<iface::cellml_services::CUSES>
    (cb->createCUSESForModel(mModel, true));

  mBooleanUnits =
    already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
    (mStrictCUSES->createEmptyUnits());
  mDimensionlessUnits =
    already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
    (mStrictCUSES->createEmptyUnits());

  RETURN_INTO_WSTRING(me, mStrictCUSES->modelError());
  if (me != L"")
  {
    SEMANTIC_ERROR(me, mModel);
    SEMANTIC_WARNING(L"Cannot perform any further checking of unit names due "
                     L"to problems processing the model units", mModel);
    mStrictCUSES = NULL;
  }

  try
  {
    validateSemantics();
  }
  catch (...)
  {
  }

  mSeenInVars.clear();
  mConnectedComps.clear();

  mStrictCUSES = NULL;

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
  __FILE__ L"): this should never happen, please report a bug"
#ifdef _MSC_VER
#define INTERNAL_ERROR_MESSAGE L"Internal error: this should never happen, please report a bug"
#else
#define INTERNAL_ERROR_MESSAGE INTERNAL_ERROR_MESSAGE_M(__LINE__)
#endif

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
    /* customValidator */&ModelValidation::nullValidator
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
                        L"units attribute (section 3.4.3.1 / 5.4.3.1)",
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
               L"validate against could not be determined",
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
               L"further errors can be displayed because of this",
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
    msg += L" is invalid in this version of CellML";
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
          else
            validateExtensionElement(cel);
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
ModelValidation::validateExtensionElement(iface::dom::Element* aEl)
{
  RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap,
                     aEl->attributes());
  uint32_t i, l = nnm->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(atn, iface::dom::Node, nnm->item(i));
    RETURN_INTO_WSTRING(ns, atn->namespaceURI());
    if (ns == CELLML_1_1_NS || ns == CELLML_1_0_NS || ns == MATHML_NS)
    {
      RETURN_INTO_WSTRING(n, atn->localName());
      std::wstring msg = L"Attribute " + n + L" in namespace " + ns +
        L"is not allowed in extension elements";
      REPR_WARNING(msg, aEl);
    }
  }

  RETURN_INTO_OBJREF(elcnl, iface::dom::NodeList, aEl->childNodes());
  l = elcnl->length();
  
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, elcnl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;

    RETURN_INTO_WSTRING(ns, el->namespaceURI());
    if (ns == CELLML_1_1_NS || ns == CELLML_1_0_NS || ns == MATHML_NS)
    {
      RETURN_INTO_WSTRING(n, el->tagName());
      std::wstring msg = L"Element " + n + L" in namespace " + ns +
        L"is not allowed in extension elements";
      REPR_WARNING(msg, aEl);
    }
  }
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
  L"(#xD), or tabs (#x9)"

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
  if ((mCellMLVersion > kCellML_1_0) && (c >= '0' && c <= '9'))
  {
    std::wstring msg(IDENT_MUST_NOT_START_NUMBER);
    msg += L": ";
    msg += aIdent;
    REPR_ERROR(msg, aContext);
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
               L"(section 3.4.3.7)", aContext);
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
ModelValidation::nullValidator(iface::dom::Element* aRR)
{
  return ModelValidation::ReprValidationElement::NOTHING_FURTHER;
}

void
ModelValidation::validateMaths(iface::cellml_api::CellMLElement* aContext,
                               iface::dom::Element* aRR)
{
  // Much of this validation is of conventions which have been established but
  // which are not adequately described in the specification. This is
  // unfortunate...

  RETURN_INTO_OBJREF(cn, iface::dom::NodeList, aRR->childNodes());
  uint32_t l = cn->length();
  for (uint32_t i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, cn->item(i));

    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
      if (tn != NULL)
      {
        RETURN_INTO_WSTRING(d, tn->data());
        for (std::wstring::const_iterator j = d.begin(); j != d.end(); j++)
        {
          if ((*j) == ' ' || (*j) == '\n' || (*j) == '\r' || (*j) == '\t')
            continue;
          REPR_ERROR(L"MathML math elements cannot contain text nodes", tn);
          break;
        }
      }
      continue;
    }

    RETURN_INTO_WSTRING(ns, el->namespaceURI());
    if (ns != MATHML_NS)
    {
      REPR_ERROR(L"Non-MathML element inside top-level MathML math element", el);
      continue;
    }

    RETURN_INTO_WSTRING(ln, el->localName());
    if (ln == L"semantics")
    {
      el = already_AddRefd<iface::dom::Element>
        (extractSemanticsValidateAnnotation(el));
      if (el == NULL)
        continue;

      wchar_t *tmp = el->localName();
      ln = tmp;
      free(tmp);
    }

    if (ln != L"apply")
    {
      REPR_ERROR(L"Expected apply element inside MathML math element",
                 el);
      continue;
    }

    // Check the operator...
    DECLARE_QUERY_INTERFACE_OBJREF(mae, el, mathml_dom::MathMLApplyElement);

    ObjRef<iface::mathml_dom::MathMLElement> op;
    try
    {
      op = already_AddRefd<iface::mathml_dom::MathMLElement>(mae->_cxx_operator());
    }
    catch (...)
    {
      REPR_ERROR(L"Missing MathML operator on apply inside MathML math element",
                 mae);
      continue;
    }
    
    RETURN_INTO_WSTRING(oln, op->localName());
    if (oln != L"eq")
    {
      REPR_ERROR(L"Expected MathML operator on apply inside MathML math "
                 L"element to be eq(equals)",
                 op);
      continue;
    }

    uint32_t m = mae->nArguments();
    if (m < 3)
    {
      REPR_ERROR(L"Expected apply inside MathML math element to be equate at "
                 L"least two expressions",
                 mae);
      continue;
    }

    RETURN_INTO_OBJREF(a, iface::mathml_dom::MathMLElement, mae->getArgument(2));
    RETURN_INTO_OBJREF(aUnits,
                       iface::cellml_services::CanonicalUnitRepresentation,
                       validateMathMLExpression(aContext, a));
    for (uint32_t k = 3; k < (m + 1); k++)
    {
      RETURN_INTO_OBJREF(b, iface::mathml_dom::MathMLElement, mae->getArgument(k));
      RETURN_INTO_OBJREF(bUnits,
                         iface::cellml_services::CanonicalUnitRepresentation,
                         validateMathMLExpression(aContext, b));

      if (
          aUnits && bUnits &&
          (!aUnits->compatibleWith(bUnits) ||
           ((!CDA_objcmp(aUnits, mBooleanUnits)) !=
            (!CDA_objcmp(bUnits, mBooleanUnits))))
         )
      {
        REPR_WARNING(L"MathML equals element has inconsistent units between "
                     L"the sides", op);
      }
    }
  }
}

iface::mathml_dom::MathMLElement*
ModelValidation::extractSemanticsValidateAnnotation(iface::dom::Element* aEl)
{
  ObjRef<iface::dom::Element> result;

  checkMathMLAttributes(aEl, true, true);
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList, aEl->childNodes());
  uint32_t n = nl->length();
  for (uint32_t i = 0; i < n; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));

    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
      if (tn != NULL)
      {
        REPR_ERROR(L"Text should not be present directly inside a MathML "
                   L"semantics element", tn);
      }
      continue;
    }

    // See if the element is in the MathML namespace...
    RETURN_INTO_WSTRING(ns, el->namespaceURI());
    if (ns != MATHML_NS)
    {
      REPR_ERROR(L"Non-MathML elements are not allowed as children of the "
                 L"MathML semantics element", el);
      continue;
    }

    RETURN_INTO_WSTRING(ln, el->localName());
    if (ln == L"annotation-xml" || ln == L"annotation")
    {
      checkMathMLAttributes(el, false, true);
      // Can we perhaps check that they are XML or non-XML?
      continue;
    }

    if (result != NULL)
    {
      REPR_ERROR(L"More than one element child other than an annotation-xml or "
                 L"annotation child inside a semantics element", el);
      continue;
    }

    result = el;
  }

  iface::mathml_dom::MathMLElement* r = NULL;
  if (result == NULL)
  {
    REPR_ERROR(L"No MathML element to which the semantics are being applied "
               L"inside MathML semantics element", aEl);
  }
  else
  {
    QUERY_INTERFACE(r, result, mathml_dom::MathMLElement);
    if (r == NULL)
    {
      REPR_ERROR(L"Expected a MathML element as the child of the semantics "
                 L"element", result);
    }
  }

  return r;
}

iface::cellml_services::CanonicalUnitRepresentation*
ModelValidation::validateMathMLExpression
(
 iface::cellml_api::CellMLElement* aContext,
 iface::dom::Element* aEl
)
{
  DECLARE_QUERY_INTERFACE_OBJREF(el, aEl, mathml_dom::MathMLElement);
  if (el == NULL)
  {
    REPR_ERROR(L"Found a non-MathML element where a MathML expression was "
               L"expected", aEl);
    return NULL;
  }

  // Get the element name...
  RETURN_INTO_WSTRING(ln, el->localName());
  
  if (ln == L"semantics")
  {
    // If it is a semantics element, extract the annotated MathML and continue...
    el = already_AddRefd<iface::mathml_dom::MathMLElement>
      (extractSemanticsValidateAnnotation(el));
    wchar_t* tmp = el->localName();
    ln = tmp;
    free(tmp);
  }

  if (el == NULL)
    return NULL;

  // Eliminate some of the possibilities which don't make much sense in
  // CellML 1.0/1.1...
  if (ln == L"integers" || ln == L"reals" || ln == L"rationals" ||
      ln == L"naturalnumbers" || ln == L"complexes" || ln == L"primes" ||
      ln == L"primes" || ln == L"emptyset" || ln == L"imaginaryi" ||
      ln == L"interval" || ln == L"set" || ln == L"list" || ln == L"matrix" ||
      ln == L"vector" || ln == L"lambda")
  {
    REPR_WARNING(L"Only real number types can be used in this version of "
                 L"CellML", aEl);
    return NULL;
  }

  // Handle remaining constants...
  bool isBoolean = false;
  if (ln == L"true" || ln == L"false")
    isBoolean = true;

  if (isBoolean || ln == L"exponentiale" || ln == L"notanumber" ||
      ln == L"pi" || ln == L"eulergamma" || ln == L"infinity")
  {
    checkMathMLElementEmpty(el, ln);
    iface::cellml_services::CanonicalUnitRepresentation* cur
      = isBoolean ? mBooleanUnits : mDimensionlessUnits;
    cur->add_ref();
    return cur;
  }

  if (ln == L"fn" || ln == L"reln")
  {
    REPR_WARNING(L"MathML 1.0 element " + ln + L" is deprecated in MathML "
                 L"2.0 and should not be used in CellML models", el);
    ln = L"apply";
  }
  else if (ln == L"csymbol")
  {
    REPR_WARNING(L"Validation of csymbol element not supported - did you "
                 L"mean ci?",
                 el);
    return NULL;
  }
  else if (ln == L"cn")
    return validateMathMLConstant(aContext, el);
  else if (ln == L"ci")
    return validateMathMLCI(aContext, el);
  else if (ln == L"piecewise")
    return validateMathMLPiecewise(aContext, el);

  if (ln == L"apply")
    return validateMathMLApply(aContext, el);

  REPR_ERROR(L"MathML element " + ln + L" was not expected in this context",
             el);

  return NULL;
}

iface::cellml_services::CanonicalUnitRepresentation*
ModelValidation::validateChildMathMLExpression
(
 iface::cellml_api::CellMLElement* aContext,
 iface::dom::Element* aEl
)
{
  ObjRef<iface::cellml_services::CanonicalUnitRepresentation> ret;

  ObjRef<iface::dom::Node> n(already_AddRefd<iface::dom::Node>(aEl->firstChild()));
  bool seenChild = false;
  for (; n; n = already_AddRefd<iface::dom::Node>(n->nextSibling()))
  {
    uint16_t nt = n->nodeType();
    switch (nt)
    {
    case iface::dom::Node::ELEMENT_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(cel, n, dom::Element);
        RETURN_INTO_WSTRING(ns, cel->namespaceURI());

        if (ns != MATHML_NS)
        {
          REPR_ERROR(L"Unexpected non-MathML content inside MathML element", n);
          continue;
        }

        if (seenChild)
        {
          REPR_ERROR(L"Only expected one child element inside MathML element", n);
          continue;
        }

        seenChild = true;
        ret = already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
          (validateMathMLExpression(aContext, cel));
      }
      break;
    case iface::dom::Node::TEXT_NODE:
    case iface::dom::Node::CDATA_SECTION_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
        RETURN_INTO_WSTRING(d, tn->data());
        for (std::wstring::const_iterator p = d.begin(); p != d.end(); p++)
        {
          if ((*p) != L' ' && (*p) != L'\n' && (*p) != L'\r' && (*p) != L'\t')
          {
            REPR_ERROR(L"MathML element shouldn't contain text",
                       n);
            break;
          }
        }
      }
      break;

    case iface::dom::Node::ENTITY_REFERENCE_NODE:
    case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
      REPR_ERROR(L"Unexpected content inside MathML element",
                 n);
      break;

    default:
      ;
    }
  }

  if (!seenChild)
  {
    REPR_ERROR(L"Expected a MathML child element inside MathML element", n);
    return NULL;
  }

  if (ret != NULL)
    ret->add_ref();

  return ret;
}

iface::cellml_services::CanonicalUnitRepresentation*
ModelValidation::validateMathMLConstant
(
 iface::cellml_api::CellMLElement* aContext,
 iface::mathml_dom::MathMLElement* aEl
)
{
  // Check for any inappropriate children...
  RETURN_INTO_OBJREF(child, iface::dom::Node, aEl->firstChild());
  std::wstring txt, name;
  RETURN_INTO_WSTRING(type, aEl->getAttributeNS(L"", L"type"));
  for (; child != NULL;
       child = already_AddRefd<iface::dom::Node>(child->nextSibling()))
  {
    uint16_t nt = child->nodeType();
    switch (nt)
    {
    case iface::dom::Node::ELEMENT_NODE:
      name = child->nodeName();
      if (type != L"e-notation")
        REPR_ERROR(L"MathML cn elements without the attribute type=e-notation "
                   L"shouldn't have element children", aEl);
      else if (name != L"sep")
        REPR_ERROR(L"MathML cn elements with the attribute type=e-notation "
                   L"shouldn't have non sep element children", aEl);
      else
        txt += L"<sep/>";
      break;
    case iface::dom::Node::TEXT_NODE:
    case iface::dom::Node::CDATA_SECTION_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tn, child, dom::Text);
        wchar_t* tmp = tn->data();
        txt += tmp;
        free(tmp);
      }
      break;
    default:
      ;
    }
  }

  RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, aEl->attributes());
  for (uint32_t i = 0, l = nnm->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));

    RETURN_INTO_WSTRING(ns, n->namespaceURI());
    if (ns == ((mCellMLVersion == kCellML_1_0) ?
               CELLML_1_1_NS : CELLML_1_0_NS))
    {
      REPR_ERROR(L"It is not valid to mix the CellML 1.0 and 1.1 namespaces "
                 L"in the same model document", n);
    }

    RETURN_INTO_WSTRING(ln, n->localName());

    if (ns == CELLML_1_1_NS || ns == CELLML_1_0_NS)
    {
      if (ln == L"units")
        continue;
      
      REPR_ERROR(L"Invalid attribute in the CellML namespace found on "
                 L"MathML cn element", n);
      continue;
    }
    else if (ns != MATHML_NS)
      continue;

    RETURN_INTO_WSTRING(v, n->nodeValue());
    if (ln == L"base")
    {
      if (v != L"10")
      {
        REPR_WARNING(L"Base other than 10 encountered; validation may "
                     L"give incorrect results", n);
      }
    }
    else if (ln == L"type")
    {
      if (v != L"real")
      {
        REPR_ERROR(L"This version of CellML doesn't support non-real numbers "
                   L"but type other than real found on constant", n);
      }
    }
    else
    {
      REPR_ERROR(L"Invalid MathML attribute " + ln + L" encountered",
                 n);
    }
  }

  // Now we have to check if textData is a valid real number...
  // But first, we need to trim any space, tab, etc. from textData
  // Note: if textData equals "   123   ", then it will become equal to "123".
  //       However, if textData equals "   1   <sep/>   2   ", then it will
  //       become equal to "1   <sep/>   2", so we will need to do some
  //       further trimming later on in case of an e-notation cn element...

  int i = 0, j = txt.length() - 1;
  wchar_t c;
  while ((c = txt[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
    i++;
  while ((c = txt[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
    j--;
  if (j < i)
    j = i - 1;
  txt = txt.substr(i, j - i + 1);
  
  // Now, we need to see if the constant is valid. For now, we are treating it
  // as a real number in base 10...
  /*
   * A real number is presented in decimal notation. Decimal notation consists
   * of an optional sign ("+" or "-") followed by a string of digits possibly
   * separated into an integer and a fractional part by a "decimal point". Some
   * examples are 0.3, 1, and -31.56. If a different base is specified, then
   * the digits are interpreted as being digits computed to that base.
   * If the constant is of type="e-notation", it will consist of a real
   * number, followed by a separator of the form "<sep/>", followed by a
   * real number. For example, the number 4.37e-32 would be represented by
   * the string "4.37<sep/>-32".
   */
  std::wstring::const_iterator p(txt.begin());
  if (p == txt.end())
  {
    REPR_ERROR(L"MathML cn element doesn't contain a constant",
               aEl);
  }
  else
  {
    if ((*p) == L'-' || (*p) == L'+')
      p++;

    bool valid = true;

    do
    {
      if (p == txt.end())
      {
        valid = false;
        break;
      }

      if ((*p) < L'0' || (*p) > L'9')
      {
        valid = false;
        break;
      }

      p++;

      while (p != txt.end() && ((*p) >= L'0' && (*p) <= L'9'))
        p++;

      if (p == txt.end())
        break;

      if ((*p) == L'.')
      {
        p++;

        while (p != txt.end() && ((*p) >= L'0' && (*p) <= L'9'))
          p++;

        if (p == txt.end())
          break;
      }

      // At this stage, textData might look something like "   <sep/>   2", so
      // we need to trim it on the left before going any further

      while ((*p) == L' ' || (*p) == L'\t' || (*p) == L'\r' || (*p) == L'\n')
        p++;

      if ((*p) == L'<' && type == L"e-notation")
      {
        std::wstring::const_iterator::difference_type charsleft(0);
        charsleft = std::distance(p, std::wstring::const_iterator(txt.end()));

        if ((charsleft > 6) && (p[1] == L's') && (p[2] == L'e') && (p[3] == L'p')
            && (p[4] == L'/') && (p[5] == L'>'))
        {
          p += 6;

          // At this stage, textData might look something like "   2", so
          // we need to trim it on the left before going any further

          while ((*p) == L' ' || (*p) == L'\t' || (*p) == L'\r' || (*p) == L'\n')
            p++;

          if (p != txt.end() && ((*p) == L'-' || (*p) == L'+'))
            p++;

          if (p == txt.end())
          {
            valid = false;
            break;
          }
        }
        else
        {
          valid = false;
          break;
        }
      }
      else
      {
        valid = false;
        break;
      }

      while (p != txt.end() && ((*p) >= L'0' && (*p) <= L'9'))
        p++;

      if (p == txt.end())
        break;

      if ((*p) == L'.')
      {
        p++;

        while (p != txt.end() && ((*p) >= L'0' && (*p) <= L'9'))
          p++;

        if (p == txt.end())
          break;
      }

      if (p != txt.end())
        valid = false;
    }
    while (0);

    if (!valid)
    {
      REPR_ERROR(L"MathML cn element contains an invalid constant representation",
                 aEl);
    }
  }

  RETURN_INTO_WSTRING
  (
   units,
   aEl->getAttributeNS(((mCellMLVersion == kCellML_1_0) ? CELLML_1_0_NS :
                        CELLML_1_1_NS), L"units")
  );

  if (units == L"")
  {
    REPR_ERROR(L"MathML cn elements must have CellML units attribute",
               aEl);
    return NULL;
  }

  if (mStrictCUSES == NULL)
    return NULL;

  // Next, we go hunting for the units...
  RETURN_INTO_OBJREF(u, iface::cellml_services::CanonicalUnitRepresentation,
                     mStrictCUSES->getUnitsByName(aContext, units.c_str()));
  if (u == NULL)
  {
    REPR_ERROR(L"MathML cn element has invalid units",
               aEl);
  }
  else
    u->add_ref();

  return u;
}

iface::cellml_services::CanonicalUnitRepresentation*
ModelValidation::validateMathMLCI
(
 iface::cellml_api::CellMLElement* aContext,
 iface::mathml_dom::MathMLElement* aEl
)
{
  // Check for any inappropriate children...
  RETURN_INTO_OBJREF(child, iface::dom::Node, aEl->firstChild());
  std::wstring txt;
  for (; child != NULL;
       child = already_AddRefd<iface::dom::Node>(child->nextSibling()))
  {
    uint16_t nt = child->nodeType();
    switch (nt)
    {
    case iface::dom::Node::ELEMENT_NODE:
      REPR_ERROR(L"MathML ci elements shouldn't have element children", aEl);
      break;
    case iface::dom::Node::TEXT_NODE:
    case iface::dom::Node::CDATA_SECTION_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tn, child, dom::Text);
        wchar_t* tmp = tn->data();
        txt += tmp;
        free(tmp);
      }
      break;
    default:
      ;
    }
  }

  RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, aEl->attributes());
  for (uint32_t i = 0, l = nnm->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));

    RETURN_INTO_WSTRING(ns, n->namespaceURI());
    if (ns == ((mCellMLVersion == kCellML_1_0) ?
               CELLML_1_1_NS : CELLML_1_0_NS))
    {
      REPR_ERROR(L"It is not valid to mix the CellML 1.0 and 1.1 namespaces "
                 L"in the same model document", n);
    }

    RETURN_INTO_WSTRING(ln, n->localName());

    if (ns == CELLML_1_1_NS || ns == CELLML_1_0_NS)
    {
      REPR_ERROR(L"Invalid attribute in the CellML namespace found on "
                 L"MathML ci element", n);
      continue;
    }
    else if (ns != MATHML_NS)
      continue;

    RETURN_INTO_WSTRING(v, n->nodeValue());
    if (ln == L"type")
    {
      if (v != L"real")
      {
        REPR_ERROR(L"This version of CellML doesn't support non-real numbers "
                   L"but type other than real found on ci", n);
      }
    }
    else
    {
      REPR_ERROR(L"Invalid MathML attribute " + ln + L" encountered",
                 n);
    }
  }

  int i = 0, j = txt.length() - 1;
  wchar_t c;
  while ((c = txt[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
    i++;
  while ((c = txt[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
    j--;
  if (j < i)
    j = i - 1;
  txt = txt.substr(i, j - i + 1);

  // Find the component from our context...
  ObjRef<iface::cellml_api::CellMLElement> context(aContext);
  ObjRef<iface::cellml_api::CellMLComponent> comp;
  while (true)
  {
    QUERY_INTERFACE(comp, context, cellml_api::CellMLComponent);
    if (comp != NULL)
      break;

    context = already_AddRefd<iface::cellml_api::CellMLElement>
      (context->parentElement());
  }

  RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet,
                     comp->variables());
  RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable,
                     vs->getVariable(txt.c_str()));
  if (v == NULL)
  {
    REPR_ERROR(L"MathML ci element references variable which doesn't exist",
               aEl);
    return NULL;
  }
  RETURN_INTO_WSTRING(vunits, v->unitsName());
  
  if (mStrictCUSES == NULL)
    return NULL;

  return mStrictCUSES->getUnitsByName(comp, vunits.c_str());
}

iface::cellml_services::CanonicalUnitRepresentation*
ModelValidation::validateMathMLPiecewise
(
 iface::cellml_api::CellMLElement* aContext,
 iface::mathml_dom::MathMLElement* aEl
)
{
  ObjRef<iface::cellml_services::CanonicalUnitRepresentation> units;

  checkMathMLAttributes(aEl);

  ObjRef<iface::dom::Node> n;
  for (n = already_AddRefd<iface::dom::Node>(aEl->firstChild());
       n;
       n = already_AddRefd<iface::dom::Node>(n->nextSibling()))
  {
    switch (n->nodeType())
    {
    case iface::dom::Node::ELEMENT_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);

        // All elements should be in the MathML namespace...
        RETURN_INTO_WSTRING(ns, el->namespaceURI());
        if (ns != MATHML_NS)
        {
          REPR_ERROR(L"MathML piecewise elements shouldn't contain elements not "
                     L"in the MathML namespace",
                     el);
          continue;
        }

        RETURN_INTO_WSTRING(ln, el->localName());
        if (ln == L"piece")
        {
          checkMathMLAttributes(el);
          ObjRef<iface::dom::Node> n2;
          uint32_t pieceChildren = 0;
          for (n2 = already_AddRefd<iface::dom::Node>(el->firstChild());
               n2;
               n2 = already_AddRefd<iface::dom::Node>(n2->nextSibling()))
          {
            switch (n2->nodeType())
            {
            case iface::dom::Node::ELEMENT_NODE:
              {
                DECLARE_QUERY_INTERFACE_OBJREF(el2, n2, dom::Element);
                RETURN_INTO_WSTRING(ns2, el2->namespaceURI());
                if (ns2 != MATHML_NS)
                {
                  REPR_ERROR(L"MathML piece elements shouldn't contain elements not "
                             L"in the MathML namespace",
                             el2);
                  continue;
                }

                pieceChildren++;
                if (pieceChildren == 1)
                {
                  RETURN_INTO_OBJREF
                  (
                   pcu,
                   iface::cellml_services::CanonicalUnitRepresentation,
                   validateMathMLExpression(aContext, el2)
                  );

                  if (pcu == NULL)
                    continue;

                  if (units == NULL)
                    units = pcu;
                  else if (!units->compatibleWith(pcu) ||
                           ((!CDA_objcmp(units, mBooleanUnits)) !=
                            (!CDA_objcmp(pcu, mBooleanUnits))))
                  {
                    REPR_WARNING(L"Inconsistent units between pieces "
                                 L"of the piecewise equation", el2);
                  }
                }
                else if (pieceChildren == 2)
                {
                  RETURN_INTO_OBJREF
                  (
                   pcu,
                   iface::cellml_services::CanonicalUnitRepresentation,
                   validateMathMLExpression(aContext, el2)
                  );

                  if (pcu && CDA_objcmp(pcu, mBooleanUnits))
                  {
                    REPR_WARNING(L"Expected piece condition to have boolean type",
                                 el2);
                  }
                }
                else
                {
                  REPR_ERROR(L"MathML piece element should only have two "
                             L"element children", el2);
                  continue;
                }
              }
              break;

            case iface::dom::Node::TEXT_NODE:
            case iface::dom::Node::CDATA_SECTION_NODE:
              {
                DECLARE_QUERY_INTERFACE_OBJREF(tn, n2, dom::Text);
                RETURN_INTO_WSTRING(d, tn->data());
                for (std::wstring::const_iterator p = d.begin(); p != d.end(); p++)
                {
                  if ((*p) != L' ' && (*p) != L'\n' && (*p) != L'\r' && (*p) != L'\t')
                  {
                    REPR_ERROR(L"MathML piece element shouldn't contain text",
                               el);
                    break;
                  }
                }
              }
              break;
              
            case iface::dom::Node::ENTITY_REFERENCE_NODE:
            case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
              REPR_ERROR(L"Unexpected content inside piece element", n2);
              break;
              
            default:
              ;
            }
          }

          if (pieceChildren < 2)
          {
            REPR_ERROR(L"MathML piece element should have two "
                       L"element children", el);
          }
        }
        else if (ln == L"otherwise")
        {
          checkMathMLAttributes(el);

          RETURN_INTO_OBJREF
          (
           pcu,
           iface::cellml_services::CanonicalUnitRepresentation,
           validateChildMathMLExpression(aContext, el)
          );

          if (pcu != NULL)
          {
            if (units == NULL)
              units = pcu;
            else if (!units->compatibleWith(pcu) ||
                     ((!CDA_objcmp(units, mBooleanUnits)) !=
                      (!CDA_objcmp(pcu, mBooleanUnits))))
            {
              REPR_WARNING(L"Inconsistent units between pieces and otherwise "
                           L"of the piecewise equation", el);
            }
          }
        }
        else
        {
          REPR_ERROR(L"MathML piecewise element can only contain piece "
                     L"and otherwise elements", aEl);
        }
      }
      break;

    case iface::dom::Node::TEXT_NODE:
    case iface::dom::Node::CDATA_SECTION_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
        RETURN_INTO_WSTRING(d, tn->data());
        for (std::wstring::const_iterator p = d.begin(); p != d.end(); p++)
        {
          if ((*p) != L' ' && (*p) != L'\n' && (*p) != L'\r' && (*p) != L'\t')
          {
            REPR_ERROR(L"MathML piecewise element shouldn't contain text",
                       n);
            break;
          }
        }
      }
      break;

    case iface::dom::Node::ENTITY_REFERENCE_NODE:
    case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
      REPR_ERROR(L"Unexpected content inside piecewise element",
                 n);
      break;

    default:
      ;
    }
  }

  if (units != NULL)
    units->add_ref();
  return units;
}

template<class T>
class ContainerReleaser
{
public:
  ContainerReleaser(T& aContainer)
    : mContainer(aContainer)
  {
  }

  ~ContainerReleaser()
  {
    for (iterator i = mContainer.begin(); i != mContainer.end(); i++)
      (*i)->release_ref();
  }

private:
  T& mContainer;
  typedef typename T::iterator iterator;
};

bool
ModelValidation::findConstantValue
(
 iface::dom::Node* an,
 double& aValue,
 const std::wstring& aType
)
{
  DECLARE_QUERY_INTERFACE_OBJREF(el2, an, dom::Element);
  if (el2 == NULL)
    return false;
    
  RETURN_INTO_WSTRING(ns2, el2->namespaceURI());
  if (ns2 != MATHML_NS)
    return false;

  RETURN_INTO_WSTRING(ln2, el2->localName());
  if (ln2 == L"semantics")
  {
    el2 = already_AddRefd<iface::dom::Element>
      (extractSemanticsValidateAnnotation(el2));
    wchar_t* str = el2->localName();
    ln2 = str;
    free(str);
  }

  if (ln2 != L"cn")
  {
    REPR_WARNING((std::wstring(L"Units validation results may be incorrect "
                               L"because the ") + aType +
                  L" is not a constant").c_str(), el2);
    return false;
  }

  // This is safe because cn is non-null...
  DECLARE_QUERY_INTERFACE_OBJREF(cn, el2, mathml_dom::MathMLCnElement);
  RETURN_INTO_OBJREF(cna, iface::dom::Node, cn->getArgument(1));
    
  DECLARE_QUERY_INTERFACE_OBJREF(tn, cna, dom::Text);
  if (tn != NULL)
  {
    RETURN_INTO_WSTRING(tns, tn->data());
    int i = 0, j = tns.length() - 1;
    wchar_t c;
    while ((c = tns[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
      i++;
    while ((c = tns[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
      j--;
    if (j < i)
      j = i - 1;
    tns = tns.substr(i, j - i + 1);
    
    wchar_t* e;
    aValue = wcstod(tns.c_str(), &e);
    if (*e != 0 || aValue == 0)
    {
      REPR_WARNING((aType + L" should be integral").c_str(), el2);
      aValue = 1;
    }
  }

  return true;
}

bool
ModelValidation::findChildConstantValue
(
 iface::mathml_dom::MathMLElement* aEl,
 double& aValue,
 const std::wstring& aType
)
{
  ObjRef<iface::dom::Node> n2;
  for (n2 = already_AddRefd<iface::dom::Node>(aEl->firstChild());
       n2 != NULL;
       n2 = already_AddRefd<iface::dom::Node>(n2->nextSibling()))
  {
    if (findConstantValue(n2, aValue, aType))
      return true;
  }

  REPR_WARNING((std::wstring(L"Units validation results may be incorrect "
                             L"because the ") + aType +
                L" is not a constant").c_str(), aEl);
  return false;
}

iface::cellml_services::CanonicalUnitRepresentation*
ModelValidation::validateMathMLApply
(
 iface::cellml_api::CellMLElement* aContext,
 iface::mathml_dom::MathMLElement* aEl
)
{
  uint32_t nChildren = 0;

  std::list<iface::mathml_dom::MathMLElement*> elList;
  ContainerReleaser<std::list<iface::mathml_dom::MathMLElement*> > elListReleaser(elList);
  ObjRef<iface::mathml_dom::MathMLElement> op;
  ObjRef<iface::mathml_dom::MathMLElement> lowlimit, uplimit, bvar, degree,
    logbase, interval, condition, domainofapplication, momentabout;

  checkMathMLAttributes(aEl);
  ObjRef<iface::dom::Node> n;
  for (n = already_AddRefd<iface::dom::Node>(aEl->firstChild());
       n;
       n = already_AddRefd<iface::dom::Node>(n->nextSibling()))
  {
    switch (n->nodeType())
    {
    case iface::dom::Node::ELEMENT_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(mel, n, mathml_dom::MathMLElement);
        if (mel == NULL)
        {
          REPR_ERROR(L"MathML apply elements shouldn't contain elements not "
                     L"in the MathML namespace",
                     n);
          continue;
        }

        RETURN_INTO_WSTRING(ln, mel->localName());

#define QUAL(x) (ln == L## #x) \
        {\
          if (x != NULL) \
          { \
            REPR_ERROR(L"More than one " L## #x L" specified", mel); \
          } \
          x = mel; \
          continue; \
        }

        // Check for qualifiers and if we find them treat them specially...
        if QUAL(lowlimit) else if QUAL(uplimit) else if QUAL(bvar)
        else if QUAL(degree) else if QUAL(logbase) else if QUAL(interval)
        else if QUAL(condition) else if QUAL(domainofapplication) else if QUAL(momentabout);
        
        nChildren++;

        if (nChildren == 1)
          op = mel;
        else
        {
          mel->add_ref();
          elList.push_back(mel);
        }
      }
      break;

    case iface::dom::Node::TEXT_NODE:
    case iface::dom::Node::CDATA_SECTION_NODE:
      {
        DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
        RETURN_INTO_WSTRING(d, tn->data());
        for (std::wstring::const_iterator p = d.begin(); p != d.end(); p++)
        {
          if ((*p) != L' ' && (*p) != L'\n' && (*p) != L'\r' && (*p) != L'\t')
          {
            REPR_ERROR(L"MathML piecewise element shouldn't contain text",
                       n);
            break;
          }
        }
      }
      break;

    case iface::dom::Node::ENTITY_REFERENCE_NODE:
    case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
      REPR_ERROR(L"Unexpected content inside piecewise element",
                 n);
      break;

    default:
      ;
    }
  }

  if (op == NULL)
  {
    REPR_ERROR(L"MathML apply element is missing operator", aEl);
    return NULL;
  }

  checkMathMLAttributes(op, true, true);

  RETURN_INTO_WSTRING(ln, op->localName());
  if (ln == L"csymbol")
  {
    REPR_WARNING(L"Apply elements with csymbol operators are not validated",
                 op);
    return NULL;
  }

  // Get rid of anything not over real numbers straight away. This simplifies
  // qualifier handling... There is a bit of an inconsistency here in that we
  // keep diff and int by thinking of variables as being functions of time, but
  // then can't allow inverse, divergance, grad, curl, laplacian and the
  // like.
  if (
      ln == L"conjugate" || ln == L"arg" || ln == L"real" || ln == L"imaginary" ||
      ln == L"inverse" || ln == L"ident" || ln == L"domain" || ln == L"codomain" ||
      ln == L"image" || ln == L"determinant" || ln == L"transpose" ||
      ln == L"divergence" || ln == L"grad" || ln == L"curl" || ln == L"laplacian" ||
      ln == L"card" || ln == L"setdiff" || ln == L"vectorproduct" ||
      ln == L"scalarproduct" || ln == L"outerproduct" || ln == L"selector" ||
      ln == L"union" || ln == L"intersect" || ln == L"cartesianproduct" ||
      ln == L"selector" || ln == L"compose" || ln == L"sum" || ln == L"product" ||
      ln == L"forall" || ln == L"exists" || ln == L"moment" || ln == L"limit"
     )
  {
    REPR_ERROR(L"This version of CellML can only be used with real numbers",
               op);
    return NULL;
  }

  if (momentabout)
  {
    checkMathMLAttributes(momentabout);
    REPR_WARNING(L"Validation of momentabout qualifier is not supported "
                 L"because all operators which can legitimately take it are "
                 L"not valid in this version of CellML",
                 momentabout);
  }

  if (logbase)
  {
    checkMathMLAttributes(logbase);
    if (ln != L"log")
      REPR_ERROR(L"Only MathML log operators can have logbase qualifiers",
                 logbase);

    // Check logbase is dimensionless...
    RETURN_INTO_OBJREF(logbaseUnits,
                       iface::cellml_services::CanonicalUnitRepresentation,
                       validateChildMathMLExpression(aContext, logbase));
    if (logbaseUnits != NULL)
    {
      if (!logbaseUnits->compatibleWith(mDimensionlessUnits) ||
          (!CDA_objcmp(logbaseUnits, mBooleanUnits)))
      {
        REPR_WARNING(L"logbase qualifier should be in dimensionless units",
                     logbase);
      }
    }
  }

  double overallDegree = 1.0;

  if (degree)
  {
    checkMathMLAttributes(degree);
    if (ln != L"diff" && ln != L"partialdiff" && ln != L"root")
    {
      REPR_ERROR(L"Only MathML diff, partialdiff, moment and root operators can "
                 L"have degree qualifiers", degree);
    }

    RETURN_INTO_OBJREF(degreeUnits,
                       iface::cellml_services::CanonicalUnitRepresentation,
                       validateChildMathMLExpression(aContext, degree));
    if (degreeUnits)
    {
      if (!degreeUnits->compatibleWith(mDimensionlessUnits) ||
          (!CDA_objcmp(degreeUnits, mBooleanUnits)))
      {
        REPR_ERROR(L"degree qualifier should be in dimensionless units",
                   degree);
      }
    }

    if (degreeUnits && ln != L"root")
    {
      // To work out the units properly, we actually need a number for the
      // degree...
      findChildConstantValue(degree, overallDegree,
                             L"degree of differentiation");
    }
  }

  ObjRef<iface::cellml_services::CanonicalUnitRepresentation> bvarUnits;
  if (bvar)
  {
    checkMathMLAttributes(bvar);
    bool seenDegree = false, seenNotDegree = false;

    // Look for a degree child...
    ObjRef<iface::dom::Node> n2;
    for (n2 = already_AddRefd<iface::dom::Node>(bvar->firstChild());
         n2;
         n2 = already_AddRefd<iface::dom::Node>(n2->nextSibling()))
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el2, n2, dom::Element);
      if (el2 == NULL)
        continue;

      RETURN_INTO_WSTRING(ns2, el2->namespaceURI());
      if (ns2 != MATHML_NS)
        continue;

      RETURN_INTO_WSTRING(ln2, el2->localName());
      if (ln2 != L"degree")
      {
        if (seenNotDegree)
        {
          REPR_ERROR(L"There should only be one MathML expression per bvar",
                     el2);
          continue;
        }

        // We need it to be ci...
        if (ln2 != L"ci")
        {
          REPR_ERROR(L"Only variables (MathML ci elements) may be used with "
                     L"bvar", el2);
          continue;
        }

        bvarUnits =
          already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
          (validateMathMLExpression(aContext, el2));

        seenNotDegree = true;
        continue;
      }

      if (seenDegree)
      {
        REPR_ERROR(L"There should only be one degree qualifier per bvar",
                   el2);
        continue;
      }
      seenDegree = true;

      ObjRef<iface::cellml_services::CanonicalUnitRepresentation> degreeUnits;

      {
        ObjRef<iface::dom::Node> n3;
        for (n3 = already_AddRefd<iface::dom::Node>(el2->firstChild());
             n3;
             n3 = already_AddRefd<iface::dom::Node>(n3->nextSibling()))
        {
          DECLARE_QUERY_INTERFACE_OBJREF(el3, n3, mathml_dom::MathMLElement);
          if (el3 == NULL)
            continue;

          if (degreeUnits)
          {
            REPR_ERROR(L"There should only be one MathML element inside degree",
                       el3);
            continue;
          }
          degreeUnits =
            already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
            (validateMathMLExpression(aContext, el3));
        }

        if (degreeUnits == NULL)
        {
          REPR_ERROR(L"Degree element does not content MathML child",
                     el2);
          continue;
        }
      }

      if (!degreeUnits->compatibleWith(mDimensionlessUnits) ||
          (!CDA_objcmp(degreeUnits, mBooleanUnits)))
      {
        REPR_ERROR(L"degree qualifier should be in dimensionless units",
                   el2);
      }
      
      // To work out the units properly, we actually need a number for the
      // degree...
      ObjRef<iface::dom::Node> n3;
      for (n3 = already_AddRefd<iface::dom::Node>(el2->firstChild());
           n3;
           n3 = already_AddRefd<iface::dom::Node>(n3->nextSibling()))
      {
        DECLARE_QUERY_INTERFACE_OBJREF(el3, n3, dom::Element);
        if (el3 == NULL)
          continue;
        
        RETURN_INTO_WSTRING(ns3, el3->namespaceURI());
        if (ns3 != MATHML_NS)
          continue;
        
        RETURN_INTO_WSTRING(ln3, el3->localName());
        if (ln3 == L"semantics")
        {
          el3 = already_AddRefd<iface::dom::Element>
            (extractSemanticsValidateAnnotation(el3));
          wchar_t* str = el3->localName();
          ln3 = str;
          free(str);
        }

        if (ln3 != L"cn")
        {
          REPR_WARNING(L"Units validation results may be incorrect because the "
                       L"degree of differentiation is not a constant", el3);
          break;
        }

        // This is safe because degreeUnits is non-null...
        DECLARE_QUERY_INTERFACE_OBJREF(cn, el3, mathml_dom::MathMLCnElement);
        RETURN_INTO_OBJREF(cna, iface::dom::Node, cn->getArgument(1));
        
        DECLARE_QUERY_INTERFACE_OBJREF(tn, cna, dom::Text);
        if (tn != NULL)
        {
          RETURN_INTO_WSTRING(tns, tn->data());
          int i = 0, j = tns.length() - 1;
          wchar_t c;
          while ((c = tns[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
            i++;
          while ((c = tns[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
            j--;
          if (j < i)
            j = i - 1;
          tns = tns.substr(i, j - i + 1);

          wchar_t* e;
          overallDegree = wcstoul(tns.c_str(), &e, 10);
          if (*e != 0 || overallDegree == 0)
          {
            REPR_WARNING(L"Degree of differentiation should be integral", el2);
            overallDegree = 1;
          }
        }
      }
    }

    if (!seenNotDegree)
    {
      REPR_ERROR(L"MathML bvar element must contain bound variable",
                 bvar);
    }
  }

  // int is the only thing which supports a domain of application if we are not
  // allowing sets...
  if (ln == L"int")
  {
    if (interval || condition || domainofapplication)
    {
      REPR_WARNING(L"The only domain of application qualifiers currently "
                   L"validated for int are uplimit and lowlimit", op);
    }

    if (!bvar)
    {
      REPR_ERROR(L"Integral without bound variable (bvar) specified", op);
      return NULL;
    }

    if (!lowlimit)
    {
      REPR_WARNING(L"No lowlimit on MathML int application; if you intended "
                   L"to express an indefinite integral please be aware that "
                   L"all or most tools will not support this", op);
    }

    if (!uplimit)
    {
      REPR_WARNING(L"No uplimit on MathML int application; if you intended "
                   L"to express an indefinite integral please be aware that "
                   L"all or most tools will not support this", op);
    }

    if (lowlimit && bvarUnits)
    {
      checkMathMLAttributes(lowlimit);
      RETURN_INTO_OBJREF(lowlimitUnits,
                         iface::cellml_services::CanonicalUnitRepresentation,
                         validateChildMathMLExpression(aContext, lowlimit));

      if (!lowlimitUnits->compatibleWith(bvarUnits) ||
          ((!CDA_objcmp(lowlimitUnits, mBooleanUnits)) !=
           (!CDA_objcmp(bvarUnits, mBooleanUnits))))
      {
        REPR_ERROR(L"MathML int apply lowlimit units should match the bound "
                   L"variable units", lowlimit);
      }
    }

    if (uplimit && bvarUnits)
    {
      checkMathMLAttributes(uplimit);
      RETURN_INTO_OBJREF(uplimitUnits,
                         iface::cellml_services::CanonicalUnitRepresentation,
                         validateChildMathMLExpression(aContext, uplimit));

      if (!uplimitUnits->compatibleWith(bvarUnits) ||
          ((!CDA_objcmp(uplimitUnits, mBooleanUnits)) !=
           (!CDA_objcmp(bvarUnits, mBooleanUnits))))
      {
        REPR_ERROR(L"MathML int apply uplimit units should match the bound "
                   L"variable units", uplimit);
      }
    }

    if (elList.size() == 0)
    {
      REPR_ERROR(L"MathML int apply missing expression to integrate", aEl);
      return NULL;
    }
    if (elList.size() > 1)
    {
      REPR_ERROR(L"MathML int apply has more than one child", aEl);
    }

    RETURN_INTO_OBJREF(integrandUnits,
                       iface::cellml_services::CanonicalUnitRepresentation,
                       validateMathMLExpression(aContext, elList.front()));
    if (integrandUnits != NULL && bvarUnits != NULL)
      return integrandUnits->mergeWith(1, bvarUnits, -overallDegree);

    return NULL;
  }

  std::wstring doaElement;
  if (lowlimit)
    doaElement = L"lowlimit";
  else if (uplimit)
    doaElement = L"uplimit";
  else if (interval)
    doaElement = L"interval";
  else if (condition)
    doaElement = L"condition";
  else if (domainofapplication)
    doaElement = L"domainofapplication";
  
  if (doaElement != L"")
  {
    REPR_WARNING(L"Domain of "
                 L"application qualifier " + doaElement +
                 L" is not valid here", op);
  }

  if (ln == L"diff" || ln == L"partialdiff")
  {
    if (!bvar)
    {
      REPR_ERROR(L"Derivative without bound variable (bvar) specified", op);
      return NULL;
    }

    if (elList.size() == 0)
    {
      REPR_ERROR(L"MathML diff apply missing expression to differentiate", aEl);
      return NULL;
    }
    if (elList.size() > 1)
    {
      REPR_ERROR(L"MathML diff apply has more than one child", aEl);
    }

    RETURN_INTO_OBJREF(diffUnits,
                       iface::cellml_services::CanonicalUnitRepresentation,
                       validateMathMLExpression(aContext, elList.front()));
    if (diffUnits != NULL && bvarUnits != NULL)
      return diffUnits->mergeWith(1, bvarUnits, -overallDegree);

    return NULL;
  }

  std::list<iface::cellml_services::CanonicalUnitRepresentation*> unitsList;
  ContainerReleaser<std::list<iface::cellml_services::
                              CanonicalUnitRepresentation*> >
    unitsListReleaser(unitsList);

  for (
       std::list<iface::mathml_dom::MathMLElement*>::iterator i
         = elList.begin();
       i != elList.end();
       i++
      )
  {
    iface::cellml_services::CanonicalUnitRepresentation* cur
      = validateMathMLExpression(aContext, *i);
    if (cur == NULL)
      return NULL;

    unitsList.push_back(cur);
  }

  std::map<std::wstring, const ApplyOperatorInformation*>::iterator opit
    (mApplyOperatorMap.find(ln));
  if (opit == mApplyOperatorMap.end())
  {
    REPR_ERROR(L"Unrecognised MathML operator encountered", op);
    return NULL;
  }

  const ApplyOperatorInformation* opinfo = (*opit).second;

  // This simplifies the logic later...
  if (unitsList.empty())
  {
    REPR_ERROR(std::wstring(L"Expected \"") + opinfo->mName + L"\" operator to have at least one argument",
               op);
    return NULL;
  }

  if (opinfo->mInput == AI_MATCH)
  {
    if (!unitsList.empty())
    {
      std::list<iface::cellml_services::CanonicalUnitRepresentation*>::iterator
        ui(unitsList.begin());

      iface::cellml_services::CanonicalUnitRepresentation *firstUnits =
        *ui;
      for (ui++; ui != unitsList.end(); ui++)
      {
        iface::cellml_services::CanonicalUnitRepresentation *thisUnits =
          *ui;
        if (
            (!firstUnits->compatibleWith(thisUnits) ||
             ((!CDA_objcmp(firstUnits, mBooleanUnits)) !=
              (!CDA_objcmp(thisUnits, mBooleanUnits))))
           )
        {
          REPR_WARNING(std::wstring(L"Expected all arguments to operator \"") +
                       opinfo->mName + L"\" to have the same units", op);
        }
      }
    }
  }
  else if (opinfo->mInput == AI_DIMENSIONLESS)
  {
    std::list<iface::cellml_services::CanonicalUnitRepresentation*>::iterator
      ui(unitsList.begin());
    for (; ui != unitsList.end(); ui++)
      if (!(*ui)->compatibleWith(mDimensionlessUnits) ||
          !CDA_objcmp(*ui, mBooleanUnits))
      {
        REPR_WARNING(std::wstring(L"Expected arguments to operator \"") +
                     opinfo->mName +
                     L"\" to be dimensionless", op);
      }
  }
  else if (opinfo->mInput == AI_BOOLEAN)
  {
    std::list<iface::cellml_services::CanonicalUnitRepresentation*>::iterator
      ui(unitsList.begin());
    for (; ui != unitsList.end(); ui++)
      if (CDA_objcmp(*ui, mBooleanUnits))
      {
        REPR_WARNING(std::wstring(L"Expected arguments to operator \"") +
                     opinfo->mName +
                     L"\" to be boolean (i.e. true or false)", op);
      }
  }

  if (opinfo->mArity == AA_UNARY)
  {
    if (unitsList.size() != 1)
    {
      wchar_t buf[30];
      swprintf(buf, 30, L"%d", unitsList.size());
      REPR_ERROR(std::wstring(L"Operator \"") +
                 opinfo->mName + L"\" is unary (i.e. takes exactly one argument), but was given " + buf + L" arguments.",
                 op);
      return NULL;
    }
  }
  else if (opinfo->mArity == AA_BINARY)
  {
    if (unitsList.size() != 2)
    {
      if (ln == L"minus")
      {
        if (unitsList.size() != 1)
        {
          wchar_t buf[30];
          swprintf(buf, 30, L"%d", unitsList.size());
          REPR_ERROR(std::wstring(
                       L"Expected operator \"minus\" to be unary or binary (i.e. "
                       L"have one or two arguments), not ") + buf, op);
          return NULL;
        }
      }
      else
      {
        wchar_t buf[30];
        swprintf(buf, 30, L"%d", unitsList.size());
        REPR_ERROR(std::wstring(L"Expected operator \"") + opinfo->mName +
                   L"\" to be binary (i.e. have exactly "
                   L"two arguments), not " + buf, op);
        return NULL;
      }
    }
  }

  if (ln == L"power")
  {
    std::list<iface::cellml_services::CanonicalUnitRepresentation*>::iterator
      ui(unitsList.begin());
    
    iface::cellml_services::CanonicalUnitRepresentation* ubase = *ui++;
    iface::cellml_services::CanonicalUnitRepresentation* uexp = *ui++;

    if (!uexp->compatibleWith(mDimensionlessUnits) ||
        !CDA_objcmp(uexp, mBooleanUnits))
    {
      REPR_ERROR(L"Expected exponent to pow operator to be dimensionless", op);
    }

    double expVal;
    std::list<iface::mathml_dom::MathMLElement*>::iterator eli(elList.begin());
    eli++;

    if (!CDA_objcmp(mBooleanUnits, ubase))
    {
      REPR_WARNING(L"It is not valid to take a power of a boolean value", op);
    }

    // Don't require exponent to be constant if the base is dimensionless.
    if (mDimensionlessUnits->compatibleWith(ubase))
    {
      mDimensionlessUnits->add_ref();
      return mDimensionlessUnits;
    }

    if (!findConstantValue(*eli, expVal, L"exponent"))
      return NULL;

    return ubase->mergeWith(expVal, NULL, 0.0);
  }
  else if (ln == L"root")
  {
    iface::cellml_services::CanonicalUnitRepresentation* ubase = unitsList.front();

    double rootDegree = 2.0;

    if (degree != NULL)
    {
      if (!findChildConstantValue(degree, rootDegree, L"degree of root"))
        return NULL;
    }

    if (!CDA_objcmp(mBooleanUnits, ubase))
    {
      REPR_WARNING(L"It is not valid to take the root of a boolean value", op);
    }

    return ubase->mergeWith(1.0 / rootDegree, NULL, 0.0);
  }
  else if (ln == L"times")
  {
    ObjRef<iface::cellml_services::CanonicalUnitRepresentation> finalUnits
      (mDimensionlessUnits);

    for (std::list<iface::cellml_services::CanonicalUnitRepresentation*>
           ::iterator i(unitsList.begin());
         i != unitsList.end();
         i++)
    {
      if (!CDA_objcmp((*i), mBooleanUnits))
      {
        REPR_WARNING(L"It is not valid to multiply with a boolean value", op);
      }

      finalUnits = already_AddRefd<iface::cellml_services::CanonicalUnitRepresentation>
        (finalUnits->mergeWith(1.0, *i, 1.0));
    }

    finalUnits->add_ref();
    return finalUnits;
  }
  else if (ln == L"quotient" || ln == L"divide")
  {
    std::list<iface::cellml_services::CanonicalUnitRepresentation*>
      ::iterator ui(unitsList.begin());

    iface::cellml_services::CanonicalUnitRepresentation* udividend(*ui++),
      * udivisor(*ui);

    if (!CDA_objcmp(udividend, mBooleanUnits))
    {
      REPR_WARNING(L"It is not valid to divide a boolean value", op);
    }
    if (!CDA_objcmp(udivisor, mBooleanUnits))
    {
      REPR_WARNING(L"It is not valid to divide by a boolean value", op);
    }
    
    return udividend->mergeWith(1.0, udivisor, -1.0);
  }

  if (opinfo->mOutput == AR_INPUT)
  {
    iface::cellml_services::CanonicalUnitRepresentation* u = unitsList.front();
    u->add_ref();
    return u;
  }
  else if (opinfo->mOutput == AR_INPUT_SQ)
    return unitsList.front()->mergeWith(2.0, NULL, 0.0);
  else if (opinfo->mOutput == AR_DIMENSIONLESS)
  {
    mDimensionlessUnits->add_ref();
    return mDimensionlessUnits;
  }
  else if (opinfo->mOutput == AR_BOOLEAN)
  {
    mBooleanUnits->add_ref();
    return mBooleanUnits;
  }

  // Unreachable...
  assert(0);
  return NULL;
}

void
ModelValidation::checkMathMLElementEmpty
(
 iface::mathml_dom::MathMLElement* aEl,
 const std::wstring& aLocalname
)
{
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList, aEl->childNodes());
  uint32_t l(nl->length());
  for (uint32_t i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    uint32_t nt = n->nodeType();
    if (nt == iface::dom::Node::ELEMENT_NODE ||
        nt == iface::dom::Node::CDATA_SECTION_NODE ||
        nt == iface::dom::Node::ENTITY_REFERENCE_NODE ||
        nt == iface::dom::Node::PROCESSING_INSTRUCTION_NODE)
    {
      REPR_ERROR(L"Unexpected content inside " + aLocalname + L" element", n);
    }
    else if (nt == iface::dom::Node::TEXT_NODE)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
      RETURN_INTO_WSTRING(d, tn->data());
      for (std::wstring::const_iterator j = d.begin(); j != d.end(); j++)
      {
        if ((*j) == ' ' || (*j) == '\n' || (*j) == '\r' || (*j) == '\t')
          continue;

        REPR_ERROR(L"Unexpected content text inside " + aLocalname +
                   L" element", tn);
        break;
      }
    }
  }
}

void
ModelValidation::checkMathMLAttributes
(
 iface::dom::Element* aEl,
 bool aAllowDefinitionURL,
 bool aAllowEncoding
)
{
  RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, aEl->attributes());
  for (uint32_t i = 0, l = nnm->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));

    RETURN_INTO_WSTRING(ns, n->namespaceURI());
    if (ns == ((mCellMLVersion == kCellML_1_0) ?
               CELLML_1_1_NS : CELLML_1_0_NS))
    {
      REPR_ERROR(L"It is not valid to mix the CellML 1.0 and 1.1 namespaces "
                 L"in the same model document", n);
    }

    RETURN_INTO_WSTRING(ln, n->localName());

    if (ns == CELLML_1_1_NS || ns == CELLML_1_0_NS)
    {
      REPR_ERROR(L"Invalid attribute in the CellML namespace found on "
                 L"MathML element", n);
      continue;
    }
    else if (ns != MATHML_NS)
      continue;

    RETURN_INTO_WSTRING(v, n->nodeValue());
    if (aAllowDefinitionURL && ln == L"definitionURL")
      continue;
    else if (aAllowEncoding && ln == L"encoding")
      continue;
    else
    {
      REPR_ERROR(L"Invalid MathML attribute " + ln + L" encountered",
                 n);
    }
  }
}

void
ModelValidation::validateNameUniqueness(iface::cellml_api::Model* aModel)
{
  // Firstly, we will validate the uniqueness of component names...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                     aModel->modelComponents());
  RETURN_INTO_OBJREF(cci, iface::cellml_api::CellMLComponentIterator,
                     mc->iterateComponents());

  std::set<std::wstring> allNames;

  while (true)
  {
    RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                       cci->nextComponent());
    if (comp == NULL)
      break;

    RETURN_INTO_WSTRING(n, comp->name());

    if (allNames.count(n) != 0)
    {
      SEMANTIC_ERROR(L"More than one component in the model named " + n, comp);
    }
    allNames.insert(n);
  }

  allNames.clear();

  // Next, the uniqueness of all units names...
  RETURN_INTO_OBJREF(mu, iface::cellml_api::UnitsSet,
                     aModel->modelUnits());
  RETURN_INTO_OBJREF(cui, iface::cellml_api::UnitsIterator,
                     mu->iterateUnits());

  while (true)
  {
    RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                       cui->nextUnits());
    if (u == NULL)
      break;

    RETURN_INTO_WSTRING(n, u->name());

    if (allNames.count(n) != 0)
    {
      SEMANTIC_ERROR(L"More than one units in the model named " + n, u);
    }

    if (mReservedUnits.count(n) != 0)
    {
      SEMANTIC_ERROR(L"Units in the model uses reserved name " + n, u);
    }

    allNames.insert(n);
  }
}

void
ModelValidation::validatePerModel(iface::cellml_api::Model* aModel)
{
  validateNameUniqueness(aModel);

  {
    RETURN_INTO_OBJREF(ccs, iface::cellml_api::CellMLComponentSet,
                       aModel->localComponents());
    RETURN_INTO_OBJREF(cci, iface::cellml_api::CellMLComponentIterator,
                       ccs->iterateComponents());
    while (true)
    {
      RETURN_INTO_OBJREF(cc, iface::cellml_api::CellMLComponent,
                         cci->nextComponent());
      if (cc == NULL)
        break;
      
      validatePerComponent(cc);
    }
  }

  {
    RETURN_INTO_OBJREF(cus, iface::cellml_api::UnitsSet,
                       aModel->localUnits());
    RETURN_INTO_OBJREF(cui, iface::cellml_api::UnitsIterator,
                       cus->iterateUnits());
    while (true)
    {
      RETURN_INTO_OBJREF(cu, iface::cellml_api::Units,
                         cui->nextUnits());
      if (cu == NULL)
        break;
      
      validatePerUnits(cu);
    }
  }

  {
    RETURN_INTO_OBJREF(cons, iface::cellml_api::ConnectionSet,
                       aModel->connections());
    RETURN_INTO_OBJREF(coni, iface::cellml_api::ConnectionIterator,
                       cons->iterateConnections());
    
    while (true)
    {
      RETURN_INTO_OBJREF(conn, iface::cellml_api::Connection,
                         coni->nextConnection());
      if (conn == NULL)
        break;

      RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                         conn->componentMapping());

      // Look for duplicate components...
      ObjRef<iface::cellml_api::CellMLComponent> comp1, comp2;
      try
      {
        comp1 = already_AddRefd<iface::cellml_api::CellMLComponent>
          (cm->firstComponent());
      }
      catch (...)
      {
        SEMANTIC_ERROR(L"Invalid component referenced by component_1 attribute",
                       cm);
      }

      try
      {
        comp2 = already_AddRefd<iface::cellml_api::CellMLComponent>
          (cm->secondComponent());
      }
      catch (...)
      {
        SEMANTIC_ERROR(L"Invalid component referenced by component_2 attribute",
                       cm);
      }

      if (comp1 != NULL && comp2 != NULL)
      {
        char* s1 = comp1->objid();
        char* s2 = comp2->objid();

        int match = strcmp(s1, s2);
        std::string id1((match > 0) ? s1 : s2);
        std::string id2((match > 0) ? s2 : s1);

        free(s1);
        free(s2);

        if (match == 0)
        {
          SEMANTIC_ERROR(L"Cannot connect a component to itself", cm);
        }

        std::pair<std::string, std::string> p(id1, id2);

        if (mAllConns.count(p) != 0)
        {
          SEMANTIC_ERROR(L"There is more than one connection element for the "
                         L"same pair of components", cm);
        }
        else
          mAllConns.insert(p);
      }

      validatePerConnection(conn);
    }
  }

  {
    RETURN_INTO_OBJREF(gs, iface::cellml_api::GroupSet,
                       aModel->groups());
    RETURN_INTO_OBJREF(gi, iface::cellml_api::GroupIterator,
                       gs->iterateGroups());
    while (true)
    {
      RETURN_INTO_OBJREF(g, iface::cellml_api::Group,
                         gi->nextGroup());
      if (g == NULL)
        break;

      validatePerGroup(g);
    }
  }

  RETURN_INTO_OBJREF(cis, iface::cellml_api::CellMLImportSet, aModel->imports());
  RETURN_INTO_OBJREF(cii, iface::cellml_api::CellMLImportIterator,
                     cis->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLImport, cii->nextImport());
    if (ci == NULL)
      break;
    
    validatePerImport(ci);

    RETURN_INTO_OBJREF(m, iface::cellml_api::Model, ci->importedModel());
    if (m != NULL)
      validatePerModel(m);
  }
}

void
ModelValidation::validatePerImport(iface::cellml_api::CellMLImport* aImport)
{
  validateComponentRefs(aImport);
  validateUnitsRefs(aImport);

  RETURN_INTO_OBJREF(ccs, iface::cellml_api::ImportComponentSet,
                     aImport->components());
  RETURN_INTO_OBJREF(cci, iface::cellml_api::ImportComponentIterator,
                     ccs->iterateImportComponents());
  while (true)
  {
    RETURN_INTO_OBJREF(cc, iface::cellml_api::ImportComponent,
                       cci->nextImportComponent());
    if (cc == NULL)
      break;
    
    validatePerImportComponent(cc);
  }
  
  RETURN_INTO_OBJREF(cus, iface::cellml_api::ImportUnitsSet,
                     aImport->units());
  RETURN_INTO_OBJREF(cui, iface::cellml_api::ImportUnitsIterator,
                     cus->iterateImportUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(cu, iface::cellml_api::ImportUnits,
                       cui->nextImportUnits());
    if (cu == NULL)
      break;
    
    validatePerImportUnits(cu);
  }
}

void
ModelValidation::validateComponentRefs(iface::cellml_api::CellMLImport* aImport)
{
  RETURN_INTO_OBJREF(ics, iface::cellml_api::ImportComponentSet,
                     aImport->components());
  RETURN_INTO_OBJREF(ici, iface::cellml_api::ImportComponentIterator,
                     ics->iterateImportComponents());
  RETURN_INTO_OBJREF(m, iface::cellml_api::Model, aImport->importedModel());
  if (m == NULL)
    return;

  RETURN_INTO_OBJREF(ccs, iface::cellml_api::CellMLComponentSet,
                     m->modelComponents());

  while (true)
  {
    RETURN_INTO_OBJREF(ic, iface::cellml_api::ImportComponent,
                       ici->nextImportComponent());
    if (ic == NULL)
      return;

    RETURN_INTO_WSTRING(cr, ic->componentRef());
    RETURN_INTO_OBJREF(rc, iface::cellml_api::CellMLComponent,
                       ccs->getComponent(cr.c_str()));
    if (rc == NULL)
    {
      SEMANTIC_ERROR(L"component_ref " + cr + L"refers to component which "
                     L"doesn't exist", ic);
    }
  }
}

void
ModelValidation::validateUnitsRefs(iface::cellml_api::CellMLImport* aImport)
{
  RETURN_INTO_OBJREF(ius, iface::cellml_api::ImportUnitsSet,
                     aImport->units());
  RETURN_INTO_OBJREF(iui, iface::cellml_api::ImportUnitsIterator,
                     ius->iterateImportUnits());
  RETURN_INTO_OBJREF(m, iface::cellml_api::Model, aImport->importedModel());
  if (m == NULL)
    return;

  RETURN_INTO_OBJREF(cus, iface::cellml_api::UnitsSet,
                     m->modelUnits());

  while (true)
  {
    RETURN_INTO_OBJREF(iu, iface::cellml_api::ImportUnits,
                       iui->nextImportUnits());
    if (iu == NULL)
      return;

    RETURN_INTO_WSTRING(ur, iu->unitsRef());
    RETURN_INTO_OBJREF(ru, iface::cellml_api::Units,
                       cus->getUnits(ur.c_str()));
    if (ru == NULL)
    {
      SEMANTIC_ERROR(L"units_ref " + ur + L"refers to units which "
                     L"don't exist", iu);
    }
  }
}

void
ModelValidation::validatePerComponent
(
 iface::cellml_api::CellMLComponent* aComponent
)
{
  // Look for variables with duplicated names, and also bad units...
  std::set<std::wstring> vnames;

  RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet, aComponent->variables());
  RETURN_INTO_OBJREF(vi, iface::cellml_api::CellMLVariableIterator, vs->iterateVariables());

  while (true)
  {
    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable, vi->nextVariable());
    if (v == NULL)
      break;

    RETURN_INTO_WSTRING(vn, v->name());

    if (vnames.count(vn) != 0)
    {
      SEMANTIC_ERROR(L"There is more than one variable in the same component called " + vn,
                     v);
    }
    vnames.insert(vn);

    validatePerVariable(v, aComponent);
  }

  RETURN_INTO_OBJREF(cus, iface::cellml_api::UnitsSet, aComponent->units());
  RETURN_INTO_OBJREF(cui, iface::cellml_api::UnitsIterator, cus->iterateUnits());
  std::set<std::wstring> allNames;
  while (true)
  {
    RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                       cui->nextUnits());
    if (u == NULL)
      break;

    RETURN_INTO_WSTRING(n, u->name());

    if (allNames.count(n) != 0)
    {
      SEMANTIC_ERROR(L"More than one units in the component named " + n, u);
    }

    if (mReservedUnits.count(n) != 0)
    {
      SEMANTIC_ERROR(L"Units in the component uses reserved name " + n, u);
    }

    allNames.insert(n);
  }

  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathList, aComponent->math());
  RETURN_INTO_OBJREF(mli, iface::cellml_api::MathMLElementIterator, ml->iterate());
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::mathml_dom::MathMLElement, mli->next());
    if (el == NULL)
      break;

    validateMaths(aComponent, el);
  }
}

void
ModelValidation::validatePerVariable
(
 iface::cellml_api::CellMLVariable* v,
 iface::cellml_api::CellMLComponent* c
)
{
  if (mStrictCUSES != NULL)
  {
    RETURN_INTO_WSTRING(u, v->unitsName());
    RETURN_INTO_OBJREF(cur,
                       iface::cellml_services::CanonicalUnitRepresentation,
                       mStrictCUSES->getUnitsByName(v, u.c_str()));
    if (cur == NULL)
    {
      SEMANTIC_ERROR(L"Invalid units on variable: " + u, v);
    }
  }

  iface::cellml_api::VariableInterface pubi(v->publicInterface()),
    privi(v->privateInterface());

  if (pubi == privi &&
      pubi == iface::cellml_api::INTERFACE_IN)
  {
    SEMANTIC_ERROR(L"Cannot have two in interfaces on variable", v);
  }

  // Now look at initial_value attribute...
  RETURN_INTO_WSTRING(iv, v->initialValue());
  if (iv != L"")
  {
    if (pubi == iface::cellml_api::INTERFACE_IN ||
        privi == iface::cellml_api::INTERFACE_IN
       )
    {
      SEMANTIC_ERROR(L"Variables with public or private interfaces of in cannot"
                     L" have initial value attributes", v);
    }

    if ((!((iv[0] >= '0' && iv[0] <= '9') || iv[0] == '-')))
    {
      RETURN_INTO_WSTRING(n, v->name());
      if (n == iv)
      {
        SEMANTIC_ERROR(L"Variable can't have initial_value attribute reference "
                       L"itself", v);
      }

      RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet,
                         c->variables());
      RETURN_INTO_OBJREF(vsrc, iface::cellml_api::CellMLVariable,
                         vs->getVariable(iv.c_str()));
      if (vsrc == NULL)
      {
        SEMANTIC_ERROR(L"Variable has initial_value attribute which is "
                       L"a CellML identifier which does not name a variable "
                       L"in the same component", v);
      }
    }
  }
}

enum EncapsulationRelationship
{
  COMP1_PARENT_COMP2,
  COMP2_PARENT_COMP1,
  COMP1_SIBLING_COMP2,
  COMP1_HIDDEN_COMP2
};

bool
ModelValidation::checkComponentRelevant(iface::cellml_api::CellMLComponent* aComp)
{
  ObjRef<iface::cellml_api::CellMLComponent> c(aComp), next;

  while (true)
  {
    next = already_AddRefd<iface::cellml_api::CellMLComponent>
      (c->encapsulationParent());
    if (next == NULL)
      break;

    c = next;
  }

  // encapsulationParent takes us to the top of the encapsulation hierarchy,
  // even when there are imports involved. So now it is a case of going straight
  // up to see if we have a path to the toplevel model...

  while (true)
  {
    RETURN_INTO_WSTRING(componentName, c->name());
    RETURN_INTO_OBJREF(m, iface::cellml_api::Model, c->modelElement());
    RETURN_INTO_OBJREF(mp, iface::cellml_api::CellMLElement,
                       m->parentElement());

    if (mp == NULL)
      return true;

    DECLARE_QUERY_INTERFACE_OBJREF(imp, mp, cellml_api::CellMLImport);

    RETURN_INTO_OBJREF(ics, iface::cellml_api::ImportComponentSet,
                       imp->components());
    RETURN_INTO_OBJREF(ici, iface::cellml_api::ImportComponentIterator,
                       ics->iterateImportComponents());
    while (true)
    {
      RETURN_INTO_OBJREF(ic, iface::cellml_api::ImportComponent,
                         ici->nextImportComponent());
      if (ic == NULL)
        return false;
      
      RETURN_INTO_WSTRING(cr, ic->componentRef());
      if (cr == componentName)
      {
        c = ic;
        break;
      }
    }
  }
  // Unreachable.
}

void
ModelValidation::validatePerConnection(iface::cellml_api::Connection* aConn)
{
  RETURN_INTO_OBJREF(vms, iface::cellml_api::MapVariablesSet,
                     aConn->variableMappings());
  RETURN_INTO_OBJREF(vmi, iface::cellml_api::MapVariablesIterator,
                     vms->iterateMapVariables());

  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     aConn->componentMapping());
  ObjRef<iface::cellml_api::CellMLComponent> c1, c2;
  try
  {
    c1 = already_AddRefd<iface::cellml_api::CellMLComponent>
      (mc->firstComponent());
  }
  catch (...)
  {
    SEMANTIC_ERROR(L"component_1 attribute doesn't refer to a valid "
                   L"component", mc);
  }

  try
  {
    c2 = already_AddRefd<iface::cellml_api::CellMLComponent>
      (mc->secondComponent());
  }
  catch (...)
  {
    SEMANTIC_ERROR(L"component_2 attribute doesn't refer to a valid "
                   L"component", mc);
  }

  if (c1 == NULL || c2 == NULL)
    return;

  if (!CDA_objcmp(c1, c2))
  {
    SEMANTIC_ERROR(L"It is not valid to map a component to itself", mc);
  }

  std::pair<iface::cellml_api::CellMLComponent*,
            iface::cellml_api::CellMLComponent*>
    cp(c1, c2);

  if (mConnectedComps.count(cp) != 0)
  {
    SEMANTIC_ERROR(L"The can only be a single connection element for each pair "
                   L"of components in the model", mc);
  }
  mConnectedComps.insert(cp);

  // We can only do the remaining checks if the component is actually imported or
  // encapsulated under something which is imported...
  if (!checkComponentRelevant(c1) || !checkComponentRelevant(c2))
    return;

  RETURN_INTO_OBJREF(ep1, iface::cellml_api::CellMLComponent,
                     c1->encapsulationParent());
  RETURN_INTO_OBJREF(ep2, iface::cellml_api::CellMLComponent,
                     c2->encapsulationParent());

  EncapsulationRelationship er(COMP1_HIDDEN_COMP2);
  if ((ep1 == NULL && ep2 == NULL) || (ep1 != NULL && ep2 != NULL && !CDA_objcmp(ep1, ep2)))
    er = COMP1_SIBLING_COMP2;
  else if ((ep2 != NULL) && !CDA_objcmp(ep2, c1))
    er = COMP1_PARENT_COMP2;
  else if ((ep1 != NULL) && !CDA_objcmp(ep1, c2))
    er = COMP2_PARENT_COMP1;
  else
  {
    SEMANTIC_ERROR(L"Connection of components which are encapsulated in the "
                   L"hidden set of each other", mc);
  }

  std::set<std::pair<iface::cellml_api::CellMLVariable*,
                     iface::cellml_api::CellMLVariable*>,
           OrderlessXPCOMPairComparator> connectedVars;

  while (true)
  {
    RETURN_INTO_OBJREF(mv, iface::cellml_api::MapVariables,
                       vmi->nextMapVariable());
    if (mv == NULL)
      break;
    
    ObjRef<iface::cellml_api::CellMLVariable> v1, v2;
    try
    {
      v1 = already_AddRefd<iface::cellml_api::CellMLVariable>
        (mv->firstVariable());
    }
    catch (...)
    {
      SEMANTIC_ERROR(L"variable_1 attribute doesn't refer to a valid "
                     L"variable", mv);
    }

    try
    {
      v2 = already_AddRefd<iface::cellml_api::CellMLVariable>
        (mv->secondVariable());
    }
    catch (...)
    {
      SEMANTIC_ERROR(L"variable_2 attribute doesn't refer to a valid "
                     L"variable", mv);
    }

    // Can't do anything further if either variable is null...
    if (v1 == NULL || v2 == NULL)
      return;

    if (mStrictCUSES != NULL)
    {
      // Connected variables must be dimensionally compatible...
      RETURN_INTO_WSTRING(u1, v1->unitsName());
      RETURN_INTO_OBJREF(cur1, iface::cellml_services::CanonicalUnitRepresentation,
                         mStrictCUSES->getUnitsByName(v1, u1.c_str()));
      RETURN_INTO_WSTRING(u2, v2->unitsName());
      RETURN_INTO_OBJREF(cur2, iface::cellml_services::CanonicalUnitRepresentation,
                         mStrictCUSES->getUnitsByName(v2, u2.c_str()));
      if (cur1 != NULL && cur2 != NULL && !cur1->compatibleWith(cur2))
      {
        SEMANTIC_ERROR(L"Connection of two variables which have dimensionally "
                       L"inconsistent units", mv);
      }
    }

    std::pair<iface::cellml_api::CellMLVariable*, iface::cellml_api::CellMLVariable*>
      vp(v1, v2);
    if (connectedVars.count(vp))
    {
      SEMANTIC_ERROR(L"Connection of the same two variables more than once",
                     mv);
    }
    connectedVars.insert(vp);

    iface::cellml_api::VariableInterface vi1, vi2;
    std::wstring int1, int2;
    if (er == COMP1_SIBLING_COMP2)
    {
      vi1 = v1->publicInterface();
      int1 = L"public";
      vi2 = v2->publicInterface();
      int2 = L"public";
    }
    else if (er == COMP1_PARENT_COMP2)
    {
      vi1 = v1->privateInterface();
      int1 = L"private";
      vi2 = v2->publicInterface();
      int2 = L"public";
    }
    else
    {
      vi1 = v1->publicInterface();
      int1 = L"public";
      vi2 = v2->privateInterface();
      int2 = L"private";
    }

    if (vi1 == iface::cellml_api::INTERFACE_NONE)
    {
      SEMANTIC_ERROR(L"Mapping variable_1 which has " + int1 + L" interface of none",
                     mv);
    }

    if (vi2 == iface::cellml_api::INTERFACE_NONE)
    {
      SEMANTIC_ERROR(L"Mapping variable_2 which has " + int2 + L" interface of none",
                     mv);
    }

    if (vi1 == vi2)
    {
      std::wstring dir = (vi1 == iface::cellml_api::INTERFACE_IN) ? L"in" : L"out";
      SEMANTIC_ERROR(L"Mapping variable_1 has " + int1 + L" interface of " + dir +
                     L" but variable_2 also has " + int2 + L" interface of " +
                     dir + L"", mv);
    }

    // We now need to add the variable with the in interface to a list of
    // variables which have been connected up so we can check that the same
    // variable isn't connected twice...
    iface::cellml_api::CellMLVariable* vin =
      (vi1 == iface::cellml_api::INTERFACE_IN) ? v1 : v2;

    if (mSeenInVars.count(vin))
    {
      SEMANTIC_ERROR(L"More than one connection to in interface of variable", vin);
    }

    mSeenInVars.insert(vin);
  }
}

void
ModelValidation::validatePerUnits(iface::cellml_api::Units* aUnits)
{
}

void
ModelValidation::validatePerGroup(iface::cellml_api::Group* aGroup)
{
  RETURN_INTO_OBJREF(rrs,
                     iface::cellml_api::RelationshipRefSet,
                     aGroup->relationshipRefs());
  RETURN_INTO_OBJREF(rri,
                     iface::cellml_api::RelationshipRefIterator,
                     rrs->iterateRelationshipRefs());

  std::set<GroupRelationship> relns;
  while (true)
  {
    RETURN_INTO_OBJREF(rr, iface::cellml_api::RelationshipRef,
                       rri->nextRelationshipRef());
    if (rr == NULL)
      break;

    RETURN_INTO_WSTRING(n, rr->name());
    RETURN_INTO_WSTRING(reln, rr->relationship());
    RETURN_INTO_WSTRING(relnns, rr->relationshipNamespace());
    GroupRelationship g(relnns, reln, n);
    if (relns.count(g) != 0)
    {
      SEMANTIC_ERROR(L"Duplicate relationship_ref within group", rr);
    }
    else
      relns.insert(g);
  }

  RETURN_INTO_OBJREF(crs,
                     iface::cellml_api::ComponentRefSet,
                     aGroup->componentRefs());
  validateGroupComponentRefs(relns, crs, true);
}

bool
ModelValidation::validateGroupComponentRefs
(
 const std::set<GroupRelationship>& aRelns,
 iface::cellml_api::ComponentRefSet* aCRS,
 bool aMustHaveChildren
)
{
  bool foundSomething = false;

  RETURN_INTO_OBJREF(cri,
                     iface::cellml_api::ComponentRefIterator,
                     aCRS->iterateComponentRefs());

  ObjRef<iface::cellml_api::CellMLComponentSet> ccs;

  while (true)
  {
    RETURN_INTO_OBJREF(cr, iface::cellml_api::ComponentRef,
                       cri->nextComponentRef());
    if (cr == NULL)
      break;

    if (!foundSomething)
    {
      RETURN_INTO_OBJREF(model, iface::cellml_api::Model,
                         cr->modelElement());
      ccs = already_AddRefd<iface::cellml_api::CellMLComponentSet>
        (model->modelComponents());
    }

    foundSomething = true;

    // Recursively check child component refs, and as a side effect, decide
    // if there are any child component refs...
    RETURN_INTO_OBJREF(childCompRefs, iface::cellml_api::ComponentRefSet,
                       cr->componentRefs());
    bool hasChildren = validateGroupComponentRefs(aRelns, childCompRefs);

    if (aMustHaveChildren && !hasChildren)
    {
      SEMANTIC_ERROR(L"component_ref element appears as child of a group element "
                     L"but does not have any child component_ref elements", cr);
      continue;
    }

    // Check that the component_ref is valid...
    RETURN_INTO_WSTRING(name, cr->componentName());
    RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                       ccs->getComponent(name.c_str()));

    if (c == NULL)
    {
      SEMANTIC_ERROR(L"component_ref element references component which does "
                     L"not exist", cr);
      continue;
    }

    while (true)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(ic, c, cellml_api::ImportComponent);
      if (ic == NULL)
        break;

      RETURN_INTO_WSTRING(compref, ic->componentRef());

      RETURN_INTO_OBJREF(impel, iface::cellml_api::CellMLElement,
                         c->parentElement());
      DECLARE_QUERY_INTERFACE_OBJREF(imp, impel, cellml_api::CellMLImport);
      if (imp == NULL)
        break;

      RETURN_INTO_OBJREF(m, iface::cellml_api::Model, imp->importedModel());

      if (m == NULL)
        break;

      RETURN_INTO_OBJREF(iccs,
                         iface::cellml_api::CellMLComponentSet,
                         m->modelComponents());
      c = already_AddRefd<iface::cellml_api::CellMLComponent>
        (iccs->getComponent(compref.c_str()));

      if (c == NULL)
        break;
    }

    // If this happens, model is invalid, but it will be reported elsewhere...
    if (c == NULL)
      continue;

    char* sobjid = c->objid();
    std::string objid = sobjid;
    free(sobjid);

    if (hasChildren)
    {
      // Now we validate the 'only one occurrence as parent' rule...
      for (std::set<GroupRelationship>::const_iterator i(aRelns.begin());
           i != aRelns.end();
           i++)
      {
        GroupParent gp(objid, *i);
        if (mGroupParents.count(gp) != 0)
        {
          std::wstring msg =
            L"In a given hierarchy, only one of the <component_ref> "
            L"elements that reference a given component may contain "
            L"further <component_ref> elements, but the ";
          msg += (*i).relationship;
          msg += L" hierarchy";
          if ((*i).relNamespace != L"")
          {
            msg += L", in the namespace ";
            msg += (*i).relNamespace;
          }
          if ((*i).name != L"")
          {
            msg += L" with name ";
            msg += name;
          }
          
          msg += L" has more than one non-terminal component_ref to ";
          RETURN_INTO_WSTRING(cname, c->name());
          msg += cname;
          
          SEMANTIC_ERROR(msg, cr);
        }
        else
          mGroupParents.insert(gp);
      }
    }
  }

  return foundSomething;
}

void
ModelValidation::validatePerImportComponent
(
 iface::cellml_api::ImportComponent* aComponent
)
{
}

void
ModelValidation::validatePerImportUnits(iface::cellml_api::ImportUnits* aUnits)
{
}

void
ModelValidation::validateSemantics()
{
  // Firstly do things that are done per import...
  validatePerModel(mModel);
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
