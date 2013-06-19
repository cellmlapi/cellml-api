#define MODULE_CONTAINS_CCGS
#include "CCGSImplementation.hpp"
#include <map>
#include <set>
#include <iterator>
#include <cmath>
#include "CodeGenerationState.hxx"
#include <stdio.h>
#include "CodeGenerationError.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "DOMBootstrap.hxx"
#include <assert.h>
#include <algorithm>
#ifdef ENABLE_RDF
#include "IfaceRDF_APISPEC.hxx"
#include "RDFBootstrap.hpp"
#endif

// Asymptotic complexity is bound around O(n^(((t^2)+t)/2)) for t=SEARCH_DEPTH. This is a pretty poor upper bound.
#define SEARCH_DEPTH 3
#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"

CodeGenerationState::~CodeGenerationState()
{
  for (std::list<ptr_tag<MathStatement> >::iterator i = mMathStatements.begin();
       i != mMathStatements.end(); i++)
    delete *i;

  for (std::set<std::pair<ptr_tag<CDA_ComputationTarget>, ptr_tag<MathStatement> > >::iterator i = mResets.begin();
       i != mResets.end(); i++)
    delete (*i).second;

  for (std::list<System*>::iterator i = mSystems.begin();
       i != mSystems.end(); i++)
    delete *i;
}

already_AddRefd<iface::cellml_services::CustomGenerator>
CodeGenerationState::CreateCustomGenerator()
{
  // Create a new code information object...
  mCodeInfo = already_AddRefd<CDA_CodeInformation>
    (new CDA_CodeInformation());
  RETURN_INTO_WSTRING(cevError, mCeVAS->modelError());
  if (cevError != L"")
    throw CodeGenerationError(cevError);
  RETURN_INTO_WSTRING(cusesError, mCUSES->modelError());
  if (cusesError != L"")
    throw CodeGenerationError(cusesError);
  
  // Create all computation targets...
  CreateBaseComputationTargets();
  
  // Create all variable edges (this creates derived computation targets as a
  //   side-effect)...
  CreateMathStatements();

  RETURN_INTO_OBJREF(cg, CDA_CustomGenerator,
                     new CDA_CustomGenerator
                     (mModel, mTransform, mCeVAS, mCUSES, mAnnoSet,
                      mStateVariableNamePattern, mAssignPattern,
                      mAssignConstantPattern,
                      mSolvePattern, mSolveNLSystemPattern, mArrayOffset));

  // Now copy mCodeInfo->mTargets
  cg->mTargets.insert(cg->mTargets.end(), mCodeInfo->mTargets.begin(),
                      mCodeInfo->mTargets.end());
  cg->indexTargets();
  // Wipe them from mCodeInfo or they will be destroyed.
  mCodeInfo->mTargets.clear();

  cg->add_ref();
  return already_AddRefd<iface::cellml_services::CustomGenerator>(cg);
}

already_AddRefd<iface::cellml_services::CustomCodeInformation>
CodeGenerationState::GenerateCustomCode
(
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aTargetSet,
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aUserWanted,
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aUserKnown,
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aUserUnwanted
)
{
  RETURN_INTO_OBJREF(cci, CDA_CustomCodeInformation, new CDA_CustomCodeInformation());

  try
  {
    // Create a new code information object...
    mCodeInfo = already_AddRefd<CDA_CodeInformation>
      (new CDA_CodeInformation());
    mCodeInfo->mRateIndexCount = 0;
    RETURN_INTO_WSTRING(cevError, mCeVAS->modelError());
    if (cevError != L"")
      throw CodeGenerationError(cevError);
    RETURN_INTO_WSTRING(cusesError, mCUSES->modelError());
    if (cusesError != L"")
      throw CodeGenerationError(cusesError);
    
    // Create all computation targets...
    CreateBaseComputationTargets();
    
    // Create all variable edges (this creates derived computation targets as a
    //   side-effect)...
    CreateMathStatements();
    SplitPiecewiseByResetRule();

    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator i =
           mLocallyBoundTargs.begin(); i != mLocallyBoundTargs.end();
         i++)
      (*i)->mEvaluationType = iface::cellml_services::LOCALLY_BOUND;

    mUnusedMathStatements.insert(mMathStatements.begin(), mMathStatements.end());

    // Translate from external computation targets to internal ones...
    std::set<ptr_tag<CDA_ComputationTarget> > wanted, known, unwanted;
    MapExternalTargetsToInternal(aTargetSet,
                                 aUserWanted, aUserKnown, aUserUnwanted,
                                 wanted, known, unwanted);

    ClassifyAndBuildFloatingForCustom(wanted, known, unwanted);
    
    std::list<System*> systems;
    // Now, find everything computable from the information the user provided.
    DecomposeIntoSystems(known, mFloating, unwanted, systems, true);

    std::map<ptr_tag<CDA_ComputationTarget>, System*> sysByTargReq;
    // Build an index from variables required to systems...
    BuildSystemsByTargetsRequired(systems, sysByTargReq);

    // Work out what systems we need to solve...
    std::list<System*> neededSys;
    std::set<ptr_tag<CDA_ComputationTarget> > known2(known);
    FindSystemsNeededForTargets(sysByTargReq, wanted, true, known2, neededSys);

    for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i =
           mCodeInfo->mTargets.begin();
         i != mCodeInfo->mTargets.end();
         i++)
    {
      if ((*i)->mEvaluationType != iface::cellml_services::FLOATING &&
          (*i)->mEvaluationType != iface::cellml_services::PSEUDOSTATE_VARIABLE)
      {
        std::wstring name;
        AllocateStateVariable(*i, name);
      }
    }

    // Generate code for wanted and the prerequisites...
    GenerateCodeForSet(cci->mGeneratedCode, known, neededSys, sysByTargReq);
    cci->mFunctionsString = mCodeInfo->mFuncsStr;

    if (mUnusedMathStatements.empty())
      cci->mConstraintLevel = iface::cellml_services::CORRECTLY_CONSTRAINED;
    else
      cci->mConstraintLevel = iface::cellml_services::OVERCONSTRAINED;

    cci->mIndexCount = mCodeInfo->mRateIndexCount;

    for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i =
           mCodeInfo->mTargets.begin();
         i != mCodeInfo->mTargets.end();
         i++)
      if ((*i)->mEvaluationType == iface::cellml_services::PSEUDOSTATE_VARIABLE)
      {
        if (cci->mConstraintLevel == iface::cellml_services::CORRECTLY_CONSTRAINED)
          cci->mConstraintLevel = iface::cellml_services::UNDERCONSTRAINED;
        else if (cci->mConstraintLevel == iface::cellml_services::OVERCONSTRAINED)
          cci->mConstraintLevel = iface::cellml_services::UNSUITABLY_CONSTRAINED;
      }
  }
  catch (...)
  {
    cci->mConstraintLevel = iface::cellml_services::UNSUITABLY_CONSTRAINED;
  }

  cci->mTargets.insert(cci->mTargets.end(),
                       mCodeInfo->mTargets.begin(), mCodeInfo->mTargets.end());
  mCodeInfo->mTargets.clear();

  cci->add_ref();
  return already_AddRefd<iface::cellml_services::CustomCodeInformation>(cci);
}

void
CodeGenerationState::ClassifyAndBuildFloatingForCustom
(
 std::set<ptr_tag<CDA_ComputationTarget> >& wanted,
 std::set<ptr_tag<CDA_ComputationTarget> >& known,
 std::set<ptr_tag<CDA_ComputationTarget> >& unwanted
)
{
  mFloating.clear();
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mCodeInfo->mTargets.begin();
       i != mCodeInfo->mTargets.end();
       i++)
  {
    if (unwanted.count(*i))
    {
      (*i)->mEvaluationType = iface::cellml_services::FLOATING;
      continue;
    }

    if (known.count(*i))
    {
      (*i)->mEvaluationType = iface::cellml_services::VARIABLE_OF_INTEGRATION;
      continue;
    }

    if (wanted.count(*i))
      (*i)->mEvaluationType = iface::cellml_services::STATE_VARIABLE;

    if (!known.count(*i) && !unwanted.count(*i))
      mFloating.insert(*i);
  }
}

void
CodeGenerationState::MapExternalTargetsToInternal
(
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExTargetSet,
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExWanted,
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExKnown,
 std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExUnwanted,
 std::set<ptr_tag<CDA_ComputationTarget> >& aInWanted,
 std::set<ptr_tag<CDA_ComputationTarget> >& aInKnown,
 std::set<ptr_tag<CDA_ComputationTarget> >& aInUnwanted
)
{
  // Build a map from (Variable, DegreeInt) -> ptr_tag<newly created CDA_ComputationTarget>
  std::map<std::pair<iface::cellml_api::CellMLVariable*, uint32_t>, ptr_tag<CDA_ComputationTarget> >
    toNewCTMap;
  for (
       std::list<ptr_tag<CDA_ComputationTarget> >::iterator i(mCodeInfo->mTargets.begin());
       i != mCodeInfo->mTargets.end();
       i++
      )
    toNewCTMap.insert(
                      std::pair<std::pair<iface::cellml_api::CellMLVariable*, uint32_t>,
                                ptr_tag<CDA_ComputationTarget> >
                      (
                       std::pair<iface::cellml_api::CellMLVariable*, uint32_t>
                       ((*i)->mVariable, (*i)->mDegree),
                       *i
                      )
                     );

  // Now map from the old ComputationTarget* to ptr_tag<newly created CDA_ComputationTarget>
  std::map<iface::cellml_services::ComputationTarget*,
           ptr_tag<CDA_ComputationTarget> > oldTargetToNewTarget;
  for (
       std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::
         iterator i = aExTargetSet.begin();
       i != aExTargetSet.end();
       i++
      )
  {
    std::map<std::pair<iface::cellml_api::CellMLVariable*, uint32_t>,
             ptr_tag<CDA_ComputationTarget> >::iterator j =
      toNewCTMap.find(std::pair<iface::cellml_api::CellMLVariable*, uint32_t>
                      (static_cast<CDA_ComputationTarget*>(*i)->mVariable,
                       static_cast<CDA_ComputationTarget*>(*i)->mDegree));
    if (j == toNewCTMap.end())
      continue; // XXX model must have changed. Consider throwing an exception?
    oldTargetToNewTarget.insert(
                                std::pair<iface::cellml_services::ComputationTarget*,
                                          ptr_tag<CDA_ComputationTarget> >
                                (*i, (*j).second)
                               );
  }

  // Now build known, wanted, unwanted...
  for (std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::iterator i = aExWanted.begin();
       i != aExWanted.end(); i++)
  {
    std::map<iface::cellml_services::ComputationTarget*, ptr_tag<CDA_ComputationTarget> >::iterator j =
      oldTargetToNewTarget.find(unsafe_dynamic_cast<CDA_ComputationTarget*>(*i));
    aInWanted.insert((*j).second);
  }

  for (std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::iterator i = aExKnown.begin();
       i != aExKnown.end(); i++)
  {
    std::map<iface::cellml_services::ComputationTarget*, ptr_tag<CDA_ComputationTarget> >::iterator j =
      oldTargetToNewTarget.find(unsafe_dynamic_cast<CDA_ComputationTarget*>(*i));
    aInKnown.insert((*j).second);
  }

  for (std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>::iterator i = aExUnwanted.begin();
       i != aExUnwanted.end(); i++)
  {
    std::map<iface::cellml_services::ComputationTarget*, ptr_tag<CDA_ComputationTarget> >::iterator j =
      oldTargetToNewTarget.find(unsafe_dynamic_cast<CDA_ComputationTarget*>(*i));
    aInUnwanted.insert((*j).second);
  }  
}


already_AddRefd<iface::cellml_services::IDACodeInformation>
CodeGenerationState::GenerateCode()
{
  // Create a new code information object...
  mCodeInfo = already_AddRefd<CDA_CodeInformation>
    (new CDA_CodeInformation());
  mCodeInfo->mConstraintLevel = iface::cellml_services::CORRECTLY_CONSTRAINED;
  mCodeInfo->mRateIndexCount = 0;
  mCodeInfo->mConstantIndexCount = 0;
  mCodeInfo->mAlgebraicIndexCount = 0;
  mCodeInfo->mConditionVariableCount = 0;
  
  try
  {
    RETURN_INTO_WSTRING(cevError, mCeVAS->modelError());
    if (cevError != L"")
      throw CodeGenerationError(cevError);
    RETURN_INTO_WSTRING(cusesError, mCUSES->modelError());
    if (cusesError != L"")
      throw CodeGenerationError(cusesError);
    
    // Create all computation targets...
    CreateBaseComputationTargets();
    
    // Create all variable edges (this creates derived computation targets as a
    //   side-effect)...
    CreateMathStatements();
    if (!mIDAStyle)
      SplitPiecewiseByResetRule();
    
    // Next, set starting classification for all targets...
    FirstPassTargetClassification();
    
    mUnusedMathStatements.insert(mMathStatements.begin(), mMathStatements.end());
    
    // Put all targets into lists based on their classification...
    BuildFloatingAndConstantLists();
    
    std::list<System*> systems;
    
    // Now, determine all constants computable from the current constants...
    DecomposeIntoSystems(mKnown, mFloating, mUnwanted, systems, true);
    CheckInappropriateStateAssignments(systems);
    mUnwanted.clear();
    
    // Assign constant variables for set...
    AllocateVariablesInSet(systems, iface::cellml_services::CONSTANT,
                           mConstantPattern, mNextConstantIndex,
                           mCodeInfo->mConstantIndexCount);
    
    // Allocate temporary names in constants for the rates...
    AllocateRateNamesAsConstants(systems);
    
    std::map<ptr_tag<CDA_ComputationTarget>, System*> sysByTargReq;
    // Build an index from variables required to systems...
    BuildSystemsByTargetsRequired(systems, sysByTargReq);
    if (!mIDAStyle)
      CloneNamesIntoDelayedNames();
    
    ProcessModellerSuppliedIVHints();
    
    // Write evaluations for all constants we just worked out how to compute...
    std::wstring tmp;
    mIsConstant = true;
    GenerateCodeForSet(tmp, mKnown, systems, sysByTargReq);
    mIsConstant = false;
    mCodeInfo->mInitConstsStr += tmp;
    
    // Also we need to initialise state variable IVs...
    systems.clear();
    
    BuildStateAndConstantLists();
    DecomposeIntoSystems(mKnown, mFloating, mUnwanted, systems);
    CheckInappropriateStateAssignments(systems);
    BuildSystemsByTargetsRequired(systems, sysByTargReq);
    CheckStateVariableIVConstraints(systems);
    
    tmp = L"";
    mIsConstant = true;
    GenerateCodeForSet(tmp, mKnown, systems, sysByTargReq);
    mIsConstant = false;
    mCodeInfo->mInitConstsStr += tmp;
      
    if (mIDAStyle)
      IDAStyleCodeGeneration();
    else
      ODESolverStyleCodeGeneration();
  }
  catch (UnderconstrainedError uce)
  {
    mCodeInfo->mConstraintLevel = iface::cellml_services::UNDERCONSTRAINED;
  }
  catch (UnderconstrainedIVError uce)
  {
    mCodeInfo->mConstraintLevel = iface::cellml_services::UNDERCONSTRAINED;
    mCodeInfo->mMissingInitial = uce.mCT;
  }
  catch (OverconstrainedError oce)
  {
    std::vector<iface::dom::Element*>::iterator fei;
    for (fei = mCodeInfo->mFlaggedEquations.begin();
         fei != mCodeInfo->mFlaggedEquations.end();
         fei++)
      (*fei)->release_ref();
    mCodeInfo->mFlaggedEquations.clear();
    
    if (oce.mEqn)
    {
      oce.mEqn->add_ref();
      mCodeInfo->mFlaggedEquations.push_back(oce.mEqn);
    }
    
    mCodeInfo->mConstraintLevel = iface::cellml_services::OVERCONSTRAINED;
  }
  catch (UnsuitablyConstrainedError uce)
  {
    std::vector<iface::dom::Element*>::iterator fei;
    for (fei = mCodeInfo->mFlaggedEquations.begin();
         fei != mCodeInfo->mFlaggedEquations.end();
         fei++)
      (*fei)->release_ref();
    mCodeInfo->mFlaggedEquations.clear();
    
    std::set<ptr_tag<MathStatement> >::iterator uel;
    for (uel = mUnusedMathStatements.begin(); uel != mUnusedMathStatements.end(); uel++)
    {
      // If there was an initial value that was set to a variable which could not
      // be computed, it will be an unused edge....
      MathStatement * ms = *uel;
      if (ms->mType != MathStatement::INITIAL_ASSIGNMENT)
      {
        iface::mathml_dom::MathMLElement* me = static_cast<MathMLMathStatement*>(ms)->mMaths;
        me->add_ref();
        mCodeInfo->mFlaggedEquations.push_back(me);
      }
    }
    
    mCodeInfo->mConstraintLevel = iface::cellml_services::UNSUITABLY_CONSTRAINED;
  }
  catch (CodeGenerationError cge)
  {
    mCodeInfo->mErrorMessage = cge.str();
  }
    
  mCodeInfo->add_ref();
  return already_AddRefd<iface::cellml_services::IDACodeInformation>(mCodeInfo);
}

#define HINTS_NS L"http://www.cellml.org/metadata/simulation/solverhints/1.0#"

void
CodeGenerationState::TransformPiecewiseConditions()
{
  // Go through all unused math statements, and look for any form of piecewise to replace...
  for (std::set<ptr_tag<MathStatement> >::iterator i = mUnusedMathStatements.begin();
       i != mUnusedMathStatements.end(); i++)
    TransformPiecewiseStatement(*i);
}

void
CodeGenerationState::TransformPiecewiseStatement(MathStatement* aStatement)
{
  switch (aStatement->mType)
  {
  case MathStatement::EQUATION:
      {
        Equation* eq = static_cast<Equation*>(aStatement);

        RETURN_INTO_OBJREF(nlhs, iface::dom::Node, eq->mLHS->cloneNode(true));
        QUERY_INTERFACE(eq->mLHS, nlhs, mathml_dom::MathMLElement);

        RETURN_INTO_OBJREF(nrhs, iface::dom::Node, eq->mRHS->cloneNode(true));
        QUERY_INTERFACE(eq->mRHS, nrhs, mathml_dom::MathMLElement);
        
        TransformPiecewisesInMaths(eq->mLHS, eq->mContext);
        TransformPiecewisesInMaths(eq->mRHS, eq->mContext);
      }
      break;
  case MathStatement::INEQUALITY:
      {
        Inequality* ieq = static_cast<Inequality*>(aStatement);
        RETURN_INTO_OBJREF(mm, iface::dom::Node, ieq->mMaths->cloneNode(true));
        QUERY_INTERFACE(ieq->mMaths, mm, mathml_dom::MathMLElement);
        TransformPiecewisesInMaths(ieq->mMaths, ieq->mContext);
      }
      break;
  case MathStatement::PIECEWISE:
      {
        Piecewise* pw = static_cast<Piecewise*>(aStatement);
        for (std::list<std::pair<ptr_tag<Equation>,
                       ptr_tag<MathMLMathStatement> > >::iterator i =
               pw->mPieces.begin();
             i != pw->mPieces.end(); i++)
        {
          TransformPiecewiseStatement((*i).first);
          // This matters if the condition contains further piecewises.
          TransformPiecewiseStatement((*i).second);

          TransformCaseCondition((*i).second->mMaths, (*i).second->mContext);
        }
      }
      break;
  default:
      ; // Nothing to do.
  }
}

void
CodeGenerationState::TransformPiecewisesInMaths(iface::mathml_dom::MathMLElement* aChange,
                                                iface::cellml_api::CellMLComponent* aContext)
{
  // Do a DFS, post-order search for piecewise elements...
  RETURN_INTO_OBJREF(c, iface::dom::Node, aChange->firstChild());
  for (; c != NULL; c = already_AddRefd<iface::dom::Node>(c->nextSibling()))
  {
    DECLARE_QUERY_INTERFACE_OBJREF(me, c, mathml_dom::MathMLElement);
    if (me == NULL)
      continue;

    TransformPiecewisesInMaths(me, aContext);
  }

  // See if this *is* a piecewise...
  DECLARE_QUERY_INTERFACE_OBJREF(pw, aChange, mathml_dom::MathMLPiecewiseElement);
  if (pw == NULL)
    return;

  for (uint32_t i = 1; ; i++)
  {
    RETURN_INTO_OBJREF(pwc, iface::mathml_dom::MathMLCaseElement, pw->getCase(i));
    if (pwc == NULL)
      break;

    RETURN_INTO_OBJREF(pwcc, iface::mathml_dom::MathMLContentElement,
                       pwc->caseCondition());
    TransformCaseCondition(pwcc, aContext);
  }
}

