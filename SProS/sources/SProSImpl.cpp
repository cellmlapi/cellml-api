#define MODULE_CONTAINS_SProS
#include "SProSImpl.hxx"
#include "SProSBootstrap.hpp"
#include "DOMBootstrap.hxx"
#include <assert.h>
#include <string>
#include <algorithm>

#define SEDML_NS L"http://sed-ml.org/"

already_AddRefd<iface::SProS::Bootstrap>
CreateSProSBootstrap() throw()
{
  return new CDA_SProSBootstrap();
}

already_AddRefd<iface::SProS::SEDMLElement>
CDA_SProSBootstrap::createEmptySEDML()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(di, iface::dom::DOMImplementation, CreateDOMImplementation());
  RETURN_INTO_OBJREF(dt, iface::dom::DocumentType, di->createDocumentType(L"sedML", L"", L""));
  RETURN_INTO_OBJREF(doc, iface::dom::Document, di->createDocument(SEDML_NS, L"sedML", dt));
  RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
  return new CDA_SProSSEDMLElement(de);
}

already_AddRefd<iface::SProS::SEDMLElement>
CDA_SProSBootstrap::makeSEDMLFromElement(iface::dom::Element* el)
  throw()
{
  return new CDA_SProSSEDMLElement(el);
}

static already_AddRefd<CDA_SProSPrecomputedNodeList> allNodesFromNamedElements(iface::dom::Element* aEl, const std::wstring& aName)
{
  RETURN_INTO_OBJREF(pnl, CDA_SProSPrecomputedNodeList, new CDA_SProSPrecomputedNodeList());

  RETURN_INTO_OBJREF(nl, iface::dom::NodeList, aEl->childNodes());
  for (uint32_t i = 0, l = nl->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;

    RETURN_INTO_WSTRING(ns, el->namespaceURI());
    if (ns != SEDML_NS)
      continue;

    RETURN_INTO_WSTRING(ln, el->localName());
    if (ln != aName)
      continue;

    RETURN_INTO_OBJREF(nl2, iface::dom::NodeList, el->childNodes());
    for (uint32_t j = 0, m = nl2->length(); j < m; j++)
    {
      RETURN_INTO_OBJREF(n2, iface::dom::Node, nl2->item(j));
      pnl->add(n2);
    }
  }

  pnl->add_ref();
  return pnl.getPointer();
}

CDA_SProSPrecomputedNodeList::~CDA_SProSPrecomputedNodeList()
{
  for (std::vector<iface::dom::Node*>::iterator i = mNodeList.begin();
       i != mNodeList.end(); i++)
    (*i)->release_ref();
}

void
CDA_SProSPrecomputedNodeList::add(iface::dom::Node* aNode)
{
  aNode->add_ref();
  mNodeList.push_back(aNode);
}

already_AddRefd<iface::dom::Node>
CDA_SProSPrecomputedNodeList::item(uint32_t aIndex)
  throw(std::exception&)
{
  if (aIndex >= mNodeList.size())
    throw iface::dom::DOMException();

  iface::dom::Node* n = mNodeList[aIndex];
  n->add_ref();
  return n;
}

uint32_t
CDA_SProSPrecomputedNodeList::length() throw()
{
  return mNodeList.size();
}

CDA_SProSBase::CDA_SProSBase(CDA_SProSBase* aParent, iface::dom::Element* aEl)
  : mParent(NULL), mDomEl(aEl)
{
  assert(mDomEl != NULL);
  reparent(aParent);
}

CDA_SProSBase::~CDA_SProSBase()
{
}

void
CDA_SProSBase::add_ref() throw()
{
  ++_cda_refcount;
  if (mParent != NULL)
    mParent->add_ref();
}

void
CDA_SProSBase::release_ref() throw()
{
  assert(_cda_refcount > 0);

  if (mParent != NULL)
    mParent->release_ref();

  if ((!--_cda_refcount) && mParent == NULL)
    delete this;
}

void
CDA_SProSBase::reparent(CDA_SProSBase* aParent)
{
  uint32_t i;

  if (mParent == aParent)
    return;

  uint32_t rc = _cda_refcount;
  if (mParent != NULL)
    for (i = 0; i < rc; i++)
      mParent->release_ref();
  mParent = aParent;
  if (mParent != NULL)
    for (i = 0; i < rc; i++)
      mParent->add_ref();

  if (mParent == NULL && rc == 0)
    delete this;
}

already_AddRefd<iface::dom::Element>
CDA_SProSBase::domElement() throw()
{
  mDomEl->add_ref();
  return mDomEl.getPointer();
}

already_AddRefd<iface::dom::NodeList>
CDA_SProSBase::notes() throw()
{
  return allNodesFromNamedElements(mDomEl, L"notes");
}

already_AddRefd<iface::dom::NodeList>
CDA_SProSBase::annotations() throw()
{
  return allNodesFromNamedElements(mDomEl, L"annotations");
}

already_AddRefd<iface::SProS::Base>
CDA_SProSBase::parent() throw()
{
  if (mParent == NULL)
    return NULL;

  mParent->add_ref();
  return mParent;
}

CDA_SomeSet::CDA_SomeSet(CDA_SProSBase* aParent,
                         const wchar_t* aListName,
                         const wchar_t** aElNames)
  : mListName(aListName), mElNames(aElNames), mParent(aParent)
{
}

CDA_SomeSet::~CDA_SomeSet()
{
  emptyElementCache();
}

void
CDA_SomeSet::add_ref() throw()
{
  mParent->add_ref();
}

void
CDA_SomeSet::release_ref() throw()
{
  mParent->release_ref();
}

void
CDA_SomeSet::insert(iface::SProS::Base* b)
  throw(std::exception&)
{
  CDA_SProSBase* sb = dynamic_cast<CDA_SProSBase*>(b);
  if (sb == NULL)
    throw iface::SProS::SProSException();

  findOrCreateListElement();

  RETURN_INTO_WSTRING(ln, sb->mDomEl->localName());
  if (!checkLocalNameMatch(ln.c_str()))
    throw iface::SProS::SProSException();

  try
  {
    mListElement->appendChild(sb->mDomEl)->release_ref();
  }
  catch (...)
  {
    throw iface::SProS::SProSException();
  }

  sb->reparent(mParent);

  std::string ids(sb->mDomEl->objid());

  cache(ids, sb);
}

void
CDA_SomeSet::remove(iface::SProS::Base* b)
  throw(std::exception&)
{
  CDA_SProSBase* sb = dynamic_cast<CDA_SProSBase*>(b);
  if (sb == NULL)
    throw iface::SProS::SProSException();

  if (!tryFindListElement())
    throw iface::SProS::SProSException();

  decache(sb);

  mListElement->removeChild(sb->mDomEl)->release_ref();
}

already_AddRefd<iface::SProS::BaseIterator>
CDA_SomeSet::iterateElements() throw()
{
  findOrCreateListElement();
  return new CDA_SProSBaseIterator(this);
}

