#define MODULE_CONTAINS_xpcom
#define MODULE_CONTAINS_RDFAPISPEC
#define MODULE_CONTAINS_CellMLAPISPEC
#define MODULE_CONTAINS_CellMLBootstrap
#define MODULE_CONTAINS_CellMLevents
#define MODULE_CONTAINS_DOMAPISPEC
#define MODULE_CONTAINS_DOMevents
#define MODULE_CONTAINS_MathMLcontentAPISPEC
#include "CellMLImplementation.hpp"
#include "IfaceCellML_events.hxx"

#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

#define DEVENT_DOMAttrModified 0
#define DEVENT_DOMCharacterDataModified 1
#define DEVENT_DOMNodeInserted 2
#define DEVENT_DOMNodeRemoved 3
#define EVENTMASK(x) (1 << x)

#define EVENT_CellMLAttributeChanged 0
#define EVENT_CellMLElementInserted 1
#define EVENT_CellMLElementRemoved 2
#define EVENT_ExtensionElementInserted 3
#define EVENT_ExtensionElementRemoved 4
#define EVENT_MathInserted 5
#define EVENT_MathModified 6
#define EVENT_MathRemoved 7

struct CellMLEventRecord
{
  const wchar_t* mEventName;
  uint32_t mNeededEventMask;
};

static CellMLEventRecord kSupportedEvents[] =
  {
    {L"CellMLAttributeChanged", EVENTMASK(DEVENT_DOMAttrModified)},
    {L"CellMLElementInserted", EVENTMASK(DEVENT_DOMNodeInserted)},
    {L"CellMLElementRemoved", EVENTMASK(DEVENT_DOMNodeRemoved)},
    {L"ExtensionElementInserted", EVENTMASK(DEVENT_DOMNodeInserted)},
    {L"ExtensionElementRemoved", EVENTMASK(DEVENT_DOMNodeRemoved)},
    {L"MathInserted", EVENTMASK(DEVENT_DOMNodeInserted)},
    {
      L"MathModified",
      EVENTMASK(DEVENT_DOMNodeInserted) | EVENTMASK(DEVENT_DOMNodeRemoved) |
      EVENTMASK(DEVENT_DOMAttrModified) |
      EVENTMASK(DEVENT_DOMCharacterDataModified)
    },
    {L"MathRemoved", EVENTMASK(DEVENT_DOMNodeRemoved)}
  };

static const wchar_t* kDOMEventNames[] = 
  {
    L"DOMAttrModified",
    L"DOMCharacterDataModified",
    L"DOMNodeInserted",
    L"DOMNodeRemoved"
  };

#define NEVENTS 8
#define NDOMEVENTS 4

// An adaptor which converts DOM events(listened for) into CellML events.
class CDA_CellMLElementEventAdaptor
  : public iface::events::EventListener
{
public:
  CDA_CellMLElementEventAdaptor
  (CDA_CellMLElement* aParent, iface::events::EventListener* aCellMLListener);
  ~CDA_CellMLElementEventAdaptor();
  
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(events::EventListener);
  CDA_IMPL_ID;

  void newEventType(int32_t aType) throw(std::exception&);
  void removeEventType(int32_t aType) throw(std::exception&);
  void removeAllEventTypes() throw(std::exception&);
  void considerDestruction() throw(std::exception&);
  void handleEvent(iface::events::Event* aEvent) throw(std::exception&);
private:
  CDA_CellMLElement* mParent;
  iface::events::EventListener* mCellMLListener;
  uint32_t mDOMCount[NDOMEVENTS];
  bool mGotEvent[NEVENTS];

  void handleNodeInserted(iface::events::Event* aEvent) throw(std::exception&);
  void handleNodeRemoved(iface::events::Event* aEvent) throw(std::exception&);
  void handleCharacterDataModified(iface::events::Event* aEvent)
    throw(std::exception&);
  void handleAttrModified(iface::events::Event* aEvent) throw(std::exception&);

  void handlePossibleMathModified(iface::events::Event* aEvent,
                                  iface::dom::Node* mathEl) 
    throw(std::exception&);
  void handleCellMLIntoCellML(iface::events::Event* aEvent,
                              iface::dom::Node* aParentEl,
                              iface::dom::Node* aChildEl)
    throw(std::exception&);
  void handleNonCellMLIntoCellML(iface::events::Event* aEvent,
                                 const wchar_t* event,
                                 iface::dom::Node* aParentEl,
                                 iface::dom::Node* aChildEl)
    throw(std::exception&);
  void handleCellMLOutOfCellML(iface::events::Event* aEvent,
                               iface::dom::Node* aParentEl,
                               iface::dom::Node* aChildEl)
    throw(std::exception&);

  CDA_CellMLElement*
  findCellMLElementFromNode(iface::dom::Node* aTarget) throw(std::exception&);
};