void
CodeGenerationState::TransformCaseCondition(iface::mathml_dom::MathMLElement* aEl,
                                            iface::cellml_api::CellMLComponent* aContext)
{
  // We only deal with apply here, because cis & constants don't need to be
  // processed, and nested piecewises have already been processed, and
  // we can't interpret csymbol elements in general.
  DECLARE_QUERY_INTERFACE_OBJREF(ap, aEl, mathml_dom::MathMLApplyElement);
  if (ap == NULL)
    return;

  RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement, ap->_cxx_operator());
  RETURN_INTO_WSTRING(opname, op->localName());
  if (opname == L"and" || opname == L"not" || opname == L"or" || opname == L"xor")
  {
    // Logical operation.
    uint32_t nargs = ap->nArguments();
    for (uint32_t i = 2; i <= nargs; i++)
    {
      RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, ap->getArgument(i));
      TransformCaseCondition(arg, aContext);
    }

    return;
  }

  // For now, we aren't doing anything with eq or neq. The reason is such a
  // condition is only useful if the value is equal for a finite amount of time,
  // as is the case with a finite state variable, and we don't want to add such
  // variables for rootfinding.
  if (opname == L"geq" || opname == L"gt" || opname == L"leq" ||
      opname == L"lt")
  {
    RETURN_INTO_OBJREF(withMinusN, iface::dom::Node,
                       ap->cloneNode(true));
    DECLARE_QUERY_INTERFACE_OBJREF(withMinus, withMinusN, mathml_dom::MathMLApplyElement);
    RETURN_INTO_OBJREF(doc, iface::dom::Document, withMinus->ownerDocument());
    RETURN_INTO_OBJREF(minusEl, iface::dom::Element,
                       doc->createElementNS(MATHML_NS, L"minus"));
    DECLARE_QUERY_INTERFACE_OBJREF(minus, minusEl, mathml_dom::MathMLElement);
    withMinus->_cxx_operator(minus);

    std::wstring str;
    GenerateVariableName(str, mConditionVariablePattern, mNextConditionVariable++);
    mCodeInfo->mConditionVariableCount++;

    withMinus->add_ref();
    mRootInformation.push_back(RootInformation
                               (str, aContext, withMinus));

    // Make a passthrough csymbol for the allocated symbol.
    RETURN_INTO_OBJREF(ptEl, iface::dom::Element,
                       doc->createElementNS(MATHML_NS, L"csymbol"));
    DECLARE_QUERY_INTERFACE_OBJREF(pt, ptEl, mathml_dom::MathMLCsymbolElement);
    pt->definitionURL(L"http://www.cellml.org/tools/api#passthrough");
    str = mTransform->wrapNumber(str);
    RETURN_INTO_OBJREF(pttn, iface::dom::Text, doc->createTextNode(str.c_str()));
    pt->appendChild(pttn)->release_ref();

    // Now we make the part we retain for the conditional:
    // First part is the condition variable...
    ap->setArgument(pt, 2)->release_ref();

    // Second part of the inequality goes to equal...

    RETURN_INTO_OBJREF(zeroEl, iface::dom::Element,
                       doc->createElementNS(MATHML_NS, L"cn"));
    RETURN_INTO_OBJREF(tn, iface::dom::Text, doc->createTextNode(L"0"));
    zeroEl->appendChild(tn)->release_ref();
    DECLARE_QUERY_INTERFACE_OBJREF(zero, zeroEl, mathml_dom::MathMLElement);
    ap->setArgument(zero, 3)->release_ref();
  }
}

void
CodeGenerationState::GenerateRootInformation()
{
  while (!mRootInformation.empty())
  {
    RootInformation p = mRootInformation.front();
    mRootInformation.pop_front();
    RETURN_INTO_OBJREF(math, iface::mathml_dom::MathMLContentElement, p.mathEl);

    RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, math, p.component,
                                             NULL, NULL, 0));
    RETURN_INTO_WSTRING(exp, mr->expression());
    std::wstring elName = p.mathEl->getAttribute(L"id");
    if (elName == L"")
      elName = L"unnamed element";
    AppendAssign(mCodeInfo->mRootInformationStr, p.storageName, exp,
                 L"piecewise condition for " + elName);
    uint32_t l = mr->supplementariesLength(), i;
    for (i = 0; i < l; i++)
    {
      RETURN_INTO_WSTRING(s, mr->getSupplementary(i));
      mCodeInfo->mFuncsStr += s + L"\r\n";
    }
  }
}

void
CodeGenerationState::CheckInappropriateStateAssignments(std::list<System*>& aSystems)
{
  for (std::list<System*>::iterator i = aSystems.begin(); i != aSystems.end(); i++)
  {
    bool hasState = false;
    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mUnknowns.begin();
         j != (*i)->mUnknowns.end(); j++)
      if ((*j)->type() == iface::cellml_services::STATE_VARIABLE)
      {
        hasState = true;
        break;
      }
    // If there is a state variable computed by this system, it must be an initial assignment...
    if (hasState && ((*i)->mMathStatements.size() != 1 ||
        (*((*i)->mMathStatements.begin()))->mType !=
                     MathStatement::INITIAL_ASSIGNMENT))
      throw OverconstrainedError
        ((static_cast<MathMLMathStatement*>(static_cast<MathStatement*>(*((*i)->mMathStatements.begin()))))
         ->mMaths);
  }
}

void
CodeGenerationState::IDAStyleCodeGeneration()
{
  if (mTrackPiecewiseConditions)
    TransformPiecewiseConditions();

  // We now want to identify variables which are currently marked as floating,
  // which can be computed from the constants, states, and VOI. However, we are
  // not aiming to solve systems here...
  
  // Put all targets into lists based on their classification...
  BuildFloatingAndKnownLists(false);

  // Now, determine all algebraic variables / rates computable from the
  // known variables (constants, state variables, and bound variable)...
  std::list<System*> algebraicSystems;
  
  DecomposeIntoAssignments(mKnown, mFloating, mUnwanted, algebraicSystems);
  // Assign algebraic variables for set...
  AllocateVariablesInSet(algebraicSystems, iface::cellml_services::ALGEBRAIC,
                         mAlgebraicVariableNamePattern,
                         mNextAlgebraicVariableIndex,
                         mCodeInfo->mAlgebraicIndexCount);

  // Build an index from variables required to systems...
  std::map<ptr_tag<CDA_ComputationTarget>, System*> sysByTargReq;
  BuildSystemsByTargetsRequired(algebraicSystems, sysByTargReq);
  
  // Now we need to assign everything else as a pseudo-state variable...
  MarkRemainingVariablesAsPseudoState();
  InitialisePseudoStates(mCodeInfo->mInitConstsStr);

  std::list<System*> essentialSystems;
  FindSystemsForResiduals(algebraicSystems, essentialSystems);
  GenerateCodeForSet(mCodeInfo->mEssentialVarsStr, mKnown, essentialSystems, sysByTargReq);
  GenerateCodeForSet(mCodeInfo->mVarsStr, mKnown, algebraicSystems, sysByTargReq);

  // Now, generate residuals for all state and pseudostate variables...
  std::set<ptr_tag<CDA_ComputationTarget> > aNeeded;
  GenerateResiduals(mCodeInfo->mRatesStr);

  // Generate the code to set-up an array distinguishing state from pseudostate...
  GenerateStateInformation(mCodeInfo->mStateInformationStr);
  GenerateRootInformation();
}

void
CodeGenerationState::GenerateStateInformation(std::wstring& aStr)
{
  CNumericLocale localeObj; // Scoped locale change...
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mBaseTargets.begin();
       i != mBaseTargets.end();
       i++)
  {
    for (CDA_ComputationTarget* ct = *i; ct != NULL; ct = ct->mUpDegree)
    {
      wchar_t id[32];
      any_swprintf(id, 32, L"%lu", ct->mAssignedIndex);
      if (ct->mEvaluationType == iface::cellml_services::STATE_VARIABLE)
      {
        aStr += ReplaceIDs(mConstrainedRateStateInfoPattern, id, L"", L"");
      }
      else if (ct->mEvaluationType == iface::cellml_services::PSEUDOSTATE_VARIABLE)
      {
        aStr += ReplaceIDs(mUnconstrainedRateStateInfoPattern, id, L"", L"");
      }
    }
  }
}

double
CodeGenerationState::GetPseudoStateIV(ptr_tag<CDA_ComputationTarget> aCT)
{
  std::map<ptr_tag<CDA_ComputationTarget>, double>::iterator i =
    mInitialOverrides.find(aCT);

  if (i == mInitialOverrides.end())
    // Default to this (non-zero and positive is often best for many real-world models).
    return 0.1001;

  return (*i).second;
}

void
CodeGenerationState::InitialisePseudoStates(std::wstring& aCode)
{
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i =
         mCodeInfo->mTargets.begin();
       i != mCodeInfo->mTargets.end(); i++)
    if ((*i)->mEvaluationType == iface::cellml_services::PSEUDOSTATE_VARIABLE ||
        ((*i)->mEvaluationType != iface::cellml_services::CONSTANT &&
         (*i)->mDegree > 0))
    {
      double iv = GetPseudoStateIV(*i);
      wchar_t ivv[30];
      any_swprintf(ivv, 30, L"%g", iv);
      RETURN_INTO_WSTRING(n, (*i)->name());
      AppendConstantAssign(aCode, n, mTransform->wrapNumber(ivv), (*i)->mVariable->name());
    }
}

void
CodeGenerationState::FindSystemsForResiduals
(
 std::list<System*>& aSystems,
 std::list<System*>& aSysForResid
)
{
  // Go through unused mathstatements looking for state or pseudostate variables, and add all
  // co-occuring variables to the set...
  std::set<ptr_tag<CDA_ComputationTarget> > sysvars;
  for (std::set<ptr_tag<MathStatement> >::iterator i = mUnusedMathStatements.begin();
       i != mUnusedMathStatements.end();
       i++)
  {
    bool isState = false;
    for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mTargets.begin();
         j != (*i)->mTargets.end();
         j++)
      if ((*j)->mEvaluationType == iface::cellml_services::STATE_VARIABLE ||
          (*j)->mEvaluationType == iface::cellml_services::PSEUDOSTATE_VARIABLE ||
          (*j)->mDegree > 0)
      {
        isState = true;
        break;
      }

    if (isState)
    {
      for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mTargets.begin();
           j != (*i)->mTargets.end();
           j++)
        sysvars.insert(*j);
    }
  }

  for (std::list<System*>::iterator i = aSystems.begin();
       i != aSystems.end();
       i++)
  {
    bool hasTarg = false;
    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mUnknowns.begin();
         j != (*i)->mUnknowns.end();
         j++)
      if (sysvars.count(*j))
      {
        hasTarg = true;
        break;
      }

    if (hasTarg)
      aSysForResid.push_back(*i);
  }
}

void
CodeGenerationState::MarkRemainingVariablesAsPseudoState()
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator i;
  uint32_t count = 0;
  for (i = mCodeInfo->mTargets.begin(); i != mCodeInfo->mTargets.end(); i++)
  {
    if ((*i)->mEvaluationType == iface::cellml_services::FLOATING)
    {
      if ((*i)->mDegree == 0)
      {
        count++;
        (*i)->mEvaluationType = iface::cellml_services::PSEUDOSTATE_VARIABLE;
        RETURN_INTO_WSTRING(n, (*i)->name());
        std::wstring str;
        if (n == L"")
        {
          uint32_t idx = mNextStateVariableIndex;
          AllocateStateVariable(*i, str);
          AllocateDelayed(*i, mInfDelayedStatePattern, idx);
        }
        mKnown.insert(*i);
      }
      else
      {
        count++;
        (*i)->mEvaluationType = iface::cellml_services::ALGEBRAIC;
      }
    }
  }

  // XXX Do we want to exclude inequalities from the math statements count?
  if (count > mUnusedMathStatements.size())
    throw UnderconstrainedError();
  else if (count < mUnusedMathStatements.size())
    throw OverconstrainedError(NULL);
}

void
CodeGenerationState::ODESolverStyleCodeGeneration()
{
  // Put all targets into lists based on their classification...
  BuildFloatingAndKnownLists();
  
  // Now, determine all algebraic variables / rates computable from the
  // known variables (constants, state variables, and bound variable)...
  std::list<System*> systems;
  
  bool wasError = DecomposeIntoSystems(mKnown, mFloating, mUnwanted, systems);
  CheckInappropriateStateAssignments(systems);
  MakeSystemsForResetRulesAndClearKnown(mResets, systems, mKnown, mFloating);
  std::map<ptr_tag<CDA_ComputationTarget>, System*> sysByTargReq;
  BuildSystemsByTargetsRequired(systems, sysByTargReq);
  
  // Assign algebraic variables for set...
  AllocateVariablesInSet(systems, iface::cellml_services::ALGEBRAIC,
                         mAlgebraicVariableNamePattern,
                         mNextAlgebraicVariableIndex,
                         mCodeInfo->mAlgebraicIndexCount);
  
  if (wasError)
  {
    if (mUnusedMathStatements.size() != 0)
    {
      if (mFloating.size() != 0)
        throw UnsuitablyConstrainedError();
      else
      {
        MathStatement * ms = *(mUnusedMathStatements.begin());
        if (ms->mType != MathStatement::INITIAL_ASSIGNMENT)
          throw OverconstrainedError
            ((static_cast<MathMLMathStatement*>(ms))->mMaths);
        // It is overconstrained because of an initial_value...
        throw OverconstrainedError(NULL);
      }
    }
    else
      throw UnderconstrainedError();
  }
  
  // Restore the saved rates...
  RestoreSavedRates(mCodeInfo->mRatesStr);
  
  // Write evaluations for all rates & algebraic variables in reachabletargets
  GenerateCodeForSetByType(mKnown, systems, sysByTargReq);
  
  // Also cascade state variables to rate variables where they are the same
  // (e.g. if d^2y/dx^2 = constant then dy/dx is a state variable due to the
  //  above equation, but also it is a rate for y).
  GenerateStateToRateCascades();
  
  GenerateInfDelayUpdates();
}

bool
CodeGenerationState::FindSystemsNeededForTargets
(
 const std::map<ptr_tag<CDA_ComputationTarget>, System*>&
   aSysByTargReq,
 const std::set<ptr_tag<CDA_ComputationTarget> >& aWantedTargets,
 bool aMarkOnly,
 std::set<ptr_tag<CDA_ComputationTarget> >& aKnownTargets,
 std::list<System*>& aNeededSystems
)
{
  for (std::set<ptr_tag<CDA_ComputationTarget> >::const_iterator i = aWantedTargets.begin();
       i != aWantedTargets.end(); i++)
  {
    if (aKnownTargets.count(*i))
      continue;
    aKnownTargets.insert(*i);

    std::map<ptr_tag<CDA_ComputationTarget>, System*>::const_iterator j =
      aSysByTargReq.find(*i);
    if (j == aSysByTargReq.end())
    {
      if (aMarkOnly)
      {
        (*i)->mEvaluationType = iface::cellml_services::PSEUDOSTATE_VARIABLE;
        continue;
      }
      else
        return false;
    }

    System* s = (*j).second;

    // Recursively check the dependencies...
    if (!FindSystemsNeededForTargets(aSysByTargReq, (*j).second->mKnowns, false,
                                     aKnownTargets, aNeededSystems))
    {
      if (aMarkOnly)
      {
        (*i)->mEvaluationType = iface::cellml_services::PSEUDOSTATE_VARIABLE;
        continue;
      }
      else
        return false;
    }

    if ((*i)->mEvaluationType == iface::cellml_services::FLOATING)
      (*i)->mEvaluationType = iface::cellml_services::ALGEBRAIC;

    aNeededSystems.push_back(s);
  }

  return true;
}

void
CodeGenerationState::CheckStateVariableIVConstraints(const std::list<System*>& aSystems)
{
  // Step one: check aSystems for overconstraints...
  for (std::list<System*>::const_iterator i = aSystems.begin();
       i != aSystems.end();
       i++)
  {
    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j =
           (*i)->mUnknowns.begin();
         j != (*i)->mUnknowns.end();
         j++)
      if ((*j)->type() == iface::cellml_services::STATE_VARIABLE)
      {
        if ((*j)->mStateHasIV)
        {
          MathStatement *ms =
            *((*i)->mMathStatements.begin());
          if (ms->mType == MathStatement::INITIAL_ASSIGNMENT)
            throw OverconstrainedError(NULL);
          else
            throw OverconstrainedError((static_cast<MathMLMathStatement*>(ms))->mMaths);
        }
        (*j)->mStateHasIV = true;
      }
  }

  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mBaseTargets.begin();
       i != mBaseTargets.end(); i++)
  {
    for (CDA_ComputationTarget* ct = (*i); ct->mUpDegree; ct = ct->mUpDegree)
    {
      // If it has an updegree, it is a state variable, so it must have an IV...
      if (!ct->mStateHasIV)
      {
        ct->mEvaluationType = iface::cellml_services::FLOATING;
        throw UnderconstrainedIVError(ct);
      }
    }
  }
}

void
CodeGenerationState::GenerateResiduals
(
 std::wstring& aCode
)
{
  // It has already been checked that the number of math statements matches
  // the number of residuals (XXX this will change when we handle inequalities
  // properly - which need to be bundled on to another residual and not counted
  // as their own residual).
  uint32_t residNumber = mArrayOffset;
  MathStatement* lastResid = NULL;

  for (std::set<ptr_tag<MathStatement> >::iterator i = mUnusedMathStatements.begin();
       i != mUnusedMathStatements.end(); i++)
  {
    MathStatement* ms = *i;

    if (ms->mType == MathStatement::PIECEWISE)
    {
      Piecewise* pw = static_cast<Piecewise*>(ms);
      std::list<std::pair<std::wstring, std::wstring> > cases;

      std::set<ptr_tag<CDA_ComputationTarget> > undelayedInAll;

      for (std::list<std::pair<ptr_tag<Equation>,
                               ptr_tag<MathMLMathStatement> > >::iterator j = pw->mPieces.begin();
           j != pw->mPieces.end(); j++)
      {
        std::wstring st;
        GenerateResidualForEquation(st, residNumber, (*j).first);
        RETURN_INTO_OBJREF(mr,
                           iface::cellml_services::MaLaESResult,
                           mTransform->transform
                           (mCeVAS, mCUSES, mAnnoSet, (*j).second->mMaths, pw->mContext,
                            NULL, NULL, 0));
        RETURN_INTO_WSTRING(cond, mr->expression());

        cases.push_back(std::pair<std::wstring, std::wstring>(st, cond));
      }
      residNumber++;
      lastResid = ms;
      GenerateCasesIntoTemplate(aCode, cases);
    }
    else if (ms->mType == MathStatement::EQUATION)
    {
      GenerateResidualForEquation(aCode, residNumber, static_cast<Equation*>(ms));
      residNumber++;
    }
  }

  while (!mRateNameBackup.empty())
  {
    std::pair<ptr_tag<CDA_ComputationTarget>, std::wstring> p =
      mRateNameBackup.front();
    mRateNameBackup.pop_front();

    RETURN_INTO_WSTRING(constName, p.first->name());

    uint32_t index = p.first->assignedIndex();

    GenerateResidualForString(aCode, residNumber++,
                              mTransform->wrapNumber(p.second),
                              mTransform->wrapNumber(constName),
                              p.first->mVariable->name());
    p.first->setNameAndIndex(index, p.second.c_str());
  }

  // Count up the total number of state / pseudostates...
  uint32_t totalState = 0;
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i =
         mCodeInfo->mTargets.begin();
       i != mCodeInfo->mTargets.end(); i++)
    if ((*i)->mEvaluationType == iface::cellml_services::STATE_VARIABLE ||
        (*i)->mEvaluationType == iface::cellml_services::PSEUDOSTATE_VARIABLE)
      totalState++;

  if (residNumber > totalState)
  {
    if (lastResid == NULL ||
        lastResid->mType == MathStatement::INITIAL_ASSIGNMENT)
      throw OverconstrainedError(NULL);
    else
      throw OverconstrainedError((static_cast<MathMLMathStatement*>(lastResid))
                                   ->mMaths);
  }
  else if (residNumber < totalState)
    throw UnderconstrainedError();
}

static std::wstring describeMaths(MathStatement* ms)
{
  if (ms == NULL)
    return L"null math statement";
  if (ms->mType == MathStatement::INITIAL_ASSIGNMENT)
    return ms->mTargets.front()->name();
  MathMLMathStatement* mms =
    static_cast<MathMLMathStatement*>(static_cast<MathStatement*>(ms));
  if (mms->mMaths == NULL)
    return L"null maths";
  std::wstring idAttr = mms->mMaths->getAttribute(L"id");
  if (idAttr == L"")
    return L"Element with no id";
  return idAttr;
}

