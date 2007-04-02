#include "CodeGenerationState.hxx"

// The temporary disjoint set data structure used to connect variables
// efficiently.
class VariableDisjointSet
{
public:
  VariableDisjointSet(iface::cellml_api::CellMLVariable* aVar)
    : mRank(0), mParent(NULL), mVar(aVar), mSource(NULL)
  {
  }

  VariableDisjointSet* findRoot()
  {
    if (mParent == NULL)
      return this;

    mParent = mParent->findRoot();

    return mParent;
  }

  void unionWith(VariableDisjointSet* aSet)
  {
    VariableDisjointSet* myRoot = findRoot();
    VariableDisjointSet* theirRoot = aSet->findRoot();

    if (myRoot == theirRoot)
      return;

    if (myRoot->mRank > theirRoot->mRank)
      theirRoot->mParent = myRoot;
    else if (myRoot->mRank < theirRoot->mRank)
      myRoot->mParent = theirRoot;
    else
    {
      theirRoot->mParent = myRoot;
      myRoot->mRank++;
    }
  }

  void setSource(VariableInformation* aSource)
  {
    findRoot()->mSource = aSource;
  }

  VariableInformation* getSource()
  {
    return findRoot()->mSource;
  }

  uint32_t mRank;
  VariableDisjointSet* mParent;
  // We don't hold a reference count, instead we assume that someone will
  // hold the model, so protect it from disappearing.
  iface::cellml_api::CellMLVariable* mVar;
  VariableInformation *mSource;
  bool mIsMainModel;
};

class AutoCleanupDisjointSets
{
public:
  AutoCleanupDisjointSets(std::map<iface::cellml_api::CellMLVariable*,
                          VariableDisjointSet*>& aCleanup)
    : mCleanup(aCleanup)
  {
  }

  ~AutoCleanupDisjointSets()
  {
    std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>::iterator i;
    for (i = mCleanup.begin(); i != mCleanup.end(); i++)
      delete (*i).second;
  }

private:
  std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>& mCleanup;
};

void
CodeGenerationState::BuildVIMForConnections
(
 iface::cellml_api::Model* aModel,
 std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>& vsMap
)
{
  // Now go through all the connections, and join sets together...
  RETURN_INTO_OBJREF(conns, iface::cellml_api::ConnectionSet,
                     aModel->connections());
  RETURN_INTO_OBJREF(conni, iface::cellml_api::ConnectionIterator,
                     conns->iterateConnections());
  while (true)
  {
    RETURN_INTO_OBJREF(conn, iface::cellml_api::Connection,
                       conni->nextConnection());
    if (conn == NULL)
      break;
    RETURN_INTO_OBJREF(mvs, iface::cellml_api::MapVariablesSet,
                       conn->variableMappings());
    RETURN_INTO_OBJREF(mvi, iface::cellml_api::MapVariablesIterator,
                       mvs->iterateMapVariables());
    while (true)
    {
      RETURN_INTO_OBJREF(mv, iface::cellml_api::MapVariables,
                         mvi->nextMapVariable());
      if (mv == NULL)
        break;
      ObjRef<iface::cellml_api::CellMLVariable> v1, v2;
      try
      {
        v1 = already_AddRefd<iface::cellml_api::CellMLVariable>
          (mv->firstVariable());
      }
      catch (iface::cellml_api::CellMLException&)
      {
        std::wstring msg(L"Cannot find variable ");
        RETURN_INTO_WSTRING(name, mv->firstVariableName());
        msg += name;
        msg += L" (or the component it is in, ";
        RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                           conn->componentMapping());
        RETURN_INTO_WSTRING(cname, cm->firstComponentName());
        msg += cname;
        msg += L"), referenced from a connection.";
        throw CodeGenerationError(msg);
      }

      try
      {
        v2 = already_AddRefd<iface::cellml_api::CellMLVariable>
          (mv->secondVariable());
      }
      catch (iface::cellml_api::CellMLException&)
      {
        std::wstring msg(L"Cannot find variable ");
        RETURN_INTO_WSTRING(name, mv->secondVariableName());
        msg += name;
        msg += L" (or the component it is in, ";
        RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                           conn->componentMapping());
        RETURN_INTO_WSTRING(cname, cm->secondComponentName());
        msg += cname;
        msg += L"), referenced from a connection.";
        throw CodeGenerationError(msg);
      }

      std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>::
        iterator i;
      i = vsMap.find(v1);
      if (i == vsMap.end())
        break;

      VariableDisjointSet* vds1 = (*i).second;
      i = vsMap.find(v2);
      
      if (i == vsMap.end())
        break;

      VariableDisjointSet* vds2 = (*i).second;
      vds1->unionWith(vds2);
    }
  }

  // Connections from imports also matter...
  RETURN_INTO_OBJREF(is, iface::cellml_api::CellMLImportSet,
                     aModel->imports());
  RETURN_INTO_OBJREF(ii, iface::cellml_api::CellMLImportIterator,
                     is->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       ii->nextImport());
    if (imp == NULL)
      break;
    RETURN_INTO_OBJREF(im, iface::cellml_api::Model,
                       imp->importedModel());
    BuildVIMForConnections(im, vsMap);
  }
}