void
CDA_SomeSet::cache(const std::string& ids, CDA_SProSBase* aSB)
{
  std::map<std::string, CDA_SProSBase*>::iterator i =
    mElCache.find(ids);
  if (i == mElCache.end())
  {
    mElCache.insert(std::pair<std::string, CDA_SProSBase*>(ids, aSB));
  }
  else
  {
    (*i).second = aSB;
  }
}

void
CDA_SomeSet::decache(CDA_SProSBase* aSB)
{
  std::string ids = aSB->mDomEl->objid();

  std::map<std::string, CDA_SProSBase*>::iterator i =
    mElCache.find(ids);

  if (i != mElCache.end())
  {
    (*i).second->reparent(NULL);
    mElCache.erase(i);
  }
}

void
CDA_SomeSet::emptyElementCache()
{
  for (std::map<std::string, CDA_SProSBase*>::iterator i = mElCache.begin();
       i != mElCache.end(); i++)
    (*i).second->reparent(NULL);
  mElCache.clear();
}

void
CDA_SomeSet::checkListParentAndInvalidateIfWrong()
{
  if (mListElement == NULL)
    return;

  RETURN_INTO_OBJREF(p, iface::dom::Node,
                     mListElement->parentNode());
  if (CDA_objcmp(p, mParent->mDomEl))
  {
    emptyElementCache();
    mListElement = NULL;
  }
}

iface::dom::Element*
CDA_SomeSet::tryFindListElement()
{
  checkListParentAndInvalidateIfWrong();
  if (mListElement != NULL)
    return mListElement;

  RETURN_INTO_OBJREF(n, iface::dom::Node, mParent->mDomEl->firstChild());
  while (n != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el != NULL)
    {
      RETURN_INTO_WSTRING(ns, el->namespaceURI());
      if (ns == SEDML_NS)
      {
        RETURN_INTO_WSTRING(ln, el->localName());
        if (ln == mListName)
        {
          mListElement = el;
          return mListElement;
        }
      }
    }

    n = already_AddRefd<iface::dom::Node>(n->nextSibling());
  }
  return NULL;
}

iface::dom::Element*
CDA_SomeSet::findOrCreateListElement()
{
  iface::dom::Element* el = tryFindListElement();
  if (el != NULL)
    return el;
  
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     mParent->mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(newel, iface::dom::Element,
                     doc->createElementNS(SEDML_NS, mListName));
  mParent->mDomEl->appendChild(newel)->release_ref();
  mListElement = newel;

  return mListElement;
}

struct BaseElementConstructors
{
  const wchar_t* localNameMatch;
  already_AddRefd<CDA_SProSBase> (*construct)(CDA_SProSBase* aParent, iface::dom::Element* aEl);
};

bool
operator<(const BaseElementConstructors& aEl1,
          const BaseElementConstructors& aEl2)
{
  return (wcscmp(aEl1.localNameMatch, aEl2.localNameMatch) < 0);
}