void
CodeGenerationState::GenerateResidualForEquation
(
 std::wstring& aCode,
 uint32_t aResidNo,
 Equation* aEq
)
{
  RETURN_INTO_OBJREF(mr1, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aEq->mLHS,
                                           aEq->mContext, NULL, NULL, 0));
  RETURN_INTO_OBJREF(mr2, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aEq->mRHS,
                                           aEq->mContext, NULL, NULL, 0));
  RETURN_INTO_WSTRING(e1, mr1->expression());
  RETURN_INTO_WSTRING(e2, mr2->expression());

  GenerateResidualForString(aCode, aResidNo, e1, e2,
                            describeMaths(aEq));
}

void
CodeGenerationState::GenerateResidualForString
(
 std::wstring& aAppendTo,
 uint32_t aResidNo,
 const std::wstring& aLHS,
 const std::wstring& aRHS,
 const std::wstring& aXmlId
)
{
  wchar_t residNo[30];
  any_swprintf(residNo, 30, L"%lu", aResidNo);

  size_t curIdx = 0;

  while (true)
  {
    size_t lIdx = mResidualPattern.find(L"<LHS>", curIdx),
      rIdx = mResidualPattern.find(L"<RHS>", curIdx),
      rnIdx = mResidualPattern.find(L"<RNO>", curIdx),
      iIdx = mResidualPattern.find(L"<XMLID>", curIdx);
    if ((lIdx == rIdx) && (lIdx == rnIdx) && (lIdx == iIdx) &&
        (lIdx == std::string::npos))
    {
      aAppendTo += mResidualPattern.substr(curIdx);
      return;
    }
    if (lIdx <= rIdx && lIdx <= rnIdx && lIdx <= iIdx)
    {
      aAppendTo += mResidualPattern.substr(curIdx, lIdx - curIdx);
      aAppendTo += aLHS;
      curIdx = lIdx + sizeof("<LHS>") - 1;
    }
    else if (rIdx <= rnIdx && rIdx <= iIdx)
    {
      aAppendTo += mResidualPattern.substr(curIdx, rIdx - curIdx);
      aAppendTo += aRHS;
      curIdx = rIdx + sizeof("<RHS>") - 1;
    }
    else if (rnIdx <= iIdx)
    {
      aAppendTo += mResidualPattern.substr(curIdx, rnIdx - curIdx);
      aAppendTo += residNo;
      curIdx = rnIdx + sizeof("<RNO>") - 1;
    }
    else
    {
      aAppendTo += mResidualPattern.substr(curIdx, iIdx - curIdx);
      aAppendTo += aXmlId;
      curIdx = iIdx + sizeof("<XMLID>") - 1;
    }
  }
}

void
CodeGenerationState::MakeSystemsForResetRulesAndClearKnown
(
 std::set<std::pair<ptr_tag<CDA_ComputationTarget>, ptr_tag<MathStatement> > >& aResets,
 std::list<System*>& aSystems,
 std::set<ptr_tag<CDA_ComputationTarget> >& aKnown,
 std::set<ptr_tag<CDA_ComputationTarget> >& aFloating
)
{
  for (
       std::set<std::pair<ptr_tag<CDA_ComputationTarget>, ptr_tag<MathStatement> > >::iterator i
         = aResets.begin();
       i != aResets.end();
       i++
      )
  {
    ptr_tag<CDA_ComputationTarget> ct = (*i).first;
    ptr_tag<MathStatement> ms = (*i).second;

    std::set<ptr_tag<MathStatement> > mss;
    mss.insert(ms);
    std::set<ptr_tag<CDA_ComputationTarget> > mk, mu;
    aKnown.erase(ct);
    aFloating.insert(ct);
    mu.insert(ct);
    std::set_difference(ms->mTargets.begin(), ms->mTargets.end(),
                        mu.begin(), mu.end(),
                        std::inserter(mk, mk.end()));

    System* syst = new System(mss, mk, mu);
    aSystems.push_back(syst);
    mSystems.push_back(syst);
  }
}

void
CodeGenerationState::SplitPiecewiseByResetRule()
{
  for (std::list<ptr_tag<MathStatement> >::iterator i = mMathStatements.begin();
       i != mMathStatements.end();
       i++)
  {
    MathStatement* ms = *i;
    if (ms->mType != MathStatement::PIECEWISE)
      continue;

    Piecewise* pw = static_cast<Piecewise*>(ms);

    std::set<ptr_tag<CDA_ComputationTarget> > undelayed_derivs;
    {
      assert(pw->mContext);
      RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                         mTransform->transform(mCeVAS, mCUSES, mAnnoSet, pw->mMaths,
                                               pw->mContext, NULL, NULL, 0));
      RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                         mr->iterateInvolvedVariablesByDegree());

      while (true)
      {
        RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable, dvi->nextDegreeVariable());
        if (dv == NULL)
          break;
        
        if (dv->degree() == 0)
          continue;
        if (!dv->appearedUndelayed())
          continue;
        
        RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                           dv->variable());
        
        std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >
          ::iterator  mi = mTargetsBySource.find(cv);
        undelayed_derivs.insert(GetTargetOfDegree((*mi).second, dv->degree()));
      }
    }

    if (undelayed_derivs.size() == 0)
      continue;

    std::set<ptr_tag<CDA_ComputationTarget> > undelayed_derivs_someloweronly;
    std::map<std::pair<ptr_tag<Equation>, std::string >, uint32_t > maxDegreeByEquationVariableID;
    for (
         std::list<std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> > >::iterator j =
           pw->mPieces.begin();
         j != pw->mPieces.end();
         j++
        )
    {
      RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                         mTransform->transform(mCeVAS, mCUSES, mAnnoSet, (*j).first->mMaths,
                                               (*j).first->mContext, NULL, NULL, 0));

      RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                         mr->iterateInvolvedVariablesByDegree());

      std::map<std::string, uint32_t> maxDegreeByVariableID;
      while (true)
      {
        RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable, dvi->nextDegreeVariable());
        if (dv == NULL)
          break;
        
        RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                           dv->variable());
        std::string objid(cv->objid());
        uint32_t deg(dv->degree());

        std::map<std::string, uint32_t>::iterator mdi = maxDegreeByVariableID.find(objid);
        if (mdi == maxDegreeByVariableID.end())
        {
          maxDegreeByVariableID.insert(std::pair<std::string, uint32_t>(objid, deg));
          maxDegreeByEquationVariableID.insert(std::pair<std::pair<ptr_tag<Equation>, std::string>, uint32_t>
                                               (std::pair<ptr_tag<Equation>, std::string>
                                                ((*j).first, objid), deg));
        }
        else if ((*mdi).second < deg)
        {
          (*mdi).second = deg;
          (*(maxDegreeByEquationVariableID.find(std::pair<ptr_tag<Equation>, std::string>
                                                ((*j).first, objid)))).second = deg;
        }
      }

      for (
           std::set<ptr_tag<CDA_ComputationTarget> >::iterator k = undelayed_derivs.begin();
           k != undelayed_derivs.end();
           k++
          )
      {
        std::string objid((*k)->mVariable->objid());

        std::map<std::string, uint32_t>::iterator mdi = maxDegreeByVariableID.find(objid);
        if (mdi == maxDegreeByVariableID.end())
          continue;

        if ((*mdi).second < (*k)->mDegree)
          undelayed_derivs_someloweronly.insert(*k);
      }
    }

    size_t sz = undelayed_derivs_someloweronly.size();
    if (sz == 0)
      continue;
    if (sz > 1)
      ContextError(L"Piecewise with more than one variable differing in "
                   L"maximum derivative degree not yet supported by reset "
                   L"rule recognition code.", pw->mMaths, pw->mContext);
    ptr_tag<CDA_ComputationTarget> resetCT = (*(undelayed_derivs_someloweronly.begin()))->mDownDegree;

    resetCT->mIsReset = true;

    std::string vobjid(resetCT->mVariable->objid());
    uint32_t deg(resetCT->degree());

    RETURN_INTO_OBJREF(doc, iface::dom::Document, pw->mMaths->ownerDocument());
    RETURN_INTO_OBJREF(apoEl, iface::dom::Element, doc->createElementNS(MATHML_NS, L"apply"));
    DECLARE_QUERY_INTERFACE_OBJREF(otherwiseExclusion, apoEl, mathml_dom::MathMLElement);
    RETURN_INTO_OBJREF(orEl, iface::dom::Element, doc->createElementNS(MATHML_NS, L"or"));
    apoEl->appendChild(orEl)->release_ref();

    // Make a new piecewise for the lower order entries...
    ptr_tag<Piecewise> pwreset(new Piecewise());

    mResets.insert(std::pair<ptr_tag<CDA_ComputationTarget>, ptr_tag<MathStatement> >(resetCT, pwreset));

    RETURN_INTO_OBJREF(pwEl, iface::dom::Element, doc->createElementNS(MATHML_NS, L"piecewise"));
    QUERY_INTERFACE(pwreset->mMaths, pwEl, mathml_dom::MathMLElement);

    for (
         std::list<std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> > >::iterator k, j =
           pw->mPieces.begin();
         j != pw->mPieces.end();
         j = k
        )
    {
      k = j;
      k++;

      std::map<std::pair<ptr_tag<Equation>, std::string >, uint32_t >::iterator i =
        maxDegreeByEquationVariableID.find(std::pair<ptr_tag<Equation>, std::string>
                                           ((*j).first, vobjid));

      if (i == maxDegreeByEquationVariableID.end())
        ContextError(L"Piece not involving reset rule variable not supported in reset rule piecewise",
                     (*j).first->mMaths, (*j).first->mContext);

      // See if this is 'otherwise'. If it is, we need to exclude the other
      // cases explicitly, as they may have been split out to a different piecewise.
      if (k == pw->mPieces.end())
      {
        RETURN_INTO_WSTRING(ln, (*j).second->mMaths->localName());
        if (ln == L"true")
        {
          RETURN_INTO_OBJREF(apn, iface::dom::Element, doc->createElementNS(MATHML_NS, L"apply"));
          RETURN_INTO_OBJREF(notel, iface::dom::Element,
                             doc->createElementNS(MATHML_NS, L"not"));
          apn->appendChild(notel)->release_ref();
          apn->appendChild(otherwiseExclusion)->release_ref();
          
          QUERY_INTERFACE((*j).second->mMaths, apn, mathml_dom::MathMLElement);
        }
      }
      else
      {
        RETURN_INTO_OBJREF(mc, iface::dom::Node, (*j).second->mMaths->cloneNode(true));
        otherwiseExclusion->appendChild(mc)->release_ref();
      }

      if ((*i).second <= deg)
      {
        RETURN_INTO_OBJREF(pieceEl, iface::dom::Element,
                           doc->createElementNS(MATHML_NS, L"piece"));
        ObjRef<iface::dom::Node> vNode =
          (*j).first->mMaths->cloneNode(true);
        ObjRef<iface::dom::Node> cNode =
          (*j).second->mMaths->cloneNode(true);
        pieceEl->appendChild(vNode)->release_ref();
        pieceEl->appendChild(cNode)->release_ref();
        pwEl->appendChild(pieceEl)->release_ref();

        pwreset->mPieces.push_back(*j);
        pw->mPieces.erase(j);
      }
    }

    SetupMathMLMathStatement(pwreset, pwreset->mMaths, pw->mContext);
  }
}

void
CodeGenerationState::ProcessModellerSuppliedIVHints()
{
#ifdef ENABLE_RDF
  RETURN_INTO_OBJREF(rr, iface::cellml_api::RDFRepresentation,
                     mModel->getRDFRepresentation(L"http://www.cellml.org/RDF/API"));
  if (rr == NULL)
    return;

  DECLARE_QUERY_INTERFACE_OBJREF(rar, rr, rdf_api::RDFAPIRepresentation);
  RETURN_INTO_OBJREF(ds, iface::rdf_api::DataSource, rar->source());

  RETURN_INTO_OBJREF(buo, iface::cellml_api::URI, mModel->xmlBase());
  RETURN_INTO_WSTRING(bu, buo->asText());
  std::wstring modelURL = bu + L"#";
  RETURN_INTO_WSTRING(cmId, mModel->cmetaId());
  modelURL += cmId;

  std::map<std::wstring, ptr_tag<CDA_ComputationTarget> > cmetaCTMap;
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mBaseTargets.begin();
       i != mBaseTargets.end(); i++)
  {
    RETURN_INTO_OBJREF(cvs, iface::cellml_services::ConnectedVariableSet,
                       mCeVAS->findVariableSet((*i)->mVariable));
    uint32_t l = cvs->length();
    for (uint32_t j = 0; j < l; j++)
    {
      RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                         cvs->getVariable(j));
      RETURN_INTO_WSTRING(vcmId, cv->cmetaId());
      if (vcmId != L"")
      {
        RETURN_INTO_OBJREF(vm, iface::cellml_api::Model, cv->modelElement());
        RETURN_INTO_OBJREF(vbuo, iface::cellml_api::URI, vm->xmlBase());
        RETURN_INTO_WSTRING(vbu, vbuo->asText());
        std::wstring url(vbu + L"#" + vcmId);
        cmetaCTMap.insert(std::pair<std::wstring, ptr_tag<CDA_ComputationTarget> >(url, *i));
      }
    }
  }

#define URI_REF(v, s) RETURN_INTO_OBJREF(v, iface::rdf_api::URIReference, \
                                         ds->getURIReference(s));
  URI_REF(modelr, modelURL.c_str());
  URI_REF(simhintr, HINTS_NS L"solverHint");
  URI_REF(variabler, HINTS_NS L"variable");
  URI_REF(degreer, HINTS_NS L"degree");
  URI_REF(ivr, HINTS_NS L"initialValue");

  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     modelr->getTriplesOutOfByPredicate(simhintr));
  RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                     ts->enumerateTriples());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
    if (t == NULL)
      break;

    RETURN_INTO_OBJREF(hintsn, iface::rdf_api::Node, t->object());
    DECLARE_QUERY_INTERFACE_OBJREF(hintsr, hintsn, rdf_api::Resource);
    
    uint32_t degree = 0;
    try
    {
      RETURN_INTO_OBJREF(degt, iface::rdf_api::Triple,
                         hintsr->getTripleOutOfByPredicate(degreer));
      RETURN_INTO_OBJREF(degn, iface::rdf_api::Node,
                         degt->object());
      DECLARE_QUERY_INTERFACE_OBJREF(degl, degn, rdf_api::Literal);
      if (degl != NULL)
      {
        RETURN_INTO_WSTRING(deglf, degl->lexicalForm());
        degree = wcstoul(deglf.c_str(), NULL, 10);
      }
    }
    catch (...)
    {
    }

    try
    {
      RETURN_INTO_OBJREF(ivt, iface::rdf_api::Triple,
                         hintsr->getTripleOutOfByPredicate(ivr));
      RETURN_INTO_OBJREF(ivn, iface::rdf_api::Node, ivt->object());
      DECLARE_QUERY_INTERFACE_OBJREF(ivl, ivn, rdf_api::Literal);
      if (ivl == NULL)
        continue;

      RETURN_INTO_WSTRING(ivlf, ivl->lexicalForm());
      double iv = wcstod(ivlf.c_str(), NULL);
      
      RETURN_INTO_OBJREF(vart, iface::rdf_api::Triple,
                         hintsr->getTripleOutOfByPredicate(variabler));
      RETURN_INTO_OBJREF(varn, iface::rdf_api::Node,
                         vart->object());
      DECLARE_QUERY_INTERFACE_OBJREF(varu, varn, rdf_api::URIReference);
      if (varu == NULL)
        continue;

      RETURN_INTO_WSTRING(varuw, varu->URI());
      std::map<std::wstring, ptr_tag<CDA_ComputationTarget> >::iterator
        it(cmetaCTMap.find(varuw));
      if (it == cmetaCTMap.end())
        continue;

      ptr_tag<CDA_ComputationTarget> ct = (*it).second;

      for (; degree && ct; degree--)
        ct = ct->mUpDegree;

      if (degree || ct == NULL)
        continue;

      mInitialOverrides.insert(std::pair<ptr_tag<CDA_ComputationTarget>,
                               double>(ct, iv));
    }
    catch (...)
    {
    }
  }
#endif
}

ptr_tag<CDA_ComputationTarget>
CodeGenerationState::GetTargetOfDegree(ptr_tag<CDA_ComputationTarget> aBase,
                                       uint32_t aDegree)
{
  ptr_tag<CDA_ComputationTarget> t = aBase;

  while (true)
  {
    if (aDegree-- == 0)
      return t;

    if (t->mUpDegree == NULL)
    {
      aBase->mHighestDegree++;
      t->mUpDegree = (new CDA_ComputationTarget())->getSelf();
      t->mUpDegree->mDownDegree = t;
      t = t->mUpDegree;
      mCodeInfo->mTargets.push_back(t);
      t->mVariable = aBase->mVariable;
      t->mAnnoSet = aBase->mAnnoSet;
      t->mDegree = aBase->mHighestDegree;
      t->mUpDegree = reinterpret_cast<CDA_ComputationTarget*>(NULL);
    }
    else
      t = t->mUpDegree;
  }
}

void
CodeGenerationState::CreateBaseComputationTargets()
{
  uint32_t i, l = mCeVAS->length();

  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(cvs, iface::cellml_services::ConnectedVariableSet,
                       mCeVAS->getVariableSet(i));
    RETURN_INTO_OBJREF(sv, iface::cellml_api::CellMLVariable,
                       cvs->sourceVariable());
    if (sv == NULL)
      ContextError(L"Found a variable set with no source.", NULL, NULL);

    RETURN_INTO_OBJREF(basect, CDA_ComputationTarget,
                       new CDA_ComputationTarget());

    basect->mVariable = sv;
    basect->mAnnoSet = mAnnoSet;
    basect->mDegree = 0;
    basect->mUpDegree = reinterpret_cast<CDA_ComputationTarget*>(NULL);
    basect->mHighestDegree = 0;

    basect->add_ref();
    mCodeInfo->mTargets.push_back(basect.getPointer()->getSelf());
    mBaseTargets.push_back(basect.getPointer()->getSelf());
    mTargetsBySource.insert
      (std::pair<iface::cellml_api::CellMLVariable*,ptr_tag<CDA_ComputationTarget> >
       (sv, basect.getPointer()->getSelf()));
  }
}

