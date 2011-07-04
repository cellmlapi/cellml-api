#define IN_CCGS_MODULE
#define MODULE_CONTAINS_CCGS
#include "CCGSImplementation.hpp"
#include "CodeGenerationError.hxx"
#include "CodeGenerationState.hxx"
#include "CCGSBootstrap.hpp"
#include "AnnoToolsBootstrap.hpp"
#include "MaLaESBootstrap.hpp"
#include "CeVASBootstrap.hpp"
#include "CUSESBootstrap.hpp"
#include "cda_compiler_support.h"

iface::cellml_api::CellMLVariable*
CDA_ComputationTarget::variable() throw()
{
  mVariable->add_ref();
  return mVariable;
}

uint32_t
CDA_ComputationTarget::degree() throw()
{
  return mDegree;
}

iface::cellml_services::VariableEvaluationType
CDA_ComputationTarget::type() throw()
{
  return mEvaluationType;
}

wchar_t*
CDA_ComputationTarget::name() throw()
{
  const wchar_t* annoname;
  wchar_t annobuf[30];

  // Scoped locale change.
  CNumericLocale locobj;

  if (mDegree == 0)
    annoname = L"expression";
  else
  {
    any_swprintf(annobuf, 30, L"expression_d%u", mDegree);
    annoname = annobuf;
  }

  return mAnnoSet->getStringAnnotation(mVariable, annoname);
}

uint32_t
CDA_ComputationTarget::assignedIndex() throw()
{
  return mAssignedIndex;
}

void
CDA_ComputationTarget::setDelayedName(const wchar_t* aSetTo)
  throw()
{
  // Scoped locale change.
  CNumericLocale locobj;

  wchar_t buf[30], dbuf[40];
  const wchar_t* annoname, *dannoname;

  if (mDegree == 0)
  {
    annoname = L"expression";
    dannoname = L"delayed_expression";
  }
  else
  {
    any_swprintf(buf, 30, L"expression_d%u", mDegree);
    any_swprintf(dbuf, 40, L"delayed_expression_d%u", mDegree);
    annoname = buf;
    dannoname = dbuf;
  }

  if (aSetTo == NULL)
  {
    RETURN_INTO_WSTRING(n, mAnnoSet->getStringAnnotation(mVariable, annoname));
    mAnnoSet->setStringAnnotation(mVariable, dannoname, n.c_str());
  }
  else
    mAnnoSet->setStringAnnotation(mVariable, dannoname, aSetTo);
}

void
CDA_ComputationTarget::setNameAndIndex
(
 uint32_t aIndex,
 const wchar_t* aName
)
  throw()
{
  // Scoped locale change.
  CNumericLocale locobj;

  const wchar_t* annoname;
  wchar_t annobuf[30];
  if (mDegree == 0)
    annoname = L"expression";
  else
  {
    any_swprintf(annobuf, 30, L"expression_d%u", mDegree);
    annoname = annobuf;
  }

  mAssignedIndex = aIndex;

  mAnnoSet->setStringAnnotation(mVariable, annoname, aName);
}

iface::cellml_services::ComputationTarget*
CDA_ComputationTargetIterator::nextComputationTarget() throw()
{
  if (mTargetsIt == mTargets.end())
    return NULL;

  ptr_tag<CDA_ComputationTarget> t = *mTargetsIt;
  mTargetsIt++;
  
  t->add_ref();
  return t;
}

CDA_TargetSet::~CDA_TargetSet()
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator ti;
  for (ti = mTargets.begin(); ti != mTargets.end(); ti++)
    (*ti)->release_ref();
}

CDA_CodeInformation::~CDA_CodeInformation()
{
  std::vector<iface::dom::Element*>::iterator fei;
  for (fei = mFlaggedEquations.begin(); fei != mFlaggedEquations.end(); fei++)
    (*fei)->release_ref();
}

wchar_t*
CDA_CodeInformation::errorMessage() throw()
{
  return CDA_wcsdup(mErrorMessage.c_str());
}

iface::cellml_services::ModelConstraintLevel
CDA_CodeInformation::constraintLevel() throw()
{
  return mConstraintLevel;
}

uint32_t
CDA_CodeInformation::algebraicIndexCount() throw()
{
  return mAlgebraicIndexCount;
}