static already_AddRefd<CDA_SProSBase>
makeaddXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSAddXML(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makechangeAttribute(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSChangeAttribute(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makechangeXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSChangeXML(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makecomputeChange(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSComputeChange(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makecurve(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSCurve(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makedataGenerator(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSDataGenerator(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makedataSet(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSDataSet(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makemodel(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSModel(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makeparameter(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSParameter(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makeplot2D(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSPlot2D(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makeplot3D(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSPlot3D(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makeremoveXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSRemoveXML(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makereport(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSReport(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makesedML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSSEDMLElement(aEl);
}

static already_AddRefd<CDA_SProSBase>
makesurface(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSSurface(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
maketask(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSTask(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makeuniformTimeCourse(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSUniformTimeCourse(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makesamplingSensitivityAnalysis(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSSamplingSensitivityAnalysis(aParent, aEl);
}

static already_AddRefd<CDA_SProSBase>
makevariable(CDA_SProSBase* aParent, iface::dom::Element* aEl)
{
  return new CDA_SProSVariable(aParent, aEl);
}


// Assumed to be sorted.
static BaseElementConstructors sBaseConstructors[] = {
  {L"addXML", makeaddXML},
  {L"changeAttribute", makechangeAttribute},
  {L"changeXML", makechangeXML},
  {L"computeChange", makecomputeChange},
  {L"curve", makecurve},
  {L"dataGenerator", makedataGenerator},
  {L"dataSet", makedataSet},
  {L"model", makemodel},
  {L"parameter", makeparameter},
  {L"plot2D", makeplot2D},
  {L"plot3D", makeplot3D},
  {L"removeXML", makeremoveXML},
  {L"report", makereport},
  {L"samplingSensitivityAnalysis", makesamplingSensitivityAnalysis},
  {L"sedML", makesedML},
  {L"surface", makesurface},
  {L"task", maketask},
  {L"uniformTimeCourse", makeuniformTimeCourse},
  {L"variable", makevariable}
};

already_AddRefd<CDA_SProSBase>
CDA_SomeSet::wrapOrFindElement(CDA_SProSBase* aBase, iface::dom::Element* aEl)
{
  // Is it cached?
  std::string ids(aEl->objid());

  std::map<std::string, CDA_SProSBase*>::iterator i = 
    mElCache.find(ids);
  if (i != mElCache.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  RETURN_INTO_WSTRING(ns, aEl->namespaceURI());
  if (ns != SEDML_NS)
    return NULL;

  RETURN_INTO_WSTRING(ln, aEl->localName());

  BaseElementConstructors findBy;
  findBy.localNameMatch = ln.c_str();
  BaseElementConstructors* bec =
    std::lower_bound(sBaseConstructors,
                     sBaseConstructors + sizeof(sBaseConstructors)/sizeof(sBaseConstructors[0]),
                     findBy);
  if (bec == sBaseConstructors + sizeof(sBaseConstructors)/sizeof(sBaseConstructors[0]))
    return NULL;
  if (wcscmp(bec->localNameMatch, findBy.localNameMatch))
    return NULL;

  CDA_SProSBase* wrapped = bec->construct(aBase, aEl);
  cache(ids, wrapped);
  return wrapped;
}

bool
CDA_SomeSet::checkLocalNameMatch(const std::wstring& aCheck)
{
  const wchar_t* against = aCheck.c_str();
  for (const wchar_t** p = mElNames; *p != NULL; p++)
    if (!wcscmp(against, *p))
      return true;

  return false;
}

CDA_SProSDOMIteratorBase::CDA_SProSDOMIteratorBase
(
 iface::dom::Element* parentElement
)
  : mParentElement(parentElement),
    icml(this)
{
  mNodeList = mParentElement->childNodes();
  DECLARE_QUERY_INTERFACE_OBJREF(targ, mParentElement, events::EventTarget);
  targ->addEventListener(L"DOMNodeInserted", &icml, false);
}

CDA_SProSDOMIteratorBase::~CDA_SProSDOMIteratorBase()
{
  mNodeList->release_ref();

  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mParentElement, events::EventTarget);
    targ->removeEventListener(L"DOMNodeInserted", &icml, false);
  }

  if (mNextElement != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mNextElement, events::EventTarget);
    targ->removeEventListener(L"DOMNodeRemoved", &icml, false);
  }
  if (mPrevElement != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrevElement, events::EventTarget);
    targ->removeEventListener(L"DOMNodeRemoved", &icml, false);
  }
}

already_AddRefd<iface::dom::Element>
CDA_SProSDOMIteratorBase::fetchNextElement()
{
    if (mPrevElement == NULL)
    {
      // Search for the first element...
      uint32_t i;
      uint32_t l = mNodeList->length();
      for (i = 0; i < l; i++)
      {
        RETURN_INTO_OBJREF(nodeHit, iface::dom::Node, mNodeList->item(i));
        QUERY_INTERFACE(mPrevElement, nodeHit, dom::Element);
        if (mPrevElement != NULL)
        {
          DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrevElement, events::EventTarget);
          targ->addEventListener(L"DOMNodeRemoved", &icml, false);
          break;
        }
      }
      if (mPrevElement == NULL)
      {
        return NULL;
      }
    }
    else
    {
      // Once mNextElement is NULL, we are at the end until more elements are
      // inserted.
      if (mNextElement == NULL)
      {
        return NULL;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrevElement, events::EventTarget);
      targ->removeEventListener(L"DOMNodeRemoved", &icml, false);
      mPrevElement = mNextElement;
      mNextElement = NULL;
    }

    // We now have a valid previous element, which will be our return value.
    // However, to maintain our assumptions, we need to find mNextElement.
    RETURN_INTO_OBJREF(nodeHit, iface::dom::Node, mPrevElement->nextSibling());
    while (nodeHit != NULL)
    {
      QUERY_INTERFACE(mNextElement, nodeHit, dom::Element);
      if (mNextElement != NULL)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(targ, mNextElement, events::EventTarget);
        targ->addEventListener(L"DOMNodeRemoved", &icml, false);
        break;
      }
      nodeHit = already_AddRefd<iface::dom::Node>(nodeHit->nextSibling());
    }
    
    mPrevElement->add_ref();
    return mPrevElement.getPointer();
}

already_AddRefd<iface::dom::Element>
CDA_SProSDOMIteratorBase::fetchNextElement(const std::wstring& aWantEl)
{
    if (mPrevElement == NULL)
    {
      // Search for the first element...
      uint32_t i;
      uint32_t l = mNodeList->length();
      for (i = 0; i < l; i++)
      {
        RETURN_INTO_OBJREF(nodeHit, iface::dom::Node, mNodeList->item(i));
        std::wstring ln = nodeHit->localName();
        if (ln != aWantEl)
          continue;

        QUERY_INTERFACE(mPrevElement, nodeHit, dom::Element);
        if (mPrevElement != NULL)
        {
          DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrevElement, events::EventTarget);
          targ->addEventListener(L"DOMNodeRemoved", &icml, false);
          break;
        }
      }
      if (mPrevElement == NULL)
      {
        return NULL;
      }
    }
    else
    {
      // Once mNextElement is NULL, we are at the end until more elements are
      // inserted.
      if (mNextElement == NULL)
      {
        return NULL;
      }
      DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrevElement, events::EventTarget);
      targ->removeEventListener(L"DOMNodeRemoved", &icml, false);
      mPrevElement = mNextElement;
      mNextElement = NULL;
    }

    // We now have a valid previous element, which will be our return value.
    // However, to maintain our assumptions, we need to find mNextElement.
    RETURN_INTO_OBJREF(nodeHit, iface::dom::Node, mPrevElement->nextSibling());

    while (nodeHit != NULL)
    {
      std::wstring elN(nodeHit->localName());
      if (elN == aWantEl)
      {
        QUERY_INTERFACE(mNextElement, nodeHit, dom::Element);
        if (mNextElement != NULL)
        {
          DECLARE_QUERY_INTERFACE_OBJREF(targ, mNextElement,
                                         events::EventTarget);
          targ->addEventListener(L"DOMNodeRemoved", &icml, false);
          break;
        }
      }
      nodeHit = already_AddRefd<iface::dom::Node>(nodeHit->nextSibling());
    }

    mPrevElement->add_ref();
    return mPrevElement.getPointer();
}

void
CDA_SProSDOMIteratorBase::IteratorChildrenModificationListener::
handleEvent(iface::events::Event* evt)
  throw(std::exception&)
{
    bool isRemoval = false, isInsertion = false;
    std::wstring et = evt->type();
    if (et == L"DOMNodeRemoved")
      isRemoval = true;
    else if (et == L"DOMNodeInserted")
      isInsertion = true;
    
    // We only care about insertions and removals...
    if (!isRemoval && !isInsertion)
      return;
    
    if (isRemoval)
    {
      if (evt->eventPhase() != iface::events::Event::AT_TARGET)
        return;

      RETURN_INTO_OBJREF(targetET, iface::events::EventTarget,
                         evt->target());
      DECLARE_QUERY_INTERFACE_OBJREF(target, targetET, dom::Node);
      int cmp1;
      if (mIterator->mPrevElement == NULL)
        cmp1 = 1;
      else
        cmp1 = CDA_objcmp(target, mIterator->mPrevElement);

      int cmp2;
      if (mIterator->mNextElement == NULL)
        cmp2 = 1;
      else
        cmp2 = CDA_objcmp(target, mIterator->mNextElement);

      if (cmp1 && cmp2)
      {
        // printf("Warning: Unexpected event sent to handler.\n");
        return;
      }

      if (cmp1 == 0)
      {
        // The previous node is about to be removed. Advance to an earlier
        // previous...
        DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mPrevElement,
                                       events::EventTarget);
        targ->removeEventListener(L"DOMNodeRemoved", this, false);
        RETURN_INTO_OBJREF(nodeHit, iface::dom::Node,
                           mIterator->mPrevElement->previousSibling());
        mIterator->mPrevElement = NULL;
        while (true)
        {
          if (nodeHit == NULL)
          {
            // If we just deleted the first element, reset to the initial
            // iterator state...
            if (mIterator->mNextElement != NULL)
            {
              DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mNextElement,
                                             events::EventTarget);
              targ->removeEventListener(L"DOMNodeRemoved", this, false);
            }
            mIterator->mNextElement = NULL;
            return;
          }
          QUERY_INTERFACE(mIterator->mPrevElement, nodeHit, dom::Element);
          if (mIterator->mPrevElement != NULL)
          {
            DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mPrevElement, events::EventTarget);
            targ->addEventListener(L"DOMNodeRemoved", this, false);
            break;
          }
          nodeHit = already_AddRefd<iface::dom::Node>(nodeHit->previousSibling());
        }
      }
      else
      {
        DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mNextElement,
                                       events::EventTarget);
        targ->removeEventListener(L"DOMNodeRemoved", this, false);
        // The next node is about to be removed. Advance to a later next...
        RETURN_INTO_OBJREF(nodeHit, iface::dom::Node,
                           mIterator->mNextElement->nextSibling());
        mIterator->mNextElement = NULL;

        while (true)
        {
          if (nodeHit == NULL)
          {
            // If we just deleted the last element, we are done.
            return;
          }
          QUERY_INTERFACE(mIterator->mNextElement, nodeHit, dom::Element);
          if (mIterator->mNextElement != NULL)
          {
            DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mNextElement,
                                           events::EventTarget);
            targ->addEventListener(L"DOMNodeRemoved", this, false);
            break;
          }
          nodeHit = already_AddRefd<iface::dom::Node>(nodeHit->nextSibling());
        }
      }
    }
    else if (isInsertion)
    {
      // Convert to a mutation event...
      DECLARE_QUERY_INTERFACE_OBJREF(mevt, evt, events::MutationEvent);

      RETURN_INTO_OBJREF(rn, iface::dom::Node, mevt->relatedNode());
      if (dynamic_cast<void*>(rn.getPointer()) !=
          dynamic_cast<void*>(static_cast<iface::dom::Element*>(mIterator->mParentElement)))
        return;
      
      RETURN_INTO_OBJREF(tn, iface::events::EventTarget, mevt->target());
      DECLARE_QUERY_INTERFACE_OBJREF(te, tn, dom::Element);
      
      if (te == NULL)
        return;

      if (mIterator->mPrevElement == NULL)
      {
        // The iterator is sitting at the beginning. In this case, we don't
        // need to do anything now, as the first element will be found later.
        return;
      }

      ObjRef<iface::dom::Element> curE;
      if (mIterator->mNextElement)
      {
        // See if target lies between mPrevElement and mNextElement...
        RETURN_INTO_OBJREF(curN, iface::dom::Node,
                           mIterator->mPrevElement->nextSibling());
        QUERY_INTERFACE(curE, curN, dom::Element);
        while (
               (curN && !curE) || /* Skip nodes that aren't elements... */
               (curE && ( /* If curN and curE are both NULL, we are done. */
                         /* If we reached the end before the target, it is out of
                          * range. */
                         CDA_objcmp(curE, mIterator->mNextElement) && 
                         /* If we reached the target first,
                          * it is in range.*/
                         CDA_objcmp(curE, te)
                        )
               )
              )
        {
          curN = already_AddRefd<iface::dom::Node>(curN->nextSibling());
          QUERY_INTERFACE(curE, curN, dom::Element);
        }

        // If the target is not in the relevant range, just return...
        if (curE == mIterator->mNextElement)
          return;

        //if (curE == NULL)
        //  printf("Something is wrong: we got from the previous node to the "
        //         "last node without passing the next node, but the next node "
        //         "is non-null!\n");
      }
      else
      {
        // The iterator was finished, but we might now have one more element...
        RETURN_INTO_OBJREF(curN, iface::dom::Node,
                           mIterator->mPrevElement->nextSibling());
        QUERY_INTERFACE(curE, curN, dom::Element);
        while (
               (curN && !curE) /* Skip nodes that aren't elements... */
              )
        {
          curN = already_AddRefd<iface::dom::Node>(curN->nextSibling());
          QUERY_INTERFACE(curE, curN, dom::Element);
        }
        
        if (curE == NULL)
          return;
      }

      // The current next element is no longer next...
      if (mIterator->mNextElement)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mNextElement,
                                       events::EventTarget);
        targ->removeEventListener(L"DOMNodeRemoved", this, false);
      }
      mIterator->mNextElement = curE;
      DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mNextElement,
                                     events::EventTarget);
      targ->addEventListener(L"DOMNodeRemoved", this, false);
    }
}


CDA_SProSIteratorBase::CDA_SProSIteratorBase(CDA_SomeSet* aSet)
  : CDA_SProSDOMIteratorBase(aSet->mListElement), mSet(aSet)
{
}

CDA_SProSIteratorBase::~CDA_SProSIteratorBase()
{
}

already_AddRefd<iface::SProS::Base>
CDA_SProSIteratorBase::nextElement() throw()
{
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::dom::Element,
                       fetchNextElement());
    if (el == NULL)
      return NULL;

    RETURN_INTO_WSTRING(ns, el->namespaceURI());
    if (ns == SEDML_NS)
    {
      RETURN_INTO_WSTRING(ln, el->localName());
      if (mSet->checkLocalNameMatch(ln.c_str()))
        return mSet->wrapOrFindElement(mSet->mParent, el);
    }
  }
}

uint32_t
CDA_SProSSEDMLElement::level()
  throw()
{
  RETURN_INTO_WSTRING(levels, mDomEl->getAttribute(L"level"));
  if (levels == L"")
    return 1;
  return wcstoul(levels.c_str(), NULL, 10);
}

void
CDA_SProSSEDMLElement::level(uint32_t aLevel)
  throw()
{
  wchar_t buf[64];
  any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%d", aLevel);
  mDomEl->setAttribute(L"level", buf);
}

uint32_t
CDA_SProSSEDMLElement::version()
  throw()
{
  RETURN_INTO_WSTRING(versions, mDomEl->getAttribute(L"version"));
  if (versions == L"")
    return 1;

  return wcstoul(versions.c_str(), NULL, 10);
}

void
CDA_SProSSEDMLElement::version(uint32_t aVersion)
  throw()
{
  wchar_t buf[64];
  any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%d", aVersion);
  mDomEl->setAttribute(L"version", buf);
}

already_AddRefd<iface::SProS::ModelSet>
CDA_SProSSEDMLElement::models()
  throw()
{
  add_ref();
  return &mModelSet;
}

already_AddRefd<iface::SProS::TaskSet>
CDA_SProSSEDMLElement::tasks()
  throw()
{
  add_ref();
  return &mTaskSet;
}

already_AddRefd<iface::SProS::SimulationSet>
CDA_SProSSEDMLElement::simulations()
  throw()
{
  add_ref();
  return &mSimulationSet;
}

already_AddRefd<iface::SProS::DataGeneratorSet>
CDA_SProSSEDMLElement::generators()
  throw()
{
  add_ref();
  return &mDataGeneneratorSet;
}

already_AddRefd<iface::SProS::OutputSet>
CDA_SProSSEDMLElement::outputs()
  throw()
{
  add_ref();
  return &mOutputSet;
}

already_AddRefd<iface::SProS::Model>
CDA_SProSSEDMLElement::createModel()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"model"));

  return new CDA_SProSModel(NULL, el);
}

already_AddRefd<iface::SProS::UniformTimeCourse>
CDA_SProSSEDMLElement::createUniformTimeCourse()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"uniformTimeCourse"));

  return new CDA_SProSUniformTimeCourse(NULL, el);
}

already_AddRefd<iface::SProS::SamplingSensitivityAnalysis>
CDA_SProSSEDMLElement::createSamplingSensitivityAnalysis()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"samplingSensitivityAnalysis"));

  return new CDA_SProSSamplingSensitivityAnalysis(NULL, el);
}

already_AddRefd<iface::SProS::Task>
CDA_SProSSEDMLElement::createTask()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"task"));

  return new CDA_SProSTask(NULL, el);
}