void
ProcessInitialValue(
                    CodeGenerationState* aCGS,
                    std::list<InitialAssignment>& aInitial,
                    VariableInformation* vi,
                    iface::cellml_api::CellMLVariable *var,
                    iface::cellml_api::CellMLVariable *sv,
                    uint32_t depth
                   )
{
  if (depth > 100)
  {
    throw CodeGenerationError(L"A sequence of >100 initial value assignments "
                              L"from other variables was found. You probably "
                              L"created a loop.");
  }

  if (vi->IsFlagged(VariableInformation::SEEN_INITIAL_VALUE))
    return;
  vi->SetFlag(VariableInformation::SEEN_INITIAL_VALUE);

  RETURN_INTO_WSTRING(iv, sv->initialValue());
  if (iv != L"")
  {
    // See if it is a double...
    const wchar_t* ivs = iv.c_str();
    wchar_t* ivend;
    double val = wcstod(ivs, &ivend);
    if (*ivend == 0)
    {
      vi->SetFlag(VariableInformation::HAS_INITIAL_VALUE);
      vi->SetInitialValue(val);
    }
    else
    {
      vi->SetFlag(VariableInformation::HAS_INITIAL_ASSIGNMENT);
      InitialAssignment ia;
      ia.destination = vi;
      // Find the component...
      RETURN_INTO_OBJREF(compel, iface::cellml_api::CellMLElement,
                         sv->parentElement());
      DECLARE_QUERY_INTERFACE_OBJREF(comp, compel,
                                     cellml_api::CellMLComponent);
      if (comp == NULL)
        throw CodeGenerationError(L"Parent of variable isn't a component.");
      // Search for the variable...
      RETURN_INTO_OBJREF(varset, iface::cellml_api::CellMLVariableSet,
                         comp->variables());
      RETURN_INTO_OBJREF(svar, iface::cellml_api::CellMLVariable,
                         varset->getVariable(ivs));
      if (svar == NULL)
      {
        std::wstring emsg = L"Variable ";
        RETURN_INTO_WSTRING(v, var->name());
        emsg += v;
        emsg += L" has initial_value ";
        emsg += ivs;
        emsg += L" which isn't a valid floating point number, nor a valid "
          L"variable name within that component.";
        throw CodeGenerationError(emsg);
      }
      VariableInformation* svi = aCGS->FindOrAddVariableInformation(svar);
      ProcessInitialValue(aCGS, aInitial, svi, svar,
                          svi->GetSourceVariable(), depth + 1);
      ia.source = svi;
      ia.factor = aCGS->GetConversion(svi->GetSourceVariable(), sv, ia.offset);
      aInitial.push_back(ia);
    }
  }
}

