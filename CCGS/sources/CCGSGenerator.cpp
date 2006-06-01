#include "CCGSImplementation.hpp"
#include <map>
#include <set>
#include <math.h>
#include "Variables.hxx"
#include "Equality.hxx"
#include "CodeGenerationState.hxx"

// Win32 hack...
#ifdef _WIN32
#define swprintf _snwprintf
#endif

#include "CodeGenerationError.hxx"
#include "TemporaryAnnotation.hxx"

static struct
{
  uint32_t badmask;
  const wchar_t* errMsg;
}  IncompatibleFlagSet[] =
{
  {
    VariableInformation::SUBJECT_OF_DIFF |
    VariableInformation::BVAR_OF_DIFF,
    L"The same variable cannot be both the subject of differentiation and the "
    L"bound variable of a differentiation."
  },
  {
    VariableInformation::OTHER_BVAR |
    VariableInformation::NONBVAR_USE,
    L"You cannot use a variable as a bound variable (other than for "
    L"differentiation) and then later as a free variable."
  }
};

void
VariableInformation::SetFlag(uint32_t flag)
  throw (CodeGenerationError&)
{
  mFlags |= flag;
  // Now check the flags for errors...
  uint32_t i;
  for (i = 0; i < sizeof(IncompatibleFlagSet)/sizeof(IncompatibleFlagSet[0]);
       i++)
    if ((mFlags & IncompatibleFlagSet[i].badmask) ==
        IncompatibleFlagSet[i].badmask)
    {
      std::wstring msg = L"Error in variable ";
      msg += mName;
      msg += L": ";
      msg += IncompatibleFlagSet[i].errMsg;
      throw CodeGenerationError(msg);
    }
}

CodeGenerationState::CodeGenerationState()
  : mLastFunctionId(0)
{
  SetupBuiltinUnits();
}

CodeGenerationState::~CodeGenerationState()
{
  std::list<Equation*>::iterator i;
  for (i = mEquations.begin(); i != mEquations.end(); i++)
    (*i)->release_ref();

  std::list<CellMLScope*>::iterator i2;
  for (i2 = mModelScopes.begin(); i2 != mModelScopes.end(); i2++)
    (*i2)->release_ref();
  for (i2 = mComponentScopes.begin(); i2 != mComponentScopes.end(); i2++)
    (*i2)->release_ref();
}

void
CodeGenerationState::CreateModelScopes(iface::cellml_api::Model* aModel)
{
  // Go through each model and create a scope...
  RETURN_INTO_OBJREF(cis, iface::cellml_api::CellMLImportSet,
                     aModel->imports());
  RETURN_INTO_OBJREF(it, iface::cellml_api::CellMLImportIterator,
                     cis->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       it->nextImport());
    if (imp == NULL)
      break;
    RETURN_INTO_OBJREF(m, iface::cellml_api::Model, imp->importedModel());
    CreateModelScopes(m);
  }
  // Now actually create the scope...
  RETURN_INTO_OBJREF(cs, CellMLScope,
                     new CellMLScope(&mGlobalScope, aModel, scopeKey));
  annot.addAnnotation(cs);
  cs->add_ref();
  mModelScopes.push_back(cs);
  
  // Next, create the component scopes for this model. We only include local
  // components here, because imported components exist in the scope of the
  // imported model.
  RETURN_INTO_OBJREF(lc, iface::cellml_api::CellMLComponentSet,
                     aModel->localComponents());
  RETURN_INTO_OBJREF(cci, iface::cellml_api::CellMLComponentIterator,
                     lc->iterateComponents());
  
  // Now add the units to the scope, so we can find them...
  RETURN_INTO_OBJREF(us, iface::cellml_api::UnitsSet,
                     aModel->modelUnits());
  RETURN_INTO_OBJREF(ui, iface::cellml_api::UnitsIterator,
                     us->iterateUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                       ui->nextUnits());
    if (u == NULL)
      break;
    PutUnitsIntoScope(cs, u);
  }
  
  while (true)
  {
    RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                       cci->nextComponent());
    if (comp == NULL)
      break;
    CreateComponentScope(cs, comp);
  }
}

