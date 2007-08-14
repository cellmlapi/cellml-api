#include "CCGSImplementation.hpp"
#include <map>
#include <set>
#include <math.h>
#include "CodeGenerationState.hxx"
#include <stdio.h>
#include "CodeGenerationError.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "DOMBootstrap.hxx"

// Win32 hack...
#ifdef _WIN32
#define swprintf _snwprintf
#endif

CodeGenerationState::~CodeGenerationState()
{
  std::list<VariableEdge*>::iterator i;
  for (i = mVariableEdges.begin(); i != mVariableEdges.end(); i++)
    delete *i;
}

iface::cellml_services::CodeInformation*
CodeGenerationState::GenerateCode()
{
  // Create a new code information object...
  mCodeInfo = already_AddRefd<CDA_CodeInformation>
    (new CDA_CodeInformation());
  mCodeInfo->mConstraintLevel = iface::cellml_services::CORRECTLY_CONSTRAINED;
  mCodeInfo->mRateIndexCount = 0;
  mCodeInfo->mConstantIndexCount = 0;
  mCodeInfo->mAlgebraicIndexCount = 0;

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
    CreateVariableEdges();

    // Next, set starting classification for all targets...
    FirstPassTargetClassification();

    mUnusedEdges = mVariableEdges;

    // Put all targets into lists based on their classification...
    BuildFloatingAndConstantLists();

    // Now, determine all constants computable from the current constants...
    std::set<CDA_ComputationTarget*> reachabletargets;
    BuildTargetSet(mKnown, mFloating, reachabletargets);

    // Assign constant variables for set...
    AllocateVariablesInSet(reachabletargets, iface::cellml_services::CONSTANT,
                           mConstantPattern, mNextConstantIndex,
                           mCodeInfo->mConstantIndexCount);

    // Write evaluations for all constants we just worked out how to compute...
    GenerateCodeForSet(mCodeInfo->mInitConstsStr, reachabletargets);

    // Also we need to initialise state variable IVs...
    reachabletargets.clear();
    BuildStateAndConstantLists();
    BuildTargetSet(mKnown, mFloating, reachabletargets);
    GenerateCodeForSet(mCodeInfo->mInitConstsStr, reachabletargets);

    // Put all targets into lists based on their classification...
    BuildFloatingAndKnownLists();

    // Now, determine all algebraic variables / rates computable from the
    // known variables (constants, state variables, and bound variable)...
    reachabletargets.clear();

    if (BuildTargetSet(mKnown, mFloating, reachabletargets) != 0)
    {
      if (mUnusedEdges.size() != 0)
        throw UnsuitablyConstrainedError();
      else
        throw UnderconstrainedError();
    }

    // Assign algebraic variables for set...
    AllocateVariablesInSet(reachabletargets, iface::cellml_services::ALGEBRAIC,
                           mAlgebraicVariableNamePattern,
                           mNextAlgebraicVariableIndex,
                           mCodeInfo->mAlgebraicIndexCount);

    // Write evaluations for all rates & algebraic variables in reachabletargets
    GenerateCodeForSetByType(reachabletargets);

    // Also cascade state variables to rate variables where they are the same
    // (e.g. if d^2y/dx^2 = constant then dy/dx is a state variable due to the
    //  above equation, but also it is a rate for y).
    GenerateStateToRateCascades();
  }
  catch (UnderconstrainedError uce)
  {
    mCodeInfo->mConstraintLevel = iface::cellml_services::UNDERCONSTRAINED;
  }
  catch (OverconstrainedError oce)
  {
    std::vector<iface::dom::Element*>::iterator fei;
    for (fei = mCodeInfo->mFlaggedEquations.begin();
         fei != mCodeInfo->mFlaggedEquations.end();
         fei++)
      (*fei)->release_ref();
    mCodeInfo->mFlaggedEquations.clear();
    oce.mEqn->add_ref();
    mCodeInfo->mFlaggedEquations.push_back(oce.mEqn);
    mCodeInfo->mConstraintLevel = iface::cellml_services::OVERCONSTRAINED;
  }
  catch (UnsuitablyConstrainedError uce)
  {
    mCodeInfo->mConstraintLevel = iface::cellml_services::UNSUITABLY_CONSTRAINED;
  }
  catch (CodeGenerationError cge)
  {
    mCodeInfo->mErrorMessage = cge.str();
  }

  mCodeInfo->add_ref();
  return mCodeInfo;
}

