#include "CellMLContextImplementation.hxx"
#include "CellMLBootstrap.hpp"
#include <locale>
#include <sstream>

CDA_TypeAnnotationManager::~CDA_TypeAnnotationManager()
{
  std::map<std::pair<std::wstring,std::wstring>, iface::XPCOM::IObject*>
    ::iterator i;
  for (i = annotations.begin(); i != annotations.end(); i++)
  {
    (*i).second->release_ref();
  }
}

void
CDA_TypeAnnotationManager::setUserData(const wchar_t* type, const wchar_t* key,
                                       iface::XPCOM::IObject* data)
  throw(std::exception&)
{
  std::pair<std::wstring,std::wstring> p(type, key);
  std::map<std::pair<std::wstring,std::wstring>, iface::XPCOM::IObject*>
    ::iterator i = annotations.find(p);
  if (i != annotations.end())
  {
    // If we can't release the reference, just drop it...
    (*i).second->release_ref();
    annotations.erase(i);
  }
  if (data == NULL)
    return;

  data->add_ref();
  annotations.insert(std::pair<std::pair<std::wstring,std::wstring>,
                     iface::XPCOM::IObject*>(p, data));
}

iface::XPCOM::IObject*
CDA_TypeAnnotationManager::getUserData(const wchar_t* type, const wchar_t* key)
  throw(std::exception&)
{
  std::pair<std::wstring,std::wstring> p(type, key);
  std::map<std::pair<std::wstring,std::wstring>, iface::XPCOM::IObject*>
    ::iterator i = annotations.find(p);
  if (i == annotations.end())
    return NULL;

  (*i).second->add_ref();
  return (*i).second;
}

CDA_CellMLModuleIterator::CDA_CellMLModuleIterator
(
 CDA_ModuleManager* aMM,
 std::list<iface::cellml_context::CellMLModule*>& aList
)
  : _cda_refcount(1), mMM(aMM), mList(aList)
{
  mMM->add_ref();
  mCurrent = mList.begin();
}

CDA_CellMLModuleIterator::~CDA_CellMLModuleIterator()
{
  mMM->release_ref();
}

iface::cellml_context::CellMLModule*
CDA_CellMLModuleIterator::nextModule()
  throw(std::exception&)
{
  while (true)
  {
    if (mCurrent == mList.end())
      return NULL;
    iface::cellml_context::CellMLModule* ret = *mCurrent;
    ret->add_ref();
    mCurrent++;
    return ret;
  }
}

CDA_ModelNodeIterator::CDA_ModelNodeIterator
(
 CDA_ModelList* aML,
 std::list<CDA_ModelNode*>& aList
)
  : _cda_refcount(1), mML(aML), mList(aList)
{
  mML->add_ref();
  mCurrent = mList.begin();
}

CDA_ModelNodeIterator::~CDA_ModelNodeIterator()
{
  mML->release_ref();
}

iface::cellml_context::ModelNode*
CDA_ModelNodeIterator::nextModelNode()
  throw(std::exception&)
{
  if (mCurrent == mList.end())
    return NULL;
  iface::cellml_context::ModelNode* ret = *mCurrent;
  ret->add_ref();
  mCurrent++;
  return ret;
}

CDA_ModuleManager::CDA_ModuleManager()
  : _cda_refcount(1)
{
}

CDA_ModuleManager::~CDA_ModuleManager()
{
  std::list<iface::cellml_context::CellMLModuleMonitor*>::iterator i;
  for (i = mMonitors.begin(); i != mMonitors.end(); i++)
  {
    (*i)->release_ref();
  }

  std::list<iface::cellml_context::CellMLModule*>::iterator i2;
  for (i2 = mRegisteredModuleList.begin(); i2 != mRegisteredModuleList.end();
       i2++)
  {
    (*i2)->release_ref();
  }
}

void
CDA_ModuleManager::registerModule
(
 iface::cellml_context::CellMLModule* aModule
)
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(mn, aModule->moduleName());
  RETURN_INTO_WSTRING(mv, aModule->moduleVersion());

  std::pair<std::wstring,std::wstring> nvp(mn, mv);

  // Ignore if already registered(is this right?)...
  if (mRegisteredModules.find(nvp) != mRegisteredModules.end())
    return;

  // If add_ref fails, so does registerModule, which is the correct behaviour.
  aModule->add_ref();

  mRegisteredModules.insert(std::pair<std::pair<std::wstring,std::wstring>,
                            iface::cellml_context::CellMLModule*>
                            (nvp, aModule));
  mRegisteredModuleList.push_back(aModule);

  std::list<iface::cellml_context::CellMLModuleMonitor*>::iterator i, j;
  for (i = mMonitors.begin(); i != mMonitors.end();)
  {
    j = i;
    i++;
    try
    {
      (*j)->moduleRegistered(aModule);
    }
    catch (...)
    {
      (*j)->release_ref();
      mMonitors.erase(j);
    }
  }
}