void
CodeGenerationState::CreateComponentScope
(
 CellMLScope* aModelScope, iface::cellml_api::CellMLComponent* aComp
)
{
  RETURN_INTO_OBJREF(cs, CellMLScope,
                     new CellMLScope(aModelScope, aComp, scopeKey));
  annot.addAnnotation(cs);
  cs->add_ref();
  mComponentScopes.push_back(cs);
  
  // Now add the units to the scope, so we can find them...
  RETURN_INTO_OBJREF(us, iface::cellml_api::UnitsSet,
                     aComp->units());
  RETURN_INTO_OBJREF(ui, iface::cellml_api::UnitsIterator,
                     us->iterateUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                       ui->nextUnits());
    if (u == NULL)
      break;
    PutUnitsIntoScope(cs, u);
  }
}

void
CodeGenerationState::CreateComponentList(iface::cellml_api::Model* aModel)
{
  std::set<iface::cellml_api::CellMLComponent*, XPCOMComparator> compset;
  // Go through each local or directly imported component...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                     aModel->modelComponents());
  RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                     mc->iterateComponents());
  while (true)
  {
    RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                       ci->nextComponent());
    if (c == NULL)
      break;
    // Append the component to the component list...
    // Don't addref, they will outlive us anyway (otherwise we create a
    //  cycle).
    if (compset.count(c) != 0)
      continue;
    mComponentList.push_back(c);
    compset.insert(c);
    // Also, any encapsulation descendents go on the list...
    AddEncapsulationDescendentComponents(compset, c);
  }
}

void
CodeGenerationState::AddEncapsulationDescendentComponents
(
 std::set<iface::cellml_api::CellMLComponent*,XPCOMComparator>& compset,
 iface::cellml_api::CellMLComponent* aComponent
)
{
  RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                     aComponent->encapsulationChildren());
  RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                     cs->iterateComponents());
  while (true)
  {
    RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                       ci->nextComponent());
    if (c == NULL)
      break;
    
    if (compset.count(c) != 0)
      continue;

    mComponentList.push_back(c);
    compset.insert(c);
    AddEncapsulationDescendentComponents(compset, c);
  }
}

void
CodeGenerationState::ProcessMath()
{
  std::list<iface::cellml_api::CellMLComponent*>::iterator i;
  for (i = mComponentList.begin(); i != mComponentList.end(); i++)
    ProcessMathInComponent(*i);
}

void
CodeGenerationState::ComponentHasMath
(
 iface::cellml_api::CellMLComponent* aComp,
 iface::mathml_dom::MathMLApplyElement* aApply
)
{
  RETURN_INTO_OBJREF(eq, iface::mathml_dom::MathMLElement,
                     aApply->_cxx_operator());
  if (eq == NULL)
    throw CodeGenerationError(L"Apply with no operator");
  DECLARE_QUERY_INTERFACE_OBJREF(eqps, eq, mathml_dom::MathMLPredefinedSymbol);
  if (eqps == NULL)
    throw CodeGenerationError(L"Apply operator is not a predefined symbol.");
  RETURN_INTO_WSTRING(sn, eqps->symbolName());
  if (sn != L"eq")
    throw CodeGenerationError(L"Equals is the only supported top-level "
                              L"operator.");
  uint32_t narg = aApply->nArguments();

  uint32_t i;
  RETURN_INTO_OBJREF(eqn, Equation, new Equation());
  for (i = 2; i <= narg; i++)
  {
    RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                       aApply->getArgument(i));
    eqn->AddPart(aComp, arg);
  }
  mEquations.push_back(eqn);
  mUnusedEquations.push_back(eqn);
  eqn->add_ref();
}