CDA_ComputationTarget*
CodeGenerationState::GetTargetOfDegree(CDA_ComputationTarget* aBase,
                                       uint32_t aDegree)
{
  CDA_ComputationTarget* t = aBase;

  while (true)
  {
    if (aDegree-- == 0)
      return t;

    if (t->mUpDegree == NULL)
    {
      aBase->mHighestDegree++;
      t->mUpDegree = new CDA_ComputationTarget();
      t = t->mUpDegree;
      mCodeInfo->mTargets.push_back(t);
      t->mVariable = aBase->mVariable;
      t->mAnnoSet = aBase->mAnnoSet;
      t->mDegree = aBase->mHighestDegree;
      t->mUpDegree = NULL;
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
    basect->mUpDegree = NULL;
    basect->mHighestDegree = 0;

    basect->add_ref();
    mCodeInfo->mTargets.push_back(basect);
    mBaseTargets.push_back(basect);
    mTargetsBySource.insert
      (std::pair<iface::cellml_api::CellMLVariable*,CDA_ComputationTarget*>
       (sv, basect));
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
    wchar_t* typen;

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
CodeGenerationState::CreateVariableEdges()
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
        
        // If it is MathML, it had better be an apply...
        DECLARE_QUERY_INTERFACE_OBJREF(mae, n, mathml_dom::MathMLApplyElement);
        if (mae == NULL)
          ContextError(L"Unexpected MathML element; was expecting an apply",
                       mn, c);
        
        RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                           mae->_cxx_operator());
        RETURN_INTO_WSTRING(opn, op->localName());
        if (opn != L"eq")
          ContextError(L"Unexpected MathML element; was expecting an eq",
                       op, c);

        RETURN_INTO_OBJREF(mr, iface::cellml_services::MaLaESResult,
                           mTransform->transform(mCeVAS, mCUSES, mAnnoSet, mae, c,
                                                 NULL, NULL, 0));

        RETURN_INTO_WSTRING(compErr, mr->compileErrors());
        if (compErr != L"")
          ContextError(compErr.c_str(), op, c);

        RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                           mr->iterateInvolvedVariablesByDegree());
        
        VariableEdge* ve = new VariableEdge();
        mVariableEdges.push_back(ve);
        
        ve->mContext = c;
        ve->mMaths = mae;

        if (mae->nArguments() != 3)
          ContextError(L"Only two-way equalities are supported (a=b not a=b=...)",
                       mae, c);
        
        ve->mLHS = already_AddRefd<iface::mathml_dom::MathMLElement>
          (mae->getArgument(2));
        ve->mRHS = already_AddRefd<iface::mathml_dom::MathMLElement>
          (mae->getArgument(3));
        
        while (true)
        {
          RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable,
                             dvi->nextDegreeVariable());
          if (dv == NULL)
            break;
          
          RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                             dv->variable());
          
          std::map<iface::cellml_api::CellMLVariable*, CDA_ComputationTarget*>
            ::iterator  mi = mTargetsBySource.find(cv);
          if (mi != mTargetsBySource.end())
          {
            ve->mTargets.push_back(GetTargetOfDegree((*mi).second, dv->degree()));
          }
        }

        RETURN_INTO_OBJREF(bvi, iface::cellml_api::CellMLVariableIterator,
                           mr->iterateBoundVariables());

        while (true)
        {
          RETURN_INTO_OBJREF(bv, iface::cellml_api::CellMLVariable,
                             bvi->nextVariable());
          if (bv == NULL)
            break;

          std::map<iface::cellml_api::CellMLVariable*, CDA_ComputationTarget*>
            ::iterator  mi = mTargetsBySource.find(bv);
          if (mi != mTargetsBySource.end())
          {
            if (mBoundTargs.count((*mi).second) == 0)
              mBoundTargs.insert((*mi).second);
          }
        }
      }
    }
  }
}