void
CodeGenerationState::ContextError
(
 const std::wstring& details,
 iface::mathml_dom::MathMLElement* context1,
 iface::cellml_api::CellMLElement* context2
)
{
  std::wstring msg(details);
  
  DECLARE_QUERY_INTERFACE_OBJREF(el, context1, dom::Element);

  while (el != NULL)
  {
    RETURN_INTO_WSTRING(uri, el->namespaceURI());
    if (uri != L"http://www.w3.org/1998/Math/MathML")
      break;

    msg += L"\r\n  In MathML ";
    RETURN_INTO_WSTRING(ln, el->localName());
    msg += ln;
    msg += L" element";
    RETURN_INTO_WSTRING(id, el->getAttribute(L"id"));
    if (id != L"")
    {
      msg += L" with ID ";
      msg += id;
    }
    msg += L".";

    RETURN_INTO_OBJREF(n, iface::dom::Node, el->parentNode());
    QUERY_INTERFACE(el, n, dom::Element);
  }

  ObjRef<iface::cellml_api::CellMLElement> cel(context2);

  while (cel != NULL)
  {
    const wchar_t* typen;

    // This is very ugly, we need a better way at the API level!
#define TYPECHECK_QI_CASE(type) \
  DECLARE_QUERY_INTERFACE_OBJREF(tmp##type, cel, cellml_api::type); \
  if (tmp##type != NULL) \
    typen = L## #type; \
  else

    TYPECHECK_QI_CASE(Model)
    {
      TYPECHECK_QI_CASE(Units)
      {
        TYPECHECK_QI_CASE(Group)
        {
          TYPECHECK_QI_CASE(MapComponents)
          {
            TYPECHECK_QI_CASE(CellMLComponent)
            {
              TYPECHECK_QI_CASE(Unit)
              {
                TYPECHECK_QI_CASE(CellMLImport)
                {
                  TYPECHECK_QI_CASE(CellMLVariable)
                  {
                    TYPECHECK_QI_CASE(ComponentRef)
                    {
                      TYPECHECK_QI_CASE(RelationshipRef)
                      {
                        TYPECHECK_QI_CASE(Connection)
                        {
                          TYPECHECK_QI_CASE(MapVariables)
                          {
                            TYPECHECK_QI_CASE(Reaction)
                            {
                              TYPECHECK_QI_CASE(VariableRef)
                              {
                                TYPECHECK_QI_CASE(Role)
                                {
                                  typen = L"non-standard";
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    msg += L"\r\n  In ";
    msg += typen;
    msg += L" element";

    RETURN_INTO_WSTRING(cm, cel->cmetaId());
    if (cm != L"")
    {
      msg += L" with cmeta:id ";
      msg += cm;
    }

    DECLARE_QUERY_INTERFACE_OBJREF(nce, cel, cellml_api::NamedCellMLElement);
    if (nce != NULL)
    {
      RETURN_INTO_WSTRING(name, nce->name());
      if (name != L"")
      {
        msg += L" with name ";
        msg += name;
      }
    }

    msg += L".";

    cel = already_AddRefd<iface::cellml_api::CellMLElement>
      (cel->parentElement());
  }

  throw CodeGenerationError(msg);
}

void
CodeGenerationState::CreateMathStatements()
{
  RETURN_INTO_OBJREF(cci, iface::cellml_api::CellMLComponentIterator,
                     mCeVAS->iterateRelevantComponents());

  while (true)
  {
    RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                       cci->nextComponent());
    if (c == NULL)
      break;

    RETURN_INTO_OBJREF(ml, iface::cellml_api::MathList, c->math());
    RETURN_INTO_OBJREF(mei, iface::cellml_api::MathMLElementIterator,
                       ml->iterate());
    
    while (true)
    {
      RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement,
                         mei->next());
      if (me == NULL)
        break;

      DECLARE_QUERY_INTERFACE_OBJREF(mme, me,
                                     mathml_dom::MathMLMathElement);
      
      if (mme == NULL)
        ContextError(L"Unexpected MathML element; was expecting a math element",
                     me, c);
      
      RETURN_INTO_OBJREF(nl, iface::dom::NodeList, mme->childNodes());
      uint32_t l = nl->length(), i;
      for (i = 0; i < l; i++)
      {
        RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
        
        if (n == NULL)
          break;
        
        // If it isn't even MathML, just ignore it...
        DECLARE_QUERY_INTERFACE_OBJREF(mn, n, mathml_dom::MathMLElement);
        if (mn == NULL)
          continue;
        
        std::auto_ptr<MathMLMathStatement> mms;

        // See if it is a piecewise...
        DECLARE_QUERY_INTERFACE_OBJREF(mpw, n, mathml_dom::MathMLPiecewiseElement);
        if (mpw != NULL)
        {
          Piecewise* pw = new Piecewise;
          mms.reset(pw);

          RETURN_INTO_OBJREF(pnl, iface::mathml_dom::MathMLNodeList, mpw->pieces());
          for (uint32_t pl = pnl->length(), pi = 1; pi <= pl + 1; pi++)
          {
            ObjRef<iface::dom::Node> val, cond;

            if (pi <= pl)
            {
              val = already_AddRefd<iface::dom::Node>(mpw->getCaseValue(pi));
              cond = already_AddRefd<iface::dom::Node>(mpw->getCaseCondition(pi));
            }
            else
            {
              try
              {
                val = already_AddRefd<iface::dom::Node>(mpw->otherwise());
                RETURN_INTO_OBJREF(doc, iface::dom::Document, val->ownerDocument());
                cond = already_AddRefd<iface::dom::Node>(doc->createElementNS(MATHML_NS, L"true"));
              }
              catch (...)
              {
                continue;
              }
            }

            ptr_tag<Equation> eq(new Equation);

            DECLARE_QUERY_INTERFACE_OBJREF(mae, val, mathml_dom::MathMLApplyElement);
            if (mae == NULL)
            {
              delete pw;
              delete eq;
              ContextError(L"Unexpected MathML element; was expecting an apply",
                           mn, c);
            }

            ObjRef<iface::mathml_dom::MathMLElement> op;
            try
            {
              op =  already_AddRefd<iface::mathml_dom::MathMLElement>
                (mae->_cxx_operator());
            }
            catch (...)
            {
              delete pw;
              delete eq;
              ContextError(L"Unexpected MathML apply element with no MathML children",
                           mae, c);
            }
            
            RETURN_INTO_WSTRING(opn, op->localName());
            if (opn != L"eq")
              ContextError(L"Unexpected MathML element; was expecting an eq",
                           op, c);
            
            eq->mContext = c;
            eq->mMaths = mae;
            
            if (mae->nArguments() != 3)
            {
              delete pw;
              delete eq;
              ContextError(L"Only two-way equalities are supported (a=b not a=b=...)",
                           mae, c);
            }
          
            eq->mLHS = already_AddRefd<iface::mathml_dom::MathMLElement>
              (mae->getArgument(2));
            eq->mRHS = already_AddRefd<iface::mathml_dom::MathMLElement>
              (mae->getArgument(3));

            DECLARE_QUERY_INTERFACE_OBJREF(cme, cond, mathml_dom::MathMLElement);

            ptr_tag<MathMLMathStatement> cms(new MathMLMathStatement(MathStatement::UNCLASSIFIED_MATHML));
            cms->mContext = c;
            cms->mMaths = cme;

            pw->mPieces.push_back(std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> >(eq, cms));
          }
        }
        else
        {
          // MathML, not piecewise? It should be an apply...
          DECLARE_QUERY_INTERFACE_OBJREF(mae, n, mathml_dom::MathMLApplyElement);
          if (mae == NULL)
            ContextError(L"Unexpected MathML element; was expecting an apply",
                         mn, c);

          ObjRef<iface::mathml_dom::MathMLElement> op;
          try
          {
            op =  already_AddRefd<iface::mathml_dom::MathMLElement>
              (mae->_cxx_operator());
          }
          catch (...)
          {
            ContextError(L"Unexpected MathML apply element with no MathML children",
                         mae, c);
          }

          DECLARE_QUERY_INTERFACE_OBJREF(opcs, op, mathml_dom::MathMLCsymbolElement);
          if (opcs != NULL)
          {
            RETURN_INTO_WSTRING(csu, opcs->definitionURL());
            if (csu != L"http://www.cellml.org/uncertainty-1#uncertainParameterWithDistribution")
            {
              ContextError(L"The only supported csymbol definitionURL in a toplevel apply "
                           L"is http://www.cellml.org/uncertainty-1#uncertainParameterWithDistribution",
                           op, c);
            }

            SampleFromDistribution* sfd = new SampleFromDistribution();
            mms.reset(sfd);
            if (mae->nArguments() != 3)
            {
              delete sfd;
              ContextError(L"uncertainParameterWithDistribution takes 2 arguments, the "
                           L"uncertain parameter and the distribution.",
                           mae, c);
            }

            RETURN_INTO_OBJREF(targArg, iface::mathml_dom::MathMLElement, mae->getArgument(2));
            DECLARE_QUERY_INTERFACE_OBJREF(ve, targArg, mathml_dom::MathMLVectorElement);
            if (ve != NULL)
            {
              for (uint32_t i = 1, l = ve->ncomponents(); i <= l; i++)
              {
                RETURN_INTO_OBJREF(vec, iface::mathml_dom::MathMLContentElement,
                                   ve->getComponent(i));
                DECLARE_QUERY_INTERFACE_OBJREF(veci, vec, mathml_dom::MathMLCiElement);
                if (veci == NULL)
                  ContextError(L"Only ci elements in vectors are supported on LHS of sample from distribution.", vec, c);
              }
            }
            else
            {
              DECLARE_QUERY_INTERFACE_OBJREF(ci, targArg, mathml_dom::MathMLCiElement);
              if (ci == NULL)
                ContextError(L"Only ci elements and vectors are supported on LHS of sample from distribution.", targArg, c);
            }

            RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                               mTransform->transform(mCeVAS, mCUSES, mAnnoSet, targArg, c,
                                                     NULL, NULL, 0));
            RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                               mr->iterateInvolvedVariablesByDegree());
            
            while (true)
            {
              RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable,
                                 dvi->nextDegreeVariable());
              if (dv == NULL)
                break;
              
              RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                                 dv->variable());
    
              std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >
                ::iterator  mi = mTargetsBySource.find(cv);
              if (mi != mTargetsBySource.end())
              {
                sfd->mOutTargets.push_back((*mi).second);
                sfd->mOutSet.insert((*mi).second);
              }
            }
            sfd->mDistrib = already_AddRefd<iface::mathml_dom::MathMLElement>(mae->getArgument(3));
          }
          else
          {
            RETURN_INTO_WSTRING(opn, op->localName());
            if (opn != L"eq")
              ContextError(L"Unexpected MathML element; was expecting an eq",
                           op, c);
            
            ptr_tag<Equation> eq(new Equation());
            mms.reset(eq);
            
            if (mae->nArguments() != 3)
            {
              delete eq;
              ContextError(L"Only two-way equalities are supported (a=b not a=b=...)",
                           mae, c);
            }
          
            eq->mLHS = already_AddRefd<iface::mathml_dom::MathMLElement>
              (mae->getArgument(2));
            eq->mRHS = already_AddRefd<iface::mathml_dom::MathMLElement>
              (mae->getArgument(3));
          }
        }

        ptr_tag<MathMLMathStatement> ptmms(mms.release());
        SetupMathMLMathStatement(ptmms, mn, c);
        mMathStatements.push_back(ptmms);
      }
    }
  }
}

void
CodeGenerationState::SetupMathMLMathStatement
(
 MathMLMathStatement* mms,
 iface::mathml_dom::MathMLElement* mn,
 iface::cellml_api::CellMLComponent* c
)
{
  assert(mn != NULL);
  assert(c != NULL);

  mms->mMaths = mn;
  mms->mContext = c;
  RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, mn, c,
                                           NULL, NULL, 0));
  
  RETURN_INTO_WSTRING(compErr, mr->compileErrors());
  if (compErr != L"")
    ContextError(compErr.c_str(), mn, c);

  RETURN_INTO_OBJREF(bvi, iface::cellml_api::CellMLVariableIterator,
                     mr->iterateBoundVariables());
  while (true)
  {
    RETURN_INTO_OBJREF(bv, iface::cellml_api::CellMLVariable,
                       bvi->nextVariable());
    if (bv == NULL)
      break;

    std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >
      ::iterator  mi = mTargetsBySource.find(bv);
    if (mi != mTargetsBySource.end())
    {
      if (mBoundTargs.count((*mi).second) == 0)
        mBoundTargs.insert((*mi).second);
    }
  }

  bvi = already_AddRefd<iface::cellml_api::CellMLVariableIterator>
    (mr->iterateLocallyBoundVariables());
  
  while (true)
  {
    RETURN_INTO_OBJREF(bv, iface::cellml_api::CellMLVariable,
                       bvi->nextVariable());
    if (bv == NULL)
      break;

    std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >
      ::iterator  mi = mTargetsBySource.find(bv);
    if (mi != mTargetsBySource.end())
    {
      if (mLocallyBoundTargs.count((*mi).second) == 0)
        mLocallyBoundTargs.insert((*mi).second);
    }
  }
  
  RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                     mr->iterateInvolvedVariablesByDegree());
  
  while (true)
  {
    RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable,
                       dvi->nextDegreeVariable());
    if (dv == NULL)
      break;
    
    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                       dv->variable());
    
    std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >
      ::iterator  mi = mTargetsBySource.find(cv);
    if (mi != mTargetsBySource.end())
    {
      if (mLocallyBoundTargs.count((*mi).second))
        continue;

      ptr_tag<CDA_ComputationTarget> targ(GetTargetOfDegree((*mi).second, dv->degree()));
      if (dv->appearedInfinitesimallyDelayed())
      {
        mms->mDelayedTargets.push_back(targ);
        mms->mInvolvesDelays = true;
        
        if (!mDelayedTargs.count(targ))
          mDelayedTargs.insert(targ);
      }
      else
        mms->mTargets.push_back(targ);
    }
  }

}

void
CodeGenerationState::FirstPassTargetClassification()
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator i;
  for (i =  mBaseTargets.begin();
       i != mBaseTargets.end();
       i++)
  {
    // Scoped locale change.
    CNumericLocale locobj;

    ptr_tag<CDA_ComputationTarget> ct = *i;

    RETURN_INTO_WSTRING(iv, ct->mVariable->initialValue());
    bool hasImmedIV = false;
    double ivVal = 0.0;
    if (iv != L"")
    {
      wchar_t* end;
      ivVal = wcstod(iv.c_str(), &end);
      if (end == NULL || *end != 0)
      {
        ptr_tag<InitialAssignment> ia(new InitialAssignment());
        mMathStatements.push_back(ia);
        ia->mTargets.push_back(ct);
        RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement,
                           ct->mVariable->parentElement());
        DECLARE_QUERY_INTERFACE_OBJREF(comp, el, cellml_api::CellMLComponent);
        ia->mContext = comp;
        RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                           comp->variables());
        RETURN_INTO_OBJREF(var, iface::cellml_api::CellMLVariable,
                           cvs->getVariable(iv.c_str()));
        if (var == NULL)
          ContextError(L"Invalid initial_value attribute", NULL,
                       ct->mVariable);

        RETURN_INTO_OBJREF(covs, iface::cellml_services::ConnectedVariableSet,
                           mCeVAS->findVariableSet(var));
        RETURN_INTO_OBJREF(sv, iface::cellml_api::CellMLVariable,
                           covs->sourceVariable());
        if (sv == NULL)
          ContextError(L"Variable not connected to non-in variable", NULL,
                       var);

        std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >::
          iterator j = mTargetsBySource.find(sv);
        if (j == mTargetsBySource.end())
          ContextError(L"Invalid initial_value attribute", NULL, var);

        ia->mTargets.push_back((*j).second);
      }
      else
      {
        hasImmedIV = true;
      }
    }

    // See if we have a VOI...
    if (mBoundTargs.count(ct))
    {
      ct->mEvaluationType = iface::cellml_services::VARIABLE_OF_INTEGRATION;
      std::wstring ignore;
      AllocateVOI(ct, ignore);
    }
    else if (mLocallyBoundTargs.count(ct))
    {
      ct->mEvaluationType = iface::cellml_services::LOCALLY_BOUND;
      std::wstring name;
      AllocateAlgebraicVariable(ct, name);
      mAnnoSet->setStringAnnotation(ct->mVariable, L"bvarIndex", name.c_str());
    }
    else
    {
      if (hasImmedIV && !mDelayedTargs.count(ct) && mCompatLevel == 0)
        ct->mEvaluationType = iface::cellml_services::CONSTANT;
      else
        ct->mEvaluationType = iface::cellml_services::FLOATING;

      if (hasImmedIV && !mDelayedTargs.count(ct) && mCompatLevel == 1 && !ct->mUpDegree)
        mInitialOverrides.insert(std::pair<ptr_tag<CDA_ComputationTarget>,
                                           double>(ct, ivVal));

      ptr_tag<CDA_ComputationTarget> tct = ct;

      std::wstring cname;
      while (tct->mUpDegree)
      {
        tct->mEvaluationType = iface::cellml_services::STATE_VARIABLE;
        tct->mStateHasIV = false;
        AllocateStateVariable(tct, cname);
        if (mIDAStyle)
          AllocateDelayed(tct, mInfDelayedStatePattern,
                          mNextStateVariableIndex - 1);
        if (tct == ct && hasImmedIV)
        {
          tct->mStateHasIV = true;
          AppendConstantAssign(mCodeInfo->mInitConstsStr, cname,
                               mTransform->wrapNumber(iv), ct->mVariable->name());
        }
        else if (tct != ct)
        {
          tct->mStateHasIV = true;
          AppendConstantAssign
            (
             mCodeInfo->mInitConstsStr, cname,
             mTransform->wrapNumber(L"0.0"),
             tct->mVariable->name());
        }
        tct = tct->mUpDegree;
        tct->mEvaluationType = iface::cellml_services::FLOATING;
      }
      if (ct->mUpDegree)
      {
        uint32_t mrvi = mNextStateVariableIndex - 1;
        uint32_t count = 0;
        AllocateVariable(tct, cname, mRateNamePattern, mrvi, count);
        if (mIDAStyle)
          AllocateDelayed(tct, mInfDelayedRatePattern, mrvi);
      }
      else if (ct->mEvaluationType == iface::cellml_services::CONSTANT)
      {
        AllocateConstant(ct, cname);
        AppendConstantAssign(mCodeInfo->mInitConstsStr, cname,
                             mTransform->wrapNumber(iv), ct->mVariable->name());
      }
    }
  }
  // We also need to ensure that bvars on distributions are treated as locally bound...
  for (std::list<ptr_tag<MathStatement> >::iterator i =  mMathStatements.begin();
       i != mMathStatements.end();
       i++)
  {
    if ((*i)->mType != MathStatement::SAMPLE_FROM_DIST)
      continue;

    SampleFromDistribution* sfd = static_cast<SampleFromDistribution*>(static_cast<MathStatement*>(*i));
    ObjRef<iface::mathml_dom::MathMLApplyElement> mae = QueryInterface(sfd->mDistrib);
    if (mae == NULL)
      continue; // This case will result in an error later in GenerateCodeForSampleFromDist
    ObjRef<iface::mathml_dom::MathMLElement> op = mae->_cxx_operator();
    ObjRef<iface::mathml_dom::MathMLCsymbolElement> csop = QueryInterface(op);
    if (csop == NULL)
      continue;
    std::wstring du = csop->definitionURL();
    if (du != L"http://www.cellml.org/uncertainty-1#distributionFromDensity")
      continue;
    ObjRef<iface::mathml_dom::MathMLLambdaElement> lambda = QueryInterface(mae->getArgument(2));
    if (lambda == NULL)
      continue;

    for (unsigned int bvIdx = lambda->nBoundVariables(); bvIdx > 0; bvIdx--)
    {
      ObjRef<iface::mathml_dom::MathMLBvarElement> bv(lambda->getBoundVariable(bvIdx));
      ObjRef<iface::mathml_dom::MathMLElement> bvci(bv->getArgument(1));
      ObjRef<iface::mathml_dom::MathMLCiElement> bvcici(QueryInterface(bvci));
      if (bvcici == NULL)
        continue;
      ObjRef<iface::dom::Node> n = bvcici->getArgument(1);

      ObjRef<iface::dom::Text> tn(QueryInterface(n));
      if (tn == NULL)
        continue;
      std::wstring txt = tn->data();

      int i = 0, j = txt.length() - 1;
      wchar_t c;
      while ((c = txt[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
        i++;
      while ((c = txt[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
        j--;
      if (j < i)
        ContextError(L"CI element with only spaces inside.", bvci, sfd->mContext);
      txt = txt.substr(i, j - i + 1);
      ObjRef<iface::cellml_api::CellMLVariableSet> vs(sfd->mContext->variables());
      ObjRef<iface::cellml_api::CellMLVariable> bvarV(vs->getVariable(txt));
      if (bvarV == NULL)
        continue;

      std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >::iterator
        tbs(mTargetsBySource.find(bvarV));
      if (tbs == mTargetsBySource.end())
        continue;
      tbs->second->mEvaluationType = iface::cellml_services::LOCALLY_BOUND;
    }
  }
}

void
CodeGenerationState::GenerateVariableName
(
 std::wstring& aStr,
 std::wstring& aPattern,
 uint32_t index
)
{
  size_t cursor = aPattern.find(L'%');
  if (cursor == std::wstring::npos)
  {
    aStr.assign(aPattern);
    return;
  }

  // Scoped locale change.
  CNumericLocale locobj;

  aStr.assign(aPattern.substr(0, cursor));
  wchar_t buf[30];
  any_swprintf(buf, 30, L"%lu", index);
  aStr.append(buf);
  aStr.append(aPattern.substr(cursor + 1));
}

void
CodeGenerationState::AllocateVariable(ptr_tag<CDA_ComputationTarget> aCT,
                                      std::wstring& aStr,
                                      std::wstring& aPattern,
                                      uint32_t& aNextIndex,
                                      uint32_t& aCount)
{
  RETURN_INTO_WSTRING(n, aCT->name());
  if (n == L"")
  {
    uint32_t index = aNextIndex++;
    aCount++;
    GenerateVariableName(aStr, aPattern, index);
    aCT->setNameAndIndex(index, aStr.c_str());
  }
  else
    aStr = n;
}

void
CodeGenerationState::AllocateDelayed(ptr_tag<CDA_ComputationTarget> aCT,
                                     std::wstring& aPattern,
                                     uint32_t aNextIndex)
{
  std::wstring str;
  GenerateVariableName(str, aPattern, aNextIndex);
  aCT->setDelayedName(str);
}

void
CodeGenerationState::ComputeInfDelayedName(ptr_tag<CDA_ComputationTarget> aCT,
                                           std::wstring& aStr)
{
  uint32_t index = 0;
  if (aCT->mInfDelayedAssignedIndex < 0)
  {
    index = mNextAlgebraicVariableIndex++;
    mCodeInfo->mAlgebraicIndexCount++;
    aCT->mInfDelayedAssignedIndex = index;
    mInfDelayedTargets.push_back(aCT);
  }
  else
    index = aCT->mInfDelayedAssignedIndex;

  GenerateVariableName(aStr, mAlgebraicVariableNamePattern, index);
  aCT->setDelayedName(aStr);
}

void
CodeGenerationState::AllocateVariablesInSet
(
 std::list<System*>& aCT,
 iface::cellml_services::VariableEvaluationType aET,
 std::wstring& aPattern,
 uint32_t& aNextIndex,
 uint32_t& aCountVar
)
{
  std::wstring str;
  
  std::list<System*>::iterator i;
  std::set<ptr_tag<CDA_ComputationTarget> >::iterator j;

  for (i = aCT.begin(); i != aCT.end(); i++)
  {
    for (j = (*i)->mUnknowns.begin(); j != (*i)->mUnknowns.end(); j++)
    {
      // Don't rename it if it is already assigned...
      RETURN_INTO_WSTRING(n, (*j)->name());
      if (n == L"" ||
          (*j)->mEvaluationType !=
          iface::cellml_services::STATE_VARIABLE)
        (*j)->mEvaluationType = aET;
      if (n == L"")
      {
        AllocateVariable(*j, str, aPattern, aNextIndex, aCountVar);
      }
    }
  }
}

void
CodeGenerationState::AllocateRateNamesAsConstants(std::list<System*>& aSystems)
{
  for (std::list<System*>::iterator i(aSystems.begin()); i != aSystems.end(); i++)
  {
    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mUnknowns.begin();
         j != (*i)->mUnknowns.end(); j++)
    {
      if ((*j)->degree() == 0)
        continue;
      
      // Backup the old name...
      uint32_t oldIndex = (*j)->assignedIndex();
      RETURN_INTO_WSTRING(oldName, (*j)->name());
      mRateNameBackup.push_back(std::pair<ptr_tag<CDA_ComputationTarget>, std::wstring>
                                (*j, oldName));
      
      // We can't use AllocateConstant / AllocateVariable here because we are
      // creating an alias not the original name.
      uint32_t index = mCodeInfo->mConstantIndexCount++;
      std::wstring tmpname;
      GenerateVariableName(tmpname, mConstantPattern, index);
      (*j)->setNameAndIndex(oldIndex, tmpname.c_str());
    }
  }
}

void
CodeGenerationState::RestoreSavedRates(std::wstring& aCodeTo)
{
  while (!mRateNameBackup.empty())
  {
    std::pair<ptr_tag<CDA_ComputationTarget>, std::wstring> p =
      mRateNameBackup.front();
    mRateNameBackup.pop_front();

    RETURN_INTO_WSTRING(constName, p.first->name());

    uint32_t index = p.first->assignedIndex();

    AppendAssign(aCodeTo, p.second, mTransform->wrapNumber(constName),
                 L"Rate Restore");
    p.first->setNameAndIndex(index, p.second.c_str());
  }
}

void
CodeGenerationState::AllocateConstant(ptr_tag<CDA_ComputationTarget> aCT,
                                      std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mConstantPattern, mNextConstantIndex,
                   mCodeInfo->mConstantIndexCount);
}

void
CodeGenerationState::AllocateStateVariable(ptr_tag<CDA_ComputationTarget> aCT,
                                           std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mStateVariableNamePattern, mNextStateVariableIndex,
                   mCodeInfo->mRateIndexCount);
}

void
CodeGenerationState::AllocateAlgebraicVariable(ptr_tag<CDA_ComputationTarget> aCT,
                                               std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mAlgebraicVariableNamePattern, mNextAlgebraicVariableIndex,
                   mCodeInfo->mAlgebraicIndexCount);
}

void
CodeGenerationState::AllocateVOI(ptr_tag<CDA_ComputationTarget> aCT,
                                 std::wstring& aStr)
{
  uint32_t count = 0;
  AllocateVariable(aCT, aStr, mVOIPattern, mNextVOI, count);
}

void
CodeGenerationState::CloneNamesIntoDelayedNames()
{
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i =
         mCodeInfo->mTargets.begin(); i != mCodeInfo->mTargets.end(); i++)
  {    
    (*i)->setDelayedName(L"");
  }
}

void
CodeGenerationState::AppendAssign
(
 std::wstring& aAppendTo,
 const std::wstring& aLHS,
 const std::wstring& aRHS,
 const std::wstring& aXmlID
)
{
  size_t curIdx = 0;

  while (true)
  {
    size_t lIdx = mAssignPattern.find(L"<LHS>", curIdx),
      rIdx = mAssignPattern.find(L"<RHS>", curIdx),
      iIdx = mAssignPattern.find(L"<XMLID>", curIdx);
    if ((lIdx == rIdx) && (lIdx == iIdx) && (lIdx == std::string::npos))
    {
      aAppendTo += mAssignPattern.substr(curIdx);
      return;
    }
    if (lIdx <= rIdx && lIdx <= iIdx)
    {
      aAppendTo += mAssignPattern.substr(curIdx, lIdx - curIdx);
      aAppendTo += aLHS;
      curIdx = lIdx + sizeof("<LHS>") - 1;
    }
    else if (rIdx <= iIdx)
    {
      aAppendTo += mAssignPattern.substr(curIdx, rIdx - curIdx);
      aAppendTo += aRHS;
      curIdx = rIdx + sizeof("<RHS>") - 1;
    }
    else
    {
      aAppendTo += mAssignPattern.substr(curIdx, iIdx - curIdx);
      aAppendTo += aXmlID;
      curIdx = iIdx + sizeof("<XMLID>") - 1;
    }
  }
}

void
CodeGenerationState::AppendConstantAssign
(
 std::wstring& aAppendTo,
 const std::wstring& aLHS,
 const std::wstring& aRHS,
 const std::wstring& aXmlID
)
{
  size_t curIdx = 0;

  while (true)
  {
    size_t lIdx = mAssignConstantPattern.find(L"<LHS>", curIdx),
      rIdx = mAssignConstantPattern.find(L"<RHS>", curIdx),
      iIdx = mAssignConstantPattern.find(L"<XMLID>", curIdx);
    if ((lIdx == rIdx) && (lIdx == iIdx) && (lIdx == std::string::npos))
    {
      aAppendTo += mAssignConstantPattern.substr(curIdx);
      return;
    }
    if (lIdx <= rIdx && lIdx <= iIdx)
    {
      aAppendTo += mAssignConstantPattern.substr(curIdx, lIdx - curIdx);
      aAppendTo += aLHS;
      curIdx = lIdx + sizeof("<LHS>") - 1;
    }
    else if (rIdx <= iIdx)
    {
      aAppendTo += mAssignConstantPattern.substr(curIdx, rIdx - curIdx);
      aAppendTo += aRHS;
      curIdx = rIdx + sizeof("<RHS>") - 1;
    }
    else
    {
      aAppendTo += mAssignConstantPattern.substr(curIdx, iIdx - curIdx);
      aAppendTo += aXmlID;
      curIdx = iIdx + sizeof("<XMLID>") - 1;
    }
  }
}


void
CodeGenerationState::BuildFloatingAndConstantLists()
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mCodeInfo->mTargets.begin();
  mFloating.clear();
  mKnown.clear();
  for (; i != mCodeInfo->mTargets.end(); i++)
    switch ((*i)->mEvaluationType)
    {
    case iface::cellml_services::CONSTANT:
    // Treat as constant for the purposes of the algorithm
    case iface::cellml_services::LOCALLY_BOUND:
      mKnown.insert(*i);
      break;
    case iface::cellml_services::FLOATING:
      if (!(*i)->mIsReset)
      {
        mFloating.insert(*i);
        break;
      }

      // fall through...
    case iface::cellml_services::STATE_VARIABLE:
    case iface::cellml_services::VARIABLE_OF_INTEGRATION:
      mUnwanted.insert(*i);

    default:
      ;
    }
}

void
CodeGenerationState::BuildStateAndConstantLists()
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mCodeInfo->mTargets.begin();
  mFloating.clear();
  mKnown.clear();
  for (; i != mCodeInfo->mTargets.end(); i++)
    switch ((*i)->mEvaluationType)
    {
    case iface::cellml_services::STATE_VARIABLE:
      mFloating.insert(*i);
      break;
    case iface::cellml_services::CONSTANT:
    // Treat as constant for the purposes of the algorithm
    case iface::cellml_services::LOCALLY_BOUND:
      mKnown.insert(*i);
      break;
    default:
      break;
    }
}

void
CodeGenerationState::BuildFloatingAndKnownLists(bool aIncludeRates)
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = mCodeInfo->mTargets.begin();
  mFloating.clear();
  mKnown.clear();
  for (; i != mCodeInfo->mTargets.end(); i++)
    switch ((*i)->mEvaluationType)
    {
    case iface::cellml_services::FLOATING:
      if (!aIncludeRates && (*i)->mDegree > 0)
      {
        mUnwanted.insert(*i);
        break;
      }
      mFloating.insert(*i);
      break;
    default:
      mKnown.insert(*i);
      break;
    }
}

void
CodeGenerationState::DecomposeIntoAssignments
(
 std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
 std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
 std::set<ptr_tag<CDA_ComputationTarget> >& aUnwanted,
 std::list<System*>& aSystems
)
{
  std::set<ptr_tag<CDA_ComputationTarget> > start(aStart);

  bool progress = true;
  while (progress)
  {
    progress = false;

    for (std::set<ptr_tag<MathStatement> >::iterator j, i = mUnusedMathStatements.begin();
         i != mUnusedMathStatements.end(); i=j)
    {
      j = i;
      j++;
      MathStatement* ms = *i;

      if (ms->mType == MathStatement::SAMPLE_FROM_DIST)
      {
        SampleFromDistribution* sfd = static_cast<SampleFromDistribution*>(ms);
        std::set<ptr_tag<MathStatement> > mss;
        mss.insert(*i);

        std::set<ptr_tag<CDA_ComputationTarget> > knowns, unknowns;
        for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator k =
               sfd->mTargets.begin(); k != sfd->mTargets.end(); k++)
        {
          if (sfd->mOutSet.count(*k) == 0)
          {
            if (aCandidates.count(*k) == 0)
              continue;
            knowns.insert(*k);
          }
          else
          {
            if (aUnwanted.count(*k))
              continue;
            start.insert(*k);
            aCandidates.erase(*k);
            unknowns.insert(*k);
          }
        }

        System* sys = new System(mss, knowns, unknowns);
        mSystems.push_back(sys);
        aSystems.push_back(sys);
        
        mUnusedMathStatements.erase(i);
        progress = true;        
        continue;
      }

      // XXX Do we want to handle top-level piecewise functions that have no
      // unknown rates in any form here too? These are not encouraged, but
      // they are probably valid.
      if (ms->mType != MathStatement::INITIAL_ASSIGNMENT &&
          ms->mType != MathStatement::EQUATION)
        continue;

      bool failed = false;
      ptr_tag<CDA_ComputationTarget> newUnknown;
      std::set<ptr_tag<CDA_ComputationTarget> > knowns;
      for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator k = ms->mTargets.begin();
           k != ms->mTargets.end(); k++)
      {
        if (start.count(*k) != 0)
        {
          knowns.insert(*k);
          continue;
        }
        if (aUnwanted.count(*k) != 0)
        {
          failed = true;
          break;
        }

        if (aCandidates.count(*k) != 0)
        {
          if (newUnknown != NULL)
          {
            failed = true;
            break;
          }

          newUnknown = *k;
        }
      }
      if (failed)
        continue;

      if (newUnknown == NULL)
      {
        if (ms->mType != MathStatement::INITIAL_ASSIGNMENT)
          throw OverconstrainedError((static_cast<MathMLMathStatement*>(ms))->mMaths);
        else
          throw OverconstrainedError(NULL);
      }

      std::set<ptr_tag<MathStatement> > mss;
      mss.insert(*i);
      std::set<ptr_tag<CDA_ComputationTarget> > unknowns;
      unknowns.insert(newUnknown);

      if (ms->mType == MathStatement::INITIAL_ASSIGNMENT)
      {
        if (ms->mTargets.front() != newUnknown)
          continue;
      }
      else if (ms->mType == MathStatement::EQUATION)
      {
        // Now we see if we can do a simple assignment...
        try
        {
          Equation* eq = static_cast<Equation*>(ms);
          std::wstring throwAway;
          mDryRun = true;
          GenerateCodeForEquation(throwAway, eq, newUnknown, true);
          mDryRun = false;
        }
        catch (AssignmentOnlyRequestedNeedSolve aorns)
        {
          // No we can't - this equation needs to be solved for by IDA instead.
          continue;
        }
      }

      System* sys = new System(mss, knowns, unknowns);
      mSystems.push_back(sys);
      aSystems.push_back(sys);

      start.insert(newUnknown);
      aCandidates.erase(newUnknown);
      mUnusedMathStatements.erase(i);
      progress = true;
    }
  }
}

