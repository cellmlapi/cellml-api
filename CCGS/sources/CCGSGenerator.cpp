#include "CCGSImplementation.hpp"
#include <map>
#include <set>
#include <math.h>
#include "CodeGenerationState.hxx"
#include <stdio.h>
#include "CodeGenerationError.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "DOMBootstrap.hxx"
#include <assert.h>
#include <algorithm>

// Win32 hack...
#ifdef _WIN32
#define swprintf _snwprintf
#endif

// Asymptotic complexity is bound around O(n^(((t^2)+t)/2)) for t=SEARCH_DEPTH. This is a pretty poor upper bound.
#define SEARCH_DEPTH 3

CodeGenerationState::~CodeGenerationState()
{
  for (std::list<Equation*>::iterator i = mEquations.begin();
       i != mEquations.end(); i++)
    delete *i;

  for (std::list<System*>::iterator i = mSystems.begin();
       i != mSystems.end(); i++)
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
    CreateEquations();

    // Next, set starting classification for all targets...
    FirstPassTargetClassification();

    mUnusedEquations.insert(mEquations.begin(), mEquations.end());

    // Put all targets into lists based on their classification...
    BuildFloatingAndConstantLists();

    std::list<System*> systems;

    // Now, determine all constants computable from the current constants...
    DecomposeIntoSystems(mKnown, mFloating, systems);

    // Assign constant variables for set...
    AllocateVariablesInSet(systems, iface::cellml_services::CONSTANT,
                           mConstantPattern, mNextConstantIndex,
                           mCodeInfo->mConstantIndexCount);

    std::map<CDA_ComputationTarget*, System*> sysByTargReq;
    // Build an index from variables required to systems...
    BuildSystemsByTargetsRequired(systems, sysByTargReq);

    // Write evaluations for all constants we just worked out how to compute...
    GenerateCodeForSet(mCodeInfo->mInitConstsStr, mKnown, systems, sysByTargReq);

    // Also we need to initialise state variable IVs...
    systems.clear();
    BuildStateAndConstantLists();
    DecomposeIntoSystems(mKnown, mFloating, systems);
    BuildSystemsByTargetsRequired(systems, sysByTargReq);
    GenerateCodeForSet(mCodeInfo->mInitConstsStr, mKnown, systems, sysByTargReq);

    // Put all targets into lists based on their classification...
    BuildFloatingAndKnownLists();

    // Now, determine all algebraic variables / rates computable from the
    // known variables (constants, state variables, and bound variable)...
    systems.clear();

    bool wasError = DecomposeIntoSystems(mKnown, mFloating, systems);
    BuildSystemsByTargetsRequired(systems, sysByTargReq);

    // Assign algebraic variables for set...
    AllocateVariablesInSet(systems, iface::cellml_services::ALGEBRAIC,
                           mAlgebraicVariableNamePattern,
                           mNextAlgebraicVariableIndex,
                           mCodeInfo->mAlgebraicIndexCount);

    if (wasError)
    {
      if (mUnusedEquations.size() != 0)
      {
        if (mFloating.size() != 0)
          throw UnsuitablyConstrainedError();
        else
          throw OverconstrainedError
            ((*(mUnusedEquations.begin()))->mMaths);
      }
      else
        throw UnderconstrainedError();
    }

    // Write evaluations for all rates & algebraic variables in reachabletargets
    GenerateCodeForSetByType(mKnown, systems, sysByTargReq);

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

    std::set<Equation*>::iterator uel;
    for (uel = mUnusedEquations.begin(); uel != mUnusedEquations.end(); uel++)
    {
      iface::mathml_dom::MathMLApplyElement* mae = (*uel)->mMaths;
      // If there was an initial value that was set to a variable which could not
      // be computed, it will be an unused edge with null mMaths.
      if (mae != NULL)
      {
        mae->add_ref();
        mCodeInfo->mFlaggedEquations.push_back(mae);
      }
    }

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
CodeGenerationState::CreateEquations()
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
        
        Equation* eq = new Equation();
        mEquations.push_back(eq);
        
        eq->mContext = c;
        eq->mMaths = mae;

        if (mae->nArguments() != 3)
          ContextError(L"Only two-way equalities are supported (a=b not a=b=...)",
                       mae, c);
        
        eq->mLHS = already_AddRefd<iface::mathml_dom::MathMLElement>
          (mae->getArgument(2));
        eq->mRHS = already_AddRefd<iface::mathml_dom::MathMLElement>
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
            eq->mTargets.push_back(GetTargetOfDegree((*mi).second, dv->degree()));
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
        Equation* eq = new Equation();
        mEquations.push_back(eq);
        eq->mMaths = NULL;
        eq->mTargets.push_back(ct);
        RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement,
                           ct->mVariable->parentElement());
        DECLARE_QUERY_INTERFACE_OBJREF(comp, el, cellml_api::CellMLComponent);
        eq->mContext = comp;
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

        eq->mTargets.push_back((*j).second);
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
        uint32_t count = 0;
        AllocateVariable(tct, cname, mRateNamePattern, mrvi, count);
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
                                      uint32_t& aNextIndex,
                                      uint32_t& aCount)
{
  RETURN_INTO_WSTRING(n, aCT->name());
  if (n == L"")
  {
    uint32_t index = aNextIndex++;
    aCount++;
    GenerateVariableName(aCT, aStr, aPattern, index);
    aCT->setNameAndIndex(index, aStr.c_str());
  }
  else
    aStr = n;
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
  std::set<CDA_ComputationTarget*>::iterator j;

  for (i = aCT.begin(); i != aCT.end(); i++)
  {
    for (j = (*i)->mUnknowns.begin(); j != (*i)->mUnknowns.end(); j++)
    {
      (*j)->mEvaluationType = aET;

      // Don't rename it if it is already assigned...
      RETURN_INTO_WSTRING(n, (*j)->name());
      if (n == L"")
      {
        AllocateVariable(*j, str, aPattern, aNextIndex, aCountVar);
      }
    }
  }
}