void
CodeGenerationState::FirstPassTargetClassification()
{
  std::list<CDA_ComputationTarget*>::iterator i;
  for (i =  mBaseTargets.begin();
       i != mBaseTargets.end();
       i++)
  {
    // Scoped locale change.
    CNumericLocale locobj;

    CDA_ComputationTarget* ct = *i;

    RETURN_INTO_WSTRING(iv, ct->mVariable->initialValue());
    bool hasImmedIV = false;
    if (iv != L"")
    {
      wchar_t* end;
      wcstod(iv.c_str(), &end);
      if (end == NULL || *end != 0)
      {
        VariableEdge* ve = new VariableEdge();
        mVariableEdges.push_back(ve);
        ve->mMaths = NULL;
        ve->mTargets.push_back(ct);
        RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement,
                           ct->mVariable->parentElement());
        DECLARE_QUERY_INTERFACE_OBJREF(comp, el, cellml_api::CellMLComponent);
        ve->mContext = comp;
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

        std::map<iface::cellml_api::CellMLVariable*, CDA_ComputationTarget*>::
          iterator j = mTargetsBySource.find(sv);
        if (j == mTargetsBySource.end())
          ContextError(L"Invalid initial_value attribute", NULL, var);

        ve->mTargets.push_back((*j).second);
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
    else
    {
      if (hasImmedIV)
        ct->mEvaluationType = iface::cellml_services::CONSTANT;
      else
        ct->mEvaluationType = iface::cellml_services::FLOATING;

      CDA_ComputationTarget* tct = ct;

      std::wstring cname;
      while (tct->mUpDegree)
      {
        tct->mEvaluationType = iface::cellml_services::STATE_VARIABLE;
        AllocateStateVariable(tct, cname);
        if (tct == ct && hasImmedIV)
        {
          AppendAssign(mCodeInfo->mInitConstsStr, cname, iv);
        }
        tct = tct->mUpDegree;
        tct->mEvaluationType = iface::cellml_services::FLOATING;
      }
      if (ct->mUpDegree)
      {
        uint32_t mrvi = mNextStateVariableIndex - 1;
        AllocateVariable(tct, cname, mRateNamePattern, mrvi);
      }
      else if (ct->mEvaluationType == iface::cellml_services::CONSTANT)
      {
        AllocateConstant(ct, cname);
        AppendAssign(mCodeInfo->mInitConstsStr, cname, iv);
      }
    }
  }
}

void
CodeGenerationState::GenerateVariableName
(
 CDA_ComputationTarget* aCT,
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
  swprintf(buf, 30, L"%lu", index);
  aStr.append(buf);
  aStr.append(aPattern.substr(cursor + 1));
}

void
CodeGenerationState::AllocateVariable(CDA_ComputationTarget* aCT,
                                      std::wstring& aStr,
                                      std::wstring& aPattern,
                                      uint32_t& aNextIndex)
{
  uint32_t index = aNextIndex++;
  GenerateVariableName(aCT, aStr, aPattern, index);
  aCT->setNameAndIndex(index, aStr.c_str());
}

void
CodeGenerationState::AllocateVariablesInSet
(
 std::set<CDA_ComputationTarget*>& aCT,
 iface::cellml_services::VariableEvaluationType aET,
 std::wstring& aPattern,
 uint32_t& aNextIndex,
 uint32_t& aCountVar
)
{
  std::wstring str;
  
  std::set<CDA_ComputationTarget*>::iterator i;
  for (i = aCT.begin(); i != aCT.end(); i++)
  {
    (*i)->mEvaluationType = aET;

    // Don't rename it if it is already assigned...
    RETURN_INTO_WSTRING(n, (*i)->name());
    if (n == L"")
    {
      AllocateVariable(*i, str, aPattern, aNextIndex);
      aCountVar++;
    }
  }
}

void
CodeGenerationState::AllocateConstant(CDA_ComputationTarget* aCT,
                                      std::wstring& aStr)
{
  mCodeInfo->mConstantIndexCount++;
  AllocateVariable(aCT, aStr, mConstantPattern, mNextConstantIndex);
}

void
CodeGenerationState::AllocateStateVariable(CDA_ComputationTarget* aCT,
                                           std::wstring& aStr)
{
  mCodeInfo->mRateIndexCount++;
  AllocateVariable(aCT, aStr, mStateVariableNamePattern, mNextStateVariableIndex);
}

void
CodeGenerationState::AllocateAlgebraicVariable(CDA_ComputationTarget* aCT,
                                               std::wstring& aStr)
{
  mCodeInfo->mAlgebraicIndexCount++;
  AllocateVariable(aCT, aStr, mAlgebraicVariableNamePattern, mNextAlgebraicVariableIndex);
}

void
CodeGenerationState::AllocateVOI(CDA_ComputationTarget* aCT,
                                 std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mVOIPattern, mNextVOI);
}