class CDA_CellMLMutationEvent
  : public iface::cellml_events::MutationEvent
{
public:
  CDA_CellMLMutationEvent()
    : mCurrentTarget(NULL), mTarget(NULL),
      mRelatedElement(NULL), mEventPhase(iface::events::Event::AT_TARGET),
      mAttrChange(iface::cellml_events::MutationEvent::MODIFICATION),
      mPropagationStopped(false), mDefaultPrevented(false)
  {
#ifdef _WIN32
    mTimeStamp = ((uint64_t)time(0)) * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    mTimeStamp = ((uint64_t)tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
#endif
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(events::Event, cellml_events::MutationEvent);
  CDA_IMPL_ID;

  virtual ~CDA_CellMLMutationEvent()
  {
    if (mCurrentTarget)
      mCurrentTarget->release_ref();
    if (mTarget)
      mTarget->release_ref();
    if (mRelatedElement)
      mRelatedElement->release_ref();
  }

  wchar_t*
  type()
    throw(std::exception&)
  {
    return CDA_wcsdup(mType.c_str());
  }

  iface::events::EventTarget*
  target()
    throw(std::exception&)
  {
    if (mTarget != NULL)
      mTarget->add_ref();
    return mTarget;
  }

  iface::events::EventTarget*
  currentTarget()
    throw(std::exception&)
  {
    if (mCurrentTarget != NULL)
      mCurrentTarget->add_ref();
    return mCurrentTarget;
  }
  
  uint16_t
  eventPhase()
    throw(std::exception&)
  {
    return mEventPhase;
  }

  bool
  bubbles()
    throw(std::exception&)
  {
    return true;
  }

  bool
  cancelable()
    throw(std::exception&)
  {
    return false;
  }

  uint64_t
  timeStamp()
    throw(std::exception&)
  {
    return mTimeStamp;
  }

  void
  stopPropagation()
    throw(std::exception&)
  {
    mPropagationStopped = true;
  }

  void
  preventDefault()
    throw(std::exception&)
  {
    mDefaultPrevented = true;
  }

  void
  initEvent(const wchar_t* aEventTypeArg,
            bool aCanBubble,
            bool aCancelable)
    throw(std::exception&)
  {
    mType = aEventTypeArg;
    if (!aCanBubble || aCancelable)
      throw iface::dom::DOMException();
  }

  wchar_t*
  prevValue()
    throw(std::exception&)
  {
    return CDA_wcsdup(mPrevValue.c_str());
  }

  wchar_t*
  newValue()
    throw(std::exception&)
  {
    return CDA_wcsdup(mNewValue.c_str());
  }

  wchar_t*
  attrLocalName()
    throw(std::exception&)
  {
    return CDA_wcsdup(mLocalName.c_str());
  }

  wchar_t*
  attrNamespaceURI()
    throw(std::exception&)
  {
    return CDA_wcsdup(mNamespaceURI.c_str());
  }

  uint16_t
  attrChange()
    throw(std::exception&)
  {
    return mAttrChange;
  }

  iface::cellml_api::CellMLElement*
  relatedElement()
    throw(std::exception&)
  {
    if (mRelatedElement)
      mRelatedElement->add_ref();
    return mRelatedElement;
  }

  iface::events::EventTarget *mCurrentTarget, *mTarget;
  iface::cellml_api::CellMLElement *mRelatedElement;
  std::wstring mType, mPrevValue, mNewValue, mLocalName, mNamespaceURI;
  uint16_t mEventPhase, mAttrChange;
  bool mPropagationStopped, mDefaultPrevented;
  uint64_t mTimeStamp;
};

static int32_t
FindEventByName(const wchar_t* aType)
{
  int32_t eventMin = 0;
  int32_t eventMax = (sizeof(kSupportedEvents)/sizeof(kSupportedEvents[0]));
  while (eventMin < eventMax)
  {
    uint32_t event = (eventMin + eventMax) / 2;
    int cmp = wcscmp(kSupportedEvents[event].mEventName, aType);
    if (cmp == 0)
      return event;
    if (cmp > 0)
      eventMax = event - 1;
    else
      eventMin = event + 1;
  }
  if (eventMin == eventMax &&
      !wcscmp(kSupportedEvents[eventMin].mEventName, aType))
    return eventMin;
  return -1;
}

void
CDA_CellMLElement::addEventListener
(
 const wchar_t* aType,
 iface::events::EventListener* aListener,
 bool aUseCapture
)
  throw(std::exception&)
{
  // Only bubbling is supported, as these events can't be cancelled.
  if (aUseCapture)
    throw iface::cellml_api::CellMLException();

  int32_t event = FindEventByName(aType);
  // Unknown events are silently ignored, as per the DOM Events specification.
  if (event == -1)
    return;

  // Find the adaptor, if there is one...
  ListenerToAdaptor_t::iterator i = mListenerToAdaptor.find(aListener);

  ObjRef<CDA_CellMLElementEventAdaptor> adaptor;

  if (i == mListenerToAdaptor.end())
  {
    // We add a refcount, putting the total to 2...
    adaptor = new CDA_CellMLElementEventAdaptor(this, aListener);
    // One refcount is used for the map, the other belongs to the ObjRef and
    // will be automatically dropped.
    mListenerToAdaptor.insert(std::pair<iface::events::EventListener*,
                              CDA_CellMLElementEventAdaptor*>
                              (aListener, adaptor)
                             );
  }
  else
    adaptor = (*i).second;

  try
  {
    adaptor->newEventType(event);
  }
  catch (std::exception& e)
  {
    adaptor->considerDestruction();
    throw e;
  }
}

void
CDA_CellMLElement::removeEventListener
(
 const wchar_t* aType,
 iface::events::EventListener* aListener,
 bool aUseCapture
)
  throw(std::exception&)
{
  // Only bubbling is supported, as these events can't be cancelled.
  if (aUseCapture)
    return;

  // Find the event...
  int32_t event = FindEventByName(aType);

  // Check it already exists...
  ListenerToAdaptor_t::iterator i = mListenerToAdaptor.find(aListener);

  if (i == mListenerToAdaptor.end())
    return;

  (*i).second->removeEventType(event);
  (*i).second->considerDestruction();
}

bool
CDA_CellMLElement::dispatchEvent(iface::events::Event* aEvent)
  throw(std::exception&)
{
  // We don't have the infrastructure to dispatch arbitrary events at this
  // level.
  throw iface::dom::DOMException();
}

void
CDA_CellMLElement::cleanupEvents()
{
  // Next, destroy any unremoved listeners...
  ListenerToAdaptor_t::iterator ltai;
  for (ltai = mListenerToAdaptor.begin(); ltai != mListenerToAdaptor.end();
       ltai++)
  {
    (*ltai).second->removeAllEventTypes();
    (*ltai).second->release_ref();
  }
}

CDA_CellMLElementEventAdaptor::CDA_CellMLElementEventAdaptor
(
 CDA_CellMLElement* aParent, iface::events::EventListener* aCellMLListener
)
  : mParent(aParent), mCellMLListener(aCellMLListener)
{
  memset(mDOMCount, 0, sizeof(mDOMCount));
  memset(mGotEvent, 0, sizeof(mGotEvent));
  mCellMLListener->add_ref();
}

CDA_CellMLElementEventAdaptor::~CDA_CellMLElementEventAdaptor()
{
  mCellMLListener->release_ref();
}

void
CDA_CellMLElementEventAdaptor::newEventType(int32_t aType)
  throw(std::exception&)
{
  if (mGotEvent[aType])
    return;

  mGotEvent[aType] = true;

  uint32_t domevent;
  for (domevent = 0;
       domevent < (sizeof(kDOMEventNames)/sizeof(kDOMEventNames[0]));
       domevent++)
    if ((kSupportedEvents[aType].mNeededEventMask) & EVENTMASK(domevent))
    {
      if (mDOMCount[domevent] == 0)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(targ, mParent->datastore,
                                       events::EventTarget);
        targ->addEventListener(kDOMEventNames[domevent], this, false);
      }
      mDOMCount[domevent]++;
    }
}

void
CDA_CellMLElementEventAdaptor::removeEventType(int32_t aType)
  throw(std::exception&)
{
  // If it doesn't exist, just ignore it(as per spec).
  if (!mGotEvent[aType])
    return;

  mGotEvent[aType] = false;

  uint32_t domevent;
  for (domevent = 0;
       domevent < (sizeof(kDOMEventNames)/sizeof(kDOMEventNames[0]));
       domevent++)
    if ((kSupportedEvents[aType].mNeededEventMask) & EVENTMASK(domevent))
    {
      mDOMCount[domevent]--;
      if (mDOMCount[domevent] == 0)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(targ, mParent->datastore,
                                       events::EventTarget);
        targ->removeEventListener(kDOMEventNames[domevent], this, false);
      }
    }
}