void
CodeGenerationState::AllocateConstant(CDA_ComputationTarget* aCT,
                                      std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mConstantPattern, mNextConstantIndex,
                   mCodeInfo->mConstantIndexCount);
}

void
CodeGenerationState::AllocateStateVariable(CDA_ComputationTarget* aCT,
                                           std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mStateVariableNamePattern, mNextStateVariableIndex,
                   mCodeInfo->mRateIndexCount);
}

void
CodeGenerationState::AllocateAlgebraicVariable(CDA_ComputationTarget* aCT,
                                               std::wstring& aStr)
{
  AllocateVariable(aCT, aStr, mAlgebraicVariableNamePattern, mNextAlgebraicVariableIndex,
                   mCodeInfo->mAlgebraicIndexCount);
}

void
CodeGenerationState::AllocateVOI(CDA_ComputationTarget* aCT,
                                 std::wstring& aStr)
{
  uint32_t count = 0;
  AllocateVariable(aCT, aStr, mVOIPattern, mNextVOI, count);
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
      mKnown.insert(*i);
      break;
    case iface::cellml_services::FLOATING:
      mFloating.insert(*i);
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
      mFloating.insert(*i);
      break;
    case iface::cellml_services::CONSTANT:
      mKnown.insert(*i);
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
      mFloating.insert(*i);
      break;
    default:
      mKnown.insert(*i);
      break;
    }
}