uint32_t
CDA_CodeInformation::rateIndexCount() throw()
{
  return mRateIndexCount;
}

uint32_t
CDA_CodeInformation::constantIndexCount() throw()
{
  return mConstantIndexCount;
}

uint32_t
CDA_CodeInformation::conditionVariableCount() throw()
{
  return mConditionVariableCount;
}

uint32_t
CDA_CodeInformation::randomIndexCount() throw()
{
  return mRandomIndexCount;
}

wchar_t*
CDA_CodeInformation::rootInformationString() throw()
{
  return CDA_wcsdup(mRootInformationStr.c_str());
}

wchar_t*
CDA_CodeInformation::initConstsString() throw()
{
  return CDA_wcsdup(mInitConstsStr.c_str());
}

wchar_t*
CDA_CodeInformation::ratesString() throw()
{
  return CDA_wcsdup(mRatesStr.c_str());
}

wchar_t*
CDA_CodeInformation::variablesString() throw()
{
  return CDA_wcsdup(mVarsStr.c_str());
}

wchar_t*
CDA_CodeInformation::functionsString() throw()
{
  return CDA_wcsdup(mFuncsStr.c_str());
}

wchar_t*
CDA_CodeInformation::essentialVariablesString() throw()
{
  return CDA_wcsdup(mEssentialVarsStr.c_str());
}

wchar_t*
CDA_CodeInformation::stateInformationString() throw()
{
  return CDA_wcsdup(mStateInformationStr.c_str());
}

iface::cellml_services::ComputationTargetIterator*
CDA_CodeInformation::iterateTargets() throw()
{
  return new CDA_ComputationTargetIterator(this);
}

CDA_CustomCodeInformation::CDA_CustomCodeInformation()
  : _cda_refcount(1)
{
}

iface::cellml_services::ModelConstraintLevel
CDA_CustomCodeInformation::constraintLevel()
  throw()
{
  return mConstraintLevel;
}

uint32_t
CDA_CustomCodeInformation::indexCount()
  throw()
{
  return mIndexCount;
}

iface::cellml_services::ComputationTargetIterator*
CDA_CustomCodeInformation::iterateTargets() throw()
{
  return new CDA_ComputationTargetIterator(this);
}

wchar_t*
CDA_CustomCodeInformation::generatedCode() throw()
{
  return CDA_wcsdup(mGeneratedCode.c_str());
}

wchar_t*
CDA_CustomCodeInformation::functionsString() throw()
{
  return CDA_wcsdup(mFunctionsString.c_str());
}

class CDA_FlaggedEquationsNodeList
  : public iface::mathml_dom::MathMLNodeList
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_QI2(mathml_dom::MathMLNodeList, dom::NodeList);
  CDA_IMPL_REFCOUNT;

  CDA_FlaggedEquationsNodeList(CDA_CodeInformation* aCodeInfo,
                               std::vector<iface::dom::Element*>& aVector)
    : _cda_refcount(1), mCodeInfo(aCodeInfo), mVector(aVector) {};
  ~CDA_FlaggedEquationsNodeList() {};
  
  uint32_t length() throw()
  {
    return mVector.size();
  }

  iface::dom::Element* item(uint32_t idx)
    throw(std::exception&)
  {
    if (idx >= mVector.size())
      throw iface::dom::DOMException();

    mVector[idx]->add_ref();

    return mVector[idx];
  }

private:
  ObjRef<CDA_CodeInformation> mCodeInfo;
  std::vector<iface::dom::Element*>& mVector;
};

iface::mathml_dom::MathMLNodeList*
CDA_CodeInformation::flaggedEquations() throw()
{
  return new CDA_FlaggedEquationsNodeList(this, mFlaggedEquations);
}