void
CodeGenerationState::BuildVariableInformationMap
(
 iface::cellml_api::Model* aModel
)
{
  std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*> vsMap;
  // When we are done with this function, destroy the sets...
  AutoCleanupDisjointSets cleaner(vsMap);

  // Iterate all variables, and build them into disjoint connected sets...
  std::list<iface::cellml_api::CellMLComponent*>::iterator compi;
  for (compi = mComponentList.begin(); compi != mComponentList.end(); compi++)
  {
    RETURN_INTO_OBJREF(vars, iface::cellml_api::CellMLVariableSet,
                       (*compi)->variables());
    RETURN_INTO_OBJREF(vari, iface::cellml_api::CellMLVariableIterator,
                       vars->iterateVariables());
    while (true)
    {
      RETURN_INTO_OBJREF(var, iface::cellml_api::CellMLVariable,
                         vari->nextVariable());
      if (var == NULL)
        break;
      vsMap.insert(std::pair<iface::cellml_api::CellMLVariable*,
                             VariableDisjointSet*>
                   (var, new VariableDisjointSet(var)));
    }
  }

  BuildVIMForConnections(aModel, vsMap);

  // Now, we must go through all variables, identify sources, and tag the
  // parent with the source...
  std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>::iterator i;
  for (i = vsMap.begin(); i != vsMap.end(); i++)
  {
    iface::cellml_api::CellMLVariable* var = (*i).first;
    if (var->publicInterface() != iface::cellml_api::INTERFACE_IN &&
        var->privateInterface() != iface::cellml_api::INTERFACE_IN)
    {
      //if (!(*i).second->getMainModel())
      //  continue;

      RETURN_INTO_OBJREF(vi, VariableInformation,
                         new VariableInformation(var, varinfoKey));

      if ((*i).second->getSource() != NULL)
      {
        std::wstring aMsg = L"Two variables with incompatible interfaces are connected: ";
        RETURN_INTO_WSTRING(vn, var->name());
        aMsg += vn;
        aMsg += L" and ";
        aMsg += (*i).second->getSource()->GetName();
        throw CodeGenerationError(aMsg);
      }

      (*i).second->setSource(vi);

      // Set the annotation...
      annot.addAnnotation(vi);
      
      char* xId = var->objid();
      std::string id = xId;
      free(xId);

      mVariableByObjid.insert(std::pair<std::string,VariableInformation*>
                              (id, vi));
      mVariableList.push_back(vi);
    }
  }
  for (i = vsMap.begin(); i != vsMap.end(); i++)
  {
    iface::cellml_api::CellMLVariable* var = (*i).first;

    if ((var->publicInterface() != iface::cellml_api::INTERFACE_IN &&
         var->privateInterface() != iface::cellml_api::INTERFACE_IN) /*||
        !(*i).second->getMainModel()*/)
      continue;

    VariableInformation* source = (*i).second->getSource();
    if (source == NULL)
    {
      std::wstring aMsg = L"Cannot find source for variable ";
      RETURN_INTO_WSTRING(vn, var->name());
      aMsg += vn;
      aMsg += L" in component ";
      RETURN_INTO_WSTRING(cn, var->componentName());
      aMsg += cn;
      throw CodeGenerationError(aMsg);
    }

    RETURN_INTO_OBJREF(vi, VariableInformation,
                       new VariableInformation(var, varinfoKey, source));

    // Set the annotation...
    annot.addAnnotation(vi);

    char* xId = var->objid();
    std::string id = xId;
    free(xId);

    mVariableByObjid.insert(std::pair<std::string,VariableInformation*>
                            (id, vi));
  }

  std::list<VariableInformation*>::iterator vii;
  for (vii = mVariableList.begin(); vii != mVariableList.end(); vii++)
  {
    iface::cellml_api::CellMLVariable* sv = (*vii)->GetSourceVariable();
    ProcessInitialValue(this, mInitialAssignments, (*vii), sv, sv, 1);
  }
}