bool
CodeGenerationState::DecomposeIntoSystems
(
 std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
 std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
 std::set<ptr_tag<CDA_ComputationTarget> >& aUnwanted,
 std::list<System*>& aSystems,
 bool aIgnoreInfdelayed
)
{
  std::set<ptr_tag<CDA_ComputationTarget> > start(aStart);

  while (true)
  {
    // The first step is to cluster all candidate variables into disjoint sets,
    // where two variables are in the same set if there is an until now unused
    // equation involving both of them.
    for (
         std::set<ptr_tag<CDA_ComputationTarget> >::iterator i = aCandidates.begin();
         i != aCandidates.end();
         i++
        )
      (*i)->resetSetMembership();

    for (std::set<ptr_tag<MathStatement> >::iterator i = mUnusedMathStatements.begin();
         i != mUnusedMathStatements.end(); i++)
    {
      std::list<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mTargets.begin(), f;

      if (aIgnoreInfdelayed && (*i)->mInvolvesDelays)
        continue;

      ptr_tag<CDA_ComputationTarget> linkWith;
      // See if we should ignore this math statement.
      bool ignoreMathStatement(false);
      for (; j != (*i)->mTargets.end(); j++)
      {
        if (start.count(*j) > 0)
          continue;
        if (aCandidates.count(*j) == 0)
        {
          ignoreMathStatement = true;
          break;
        }
        if (linkWith == NULL)
        {
          linkWith = *j;
          f = j;
        }
      }
      if (ignoreMathStatement)
        continue;
      if (linkWith == NULL)
      {
        // This means that the model is overconstrained.
        MathStatement* ms = *i;
        if ((*i)->mType == MathStatement::INITIAL_ASSIGNMENT)
          throw OverconstrainedError(NULL);
        else
          throw OverconstrainedError(static_cast<MathMLMathStatement*>(ms)->mMaths);
      }

      for (j = f, j++; j != (*i)->mTargets.end(); j++)
      {
        if (start.count(*j) > 0)
          continue;

        (*j)->unionWith(linkWith);
        assert(linkWith->findRoot() == (*j)->findRoot());
      }
    }
    
    typedef std::pair<std::set<ptr_tag<MathStatement> >, std::set<ptr_tag<CDA_ComputationTarget> > >
      MathStatCTSetPair ;
    typedef std::pair<ptr_tag<CDA_ComputationTarget>, MathStatCTSetPair>
      MathStatCTSetPairByCT;
    typedef std::map<ptr_tag<CDA_ComputationTarget>, MathStatCTSetPair>
      MapMathStatCTSetPairByCT;

    MapMathStatCTSetPairByCT targets;

    for (
         std::set<ptr_tag<CDA_ComputationTarget> >::iterator j = aCandidates.begin();
         j != aCandidates.end();
         j++
        )
    {
      ptr_tag<CDA_ComputationTarget> root = (*j)->findRoot();

      MapMathStatCTSetPairByCT::iterator cti(targets.find(root));

      if (cti == targets.end())
      {
        std::set<ptr_tag<CDA_ComputationTarget> > s;
        s.insert(*j);
        targets.insert(MathStatCTSetPairByCT(root, MathStatCTSetPair
                                             (std::set<ptr_tag<MathStatement> >(), s)));
      }
      else
      {
        (*cti).second.second.insert(*j);
      }
    }

    // Now, we need to go through all the unused equations and stick them in a
    // set with the appropriate set of computation targets...
    for (
         std::set<ptr_tag<MathStatement> >::iterator i = mUnusedMathStatements.begin();
         i != mUnusedMathStatements.end();
         i++
        )
    {
      if (aIgnoreInfdelayed && (*i)->mInvolvesDelays)
        continue;

      // See if we should ignore this equation.
      bool ignoreMathStatement(false);
      ptr_tag<CDA_ComputationTarget> first;
      for (
           std::list<ptr_tag<CDA_ComputationTarget> >::iterator j((*i)->mTargets.begin());
           j != (*i)->mTargets.end();
           j++
          )
      {
        if (start.count(*j) > 0)
          continue;

        if (aCandidates.count(*j) == 0)
        {
          ignoreMathStatement = true;
          break;
        }

        first = *j;
      }
      if (ignoreMathStatement)
        continue;

      // First is guaranteed not to be null because of the overconstrained
      // checks above, and is guaranteed to belong to exactly one of the
      // disjoint sets of variables. Find which one...

      MapMathStatCTSetPairByCT::iterator j(targets.find(first->findRoot()));
      (*j).second.first.insert(*i);
    }

    bool progress = false;

    // We now have a set of disjoint sets of variables (i.e. there are no
    // equations which have not yet been used linking variables in different
    // disjoint sets), and the equations linking them.
    for (MapMathStatCTSetPairByCT::iterator i(targets.begin());
         i != targets.end();
         i++)
    {
      if ((*i).second.first.empty())
        continue;

      if (FindSmallSystem((*i).second.first, (*i).second.second, start,
                          aCandidates, aSystems))
      {
        progress = true;
        continue;
      }

      // We failed to find a small system. Instead, start off with everything
      // in one system and start taking stuff out...
      bool error = FindBigSystem((*i).second.first, (*i).second.second, start, aCandidates,
                                 aSystems);

      progress |= (!error);
    }

    if (!progress)
      return (aCandidates.size() != 0) || (mUnusedMathStatements.size() != 0);

    // Having found one system, we may have further fragmented the systems by
    // removing the linking variables. The next iteration will detect this and
    // then we can try again.
  }
}

bool
CodeGenerationState::FindSmallSystem
(
 std::set<ptr_tag<MathStatement> >& aUseMathStatements,
 std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
 std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
 std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
 std::list<System*>& aSystems
)
{

  for (uint32_t systemCardinality = 1;
       systemCardinality <= SEARCH_DEPTH;
       systemCardinality++)
  {
    std::set<ptr_tag<MathStatement> > s;
    std::set<ptr_tag<MathStatement> >::iterator i(aUseMathStatements.begin());

    // printf("Trying to find small system of size %u from network of %u candidates, %u equations\n",
    //        systemCardinality, aCandidates.size(), aUseMathStatements.size());
    if (RecursivelyTestSmallSystem(s, i, systemCardinality,
                                   aUseMathStatements, aUseVars, aStart, aCandidates,
                                   aSystems
                                  ))
    {
      // printf("Success\n");
      return true;
    }
    // printf("No luck, upping cardinality.\n");
  }

  return false;
}

uint32_t
ComputeSystemSize(std::set<ptr_tag<MathStatement> >& aSystem)
{
  uint32_t totalDegF = 0;
  for (std::set<ptr_tag<MathStatement> >::iterator i = aSystem.begin();
       i != aSystem.end(); i++)
    totalDegF += (*i)->degFreedom();
  return totalDegF;
}