void
CodeGenerationState::ProcessMathInComponent
(
 iface::cellml_api::CellMLComponent* aComp
)
{
  {
    RETURN_INTO_OBJREF(ml, iface::cellml_api::MathList, aComp->math());
    RETURN_INTO_OBJREF(mei, iface::cellml_api::MathMLElementIterator,
                       ml->iterate());
    while (true)
    {
      RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, mei->next());
      if (me == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(mme, me, mathml_dom::MathMLMathElement);
      if (mme == NULL)
        continue;
      // We assume each apply child is a single equation...
      uint32_t l = mme->nArguments(), i;
      // MathML is all 1 based...
      for (i = 1; i <= l; i++)
      {
        RETURN_INTO_OBJREF(eqme, iface::mathml_dom::MathMLElement,
                           mme->getArgument(i));
        DECLARE_QUERY_INTERFACE_OBJREF(eqmae, eqme,
                                       mathml_dom::MathMLApplyElement);
        if (eqmae == NULL)
          continue;
        ComponentHasMath(aComp, eqmae);
      }
    }
  }
  // Now go through all reactions...
  RETURN_INTO_OBJREF(rs, iface::cellml_api::ReactionSet, aComp->reactions());
  RETURN_INTO_OBJREF(ri, iface::cellml_api::ReactionIterator,
                     rs->iterateReactions());
  while (true)
  {
    RETURN_INTO_OBJREF(r, iface::cellml_api::Reaction, ri->nextReaction());
    if (r == NULL)
      break;
    RETURN_INTO_OBJREF(vrs, iface::cellml_api::VariableRefSet,
                       r->variableReferences());
    RETURN_INTO_OBJREF(vri, iface::cellml_api::VariableRefIterator,
                       vrs->iterateVariableRefs());
    while (true)
    {
      RETURN_INTO_OBJREF(vr, iface::cellml_api::VariableRef,
                         vri->nextVariableRef());
      if (vr == NULL)
        break;
      RETURN_INTO_OBJREF(roles, iface::cellml_api::RoleSet,
                         vr->roles());
      RETURN_INTO_OBJREF(rolei, iface::cellml_api::RoleIterator,
                         roles->iterateRoles());
      while (true)
      {
        RETURN_INTO_OBJREF(role, iface::cellml_api::Role,
                           rolei->nextRole());
        if (role == NULL)
          break;
        RETURN_INTO_OBJREF(ml, iface::cellml_api::MathList, role->math());
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
            continue;
          // We assume each apply child is a single equation...
          uint32_t l = mme->nArguments(), i;
          // MathML is all 1 based...
          for (i = 1; i <= l; i++)
          {
            RETURN_INTO_OBJREF(eqme, iface::mathml_dom::MathMLElement,
                               mme->getArgument(i));
            DECLARE_QUERY_INTERFACE_OBJREF(eqmae, eqme,
                                           mathml_dom::MathMLApplyElement);
            if (eqmae == NULL)
              continue;
            ComponentHasMath(aComp, eqmae);
          }
        }
      }
    }
  }
}

void
CodeGenerationState::ProcessVariables()
{
  // Go through each equation looking for variables. This will also find all
  // variables which are used and have initial_value="variable" on them, and
  // add them to the separate initial assignments list...
  std::list<Equation*>::iterator i;
  for (i = mEquations.begin(); i != mEquations.end(); i++)
    (*i)->FlagVariables(this, mInitialAssignments);
}