void
CDA_ModuleManager::deregisterModule
(
 iface::cellml_context::CellMLModule* aModule
)
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(mn, aModule->moduleName());
  RETURN_INTO_WSTRING(mv, aModule->moduleVersion());

  std::map<std::pair<std::wstring,std::wstring>,
    iface::cellml_context::CellMLModule*>::iterator i;
  std::pair<std::wstring,std::wstring> nvp(mn, mv);

  i = mRegisteredModules.find(nvp);
  // Ignore if already registered(is this right?)...
  if (i == mRegisteredModules.end())
    return;

  std::list<iface::cellml_context::CellMLModuleMonitor*>::iterator i2, j2;
  for (i2 = mMonitors.begin(); i2 != mMonitors.end(); i2++)
  {
    j2 = i2;
    i2++;
    try
    {
      (*j2)->moduleDeregistered(aModule);
    }
    catch (...)
    {
      (*j2)->release_ref();
      mMonitors.erase(j2);
    }
  }

  std::list<iface::cellml_context::CellMLModule*>::iterator i3
    = std::find(mRegisteredModuleList.begin(), mRegisteredModuleList.end(),
                aModule);

  mRegisteredModules.erase(i);
  mRegisteredModuleList.erase(i3);
  // We might be removing a dead module, so ignore failures...
  (*i3)->release_ref();
}

iface::cellml_context::CellMLModule*
CDA_ModuleManager::findModuleByName
(
 const wchar_t* moduleName, const wchar_t* moduleVersion
)
  throw(std::exception&)
{
  std::map<std::pair<std::wstring,std::wstring>,
    iface::cellml_context::CellMLModule*>::iterator i;
  std::pair<std::wstring,std::wstring> nvp(moduleName, moduleVersion);

  i = mRegisteredModules.find(nvp);
  if (i == mRegisteredModules.end())
    return NULL;

  (*i).second->add_ref();
  return (*i).second;
}

void
CDA_ModuleManager::requestModuleByName
(
 const wchar_t* moduleName, const wchar_t* moduleVersion
)
  throw(std::exception&)
{
  // This module manager currently doesn't support lazy loading.
  throw iface::cellml_api::CellMLException();
}

void
CDA_ModuleManager::addMonitor
(
 iface::cellml_context::CellMLModuleMonitor* aModuleMonitor
)
  throw(std::exception&)
{
  // If add_ref fails, so does the call (correct behaviour).
  aModuleMonitor->add_ref();
  mMonitors.push_back(aModuleMonitor);
}

void
CDA_ModuleManager::removeMonitor
(
 iface::cellml_context::CellMLModuleMonitor* aModuleMonitor
)
  throw(std::exception&)
{
  std::list<iface::cellml_context::CellMLModuleMonitor*>::iterator i;
  for (i = mMonitors.begin(); i != mMonitors.end(); i++)
    if (CDA_objcmp((*i), aModuleMonitor) == 0)
    {
      aModuleMonitor->release_ref();
      mMonitors.erase(i);
      return;
    }
}

iface::cellml_context::CellMLModuleIterator*
CDA_ModuleManager::iterateModules()
  throw(std::exception&)
{
  return new CDA_CellMLModuleIterator(this, mRegisteredModuleList);
}

CDA_ModelNode::CDA_ModelNode(iface::cellml_api::Model* aModel)
  : _cda_refcount(1), mIsFrozen(false), mParentList(NULL)
{
  // If add_ref fails, so does this call (correct behaviour).
  aModel->add_ref();
  mDerivedModels = new CDA_ModelList();
  mDerivedModels->mParentNode = this;
  mModel = aModel;
  std::wstringstream wss;
  std::ostreambuf_iterator<wchar_t> osbi(wss);
  // XXX threadsafety (but localtime_r isn't portable).
  time_t t = time(0);
  struct tm* lttm = localtime(&t);
  const wchar_t* format = L"%H:%M:%S %Y-%m-%d";
  const wchar_t* format_end = format + wcslen(format);
  std::use_facet< std::time_put<wchar_t> >(wss.getloc())
    .put(osbi, wss, wss.fill(), lttm, format, format_end);
  mName = wss.str();
  stampModifiedNow();
}

CDA_ModelNode::~CDA_ModelNode()
{
  mDerivedModels->release_ref();
  mModel->release_ref();
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end(); i++)
  {
    (*i)->release_ref();
  }
}

