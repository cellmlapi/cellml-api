#define IN_CEVAS_MODULE
#define MODULE_CONTAINS_CeVAS
#include "CeVASImpl.hpp"
#include "CeVASBootstrap.hpp"
#include <set>
#include <assert.h>

class CeVASError
{
public:
  CeVASError(const std::wstring& aMsg)
    : mMsg(aMsg)
  {
  }

  std::wstring&
  getMessage()
  {
    return mMsg;
  }

private:
  std::wstring mMsg;
};

class RelevanceModelData
{
public:
  RelevanceModelData(iface::cellml_api::Model* aModel)
    : inQueue(true), model(aModel) {}
  bool inQueue;
  ObjRef<iface::cellml_api::Model> model;
};

class RelevanceDetermination
{
public:
  RelevanceDetermination(
                         iface::cellml_api::Model * aModel,
                         std::list<iface::cellml_api::CellMLComponent*>&
                           aRelevantComponents,
                         std::list<iface::cellml_api::Model*>&
                           aRelevantModels
                        )
    : mModel(aModel), mRelevantComponents(aRelevantComponents),
      mRelevantModels(aRelevantModels)
  {
  }

  void computeRelevantComponents()
  {
    RETURN_INTO_OBJREF(ccs, iface::cellml_api::CellMLComponentSet,
                       mModel->modelComponents());
    RETURN_INTO_OBJREF(cci, iface::cellml_api::CellMLComponentIterator,
                       ccs->iterateComponents());

    // Firstly the model components...
    while (true)
    {
      RETURN_INTO_OBJREF(cc, iface::cellml_api::CellMLComponent,
                         cci->nextComponent());
      if (cc == NULL)
        break;

      newRelevantComponent(mModel, cc);
    }

    // Now, we have to keep looping around our queue until we manage to empty
    // it out...
    std::list<iface::cellml_api::Model*>::iterator i, j;
    for (i = groupProcessingQueue.begin(); i != groupProcessingQueue.end();
         i = groupProcessingQueue.begin())
    {
      expandUsingEncapsulationInModel(*i);
      groupProcessingQueue.pop_front();
    }

    // Next, populate the list of relevant models...
    std::list<iface::cellml_api::Model*>::iterator mi;
    for (mi = groupList.begin(); mi != groupList.end(); mi++)
    {
      iface::cellml_api::Model* mod = *mi;
      mRelevantModels.push_back(mod);
      mod->add_ref();
    }
  }

private:
  iface::cellml_api::Model * mModel;
  std::list<iface::cellml_api::CellMLComponent*>& mRelevantComponents;
  std::set<iface::cellml_api::CellMLComponent*> relevantComponents;
  std::list<iface::cellml_api::Model*>& mRelevantModels;
  std::list<iface::cellml_api::Model*> groupProcessingQueue;
  std::map<iface::cellml_api::Model*, RelevanceModelData, XPCOMComparator> groupProcessingMap;
  std::list<iface::cellml_api::Model*> groupList;

  bool markComponentRelevant(iface::cellml_api::CellMLComponent* aComp)
  {
    if (relevantComponents.count(aComp) == 0)
    {
      aComp->add_ref();
      mRelevantComponents.push_back(aComp);
      relevantComponents.insert(aComp);
      return true;
    }
    else
      return false;
  }

  void queueModelForGroups(iface::cellml_api::Model* aModel)
  {
    std::map<iface::cellml_api::Model*,RelevanceModelData,XPCOMComparator>::iterator i
      = groupProcessingMap.find(aModel);
    if (i == groupProcessingMap.end())
    {
      groupProcessingMap.insert(std::pair<iface::cellml_api::Model*,
                                          RelevanceModelData>(aModel, aModel));
      groupProcessingQueue.push_back(aModel);
      groupList.push_back(aModel);
    }
    else if (!(*i).second.inQueue)
    {
      (*i).second.inQueue = true;
      groupProcessingQueue.push_back(aModel);
    }
  }