CDA_CodeGenerator::CDA_CodeGenerator(bool aIDAStyle)
 : _cda_refcount(1),
   mConstantPattern(L"CONSTANTS[%]"),
   mStateVariableNamePattern(L"STATES[%]"),
   mAlgebraicVariableNamePattern(L"ALGEBRAIC[%]"),
   mRateNamePattern(L"RATES[%]"),
   mVOIPattern(L"VOI"),
   mRandomPattern(L"RANDOM[%]"),
   mAssignPattern(L"<LHS> = <RHS>;\r\n"),
   mSolvePattern
   (
    L"rootfind_<ID>(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);\r\n"
    L"<SUP>"
    L"void objfunc_<ID>(double *p, double *hx, int m, int n, void *adata)\r\n"
    L"{\r\n"
    L"  struct rootfind_info* rfi = (struct rootfind_info*)adata;\r\n"
    L"#define VOI rfi->aVOI\r\n"
    L"#define CONSTANTS rfi->aCONSTANTS\r\n"
    L"#define RATES rfi->aRATES\r\n"
    L"#define STATES rfi->aSTATES\r\n"
    L"#define ALGEBRAIC rfi->aALGEBRAIC\r\n"
    L"#define pret rfi->aPRET\r\n"
    L"  <VAR> = *p;\r\n"
    L"  *hx = (<LHS>) - (<RHS>);\r\n"
    L"#undef VOI\r\n"
    L"#undef CONSTANTS\r\n"
    L"#undef RATES\r\n"
    L"#undef STATES\r\n"
    L"#undef ALGEBRAIC\r\n"
    L"#undef pret\r\n"
    L"}\r\n"
    L"void rootfind_<ID>(double VOI, double* CONSTANTS, double* RATES, "
    L"double* STATES, double* ALGEBRAIC, int* pret)\r\n"
    L"{\r\n"
    L"  static double val = <IV>;\r\n"
    L"  double bp, work[LM_DIF_WORKSZ(1, 1)];\r\n"
    L"  struct rootfind_info rfi;\r\n"
    L"  rfi.aVOI = VOI;\r\n"
    L"  rfi.aCONSTANTS = CONSTANTS;\r\n"
    L"  rfi.aRATES = RATES;\r\n"
    L"  rfi.aSTATES = STATES;\r\n"
    L"  rfi.aALGEBRAIC = ALGEBRAIC;\r\n"
    L"  rfi.aPRET = pret;\r\n"
    L"  do_levmar(objfunc_<ID>, &val, &bp, work, pret, 1, &rfi);\r\n"
    L"  <VAR> = val;\r\n"
    L"}\r\n"
   ),
   mSolveNLSystemPattern
   (
    L"rootfind_<ID>(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);\r\n"
    L"<SUP>"
    L"void objfunc_<ID>(double *p, double *hx, int m, int n, void *adata)\r\n"
    L"{\r\n"
    L"  struct rootfind_info* rfi = (struct rootfind_info*)adata;\r\n"
    L"#define VOI rfi->aVOI\r\n"
    L"#define CONSTANTS rfi->aCONSTANTS\r\n"
    L"#define RATES rfi->aRATES\r\n"
    L"#define STATES rfi->aSTATES\r\n"
    L"#define ALGEBRAIC rfi->aALGEBRAIC\r\n"
    L"#define pret rfi->aPRET\r\n"
    L"  <EQUATIONS><VAR> = p[<INDEX>];<JOIN>\r\n"
    L"  </EQUATIONS>\r\n"
    L"  <EQUATIONS>hx[<INDEX>] = <EXPR>;<JOIN>\r\n"
    L"  </EQUATIONS>\r\n"
    L"#undef VOI\r\n"
    L"#undef CONSTANTS\r\n"
    L"#undef RATES\r\n"
    L"#undef STATES\r\n"
    L"#undef ALGEBRAIC\r\n"
    L"#undef pret\r\n"
    L"}\r\n"
    L"void rootfind_<ID>(double VOI, double* CONSTANTS, double* RATES, "
    L"double* STATES, double* ALGEBRAIC, int* pret)\r\n"
    L"{\r\n"
    L"  static double p[<COUNT>] = {<EQUATIONS><IV><JOIN>,</EQUATIONS>};\r\n"
    L"  double bp[<COUNT>], work[LM_DIF_WORKSZ(<COUNT>, <COUNT>)];\r\n"
    L"  struct rootfind_info rfi;\r\n"
    L"  rfi.aVOI = VOI;\r\n"
    L"  rfi.aCONSTANTS = CONSTANTS;\r\n"
    L"  rfi.aRATES = RATES;\r\n"
    L"  rfi.aSTATES = STATES;\r\n"
    L"  rfi.aALGEBRAIC = ALGEBRAIC;\r\n"
    L"  rfi.aPRET = pret;\r\n"
    L"  do_levmar(objfunc_<ID>, p, bp, work, pret, <COUNT>, &rfi);\r\n"
    L"  <EQUATIONS><VAR> = p[<INDEX>];<JOIN>\r\n"
    L"  </EQUATIONS>\r\n"
    L"}\r\n"
   ),
   mTemporaryVariablePattern(L"temp%"),
   mDeclareTemporaryPattern(L"double %;\r\n"),
   mConditionalAssignmentPattern
   (
    L"if (<CONDITION>)\r\n"
    L"{\r\n"
    L"  <STATEMENT>"
    L"}\r\n"
    L"<CASES>else if (<CONDITION>)\r\n"
    L"{\r\n"
    L"  <STATEMENT>"
    L"}\r\n"
    L"</CASES>"
   ),
   mResidualPattern
   (
    L"resid[<RNO>] = (<LHS>) - (<RHS>);\r\n"
   ),
   mConstrainedRateStateInfoPattern
   (
    L"SI[<ID>] = 1.0;\r\n"
   ),
   mUnconstrainedRateStateInfoPattern
   (
    L"SI[<ID>] = 0.0;\r\n"
   ),
   mInfDelayedRatePattern(L"RATES[%]"),
   mInfDelayedStatePattern(L"STATES[%]"),
   mConditionVariablePattern
   (
    L"CONDVAR[%]"
   ),
   mTrackPiecewiseConditions(true),
   mAllowPassthrough(false),
   mArrayOffset(0),
   mIDAStyle(aIDAStyle)
{
}

