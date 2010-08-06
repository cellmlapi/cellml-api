#define IN_CELLMLCONTEXT_MODULE
#include "CellMLContextImplementation.hxx"
#include "CellMLContextBootstrap.hxx"
#include "CellMLBootstrap.hpp"
#include <locale>
#include <sstream>
#include <algorithm>

// Win32 hack...
#ifdef _WIN32
#define swprintf _snwprintf
#endif

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
  mItIt = mMM->registerIterator(this);
}

CDA_CellMLModuleIterator::~CDA_CellMLModuleIterator()
{
  mMM->deregisterIterator(mItIt);
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
  mItIt = mML->registerIterator(this);
}

CDA_ModelNodeIterator::~CDA_ModelNodeIterator()
{
  mML->deregisterIterator(mItIt);
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

struct XPCOMEquator
{
  // Warning: does *not* add_ref ao2, instead you must keep reference.
  XPCOMEquator(iface::XPCOM::IObject* ao2)
  {
    o2 = ao2;
  }

  bool
  operator()(
             iface::XPCOM::IObject* o1
            ) const
  {
    /* NULL != x unless x == NULL. */
    if (o1 == NULL)
      return o2 ? false : true;
    char *id1, *id2;
    id1 = o1->objid();
    id2 = o2->objid();
    bool ret = (strcmp(id1, id2) == 0);
    free(id1);
    free(id2);
    return ret;
  }
  
private:
  iface::XPCOM::IObject* o2;
};

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

  XPCOMEquator xe(aModule);
  std::list<iface::cellml_context::CellMLModule*>::iterator i3
    = std::find_if(mRegisteredModuleList.begin(), mRegisteredModuleList.end(),
                   xe);
  if (i3 == mRegisteredModuleList.end())
    return;

  std::list<iface::cellml_context::CellMLModuleMonitor*>::iterator i2, j2;
  for (i2 = mMonitors.begin(); i2 != mMonitors.end();)
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

  std::list<CDA_CellMLModuleIterator*>::iterator i4;
  for (i4 = mIterators.begin(); i4 != mIterators.end(); i4++)
    (*i4)->invalidate(i3);

  mRegisteredModules.erase(i);
  (*i3)->release_ref();
  mRegisteredModuleList.erase(i3);
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
  : _cda_refcount(1), mIsFrozen(false), mParentList(NULL),
    mModelDirty(false)
{
  // Scoped locale change.
  CNumericLocale locobj;

  // If add_ref fails, so does this call (correct behaviour).
  aModel->add_ref();
  mDerivedModels = new CDA_ModelList();
  mDerivedModels->mParentNode = this;
  mModel = aModel;
  
  installModelEventListener(mModel);

  // XXX threadsafety (but localtime_r isn't portable).
  // It would be nice to use C++ standard library locales for this, but it
  // doesn't work in enough places (e.g. not in Wine).
  time_t t = time(0);
  struct tm* lttm = localtime(&t);
  wchar_t buf[20];
  swprintf(buf, 20, L"%02u:%02u:%02u %04u-%02u-%02u",
           lttm->tm_hour, lttm->tm_min, lttm->tm_sec,
           lttm->tm_year + 1900, lttm->tm_mon + 1, lttm->tm_mday);
  mName = buf;
  stampModifiedNow();
}

CDA_ModelNode::~CDA_ModelNode()
{
  // We need this or our reference count will go back to 1 and then down to 0
  // again in removeModelEventListener, triggering re-entrancy.
  add_ref();

  removeModelEventListener(mModel);

  mModel->release_ref();
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end(); i++)
  {
    (*i)->release_ref();
  }
  delete mDerivedModels;
}

void
CDA_ModelNode::add_ref()
  throw()
{
  _cda_refcount++;
  if (mParentList)
    mParentList->add_ref();
}