  void newRelevantComponent(iface::cellml_api::Model* aImportStart,
                            iface::cellml_api::CellMLComponent* aComp)
  {
    ObjRef<iface::cellml_api::CellMLComponent> comp(aComp);

    // If the component is actually an import component, find the real
    // component.
    while (true)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(icomp, comp, cellml_api::ImportComponent);
      if (icomp == NULL)
        break;
      RETURN_INTO_OBJREF(impEl, iface::cellml_api::CellMLElement,
                         icomp->parentElement());
      DECLARE_QUERY_INTERFACE_OBJREF(imp, impEl, cellml_api::CellMLImport);
      RETURN_INTO_OBJREF(mod, iface::cellml_api::Model,
                         imp->importedModel());
      RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                         mod->modelComponents());
      RETURN_INTO_WSTRING(compRef, icomp->componentRef());
      comp = already_AddRefd<iface::cellml_api::CellMLComponent>
        (mc->getComponent(compRef.c_str()));
      if (comp == NULL)
      {
        std::wstring msg = L"Import component ";
        msg += compRef;
        msg += L" could not be found; ";
        throw CeVASError(msg);
      }
    }

    if (!markComponentRelevant(comp))
      return;

    // Find each model from aComp up to aImportStart and put that model in the
    // group processing queue.
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement,
                       comp->parentElement());
    DECLARE_QUERY_INTERFACE_OBJREF(importEnd, el, cellml_api::Model);

    do
    {
      queueModelForGroups(importEnd);

      if (importEnd == aImportStart)
        break;
      
      RETURN_INTO_OBJREF(el1, iface::cellml_api::CellMLElement,
                         importEnd->parentElement());
      if (el1 == NULL)
        break;
      RETURN_INTO_OBJREF(el2, iface::cellml_api::CellMLElement,
                         el1->parentElement());
      QUERY_INTERFACE(importEnd, el2, cellml_api::Model);
    }
    while (true);
  }

  void expandUsingEncapsulationInModel(iface::cellml_api::Model* aModel)
  {
    std::map<iface::cellml_api::Model*,RelevanceModelData,XPCOMComparator>::iterator i
      = groupProcessingMap.find(aModel);
    (*i).second.inQueue = false;

    RETURN_INTO_OBJREF(gs, iface::cellml_api::GroupSet, aModel->groups());
    RETURN_INTO_OBJREF(egs, iface::cellml_api::GroupSet,
                       gs->subsetInvolvingEncapsulation());
    RETURN_INTO_OBJREF(egi, iface::cellml_api::GroupIterator,
                       egs->iterateGroups());

    while (true)
    {
      RETURN_INTO_OBJREF(grp, iface::cellml_api::Group,
                         egi->nextGroup());
      if (grp == NULL)
        break;

      RETURN_INTO_OBJREF(crs, iface::cellml_api::ComponentRefSet,
                         grp->componentRefs());
      processEncapsulationComponentRefs(aModel, crs, false);
    }
  }

  void
  processEncapsulationComponentRefs
  (
   iface::cellml_api::Model* aModel,
   iface::cellml_api::ComponentRefSet* aRefs,
   bool aAutoRelevant
  )
  {
    RETURN_INTO_OBJREF(refi, iface::cellml_api::ComponentRefIterator,
                       aRefs->iterateComponentRefs());
    while (true)
    {
      RETURN_INTO_OBJREF(ref, iface::cellml_api::ComponentRef,
                         refi->nextComponentRef());
      if (ref == NULL)
        break;
      
      RETURN_INTO_WSTRING(cn, ref->componentName());
      
      RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                         aModel->modelComponents());
      RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                         mc->getComponent(cn.c_str()));
      if (comp == NULL)
      {
        std::wstring msg = L"Component ";
        msg += cn;
        msg += L" referred to in encapsulation component_ref does not "
               L"exist in model ";
        RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, aModel->xmlBase());
        RETURN_INTO_WSTRING(uri, bu->asText());

        msg += uri;
        msg += L".";

        throw CeVASError(msg);
      }

      // Find the real component...
      while (true)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(ic, comp, cellml_api::ImportComponent);
        if (ic == NULL)
          break;
        RETURN_INTO_OBJREF(impEl, iface::cellml_api::CellMLElement,
                           ic->parentElement());
        DECLARE_QUERY_INTERFACE_OBJREF(imp, impEl, cellml_api::CellMLImport);
        RETURN_INTO_OBJREF(impModel, iface::cellml_api::Model,
                           imp->importedModel());
        RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                           impModel->modelComponents());
        RETURN_INTO_WSTRING(compRef, ic->componentRef());
        comp = already_AddRefd<iface::cellml_api::CellMLComponent>
          (mc->getComponent(compRef.c_str()));
        if (comp == NULL)
        {
          std::wstring msg = L"Component ";
          msg += cn;
          msg += L" referred to from import component_ref does not exist "
                 L"in model ";

          RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, aModel->xmlBase());
          RETURN_INTO_WSTRING(uri, bu->asText());
          
          msg += uri;
          msg += L".";

          throw CeVASError(msg);
        }
      }

      bool childAutoRelevant;
      if (aAutoRelevant)
      {
        newRelevantComponent(aModel, comp);
        childAutoRelevant = true;
      }
      else
        childAutoRelevant = (relevantComponents.count(comp) > 0);

      RETURN_INTO_OBJREF(crs, iface::cellml_api::ComponentRefSet,
                         ref->componentRefs());

      processEncapsulationComponentRefs(aModel, crs, childAutoRelevant);
    }
  }
};