already_AddRefd<iface::SProS::DataGenerator>
CDA_SProSSEDMLElement::createDataGenerator()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"dataGenerator"));

  return new CDA_SProSDataGenerator(NULL, el);
}

already_AddRefd<iface::SProS::Plot2D>
CDA_SProSSEDMLElement::createPlot2D()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"plot2D"));

  return new CDA_SProSPlot2D(NULL, el);
}

already_AddRefd<iface::SProS::Plot3D>
CDA_SProSSEDMLElement::createPlot3D()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"plot3D"));

  return new CDA_SProSPlot3D(NULL, el);
}

already_AddRefd<iface::SProS::Report>
CDA_SProSSEDMLElement::createReport()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"report"));

  return new CDA_SProSReport(NULL, el);
}

already_AddRefd<iface::SProS::ComputeChange>
CDA_SProSSEDMLElement::createComputeChange()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"computeChange"));

  return new CDA_SProSComputeChange(NULL, el);
}

already_AddRefd<iface::SProS::ChangeAttribute>
CDA_SProSSEDMLElement::createChangeAttribute()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"changeAttribute"));

  return new CDA_SProSChangeAttribute(NULL, el);
}

already_AddRefd<iface::SProS::AddXML>
CDA_SProSSEDMLElement::createAddXML()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"addXML"));

  return new CDA_SProSAddXML(NULL, el);
}