void
CDA_ModelNode::name(const wchar_t* name)
  throw(std::exception&)
{
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, j;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end();)
  {
    j = i;
    i++;
    try
    {
      (*j)->modelRenamed(this, name);
    }
    catch (...)
    {
      // Dead listeners get removed from the list...
      (*j)->release_ref();
      mModelMonitors.erase(j);
    }
  }
  // Now inform the ancestor lists...
  CDA_ModelList* curList = mParentList;
  while (curList)
  {
    for (i = curList->mNodeMonitors.begin(); i != curList->mNodeMonitors.end();)
    {
      j = i;
      i++;
      try
      {
        (*j)->modelRenamed(this, name);
      }
      catch (...)
      {
        // Dead listeners get removed from the list...
        (*j)->release_ref();
        curList->mNodeMonitors.erase(j);
      }
    }
    if (curList->mParentNode)
      curList = curList->mParentNode->mParentList;
    else
      curList = NULL;
  }
  mName = name;
}

wchar_t*
CDA_ModelNode::name()
  throw(std::exception&)
{
  return wcsdup(mName.c_str());
}

iface::cellml_context::ModelNode*
CDA_ModelNode::getLatestDerivative()
  throw(std::exception&)
{
  ObjRef<iface::cellml_context::ModelNode> bestCandidate = this;
  uint32_t bestStamp = mTimestamp;

  // Recurse into children...
  ObjRef<iface::cellml_context::ModelNodeIterator> mni =
    already_AddRefd<iface::cellml_context::ModelNodeIterator>
    (mDerivedModels->iterateModelNodes());
  while (true)
  {
    ObjRef<iface::cellml_context::ModelNode> mn =
      already_AddRefd<iface::cellml_context::ModelNode>(mni->nextModelNode());
    if (mn == NULL)
      break;
    ObjRef<iface::cellml_context::ModelNode> mnd =
      already_AddRefd<iface::cellml_context::ModelNode>
      (mn->getLatestDerivative());
    uint32_t newStamp = mnd->modificationTimestamp();
    if (newStamp > bestStamp)
    {
      bestStamp = newStamp;
      bestCandidate = mnd;
    }
  }

  bestCandidate->add_ref();
  return bestCandidate;
}

iface::cellml_context::ModelNode*
CDA_ModelNode::getWritable()
  throw(std::exception&)
{
  if (!mIsFrozen)
  {
    add_ref();
    return this;
  }

  // We are frozen, so need to clone model & make it a derivative...

  // There is no clone, so we abuse getAlternateVersion...
  wchar_t* cv = mModel->cellmlVersion();
  ObjRef<iface::cellml_api::Model> modclone =
    already_AddRefd<iface::cellml_api::Model>(mModel->getAlternateVersion(cv));

  ObjRef<iface::cellml_context::ModelNode> cmn =
    already_AddRefd<iface::cellml_context::ModelNode>(new CDA_ModelNode
                                                      (modclone));

  mDerivedModels->addModel(cmn);

  cmn->add_ref();
  return cmn;
}

bool
CDA_ModelNode::isFrozen()
  throw(std::exception&)
{
  return mIsFrozen;
}

void
CDA_ModelNode::freeze()
  throw(std::exception&)
{
  mIsFrozen = true;
}

uint32_t
CDA_ModelNode::modificationTimestamp()
  throw(std::exception&)
{
  return mTimestamp;
}

void
CDA_ModelNode::stampModifiedNow()
  throw(std::exception&)
{
  mTimestamp = time(0);
}

iface::cellml_api::Model*
CDA_ModelNode::model()
  throw(std::exception&)
{
  mModel->add_ref();
  return mModel;
}

iface::cellml_context::ModelList*
CDA_ModelNode::derivedModels()
  throw(std::exception&)
{
  mDerivedModels->add_ref();
  return mDerivedModels;
}

void
CDA_ModelNode::addModelMonitor
(
 iface::cellml_context::ModelNodeMonitor* monitor
)
  throw(std::exception&)
{
  monitor->add_ref();
  mModelMonitors.push_back(monitor);
}

void
CDA_ModelNode::removeModelMonitor
(
 iface::cellml_context::ModelNodeMonitor* monitor
)
  throw(std::exception&)
{
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end(); i++)
  {
    (*i)->release_ref();
    mModelMonitors.erase(i);
  }
}

iface::cellml_context::ModelList*
CDA_ModelNode::parentList()
  throw(std::exception&)
{
  if (mParentList == NULL)
    return NULL;
  mParentList->add_ref();
  return mParentList;
}

CDA_ModelList::CDA_ModelList()
  : _cda_refcount(1), mParentNode(NULL)
{
}

CDA_ModelList::~CDA_ModelList()
{
  std::list<CDA_ModelNode*>::iterator i;
  for (i = mModels.begin(); i != mModels.end(); i++)
  {
    (*i)->mParentList = NULL;
    (*i)->release_ref();
  }
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i2;
  for (i2 = mNodeMonitors.begin(); i2 != mNodeMonitors.end(); i2++)
    (*i2)->release_ref();
  std::list<iface::cellml_context::ModelListMonitor*>::iterator i3;
  for (i3 = mListMonitors.begin(); i3 != mListMonitors.end(); i3++)
    (*i3)->release_ref();
}