bool
CodeGenerationState::DecomposeIntoSystems
(
 std::set<CDA_ComputationTarget*>& aStart,
 std::set<CDA_ComputationTarget*>& aCandidates,
 std::list<System*>& aSystems
)
{
  std::set<CDA_ComputationTarget*> start(aStart);

  while (true)
  {
    // The first step is to cluster all candidate variables into disjoint sets,
    // where two variables are in the same set if there is an until now unused
    // equation involving both of them.
    for (
         std::set<CDA_ComputationTarget*>::iterator i = aCandidates.begin();
         i != aCandidates.end();
         i++
        )
      (*i)->resetSetMembership();

    for (std::set<Equation*>::iterator i = mUnusedEquations.begin();
         i != mUnusedEquations.end(); i++)
    {
      std::list<CDA_ComputationTarget*>::iterator j = (*i)->mTargets.begin(), f;

      CDA_ComputationTarget* linkWith = NULL;
      // See if we should ignore this equation.
      bool ignoreEquation(false);
      for (; j != (*i)->mTargets.end(); j++)
      {
        if (start.count(*j) > 0)
          continue;
        if (aCandidates.count(*j) == 0)
        {
          ignoreEquation = true;
          break;
        }
        if (linkWith == NULL)
        {
          linkWith = *j;
          f = j;
        }
      }
      if (ignoreEquation)
        continue;
      if (linkWith == NULL)
        // This means that the model is overconstrained.
        throw OverconstrainedError((*i)->mMaths);

      for (j = f, j++; j != (*i)->mTargets.end(); j++)
      {
        if (start.count(*j) > 0)
          continue;

        (*j)->unionWith(linkWith);
        assert(linkWith->findRoot() == (*j)->findRoot());
      }
    }
    
    typedef std::pair<std::set<Equation*>, std::set<CDA_ComputationTarget*> >
      EquationCTSetPair ;
    typedef std::pair<CDA_ComputationTarget*, EquationCTSetPair>
      EquationCTSetPairByCT;
    typedef std::map<CDA_ComputationTarget*, EquationCTSetPair>
      MapEquationCTSetPairByCT;

    MapEquationCTSetPairByCT targets;

    for (
         std::set<CDA_ComputationTarget*>::iterator j = aCandidates.begin();
         j != aCandidates.end();
         j++
        )
    {
      CDA_ComputationTarget* root = (*j)->findRoot();

      MapEquationCTSetPairByCT::iterator cti(targets.find(root));

      if (cti == targets.end())
      {
        std::set<CDA_ComputationTarget*> s;
        s.insert(*j);
        targets.insert(EquationCTSetPairByCT(root, EquationCTSetPair
                                             (std::set<Equation*>(), s)));
      }
      else
      {
        (*cti).second.second.insert(*j);
      }
    }

    // Now, we need to go through all the unused equations and stick them in a
    // set with the appropriate set of computation targets...
    for (
         std::set<Equation*>::iterator i = mUnusedEquations.begin();
         i != mUnusedEquations.end();
         i++
        )
    {
      // See if we should ignore this equation.
      bool ignoreEquation(false);
      CDA_ComputationTarget* first = NULL;
      for (
           std::list<CDA_ComputationTarget*>::iterator j((*i)->mTargets.begin());
           j != (*i)->mTargets.end();
           j++
          )
      {
        if (start.count(*j) > 0)
          continue;
        if (aCandidates.count(*j) == 0)
        {
          ignoreEquation = true;
          break;
        }

        first = *j;
      }
      if (ignoreEquation)
        continue;

      // First is guaranteed not to be null because of the overconstrained
      // checks above, and is guaranteed to belong to exactly one of the
      // disjoint sets of variables. Find which one...

      MapEquationCTSetPairByCT::iterator j(targets.find(first->findRoot()));
      (*j).second.first.insert(*i);
    }

    bool progress = false;

    // We now have a set of disjoint sets of variables (i.e. there are no
    // equations which have not yet been used linking variables in different
    // disjoint sets), and the equations linking them.
    for (MapEquationCTSetPairByCT::iterator i(targets.begin());
         i != targets.end();
         i++)
    {
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
      return (aCandidates.size() != 0) || (mUnusedEquations.size() != 0);

    // Having found one system, we may have further fragmented the systems by
    // removing the linking variables. The next iteration will detect this and
    // then we can try again.
  }
}

bool
CodeGenerationState::FindSmallSystem
(
 std::set<Equation*>& aUseEquations,
 std::set<CDA_ComputationTarget*>& aUseVars,
 std::set<CDA_ComputationTarget*>& aStart,
 std::set<CDA_ComputationTarget*>& aCandidates,
 std::list<System*>& aSystems
)
{

  for (uint32_t systemCardinality = 1;
       systemCardinality <= SEARCH_DEPTH;
       systemCardinality++)
  {
    std::set<Equation*> s;
    std::set<Equation*>::iterator i(aUseEquations.begin());
    if (RecursivelyTestSmallSystem(s, i, systemCardinality,
                                   aUseEquations, aUseVars, aStart, aCandidates,
                                   aSystems
                                  ))
      return true;
  }

  return false;
}

bool
CodeGenerationState::RecursivelyTestSmallSystem
(
 std::set<Equation*>& aSystem,
 std::set<Equation*>::iterator& aEqIt,
 uint32_t aNeedToAdd,
 std::set<Equation*>& aUseEquations,
 std::set<CDA_ComputationTarget*>& aUseVars,
 std::set<CDA_ComputationTarget*>& aStart,
 std::set<CDA_ComputationTarget*>& aCandidates,
 std::list<System*>& aSystems
)
{
  // Pre: aNeedToAdd is >= 1...
  aNeedToAdd--;

  for (std::set<Equation*>::iterator i(aEqIt); i != aUseEquations.end();)
  {
    // We do this insert and erase thing rather than copy the set to save stack
    // space.
    std::set<Equation*>::iterator sysEq(aSystem.insert(*i).first);
    i++;

    if (aNeedToAdd > 0)
    {
      if (RecursivelyTestSmallSystem(aSystem, i, aNeedToAdd, aUseEquations,
                                     aUseVars, aStart, aCandidates, aSystems))
        return true;
    }
    else
    {
      // Well, we now have a set of equations which we are going to consider as
      // a possible system in aSystem. Work out the variables involved...
      std::set<CDA_ComputationTarget*> targets;
      std::set<CDA_ComputationTarget*> known;

      for (std::set<Equation*>::iterator j(aSystem.begin());
           j != aSystem.end(); j++)
        for (std::list<CDA_ComputationTarget*>::iterator k
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
            targets.insert(*k);
          }
        }
      
      uint32_t nEqns = aSystem.size(), nUnknowns = targets.size();

      if (nEqns > nUnknowns)
        throw OverconstrainedError((*(aUseEquations.begin()))->mMaths);

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

      for (std::set<CDA_ComputationTarget*>::iterator k(targets.begin());
           k != targets.end();
           k++)
        aCandidates.erase(*k);

      for (std::set<Equation*>::iterator k(aSystem.begin());
           k != aSystem.end(); k++)
        mUnusedEquations.erase(*k);

      return true;
    }

    aSystem.erase(sysEq);
  }

  return false;
}