already_AddRefd<iface::SProS::RemoveXML>
CDA_SProSSEDMLElement::createRemoveXML()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"removeXML"));

  return new CDA_SProSRemoveXML(NULL, el);
}

already_AddRefd<iface::SProS::ChangeXML>
CDA_SProSSEDMLElement::createChangeXML()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"changeXML"));

  return new CDA_SProSChangeXML(NULL, el);
}

already_AddRefd<iface::SProS::Variable>
CDA_SProSSEDMLElement::createVariable()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"variable"));

  return new CDA_SProSVariable(NULL, el);
}

already_AddRefd<iface::SProS::Parameter>
CDA_SProSSEDMLElement::createParameter()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"parameter"));

  return new CDA_SProSParameter(NULL, el);
}

already_AddRefd<iface::SProS::Curve>
CDA_SProSSEDMLElement::createCurve()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"curve"));

  return new CDA_SProSCurve(NULL, el);
}

already_AddRefd<iface::SProS::Surface>
CDA_SProSSEDMLElement::createSurface()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"surface"));

  return new CDA_SProSSurface(NULL, el);
}

already_AddRefd<iface::SProS::DataSet>
CDA_SProSSEDMLElement::createDataSet()
  throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, mDomEl->ownerDocument());
  RETURN_INTO_OBJREF(el, iface::dom::Element, doc->createElementNS(SEDML_NS, L"dataSet"));

  return new CDA_SProSDataSet(NULL, el);
}

std::wstring CDA_SProSSEDMLElement::originalURL()
  throw()
{
  return mOriginalURL;
}

void
CDA_SProSSEDMLElement::originalURL(const std::wstring& aURL)
  throw()
{
  mOriginalURL = aURL;
}

already_AddRefd<iface::SProS::NamedElementIterator>
CDA_SProSNamedElementSet::iterateNamedElement()
  throw()
{
  findOrCreateListElement();
  return new CDA_SProSNamedElementIterator(this);
}

already_AddRefd<iface::SProS::NamedIdentifiedElementIterator>
CDA_SProSNamedIdentifiedElementSet::iterateNamedIdentifiedElements() throw()
{
  findOrCreateListElement();
  return new CDA_SProSNamedIdentifiedElementIterator(this);
}

already_AddRefd<iface::SProS::NamedIdentifiedElement>
CDA_SProSNamedIdentifiedElementSet::getNamedIdentifiedElementByIdentifier(const std::wstring& aIdMatch)
  throw()
{
  RETURN_INTO_OBJREF(it, iface::SProS::NamedIdentifiedElementIterator,
                     iterateNamedIdentifiedElements());
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::SProS::NamedIdentifiedElement,
                       it->nextNamedIdentifiedElement());
    if (el == NULL)
      return NULL;

    if (el->id() == aIdMatch)
    {
      el->add_ref();
      return el.getPointer();
    }
  }
}

std::wstring CDA_SProSModel::language()
  throw()
{
  return mDomEl->getAttribute(L"language");
}

void CDA_SProSModel::language(const std::wstring& aLang)
  throw()
{
  return mDomEl->setAttribute(L"language", aLang);
}

std::wstring CDA_SProSModel::source()
  throw()
{
  return mDomEl->getAttribute(L"source");
}

void CDA_SProSModel::source(const std::wstring& aSource)
  throw()
{
  return mDomEl->setAttribute(L"source", aSource);
}

already_AddRefd<iface::SProS::ChangeSet>
CDA_SProSModel::changes()
  throw()
{
  add_ref();
  return &mChangeSet;
}

#undef SomeSProSSet
#define SomeSProSSet(whatUpper, elName, elList) \
  const wchar_t* s##whatUpper##ElNames[] = {elList, NULL };         \
  CDA_SProS##whatUpper##Set::CDA_SProS##whatUpper##Set(CDA_SProSBase* aParent) : CDA_SProSNamedIdentifiedElementSet(aParent, elName, s##whatUpper##ElNames) {} \
already_AddRefd<iface::SProS::whatUpper##Iterator> \
CDA_SProS##whatUpper##Set::iterate##whatUpper##s() throw() \
{ \
  findOrCreateListElement(); \
  return new CDA_SProS##whatUpper##Iterator(this); \
}

#undef SomeAnonSProSSet
#define SomeAnonSProSSet(whatUpper, elName, elList) \
  const wchar_t* s##whatUpper##ElNames[] = {elList, NULL}; \
  CDA_SProS##whatUpper##Set::CDA_SProS##whatUpper##Set(CDA_SProSBase* aParent) : CDA_SomeSet(aParent, elName, s##whatUpper##ElNames) {} \
already_AddRefd<iface::SProS::whatUpper##Iterator>                    \
CDA_SProS##whatUpper##Set::iterate##whatUpper##s() throw() \
{ \
  findOrCreateListElement();                       \
  return new CDA_SProS##whatUpper##Iterator(this); \
}

SomeSProSSet(Model, L"listOfModels", L"model");

std::wstring
CDA_SProSSimulation::algorithmKisaoID() throw()
{
  RETURN_INTO_OBJREF(n, iface::dom::Node, mDomEl->firstChild());
  while (true)
  {
    if (n == NULL)
      return L"";
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el != NULL)
    {
      RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
      if (nsURI == SEDML_NS)
      {
        RETURN_INTO_WSTRING(ln, el->localName());
        if (ln == L"algorithm")
        {
          return el->getAttribute(L"kisaoID");
        }
      }
    }

    n = already_AddRefd<iface::dom::Node>(n->nextSibling());
  }
}

void
CDA_SProSSimulation::algorithmKisaoID(const std::wstring& aID) throw()
{
  RETURN_INTO_OBJREF(n, iface::dom::Node, mDomEl->firstChild());
  while (true)
  {
    if (n == NULL)
    {
      RETURN_INTO_OBJREF(od, iface::dom::Document, mDomEl->ownerDocument());
      RETURN_INTO_OBJREF(el, iface::dom::Element,
                         od->createElementNS(SEDML_NS, L"algorithm"));
      el->setAttribute(L"kisaoID", aID);
      mDomEl->appendChild(el)->release_ref();
      return;
    }
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el != NULL)
    {
      RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
      if (nsURI == SEDML_NS)
      {
        RETURN_INTO_WSTRING(ln, el->localName());
        if (ln == L"algorithm")
        {
          el->setAttribute(L"kisaoID", aID);
          return;
        }
      }
    }

    n = already_AddRefd<iface::dom::Node>(n->nextSibling());
  }
}