void
CodeGenerationState::AppendAssign
(
 std::wstring& aAppendTo,
 std::wstring& aLHS,
 std::wstring& aRHS
)
{
  size_t idx1 = mAssignPattern.find(L"<LHS>");
  size_t idx2 = mAssignPattern.find(L"<RHS>");
  if (idx1 == std::wstring::npos && idx2 == std::wstring::npos)
  {
    aAppendTo.append(mAssignPattern);
    return;
  }

  if (idx1 == std::wstring::npos)
  {
    aAppendTo.append(mAssignPattern.substr(0, idx2));
    aAppendTo.append(aRHS);
    aAppendTo.append(mAssignPattern.substr(idx2 + 5));
    return;
  }

  if (idx2 == std::wstring::npos)
  {
    aAppendTo.append(mAssignPattern.substr(0, idx1));
    aAppendTo.append(aLHS);
    aAppendTo.append(mAssignPattern.substr(idx1 + 5));
    return;
  }

  if (idx1 < idx2)
  {
    aAppendTo.append(mAssignPattern.substr(0, idx1));
    aAppendTo.append(aLHS);
    aAppendTo.append(mAssignPattern.substr(idx1 + 5, idx2 - idx1 - 5));
    aAppendTo.append(aRHS);
    aAppendTo.append(mAssignPattern.substr(idx2 + 5));
    return;
  }

  aAppendTo.append(mAssignPattern.substr(0, idx2));
  aAppendTo.append(aRHS);
  aAppendTo.append(mAssignPattern.substr(idx2 + 5, idx1 - idx2 - 5));
  aAppendTo.append(aLHS);
  aAppendTo.append(mAssignPattern.substr(idx1 + 5));
}

void
CodeGenerationState::BuildFloatingAndConstantLists()
{
  std::list<CDA_ComputationTarget*>::iterator i = mCodeInfo->mTargets.begin();
  mFloating.clear();
  mKnown.clear();
  for (; i != mCodeInfo->mTargets.end(); i++)
    switch ((*i)->mEvaluationType)
    {
    case iface::cellml_services::CONSTANT:
      mKnown.push_back(*i);
      break;
    case iface::cellml_services::FLOATING:
      mFloating.push_back(*i);
      break;
    default:
      ;
    }
}

void
CodeGenerationState::BuildStateAndConstantLists()
{
  std::list<CDA_ComputationTarget*>::iterator i = mCodeInfo->mTargets.begin();
  mFloating.clear();
  mKnown.clear();
  for (; i != mCodeInfo->mTargets.end(); i++)
    switch ((*i)->mEvaluationType)
    {
    case iface::cellml_services::STATE_VARIABLE:
      mFloating.push_back(*i);
      break;
    case iface::cellml_services::CONSTANT:
      mKnown.push_back(*i);
      break;
    default:
      break;
    }
}