void
CDA_CellMLElementEventAdaptor::removeAllEventTypes()
  throw(std::exception&)
{
  DECLARE_QUERY_INTERFACE_OBJREF(targ, mParent->datastore, events::EventTarget);

  uint32_t domevent;
  for (domevent = 0;
       domevent < (sizeof(kDOMEventNames)/sizeof(kDOMEventNames[0]));
       domevent++)
  {
    if (mDOMCount[domevent] != 0)
    {
      targ->removeEventListener(kDOMEventNames[domevent], this, false);
    }
  }
  memset(mDOMCount, 0, sizeof(mDOMCount));
  memset(mGotEvent, 0, sizeof(mGotEvent));
}

void
CDA_CellMLElementEventAdaptor::considerDestruction()
   throw(std::exception&)
{
  // See if we have any events...
  uint32_t i;
  for (i = 0; i < sizeof(kSupportedEvents)/sizeof(kSupportedEvents[0]); i++)
    if (mGotEvent[i])
      return;

  // The typemap is empty, so we need to remove ourselves from our parent and
  // self-destruct...
  mParent->mListenerToAdaptor.erase(mCellMLListener);
  release_ref();
}

void
CDA_CellMLElementEventAdaptor::handleEvent(iface::events::Event* aEvent)
   throw(std::exception&)
{
  RETURN_INTO_WSTRING(type, aEvent->type());
  uint32_t sz = type.size();
  if (sz == 15)
  {
    if (type[3] == L'A' &&
        type == L"DOMAttrModified")
      handleAttrModified(aEvent);
    else if (type == L"DOMNodeInserted")
      handleNodeInserted(aEvent);
  }
  else if (sz == 14)
  {
    if (type == L"DOMNodeRemoved")
      handleNodeRemoved(aEvent);
  }
  else if (type == L"DOMCharacterDataModified")
    handleCharacterDataModified(aEvent);
}