void
CDA_ModelNode::release_ref()
  throw()
{
  _cda_refcount--;
  if (mParentList)
    mParentList->release_ref();
  else if (_cda_refcount == 0)
    delete this;
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
  return CDA_wcsdup(mName.c_str());
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

  if (modclone == NULL)
    throw iface::cellml_api::CellMLException();

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
CDA_ModelNode::isFrozen(bool newState)
  throw(std::exception&)
{
  if (mIsFrozen == newState)
    return;
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, j;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end();)
  {
    j = i;
    i++;
    try
    {
      (*j)->modelFrozenStateChanged(this, newState);
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
        (*j)->modelFrozenStateChanged(this, newState);
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
  mIsFrozen = newState;
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

void
CDA_ModelNode::model(iface::cellml_api::Model* aModel)
  throw(std::exception&)
{
  if (aModel == NULL)
    throw iface::cellml_api::CellMLException();

  removeModelEventListener(mModel);
  installModelEventListener(aModel);

  // Now notify the change...
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, j;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end();)
  {
    j = i;
    i++;
    try
    {
      (*j)->modelReplaced(this, aModel);
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
        (*j)->modelReplaced(this, aModel);
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

  if (CDA_objcmp(mModel, aModel))
  {
    mModel->release_ref();
    mModel = aModel;
    mModel->add_ref();
  }
}

void
CDA_ModelNode::installModelEventListener(iface::cellml_api::Model* aModel)
{
  DECLARE_QUERY_INTERFACE_OBJREF(m, aModel, cellml_api::CellMLDOMElement);
  if (m == NULL)
    return;

  RETURN_INTO_OBJREF(de, iface::dom::Element, m->domElement());
  if (de == NULL)
    return;

  RETURN_INTO_OBJREF(doc, iface::dom::Document, de->ownerDocument());
  if (doc == NULL)
    return;

  DECLARE_QUERY_INTERFACE_OBJREF(et, doc, events::EventTarget);
  et->addEventListener(L"DOMSubtreeModified", this, false);

  // Workaround for reference cycle: set refcount down by one after adding the
  // event listener. This means we will get deleted even while the event
  // listener still exists. We bump our own refcount up by one before removing
  // the event listener.
  release_ref();
}

void
CDA_ModelNode::removeModelEventListener(iface::cellml_api::Model* aModel)
{
  DECLARE_QUERY_INTERFACE_OBJREF(m, aModel, cellml_api::CellMLDOMElement);
  if (m == NULL)
    return;

  RETURN_INTO_OBJREF(de, iface::dom::Element, m->domElement());
  if (de == NULL)
    return;

  RETURN_INTO_OBJREF(doc, iface::dom::Document, de->ownerDocument());
  if (doc == NULL)
    return;

  DECLARE_QUERY_INTERFACE_OBJREF(et, doc, events::EventTarget);

  // Workaround for reference cycle: set refcount down by one after adding the
  // event listener, bump it back up before removing...
  add_ref();
  et->removeEventListener(L"DOMSubtreeModified", this, false);
}

void
CDA_ModelNode::flushChanges()
  throw(std::exception&)
{
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, j;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end();)
  {
    j = i;
    i++;
    try
    {
      (*j)->changesFlushed(this);
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
        (*j)->changesFlushed(this);
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
}

iface::XPCOM::IObject*
CDA_ModelNode::owner()
  throw(std::exception&)
{
  if (mOwner)
    mOwner->add_ref();

  return mOwner;
}

void
CDA_ModelNode::owner(iface::XPCOM::IObject* aOwner)
  throw(std::exception&)
{
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, j;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end();)
  {
    j = i;
    i++;
    try
    {
      (*j)->ownerChanged(this, aOwner);
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
        (*j)->ownerChanged(this, aOwner);
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
  mOwner = aOwner;
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
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, i2;
  for (i = mModelMonitors.begin(); i != mModelMonitors.end();)
  {
    i2 = i;
    i++;
    (*i2)->release_ref();
    mModelMonitors.erase(i2);
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
    delete (*i);
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i2;
  for (i2 = mNodeMonitors.begin(); i2 != mNodeMonitors.end(); i2++)
    (*i2)->release_ref();
  std::list<iface::cellml_context::ModelListMonitor*>::iterator i3;
  for (i3 = mListMonitors.begin(); i3 != mListMonitors.end(); i3++)
    (*i3)->release_ref();
}

void
CDA_ModelList::add_ref()
  throw()
{
  _cda_refcount++;
  if (mParentNode)
    mParentNode->add_ref();
}

void
CDA_ModelList::release_ref()
  throw()
{
  _cda_refcount--;
  if (mParentNode)
    mParentNode->release_ref();
  else if (_cda_refcount == 0)
    delete this;
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
  std::list<iface::cellml_context::ModelNodeMonitor*>::iterator i, i2;
  for (i = mNodeMonitors.begin(); i != mNodeMonitors.end();)
  {
    i2 = i;
    i++;
    (*i2)->release_ref();
    mNodeMonitors.erase(i2);
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
  std::list<iface::cellml_context::ModelListMonitor*>::iterator i, i2;
  for (i = mListMonitors.begin(); i != mListMonitors.end();)
  {
    i2 = i;
    i++;
    (*i2)->release_ref();
    mListMonitors.erase(i2);
  }
}

iface::cellml_context::ModelNode*
CDA_ModelList::makeNode(iface::cellml_api::Model* mod)
  throw(std::exception&)
{
  if (mod == NULL)
    throw iface::cellml_api::CellMLException();
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

  cnode->setParentList(this);
  mModels.push_back(cnode);

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

  std::list<CDA_ModelNode*>::iterator i2, i3;
  for (i2 = mModels.begin(); i2 != mModels.end();)
  {
    i3 = i2;
    i2++;
    CDA_ModelNode* targ = *i3;
    if (targ != node)
      continue;
    targ->setParentList(NULL);

    std::list<CDA_ModelNodeIterator*>::iterator i4;
    for (i4 = mIterators.begin(); i4 != mIterators.end(); i4++)
      (*i4)->invalidate(i3);

    mModels.erase(i3);
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

void
CDA_ModelNode::setParentList(CDA_ModelList* aParentList)
{
  if (mParentList == aParentList)
    return;

  if (mParentList != NULL)
  {
    uint32_t i;
    for (i = 0; i < _cda_refcount; i++)
      mParentList->release_ref();
  }
  mParentList = aParentList;

  if (mParentList != NULL)
  {
    uint32_t i;
    for (i = 0; i < _cda_refcount; i++)
      mParentList->add_ref();
  }
  else if (_cda_refcount == 0)
    delete this;
}

void
CDA_ModelNode::dirty(bool aDirty)
  throw(std::exception&)
{
  mModelDirty = aDirty;
}

bool
CDA_ModelNode::dirty()
  throw(std::exception&)
{
  return mModelDirty;
}

void
CDA_ModelNode::handleEvent(iface::events::Event* aEvent)
  throw(std::exception&)
{
  mModelDirty = true;
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

CDA_EXPORT_PRE CDA_EXPORT_POST iface::cellml_context::CellMLContext*
CreateCellMLContext()
{
  return new CDA_CellMLContext();
}