void
CodeGenerationState::BuildFloatingAndKnownLists()
{
  std::list<CDA_ComputationTarget*>::iterator i = mCodeInfo->mTargets.begin();
  mFloating.clear();
  mKnown.clear();
  for (; i != mCodeInfo->mTargets.end(); i++)
    switch ((*i)->mEvaluationType)
    {
    case iface::cellml_services::FLOATING:
      mFloating.push_back(*i);
      break;
    default:
      mKnown.push_back(*i);
      break;
    }
}

uint32_t
CodeGenerationState::BuildTargetSet
(
 std::list<CDA_ComputationTarget*>& aStart,
 std::list<CDA_ComputationTarget*>& aCandidates,
 std::set<CDA_ComputationTarget*>& aTargetSet
)
{
  bool didWork = false;

  std::set<CDA_ComputationTarget*> start(aStart.begin(), aStart.end());
  std::set<CDA_ComputationTarget*> candidates(aCandidates.begin(), aCandidates.end());

  do
  {
    didWork = false;

    // Go through all unused edges looking for a suitable candidate...
    std::list<VariableEdge*>::iterator i, j;
    for (i = mUnusedEdges.begin(); i != mUnusedEdges.end();)
    {
      j = i;
      i++;

      bool usedEdge(ConsiderEdgeInTargetSet(*j, start, candidates,
                                            aTargetSet));
      if (usedEdge)
      {
        didWork = true;
        mUnusedEdges.erase(j);
      }
    }
  }
  while (didWork);

  return candidates.size();
}

bool
CodeGenerationState::ConsiderEdgeInTargetSet
(
 VariableEdge* aVarEdge,
 std::set<CDA_ComputationTarget*>& aStart,
 std::set<CDA_ComputationTarget*>& aCandidates,
 std::set<CDA_ComputationTarget*>& aTargetSet
)
{
  std::list<CDA_ComputationTarget*>::iterator i = aVarEdge->mTargets.begin();
  CDA_ComputationTarget* unknown;

  int unknownCount = 0;
  int idx = 0;

  for (; i != aVarEdge->mTargets.end(); i++)
  {
    // Ignore edges that involve completely unknown targets...
    if (aStart.count(*i) == 0 &&
        aCandidates.count(*i) == 0)
    {
      return false;
    }

    // Can't use initial_value="foo" if we don't know foo...
    if (aVarEdge->mMaths == NULL && idx++ != 0 &&
        aStart.count(*i) == 0)
    {
      return false;
    }

    if (aStart.count(*i) == 0)
    {
      // Either mMaths is non-null (i.e. it is a normal MathML equation, not
      // an initial value assignment) or this is the LHS of the initial value
      // assignment.
      unknown = *i;
      unknownCount++;

      if (unknownCount > 1)
      {
        return false;
      }

      continue;
    }
  }

  if (unknownCount == 0)
    throw OverconstrainedError(aVarEdge->mMaths);

  aVarEdge->mComputedTarget = unknown;

  aCandidates.erase(unknown);
  aStart.insert(unknown);
  aTargetSet.insert(unknown);

  mEdgesInto.insert(std::pair<CDA_ComputationTarget*, VariableEdge*>
                    (unknown, aVarEdge));

  return true;
}

#include <assert.h>

void
CodeGenerationState::GenerateCodeForSet
(
 std::wstring& aCodeTo,
 std::set<CDA_ComputationTarget*>& aTargets
)
{
  while (!aTargets.empty())
  {
    CDA_ComputationTarget* t = *(aTargets.begin());
    aTargets.erase(aTargets.begin());

    std::map<CDA_ComputationTarget*, VariableEdge*>::iterator it =
      mEdgesInto.find(t);
    assert(it != mEdgesInto.end());
    VariableEdge* ve = (*it).second;

    std::list<CDA_ComputationTarget*>::iterator i;
    for (i = ve->mTargets.begin(); i != ve->mTargets.end(); i++)
      if (aTargets.count(*i) != 0)
        GenerateCodeForSet(aCodeTo, aTargets);

    GenerateCodeForEdge(aCodeTo, ve);
  }
}