bool
CodeGenerationState::RecursivelyTestSmallSystem
(
 std::set<ptr_tag<MathStatement> >& aSystem,
 std::set<ptr_tag<MathStatement> >::iterator& aEqIt,
 uint32_t aNeedToAdd,
 std::set<ptr_tag<MathStatement> >& aUseMathStatements,
 std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
 std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
 std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
 std::list<System*>& aSystems
)
{
  // Pre: aNeedToAdd is >= 1...
  aNeedToAdd--;

  for (std::set<ptr_tag<MathStatement> >::iterator i(aEqIt); i != aUseMathStatements.end();)
  {
    if ((*i)->mType == MathStatement::SAMPLE_FROM_DIST)
    {
      // Sampling cannot be part of a set of simultaneous equation, but can appear by itself.
      if (aNeedToAdd > 0 || aSystem.size() > 0)
      {
        i++;
        continue;
      }
    }

    // We do this insert and erase thing rather than copy the set to save stack
    // space.
    std::set<ptr_tag<MathStatement> >::iterator sysEq(aSystem.insert(*i).first);
    i++;


    if (aNeedToAdd > 0)
    {
      if (RecursivelyTestSmallSystem(aSystem, i, aNeedToAdd, aUseMathStatements,
                                     aUseVars, aStart, aCandidates, aSystems))
        return true;
    }
    else
    {
      // Well, we now have a set of equations which we are going to consider as
      // a possible system in aSystem. Work out the variables involved...
      std::set<ptr_tag<CDA_ComputationTarget> > targets;
      std::set<ptr_tag<CDA_ComputationTarget> > known;

      for (std::set<ptr_tag<MathStatement> >::iterator j(aSystem.begin());
           j != aSystem.end(); j++)
        for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator k
               ((*j)->mTargets.begin());
             k != ((*j)->mTargets.end()); k++)
        {
          if (aStart.count(*k))
          {
            assert(aCandidates.count(*k) == 0);
            known.insert(*k);
          }
          else
          {
            assert(aCandidates.count(*k));
            assert(aUseVars.count(*k));
            targets.insert(*k);
          }
        }
      
      uint32_t nEqns = ComputeSystemSize(aSystem), nUnknowns = targets.size();

#if 0
      if ((*targets.begin())->mVariable->name() == L"GK1_scal_rv")
      {
        for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator i = targets.begin();
             i != targets.end(); i++)
        {
          assert((*i)->mEvaluationType != iface::cellml_services::LOCALLY_BOUND);
        }
        asm("int $3");
      }
#endif

      if ((*aSystem.begin())->mType == MathStatement::SAMPLE_FROM_DIST &&
          static_cast<SampleFromDistribution*>(static_cast<MathStatement*>(*aSystem.begin()))->mOutSet != targets)
      {
        aSystem.erase(sysEq);
        continue;
      }

      // printf("In this case, nEqns = %u, nUnknowns = %u\n", nEqns, nUnknowns);

      if (nEqns > nUnknowns)
      {
        MathStatement * me = *(aUseMathStatements.begin());
        if (me->mType != MathStatement::INITIAL_ASSIGNMENT)
          throw OverconstrainedError(static_cast<MathMLMathStatement*>(me)->mMaths);
        else
          throw OverconstrainedError(NULL);
      }

      if (nEqns < nUnknowns)
      {
        aSystem.erase(sysEq);
        continue;
      }

      // If we get here, we found a small system to remove.
      System* syst = new System(aSystem, known, targets);
      mSystems.push_back(syst);
      aSystems.push_back(syst);

      aStart.insert(targets.begin(), targets.end());

      for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator k(targets.begin());
           k != targets.end();
           k++)
        aCandidates.erase(*k);

      for (std::set<ptr_tag<MathStatement> >::iterator k(aSystem.begin());
           k != aSystem.end(); k++)
        mUnusedMathStatements.erase(*k);

      return true;
    }

    aSystem.erase(sysEq);
  }

  return false;
}

bool
CodeGenerationState::FindBigSystem
(
 std::set<ptr_tag<MathStatement> >& aUseMathStatements,
 std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
 std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
 std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
 std::list<System*>& aSystems
)
{
  // There is absolutely no point in doing this if the system is improperly
  // constrained...
  if (ComputeSystemSize(aUseMathStatements) != aUseVars.size())
    return true;

  bool didWork;
  
  do
  {
    didWork = false;

    for (uint32_t nonSystemCardinality = 1;
         nonSystemCardinality <= SEARCH_DEPTH;
         nonSystemCardinality++)
    {
      // Empty systems are entirely meaningless...
      if (nonSystemCardinality >= aUseMathStatements.size())
        break;

      std::set<ptr_tag<MathStatement> > s;
      std::set<ptr_tag<MathStatement> >::iterator i(aUseMathStatements.begin());
      if (RecursivelyTestBigSystem(s, i, nonSystemCardinality,
                                   aUseMathStatements, aUseVars, aStart, aCandidates
                                  ))
      {
        didWork = true;
        break;
      }
    }
  }
  while (didWork);

  // If we get here, we have removed as many equations as we can from aUseVars
  // and aUseMathStatements, so that is now our system...

  std::set<ptr_tag<CDA_ComputationTarget> > known;

  for (std::set<ptr_tag<MathStatement> >::iterator i(aUseMathStatements.begin());
       i != aUseMathStatements.end();
       i++)
    for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator j((*i)->mTargets.begin());
         j != (*i)->mTargets.end();
         j++)
    {
      if (aStart.count(*j))
        known.insert(*j);
    }

  System* st = new System(aUseMathStatements, known, aUseVars);
  mSystems.push_back(st);
  aSystems.push_back(st);

  aStart.insert(aUseVars.begin(), aUseVars.end());
  for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator k(aUseVars.begin());
       k != aUseVars.end();
       k++)
    aCandidates.erase(*k);
  
  for (std::set<ptr_tag<MathStatement> >::iterator k(aUseMathStatements.begin());
       k != aUseMathStatements.end(); k++)
    mUnusedMathStatements.erase(*k);

  return false;
}

bool
CodeGenerationState::RecursivelyTestBigSystem
(
 std::set<ptr_tag<MathStatement> >& aNonSystem,
 std::set<ptr_tag<MathStatement> >::iterator& aEqIt,
 uint32_t aNeedToRemove,
 std::set<ptr_tag<MathStatement> >& aUseMathStatements,
 std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
 std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
 std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates
)
{
  // Pre: aNeedToRemove is >= 1...
  aNeedToRemove--;

  for (std::set<ptr_tag<MathStatement> >::iterator i(aEqIt); i != aUseMathStatements.end();)
  {
    if ((*i)->mType == MathStatement::INITIAL_ASSIGNMENT || (*i)->mType == MathStatement::SAMPLE_FROM_DIST)
    {
      i++;
      continue;
    }

    // We do this insert and erase thing rather than copy the set to save stack
    // space.
    std::set<ptr_tag<MathStatement> >::iterator sysEq(aNonSystem.insert(*i).first);
    i++;

    if (aNeedToRemove > 0)
    {
      if (RecursivelyTestBigSystem(aNonSystem, i, aNeedToRemove, aUseMathStatements,
                                   aUseVars, aStart, aCandidates))
        return true;
    }
    else
    {
      // Build aUseMathStatements \ aNonSystem...
      std::set<ptr_tag<MathStatement> > syst;
      std::set_difference(aUseMathStatements.begin(),
                          aUseMathStatements.end(),
                          aNonSystem.begin(),
                          aNonSystem.end(),
                          std::inserter(syst, syst.end()));

      // Which variables does this involve?
      std::set<ptr_tag<CDA_ComputationTarget> > targs;
      for (std::set<ptr_tag<MathStatement> >::iterator j(syst.begin());
           j != syst.end(); j++)
        for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator k
               ((*j)->mTargets.begin());
             k != (*j)->mTargets.end(); k++)
          if (aCandidates.count(*k))
            targs.insert(*k);

      if (targs.size() != syst.size())
      {
        aNonSystem.erase(sysEq);
        continue;
      }

      // Well, we managed to shrink the system, so keep these changes and we
      // will see if we get any further later...

      aUseMathStatements = syst;
      aUseVars = targs;

      return true;
    }

    aNonSystem.erase(sysEq);
  }

  return false;
}

void
CodeGenerationState::BuildSystemsByTargetsRequired
(
 std::list<System*>& aSystems,
 std::map<ptr_tag<CDA_ComputationTarget>, System*>& aSysByTargReq
)
{
  aSysByTargReq.clear();

  for (std::list<System*>::iterator i = aSystems.begin();
       i != aSystems.end();
       i++)
  {
    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j = (*i)->mUnknowns.begin();
         j != (*i)->mUnknowns.end();
         j++)
      aSysByTargReq.insert(std::pair<ptr_tag<CDA_ComputationTarget>, System*>(*j, *i));
  }
}

void
CodeGenerationState::GenerateCodeForSet
(
 std::wstring& aCodeTo,
 std::set<ptr_tag<CDA_ComputationTarget> >& aKnown,
 std::list<System*>& aSystems,
 std::map<ptr_tag<CDA_ComputationTarget>, System*>&
   aSysByTargReq
)
{
  std::list<System*> sysCopy(aSystems);
  while (!sysCopy.empty())
  {
    System* sys = sysCopy.front();
    sysCopy.pop_front();

    // See if this system is already known. Since we always compute whole
    // systems at a time, we can do this by looking at the first target of the
    // system...
    if (aKnown.count(*sys->mUnknowns.begin()))
      continue;

    // Find everything we don't know but want to...
    std::list<System*> subtargets;
    
    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j = sys->mKnowns.begin();
         j != sys->mKnowns.end();
         j++)
    {
      if (aKnown.count(*j))
        continue;

      std::map<ptr_tag<CDA_ComputationTarget>, System*>::iterator sbtr
        (aSysByTargReq.find(*j));

      // This should be guaranteed if we got this far...
      assert(sbtr != aSysByTargReq.end());

      subtargets.push_back((*sbtr).second);
    }

    aKnown.insert(sys->mUnknowns.begin(), sys->mUnknowns.end());

    // Assign names to any infinitesimally delayed variables...
    for (std::set<ptr_tag<MathStatement> >::iterator i =
           sys->mMathStatements.begin();
         i != sys->mMathStatements.end();
         i++)
      for (
           std::list<ptr_tag<CDA_ComputationTarget> >::iterator dcti =
             (*i)->mDelayedTargets.begin();
           dcti != (*i)->mDelayedTargets.end();
           dcti++
          )
        /* XXX should we special case the situation where we are doing an
         * assign, and the delayed target is only overwritten after the assign?
         * What we have here is correct, but means we copy unnecessarily in that
         * particular case. We can't simply move this before the aKnown.insert
         * above, however, because that would break the case where we have to
         * solve for the non-delayed version of a delayed target.
         */
        if (aKnown.count(*dcti))
        {
          std::wstring ignore;
          ComputeInfDelayedName(*dcti, ignore);
        }

    GenerateCodeForSet(aCodeTo, aKnown, subtargets, aSysByTargReq);
    GenerateCodeForSystem(aCodeTo, sys);
  }
}

/*
 * Note: This function takes a set of variables which are either rates or
 * algebraic variables. It generates code in the correct place for both.
 */
void
CodeGenerationState::GenerateCodeForSetByType
(
 std::set<ptr_tag<CDA_ComputationTarget> >& aKnown,
 std::list<System*>& aSystems,
 std::map<ptr_tag<CDA_ComputationTarget>, System*>&
   aSysByTargReq
)
{
  // Make a list of all systems which allow us to compute rates...
  std::list<System*> rateSys;
  for
  (
   std::list<System*>::iterator i(aSystems.begin());
   i != aSystems.end();
   i++
  )
  {
    System* sys(*i);

    // See if this system is already known. Since we always compute whole
    // systems at a time, we can do this by looking at the first target of the
    // system...
    if (aKnown.count(*sys->mUnknowns.begin()))
      continue;

    // See if this system has a rate or infdelayed variable...
    bool hasRateOrInfdel(false);

    for (std::set<ptr_tag<CDA_ComputationTarget> >::iterator j(sys->mUnknowns.begin());
         j != sys->mUnknowns.end();
         j++)
      if ((*j)->mDegree != 0 || mDelayedTargs.count(*j) != 0 || (*j)->mIsReset)
      {
        hasRateOrInfdel = true;
        break;
      }

    if (hasRateOrInfdel)
      rateSys.push_back(sys);
  }

  GenerateCodeForSet(mCodeInfo->mRatesStr, aKnown, rateSys, aSysByTargReq);

  // And now everything else goes in mVarsStr
  GenerateCodeForSet(mCodeInfo->mVarsStr, aKnown, aSystems, aSysByTargReq);
}

static std::wstring
SubstituteCase
(
 std::pair<std::wstring, std::wstring>& aPair,
 std::wstring& aTemplate
)
{
  uint32_t state = 0;
  std::wstring result;
  for (std::wstring::iterator i = aTemplate.begin(); i != aTemplate.end();)
  {
    wchar_t c = *i;
    switch (state)
    {
    case 0:
      if (c != '<')
      {
        result += c;
        i++;
        continue;
      }
      else
      {
        state = 1;
        i++;
      }

      continue;

    default:
      if (c == 'C')
      {
        if (aTemplate.substr(i - aTemplate.begin(), 10) == L"CONDITION>")
        {
          result += aPair.second;
          i += 10;
          state = 0;
          continue;
        }
      }
      else if (c == 'S')
      {
        if (aTemplate.substr(i - aTemplate.begin(), 10) == L"STATEMENT>")
        {
          result += aPair.first;
          i += 10;
          state = 0;
          continue;
        }
      }
      result += L"<";
      state = 0;
      continue;
    }
  }

  return result;
}

void
CodeGenerationState::GenerateCasesIntoTemplate
(
 std::wstring& aCodeTo,
 std::list<std::pair<std::wstring, std::wstring> >& aCases
)
{
  size_t casepos = mConditionalAssignmentPattern.find(L"<CASES>"), initEnd, caseStart, caseEnd, finiStart;
  if (casepos == std::wstring::npos)
    initEnd = caseStart = caseEnd = finiStart = mConditionalAssignmentPattern.size();
  else
  {
    initEnd = casepos;
    caseStart = casepos + 7;
    caseEnd = mConditionalAssignmentPattern.find(L"</CASES>", caseStart);
    if (caseEnd == std::wstring::npos)
      finiStart = caseEnd = std::wstring::npos;
    else
      finiStart = caseEnd + 8;
  }
  std::wstring init = mConditionalAssignmentPattern.substr(0, initEnd);
  std::wstring each = mConditionalAssignmentPattern.substr(caseStart, caseEnd - caseStart);
  std::wstring fini = mConditionalAssignmentPattern.substr(finiStart);

  assert(aCases.begin() != aCases.end());
  aCodeTo += SubstituteCase(*(aCases.begin()), init);
  for (std::list<std::pair<std::wstring, std::wstring> >::iterator i = ++aCases.begin();
       i != aCases.end();
       i++)
  {
    aCodeTo += SubstituteCase(*i, each);
  }
  aCodeTo += SubstituteCase(*(aCases.begin()), fini);
}

void
CodeGenerationState::GenerateCodeForSystem
(
 std::wstring& aCodeTo,
 System* aSys
)
{
  // If the system is of size > 1, hand it off to the multivariate solver
  // code, since all of the algebraic simplification attempts below only work
  // for the univariate case...

  if (aSys->mMathStatements.size() > 1)
  {
    GenerateMultivariateSolveCode(aCodeTo, aSys);
    return;
  }

  MathStatement* ms = *(aSys->mMathStatements.begin());

  if (ms->mType == MathStatement::INITIAL_ASSIGNMENT)
  {
    // It is a simple LHS = RHS initial_value assignment.
    std::list<ptr_tag<CDA_ComputationTarget> >::iterator i = ms->mTargets.begin();
    ptr_tag<CDA_ComputationTarget> t1 = *i;
    i++;
    ptr_tag<CDA_ComputationTarget> t2 = *i;

    RETURN_INTO_OBJREF(localVarLHS, iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(ms->mContext, t1->mVariable));
    RETURN_INTO_OBJREF(localVarRHS, iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(ms->mContext, t2->mVariable));

    RETURN_INTO_OBJREF(di, iface::dom::DOMImplementation,
                       CreateDOMImplementation());
    RETURN_INTO_OBJREF(doc, iface::dom::Document,
                       di->createDocument(L"http://www.w3.org/1998/Math/MathML",
                                          L"ci", NULL));
    RETURN_INTO_OBJREF(dci, iface::dom::Element, doc->documentElement());
    RETURN_INTO_WSTRING(lvrhsname, localVarRHS->name());
    RETURN_INTO_OBJREF(tn, iface::dom::Text,
                       doc->createTextNode(lvrhsname.c_str()));
    dci->appendChild(tn)->release_ref();
    DECLARE_QUERY_INTERFACE_OBJREF(ci, dci, mathml_dom::MathMLElement);
    RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, ci,
                                             ms->mContext, localVarLHS,
                                             NULL, 0));
    GenerateAssignmentMaLaESResult(aCodeTo, t1, mr, t1->mVariable->name());
    return;
  }

  ptr_tag<CDA_ComputationTarget> computedTarget = *(aSys->mUnknowns.begin());

  // See if we have a piecewise. In that case, we can generate a series of statements...
  if (ms->mType == MathStatement::PIECEWISE)
  {
    Piecewise* pw = static_cast<Piecewise*>(ms);
    std::list<std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> > >::iterator i;
    std::list<std::pair<std::wstring, std::wstring> > cases;
    assert(!pw->mPieces.empty());
    for (i = pw->mPieces.begin(); i != pw->mPieces.end(); i++)
    {
      std::wstring statement;
      GenerateCodeForEquation(statement, (*i).first, computedTarget);

      RETURN_INTO_OBJREF(mr,
                         iface::cellml_services::MaLaESResult,
                         mTransform->transform
                         (mCeVAS, mCUSES, mAnnoSet, (*i).second->mMaths, pw->mContext,
                          NULL, NULL, 0));
      RETURN_INTO_WSTRING(cond, mr->expression());
      cases.push_back(std::pair<std::wstring, std::wstring>(statement, cond));
    }
    GenerateCasesIntoTemplate(aCodeTo, cases);
  }
  else if (ms->mType == MathStatement::SAMPLE_FROM_DIST)
    GenerateCodeForSampleFromDist(aCodeTo, static_cast<SampleFromDistribution*>(ms));
  else if (ms->mType == MathStatement::EQUATION)
    GenerateCodeForEquation(aCodeTo, static_cast<Equation*>(ms),
                            computedTarget);
}