enum ElementType
{
  NODETYPE_CELLML,
  NODETYPE_MATHML,
  NODETYPE_RDF,
  NODETYPE_EXTENSION
};

static uint16_t
ClassifyNode(iface::dom::Node* aNode, ElementType& aElType)
{
  uint16_t nodeType = aNode->nodeType();
  if (nodeType != iface::dom::Node::ELEMENT_NODE)
    return nodeType;

  RETURN_INTO_WSTRING(nsURI, aNode->namespaceURI());
  if (nsURI == L"http://www.cellml.org/cellml/1.0#" ||
      nsURI == L"http://www.cellml.org/cellml/1.1#")
    aElType = NODETYPE_CELLML;
  else if (nsURI == L"http://www.w3.org/1998/Math/MathML")
    aElType = NODETYPE_MATHML;
  else if (nsURI == L"http://www.w3.org/1999/02/22-rdf-syntax-ns#")
    aElType = NODETYPE_RDF;
  else
    aElType = NODETYPE_EXTENSION;

  return iface::dom::Node::ELEMENT_NODE;
}

void
CDA_CellMLElementEventAdaptor::handleNodeInserted(iface::events::Event* aEvent)
   throw(std::exception&)
{
  DECLARE_QUERY_INTERFACE_OBJREF(mutation, aEvent, events::MutationEvent);
  if (mutation == NULL)
    return;
  RETURN_INTO_OBJREF(relnode, iface::dom::Node, mutation->relatedNode());
  if (relnode == NULL)
    return;
  RETURN_INTO_OBJREF(etarg, iface::events::EventTarget, mutation->target());
  DECLARE_QUERY_INTERFACE_OBJREF(targ, etarg, dom::Node);
  if (targ == NULL)
    return;

  ElementType relnodeElType, targetElType;

  uint16_t relnodeType = ClassifyNode(relnode, relnodeElType);
  uint16_t targetType = ClassifyNode(targ, targetElType);

  // Inserting into a non-element node never causes events...
  if (relnodeType != iface::dom::Node::ELEMENT_NODE)
    return;
  
  // Only insertions into MathML and CellML ever cause events...
  if (relnodeElType != NODETYPE_CELLML && relnodeElType != NODETYPE_MATHML)
    return;

  // Insertions into MathML only ever cause MathModified...
  if (relnodeElType == NODETYPE_MATHML)
  {
    if (!mGotEvent[EVENT_MathModified])
      return;

    if (targetType != iface::dom::Node::ELEMENT_NODE &&
        targetType != iface::dom::Node::TEXT_NODE &&
        targetType != iface::dom::Node::CDATA_SECTION_NODE)
      return;

    handlePossibleMathModified(aEvent, relnode);
  }
  else // inserting into CellML...
  {
    if (targetType != iface::dom::Node::ELEMENT_NODE)
      return;
    switch (targetElType)
    {
    case NODETYPE_CELLML:
      if (mGotEvent[EVENT_CellMLElementInserted])
        handleCellMLIntoCellML(aEvent, relnode, targ);
      return;
    case NODETYPE_MATHML:
      if (mGotEvent[EVENT_MathInserted])
        handleNonCellMLIntoCellML(aEvent, L"MathMLElementInserted", relnode,
                                  targ);
      return;
    case NODETYPE_EXTENSION:
      if (mGotEvent[EVENT_ExtensionElementInserted])
        handleNonCellMLIntoCellML(aEvent, L"ExtensionElementInserted", relnode,
                                  targ);
      return;
    default:
      return;
    }
  }
}