/*
 * Note: This function takes a set of variables which be either rates or
 * algebraic variables. It generates code in the correct place for both.
 */
void
CodeGenerationState::GenerateCodeForSetByType(std::set<CDA_ComputationTarget*>& aTargets)
{
  std::set<CDA_ComputationTarget*> finalTargets;
  std::set<CDA_ComputationTarget*>::iterator ati, fti;
  for (ati = aTargets.begin(); ati != aTargets.end(); ati++)
  {
    if ((*ati)->mDegree != 0)
      finalTargets.insert(*ati);
  }

  for (fti = finalTargets.begin(); fti != finalTargets.end(); fti++)
  {
    CDA_ComputationTarget* t = *fti;

    ati = aTargets.find(t);
    if (ati == aTargets.end())
      continue;

    aTargets.erase(ati);

    VariableEdge* ve = mEdgesInto[t];

    std::list<CDA_ComputationTarget*>::iterator i;
    for (i = ve->mTargets.begin(); i != ve->mTargets.end(); i++)
      if (aTargets.count(*i) != 0)
        GenerateCodeForSet(mCodeInfo->mRatesStr, aTargets);

    GenerateCodeForEdge(mCodeInfo->mRatesStr, ve);
  }

  GenerateCodeForSet(mCodeInfo->mVarsStr, aTargets);
}

void
CodeGenerationState::GenerateCodeForEdge
(
 std::wstring& aCodeTo,
 VariableEdge* aVE
)
{
  if (aVE->mMaths == NULL)
  {
    // No maths, it is a simple LHS = RHS initial_value assignment.
    std::list<CDA_ComputationTarget*>::iterator i = aVE->mTargets.begin();
    CDA_ComputationTarget* t1 = *i;
    i++;
    CDA_ComputationTarget* t2 = *i;

    RETURN_INTO_OBJREF(localVarLHS, iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(aVE->mContext, t1->mVariable));
    RETURN_INTO_OBJREF(localVarRHS, iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(aVE->mContext, t2->mVariable));

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
                                             aVE->mContext, localVarLHS,
                                             NULL, 0));
    GenerateAssignmentMaLaESResult(aCodeTo, t1, mr);
    return;
  }

  // If we get here, we do have an equation. However, we are not yet sure
  // if we need to do a non-linear solve to evaluate it.

  bool swapOk = false;

  if (aVE->mComputedTarget->mDegree == 0)
  {
    // It isn't a derivative we are after, so see if one side or the other is a
    // CI...
    DECLARE_QUERY_INTERFACE_OBJREF(rhsci, aVE->mRHS, mathml_dom::MathMLCiElement);
    DECLARE_QUERY_INTERFACE_OBJREF(lhsci, aVE->mLHS, mathml_dom::MathMLCiElement);
    if (lhsci == NULL && rhsci == NULL)
    {
      GenerateSolveCode(aCodeTo, aVE);
      return;
    }

    if (lhsci == NULL)
    {
      aVE->mRHS = aVE->mLHS;
      aVE->mLHS = rhsci;
    }

    if (lhsci != NULL && rhsci != NULL)
      swapOk = true;
  }
  else
  {
    // We want to evaluate a derivative, so see if there is a derivative by
    // itself on either side of the equation...
    DECLARE_QUERY_INTERFACE_OBJREF(rhsapply, aVE->mRHS,
                                   mathml_dom::MathMLApplyElement);
    DECLARE_QUERY_INTERFACE_OBJREF(lhsapply, aVE->mLHS,
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
      GenerateSolveCode(aCodeTo, aVE);
      return;
    }

    if (!lhsIsDiff)
    {
      aVE->mRHS = aVE->mLHS;
      aVE->mLHS = rhsapply;
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
                       (mCeVAS, mCUSES, mAnnoSet, aVE->mLHS, aVE->mContext,
                        NULL, NULL, 0));
    RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                       mr->iterateInvolvedVariablesByDegree());
    RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable,
                       dvi->nextDegreeVariable());
    if (dv == NULL)
      ContextError(L"Couldn't find variable in ci",
                   NULL, aVE->mContext);

    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, dv->variable());
    if (dv->degree() != aVE->mComputedTarget->mDegree ||
        CDA_objcmp(cv, aVE->mComputedTarget->mVariable))
    {
      if (!swapOk)
        break;

      // It didn't work out that way around, but swapping is allowed...
      swapOk = false;
      ObjRef<iface::mathml_dom::MathMLElement> tmp(aVE->mRHS);
      aVE->mRHS = aVE->mLHS;
      aVE->mLHS = tmp;
      continue;
    }

    // The LHS has the variable we want by itself. However, the RHS might also
    // have the variable (e.g. x = x^2), so we start off as if it doesn't but
    // check that assumption and bail to the NR solver if it doesn't hold.

    RETURN_INTO_OBJREF(
                       localVar,
                       iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(
                                              aVE->mContext,
                                              aVE->mComputedTarget->mVariable
                                             )
                      );

    ObjRef<iface::cellml_api::CellMLVariable> localBound;

    if (mBoundTargs.begin() != mBoundTargs.end())
    {
      localBound = already_AddRefd<iface::cellml_api::CellMLVariable>
        (
         GetVariableInComponent(
                                aVE->mContext,
                                (*mBoundTargs.begin())->mVariable
                               )
        );
    }

    mr = already_AddRefd<iface::cellml_services::MaLaESResult>
      (
       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aVE->mRHS,
                             aVE->mContext, localVar, localBound,
                             aVE->mComputedTarget->mDegree)
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
      if (dv->degree() != aVE->mComputedTarget->mDegree)
        continue;
      RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                         dv->variable());
      if (CDA_objcmp(cv, aVE->mComputedTarget->mVariable))
        continue;

      match = true;
      break;
    }
    if (match)
      break;

    // We have the variable we want by itself on one side of the equation. A
    // straight assignment will suffice.
    GenerateAssignmentMaLaESResult(aCodeTo, aVE->mComputedTarget, mr);
    return;
  }
  while (true);

  GenerateSolveCode(aCodeTo, aVE);
}