// Note: Internal only. Does not call add_ref.
VariableInformation*
CodeGenerationState::FindOrAddVariableInformation
(
 iface::cellml_api::CellMLVariable* var
)
{
  char* xId = var->objid();
  std::string id = xId;
  free(xId);
  std::map<std::string,VariableInformation*>::iterator i;

  i = mVariableByObjid.find(id);
  if (i == mVariableByObjid.end())
  {
    RETURN_INTO_OBJREF(svar, iface::cellml_api::CellMLVariable,
                       var->sourceVariable());
    if (svar == NULL)
    {
      std::wstring aMsg = L"Cannot find source for variable ";
      RETURN_INTO_WSTRING(vn, var->name());
      aMsg += vn;
      throw CodeGenerationError(aMsg);
    }
    RETURN_INTO_OBJREF(vi, VariableInformation,
                       new VariableInformation(svar, varinfoKey));
    // Set the annotation...
    annot.addAnnotation(vi);
    xId = svar->objid();
    id = xId;
    free(xId);
    mVariableByObjid.insert(std::pair<std::string,VariableInformation*>
                            (id, vi));
    mVariableList.push_back(vi);

    // Now we iterate all connected variables, and set them up too...
    RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                       svar->connectedVariables());
    RETURN_INTO_OBJREF(cvi, iface::cellml_api::CellMLVariableIterator,
                       cvs->iterateVariables());
    while (true)
    {
      RETURN_INTO_OBJREF(cvar, iface::cellml_api::CellMLVariable,
                         cvi->nextVariable());
      if (cvar == NULL)
        break;
      RETURN_INTO_OBJREF(vic, VariableInformation,
                         new VariableInformation(cvar, varinfoKey, vi));
      RETURN_INTO_WSTRING(svn, svar->name());
      RETURN_INTO_WSTRING(cvn, cvar->name());
      annot.addAnnotation(vic);
      xId = cvar->objid();
      id = xId;
      free(xId);
      mVariableByObjid.insert(std::pair<std::string,VariableInformation*>
                              (id, vi));
    }

    return vi;
  }
  return (*i).second;
}

void
CodeGenerationState::AssignVariableIndices()
{
  std::list<VariableInformation*>::iterator i;
  uint32_t diffcount = 0;
  // Firstly count differentials (including higher diffs)...
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
    if ((*i)->IsFlagged(VariableInformation::SUBJECT_OF_DIFF))
    {
      diffcount += (*i)->GetDegree();
    }
  uint32_t diffIndex = 0, mainIndex = diffcount, boundIndex = 0,
           constIndex = 0;
  // Now, we assign indices to each variable.
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
  {
    if ((*i)->IsFlagged(VariableInformation::BVAR_OF_DIFF))
    {
      (*i)->SetIndex(boundIndex++);
      (*i)->SetArray(VariableInformation::INDEPENDENT);
    }
    else if ((*i)->IsFlagged(VariableInformation::SUBJECT_OF_DIFF))
    {
      (*i)->SetIndex(diffIndex);
      diffIndex += (*i)->GetDegree();
      (*i)->SetArray(VariableInformation::DEPENDENT_AND_RATE);
    }
    else if ((*i)->IsFlagged(VariableInformation::HAS_INITIAL_VALUE))
    {
      (*i)->SetIndex(constIndex++);
      (*i)->SetArray(VariableInformation::CONSTANT);
    }
    else
    {
      (*i)->SetIndex(mainIndex++);
      (*i)->SetArray(VariableInformation::DEPENDENT);
    }
  }
}

void
CodeGenerationState::DetermineConstants(std::stringstream& aConstStream)
{
  std::list<VariableInformation*>::iterator i;
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
  {
    if ((*i)->GetArray() == VariableInformation::CONSTANT)
      aConstStream << "CONSTANTS[" << (*i)->GetIndex() << "] = "
                   << (*i)->GetInitialValue() << ";" << std::endl;;
  }
}