#define SimulationTypes L"uniformTimeCourse", L"samplingSensitivityAnalysis"
SomeSProSSet(Simulation, L"listOfSimulations", SimulationTypes);

double
CDA_SProSUniformTimeCourseBase::initialTime()
  throw()
{
  RETURN_INTO_WSTRING(it, mDomEl->getAttribute(L"initialTime"));
  return wcstod(it.c_str(), NULL);
}

void
CDA_SProSUniformTimeCourseBase::initialTime(double aValue)
  throw()
{
  wchar_t buf[32];
  any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%g", aValue);
  mDomEl->setAttribute(L"initialTime", buf);
}

double
CDA_SProSUniformTimeCourseBase::outputStartTime()
  throw()
{
  RETURN_INTO_WSTRING(it, mDomEl->getAttribute(L"outputStartTime"));
  return wcstod(it.c_str(), NULL);
}

void
CDA_SProSUniformTimeCourseBase::outputStartTime(double aValue)
  throw()
{
  wchar_t buf[32];
  any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%g", aValue);
  mDomEl->setAttribute(L"outputStartTime", buf);
}

double
CDA_SProSUniformTimeCourseBase::outputEndTime()
  throw()
{
  RETURN_INTO_WSTRING(it, mDomEl->getAttribute(L"outputEndTime"));
  return wcstod(it.c_str(), NULL);
}

void
CDA_SProSUniformTimeCourseBase::outputEndTime(double aValue)
  throw()
{
  wchar_t buf[32];
  any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%g", aValue);
  mDomEl->setAttribute(L"outputEndTime", buf);
}

void
CDA_SProSUniformTimeCourseBase::numberOfPoints(uint32_t aNumPoints)
  throw()
{
  wchar_t buf[32];
  any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%lu", aNumPoints);
  mDomEl->setAttribute(L"numberOfPoints", buf);
}

uint32_t
CDA_SProSUniformTimeCourseBase::numberOfPoints()
  throw()
{
  RETURN_INTO_WSTRING(it, mDomEl->getAttribute(L"numberOfPoints"));
  return wcstoul(it.c_str(), NULL, 10);
}
SomeSProSSet(Task, L"listOfTasks", L"task");

std::wstring
CDA_SProSTask::simulationReferenceIdentifier()
  throw()
{
  return mDomEl->getAttribute(L"simulationReference");
}

void
CDA_SProSTask::simulationReferenceIdentifier(const std::wstring& aSim)
  throw()
{
  return mDomEl->setAttribute(L"simulationReference", aSim);
}

already_AddRefd<iface::SProS::Simulation>
CDA_SProSTask::simulationReference() throw()
{
  if (mParent == NULL)
    return NULL;

  RETURN_INTO_WSTRING(sr, simulationReferenceIdentifier());
  RETURN_INTO_OBJREF(ss, iface::SProS::SimulationSet,
                     static_cast<CDA_SProSSEDMLElement*>(mParent)->simulations());
  return ss->getSimulationByIdentifier(sr.c_str());
}

void
CDA_SProSTask::simulationReference(iface::SProS::Simulation* aSim) throw()
{
  RETURN_INTO_WSTRING(ident, aSim->id());
  simulationReferenceIdentifier(ident.c_str());
}

std::wstring
CDA_SProSTask::modelReferenceIdentifier()
  throw()
{
  return mDomEl->getAttribute(L"modelReference");
}

void
CDA_SProSTask::modelReferenceIdentifier(const std::wstring& aSim)
  throw()
{
  return mDomEl->setAttribute(L"modelReference", aSim);
}

already_AddRefd<iface::SProS::Model>
CDA_SProSTask::modelReference() throw()
{
  if (mParent == NULL)
    return NULL;

  RETURN_INTO_WSTRING(sr, modelReferenceIdentifier());
  RETURN_INTO_OBJREF(ss, iface::SProS::ModelSet,
                     static_cast<CDA_SProSSEDMLElement*>(mParent)->models());
  return ss->getModelByIdentifier(sr.c_str());
}

void
CDA_SProSTask::modelReference(iface::SProS::Model* aSim) throw()
{
  RETURN_INTO_WSTRING(ident, aSim->id());
  modelReferenceIdentifier(ident.c_str());
}

already_AddRefd<iface::SProS::ParameterSet>
CDA_SProSDataGenerator::parameters() throw()
{
  add_ref();
  return &mParameterSet;
}

already_AddRefd<iface::SProS::VariableSet>
CDA_SProSDataGenerator::variables() throw()
{
  add_ref();
  return &mVariableSet;
}

already_AddRefd<iface::mathml_dom::MathMLMathElement>
CDA_SProSDataGenerator::math() throw()
{
  RETURN_INTO_OBJREF(cn, iface::dom::Node, mDomEl->firstChild());
  for (; cn; cn = already_AddRefd<iface::dom::Node>(cn->nextSibling()))
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, cn, mathml_dom::MathMLMathElement);
    if (el == NULL)
      continue;
    el->add_ref();
    return el.getPointer();
  }
  return NULL;
}

void
CDA_SProSDataGenerator::math(iface::mathml_dom::MathMLMathElement* aMath) throw()
{
  RETURN_INTO_OBJREF(cn, iface::dom::Node, mDomEl->firstChild());
  while (cn)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, cn, mathml_dom::MathMLMathElement);
    cn = already_AddRefd<iface::dom::Node>(cn->nextSibling());

    if (el == NULL)
      continue;

    mDomEl->removeChild(el)->release_ref();
  }

  mDomEl->appendChild(aMath)->release_ref();
}

SomeSProSSet(DataGenerator, L"listOfDataGenerators", L"dataGenerator");
#define OutputTypes L"plot2D", L"plot3D", L"report"
SomeSProSSet(Output, L"listOfOutputs", OutputTypes);

already_AddRefd<iface::SProS::CurveSet>
CDA_SProSPlot2D::curves()
  throw()
{
  add_ref();
  return &mCurveSet;
}

already_AddRefd<iface::SProS::SurfaceSet>
CDA_SProSPlot3D::surfaces()
  throw()
{
  add_ref();
  return &mSurfaceSet;
}

already_AddRefd<iface::SProS::DataSetSet>
CDA_SProSReport::datasets()
  throw()
{
  add_ref();
  return &mDataSetSet;
}

std::wstring
CDA_SProSChange::target()
  throw()
{
  return mDomEl->getAttribute(L"target");
}

void
CDA_SProSChange::target(const std::wstring& aTarget)
  throw()
{
  mDomEl->setAttribute(L"target", aTarget);
}

#define ChangeTypes L"computeChange", L"changeAttribute", L"changeXML", L"addXML", L"removeXML"
SomeAnonSProSSet(Change, L"listOfChanges", ChangeTypes);

already_AddRefd<iface::SProS::VariableSet>
CDA_SProSComputeChange::variables()
  throw()
{
  add_ref();
  return &mVariables;
}