bool
CodeGenerationState::FindBigSystem
(
 std::set<Equation*>& aUseEquations,
 std::set<CDA_ComputationTarget*>& aUseVars,
 std::set<CDA_ComputationTarget*>& aStart,
 std::set<CDA_ComputationTarget*>& aCandidates,
 std::list<System*>& aSystems
)
{
  // There is absolutely no point in doing this if the system is improperly
  // constrained...
  if (aUseEquations.size() != aUseVars.size())
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
      if (nonSystemCardinality >= aUseEquations.size())
        break;

      std::set<Equation*> s;
      std::set<Equation*>::iterator i(aUseEquations.begin());
      if (RecursivelyTestBigSystem(s, i, nonSystemCardinality,
                                   aUseEquations, aUseVars, aStart, aCandidates
                                  ))
      {
        didWork = true;
        break;
      }
    }
  }
  while (didWork);

  // If we get here, we have removed as many equations as we can from aUseVars
  // and aUseEquations, so that is now our system...

  std::set<CDA_ComputationTarget*> known;

  for (std::set<Equation*>::iterator i(aUseEquations.begin());
       i != aUseEquations.end();
       i++)
    for (std::list<CDA_ComputationTarget*>::iterator j((*i)->mTargets.begin());
         j != (*i)->mTargets.end();
         j++)
    {
      if (aStart.count(*j))
        known.insert(*j);
    }

  System* st = new System(aUseEquations, known, aUseVars);
  mSystems.push_back(st);
  aSystems.push_back(st);

  aStart.insert(aUseVars.begin(), aUseVars.end());
  for (std::set<CDA_ComputationTarget*>::iterator k(aUseVars.begin());
       k != aUseVars.end();
       k++)
    aCandidates.erase(*k);
  
  for (std::set<Equation*>::iterator k(aUseEquations.begin());
       k != aUseEquations.end(); k++)
    mUnusedEquations.erase(*k);

  return false;
}