void
CodeGenerationState::DetermineComputedConstants
(
 std::stringstream& aCompConstStream,
 std::stringstream& aSupplementary
)
{
  std::set<VariableInformation*> availableVariables;
  std::set<VariableInformation*> wantedVariables;
  // All constants are available, all non-differentials are wanted...
  std::list<VariableInformation*>::iterator i;
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
  {
    if ((*i)->GetArray() == VariableInformation::CONSTANT)
      availableVariables.insert(*i);
    else if ((*i)->GetArray() == VariableInformation::DEPENDENT)
      wantedVariables.insert(*i);
  }
  
  bool lastRoundUseful;
  // Now we keep cycling through the equations until we stop finding anything
  // new. Also check initial value copies...
  do
  {
    lastRoundUseful = false;
    
    // Go through all initial values...
    std::list<InitialAssignment>::iterator iia, iiatmp;
    for (iia = mInitialAssignments.begin(); iia != mInitialAssignments.end();)
    {
      iiatmp = iia;
      iia++;

      // If the source is available, and the destination wanted, do the
      // assignment...
      if (availableVariables.count((*iiatmp).source) &&
          (wantedVariables.count((*iiatmp).destination) ||
           ((*iiatmp).destination->GetArray() ==
            VariableInformation::DEPENDENT_AND_RATE)))
      {
        if ((*iiatmp).destination->GetArray() !=
            VariableInformation::DEPENDENT_AND_RATE)
        {
          wantedVariables.erase((*iiatmp).destination);
          availableVariables.insert((*iiatmp).destination);
        }
        mInitialAssignments.erase(iiatmp);
        lastRoundUseful = true;
        // Write out the assignment...
        aCompConstStream << GetVariableText((*iiatmp).destination)
                         << " = ";
        if ((*iiatmp).offset != 0.0)
          aCompConstStream << "(";
        if ((*iiatmp).factor != 1.0)
          aCompConstStream << "(";
        aCompConstStream << GetVariableText((*iiatmp).source);
        if ((*iiatmp).factor != 1.0)
          aCompConstStream << ") * " << (*iiatmp).factor;
        if ((*iiatmp).offset != 0.0)
          aCompConstStream << ") + " << (*iiatmp).offset;
        aCompConstStream << ";" << std::endl;
      }
    }

    std::list<Equation*>::iterator i2, itmp;
    for (i2 = mUnusedEquations.begin(); i2 != mUnusedEquations.end();)
    {
      itmp = i2;
      i2++;
      lastRoundUseful |=
        (*itmp)->AttemptEvaluation(this, availableVariables, wantedVariables,
                                 aCompConstStream, aSupplementary);
    }
  }
  while (lastRoundUseful);

  // Now flag everything available...
  std::set<VariableInformation*>::iterator i2;
  for (i2 = availableVariables.begin(); i2 != availableVariables.end(); i2++)
    (*i2)->SetFlag(VariableInformation::PRECOMPUTED);

  // Initial values can now be computed...
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
  {
    if ((*i)->GetArray() != VariableInformation::DEPENDENT_AND_RATE)
      continue;
    if ((*i)->IsFlagged(VariableInformation::HAS_INITIAL_VALUE))
    {
      aCompConstStream << GetVariableText((*i)) << " = "
                       << (*i)->GetInitialValue() << ";" << std::endl;
    }
  }
}

void
CodeGenerationState::DetermineIterationVariables
(
 std::stringstream& aIterationStream,
 std::stringstream& aSupplementary
)
{
  std::set<VariableInformation*> availableVariables;
  std::set<VariableInformation*> wantedVariables;

  std::list<VariableInformation*>::iterator i;
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
  {
    if ((*i)->GetArray() == VariableInformation::DEPENDENT &&
        !(*i)->IsFlagged(VariableInformation::PRECOMPUTED))
      // dependent, non-precomputed variables are wanted...
      wantedVariables.insert(*i);
    else // everything else is already available...
      availableVariables.insert(*i);
  }

  bool lastRoundUseful;
  // Now we keep cycling through the equations until we stop finding anything
  // new. Also check initial value copies...
  do
  {
    lastRoundUseful = false;
    
    std::list<Equation*>::iterator i2, itmp;
    for (i2 = mUnusedEquations.begin(); i2 != mUnusedEquations.end();)
    {
      itmp = i2;
      i2++;
      lastRoundUseful |=
        (*itmp)->AttemptEvaluation(this, availableVariables, wantedVariables,
                                   aIterationStream, aSupplementary);
    }
  }
  while (lastRoundUseful);

  // wantedVariables should be the empty set, unless we are underconstrained...
  if (wantedVariables.size() == 0)
    return;

  // We are underconstrained...
  UnderconstrainedError uce;
  std::set<VariableInformation*>::iterator si;
  for (si = wantedVariables.begin(); si != wantedVariables.end(); si++)
    uce.addUnresolvedWanted((*si)->GetSourceVariable());

  throw uce;
}