already_AddRefd<iface::SProS::ParameterSet>
CDA_SProSComputeChange::parameters()
  throw()
{
  add_ref();
  return &mParameters;
}

already_AddRefd<iface::mathml_dom::MathMLMathElement>
CDA_SProSComputeChange::math() throw()
{
  RETURN_INTO_OBJREF(cn, iface::dom::Node, mDomEl->firstChild());
  for (; cn; cn = already_AddRefd<iface::dom::Node>(cn->nextSibling()))
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, cn, mathml_dom::MathMLMathElement);
    if (el == NULL)
      continue;
    el->add_ref();
    return el.getPointer();
  }
  return NULL;
}

void
CDA_SProSComputeChange::math(iface::mathml_dom::MathMLMathElement* aMath) throw()
{
  RETURN_INTO_OBJREF(cn, iface::dom::Node, mDomEl->firstChild());
  while (cn)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, cn, mathml_dom::MathMLMathElement);
    cn = already_AddRefd<iface::dom::Node>(cn->nextSibling());

    if (el == NULL)
      continue;

    mDomEl->removeChild(el)->release_ref();
  }

  mDomEl->appendChild(aMath)->release_ref();
}

std::wstring
CDA_SProSChangeAttribute::newValue()
  throw()
{
  return mDomEl->getAttribute(L"newValue");
}

void
CDA_SProSChangeAttribute::newValue(const std::wstring& aValue)
  throw()
{
  return mDomEl->setAttribute(L"newValue", aValue);
}

already_AddRefd<iface::dom::NodeList>
CDA_SProSAddXML::anyXML()
  throw()
{
  return allNodesFromNamedElements(mDomEl, L"newXML");
}

std::wstring
CDA_SProSVariable::target() throw()
{
  return mDomEl->getAttribute(L"target");
}

void
CDA_SProSVariable::target(const std::wstring& aTarget) throw()
{
  mDomEl->setAttribute(L"target", aTarget);
}

std::wstring
CDA_SProSVariable::symbol() throw()
{
  return mDomEl->getAttribute(L"symbol");
}

void
CDA_SProSVariable::symbol(const std::wstring& aSymbol)
  throw()
{
  mDomEl->setAttribute(L"symbol", aSymbol);
}

std::wstring
CDA_SProSVariable::taskReferenceID()
  throw()
{
  return mDomEl->getAttribute(L"taskReference");
}

void
CDA_SProSVariable::taskReferenceID(const std::wstring& aRefID)
  throw()
{
  mDomEl->setAttribute(L"taskReference", aRefID);
}

already_AddRefd<iface::SProS::Task>
CDA_SProSVariable::taskReference()
  throw()
{
  if (mParent == NULL || mParent->mParent == NULL)
    return NULL;

  // Hierarchy can either be Variable -> ComputeChange -> Model -> SEDML or
  //   Variable -> DataGenerator -> SEDML. The former is invalid, but we
  // should make sure we don't crash on that case...
  CDA_SProSSEDMLElement* sedml = static_cast<CDA_SProSSEDMLElement*>(mParent->mParent);
  if (sedml == NULL)
    return NULL;

  RETURN_INTO_WSTRING(trid, taskReferenceID());

  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet,
                     static_cast<CDA_SProSSEDMLElement*>(mParent->mParent)->tasks());

  return ts->getTaskByIdentifier(trid.c_str());
}

void
CDA_SProSVariable::taskReference(iface::SProS::Task* aTask)
  throw(std::exception&)
{
  if (mParent == NULL || mParent->mParent == NULL)
    throw iface::SProS::SProSException();

    // Hierarchy can either be Variable -> ComputeChange -> Model -> SEDML or
  //   Variable -> DataGenerator -> SEDML. The former is invalid, but we
  // should make sure we don't crash on that case...
  CDA_SProSSEDMLElement* sedml = static_cast<CDA_SProSSEDMLElement*>(mParent->mParent);
  if (sedml == NULL)
    throw iface::SProS::SProSException();

  if (CDA_objcmp(static_cast<CDA_SProSTask*>(aTask)->mParent, sedml))
    throw iface::SProS::SProSException();

  RETURN_INTO_WSTRING(tid, aTask->id());
  taskReferenceID(tid.c_str());
}

SomeSProSSet(Variable, L"listOfVariables", L"variable");

double
CDA_SProSParameter::value()
  throw()
{
  RETURN_INTO_WSTRING(value, mDomEl->getAttribute(L"value"));
  if (value == L"")
    return 0.0;
  return wcstod(value.c_str(), NULL);
}

void
CDA_SProSParameter::value(double aValue) throw()
{
  wchar_t value[64];
  any_swprintf(value, 64, L"%e", aValue);
  mDomEl->setAttribute(L"value", value);
}
SomeSProSSet(Parameter, L"listOfParameters", L"parameter");

bool
CDA_SProSCurve::logX() throw()
{
  RETURN_INTO_WSTRING(v, mDomEl->getAttribute(L"logX"));
  if (v == L"true")
    return true;
  return false;
}

void
CDA_SProSCurve::logX(bool aValue) throw()
{
  mDomEl->setAttribute(L"logX", aValue ? L"true" : L"false");
}

bool
CDA_SProSCurve::logY() throw()
{
  RETURN_INTO_WSTRING(v, mDomEl->getAttribute(L"logY"));
  if (v == L"true")
    return true;
  return false;
}

void
CDA_SProSCurve::logY(bool aValue) throw()
{
  mDomEl->setAttribute(L"logY", aValue ? L"true" : L"false");
}

std::wstring
CDA_SProSCurve::xDataGeneratorID() throw()
{
  return mDomEl->getAttribute(L"xDataReference");
}

void
CDA_SProSCurve::xDataGeneratorID(const std::wstring& aRef) throw()
{
  return mDomEl->setAttribute(L"xDataReference", aRef);
}

std::wstring
CDA_SProSCurve::yDataGeneratorID() throw()
{
  return mDomEl->getAttribute(L"yDataReference");
}

void
CDA_SProSCurve::yDataGeneratorID(const std::wstring& aRef) throw()
{
  return mDomEl->setAttribute(L"yDataReference", aRef);
}

already_AddRefd<iface::SProS::DataGenerator>
CDA_SProSCurve::xDataGenerator() throw()
{
  if (mParent == NULL || mParent->mParent == NULL)
    return NULL;
  RETURN_INTO_OBJREF(gs, iface::SProS::DataGeneratorSet,
                     static_cast<CDA_SProSSEDMLElement*>(mParent->mParent)->generators());
  RETURN_INTO_WSTRING(ident, xDataGeneratorID());

  return gs->getDataGeneratorByIdentifier(ident.c_str());
}

void
CDA_SProSCurve::xDataGenerator(iface::SProS::DataGenerator* aGen) throw()
{
  RETURN_INTO_WSTRING(ident, aGen->id());
  mDomEl->setAttribute(L"xDataReference", ident.c_str());
}

void
CDA_SProSCurve::yDataGenerator(iface::SProS::DataGenerator* aGen) throw()
{
  RETURN_INTO_WSTRING(ident, aGen->id());
  mDomEl->setAttribute(L"yDataReference", ident.c_str());
}