bool
CodeGenerationState::RecursivelyTestBigSystem
(
 std::set<Equation*>& aNonSystem,
 std::set<Equation*>::iterator& aEqIt,
 uint32_t aNeedToRemove,
 std::set<Equation*>& aUseEquations,
 std::set<CDA_ComputationTarget*>& aUseVars,
 std::set<CDA_ComputationTarget*>& aStart,
 std::set<CDA_ComputationTarget*>& aCandidates
)
{
  // Pre: aNeedToRemove is >= 1...
  aNeedToRemove--;

  for (std::set<Equation*>::iterator i(aEqIt); i != aUseEquations.end();)
  {
    // We do this insert and erase thing rather than copy the set to save stack
    // space.
    std::set<Equation*>::iterator sysEq(aNonSystem.insert(*i).first);
    i++;

    if (aNeedToRemove > 0)
    {
      if (RecursivelyTestBigSystem(aNonSystem, i, aNeedToRemove, aUseEquations,
                                   aUseVars, aStart, aCandidates))
        return true;
    }
    else
    {
      // Build aUseEquations \ aNonSystem...
      std::set<Equation*> syst;
      std::set_difference(aUseEquations.begin(),
                          aUseEquations.end(),
                          aNonSystem.begin(),
                          aNonSystem.end(),
                          std::inserter(syst, syst.end()));

      // Which variables does this involve?
      std::set<CDA_ComputationTarget*> targs;
      for (std::set<Equation*>::iterator j(syst.begin());
           j != syst.end(); j++)
        for (std::list<CDA_ComputationTarget*>::iterator k
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

      aUseEquations = syst;
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
 std::map<CDA_ComputationTarget*, System*>& aSysByTargReq
)
{
  aSysByTargReq.clear();

  for (std::list<System*>::iterator i = aSystems.begin();
       i != aSystems.end();
       i++)
  {
    for (std::set<CDA_ComputationTarget*>::iterator j = (*i)->mUnknowns.begin();
         j != (*i)->mUnknowns.end();
         j++)
      aSysByTargReq.insert(std::pair<CDA_ComputationTarget*, System*>(*j, *i));
  }
}

void
CodeGenerationState::GenerateCodeForSet
(
 std::wstring& aCodeTo,
 std::set<CDA_ComputationTarget*>& aKnown,
 std::list<System*>& aSystems,
 std::map<CDA_ComputationTarget*, System*>&
   aSysByTargReq
)
{
  while (!aSystems.empty())
  {
    System* sys = aSystems.front();
    aSystems.pop_front();

    // See if this system is already known. Since we always compute whole
    // systems at a time, we can do this by looking at the first target of the
    // system...
    if (aKnown.count(*sys->mUnknowns.begin()))
      continue;

    // Find everything we don't know but want to...
    std::list<System*> subtargets;
    
    for (std::set<CDA_ComputationTarget*>::iterator j = sys->mKnowns.begin();
         j != sys->mKnowns.end();
         j++)
    {
      if (aKnown.count(*j))
        continue;

      std::map<CDA_ComputationTarget*, System*>::iterator sbtr
        (aSysByTargReq.find(*j));

      // This should be guaranteed if we got this far...
      assert(sbtr != aSysByTargReq.end());

      subtargets.push_back((*sbtr).second);
    }

    aKnown.insert(sys->mUnknowns.begin(), sys->mUnknowns.end());

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
 std::set<CDA_ComputationTarget*>& aKnown,
 std::list<System*>& aSystems,
 std::map<CDA_ComputationTarget*, System*>&
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

    // See if this system has a rate variable...
    bool hasRate(false);

    for (std::set<CDA_ComputationTarget*>::iterator j(sys->mUnknowns.begin());
         j != sys->mUnknowns.end();
         j++)
      if ((*j)->mDegree != 0)
      {
        hasRate = true;
        break;
      }

    if (hasRate)
      rateSys.push_back(sys);
  }

  GenerateCodeForSet(mCodeInfo->mRatesStr, aKnown, rateSys, aSysByTargReq);

  // And now everything else goes in mVarsStr
  GenerateCodeForSet(mCodeInfo->mVarsStr, aKnown, aSystems, aSysByTargReq);
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

  if (aSys->mEquations.size() > 1)
  {
    GenerateMultivariateSolveCode(aCodeTo, aSys);
    return;
  }

  Equation* eq = *(aSys->mEquations.begin());

  if (eq->mMaths == NULL)
  {
    // No maths, it is a simple LHS = RHS initial_value assignment.
    std::list<CDA_ComputationTarget*>::iterator i = eq->mTargets.begin();
    CDA_ComputationTarget* t1 = *i;
    i++;
    CDA_ComputationTarget* t2 = *i;

    RETURN_INTO_OBJREF(localVarLHS, iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(eq->mContext, t1->mVariable));
    RETURN_INTO_OBJREF(localVarRHS, iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(eq->mContext, t2->mVariable));

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
                                             eq->mContext, localVarLHS,
                                             NULL, 0));
    GenerateAssignmentMaLaESResult(aCodeTo, t1, mr);
    return;
  }

  // If we get here, we do have an equation. However, we are not yet sure
  // if we need to do a non-linear solve to evaluate it.

  bool swapOk = false;

  CDA_ComputationTarget* computedTarget = *(aSys->mUnknowns.begin());

  if (computedTarget->mDegree == 0)
  {
    // It isn't a derivative we are after, so see if one side or the other is a
    // CI...
    DECLARE_QUERY_INTERFACE_OBJREF(rhsci, eq->mRHS, mathml_dom::MathMLCiElement);
    DECLARE_QUERY_INTERFACE_OBJREF(lhsci, eq->mLHS, mathml_dom::MathMLCiElement);
    if (lhsci == NULL && rhsci == NULL)
    {
      GenerateSolveCode(aCodeTo, eq, computedTarget);
      return;
    }

    if (lhsci == NULL)
    {
      eq->mRHS = eq->mLHS;
      eq->mLHS = rhsci;
    }

    if (lhsci != NULL && rhsci != NULL)
      swapOk = true;
  }
  else
  {
    // We want to evaluate a derivative, so see if there is a derivative by
    // itself on either side of the equation...
    DECLARE_QUERY_INTERFACE_OBJREF(rhsapply, eq->mRHS,
                                   mathml_dom::MathMLApplyElement);
    DECLARE_QUERY_INTERFACE_OBJREF(lhsapply, eq->mLHS,
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
      GenerateSolveCode(aCodeTo, eq, computedTarget);
      return;
    }

    if (!lhsIsDiff)
    {
      eq->mRHS = eq->mLHS;
      eq->mLHS = rhsapply;
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
                       (mCeVAS, mCUSES, mAnnoSet, eq->mLHS, eq->mContext,
                        NULL, NULL, 0));
    RETURN_INTO_OBJREF(dvi, iface::cellml_services::DegreeVariableIterator,
                       mr->iterateInvolvedVariablesByDegree());
    RETURN_INTO_OBJREF(dv, iface::cellml_services::DegreeVariable,
                       dvi->nextDegreeVariable());
    if (dv == NULL)
      ContextError(L"Couldn't find variable in ci",
                   NULL, eq->mContext);

    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, dv->variable());
    if (dv->degree() != computedTarget->mDegree ||
        CDA_objcmp(cv, computedTarget->mVariable))
    {
      if (!swapOk)
        break;

      // It didn't work out that way around, but swapping is allowed...
      swapOk = false;
      ObjRef<iface::mathml_dom::MathMLElement> tmp(eq->mRHS);
      eq->mRHS = eq->mLHS;
      eq->mLHS = tmp;
      continue;
    }

    // The LHS has the variable we want by itself. However, the RHS might also
    // have the variable (e.g. x = x^2), so we start off as if it doesn't but
    // check that assumption and bail to the NR solver if it doesn't hold.

    RETURN_INTO_OBJREF(
                       localVar,
                       iface::cellml_api::CellMLVariable,
                       GetVariableInComponent(
                                              eq->mContext,
                                              computedTarget->mVariable
                                             )
                      );

    ObjRef<iface::cellml_api::CellMLVariable> localBound;

    if (mBoundTargs.begin() != mBoundTargs.end())
    {
      localBound = already_AddRefd<iface::cellml_api::CellMLVariable>
        (
         GetVariableInComponent(
                                eq->mContext,
                                (*mBoundTargs.begin())->mVariable
                               )
        );
    }

    mr = already_AddRefd<iface::cellml_services::MaLaESResult>
      (
       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, eq->mRHS,
                             eq->mContext, localVar, localBound,
                             computedTarget->mDegree)
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
      if (dv->degree() != computedTarget->mDegree)
        continue;
      RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                         dv->variable());
      if (CDA_objcmp(cv, computedTarget->mVariable))
        continue;

      match = true;
      break;
    }
    if (match)
      break;

    // We have the variable we want by itself on one side of the equation. A
    // straight assignment will suffice.
    GenerateAssignmentMaLaESResult(aCodeTo, computedTarget, mr);
    return;
  }
  while (true);

  GenerateSolveCode(aCodeTo, eq, computedTarget);
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
 Equation* aEq,
 CDA_ComputationTarget* aComputedTarget
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
  swprintf(id, 20, L"%u", mNextSolveId++);

  RETURN_INTO_WSTRING(vname, aComputedTarget->name());

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
  std::set<CDA_ComputationTarget*>::iterator k(aSys->mUnknowns.begin());
  for(std::set<Equation*>::iterator i(aSys->mEquations.begin());
      i != aSys->mEquations.end();
      i++, k++)
  {
    Equation* eq(*i);

    RETURN_INTO_OBJREF(mr1, iface::cellml_services::MaLaESResult,
                       mTransform->transform(mCeVAS, mCUSES, mAnnoSet, eq->mLHS,
                                             eq->mContext, NULL, NULL, 0));
    RETURN_INTO_OBJREF(mr2, iface::cellml_services::MaLaESResult,
                     mTransform->transform(mCeVAS, mCUSES, mAnnoSet, eq->mRHS,
                                           eq->mContext, NULL, NULL, 0));

    eq->mMaths->add_ref();
    mCodeInfo->mFlaggedEquations.push_back(eq->mMaths);

    RETURN_INTO_WSTRING(e1, mr1->expression());
    RETURN_INTO_WSTRING(e2, mr2->expression());

    eq->mLHSCode = e1;
    eq->mRHSCode = e2;

    uint32_t l = mr1->supplementariesLength(), j;
    for (j = 0; j < l; j++)
    {
      RETURN_INTO_WSTRING(s, mr1->getSupplementary(j));
      mCodeInfo->mFuncsStr += s;
    }

    l = mr2->supplementariesLength();
    for (j = 0; j < l; j++)
    {
      RETURN_INTO_WSTRING(s, mr2->getSupplementary(j));
      mCodeInfo->mFuncsStr += s;
    }

    RETURN_INTO_WSTRING(vname, (*k)->name());
    eq->mVarName = vname;
  }

  // Scoped locale change.
  CNumericLocale locobj;
  wchar_t id[20];
  swprintf(id, 20, L"%u", mNextSolveId++);

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
  swprintf(countStr, 15, L"%u", aSys->mEquations.size());

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
    for(std::set<Equation*>::iterator i(aSys->mEquations.begin());
        i != aSys->mEquations.end();
        i++)
    {
      wchar_t indexStr[15];
      swprintf(indexStr, 15, L"%u", index);
      index++;
      if (i != aSys->mEquations.begin())
        aCodeTo += ReplaceIDs(join, aId, indexStr, countStr);
      GenerateMultivariateSolveCodeEq(aCodeTo, *i, perEqPattern, aId, indexStr);
    }

    offset = occurrence + 12;
  }

  aCodeTo += ReplaceIDs(aPattern.substr(offset), aId, L"", countStr);
}