void
CodeGenerationState::DetermineRateVariables
(
 std::stringstream& aRateStream,
 std::stringstream& aSupplementary
)
{
  std::list<Equation*>::iterator i;
  for (i = mEquations.begin(); i != mEquations.end(); i++)
    (*i)->AttemptRateEvaluation(this, aRateStream, aSupplementary);
}

bool IsPlainCI(iface::mathml_dom::MathMLElement* aSubExpr)
{
  DECLARE_QUERY_INTERFACE_OBJREF(aCISub, aSubExpr,
                                 mathml_dom::MathMLCiElement);
  return (aCISub != NULL);
}

double
CodeGenerationState::GetConversion
(
 iface::cellml_api::CellMLVariable* vfrom,
 iface::cellml_api::CellMLVariable* vto,
 double& offset
)
{
  RETURN_INTO_OBJREF(fromcompe, iface::cellml_api::CellMLElement,
                     vfrom->parentElement());
  if (fromcompe == NULL)
    throw CodeGenerationError
      (
       L"Attempt to convert from a variable which isn't in any component."
      );
  RETURN_INTO_OBJREF(tocompe, iface::cellml_api::CellMLElement,
                     vto->parentElement());
  if (tocompe == NULL)
    throw CodeGenerationError
      (
       L"Attempt to convert to a variable which isn't in any component."
      );

  RETURN_INTO_OBJREF(toscopeud, iface::cellml_api::UserData,
                     tocompe->getUserData(scopeKey.str().c_str()));
  RETURN_INTO_OBJREF(fromscopeud, iface::cellml_api::UserData,
                     fromcompe->getUserData(scopeKey.str().c_str()));
  if (toscopeud == NULL)
    throw CodeGenerationError
      (
       L"To variable component in conversion doesn't have scope."
      );
  if (fromscopeud == NULL)
    throw CodeGenerationError
      (
       L"From variable component in conversion doesn't have scope."
      );

  CellMLScope* toscope = dynamic_cast<CellMLScope*>(toscopeud.getPointer());
  CellMLScope* fromscope = dynamic_cast<CellMLScope*>
    (fromscopeud.getPointer());
  if (toscope == NULL || fromscope == NULL)
    throw CodeGenerationError
      (
       L"Found scope user data with wrong type."
      );

  RETURN_INTO_WSTRING(tounitsname, vto->unitsName());
  RETURN_INTO_WSTRING(fromunitsname, vfrom->unitsName());

  RETURN_INTO_OBJREF(tounits, CanonicalUnitRepresentation,
                     toscope->findUnit(tounitsname.c_str()));
  if (tounits == NULL)
  {
    std::wstring emsg = L"Variable references unknown units ";
    emsg += tounitsname;
    throw CodeGenerationError(emsg);
  }
  RETURN_INTO_OBJREF(fromunits, CanonicalUnitRepresentation,
                     fromscope->findUnit(fromunitsname.c_str()));
  if (fromunits == NULL)
  {
    std::wstring emsg  = L"Variable references unknown units ";
    emsg += fromunitsname;
    throw CodeGenerationError
      (
       L"From units is not defined in the from units name."
      );
  }

  offset = tounits->getOffset() - fromunits->getOffset();
  return tounits->getOverallFactor() / fromunits->getOverallFactor();
}

bool
CodeGenerationState::UnitsValid(iface::cellml_api::CellMLComponent* aComp,
                                const std::wstring& aUnits)
{
  RETURN_INTO_OBJREF(scopeud, iface::cellml_api::UserData,
                     aComp->getUserData(scopeKey.str().c_str()));
  if (scopeud == NULL)
    throw CodeGenerationError
      (
       L"Component doesn't have scope (bug)."
      );
  CellMLScope* scope = dynamic_cast<CellMLScope*>(scopeud.getPointer());
  if (scope == NULL)
    throw CodeGenerationError
      (
       L"Scope user data is of wrong type (bug)."
      );
  RETURN_INTO_OBJREF(un, CanonicalUnitRepresentation,
                     scope->findUnit(aUnits.c_str()));
  return (un != NULL);
}