static void
SubstituteVariableForStringEverywhere(iface::mathml_dom::MathMLElement* aExpr,
                                      const std::wstring& aCIName,
                                      const std::wstring& aSubstTo)
{
  // See if aExpr is a CI element...
  DECLARE_QUERY_INTERFACE_OBJREF(ciExpr, aExpr, mathml_dom::MathMLCiElement);
  if (ciExpr != NULL)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, ciExpr->getArgument(1));
    DECLARE_QUERY_INTERFACE_OBJREF(t, n, dom::Text);
    RETURN_INTO_WSTRING(txt, t->data());
    int i = 0, j = txt.length() - 1;
    wchar_t c;
    while ((c = txt[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
      i++;
    while ((c = txt[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
      j--;
    if (j < i)
      return;
    txt = txt.substr(i, j - i + 1);

    if (txt == aCIName)
    {
      RETURN_INTO_OBJREF(pn, iface::dom::Node, ciExpr->parentNode());
      RETURN_INTO_OBJREF(doc, iface::dom::Document, pn->ownerDocument());
      RETURN_INTO_OBJREF(csymEl, iface::dom::Element,
                         doc->createElementNS(MATHML_NS, L"csymbol"));
      DECLARE_QUERY_INTERFACE_OBJREF(csym, csymEl, mathml_dom::MathMLCsymbolElement);
      csym->definitionURL(L"http://www.cellml.org/tools/api#passthrough");
      RETURN_INTO_OBJREF(t, iface::dom::Text, doc->createTextNode(aSubstTo.c_str()));
      csym->appendChild(t)->release_ref();
      pn->replaceChild(csym, ciExpr)->release_ref();
    }

    return;
  }

  for (ObjRef<iface::dom::Node> n(already_AddRefd<iface::dom::Node>(aExpr->firstChild())); n != NULL;
       n = already_AddRefd<iface::dom::Node>(n->nextSibling()))
  {
    DECLARE_QUERY_INTERFACE_OBJREF(me, n, mathml_dom::MathMLElement);
    if (me == NULL)
      continue;

    SubstituteVariableForStringEverywhere(me, aCIName, aSubstTo);
  }
}

static void
ExtractPiecewiseConditionsFromExpression(iface::mathml_dom::MathMLElement* aEl,
                                         std::list<iface::mathml_dom::MathMLElement*>&aList)
{
  ObjRef<iface::mathml_dom::MathMLPiecewiseElement> pwel(QueryInterface(aEl));
  if (pwel != NULL)
  {
    ObjRef<iface::mathml_dom::MathMLNodeList> pcs(pwel->pieces());
    unsigned long nPieces = pcs->length();
    for (unsigned long i = 0; i < nPieces; i++)
    {
      ObjRef<iface::dom::Node> pieceEl(pcs->item(i));
      ObjRef<iface::mathml_dom::MathMLCaseElement> piece(QueryInterface(pieceEl));
      aList.push_back(piece->caseCondition());
    }
  }

  ObjRef<iface::mathml_dom::MathMLContainer> cel(QueryInterface(aEl));
  if (cel == NULL)
    return;

  unsigned int nargs = cel->nArguments();
  for (unsigned int i = 1; i <= nargs; i++)
  {
    ObjRef<iface::mathml_dom::MathMLElement> mel(cel->getArgument(i));
    ExtractPiecewiseConditionsFromExpression(mel, aList);
  }
}

static void
TryMakeAPiecewiseConditionIntoRoot(iface::mathml_dom::MathMLElement* aInput,
                                   std::list<iface::mathml_dom::MathMLElement*>& aOutput)
{
  ObjRef<iface::mathml_dom::MathMLApplyElement> ap(QueryInterface(aInput));
  if (ap == NULL)
    return;
  
  ObjRef<iface::mathml_dom::MathMLPredefinedSymbol> opel(QueryInterface(ap->_cxx_operator()));
  if (opel == NULL)
    return;
  
  std::wstring sn = opel->symbolName();
  if (sn == L"and" || sn == L"or")
  {
    unsigned long nargs = ap->nArguments();
    for (unsigned long i = 1; i <= nargs; i++)
    {
      ObjRef<iface::mathml_dom::MathMLElement> arg(ap->getArgument(i));
      TryMakeAPiecewiseConditionIntoRoot(arg, aOutput);
    }
  }
  else if (sn == L"eq" || sn == L"lt" || sn == L"gt" || sn == L"leq" || sn == L"geq")
  {
    if (ap->nArguments() >= 3)
    {
      ObjRef<iface::mathml_dom::MathMLElement> mel1 = ap->getArgument(2);
      ObjRef<iface::mathml_dom::MathMLElement> mel2 = ap->getArgument(3);
      ObjRef<iface::dom::Node> cmel1 = mel1->cloneNode(true);
      ObjRef<iface::dom::Node> cmel2 = mel2->cloneNode(true);
      ObjRef<iface::dom::Document> d(ap->ownerDocument());
      ObjRef<iface::mathml_dom::MathMLElement> newapply(QueryInterface(d->createElementNS(MATHML_NS, L"apply")));
      ObjRef<iface::dom::Element> minusop(d->createElementNS(MATHML_NS, L"minus"));
      newapply->appendChild(minusop)->release_ref();
      newapply->appendChild(cmel1)->release_ref();
      newapply->appendChild(cmel2)->release_ref();
      newapply->add_ref();
      aOutput.push_back(newapply.getPointer());
    }
  }
}

static void
TryMakePiecewiseConditionsIntoRoots(const std::list<iface::mathml_dom::MathMLElement*>& aInput,
                                    std::list<iface::mathml_dom::MathMLElement*>& aOutput)
{
  for (std::list<iface::mathml_dom::MathMLElement*>::const_iterator i = aInput.begin();
       i != aInput.end();
       i++)
    TryMakeAPiecewiseConditionIntoRoot(*i, aOutput);
}

void
CodeGenerationState::GenerateCodeForSampleFromDist(std::wstring& aCodeTo, SampleFromDistribution* aSFD)
{
  DECLARE_QUERY_INTERFACE_OBJREF(mae, aSFD->mDistrib, mathml_dom::MathMLApplyElement);
  if (mae == NULL)
    ContextError(L"Expected a distribution description in this context; distributions should always be described using an apply element.",
                 aSFD->mDistrib, aSFD->mContext);
  RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement, mae->_cxx_operator());
  DECLARE_QUERY_INTERFACE_OBJREF(csop, op, mathml_dom::MathMLCsymbolElement);
  if (csop == NULL)
    ContextError(L"Expected a csymbol describing the type of distribution.",
                 op, aSFD->mContext);
  RETURN_INTO_WSTRING(du, csop->definitionURL());
  if (du == L"http://www.cellml.org/uncertainty-1#distributionFromDensity")
  {
    std::wstring t = mSampleDensityFunctionPattern;
    wchar_t buf[30];
    any_swprintf(buf, 30, L"%u", mNextSolveId++);

    size_t pos = 0;
    while ((pos = t.find(L"<ID>", pos)) != std::wstring::npos)
      t.replace(pos, 4, buf);
    
    if (mae->nArguments() != 2)
      ContextError(L"distributionFromDensity descriptions should have exactly one argument, the probability density function.",
                   mae, aSFD->mContext);
    RETURN_INTO_OBJREF(pdf, iface::mathml_dom::MathMLElement, mae->getArgument(2));
    DECLARE_QUERY_INTERFACE_OBJREF(pdfl, pdf, mathml_dom::MathMLLambdaElement);
    if (pdfl == NULL)
      ContextError(L"The distributionFromDensity operator only takes a lambda function", pdf, aSFD->mContext);
    if (pdfl->nBoundVariables() != 1)
      ContextError(L"The distributionFromDensity operator expects a lambda function with exactly one bound variable.", pdf, aSFD->mContext);
    RETURN_INTO_OBJREF(bv, iface::mathml_dom::MathMLBvarElement, pdfl->getBoundVariable(1));
    if (bv->nArguments() != 1)
      ContextError(L"The distributionFromDensity operator expects a lambda function with exactly one bound variable.", bv, aSFD->mContext);
    RETURN_INTO_OBJREF(bvcontents, iface::mathml_dom::MathMLElement, bv->getArgument(1));
    DECLARE_QUERY_INTERFACE_OBJREF(bvci, bvcontents, mathml_dom::MathMLCiElement);
    if (bvci == NULL)
      ContextError(L"Expected a ci element inside the bvar element.", bv, aSFD->mContext);
    RETURN_INTO_OBJREF(n, iface::dom::Node, bvci->getArgument(1));
    DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);
    RETURN_INTO_WSTRING(txt, tn->data());
    int i = 0, j = txt.length() - 1;
    wchar_t c;
    while ((c = txt[i]) == ' ' || c == '\t' || c == '\r' || c == '\n')
      i++;
    while ((c = txt[j]) == ' ' || c == '\t' || c == '\r' || c == '\n')
      j--;
    if (j < i)
      ContextError(L"CI element with only spaces inside.", bvci, aSFD->mContext);
    txt = txt.substr(i, j - i + 1);

    if (pdfl->nArguments() != 1)
      ContextError(L"Expected lambda element to have one argument.", pdfl, aSFD->mContext);
    RETURN_INTO_OBJREF(exprOrig, iface::mathml_dom::MathMLElement, pdfl->getArgument(1));
    RETURN_INTO_OBJREF(exprN, iface::dom::Node, exprOrig->cloneNode(true));
    DECLARE_QUERY_INTERFACE_OBJREF(expr, exprN, mathml_dom::MathMLElement);
    SubstituteVariableForStringEverywhere(expr, txt, mBoundVariableName);

    RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, expr,
                                             aSFD->mContext, NULL,
                                             NULL, 0));
    for (uint32_t si = 0, sl = mr->supplementariesLength(); si < sl; si++)
    {
      RETURN_INTO_WSTRING(sup, mr->getSupplementary(si));
      mCodeInfo->mFuncsStr += sup;
    }
    RETURN_INTO_WSTRING(exprStr, mr->expression());

    std::list<iface::mathml_dom::MathMLElement*> pwCondList;
    scoped_destroy<std::list<iface::mathml_dom::MathMLElement*> >
      (pwCondList, new container_destructor<std::list<iface::mathml_dom::MathMLElement*> >
       (new objref_destructor<iface::mathml_dom::MathMLElement>()));
    ExtractPiecewiseConditionsFromExpression(expr, pwCondList);
    std::list<iface::mathml_dom::MathMLElement*> pwRootList;
    scoped_destroy<std::list<iface::mathml_dom::MathMLElement*> >
      (pwRootList, new container_destructor<std::list<iface::mathml_dom::MathMLElement*> >
       (new objref_destructor<iface::mathml_dom::MathMLElement>()));
    TryMakePiecewiseConditionsIntoRoots(pwCondList, pwRootList);
    std::list<std::wstring> pwRootStrs;
    for (std::list<iface::mathml_dom::MathMLElement*>::iterator rit = pwRootList.begin();
         rit != pwRootList.end(); rit++)
    {
      ObjRef<iface::cellml_services::MaLaESResult> mrpw
        (mTransform->transform(mCeVAS, mCUSES, mAnnoSet, *rit,
                               aSFD->mContext, NULL,
                               NULL, 0));
      for (uint32_t si = 0, sl = mrpw->supplementariesLength(); si < sl; si++)
      {
        RETURN_INTO_WSTRING(sup, mrpw->getSupplementary(si));
        mCodeInfo->mFuncsStr += sup;
      }
      pwRootStrs.push_back(mrpw->expression());
    }


    wchar_t rcBuf[30];
    any_swprintf(rcBuf, 30, L"%u", pwRootStrs.size());
    pos = 0;
    while ((pos = t.find(L"<ROOTCOUNT>", pos)) != std::wstring::npos)
      t.replace(pos, 11, rcBuf);

    pos = 0;
    for (pos = t.find(L"<FOREACH_ROOT>", pos); pos != std::wstring::npos; pos = t.find(L"<FOREACH_ROOT>", pos))
    {
      size_t posEnd = t.find(L"</FOREACH_ROOT>", pos);
      if (posEnd == std::wstring::npos)
        break;
      std::wstring templ = t.substr(pos + 14, posEnd - pos - 14);
      std::wstring rootStr;
      int rootID = 0;
      for (std::list<std::wstring>::iterator rootIt = pwRootStrs.begin(); rootIt != pwRootStrs.end(); rootIt++)
      {
        std::wstring thisRootStr = templ;
        wchar_t buf[30];
        any_swprintf(buf, 30, L"%u", rootID++);

        size_t rsPos = 0;
        while ((rsPos = thisRootStr.find(L"<EXPR>", rsPos)) != std::wstring::npos)
          thisRootStr.replace(rsPos, 6, *rootIt);
        rsPos = 0;
        while ((rsPos = thisRootStr.find(L"<ROOTID>", rsPos)) != std::wstring::npos)
          thisRootStr.replace(rsPos, 8, buf);

        rsPos = thisRootStr.find(L"<ROOTSUP>");
        if (rsPos == std::wstring::npos)
          rootStr += thisRootStr;
        else
        {
          rootStr += thisRootStr.substr(0, rsPos);
          mCodeInfo->mFuncsStr += thisRootStr.substr(rsPos + 9);
        }
      }
      t.replace(pos, posEnd - pos + 15, rootStr);
      pos++;
    }

    pos = 0;
    while ((pos = t.find(L"<EXPR>", pos)) != std::wstring::npos)
      t.replace(pos, 6, exprStr);

    // Look for <SUP> and split there if we need to...
    std::wstring main, sup;
    pos = t.find(L"<SUP>");
    if (pos == std::wstring::npos)
      main = t;
    else
    {
      main = t.substr(0, pos);
      sup = t.substr(pos + 5);
    }

    if (sup != L"")
      mCodeInfo->mFuncsStr += sup;

    RETURN_INTO_WSTRING(lhs, aSFD->mOutTargets.front()->name());
    if (mIsConstant)
      AppendConstantAssign(aCodeTo, lhs, main, describeMaths(aSFD));
    else
      AppendAssign(aCodeTo, lhs, main, describeMaths(aSFD));
  }
  else if (du == L"http://www.cellml.org/uncertainty-1#distributionFromRealisations")
  {
    if (mae->nArguments() != 2)
      ContextError(L"distributionFromRealisations should only have one argument - the vector of realisations.", mae, aSFD->mContext);
    RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, mae->getArgument(2));
    DECLARE_QUERY_INTERFACE_OBJREF(vec, arg, mathml_dom::MathMLVectorElement);
    if (vec == NULL)
      ContextError(L"Argument to distributionFromRealisations should be a vector.",
                   arg, aSFD->mContext);

    std::wstring output = mSampleRealisationsPattern;

    wchar_t buf[30];
    any_swprintf(buf, 30, L"%u", vec->ncomponents());
    size_t pos = 0;
    while ((pos = output.find(L"<numChoices>", pos)) != std::wstring::npos)
    {
      output.replace(pos, 12, buf);
      pos++;
    }

    size_t ecpos = output.find(L"<eachChoice>");
    if (ecpos != std::wstring::npos)
    {
      size_t ecEndPos = output.find(L"</eachChoice>", ecpos + 12);
      if (ecEndPos != std::wstring::npos)
      {
        std::wstring templ = output.substr(ecpos + 12, ecEndPos - ecpos - 12);
        std::wstring allChoices;
        for (uint32_t i = 1, l = vec->ncomponents(); i <= l; i++)
        {
          std::wstring subIn = templ, assignments;
          RETURN_INTO_OBJREF(ce, iface::mathml_dom::MathMLContentElement,
                             vec->getComponent(i));
          DECLARE_QUERY_INTERFACE_OBJREF(ivec, ce, mathml_dom::MathMLVectorElement);
          if (ivec != NULL)
          {
            if (ivec->ncomponents() != aSFD->mOutTargets.size())
              ContextError(L"Realisations of a distribution must be either a vector of constants or contants", ce, aSFD->mContext);
            std::vector<ptr_tag<CDA_ComputationTarget> >::iterator k;
            uint32_t j = 1;
            for (j = 1, k = aSFD->mOutTargets.begin(); k != aSFD->mOutTargets.end();
                 k++, j++)
            {
              RETURN_INTO_OBJREF(mel, iface::mathml_dom::MathMLContentElement,
                                 ivec->getComponent(j));
              RETURN_INTO_OBJREF(localVar, iface::cellml_api::CellMLVariable,
                                 GetVariableInComponent(aSFD->mContext,
                                                        (*k)->mVariable));
              RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                                 mTransform->transform(mCeVAS, mCUSES, mAnnoSet, mel,
                                                       aSFD->mContext, localVar,
                                                       NULL, 0));
              GenerateAssignmentMaLaESResult(assignments, *k, mr, describeMaths(aSFD));
            }
          }
          else if (aSFD->mOutTargets.size() != 1)
            ContextError(L"Realisations of a distribution that generates more than one output must be a vector", ce, aSFD->mContext);
          else
          {
            ptr_tag<CDA_ComputationTarget> ct = *aSFD->mOutTargets.begin();
            RETURN_INTO_OBJREF(localVar, iface::cellml_api::CellMLVariable,
                               GetVariableInComponent(aSFD->mContext,
                                                      ct->mVariable));
            RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                               mTransform->transform(mCeVAS, mCUSES, mAnnoSet, ce,
                                                     aSFD->mContext, localVar,
                                                     NULL, 0));
            GenerateAssignmentMaLaESResult(assignments, ct, mr, describeMaths(aSFD));
          }

          any_swprintf(buf, 30, L"%u", i - 1);
          pos = 0;
          while ((pos = subIn.find(L"<choiceNumber>", pos)) != std::wstring::npos)
            subIn.replace(pos, 14, buf);
          pos = 0;
          while ((pos = subIn.find(L"<choiceAssignments>", pos)) != std::wstring::npos)
            subIn.replace(pos, 19, assignments);

          allChoices += subIn;
        }
        output.replace(ecpos, ecEndPos - ecpos + 13, allChoices);
      }
    }
    aCodeTo += output;
  }
  else
    ContextError(L"The only supported ways to specify distributions for uncertain parameters are distributionFromDensity and distributionFromRealisations", csop, aSFD->mContext);
}

void
CodeGenerationState::GenerateCodeForEquation
(
 std::wstring& aCodeTo,
 Equation* aEq,
 ptr_tag<CDA_ComputationTarget> aComputedTarget,
 bool aAssignmentOnly
)
{
  // If we get here, we do have an equation. However, we are not yet sure
  // if we need to do a non-linear solve to evaluate it.

  bool swapOk = false;

  if (aComputedTarget->mDegree == 0)
  {
    // It isn't a derivative we are after, so see if one side or the other is a
    // CI...
    DECLARE_QUERY_INTERFACE_OBJREF(rhsci, aEq->mRHS, mathml_dom::MathMLCiElement);
    DECLARE_QUERY_INTERFACE_OBJREF(lhsci, aEq->mLHS, mathml_dom::MathMLCiElement);
    if (lhsci == NULL && rhsci == NULL)
    {
      if (aAssignmentOnly)
        throw AssignmentOnlyRequestedNeedSolve();

      GenerateSolveCode(aCodeTo, aEq, aComputedTarget);
      aEq->mLHS = NULL;
      return;
    }

    if (lhsci == NULL)
    {
      aEq->mRHS = aEq->mLHS;
      aEq->mLHS = rhsci;
    }

    if (lhsci != NULL && rhsci != NULL)
      swapOk = true;
  }
  else
  {
    // We want to evaluate a derivative, so see if there is a derivative by
    // itself on either side of the equation...
    DECLARE_QUERY_INTERFACE_OBJREF(rhsapply, aEq->mRHS,
                                   mathml_dom::MathMLApplyElement);
    DECLARE_QUERY_INTERFACE_OBJREF(lhsapply, aEq->mLHS,
                                   mathml_dom::MathMLApplyElement);
    bool lhsIsDiff = false, rhsIsDiff = false;
    if (rhsapply)
    {
      RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                         rhsapply->_cxx_operator());
      RETURN_INTO_WSTRING(ln, op->localName());
      if (ln == L"diff")
        rhsIsDiff = true;
    }
    if (lhsapply)
    {
      RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                         lhsapply->_cxx_operator());
      RETURN_INTO_WSTRING(ln, op->localName());
      if (ln == L"diff")
        lhsIsDiff = true;
    }

    if (lhsIsDiff == false && rhsIsDiff == false)
    {
      if (aAssignmentOnly)
        throw AssignmentOnlyRequestedNeedSolve();

      GenerateSolveCode(aCodeTo, aEq, aComputedTarget);
      aEq->mLHS = NULL;
      return;
    }

    if (!lhsIsDiff)
    {
      aEq->mRHS = aEq->mLHS;
      aEq->mLHS = rhsapply;
    }
    else
      swapOk = true;
  }

  do
  {
    // If the LHS doesn't involve the target we want, we have to swap...
    RETURN_INTO_OBJREF(mr,
                       iface::cellml_services::MaLaESResult,
                       mTransform->transform
                       (mCeVAS, mCUSES, mAnnoSet, aEq->mLHS, aEq->mContext,
                        NULL, NULL, 0));
    RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                       mr->iterateInvolvedVariablesByDegree());
    RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable,
                       dvi->nextDegreeVariable());
    if (dv == NULL)
      ContextError(L"Couldn't find variable in ci",
                   NULL, aEq->mContext);

    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, dv->variable());
    if (dv->degree() != aComputedTarget->mDegree ||
        CDA_objcmp(cv, aComputedTarget->mVariable))
    {
      if (!swapOk)
        break;

      // It didn't work out that way around, but swapping is allowed...
      swapOk = false;
      ObjRef<iface::mathml_dom::MathMLElement> tmp(aEq->mRHS);
      aEq->mRHS = aEq->mLHS;
      aEq->mLHS = tmp;
      continue;
    }

    // The LHS has the variable we want by itself. However, the RHS might also
    // have the variable (e.g. x = x^2), so we start off as if it doesn't but
    // check that assumption and bail to the NR solver if it doesn't hold.

    RETURN_INTO_OBJREF(
                       localVar,
                       iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(
                                              aEq->mContext,
                                              aComputedTarget->mVariable
                                             )
                      );

    ObjRef<iface::cellml_api::CellMLVariable> localBound;

    if (mBoundTargs.begin() != mBoundTargs.end())
    {
      localBound = already_AddRefd<iface::cellml_api::CellMLVariable>
        (
         GetVariableInComponent(
                                aEq->mContext,
                                (*mBoundTargs.begin())->mVariable
                               )
        );
    }

    mr = already_AddRefd<iface::cellml_services::MaLaESResult>
      (
       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aEq->mRHS,
                             aEq->mContext, localVar, localBound,
                             aComputedTarget->mDegree)
      );

    dvi = already_AddRefd<iface::cellml_services::DegreeVariableIterator>
      (
       mr->iterateInvolvedVariablesByDegree()
      );
    
    bool match = false;
    while (true)
    {
      dv = already_AddRefd<iface::cellml_services::DegreeVariable>
        (dvi->nextDegreeVariable());
      if (dv == NULL)
        break;
      if (dv->degree() != aComputedTarget->mDegree)
        continue;
      RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                         dv->variable());
      if (CDA_objcmp(cv, aComputedTarget->mVariable))
        continue;

      match = true;
      break;
    }
    if (match)
      break;

    // We have the variable we want by itself on one side of the equation. A
    // straight assignment will suffice.
    GenerateAssignmentMaLaESResult(aCodeTo, aComputedTarget, mr, describeMaths(aEq));
    return;
  }
  while (true);

  if (aAssignmentOnly)
    throw AssignmentOnlyRequestedNeedSolve();

  GenerateSolveCode(aCodeTo, aEq, aComputedTarget);
  aEq->mLHS = NULL;
}