void
CDA_CellMLElementEventAdaptor::handleNodeRemoved(iface::events::Event* aEvent)
   throw(std::exception&)
{
  DECLARE_QUERY_INTERFACE_OBJREF(mutation, aEvent, events::MutationEvent);
  if (mutation == NULL)
    return;
  RETURN_INTO_OBJREF(relnode, iface::dom::Node, mutation->relatedNode());
  if (relnode == NULL)
    return;
  RETURN_INTO_OBJREF(etarg, iface::events::EventTarget, mutation->target());
  DECLARE_QUERY_INTERFACE_OBJREF(targ, etarg, dom::Node);
  if (targ == NULL)
    return;

  ElementType relnodeElType, targetElType;

  uint16_t relnodeType = ClassifyNode(relnode, relnodeElType);
  uint16_t targetType = ClassifyNode(targ, targetElType);

  // Removing from a non-element node never causes events...
  if (relnodeType != iface::dom::Node::ELEMENT_NODE)
    return;
  
  // Only removals from MathML and CellML ever cause events...
  if (relnodeElType != NODETYPE_CELLML && relnodeElType != NODETYPE_MATHML)
    return;

  // Removals from MathML only ever cause MathModified...
  if (relnodeElType == NODETYPE_MATHML)
  {
    if (!mGotEvent[EVENT_MathModified])
      return;

    if (targetType != iface::dom::Node::ELEMENT_NODE &&
        targetType != iface::dom::Node::TEXT_NODE &&
        targetType != iface::dom::Node::CDATA_SECTION_NODE)
      return;

    handlePossibleMathModified(aEvent, relnode);
  }
  else // removing from CellML...
  {
    if (targetType != iface::dom::Node::ELEMENT_NODE)
      return;
    switch (targetElType)
    {
    case NODETYPE_CELLML:
      if (mGotEvent[EVENT_CellMLElementRemoved])
        handleCellMLOutOfCellML(aEvent, relnode, targ);
      return;
    case NODETYPE_MATHML:
      // We use IntoCellML since the logic is the same(rename?)
      if (mGotEvent[EVENT_MathRemoved])
        handleNonCellMLIntoCellML(aEvent, L"MathMLElementRemoved", relnode,
                                   targ);
      return;
    case NODETYPE_EXTENSION:
      if (mGotEvent[EVENT_ExtensionElementRemoved])
        handleNonCellMLIntoCellML(aEvent, L"ExtensionElementRemoved", relnode,
                                  targ);
      return;
    default:
      return;
    }
  }
}