uint32_t
CodeGenerationState::AssignFunctionId()
{
  return ++mLastFunctionId;
}

uint32_t
CodeGenerationState::GetVariableIndex
(
 iface::cellml_api::CellMLVariable* aVar
)
{
  // Find the variable information...
  RETURN_INTO_OBJREF(ud, iface::cellml_api::UserData,
                     aVar->getUserData(varinfoKey.str().c_str()));
  VariableInformation* vi = dynamic_cast<VariableInformation*>(ud.getPointer());
  if (vi == NULL)
    throw CodeGenerationError(L"Could not convert the user data to variable "
                              L"information");
  return vi->GetIndex();
}

std::string
CodeGenerationState::GetVariableText
(
 iface::cellml_api::CellMLVariable* aVar
)
{
  // Find the variable information...
  RETURN_INTO_OBJREF(ud, iface::cellml_api::UserData,
                     aVar->getUserData(varinfoKey.str().c_str()));
  VariableInformation* vi = dynamic_cast<VariableInformation*>(ud.getPointer());
  if (vi == NULL)
    throw CodeGenerationError(L"Could not convert the user data to variable "
                              L"information");
  std::string text;
  switch (vi->GetArray())
  {
  case VariableInformation::INDEPENDENT:
    text = "BOUND[";
    break;
  case VariableInformation::DEPENDENT_AND_RATE:
  case VariableInformation::DEPENDENT:
    text = "VARIABLES[";
    break;
  case VariableInformation::CONSTANT:
    text = "CONSTANTS[";
  }
  char buf[30];
  sprintf(buf, "%u]", vi->GetIndex());
  text += buf;

  return text;
}

std::string
CodeGenerationState::GetVariableText
(
 VariableInformation* vi
)
{
  std::string text;
  switch (vi->GetArray())
  {
  case VariableInformation::INDEPENDENT:
    text = "BOUND[";
    break;
  case VariableInformation::DEPENDENT_AND_RATE:
  case VariableInformation::DEPENDENT:
    text = "VARIABLES[";
    break;
  case VariableInformation::CONSTANT:
    text = "CONSTANTS[";
  }
  char buf[30];
  sprintf(buf, "%u]", vi->GetIndex());
  text += buf;

  return text;
}

void
CodeGenerationState::EquationFullyUsed(Equation* aUsedEquation)
{
  std::list<Equation*>::iterator i;
  for (i = mUnusedEquations.begin(); i != mUnusedEquations.end(); i++)
    if ((*i) == aUsedEquation)
    {
      // delete (*i);
      mUnusedEquations.erase(i);
      return;
    }
}

void
CodeGenerationState::CountVariablesAndRates
(
 uint32_t& aVariableCount,
 uint32_t& aConstantCount,
 uint32_t& aBoundCount,
 uint32_t& aRateCount
)
{
  std::list<VariableInformation*>::iterator vii;
  aVariableCount = 0;
  aConstantCount = 0;
  aBoundCount = 0;
  aRateCount = 0;
  for (vii = mVariableList.begin(); vii != mVariableList.end(); vii++)
  {
    if ((*vii)->GetArray() == VariableInformation::INDEPENDENT)
      aBoundCount++;
    else if ((*vii)->GetArray() == VariableInformation::CONSTANT)
      aConstantCount++;
    else
    {
      aVariableCount += (*vii)->GetDegree();
      if ((*vii)->GetArray() == VariableInformation::DEPENDENT_AND_RATE)
        aRateCount += (*vii)->GetDegree();
    }
  }
}