void
CDA_ModelList::addModelMonitor
(
 iface::cellml_context::ModelNodeMonitor* monitor
)
  throw(std::exception&)
{
  monitor->add_ref();
  mNodeMonitors.push_back(monitor);
}

void
CDA_ModelList::removeModelMonitor
(
 iface::cellml_context::ModelNodeMonitor* monitor
)
  throw(std::exception&)
{
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i;
  for (i = mNodeMonitors.begin(); i != mNodeMonitors.end(); i++)
  {
    (*i)->release_ref();
    mNodeMonitors.erase(i);
  }
}

void
CDA_ModelList::addListMonitor
(
 iface::cellml_context::ModelListMonitor* monitor
)
  throw(std::exception&)
{
  monitor->add_ref();
  mListMonitors.push_back(monitor);
}

void
CDA_ModelList::removeListMonitor
(
 iface::cellml_context::ModelListMonitor* monitor
)
  throw(std::exception&)
{
  std::list<iface::cellml_context::ModelListMonitor*>::iterator i;
  for (i = mListMonitors.begin(); i != mListMonitors.end(); i++)
  {
    (*i)->release_ref();
    mListMonitors.erase(i);
  }
}

iface::cellml_context::ModelNode*
CDA_ModelList::makeNode(iface::cellml_api::Model* mod)
  throw(std::exception&)
{
  return new CDA_ModelNode(mod);
}

void
CDA_ModelList::addModel(iface::cellml_context::ModelNode* node)
  throw(std::exception&)
{
  // Convert node...
  CDA_ModelNode* cnode = dynamic_cast<CDA_ModelNode*>(node);
  if (cnode == NULL)
    throw iface::cellml_api::CellMLException();
  if (cnode->mParentList != NULL)
    throw iface::cellml_api::CellMLException();

  mModels.push_back(cnode);
  cnode->mParentList = this;

  // Call the monitors back...
  std::list<iface::cellml_context::ModelListMonitor*>::iterator i;
  CDA_ModelList* curList = this;
  uint16_t depth = 0;
  while (curList)
  {
    for (i = curList->mListMonitors.begin(); i != curList->mListMonitors.end();
         i++)
      (*i)->modelAdded(node, depth);
    depth++;
    if (curList->mParentNode)
      curList = curList->mParentNode->mParentList;
    else
      curList = NULL;
  }
}

void
CDA_ModelList::removeModel(iface::cellml_context::ModelNode* node)
  throw(std::exception&)
{
  // Call the monitors back...
  std::list<iface::cellml_context::ModelListMonitor*>::iterator i;
  CDA_ModelList* curList = this;
  uint16_t depth = 0;
  while (curList)
  {
    for (i = curList->mListMonitors.begin(); i != curList->mListMonitors.end();
         i++)
      (*i)->modelRemoved(node, depth);
    depth++;
    if (curList->mParentNode)
      curList = curList->mParentNode->mParentList;
    else
      curList = NULL;
  }

  std::list<CDA_ModelNode*>::iterator i2;
  for (i2 = mModels.begin(); i2 != mModels.end(); i2++)
  {
    if ((*i2) != node)
      continue;
    (*i2)->release_ref();
    (*i2)->mParentList = NULL;
    mModels.erase(i2);
  }
}

iface::cellml_context::ModelNodeIterator*
CDA_ModelList::iterateModelNodes()
  throw(std::exception&)
{
  return new CDA_ModelNodeIterator(this, mModels);
}

iface::cellml_context::ModelNode*
CDA_ModelList::parentNode()
  throw(std::exception&)
{
  if (mParentNode == NULL)
    return NULL;
  mParentNode->add_ref();
  return mParentNode;
}

CDA_CellMLContext::CDA_CellMLContext()
  : _cda_refcount(1)
{
  mModuleManager = new CDA_ModuleManager();
  mTypeAnnotationManager = new CDA_TypeAnnotationManager();
  mCellMLBootstrap = CreateCellMLBootstrap();
  mModelList = new CDA_ModelList();
}

CDA_CellMLContext::~CDA_CellMLContext()
{
  if (mModuleManager != NULL)
    mModuleManager->release_ref();
  if (mTypeAnnotationManager != NULL)
    mTypeAnnotationManager->release_ref();
  if (mCellMLBootstrap != NULL)
    mCellMLBootstrap->release_ref();
  if (mModelList != NULL)
    mModelList->release_ref();
}

iface::cellml_context::CellMLContext*
CreateCellMLContext()
{
  return new CDA_CellMLContext();
}