void
CodeGenerationState::GenerateStateToRateCascades()
{
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator i;
  for (i = mBaseTargets.begin(); i != mBaseTargets.end(); i++)
  {
    ptr_tag<CDA_ComputationTarget> ct = (*i)->mUpDegree;
    if (ct == NULL)
      continue;

    while (ct->mUpDegree != NULL)
    {
      RETURN_INTO_WSTRING(stateN, ct->name());
      std::wstring rateN;

      GenerateVariableName(rateN, mRateNamePattern,
                           ct->mAssignedIndex - 1);

      AppendAssign(mCodeInfo->mRatesStr, rateN, mTransform->wrapNumber(stateN),
                   ct->mVariable->name());
      ct = ct->mUpDegree;
    }
  }
}

void
CodeGenerationState::GenerateInfDelayUpdates()
{
  if (mInfDelayedTargets.empty())
    return;

  std::wstring oldRates = mCodeInfo->mRatesStr;
  mCodeInfo->mRatesStr = L"";
  for (std::list<ptr_tag<CDA_ComputationTarget> >::iterator i =
         mInfDelayedTargets.begin();
       i != mInfDelayedTargets.end();
       i++)
  {
    std::wstring delname;
    ComputeInfDelayedName(*i, delname);
    RETURN_INTO_WSTRING(name, (*i)->name());

    AppendAssign(mCodeInfo->mRatesStr, delname,
                 mTransform->wrapNumber(name),
                 (*i)->mVariable->name());
  }

  mCodeInfo->mRatesStr += oldRates;
}

iface::cellml_api::CellMLVariable*
CodeGenerationState::GetVariableInComponent
(
 iface::cellml_api::CellMLComponent* aComp,
 iface::cellml_api::CellMLVariable* aVar
)
{
  RETURN_INTO_OBJREF(cvs, iface::cellml_services::ConnectedVariableSet,
                     mCeVAS->findVariableSet(aVar));
  uint32_t l = cvs->length(), i = 0;
  for (; i < l; i++)
  {
    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable,
                       cvs->getVariable(i));
    RETURN_INTO_OBJREF(cel, iface::cellml_api::CellMLElement,
                       v->parentElement());
    if (!CDA_objcmp(cel, aComp))
    {
      v->add_ref();
      return v;
    }
  }

  return NULL;
}

void
CodeGenerationState::GenerateAssignmentMaLaESResult
(
 std::wstring& aCodeTo,
 ptr_tag<CDA_ComputationTarget> aTarget,
 iface::cellml_services::MaLaESResult* aMR,
 const std::wstring& aXMLId
)
{
  RETURN_INTO_WSTRING(lhs, aTarget->name());
  RETURN_INTO_WSTRING(rhs, aMR->expression());

  uint32_t l = aMR->supplementariesLength(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_WSTRING(s, aMR->getSupplementary(i));
    if (!mDryRun)
      mCodeInfo->mFuncsStr += s + L"\r\n";
  }

  if (mIsConstant)
    AppendConstantAssign(aCodeTo, lhs, rhs, aXMLId);
  else
    AppendAssign(aCodeTo, lhs, rhs, aXMLId);
}

void
CodeGenerationState::GenerateSolveCode
(
 std::wstring& aCodeTo,
 Equation* aEq,
 ptr_tag<CDA_ComputationTarget> aComputedTarget
)
{
  RETURN_INTO_OBJREF(mr1, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aEq->mLHS,
                                           aEq->mContext, NULL, NULL, 0));
  RETURN_INTO_OBJREF(mr2, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aEq->mRHS,
                                           aEq->mContext, NULL, NULL, 0));
  aEq->mMaths->add_ref();
  mCodeInfo->mFlaggedEquations.push_back(aEq->mMaths);
  RETURN_INTO_WSTRING(e1, mr1->expression());
  RETURN_INTO_WSTRING(e2, mr2->expression());
  uint32_t l = mr1->supplementariesLength(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_WSTRING(s, mr1->getSupplementary(i));
    mCodeInfo->mFuncsStr += s;
  }
  
  l = mr2->supplementariesLength();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_WSTRING(s, mr2->getSupplementary(i));
    mCodeInfo->mFuncsStr += s;
  }
   // Scoped locale change.
  CNumericLocale locobj;
  wchar_t id[20];
  any_swprintf(id, 20, L"%u", mNextSolveId++);
  RETURN_INTO_WSTRING(vname, aComputedTarget->name());

  wchar_t iv[30] = { L'0', L'.', L'1', L'\0' };
  std::map<ptr_tag<CDA_ComputationTarget>, double>::iterator ivIt
    (mInitialOverrides.find(aComputedTarget));
  if (ivIt != mInitialOverrides.end())
    any_swprintf(iv, 30, L"%g", (*ivIt).second);
  uint32_t state = 0;
  uint32_t idx = 0;
  std::wstring* dest = &aCodeTo;
  for (idx = 0; idx < mSolvePattern.length(); idx++)
  {
    wchar_t c = mSolvePattern[idx];
    switch (state)
    {
    case 0:
      if (c == L'<')
        state = 1;
      else
        *dest += c;
      continue;
    case 1: // Seen <
      if (c == L'L')
        state = 2;
      else if (c == L'R')
        state = 3;
      else if (c == L'I')
        state = 4;
      else if (c == L'V')
        state = 5;
      else if (c == L'S')
        state = 13;
      else if (c == L'X')
        state = 17;
      else
      {
        *dest += L'<';
        *dest += c;
        state = 0;
      }
      break;
    case 2: // Seen <L
      if (c == L'H')
        state = 6;
      else
      {
        *dest += L"<L";
        *dest += c;
        state = 0;
      }
      break;
    case 3: // Seen <R
      if (c == L'H')
        state = 8;
      else
      {
        *dest += L"<R";
        *dest += c;
        state = 0;
      }
      break;
    case 4: // Seen <I
      if (c == L'D')
        state = 10;
      else if (c == L'V')
        state = 16;
      else
      {
        *dest += L"<I";
        *dest += c;
        state = 0;
      }
      break;
    case 5: // Seen <V
      if (c == L'A')
        state = 11;
      else
      {
        *dest += L"<V";
        *dest += c;
        state = 0;
      }
      break;
    case 6: // Seen <LH
      if (c == L'S')
        state = 7;
      else
      {
        *dest += L"<LH";
        *dest += c;
        state = 0;
      }
      break;
    case 7: // Seen <LHS
      if (c == L'>')
      {
        // Matched <LHS>
        *dest += e1;
        state = 0;
      }
      else
      {
        *dest += L"<LHS";
        *dest += c;
        state = 0;
      }
      break;
    case 8: // Seen <RH
      if (c == L'S')
        state = 9;
      else
      {
        *dest += L"<RH";
        *dest += c;
        state = 0;
      }
      break;
    case 9: // Seen <RHS
      if (c == L'>')
      {
        // Matched <RHS>
        *dest += e2;
        state = 0;
      }
      else
      {
        *dest += L"<RHS";
        *dest += c;
        state = 0;
      }
      break;
    case 10: // Seen <ID
      if (c == L'>')
      {
        // Matched <ID>
        *dest += id;
        state = 0;
      }
      else
      {
        *dest += L"<ID";
        *dest += c;
        state = 0;
      }
      break;
    case 11: // Seen <VA
      if (c == L'R')
        state = 12;
      else
      {
        *dest += L"<VA";
        *dest += c;
        state = 0;
      }
      break;
    case 12: // Seen <VAR
      if (c == L'>')
      {
        // Matched <VAR>
        *dest += vname;
        state = 0;
      }
      else
      {
        *dest += L"<VAR";
        *dest += c;
        state = 0;
      }
      break;
    case 13: // Seen <S
      if (c == L'U')
        state = 14;
      else
      {
        *dest += L"<S";
        *dest += c;
        state = 0;
      }
      break;
    case 14: // Seen <SU
      if (c == L'P')
        state = 15;
      else
      {
        *dest += L"<SU";
        *dest += c;
        state = 0;
      }
      break;
    case 15: // Seen <SUP
      if (c == L'>')
      {
        // Matched <SUP>
        dest = &mCodeInfo->mFuncsStr;
        state = 0;
      }
      break;
    case 16: // Seen <IV
      if (c == L'>')
      {
        // Matched <IV>
        *dest += iv;
        state = 0;
      }
      else
      {
        *dest += L"<IV";
        *dest += c;
        state = 0;
      }
      break;
    case 17: // Seen <X
      if (mSolvePattern.substr(idx, 5) == L"MLID>")
      {
        *dest += describeMaths(aEq);
        idx += 4;
        state = 0;
      }
      else
      {
        state = 0;
        *dest += L"<X";
        *dest += c;
      }
      break;
    }
  }
}

void
CodeGenerationState::GenerateMultivariateSolveCode
(
 std::wstring& aCodeTo,
 System* aSys
)
{
  /*
   * Annotate the equation with the code... We implement a bit of a kludge
   * here: because we assume the number of equations is equal to the number of
   * variables, we iterate through the unknown targets at the same time so
   * that in our specification language we can use a single
   * <EQUATION></EQUATION> iterator construct.
   */
  std::set<ptr_tag<CDA_ComputationTarget> >::iterator k(aSys->mUnknowns.begin());
  for (std::set<ptr_tag<MathStatement> >::iterator i(aSys->mMathStatements.begin());
       i != aSys->mMathStatements.end();
       i++, k++)
  {
    MathStatement * ms(*i);
    if (ms->mType == MathStatement::INITIAL_ASSIGNMENT)
      throw CodeGenerationError(L"The 'impossible' happened - multivariate system contains initial assignment.");
    MathMLMathStatement* mms = static_cast<MathMLMathStatement*>(ms);

    RETURN_INTO_OBJREF(doc, iface::dom::Document, mms->mMaths->ownerDocument());
    ObjRef<iface::mathml_dom::MathMLElement> topel;

    if (mms->mType == MathStatement::PIECEWISE)
    {
      RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(MATHML_NS, L"piecewise"));
      QUERY_INTERFACE(topel, el, mathml_dom::MathMLElement);

      Piecewise* pw = static_cast<Piecewise*>(mms);
      for (std::list<std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> > >::iterator j = pw->mPieces.begin();
           j != pw->mPieces.end();
           j++)
      {
        RETURN_INTO_OBJREF(piece, iface::dom::Element, doc->createElementNS(MATHML_NS, L"piece"));
        el->appendChild(piece)->release_ref();
        RETURN_INTO_OBJREF(val, iface::dom::Element, doc->createElementNS(MATHML_NS, L"apply"));
        RETURN_INTO_OBJREF(minus, iface::dom::Element, doc->createElementNS(MATHML_NS, L"minus"));
        val->appendChild(minus)->release_ref();
        RETURN_INTO_OBJREF(lclone, iface::dom::Node, (*j).first->mLHS->cloneNode(true));
        RETURN_INTO_OBJREF(rclone, iface::dom::Node, (*j).first->mRHS->cloneNode(true));
        val->appendChild(lclone)->release_ref();
        val->appendChild(rclone)->release_ref();
        piece->appendChild(val)->release_ref();
        RETURN_INTO_OBJREF(cond, iface::dom::Node, (*j).second->mMaths->cloneNode(true));
        piece->appendChild(cond)->release_ref();
      }
    }
    else if (mms->mType == MathStatement::EQUATION)
    {
      Equation* eq = static_cast<Equation*>(mms);

      RETURN_INTO_OBJREF(val, iface::dom::Element, doc->createElementNS(MATHML_NS, L"apply"));
      RETURN_INTO_OBJREF(minus, iface::dom::Element, doc->createElementNS(MATHML_NS, L"minus"));
      val->appendChild(minus)->release_ref();
      RETURN_INTO_OBJREF(lclone, iface::dom::Node, eq->mLHS->cloneNode(true));
      RETURN_INTO_OBJREF(rclone, iface::dom::Node, eq->mRHS->cloneNode(true));
      val->appendChild(lclone)->release_ref();
      val->appendChild(rclone)->release_ref();

      QUERY_INTERFACE(topel, val, mathml_dom::MathMLElement);
    }
    else
      // TODO: Fix this, probably by including inequalities in a separate list.
      continue;

    RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, topel,
                                             ms->mContext, NULL, NULL, 0));

    mms->mMaths->add_ref();
    mCodeInfo->mFlaggedEquations.push_back(mms->mMaths);

    RETURN_INTO_WSTRING(e, mr->expression());

    ms->mCode = e;

    uint32_t l = mr->supplementariesLength(), j;
    for (j = 0; j < l; j++)
    {
      RETURN_INTO_WSTRING(s, mr->getSupplementary(j));
      mCodeInfo->mFuncsStr += s;
    }

    RETURN_INTO_WSTRING(vname, (*k)->name());
    ms->mVarName = vname;
  }

  // Scoped locale change.
  CNumericLocale locobj;
  wchar_t id[20];
  any_swprintf(id, 20, L"%u", mNextSolveId++);

  // See if there is a <SUP> marker...
  size_t supPos = mSolveNLSystemPattern.find(L"<SUP>");
  if (supPos == std::wstring::npos)
    GenerateMultivariateSolveCodeTo(aCodeTo, aSys, mSolveNLSystemPattern, id);
  else
  {
    GenerateMultivariateSolveCodeTo(aCodeTo, aSys,
                                    mSolveNLSystemPattern.substr(0, supPos), id);
    GenerateMultivariateSolveCodeTo(mCodeInfo->mFuncsStr, aSys,
                                    mSolveNLSystemPattern.substr(supPos + 5), id);
  }
}

std::wstring
CodeGenerationState::ReplaceIDs(std::wstring aReplaceFrom,
                                const wchar_t* aId,
                                const wchar_t* aIndex,
                                const wchar_t* aCount)
{
  size_t offset = 0;

  while (true)
  {
    offset = aReplaceFrom.find(L"<ID>", offset);
    if (offset == std::wstring::npos)
      break;

    aReplaceFrom.replace(offset, 4, aId);
    offset++;
  }
  offset = 0;
  while (true)
  {
    offset = aReplaceFrom.find(L"<INDEX>", offset);
    if (offset == std::wstring::npos)
      break;

    aReplaceFrom.replace(offset, 7, aIndex);
    offset++;
  }
  offset = 0;
  while (true)
  {
    offset = aReplaceFrom.find(L"<COUNT>", offset);
    if (offset == std::wstring::npos)
      break;

    aReplaceFrom.replace(offset, 7, aCount);
    offset++;
  }

  return aReplaceFrom;
}

void
CodeGenerationState::GenerateMultivariateSolveCodeTo
(
 std::wstring& aCodeTo,
 System* aSys,
 const std::wstring& aPattern,
 const wchar_t* aId
)
{
  // Look for <EQUATIONS>...
  size_t offset = 0;
  size_t occurrence;

  wchar_t countStr[15];
  any_swprintf(countStr, 15, L"%u", aSys->mMathStatements.size());

  while ((occurrence = aPattern.find(L"<EQUATIONS>", offset))
         != std::wstring::npos)
  {
    aCodeTo += ReplaceIDs(aPattern.substr(offset, occurrence - offset),
                          aId, L"", countStr);
    offset = occurrence + 11;

    occurrence = aPattern.find(L"</EQUATIONS>", offset);
    if (occurrence == std::wstring::npos)
    {
      // This only happens if we were given a dubious string with no closing
      // </EQUATIONS>...
      offset -= 11;
      break;
    }

    std::wstring perEqPattern(aPattern.substr(offset, occurrence - offset));

    size_t joinOffset(perEqPattern.find(L"<JOIN>"));
    std::wstring join;
    if (joinOffset != std::wstring::npos)
    {
      join = perEqPattern.substr(joinOffset + 6);
      perEqPattern = perEqPattern.substr(0, joinOffset);
    }

    uint32_t index = 0 + mArrayOffset;
    std::set<ptr_tag<MathStatement> >::iterator i;
    std::set<ptr_tag<CDA_ComputationTarget> >::iterator j;
    for(i = aSys->mMathStatements.begin(), j = aSys->mUnknowns.begin();
        i != aSys->mMathStatements.end(); i++, j++)
    {
      wchar_t ivStr[30] = {L'0', L'.', L'1', L'\0'};
      std::map<ptr_tag<CDA_ComputationTarget>, double>::iterator ioi(mInitialOverrides.find(*j));
      if (ioi != mInitialOverrides.end())
        any_swprintf(ivStr, 30, L"%g", (*ioi).second);

      wchar_t indexStr[15];
      any_swprintf(indexStr, 15, L"%u", index);
      index++;
      if (i != aSys->mMathStatements.begin())
        aCodeTo += ReplaceIDs(join, aId, indexStr, countStr);
      GenerateMultivariateSolveCodeEq(aCodeTo, *i, perEqPattern, aId, indexStr,
                                      ivStr);
    }

    offset = occurrence + 12;
  }

  aCodeTo += ReplaceIDs(aPattern.substr(offset), aId, L"", countStr);
}

void
CodeGenerationState::GenerateMultivariateSolveCodeEq
(
 std::wstring& aCodeTo,
 ptr_tag<MathStatement> aEq,
 const std::wstring& aPattern,
 const wchar_t* aId,
 const wchar_t* aIndex,
 const wchar_t* aIV
)
{
  uint32_t state = 0;
  uint32_t idx = 0;
  for (idx = 0; idx < aPattern.length(); idx++)
  {
    wchar_t c = aPattern[idx];
    switch (state)
    {
    case 0:
      if (c == L'<')
        state = 1;
      else
        aCodeTo += c;
      continue;
    case 1: // Seen <
      if (c == L'E')
        state = 2;
      else if (c == L'I')
        state = 4;
      else if (c == L'V')
        state = 5;
      else if (c == L'X')
        state = 18;
      else
      {
        aCodeTo += L'<';
        aCodeTo += c;
        state = 0;
      }
      break;
    case 2: // Seen <E
      if (c == L'X')
        state = 6;
      else
      {
        aCodeTo += L"<E";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 4: // Seen <I
      if (c == L'D')
        state = 10;
      else if (c == L'N')
        state = 13;
      else if (c == L'V')
        state = 17;
      else
      {
        aCodeTo += L"<I";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 5: // Seen <V
      if (c == L'A')
        state = 11;
      else
      {
        aCodeTo += L"<V";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 6: // Seen <EX
      if (c == L'P')
        state = 7;
      else
      {
        aCodeTo += L"<EX";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 7: // Seen <EXP
      if (c == L'R')
        state = 8;
      else
      {
        aCodeTo += L"<EXP";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 8: // Seen <EXPR
      if (c == L'>')
      {
        // Matched <EXPR>
        aCodeTo += aEq->mCode;
        state = 0;
      }
      else
      {
        aCodeTo += L"<EXPR";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 10: // Seen <ID
      if (c == L'>')
      {
        // Matched <ID>
        aCodeTo += aId;
        state = 0;
      }
      else
      {
        aCodeTo += L"<ID";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 11: // Seen <VA
      if (c == L'R')
        state = 12;
      else
      {
        aCodeTo += L"<VA";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 12: // Seen <VAR
      if (c == L'>')
      {
        // Matched <VAR>
        aCodeTo += aEq->mVarName;
        state = 0;
      }
      else
      {
        aCodeTo += L"<VAR";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 13: // Seen <IN
      if (c == L'D')
        state = 14;
      else
      {
        aCodeTo += L"<IN";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 14: // Seen <IND
      if (c == L'E')
        state = 15;
      else
      {
        aCodeTo += L"<IND";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 15: // Seen <INDE
      if (c == L'X')
        state = 16;
      else
      {
        aCodeTo += L"<INDE";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 16: // Seen <INDEX
      if (c == L'>')
      {
        // Matched <INDEX>
        aCodeTo += aIndex;
        state = 0;
      }
      else
      {
        aCodeTo += L"<INDEX";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 17: // Seen <IV
      if (c == L'>')
      {
        // matched <IV>
        aCodeTo += aIV;
        state = 0;
      }
      else
      {
        aCodeTo += L"<IV";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 18: // Seen <X
      if (aPattern.substr(idx, 5) == L"MLID>")
      {
        aCodeTo += describeMaths(aEq);
        idx += 4;
        state = 0;
      }
      else
      {
        state = 0;
        aCodeTo += L"<X";
        aCodeTo += c;
      }
      break;
    }
  }
}