void
CDA_CellMLElementEventAdaptor::handleCharacterDataModified
(iface::events::Event* aEvent)
  throw(std::exception&)
{
  if (!mGotEvent[EVENT_MathModified])
    return;


  RETURN_INTO_OBJREF(etarg, iface::events::EventTarget, aEvent->target());
  DECLARE_QUERY_INTERFACE_OBJREF(targ, etarg, dom::Node);
  if (targ == NULL)
    return;

  RETURN_INTO_OBJREF(par, iface::dom::Node, targ->parentNode());
  if (par == NULL)
    return;

  ElementType parElType;
  uint16_t parType = ClassifyNode(par, parElType);

  if (parType != iface::dom::Node::ELEMENT_NODE ||
      parElType != NODETYPE_MATHML)
    return;

  handlePossibleMathModified(aEvent, par);
}

void
CDA_CellMLElementEventAdaptor::handleAttrModified(iface::events::Event* aEvent)
  throw(std::exception&)
{
  DECLARE_QUERY_INTERFACE_OBJREF(mutation, aEvent, events::MutationEvent);
  if (mutation == NULL)
    return;
  RETURN_INTO_OBJREF(etarg, iface::events::EventTarget, mutation->target());
  DECLARE_QUERY_INTERFACE_OBJREF(targ, etarg, dom::Node);
  if (targ == NULL)
    return;

  ElementType targetElType;
  uint16_t targetType = ClassifyNode(targ, targetElType);
  if (targetType != iface::dom::Node::ELEMENT_NODE)
    return;
  if (targetElType == NODETYPE_MATHML)
  {
    if (mGotEvent[EVENT_MathModified])
      handlePossibleMathModified(aEvent, targ);
    return;
  }
  if (targetElType != NODETYPE_CELLML ||
      !mGotEvent[EVENT_CellMLAttributeChanged])
    return;

  RETURN_INTO_OBJREF(me, CDA_CellMLMutationEvent,
                     new CDA_CellMLMutationEvent());
  me->mEventPhase = mutation->eventPhase();
  mParent->add_ref();
  me->mCurrentTarget = mParent;
  me->mTarget = findCellMLElementFromNode(targ);
  if (me->mTarget == NULL)
    return;
  me->mAttrChange = mutation->attrChange();
  me->mType = L"CellMLAtttibuteChanged";
  wchar_t* tmp  = mutation->prevValue();
  me->mPrevValue = tmp;
  free(tmp);
  tmp = mutation->newValue();
  me->mNewValue = tmp;
  free(tmp);
  RETURN_INTO_OBJREF(attrN, iface::dom::Node, mutation->relatedNode());
  tmp = attrN->localName();
  me->mLocalName = tmp;
  free(tmp);
  tmp = attrN->namespaceURI();
  me->mNamespaceURI = tmp;
  free(tmp);
  mCellMLListener->handleEvent(me);
  if (me->mPropagationStopped)
    mutation->stopPropagation();
  if (me->mDefaultPrevented)
    mutation->preventDefault();
}