CDAConnectedVariableSet::CDAConnectedVariableSet
(
 iface::cellml_api::CellMLVariable* aSource
)
  throw()
  : mSource(aSource)
{
}

CDAConnectedVariableSet::~CDAConnectedVariableSet()
  throw()
{
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDAConnectedVariableSet::sourceVariable()
  throw(std::exception&)
{
  if (mSource != NULL)
    mSource->add_ref();
  return mSource;
}

uint32_t
CDAConnectedVariableSet::length()
  throw(std::exception&)
{
  return mVariables.size();
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDAConnectedVariableSet::getVariable(uint32_t aIndex)
  throw(std::exception&)
{
  if (aIndex >= mVariables.size())
    return NULL;

  iface::cellml_api::CellMLVariable* v = mVariables[aIndex];
  v->add_ref();

  return v;
}

void
CDAConnectedVariableSet::addVariable(iface::cellml_api::CellMLVariable* v)
{
  v->add_ref();
  mVariables.push_back(v);
}

class VariableDisjointSet
{
public:
  VariableDisjointSet(iface::cellml_api::CellMLVariable* aV)
    : mRank(0), mV(aV), mSource(NULL), mParent(NULL)
  {
    if ((mV->publicInterface() != iface::cellml_api::INTERFACE_IN) ||
        (mV->privateInterface() != iface::cellml_api::INTERFACE_IN))
      mSource = mV;
  }

  VariableDisjointSet* root()
  {
    if (mParent == NULL)
      return this;

    return (mParent = mParent->root());
  }

  void merge(VariableDisjointSet* aWith, iface::cellml_api::CellMLVariable* v1,
             iface::cellml_api::CellMLVariable* v2, bool isParent, bool isChild)
  {
    VariableDisjointSet* s1 = root();
    VariableDisjointSet* s2 = aWith->root();

    // This probably means we have more connections than necessary. It is an
    // error a validator should find, but the intention of the modeller is
    // clear, so no point failing here.
    if (s1 == s2)
      return;

    if (s1->mSource && s2->mSource)
    {
      iface::cellml_api::VariableInterface iface1Conn, iface2Conn, iface1Noconn, iface2Noconn;
      if (isParent)
      {
        iface1Conn = v1->privateInterface();
        iface2Conn = v2->publicInterface();
        iface1Noconn = v1->publicInterface();
        iface2Noconn = v2->privateInterface();
      }
      else if (isChild)
      {
        iface1Conn = v1->publicInterface();
        iface2Conn = v2->privateInterface();
        iface1Noconn = v1->privateInterface();
        iface2Noconn = v2->publicInterface();
      }
      else
      {
        iface1Conn = v1->publicInterface();
        iface2Conn = v2->publicInterface();
        iface1Noconn = v1->privateInterface();
        iface2Noconn = v2->privateInterface();
      }

      if (iface1Conn == iface::cellml_api::INTERFACE_NONE)
      {
        std::wstring msg = L"Variable is connected to another variable on 'none' interface: Variable with none interface: " +
          s1->mSource->name() + L" in component " + s1->mSource->componentName() + L"; other variable: " +
          s2->mSource->name() + L" in component " + s2->mSource->componentName() + L". Note that the relationship between " +
          L"the first and second variables is that the first is the " + (isParent ? L"parent" : (isChild ? L"child" : L"sibling")) +
          L" of the second";
        throw CeVASError(msg);
      }
      if (iface2Conn == iface::cellml_api::INTERFACE_NONE)
      {
        std::wstring msg = L"Variable is connected to another variable on 'none' interface: Variable with none interface: " +
          s2->mSource->name() + L" in component " + s2->mSource->componentName() + L"; other variable: " +
          s1->mSource->name() + L" in component " + s1->mSource->componentName() + L". Note that the relationship between " +
          L"the first and second variables is that the first is the " + (isParent ? L"parent" : (isChild ? L"child" : L"sibling")) +
          L" of the second";
        throw CeVASError(msg);
      }
      if (iface1Conn == iface::cellml_api::INTERFACE_IN && iface2Conn == iface::cellml_api::INTERFACE_IN)
      {
        std::wstring msg = L"Two 'in' variables are connected: variable " +
          s1->mSource->name() + L" in component " + s1->mSource->componentName() + L" and variable " +
          s2->mSource->name() + L" in component " + s2->mSource->componentName();
        throw CeVASError(msg);
      }
      if (iface1Conn == iface::cellml_api::INTERFACE_OUT && iface2Conn == iface::cellml_api::INTERFACE_OUT)
      {
        std::wstring msg = L"Two 'out' variables are connected: variable " +
          s1->mSource->name() + L" in component " + s1->mSource->componentName() + L" and variable " +
          s2->mSource->name() + L" in component " + s2->mSource->componentName();
        throw CeVASError(msg);
      }
      if (iface1Noconn == iface::cellml_api::INTERFACE_IN && iface1Conn == iface::cellml_api::INTERFACE_IN)
      {
        std::wstring msg = L"Variable has two 'in' interfaces: variable " +
          s1->mSource->name() + L" in component " + s1->mSource->componentName();
        throw CeVASError(msg);
      }
      if (iface2Noconn == iface::cellml_api::INTERFACE_IN && iface2Conn == iface::cellml_api::INTERFACE_IN)
      {
        std::wstring msg = L"Variable has two 'in' interfaces: variable " +
          s2->mSource->name() + L" in component " + s2->mSource->componentName();
        throw CeVASError(msg);
      }

      if (iface1Conn == iface::cellml_api::INTERFACE_OUT)
        s2->mSource = s1->mSource;
      else
        s1->mSource = s2->mSource;
    }

    if (s1->mRank > s2->mRank)
    {
      s2->mParent = s1;
      if (s2->mSource)
        s1->mSource = s2->mSource;
    }
    else if (s1->mRank < s2->mRank)
    {
      s1->mParent = s2;
      if (s1->mSource)
        s2->mSource = s1->mSource;
    }
    else
    {
      s2->mParent = s1;
      s1->mRank = s2->mRank + 1;
      if (s2->mSource)
        s1->mSource = s2->mSource;
    }
  }

  uint32_t mRank;
  ObjRef<iface::cellml_api::CellMLVariable> mV;
  iface::cellml_api::CellMLVariable* mSource;
  VariableDisjointSet* mParent;
};

CDACeVAS::CDACeVAS(iface::cellml_api::Model* aModel)
  throw()
{
  try
  {
    aModel->fullyInstantiateImports();
  }
  catch (...)
  {
    mErrorDescription += L"Could not fully instantiate imports.";
    return;
  }

  CleanupList<iface::cellml_api::Model*> relevantModels;

  // Firstly, build up a set of 'relevant' components. A relevant component is
  // a component which actually affects the results of the model.
  try
  {
    {
      RelevanceDetermination rd(aModel, mRelevantComponents, relevantModels);
      rd.computeRelevantComponents();
    }

    ComputeConnectedVariables(relevantModels);
  }
  catch (CeVASError& ce)
  {
    mErrorDescription += ce.getMessage();
  }
}

CDACeVAS::~CDACeVAS()
  throw()
{
}

static void
recursivelyAddRelationshipsFrom
(
 std::set<std::pair<iface::cellml_api::CellMLComponent*, iface::cellml_api::CellMLComponent*> >& aRels,
 iface::cellml_api::ComponentRef* aCR
)
{
  ObjRef<iface::cellml_api::Model> me(aCR->modelElement());
  ObjRef<iface::cellml_api::CellMLComponentSet> ccs(me->modelComponents());
  ObjRef<iface::cellml_api::ComponentRefSet> crs(aCR->componentRefs());
  ObjRef<iface::cellml_api::ComponentRefIterator> cri(crs->iterateComponentRefs());

  std::wstring cn1 = aCR->componentName();
  ObjRef<iface::cellml_api::CellMLComponent> c1(ccs->getComponent(cn1.c_str()));
  if (c1 == NULL)
    return;

  while (true)
  {
    ObjRef<iface::cellml_api::ComponentRef> cr(cri->nextComponentRef());
    if (cr == NULL)
      break;

    std::wstring cn2 = cr->componentName();
    ObjRef<iface::cellml_api::CellMLComponent> c2(ccs->getComponent(cn2.c_str()));
    if (c2 == NULL)
      continue;

    c1->add_ref();
    c2->add_ref();
    aRels.insert(std::pair<iface::cellml_api::CellMLComponent*, iface::cellml_api::CellMLComponent*>
                 (c1, c2));

    recursivelyAddRelationshipsFrom(aRels, cr);
  }
}

void
CDACeVAS::ComputeConnectedVariables
(
 std::list<iface::cellml_api::Model*>& aRelevantModels
)
{
  // Build a set of encapsulation relationships...
  std::set<std::pair<iface::cellml_api::CellMLComponent*, iface::cellml_api::CellMLComponent*> > encap;
  scoped_destroy<std::set<std::pair<iface::cellml_api::CellMLComponent*, iface::cellml_api::CellMLComponent*> > > dEncap(encap,
    new container_destructor<std::set<std::pair<iface::cellml_api::CellMLComponent*, iface::cellml_api::CellMLComponent*> > >(
    (new pair_both_destructor<iface::cellml_api::CellMLComponent*, iface::cellml_api::CellMLComponent*>
     (new objref_destructor<iface::cellml_api::CellMLComponent>(),
      new objref_destructor<iface::cellml_api::CellMLComponent>()))));

  // Build a collection of disjoint sets.
  AutoList<VariableDisjointSet*> disjointSetElements;
  std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*,
           XPCOMComparator>
    varToDSMap;

  for (std::list<iface::cellml_api::Model*>::iterator i = aRelevantModels.begin(); i != aRelevantModels.end(); i++)
  {
    ObjRef<iface::cellml_api::GroupSet> gs = 
      (*i)->findGroupsWithRelationshipRefName(L"encapsulation");
    ObjRef<iface::cellml_api::GroupIterator> gi(gs->iterateGroups());
    while (true)
    {
      ObjRef<iface::cellml_api::Group> g(gi->nextGroup());
      if (g == NULL)
        break;
      ObjRef<iface::cellml_api::ComponentRefSet> crs(g->componentRefs());
      ObjRef<iface::cellml_api::ComponentRefIterator> cri(crs->iterateComponentRefs());
      while (true)
      {
        ObjRef<iface::cellml_api::ComponentRef> cr(cri->nextComponentRef());
        if (cr == NULL)
          break;
        recursivelyAddRelationshipsFrom(encap, cr);
      }
    }
  }
  
  CleanupList<iface::cellml_api::CellMLComponent*>::iterator i;
  for (i = mRelevantComponents.begin(); i != mRelevantComponents.end(); i++)
  {
    RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                       (*i)->variables());
    RETURN_INTO_OBJREF(cvi, iface::cellml_api::CellMLVariableIterator,
                       cvs->iterateVariables());
    while (true)
    {
      RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                         cvi->nextVariable());
      if (cv == NULL)
        break;
      VariableDisjointSet* vds = new VariableDisjointSet(cv);
      disjointSetElements.push_back(vds);
      varToDSMap.insert(std::pair<iface::cellml_api::CellMLVariable*,
                                  VariableDisjointSet*>(cv, vds));
    }
  }

  std::list<iface::cellml_api::Model*>::iterator rmi;
  for (rmi = aRelevantModels.begin();
       rmi != aRelevantModels.end();
       rmi++)
  {
    RETURN_INTO_OBJREF(comps,
                       iface::cellml_api::CellMLComponentSet,
                       (*rmi)->modelComponents());
    RETURN_INTO_OBJREF(conns, iface::cellml_api::ConnectionSet,
                       (*rmi)->connections());
    RETURN_INTO_OBJREF(conni, iface::cellml_api::ConnectionIterator,
                       conns->iterateConnections());
    while (true)
    {
      RETURN_INTO_OBJREF(conn, iface::cellml_api::Connection,
                         conni->nextConnection());
      if (conn == NULL)
        break;

      RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                         conn->componentMapping());
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

        ObjRef<iface::cellml_api::CellMLVariable> cv1, cv2;
        try
        {
          cv1 = already_AddRefd<iface::cellml_api::CellMLVariable>
            (mv->firstVariable());
        }
        catch (...)
        {
          std::wstring msg = L"Invalid first variable or component in "
            L"connection to component ";
          RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                             conn->componentMapping());
          RETURN_INTO_WSTRING(cname, mc->firstComponentName());
          msg += cname;
          msg += L", variable ";
          RETURN_INTO_WSTRING(vname, mv->firstVariableName());
          msg += vname;
          throw CeVASError(msg);
        }
        
        try
        {
          cv2 = already_AddRefd<iface::cellml_api::CellMLVariable>
            (mv->secondVariable());
        }
        catch (...)
        {
          std::wstring msg = L"Invalid second variable or component in "
            L"connection to component ";
          RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                             conn->componentMapping());
          RETURN_INTO_WSTRING(cname, mc->secondComponentName());
          msg += cname;
          msg += L", variable ";
          RETURN_INTO_WSTRING(vname, mv->secondVariableName());
          msg += vname;
          throw CeVASError(msg);
        }

        std::map<iface::cellml_api::CellMLVariable*, VariableDisjointSet*,
                 XPCOMComparator>::
          iterator vli1 = varToDSMap.find(cv1), vli2 = varToDSMap.find(cv2);
        if (vli1 == varToDSMap.end() || vli2 == varToDSMap.end())
          continue;

        ObjRef<iface::cellml_api::CellMLComponent> cvc1(comps->getComponent(mc->firstComponentName())),
          cvc2(comps->getComponent(mc->secondComponentName()));

        /*
        std::wcout << cvc1->name() << L", " << cvc2->name() << L" " <<
          mc->firstComponentName() << L", " << mc->secondComponentName() << L": " << L"1in2: "<< 
          (encap.count(std::pair<iface::cellml_api::CellMLComponent*,
                                 iface::cellml_api::CellMLComponent*>(cvc1, cvc2)) > 0)
                  << L" 2in1: " << 
          (encap.count(std::pair<iface::cellml_api::CellMLComponent*,
                                 iface::cellml_api::CellMLComponent*>(cvc2, cvc1)) > 0)
                  << std::endl;
        */

        (*vli1).second->merge
          (
           (*vli2).second, cv1, cv2,
           encap.count(std::pair<iface::cellml_api::CellMLComponent*,
                                 iface::cellml_api::CellMLComponent*>(cvc1, cvc2)) > 0,
           encap.count(std::pair<iface::cellml_api::CellMLComponent*,
                                 iface::cellml_api::CellMLComponent*>(cvc2, cvc1)) > 0
          );
      }
    }
  }

  AutoList<VariableDisjointSet*>::iterator dsei;
  for (dsei = disjointSetElements.begin(); dsei != disjointSetElements.end();
       dsei++)
  {
    VariableDisjointSet* vds = *dsei;
    VariableDisjointSet* root = vds->root();

    // Confirm that we have a 'source' variable...
    if (root->mSource == NULL)
    {
      RETURN_INTO_WSTRING(vname, root->mV->name());
      RETURN_INTO_WSTRING(vcomp, root->mV->componentName());

      std::wstring msg = L"CellML Variable ";
      msg += vname;
      msg += L" in component ";
      msg += vcomp;
      msg += L" has in interfaces, but is not connected (directly or "
             L"indirectly) to any variable with no in interfaces; ";
      throw CeVASError(msg);
    }

    // See if we have already started the set...
    maptype::iterator vsi = mVariableSets.find(root->mV);
    if (vsi != mVariableSets.end())
    {
      if (root != vds)
      {
        CDAConnectedVariableSet* cvs = (*vsi).second;
        mVariableSets.insert(std::pair<iface::cellml_api::CellMLVariable*,
                                       CDAConnectedVariableSet*>(vds->mV, cvs));
        cvs->addVariable(vds->mV);
      }
    }
    else
    {
      CDAConnectedVariableSet* cvs = new CDAConnectedVariableSet(root->mSource);
      mSetList.push_back(cvs);
      cvs->addVariable(vds->mV);
      mVariableSets.insert(std::pair<iface::cellml_api::CellMLVariable*,
                           CDAConnectedVariableSet*>(vds->mV, cvs));
      if (vds != root)
      {
        cvs->addVariable(root->mV);
        mVariableSets.insert(std::pair<iface::cellml_api::CellMLVariable*,
                             CDAConnectedVariableSet*>(root->mV, cvs));
      }
    }
  }
}