wchar_t*
CDA_CodeGenerator::constantPattern() throw()
{
  return CDA_wcsdup(mConstantPattern.c_str());
}

void
CDA_CodeGenerator::constantPattern(const wchar_t* aPattern) throw()
{
  mConstantPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::stateVariableNamePattern() throw()
{
  return CDA_wcsdup(mStateVariableNamePattern.c_str());
}

void
CDA_CodeGenerator::stateVariableNamePattern(const wchar_t* aPattern) throw()
{
  mStateVariableNamePattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::algebraicVariableNamePattern() throw()
{
  return CDA_wcsdup(mAlgebraicVariableNamePattern.c_str());
}

void
CDA_CodeGenerator::algebraicVariableNamePattern(const wchar_t* aPattern) throw()
{
  mAlgebraicVariableNamePattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::rateNamePattern() throw()
{
  return CDA_wcsdup(mRateNamePattern.c_str());
}

void
CDA_CodeGenerator::rateNamePattern(const wchar_t* aPattern) throw()
{
  mRateNamePattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::voiPattern() throw()
{
  return CDA_wcsdup(mVOIPattern.c_str());
}

void
CDA_CodeGenerator::voiPattern(const wchar_t* aPattern) throw()
{
  mVOIPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::randomPattern() throw()
{
  return CDA_wcsdup(mRandomPattern.c_str());
}

void
CDA_CodeGenerator::randomPattern(const wchar_t* aPattern) throw()
{
  mRandomPattern = aPattern;
}

uint32_t
CDA_CodeGenerator::arrayOffset() throw()
{
  return mArrayOffset;
}

void
CDA_CodeGenerator::arrayOffset(uint32_t offset) throw()
{
  mArrayOffset = offset;
}

wchar_t*
CDA_CodeGenerator::assignPattern() throw()
{
  return CDA_wcsdup(mAssignPattern.c_str());
}

void
CDA_CodeGenerator::assignPattern(const wchar_t* aPattern) throw()
{
  mAssignPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::solvePattern() throw()
{
  return CDA_wcsdup(mSolvePattern.c_str());
}

void
CDA_CodeGenerator::solvePattern(const wchar_t* aPattern) throw()
{
  mSolvePattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::solveNLSystemPattern() throw()
{
  return CDA_wcsdup(mSolveNLSystemPattern.c_str());
}

void
CDA_CodeGenerator::solveNLSystemPattern(const wchar_t* aPattern) throw()
{
  mSolveNLSystemPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::temporaryVariablePattern()
  throw()
{
  return CDA_wcsdup(mTemporaryVariablePattern.c_str());
}

void
CDA_CodeGenerator::temporaryVariablePattern(const wchar_t* aPattern)
  throw()
{
  mTemporaryVariablePattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::declareTemporaryPattern()
  throw()
{
  return CDA_wcsdup(mDeclareTemporaryPattern.c_str());
}

void
CDA_CodeGenerator::declareTemporaryPattern(const wchar_t* aPattern)
  throw()
{
  mDeclareTemporaryPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::conditionalAssignmentPattern()
  throw()
{
  return CDA_wcsdup(mConditionalAssignmentPattern.c_str());
}

void
CDA_CodeGenerator::conditionalAssignmentPattern(const wchar_t* aPattern)
  throw()
{
  mConditionalAssignmentPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::residualPattern()
  throw()
{
  return CDA_wcsdup(mResidualPattern.c_str());
}

void
CDA_CodeGenerator::residualPattern(const wchar_t* aPattern)
  throw()
{
  mResidualPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::constrainedRateStateInfoPattern()
  throw()
{
  return CDA_wcsdup(mConstrainedRateStateInfoPattern.c_str());
}

void
CDA_CodeGenerator::constrainedRateStateInfoPattern(const wchar_t* aPattern)
  throw()
{
  mConstrainedRateStateInfoPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::unconstrainedRateStateInfoPattern()
  throw()
{
  return CDA_wcsdup(mUnconstrainedRateStateInfoPattern.c_str());
}

void
CDA_CodeGenerator::unconstrainedRateStateInfoPattern(const wchar_t* aPattern)
  throw()
{
  mUnconstrainedRateStateInfoPattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::infDelayedRatePattern() throw()
{
  return CDA_wcsdup(mInfDelayedRatePattern.c_str());
}

void
CDA_CodeGenerator::infDelayedRatePattern(const wchar_t* aPattern)
  throw()
{
  mInfDelayedRatePattern = aPattern;
}

wchar_t*
CDA_CodeGenerator::infDelayedStatePattern()
  throw()
{
  return CDA_wcsdup(mInfDelayedStatePattern.c_str());
}

void
CDA_CodeGenerator::infDelayedStatePattern(const wchar_t* aPattern)
  throw()
{
  mInfDelayedStatePattern = aPattern;
}


wchar_t*
CDA_CodeGenerator::conditionVariablePattern() throw()
{
  return CDA_wcsdup(mConditionVariablePattern.c_str());
}

void
CDA_CodeGenerator::conditionVariablePattern(const wchar_t* aPattern) throw()
{
  mConditionVariablePattern = aPattern;
}

bool
CDA_CodeGenerator::trackPiecewiseConditions() throw()
{
  return mTrackPiecewiseConditions;
}

void
CDA_CodeGenerator::trackPiecewiseConditions(bool aTrack) throw()
{
  mTrackPiecewiseConditions = aTrack;
}

iface::cellml_services::MaLaESTransform*
CDA_CodeGenerator::transform() throw()
{
  if (mTransform != NULL)
    mTransform->add_ref();

  return mTransform;
}

void
CDA_CodeGenerator::transform(iface::cellml_services::MaLaESTransform* aTransform)
 throw()
{
  mTransform = aTransform;
}

iface::cellml_services::CeVAS*
CDA_CodeGenerator::useCeVAS() throw()
{
  if (mCeVAS != NULL)
    mCeVAS->add_ref();
  return mCeVAS;
}

void
CDA_CodeGenerator::useCeVAS(iface::cellml_services::CeVAS* aCeVAS)
 throw()
{
  mCeVAS = aCeVAS;
}

iface::cellml_services::CUSES*
CDA_CodeGenerator::useCUSES() throw()
{
  if (mCUSES != NULL)
    mCUSES->add_ref();
  return mCUSES;
}

void
CDA_CodeGenerator::useCUSES(iface::cellml_services::CUSES* aCUSES)
 throw()
{
  mCUSES = aCUSES;
}

iface::cellml_services::AnnotationSet*
CDA_CodeGenerator::useAnnoSet() throw()
{
  if (mAnnoSet != NULL)
    mAnnoSet->add_ref();
  return mAnnoSet;
}

void
CDA_CodeGenerator::useAnnoSet(iface::cellml_services::AnnotationSet* aAnnoSet)
 throw()
{
  mAnnoSet = aAnnoSet;
}

static iface::cellml_services::IDACodeInformation*
CDA_ErrorCodeInformation(const wchar_t* aMessage)
{
  CDA_CodeInformation* ci = new CDA_CodeInformation();
  ci->mErrorMessage = aMessage;
  return ci;
}

iface::cellml_services::CustomGenerator*
CDA_CodeGenerator::createCustomGenerator(iface::cellml_api::Model* aSourceModel)
  throw(std::exception&)
{
  return makeCodeGenerationState(aSourceModel)->CreateCustomGenerator();
}

/* Note: this generates both IDA and normal code - we implement it as
 * GenerateIDACode to avoid the need to QueryInterface in the case where we
 * are generating IDA code.
 */
iface::cellml_services::IDACodeInformation*
CDA_CodeGenerator::generateIDACode(iface::cellml_api::Model* aSourceModel)
 throw()
{
  try
  {
    return makeCodeGenerationState(aSourceModel)->GenerateCode();
  }
  catch (...)
  {
    return CDA_ErrorCodeInformation(L"Error processing CellML model.");
  }
}

std::auto_ptr<CodeGenerationState>
CDA_CodeGenerator::makeCodeGenerationState(iface::cellml_api::Model* aSourceModel)
{
  std::auto_ptr<CodeGenerationState> cgs
    (
     new CodeGenerationState
     (
      aSourceModel,
      mConstantPattern, mStateVariableNamePattern,
      mAlgebraicVariableNamePattern,
      mRateNamePattern, mVOIPattern, mRandomPattern,
      mAssignPattern, mSolvePattern,
      mSolveNLSystemPattern, mTemporaryVariablePattern,
      mDeclareTemporaryPattern, mConditionalAssignmentPattern,
      mResidualPattern, mConstrainedRateStateInfoPattern,
      mUnconstrainedRateStateInfoPattern,
      mInfDelayedRatePattern, mInfDelayedStatePattern,
      mConditionVariablePattern, mTrackPiecewiseConditions,
      mArrayOffset, mTransform,
      mCeVAS, mCUSES, mAnnoSet, mIDAStyle
      )
    );

  if (cgs->mAnnoSet == NULL)
  {
    RETURN_INTO_OBJREF(ats, iface::cellml_services::AnnotationToolService,
                       CreateAnnotationToolService());
    cgs->mAnnoSet =
      already_AddRefd<iface::cellml_services::AnnotationSet>
      (ats->createAnnotationSet());
  }

  if (cgs->mTransform == NULL)
  {
    RETURN_INTO_OBJREF(mb, iface::cellml_services::MaLaESBootstrap,
                       CreateMaLaESBootstrap());
    cgs->mTransform =
      already_AddRefd<iface::cellml_services::MaLaESTransform>
      (mb->compileTransformer(
L"opengroup: (\r\n"
L"closegroup: )\r\n"
L"abs: #prec[H]fabs(#expr1)\r\n"
L"and: #prec[20]#exprs[&&]\r\n"
L"arccos: #prec[H]acos(#expr1)\r\n"
L"arccosh: #prec[H]acosh(#expr1)\r\n"
L"arccot: #prec[1000(900)]atan(1.0/#expr1)\r\n"
L"arccoth: #prec[1000(900)]atanh(1.0/#expr1)\r\n"
L"arccsc: #prec[1000(900)]asin(1/#expr1)\r\n"
L"arccsch: #prec[1000(900)]asinh(1/#expr1)\r\n"
L"arcsec: #prec[1000(900)]acos(1/#expr1)\r\n"
L"arcsech: #prec[1000(900)]acosh(1/#expr1)\r\n"
L"arcsin: #prec[H]asin(#expr1)\r\n"
L"arcsinh: #prec[H]asinh(#expr1)\r\n"
L"arctan: #prec[H]atan(#expr1)\r\n"
L"arctanh: #prec[H]atanh(#expr1)\r\n"
L"ceiling: #prec[H]ceil(#expr1)\r\n"
L"cos: #prec[H]cos(#expr1)\r\n"
L"cosh: #prec[H]cosh(#expr1)\r\n"
L"cot: #prec[900(0)]1.0/tan(#expr1)\r\n"
L"coth: #prec[900(0)]1.0/tanh(#expr1)\r\n"
L"csc: #prec[900(0)]1.0/sin(#expr1)\r\n"
L"csch: #prec[900(0)]1.0/sinh(#expr1)\r\n"
L"diff: #lookupDiffVariable\r\n"
L"divide: #prec[900]#expr1/#expr2\r\n"
L"eq: #prec[30]#exprs[==]\r\n"
L"exp: #prec[H]exp(#expr1)\r\n"
L"factorial: #prec[H]factorial(#expr1)\r\n"
L"factorof: #prec[30(900)]#expr1 % #expr2 == 0\r\n"
L"floor: #prec[H]floor(#expr1)\r\n"
L"gcd: #prec[H]gcd_multi(#count, #exprs[, ])\r\n"
L"geq: #prec[30]#exprs[>=]\r\n"
L"gt: #prec[30]#exprs[>]\r\n"
L"implies: #prec[10(950)] !#expr1 || #expr2\r\n"
L"int: #prec[H]defint(func#unique1, VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, &#bvarIndex, #lowlimit, #uplimit, "
L"pret)#supplement double func#unique1(double VOI, "
L"double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, int* pret) { return #expr1; }\r\n"
L"lcm: #prec[H]lcm_multi(#count, #exprs[, ])\r\n"
L"leq: #prec[30]#exprs[<=]\r\n"
L"ln: #prec[H]log(#expr1)\r\n"
L"log: #prec[H]arbitrary_log(#expr1, #logbase)\r\n"
L"lt: #prec[30]#exprs[<]\r\n"
L"max: #prec[H]multi_max(#count, #exprs[, ])\r\n"
L"min: #prec[H]multi_min(#count, #exprs[, ])\r\n"
L"minus: #prec[500]#expr1 - #expr2\r\n"
L"neq: #prec[30]#expr1 != #expr2\r\n"
L"not: #prec[950]!#expr1\r\n"
L"or: #prec[10]#exprs[||]\r\n"
L"plus: #prec[500]#exprs[+]\r\n"
L"power: #prec[H]pow(#expr1, #expr2)\r\n"
L"quotient: #prec[1000(0)] (double)(((int)#expr2) == 0 ? #expr1 / 0.0 : (int)(#expr1) / (int)(#expr2))\r\n"
L"rem: #prec[1000(0)] (double)(((int)#expr2) == 0 ? (#expr1) / 0.0 : (int)(#expr1) % (int)(#expr2))\r\n"
L"root: #prec[1000(900)] pow(#expr1, 1.0 / #degree)\r\n"
L"sec: #prec[900(0)]1.0 / cos(#expr1)\r\n"
L"sech: #prec[900(0)]1.0 / cosh(#expr1)\r\n"
L"sin: #prec[H] sin(#expr1)\r\n"
L"sinh: #prec[H] sinh(#expr1)\r\n"
L"tan: #prec[H] tan(#expr1)\r\n"
L"tanh: #prec[H] tanh(#expr1)\r\n"
L"times: #prec[900] #exprs[*]\r\n"
L"unary_minus: #prec[950]- #expr1\r\n"
L"units_conversion: #prec[500(900)]#expr1*#expr2 + #expr3\r\n"
L"units_conversion_factor: #prec[900]#expr1*#expr2\r\n"
L"units_conversion_offset: #prec[500]#expr1+#expr2\r\n"
L"xor: #prec[25(30)] (#expr1 != 0) ^ (#expr2 != 0)\r\n"
L"piecewise_first_case: #prec[1000(5)](#expr1 ? #expr2 : \r\n"
L"piecewise_extra_case: #prec[1000(5)]#expr1 ? #expr2 : \r\n"
L"piecewise_otherwise: #prec[1000(5)]#expr1)\r\n"
L"piecewise_no_otherwise: #prec[1000(5)]0.0/0.0)\r\n"
L"eulergamma: #prec[999]0.577215664901533\r\n"
L"exponentiale: #prec[999]2.71828182845905\r\n"
L"false: #prec[999]0.0\r\n"
L"infinity: #prec[900]1.0/0.0\r\n"
L"notanumber: #prec[999]0.0/0.0\r\n"
L"pi: #prec[999] 3.14159265358979\r\n"
L"true: #prec[999]1.0\r\n"
                             ));
  }

  if (!mAllowPassthrough)
    cgs->mTransform->stripPassthrough(aSourceModel);

  if (cgs->mCeVAS == NULL)
  {
    RETURN_INTO_OBJREF(cb, iface::cellml_services::CeVASBootstrap,
                       CreateCeVASBootstrap());
    cgs->mCeVAS = already_AddRefd<iface::cellml_services::CeVAS>
      (cb->createCeVASForModel(aSourceModel));
  }

  if (cgs->mCUSES == NULL)
  {
    RETURN_INTO_OBJREF(cb, iface::cellml_services::CUSESBootstrap,
                       CreateCUSESBootstrap());
    cgs->mCUSES = already_AddRefd<iface::cellml_services::CUSES>
      (cb->createCUSESForModel(aSourceModel, false));
  }

  if (!mAllowPassthrough)
    cgs->mTransform->stripPassthrough(aSourceModel);

  return cgs;
}

CDA_CustomGenerator::CDA_CustomGenerator
(
 iface::cellml_api::Model* aModel,
 iface::cellml_services::MaLaESTransform* aTransform,
 iface::cellml_services::CeVAS* aCeVAS,
 iface::cellml_services::CUSES* aCUSES,
 iface::cellml_services::AnnotationSet* aAnnoSet,
 std::wstring& aStateVariableNamePattern,
 std::wstring& aAssignPattern,
 std::wstring& aSolvePattern,
 std::wstring& aSolveNLSystemPattern,
 uint32_t aArrayOffset
)
  : _cda_refcount(1), mModel(aModel),
    mTransform(aTransform), mCeVAS(aCeVAS), mCUSES(aCUSES), mAnnoSet(aAnnoSet),
    mStateVariableNamePattern(aStateVariableNamePattern), mAssignPattern(aAssignPattern),
    mSolvePattern(aSolvePattern), mSolveNLSystemPattern(aSolveNLSystemPattern),
    mArrayOffset(aArrayOffset)
{
}

void
CDA_CustomGenerator::indexTargets()
{
  mTargetSet.insert(mTargets.begin(), mTargets.end());
}

iface::cellml_services::ComputationTargetIterator*
CDA_CustomGenerator::iterateTargets()
  throw()
{
  return new CDA_ComputationTargetIterator(this);
}

void
CDA_CustomGenerator::requestComputation(iface::cellml_services::ComputationTarget* aTarget)
  throw(std::exception&)
{
  std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::
    iterator i = mTargetSet.find(aTarget);

  if (i == mTargetSet.end())
    throw iface::cellml_api::CellMLException();

  mRequestComputation.insert(*i);
}

void
CDA_CustomGenerator::markAsKnown(iface::cellml_services::ComputationTarget* aTarget)
  throw(std::exception&)
{
  std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::
    iterator i = mTargetSet.find(aTarget);

  if (i == mTargetSet.end())
    throw iface::cellml_api::CellMLException();

  mKnown.insert(*i);
}

void
CDA_CustomGenerator::markAsUnwanted(iface::cellml_services::ComputationTarget* aTarget)
  throw(std::exception&)
{
  std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::
    iterator i = mTargetSet.find(aTarget);

  if (i == mTargetSet.end())
    throw iface::cellml_api::CellMLException();

  mUnwanted.insert(*i);
}

iface::cellml_services::CustomCodeInformation*
CDA_CustomGenerator::generateCode()
  throw(std::exception&)
{
  std::wstring emp;

  CodeGenerationState cgs(mModel, emp, mStateVariableNamePattern, emp, emp, emp,
                          mAssignPattern, emp,
                          mSolvePattern, mSolveNLSystemPattern,
                          emp, emp, emp, emp, emp, emp, emp, emp, emp, false,
                          mArrayOffset, mTransform, mCeVAS, mCUSES, mAnnoSet, false);
  return cgs.GenerateCustomCode(mTargetSet, mRequestComputation, mKnown,
                                mUnwanted);
}

bool
CDA_CodeGenerator::allowPassthrough() throw()
{
  return mAllowPassthrough;
}

void
CDA_CodeGenerator::allowPassthrough(bool aAllowPassthrough) throw()
{
  mAllowPassthrough = aAllowPassthrough;
}

iface::cellml_services::CodeGeneratorBootstrap*
CreateCodeGeneratorBootstrap(void)
{
  return new CDA_CodeGeneratorBootstrap();
}