void
CDA_CellMLElementEventAdaptor::handlePossibleMathModified
(
 iface::events::Event* aEvent,
 iface::dom::Node* aNode
)
  throw(std::exception&)
{
  // We have a MathML node. Head towards the parent until we hit non-MathML or
  // CellML...
  ObjRef<iface::dom::Node> prevNode, currentNode = aNode;

  while (true)
  {
    ElementType elType;
    uint16_t nodeType = ClassifyNode(currentNode, elType);
    if (nodeType != iface::dom::Node::ELEMENT_NODE)
      return;
    if (elType == NODETYPE_CELLML)
      break;
    if (elType != NODETYPE_MATHML)
      return;
    prevNode = currentNode;
    currentNode = already_AddRefd<iface::dom::Node>(prevNode->parentNode());
    if (currentNode == NULL)
      return;
  }

  // We now have a MathML node in prevNode, which will be our target, and a
  // CellML node in currentNode, which we have to translate to a CelLMLElement
  // to get our relatedElement.
  RETURN_INTO_OBJREF(me, CDA_CellMLMutationEvent,
                     new CDA_CellMLMutationEvent());
  mParent->add_ref();
  me->mCurrentTarget = mParent;
  QUERY_INTERFACE(me->mTarget, prevNode, events::EventTarget);
  me->mRelatedElement = findCellMLElementFromNode(currentNode);
  if (me->mRelatedElement == NULL)
    return;
  me->mEventPhase = iface::events::Event::BUBBLING_PHASE;
  me->mType = L"MathModified";
  mCellMLListener->handleEvent(me);
  if (me->mPropagationStopped)
    aEvent->stopPropagation();
  if (me->mDefaultPrevented)
    aEvent->preventDefault();
}

void
CDA_CellMLElementEventAdaptor::handleCellMLIntoCellML
(
 iface::events::Event* aEvent,
 iface::dom::Node* aParentEl, iface::dom::Node* aChildEl
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(me, CDA_CellMLMutationEvent,
                     new CDA_CellMLMutationEvent());
  mParent->add_ref();
  me->mCurrentTarget = mParent;
  CDA_CellMLElement* el = findCellMLElementFromNode(aChildEl);
  if (el == NULL)
    return;
  me->mRelatedElement = dynamic_cast<CDA_CellMLElement*>(el->mParent);
  if (me->mRelatedElement == NULL)
  {
    el->release_ref();
    return;
  }
  else
    me->mRelatedElement->add_ref();
  me->mTarget = el;
  if (CDA_objcmp(me->mTarget, me->mCurrentTarget) == 0)
    me->mEventPhase = iface::events::Event::AT_TARGET;
  else
    me->mEventPhase = iface::events::Event::BUBBLING_PHASE;
  me->mType = L"CellMLElementInserted";
  mCellMLListener->handleEvent(me);
  if (me->mPropagationStopped)
    aEvent->stopPropagation();
  if (me->mDefaultPrevented)
    aEvent->preventDefault();
}