void
CodeGenerationState::VariablesToCCodeVariables
(std::list<iface::cellml_services::CCodeVariable*>& aVarList)
{
  std::list<VariableInformation*>::iterator i;
  for (i = mVariableList.begin(); i != mVariableList.end(); i++)
  {
    iface::cellml_services::VariableEvaluationType type;
    if ((*i)->IsFlagged(VariableInformation::BVAR_OF_DIFF))
      type = iface::cellml_services::BOUND;
    else if ((*i)->IsFlagged(VariableInformation::SUBJECT_OF_DIFF))
      type = iface::cellml_services::DIFFERENTIAL;
    else if ((*i)->IsFlagged(VariableInformation::HAS_INITIAL_VALUE))
      type = iface::cellml_services::CONSTANT;
    else if ((*i)->IsFlagged(VariableInformation::PRECOMPUTED))
      type = iface::cellml_services::COMPUTED_CONSTANT;
    else
      type = iface::cellml_services::COMPUTED;

    aVarList.push_back
      (new CDA_CCodeVariable
       ((*i)->GetSourceVariable(), (*i)->GetIndex(),
        ((*i)->GetArray() == VariableInformation::DEPENDENT_AND_RATE),
        (*i)->GetDegree(), type
        ));
  }
}

void
CodeGenerationState::ListFlaggedEquations
(std::vector<iface::dom::Element*>& aFlaggedEqns)
{
  std::list<Equation*>::iterator i;
  for (i = mEquations.begin(); i != mEquations.end(); i++)
    (*i)->AddIfTriggersNR(aFlaggedEqns);
}

CDA_CCodeInformation::CDA_CCodeInformation
(
 iface::cellml_api::Model* aSourceModel
)
  : _cda_refcount(1)
{
  CodeGenerationState cgs;
  
  // If you don't want this to block, simply instantiate first.
  aSourceModel->fullyInstantiateImports();
  
  // Build the set of all scopes (this will build model scopes, and as a
  // side-effect, also build component scopes). The scopes will contain the
  // appropriate units.
  cgs.CreateModelScopes(aSourceModel);
  
  // Build a list of all components which are required by this particular
  // model.
  cgs.CreateComponentList(aSourceModel);

  // Build a list of the mathematics...
  cgs.ProcessMath();

  // Build a list of all variables, and classify them...
  cgs.ProcessVariables();

  // Assign indices (and arrays) to variables...
  cgs.AssignVariableIndices();

  cgs.CountVariablesAndRates(mVariableCount, mConstantCount, mBoundCount,
                             mRateVariableCount);

  // Generate the constant assignment code...
  cgs.DetermineConstants(mFixedConstantFragment);

  try
  {
    // Work out what can be computed without resorting known differentials or
    // bound variables...
    cgs.DetermineComputedConstants(mComputedConstantFragment,
                                   mFunctionsFragment);
    
    // Now write the code to compute the remaining variables...
    cgs.DetermineIterationVariables(mVariableCodeFragment, mFunctionsFragment);
    cgs.DetermineRateVariables(mRateCodeFragment, mFunctionsFragment);

    mConstraintLevel = iface::cellml_services::CORRECTLY_CONSTRAINED;

    // Convert the variables into CCodeVariables...
    cgs.VariablesToCCodeVariables(mVariables);

    // Populate the flagged variables list...
    cgs.ListFlaggedEquations(mFlaggedEquations);
  }
  catch (UnderconstrainedError& uce)
  {
    std::list<iface::cellml_api::CellMLVariable*>::iterator i;
    for (i = uce.mUnresolvedWanted.begin(); i != uce.mUnresolvedWanted.end();
         i++)
      mVariables.push_back(new CDA_CCodeVariable(*i));
    mConstraintLevel = iface::cellml_services::UNDERCONSTRAINED;
  }
  catch (OverconstrainedError& oce)
  {
    std::list<iface::cellml_api::CellMLVariable*>::iterator i;
    for (i = oce.mKnownVariables.begin(); i != oce.mKnownVariables.end();
         i++)
      mVariables.push_back(new CDA_CCodeVariable(*i));
    oce.mEqn->add_ref();
    mFlaggedEquations.push_back(oce.mEqn);
    mConstraintLevel = iface::cellml_services::OVERCONSTRAINED;
  }
}