std::wstring
CDACeVAS::modelError()
  throw()
{
  return mErrorDescription;
}

class CDARelevantComponentIterator
  : public iface::cellml_api::CellMLComponentIterator
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator);

  CDARelevantComponentIterator(CDACeVAS* aListOwner,
                               std::list<iface::cellml_api::
                                        CellMLComponent*>& aList)
    : mListOwner(aListOwner), mRelevantComponents(aList)
  {
    mIt = mRelevantComponents.begin();
  }

  already_AddRefd<iface::cellml_api::CellMLComponent>
  nextComponent()
    throw(std::exception&)
  {
    if (mIt == mRelevantComponents.end())
      return NULL;

    iface::cellml_api::CellMLComponent* c = (*mIt);
    mIt++;

    c->add_ref();
    return c;
  }

  already_AddRefd<iface::cellml_api::CellMLElement>
  next()
    throw(std::exception&)
  {
    return static_cast<iface::cellml_api::CellMLComponent*>(nextComponent());
  }

private:
  ObjRef<CDACeVAS> mListOwner;
  std::list<iface::cellml_api::CellMLComponent*>& mRelevantComponents;
  std::list<iface::cellml_api::CellMLComponent*>::iterator mIt;
};

already_AddRefd<iface::cellml_api::CellMLComponentIterator>
CDACeVAS::iterateRelevantComponents()
  throw(std::exception&)
{
  return new CDARelevantComponentIterator(this, mRelevantComponents);
}

already_AddRefd<iface::cellml_services::ConnectedVariableSet>
CDACeVAS::findVariableSet
(
 iface::cellml_api::CellMLVariable* aVariable
)
  throw(std::exception&)
{
  if (aVariable == NULL)
    throw iface::cellml_api::CellMLException();
  maptype::iterator i = mVariableSets.find(aVariable);
  if (i == mVariableSets.end())
    return NULL;
  CDAConnectedVariableSet* ccvs = (*i).second;
  ccvs->add_ref();
  return ccvs;
}

uint32_t
CDACeVAS::length()
  throw(std::exception&)
{
  return mSetList.size();
}

already_AddRefd<iface::cellml_services::ConnectedVariableSet>
CDACeVAS::getVariableSet(uint32_t aIndex)
  throw(std::exception&)
{
  if (aIndex >= mSetList.size())
    return NULL;
  iface::cellml_services::ConnectedVariableSet* cvs = mSetList[aIndex];
  cvs->add_ref();
  return cvs;
}

already_AddRefd<iface::cellml_services::CeVASBootstrap>
CreateCeVASBootstrap(void)
{
  return new CDACeVASBootstrap();
}