void
CDA_CellMLElementEventAdaptor::handleNonCellMLIntoCellML
(
 iface::events::Event* aEvent,
 const wchar_t* event, iface::dom::Node* aParentEl, iface::dom::Node* aChildEl
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(me, CDA_CellMLMutationEvent,
                     new CDA_CellMLMutationEvent());
  mParent->add_ref();
  me->mCurrentTarget = mParent;
  QUERY_INTERFACE(me->mTarget, aChildEl, events::EventTarget);
  me->mRelatedElement = findCellMLElementFromNode(aParentEl);
  me->mEventPhase = iface::events::Event::BUBBLING_PHASE;
  me->mType = event;
  mCellMLListener->handleEvent(me);
  if (me->mPropagationStopped)
    aEvent->stopPropagation();
  if (me->mDefaultPrevented)
    aEvent->preventDefault();
}

void
CDA_CellMLElementEventAdaptor::handleCellMLOutOfCellML
(
 iface::events::Event* aEvent,
 iface::dom::Node* aParentEl, iface::dom::Node* aChildEl
)
  throw(std::exception&)
{
  // We now have a MathML node in prevNode, which will be our target, and a
  // CellML node in currentNode, which we have to translate to a CelLMLElement
  // to get our relatedElement.
  RETURN_INTO_OBJREF(me, CDA_CellMLMutationEvent,
                     new CDA_CellMLMutationEvent());
  mParent->add_ref();
  me->mCurrentTarget = mParent;
  CDA_CellMLElement* el = findCellMLElementFromNode(aChildEl);
  if (el == NULL)
    return;
  me->mRelatedElement = dynamic_cast<CDA_CellMLElement*>(el->mParent);
  if (me->mRelatedElement == NULL)
  {
    el->release_ref();
    return;
  }
  else
    me->mRelatedElement->add_ref();
  me->mTarget = el;
  if (CDA_objcmp(me->mTarget, me->mCurrentTarget) == 0)
    me->mEventPhase = iface::events::Event::AT_TARGET;
  else
    me->mEventPhase = iface::events::Event::BUBBLING_PHASE;
  me->mType = L"CellMLElementRemoved";
  mCellMLListener->handleEvent(me);
  if (me->mPropagationStopped)
    aEvent->stopPropagation();
  if (me->mDefaultPrevented)
    aEvent->preventDefault();
}

static CDA_CellMLElement*
recurseFindCellMLElementFromNode
(
 CDA_CellMLElement* aSearchRoot,
 iface::dom::Node* aTarget
)
{
  if (CDA_objcmp(aSearchRoot->datastore, aTarget) == 0)
  {
    aSearchRoot->add_ref();
    return aSearchRoot;
  }

  RETURN_INTO_OBJREF(ce, iface::cellml_api::CellMLElementSet,
                     aSearchRoot->childElements());
  RETURN_INTO_OBJREF(it, iface::cellml_api::CellMLElementIterator,
                     ce->iterate());
  while (true)
  {
    RETURN_INTO_OBJREFD(el, CDA_CellMLElement, it->next());
    if (el == NULL)
      return NULL;
    CDA_CellMLElement* ret =
      recurseFindCellMLElementFromNode(el, aTarget);
    if (ret != NULL)
      return ret;
  }
}

CDA_CellMLElement*
CDA_CellMLElementEventAdaptor::findCellMLElementFromNode
(
 iface::dom::Node* aTarget
)
  throw(std::exception&)
{
  return recurseFindCellMLElementFromNode(mParent, aTarget);
}
