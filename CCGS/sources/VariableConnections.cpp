#include "CodeGenerationState.hxx"

// The temporary disjoint set data structure used to connect variables
// efficiently.
class VariableDisjointSet
{
public:
  VariableDisjointSet(iface::cellml_api::CellMLVariable* aVar)
    : mRank(0), mParent(NULL), mVar(aVar)
  {
  }

  VariableDisjointSet* findRoot()
  {
    if (this->mParent == NULL)
      return this;

    this->mParent = this->mParent->findRoot();
    return this->mParent;
  }

  void unionWith(VariableDisjointSet* aSet)
  {
    VariableDisjointSet* myRoot = findRoot();
    VariableDisjointSet* theirRoot = aSet->findRoot();

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
CodeGenerationState::BuildVariableInformationMap
(
 iface::cellml_api::Model* aModel
)
{
  std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*> vsMap;
  // When we are done with this function, destroy the sets...
  AutoCleanupDisjointSets cleaner(vsMap);

  // Iterate all variables, and build them a set...
  RETURN_INTO_OBJREF(comps, iface::cellml_api::CellMLComponentSet,
                     aModel->allComponents());
  RETURN_INTO_OBJREF(compi, iface::cellml_api::CellMLComponentIterator,
                     comps->iterateComponents());
  while (true)
  {
    RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                       compi->nextComponent());
    if (comp == NULL)
      break;
    RETURN_INTO_OBJREF(vars, iface::cellml_api::CellMLVariableSet,
                       comp->variables());
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
      RETURN_INTO_OBJREF(v1, iface::cellml_api::CellMLVariable,
                         mv->firstVariable());
      RETURN_INTO_OBJREF(v2, iface::cellml_api::CellMLVariable,
                         mv->secondVariable());

      std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>::
        iterator i;
      i = vsMap.find(v1);
      VariableDisjointSet* vds1 = (*i).second;
      i = vsMap.find(v2);
      VariableDisjointSet* vds2 = (*i).second;
      vds1->unionWith(vds2);
    }
  }

  // Now, we must go through all variables, identify sources, and tag the
  // parent with the source...
  std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*>::iterator i;
  for (i = vsMap.begin(); i != vsMap.end(); i++)
  {
    iface::cellml_api::CellMLVariable* var = (*i).first;
    if (var->publicInterface() != iface::cellml_api::INTERFACE_IN &&
        var->privateInterface() != iface::cellml_api::INTERFACE_IN)
    {
      RETURN_INTO_OBJREF(vi, VariableInformation,
                         new VariableInformation(var, varinfoKey));
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

    if (var->publicInterface() != iface::cellml_api::INTERFACE_IN &&
        var->privateInterface() != iface::cellml_api::INTERFACE_IN)
      continue;

    VariableInformation* source = (*i).second->getSource();
    if (source == NULL)
    {
      std::wstring aMsg = L"Cannot find source for variable ";
      RETURN_INTO_WSTRING(vn, var->name());
      aMsg += vn;
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
}