void
CodeGenerationState::GenerateStateToRateCascades()
{
  std::list<CDA_ComputationTarget*>::iterator i;
  for (i = mBaseTargets.begin(); i != mBaseTargets.end(); i++)
  {
    CDA_ComputationTarget* ct = (*i)->mUpDegree;
    if (ct == NULL)
      continue;

    while (ct->mUpDegree != NULL)
    {
      RETURN_INTO_WSTRING(stateN, ct->name());
      std::wstring rateN;
      GenerateVariableName(ct, rateN, mRateNamePattern,
                           ct->mAssignedIndex - 1);

      AppendAssign(mCodeInfo->mRatesStr, rateN, stateN);
      ct = ct->mUpDegree;
    }
  }
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
 CDA_ComputationTarget* aTarget,
 iface::cellml_services::MaLaESResult* aMR
)
{
  RETURN_INTO_WSTRING(lhs, aTarget->name());
  RETURN_INTO_WSTRING(rhs, aMR->expression());

  uint32_t l = aMR->supplementariesLength(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_WSTRING(s, aMR->getSupplementary(i));
    mCodeInfo->mFuncsStr += s;
  }

  AppendAssign(aCodeTo, lhs, rhs);
}

void
CodeGenerationState::GenerateSolveCode
(
 std::wstring& aCodeTo,
 VariableEdge* aVE
)
{
  RETURN_INTO_OBJREF(mr1, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aVE->mLHS,
                                           aVE->mContext, NULL, NULL, 0));
  RETURN_INTO_OBJREF(mr2, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, aVE->mRHS,
                                           aVE->mContext, NULL, NULL, 0));

  aVE->mMaths->add_ref();
  mCodeInfo->mFlaggedEquations.push_back(aVE->mMaths);

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
  swprintf(id, 20, L"%u", mNextSolveId++);

  RETURN_INTO_WSTRING(vname, aVE->mComputedTarget->name());

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
    }
  }
}