void
CodeGenerationState::GenerateMultivariateSolveCodeEq
(
 std::wstring& aCodeTo,
 Equation* aEq,
 const std::wstring& aPattern,
 const wchar_t* aId,
 const wchar_t* aIndex
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
      if (c == L'L')
        state = 2;
      else if (c == L'R')
        state = 3;
      else if (c == L'I')
        state = 4;
      else if (c == L'V')
        state = 5;
      else
      {
        aCodeTo += L'<';
        aCodeTo += c;
        state = 0;
      }
      break;
    case 2: // Seen <L
      if (c == L'H')
        state = 6;
      else
      {
        aCodeTo += L"<L";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 3: // Seen <R
      if (c == L'H')
        state = 8;
      else
      {
        aCodeTo += L"<R";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 4: // Seen <I
      if (c == L'D')
        state = 10;
      else if (c == L'N')
        state = 13;
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
    case 6: // Seen <LH
      if (c == L'S')
        state = 7;
      else
      {
        aCodeTo += L"<LH";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 7: // Seen <LHS
      if (c == L'>')
      {
        // Matched <LHS>
        aCodeTo += aEq->mLHSCode;
        state = 0;
      }
      else
      {
        aCodeTo += L"<LHS";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 8: // Seen <RH
      if (c == L'S')
        state = 9;
      else
      {
        aCodeTo += L"<RH";
        aCodeTo += c;
        state = 0;
      }
      break;
    case 9: // Seen <RHS
      if (c == L'>')
      {
        // Matched <RHS>
        aCodeTo += aEq->mRHSCode;
        state = 0;
      }
      else
      {
        aCodeTo += L"<RHS";
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
    }
  }
}