already_AddRefd<iface::SProS::DataGenerator>
CDA_SProSCurve::yDataGenerator() throw()
{
  if (mParent == NULL || mParent->mParent == NULL)
    return NULL;
  RETURN_INTO_OBJREF(gs, iface::SProS::DataGeneratorSet,
                     static_cast<CDA_SProSSEDMLElement*>(mParent->mParent)->generators());
  RETURN_INTO_WSTRING(ident, yDataGeneratorID());

  return gs->getDataGeneratorByIdentifier(ident.c_str());
}

CDA_SProSCurveSetBase::CDA_SProSCurveSetBase(CDA_SProSBase* aParent, const wchar_t* aName, const wchar_t** aElNames)
  : CDA_SProSNamedElementSet(aParent, aName, aElNames)
{
}

already_AddRefd<iface::SProS::CurveIterator>
CDA_SProSCurveSetBase::iterateCurves() throw()
{
  findOrCreateListElement();
  return new CDA_SProSCurveIterator(this);
}

static const wchar_t* sCurveSetNames[] = {L"curve", NULL};
CDA_SProSCurveSet::CDA_SProSCurveSet(CDA_SProSBase* aParent)
  : CDA_SProSCurveSetBase(aParent, L"listOfCurves", sCurveSetNames) {}

bool
CDA_SProSSurface::logZ() throw()
{
  RETURN_INTO_WSTRING(v, mDomEl->getAttribute(L"logZ"));
  if (v == L"true")
    return true;
  return false;
}

void
CDA_SProSSurface::logZ(bool aValue) throw()
{
  mDomEl->setAttribute(L"logZ", aValue ? L"true" : L"false");
}

std::wstring
CDA_SProSSurface::zDataGeneratorID() throw()
{
  return mDomEl->getAttribute(L"zDataReference");
}

void
CDA_SProSSurface::zDataGeneratorID(const std::wstring& aRef) throw()
{
  return mDomEl->setAttribute(L"zDataReference", aRef);
}

already_AddRefd<iface::SProS::DataGenerator>
CDA_SProSSurface::zDataGenerator() throw()
{
  if (mParent == NULL || mParent->mParent == NULL)
    return NULL;
  RETURN_INTO_OBJREF(gs, iface::SProS::DataGeneratorSet,
                     static_cast<CDA_SProSSEDMLElement*>(mParent->mParent)->generators());
  RETURN_INTO_WSTRING(ident, zDataGeneratorID());

  return gs->getDataGeneratorByIdentifier(ident.c_str());
}

void
CDA_SProSSurface::zDataGenerator(iface::SProS::DataGenerator* aGen) throw()
{
  RETURN_INTO_WSTRING(ident, aGen->id());
  mDomEl->setAttribute(L"zDataReference", ident.c_str());
}

static const wchar_t* sSurfaceSetNames[] = {L"curve", L"surface", NULL};
CDA_SProSSurfaceSet::CDA_SProSSurfaceSet(CDA_SProSBase* aParent)
  : CDA_SProSCurveSetBase(aParent, L"listOfSurfaces", sSurfaceSetNames) {}

already_AddRefd<iface::SProS::SurfaceIterator>
CDA_SProSSurfaceSet::iterateSurfaces() throw()
{
  findOrCreateListElement();
  return new CDA_SProSSurfaceIterator(this);
}

CDA_SProSDataSet::CDA_SProSDataSet(CDA_SProSBase* aParent,
                                   iface::dom::Element* aEl)
  : CDA_SProSBase(aParent, aEl), CDA_SProSNamedElement(aParent, aEl)
{
}

std::wstring
CDA_SProSDataSet::dataGeneratorID()
  throw()
{
  return mDomEl->getAttribute(L"dataReference");
}

void
CDA_SProSDataSet::dataGeneratorID(const std::wstring& aRef)
  throw()
{
  mDomEl->setAttribute(L"dataReference", aRef);
}

already_AddRefd<iface::SProS::DataGenerator>
CDA_SProSDataSet::dataGen(void) throw()
{
  if (mParent == NULL || mParent->mParent == NULL)
    return NULL;
  CDA_SProSSEDMLElement* model =
    dynamic_cast<CDA_SProSSEDMLElement*>(mParent->mParent);
  if (model == NULL)
    return NULL;

  RETURN_INTO_WSTRING(dgid, dataGeneratorID());
  RETURN_INTO_OBJREF(dgs, iface::SProS::DataGeneratorSet, model->generators());
  RETURN_INTO_OBJREF(dg, iface::SProS::DataGenerator, dgs->getDataGeneratorByIdentifier(dgid.c_str()));

  dg->add_ref();
  return dg.getPointer();
}

void
CDA_SProSDataSet::dataGen(iface::SProS::DataGenerator* aDataGen) throw()
{
  RETURN_INTO_WSTRING(dgid, aDataGen->id());

  dataGeneratorID(dgid.c_str());
}

static const wchar_t* sDataSetSetMembers[] = {L"dataSet", NULL};

CDA_SProSDataSetSet::CDA_SProSDataSetSet(CDA_SProSBase* aParent)
  : CDA_SProSNamedElementSet(aParent, L"listOfDataSets", sDataSetSetMembers)
{
}

already_AddRefd<iface::SProS::DataSetIterator>
CDA_SProSDataSetSet::iterateDataSets()
  throw()
{
  findOrCreateListElement();
  return new CDA_SProSDataSetIterator(this);
}

#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"

already_AddRefd<iface::SProS::SEDMLElement>
CDA_SProSBootstrap::parseSEDMLFromURI(const std::wstring& uri, const std::wstring& relativeTo)
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_OBJREF(cbs, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    RETURN_INTO_OBJREF(dul, iface::cellml_api::DOMURLLoader, cbs->localURLLoader());
    RETURN_INTO_WSTRING(absu, cbs->makeURLAbsolute(relativeTo, uri));
    RETURN_INTO_OBJREF(doc, iface::dom::Document, dul->loadDocument(absu.c_str()));
    RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());

    CDA_SProSSEDMLElement* el = new CDA_SProSSEDMLElement(de);
    el->originalURL(absu.c_str());
    return el;
  }
  catch (...)
  {
    throw iface::SProS::SProSException();
  }
}

already_AddRefd<iface::SProS::SEDMLElement>
CDA_SProSBootstrap::parseSEDMLFromText(const std::wstring& txt, const std::wstring&)
  throw()
{
  RETURN_INTO_OBJREF(cbs, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(dul, iface::cellml_api::DOMURLLoader, cbs->localURLLoader());
  RETURN_INTO_OBJREF(doc, iface::dom::Document, dul->loadDocumentFromText(txt));
  RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
  return new CDA_SProSSEDMLElement(de);
}

std::wstring
CDA_SProSBootstrap::sedmlToText(iface::SProS::SEDMLElement* el)
  throw()
{
  RETURN_INTO_OBJREF(cbs, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
  CDA_SProSBase* sb = dynamic_cast<CDA_SProSBase*>(el);
  if (sb == NULL)
    return L"";

  return cbs->serialiseNode(sb->mDomEl);
}
