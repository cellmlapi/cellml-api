#define MODULE_CONTAINS_xpcom
#define MODULE_CONTAINS_DOMAPISPEC
#define MODULE_CONTAINS_MathMLcontentAPISPEC
#define MODULE_CONTAINS_DOMevents
#include <algorithm>
#include "cda_compiler_support.h"
#include "DOMImplementation.hpp"
#include <stdexcept>

/*
 * This next bit is a very pragmatic way of deciding when to throw away the
 * caches. We need to know when the cache might be stale, while at the same
 * time changes must be efficient. This problem is not as easy as checking the
 * root, since we could easily be working with partial fragments, which get
 * re-arranged. We could do this properly by visiting all elements connected to
 * the element being changed, and invalidating the cache at that time, but that
 * would be too slow. Instead, we have a single global serial number, which we
 * touch every time a change is made. If the current serial number doesn't
 * match the number on the cache, we throw the cache away and rebuild. This
 * does, of course, have the downside that if you have two models open, changes
 * to one model will wipe the cache of the other, completely separate model.
 * The other issue is that serial numbers wrap, so we should make sure the type
 * is big enough. At 1000 changes/second, it would take 49 days to get a
 * conflict with 32 bits, or about 585 million years with 64 bits.
 */
cda_serial_t gCDADOMChangeSerial = 1;

static void CDA_DOM_SomethingChanged()
{
  gCDADOMChangeSerial++;
}

// Returns true if a cache serial matches the global serial.
static bool CDA_DOMCompareSerial(cda_serial_t aSerial)
{
  return gCDADOMChangeSerial == aSerial;
}

CDA_DOMImplementation* CDA_DOMImplementation::sDOMImplementation =
  new CDA_DOMImplementation();

already_AddRefd<CDA_Element> CDA_NewElement
(
 CDA_Document* doc,  const std::wstring& nsURI, const std::wstring& elname
)
{
  if (nsURI == L"http://www.w3.org/1998/Math/MathML")
    return WrapMathMLElement(doc, elname);

  return new CDA_Element(doc);
}

already_AddRefd<CDA_Document> CDA_NewDocument
(
 const std::wstring& nsURI
)
{
  if (nsURI == L"http://www.w3.org/1998/Math/MathML")
  {
    return WrapMathMLDocument();
  }

  return new CDA_Document();
}

bool
CDA_DOMImplementation::hasFeature(const std::wstring& feature, const std::wstring& version)
  throw(std::exception&)
{
  if (feature == L"Core" &&
      (version == L"1.0" || version == L"2.0"))
    return true;
  if (feature == L"Events" && version == L"2.0")
    return true;
  if (feature == L"org.w3c.dom.mathml" && version == L"2.0")
    return true;

  return false;
}


already_AddRefd<iface::dom::DocumentType>
CDA_DOMImplementation::createDocumentType(const std::wstring& qualifiedName,
                                          const std::wstring& publicId,
                                          const std::wstring& systemId)
  throw(std::exception&)
{
  return new CDA_DocumentType(NULL, qualifiedName, publicId, systemId);
}

already_AddRefd<iface::dom::Document>
CDA_DOMImplementation::createDocument(const std::wstring& namespaceURI,
                                      const std::wstring& qualifiedName,
                                      iface::dom::DocumentType* idoctype)
  throw(std::exception&)
{
  const wchar_t* cqname = qualifiedName.c_str();
  const wchar_t* colon = wcschr(cqname, L':');
  if (colon != NULL)
  {
    if (namespaceURI == L"")
      throw iface::dom::DOMException(iface::dom::SYNTAX_ERR);
    if (colon-cqname == 3 &&
        !wcsncmp(cqname, L"xml", 3) &&
    qualifiedName != L"http://www.w3.org/XML/1998/namespace")
      throw iface::dom::DOMException(iface::dom::SYNTAX_ERR);
  }

  CDA_DocumentType* doctype = dynamic_cast<CDA_DocumentType*>(idoctype);

  if (doctype && doctype->mDocument != NULL)
    throw iface::dom::DOMException(iface::dom::WRONG_DOCUMENT_ERR);

  RETURN_INTO_OBJREF(doc, CDA_Document, CDA_NewDocument(namespaceURI));

  if (doctype)
  {
    doctype->mDocument = doc;
    doctype->mDocumentIsAncestor = false;
    doc->add_ref();
    doc->insertBeforePrivate(doctype, NULL)->release_ref();
  }

  const wchar_t* pos = wcschr(cqname, L':');
  if (pos == NULL)
    pos = cqname;
  else
    pos++;
  RETURN_INTO_OBJREF(docel, CDA_Element,
                     CDA_NewElement(doc, namespaceURI, pos));
  docel->mNamespaceURI = namespaceURI;
  docel->mNodeName = cqname;
  docel->mLocalName = pos;

  doc->appendChild(docel)->release_ref();

  doc->add_ref();
  return doc.getPointer();
}

CDA_Node::CDA_Node(CDA_Document* aDocument)
  : mParent(NULL), mDocumentIsAncestor(false), mDocument(aDocument)
{
  if (mDocument)
    mDocument->add_ref();
}

CDA_Node::~CDA_Node()
{
  if (!mDocumentIsAncestor && mDocument)
    mDocument->release_ref();
  std::multimap<eventid, iface::events::EventListener*>
     ::iterator i(mListeners.begin());
  for (; i != mListeners.end(); i++)
  {
    eventid e((*i).first);
    (*i).second->release_ref();
  }
  std::list<CDA_Node*>::iterator i2(mNodeList.begin());
  // Now start deleting the children...
  for (; i2 != mNodeList.end(); i2++)
    delete (*i2);
}

std::wstring
CDA_Node::nodeName()
  throw(std::exception&)
{
  return mNodeName;
}

std::wstring
CDA_Node::nodeValue()
  throw(std::exception&)
{
  return mNodeValue;
}

void
CDA_Node::nodeValue(const std::wstring& attr)
  throw(std::exception&)
{
  mNodeValue = attr;
}

already_AddRefd<iface::dom::Node>
CDA_Node::parentNode()
  throw(std::exception&)
{
  if (mParent != NULL)
    mParent->add_ref();
  return mParent;
}

already_AddRefd<iface::dom::NodeList>
CDA_Node::childNodes()
  throw(std::exception&)
{
  return new CDA_NodeList(this);
}

already_AddRefd<iface::dom::Node>
CDA_Node::firstChild()
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
  {
    uint16_t type = (*i)->nodeType();
    if (type != iface::dom::Node::ATTRIBUTE_NODE &&
        type != iface::dom::Node::NOTATION_NODE &&
        type != iface::dom::Node::ENTITY_NODE)
    {
      (*i)->add_ref();
      return (*i);
    }
  }
  return NULL;
}

already_AddRefd<iface::dom::Node>
CDA_Node::lastChild()
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mNodeList.end();
  while (i != mNodeList.begin())
  {
    i--;
    uint16_t type = (*i)->nodeType();
    if (type != iface::dom::Node::ATTRIBUTE_NODE &&
        type != iface::dom::Node::NOTATION_NODE &&
        type != iface::dom::Node::ENTITY_NODE)
    {
      (*i)->add_ref();
      return (*i);
    }
  }
  return NULL;
}

already_AddRefd<iface::dom::Node>
CDA_Node::previousSibling()
  throw(std::exception&)
{
  if (mParent == NULL)
    return NULL;

  if (nodeType() == iface::dom::Node::ATTRIBUTE_NODE)
    return NULL;

  std::list<CDA_Node*>::iterator i = mPositionInParent;
  while (true)
  {
    if (i == mParent->mNodeList.begin())
      return NULL;
    i--;
    uint16_t type = (*i)->nodeType();

    if (type != iface::dom::Node::ATTRIBUTE_NODE &&
        type != iface::dom::Node::ENTITY_NODE &&
        type != iface::dom::Node::NOTATION_NODE)
    {
      (*i)->add_ref();
      return (*i);
    }
  }
}

already_AddRefd<iface::dom::Node>
CDA_Node::nextSibling()
  throw(std::exception&)
{
  if (mParent == NULL)
    return NULL;

  if (nodeType() == iface::dom::Node::ATTRIBUTE_NODE)
    return NULL;

  std::list<CDA_Node*>::iterator i = mPositionInParent;

  while (true)
  {
    i++;
    if (i == mParent->mNodeList.end())
      return NULL;
    uint16_t type = (*i)->nodeType();

    if (
        type != iface::dom::Node::ATTRIBUTE_NODE &&
        type != iface::dom::Node::ENTITY_NODE &&
        type != iface::dom::Node::NOTATION_NODE
       )
    {
      (*i)->add_ref();
      return (*i);
    }
  }
}

already_AddRefd<iface::dom::NamedNodeMap>
CDA_Node::attributes()
  throw(std::exception&)
{
  return new CDA_EmptyNamedNodeMap();
}

already_AddRefd<iface::dom::Document>
CDA_Node::ownerDocument()
  throw(std::exception&)
{
  iface::dom::Document* d = mDocument;
  if (d != NULL)
    d->add_ref();
  return d;
}

void
CDA_Node::updateDocumentAncestorStatus(bool aStatus)
{
  if (mDocumentIsAncestor == aStatus)
    return;

  if (aStatus && mDocument)
  {
    // We are now under the document's refcount...
    mDocument->release_ref();
  }
  else if (mDocument && !aStatus)
  {
    // The document is no longer sharing our refcount, so needs an explicit ref
    mDocument->add_ref();
  }

  mDocumentIsAncestor = aStatus;
  // Now tell our children...
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (i = mNodeList.begin(); i != mNodeList.end(); i++)
    (*i)->updateDocumentAncestorStatus(aStatus);
}

void
CDA_Node::dispatchInsertedIntoDocument(CDA_MutationEvent* me)
  throw(std::exception&)
{
  me->initMutationEvent(L"DOMNodeInsertedIntoDocument", false, false,
                        NULL, L"", L"", L"",
                        iface::events::MutationEvent::MODIFICATION);
  dispatchEvent(me);

  // And now we need to recurse down the tree...
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
    (*i)->dispatchInsertedIntoDocument(me);
}

void
CDA_Node::dispatchRemovedFromDocument(CDA_MutationEvent* me)
  throw(std::exception&)
{
  me->initMutationEvent(L"DOMNodeRemovedFromDocument", false, false,
                        NULL, L"", L"", L"",
                        iface::events::MutationEvent::MODIFICATION);
  dispatchEvent(me);

  // And now we need to recurse down the tree...
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
    (*i)->dispatchRemovedFromDocument(me);
}

already_AddRefd<iface::dom::Node>
CDA_Node::insertBefore(iface::dom::Node* inewChild,
                       iface::dom::Node* irefChild)
  throw(std::exception&)
{
  if (inewChild == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  uint16_t type = inewChild->nodeType();

  // Get rid of nodes which can't be added this way...
  if (type == iface::dom::Node::ATTRIBUTE_NODE ||
      type == iface::dom::Node::DOCUMENT_NODE ||
      type == iface::dom::Node::DOCUMENT_TYPE_NODE ||
      type == iface::dom::Node::NOTATION_NODE ||
      type == iface::dom::Node::ENTITY_NODE)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  CDA_Node* newChild = dynamic_cast<CDA_Node*>(inewChild);
  CDA_Node* refChild = dynamic_cast<CDA_Node*>(irefChild);

  if (type == iface::dom::Node::DOCUMENT_FRAGMENT_NODE)
  {
    if (newChild == NULL)
      throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

    // We skip the d.f. and recurse onto its children...
    std::list<CDA_Node*>::iterator i = newChild->mNodeList.begin();
    for (; i != newChild->mNodeList.end(); i++)
      insertBefore(*i, refChild)->release_ref();

    newChild->add_ref();
    return newChild;
  }
  return insertBeforePrivate(newChild, refChild);
}

already_AddRefd<iface::dom::Node>
CDA_Node::insertBeforePrivate(CDA_Node* newChild,
                       CDA_Node* refChild)
  throw(std::exception&)
{
  // Check the new child...
  if (newChild == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  // If there is a refchild, it must belong to us...
  if (refChild && refChild->mParent != this)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  if (newChild->mDocument != mDocument)
    throw iface::dom::DOMException(iface::dom::WRONG_DOCUMENT_ERR);

  if (newChild == refChild)
  {
    // It is already in the right place...
    newChild->add_ref();
    return newChild;
  }

  if (newChild->mParent != NULL)
  {
    // Check that it is not our ancestor
    CDA_Node* n = this;
    while (n)
    {
      if (n == newChild)
        throw iface::dom::DOMException(iface::dom::INVALID_MODIFICATION_ERR);
      n = n->mParent;
    }
  }

  std::list<CDA_Node*>::iterator posit;

  if (newChild->mParent)
  {
    assert(!refChild || (refChild->mParent == this));
    newChild->mParent->removeChild(newChild)->release_ref();
    assert(!refChild || (refChild->mParent == this));
  }

  // Just in case the remove failed.
  if (newChild->mParent != NULL)
    throw iface::dom::DOMException(iface::dom::INVALID_MODIFICATION_ERR);

  if (refChild != NULL)
  {
    posit = std::find(mNodeList.begin(), mNodeList.end(), refChild);
    if (posit == mNodeList.end())
    {
      // The child belongs to us, but isn't on the list!
      assert(refChild->mParent == this);
      assert(0);
      throw iface::dom::DOMException(iface::dom::INVALID_STATE_ERR);
    }
  }
  else
    posit = mNodeList.end();

  // Update nodes' mDocumentIsAncestor...
  if (mDocumentIsAncestor)
    newChild->updateDocumentAncestorStatus(true);

  newChild->mParent = this;
  newChild->mPositionInParent = mNodeList.insert(posit, newChild);
  uint32_t i, rc = newChild->_cda_refcount;
  for (i = 0; i < rc; i++)
    add_ref();

  CDA_DOM_SomethingChanged();

  // Fire off a DOMNodeInserted(unless it is an attribute)...
  if (newChild->eventsHaveEffects() &&
      newChild->nodeType() != iface::dom::Node::ATTRIBUTE_NODE)
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMNodeInserted", true, false,
                          this, L"", L"", L"",
                          iface::events::MutationEvent::MODIFICATION);
    newChild->dispatchEvent(me);

    if (mDocumentIsAncestor)
    {
      // All ancestors of newChild now need a DOMNodeInsertedIntoDocument...
      newChild->dispatchInsertedIntoDocument(me);
    }
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }

  newChild->add_ref();
  return newChild;
}

already_AddRefd<iface::dom::Node>
CDA_Node::replaceChild(iface::dom::Node* inewChild,
                       iface::dom::Node* ioldChild)
  throw(std::exception&)
{
  CDA_Node* newChild = dynamic_cast<CDA_Node*>(inewChild);
  CDA_Node* oldChild = dynamic_cast<CDA_Node*>(ioldChild);

  if (newChild == oldChild)
  {
    oldChild->add_ref();
    return oldChild;
  }

  if (oldChild == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  insertBefore(newChild, oldChild)->release_ref();
  return removeChild(oldChild);
}

already_AddRefd<iface::dom::Node>
CDA_Node::removeChild(iface::dom::Node* ioldChild)
  throw(std::exception&)
{
  if (ioldChild == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
  uint16_t type = ioldChild->nodeType();
  if (type == iface::dom::Node::ATTRIBUTE_NODE ||
      type == iface::dom::Node::DOCUMENT_TYPE_NODE ||
      type == iface::dom::Node::NOTATION_NODE ||
      type == iface::dom::Node::ENTITY_NODE)
    throw iface::dom::DOMException(iface::dom::INVALID_STATE_ERR);

  CDA_Node* oldChild = dynamic_cast<CDA_Node*>(ioldChild);
  if (oldChild == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  return removeChildPrivate(oldChild);
}

already_AddRefd<iface::dom::Node>
CDA_Node::removeChildPrivate(CDA_Node* oldChild)
  throw(std::exception&)
{
  bool oldTreeHadEffects = false;

  if (oldChild->eventsHaveEffects() &&
      oldChild->nodeType() != iface::dom::Node::ATTRIBUTE_NODE)
  {
    oldTreeHadEffects = true;
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMNodeRemoved", true, false,
                          this, L"", L"", L"",
                          iface::events::MutationEvent::MODIFICATION);
    oldChild->dispatchEvent(me);
    if (mDocumentIsAncestor)
      oldChild->dispatchRemovedFromDocument(me);
  }

  CDA_DOM_SomethingChanged();

  std::list<CDA_Node*>::iterator posit =
    std::find(mNodeList.begin(), mNodeList.end(), oldChild);
  if (posit == mNodeList.end())
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  mNodeList.erase(posit);
  oldChild->mParent = NULL;
  uint32_t i, rc = oldChild->_cda_refcount;
  for (i = 0; i < rc; i++)
    release_ref();

  if (mDocumentIsAncestor)
    oldChild->updateDocumentAncestorStatus(false);

  if (oldTreeHadEffects)
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMSubtreeModified", true, false,
                          this, L"", L"", L"",
                          iface::events::MutationEvent::MODIFICATION);
    dispatchEvent(me);
  }

  oldChild->add_ref();
  return oldChild;
}

already_AddRefd<iface::dom::Node>
CDA_Node::appendChild(iface::dom::Node* inewChild)
  throw(std::exception&)
{
  CDA_Node* newChild = dynamic_cast<CDA_Node*>(inewChild);

  return insertBefore(newChild, NULL);
}

bool
CDA_Node::hasChildNodes()
  throw(std::exception&)
{
  return !mNodeList.empty();
}

already_AddRefd<iface::dom::Node>
CDA_Node::cloneNode(bool deep)
  throw(std::exception&)
{
  return cloneNodePrivate(mDocument, deep);
}

already_AddRefd<CDA_Node>
CDA_Node::cloneNodePrivate(CDA_Document* aDoc, bool deep)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(c, CDA_Node, shallowCloneNode(aDoc));

  if (!deep)
  {
    c->add_ref();
    return c.getPointer();
  }

  // Clone all children...
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
  {
    // See if its an attribute(in which case we already cloned it).
    if ((*i)->nodeType() == iface::dom::Node::ATTRIBUTE_NODE)
      continue;
    RETURN_INTO_OBJREF(n, CDA_Node, (*i)->cloneNodePrivate(c->mDocument, true));
    c->insertBeforePrivate(n, NULL)->release_ref();
  }

  c->add_ref();

  return c.getPointer();
}

void
CDA_Node::normalize()
  throw(std::exception&)
{
  // Normalize the children...
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
    (*i)->normalize();

  // Now scan through our nodes and look for adjacent text nodes to fold into
  // single nodes, or delete...
  ObjRef<CDA_TextBase> lastText;
  for (i = mNodeList.begin(); i != mNodeList.end(); i++)
  {
    ObjRef<CDA_TextBase> tb = dynamic_cast<CDA_TextBase*>(*i);
    if (tb == NULL)
    {
      lastText = NULL;
      continue;
    }
    if (tb->mNodeValue == L"")
    {
      removeChild(tb)->release_ref();
      continue;
    }
    if (lastText != NULL)
    {
      removeChild(tb)->release_ref();
      std::wstring rdo = lastText->mNodeValue;
      lastText->mNodeValue += tb->mNodeValue;
      lastText->dispatchCharDataModified(rdo);
      continue;
    }
    lastText = tb;
  }
}

bool
CDA_Node::isSupported(const std::wstring& feature, const std::wstring& version)
  throw(std::exception&)
{
  if (feature == L"Core" &&
      (version == L"1.0") || (version == L"2.0"))
    return true;
  if (feature == L"Events" && version == L"2.0")
    return true;
  if (feature == L"org.w3c.dom.mathml" && version == L"2.0")
    return true;

  return false;
}

std::wstring
CDA_Node::namespaceURI()
  throw(std::exception&)
{
  return mNamespaceURI;
}

std::wstring
CDA_Node::prefix()
  throw(std::exception&)
{
  size_t pos = mNodeName.find(L':');
  if (pos == std::wstring::npos)
    return L"";

  return mNodeName.substr(0, pos);
}

void
CDA_Node::prefix(const std::wstring& attr)
  throw(std::exception&)
{
  if (mNamespaceURI == L"")
    throw iface::dom::DOMException(iface::dom::INVALID_STATE_ERR);

  if (attr == L"xml" &&
      mNamespaceURI != L"http://www.w3.org/XML/1998/namespace")
    throw iface::dom::DOMException(iface::dom::SYNTAX_ERR);

  if (mLocalName == L"xmlns" && attr != L"")
    throw iface::dom::DOMException(iface::dom::SYNTAX_ERR);

  if (attr == L"xmlns" &&
      mNamespaceURI != L"http://www.w3.org/2000/xmlns/")
    throw iface::dom::DOMException(iface::dom::SYNTAX_ERR);

  mNodeName = attr;
  mNodeName += L":";
  mNodeName += mLocalName;
}

std::wstring
CDA_Node::localName()
  throw(std::exception&)
{
  return mLocalName;
}

void
CDA_Node::addEventListener(const std::wstring& type,
                           iface::events::EventListener* listener,
                           bool useCapture)
  throw(std::exception&)
{
  if (listener == NULL)
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);
  eventid p(type, useCapture);
  std::multimap<eventid, iface::events::EventListener*>
    ::iterator i = mListeners.lower_bound(p);

  // Silently ignore a request to add a duplicate, as per spec...
  for (; i != mListeners.end() && (*i).first == p; i++)
    if (CDA_objcmp((*i).second, listener) == 0)
      return;

  listener->add_ref();
  mListeners.insert(std::pair<eventid,
                    iface::events::EventListener*>
                    (p, listener));
}

void
CDA_Node::removeEventListener(const std::wstring& type,
                              iface::events::EventListener* listener,
                              bool useCapture)
  throw(std::exception&)
{
  if (listener == NULL)
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);
  eventid p(type, useCapture);
  std::multimap<eventid,iface::events::EventListener*>
     ::iterator i =
    mListeners.lower_bound(p);
  for (; i != mListeners.end() && (*i).first == p; i++)
    if (CDA_objcmp((*i).second, listener) == 0)
    {
      eventid e((*i).first);
      (*i).second->release_ref();
      mListeners.erase(i);
      return;
    }
}

bool
CDA_Node::eventsHaveEffects()
  throw(std::exception&)
{
  bool anyoneListening = false;
  CDA_Node* n = this;
  while (true)
  {
    if (n->hasEventListeners())
    {
      anyoneListening = true;
      break;
    }
    n = n->mParent;
    if (!n)
      break;
  }

  return anyoneListening;
}

bool
CDA_Node::dispatchEvent(iface::events::Event* evt)
  throw(std::exception&)
{
  // We can't dispatch just any event, so try a cast first.
  CDA_MutationEvent* me = dynamic_cast<CDA_MutationEvent*>(evt);
  if (me == NULL)
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);

  // Build a list of all ancestors, in case it changes...
  std::list<ObjRef<CDA_Node> > nodeList;

  CDA_Node* n = this;

  while (true)
  {
    n = n->mParent;
    if (n)
    {
      nodeList.push_back(n);
    }
    else
      break;
  }

  // Set up some details.
  me->mPropagationStopped = false;
  me->mCanceled = false;
  me->mTarget = this;

  // First do capturing phase...
  me->mPhase = iface::events::Event::CAPTURING_PHASE;
  std::list<ObjRef<CDA_Node> >::iterator i = nodeList.end();
  while (true)
  {
    if (i == nodeList.begin())
      break;
    i--;
    me->mCurrentTarget = (*i);

    (*i)->callEventListeners(me);

    if (me->mPropagationStopped)
      return !(me->mCanceled);
  }

  me->mCurrentTarget = this;
  me->mPhase = iface::events::Event::AT_TARGET;
  callEventListeners(me);

  if (me->mBubbles)
  {
    me->mPhase = iface::events::Event::BUBBLING_PHASE;
    for (i = nodeList.begin(); i != nodeList.end(); i++)
    {
      me->mCurrentTarget = (*i);

      (*i)->callEventListeners(me);

      if (me->mPropagationStopped)
        return !(me->mCanceled);
    }
  }

  return !(me->mCanceled);
}

bool
CDA_Node::hasEventListeners()
  throw(std::exception&)
{
  return (mListeners.begin() != mListeners.end());
}

void
CDA_Node::callEventListeners(CDA_MutationEvent* me)
  throw(std::exception&)
{
  bool wantCapturing, wantBubbling;

  if (!hasEventListeners())
    return;

  if (me->mPhase == iface::events::Event::CAPTURING_PHASE)
  {
    wantCapturing = true;
    wantBubbling = false;
  }
  else if (me->mPhase == iface::events::Event::BUBBLING_PHASE)
  {
    wantCapturing = false;
    wantBubbling = true;
  }
  else
  {
    wantCapturing = wantBubbling = true;
  }

  if (wantCapturing)
  {
    eventid p(me->mType, true);
    std::multimap<eventid,iface::events::EventListener*>
       ::iterator i(mListeners.lower_bound(p)), i2;
    while (i != mListeners.end() && (*i).first == p)
    {
      i2 = i;
      i++;

      try
      {
        (*i2).second->handleEvent(me);
      }
      catch (...)
      {
      }

      if (me->mPropagationStopped)
        return;
    }
  }
  if (wantBubbling)
  {
    eventid p(me->mType, false);
    std::multimap<eventid,iface::events::EventListener*>
       ::iterator i(mListeners.lower_bound(p)), i2;
    while (i != mListeners.end() && (*i).first == p)
    {
      i2 = i;
      i++;
      try
      {
        (*i2).second->handleEvent(me);
      }
      catch (...)
      {
      }

      if (me->mPropagationStopped)
        return;
    }
  }
}

void
CDA_Node::recursivelyChangeDocument(CDA_Document* aNewDocument)
{
  assert(!mDocumentIsAncestor);

  if (mDocument == aNewDocument)
    return;

  if (mDocument != NULL)
    mDocument->release_ref();
  mDocument = aNewDocument;
  if (mDocument != NULL)
    mDocument->add_ref();

  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
    (*i)->recursivelyChangeDocument(mDocument);
}

already_AddRefd<iface::dom::Element>
CDA_Node::searchForElementById(const std::wstring& elementId)
{
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
  {
    iface::dom::Element* e = (*i)->searchForElementById(elementId);
    if (e != NULL)
      return e;
  }

  return NULL;
}

#ifdef DEBUG_NODELEAK
void
CDA_Node::find_leaked()
{
  uint32_t sum = 0;
  std::list<CDA_Node*>::const_iterator i(mNodeList.begin());

  for (; i != mNodeList.end(); i++)
  {
    sum += (*i)->_cda_refcount;
    (*i)->find_leaked();
  }
  assert(sum == _cda_refcount);
}
#endif // DEBUG_NODELEAK

already_AddRefd<iface::dom::Node>
CDA_NodeList::item(uint32_t index)
  throw(std::exception&)
{
  uint32_t saveIndex = index;
  if (mParent == NULL)
    return NULL;

  std::list<CDA_Node*>::iterator i;
  // Can we use a hint saved from last time?
  if (CDA_DOMCompareSerial(hintSerial) && hintIndex <= index)
  {
    i = hintIterator;
    index -= hintIndex;
  }
  else
    i = mParent->mNodeList.begin();

  for (; i != mParent->mNodeList.end(); i++)
  {
    // See if it is a type we ignore...
    uint16_t type = (*i)->nodeType();
    if (type == iface::dom::Node::ATTRIBUTE_NODE ||
        type == iface::dom::Node::DOCUMENT_TYPE_NODE)
      continue;
    if (index == 0)
    {
      (*i)->add_ref();

      hintSerial = gCDADOMChangeSerial;
      hintIterator = i;
      hintIndex = saveIndex;

      return (*i);
      break;
    }
    index--;
  }

  return NULL;
}

uint32_t
CDA_NodeList::length()
  throw(std::exception&)
{
  if (lenCacheSerial == gCDADOMChangeSerial)
    return lenCache;

  std::list<CDA_Node*>::iterator i = mParent->mNodeList.begin();
  uint32_t length = 0;
  for (; i != mParent->mNodeList.end(); i++)
  {
    uint16_t type = (*i)->nodeType();
    if (type == iface::dom::Node::ATTRIBUTE_NODE ||
        type == iface::dom::Node::DOCUMENT_TYPE_NODE)
      continue;
    length++;
  }

  lenCacheSerial = gCDADOMChangeSerial;
  lenCache = length;
  return length;
}

already_AddRefd<iface::dom::Node>
CDA_NodeListDFSSearch::item(uint32_t index)
  throw(std::exception&)
{
  if (mParent == NULL)
    return NULL;

  // We have a list of iterators...
  std::list<std::pair<std::list<CDA_Node*>::iterator,
                      std::list<CDA_Node*>::iterator> > iteratorStack;
  iteratorStack.push_front(std::pair<std::list<CDA_Node*>::iterator,
                                     std::list<CDA_Node*>::iterator>
                           (mParent->mNodeList.begin(),
                            mParent->mNodeList.end()));

  while (!iteratorStack.empty())
  {
    std::pair<std::list<CDA_Node*>::iterator,
              std::list<CDA_Node*>::iterator>& itp
      = iteratorStack.front();

    if (itp.first == itp.second)
    {
      iteratorStack.pop_front();
      continue;
    }

    std::list<CDA_Node*>::iterator p = itp.first;
    itp.first++;

    // This is a pre-order traversal, so consider the element first...
    if ((*p)->nodeType() == iface::dom::Node::ELEMENT_NODE)
    {
      bool hit = true;
      switch (mFilterType)
      {
      case LEVEL_1_NAME_FILTER:
        if ((*p)->mNodeName != mNameFilter &&
            (*p)->mNodeName != L"*")
          hit = false;
        break;
      case LEVEL_2_NAME_FILTER:
        if ((*p)->mLocalName != mNameFilter &&
            (mNameFilter != L"*"))
          hit = false;
        if ((*p)->mNamespaceURI != mNamespaceFilter &&
            (mNamespaceFilter != L"*"))
          hit = false;
        break;
      }
      if (hit)
      {
        if (index == 0)
        {
          (*p)->add_ref();
          return *p;
        }
        index--;
      }
    }

    // Next, we need to recurse...
    iteratorStack.push_front(std::pair<std::list<CDA_Node*>::iterator,
                             std::list<CDA_Node*>::iterator>
                             ((*p)->mNodeList.begin(),
                              (*p)->mNodeList.end()));
  }
  return NULL;
}

uint32_t
CDA_NodeListDFSSearch::length()
  throw(std::exception&)
{
  if (mParent == NULL)
    return 0;

  uint32_t length = 0;

  // We have a list of iterators...
  std::list<std::pair<std::list<CDA_Node*>::iterator,
                      std::list<CDA_Node*>::iterator> > iteratorStack;
  iteratorStack.push_front(std::pair<std::list<CDA_Node*>::iterator,
                                     std::list<CDA_Node*>::iterator>
                           (mParent->mNodeList.begin(),
                            mParent->mNodeList.end()));

  while (!iteratorStack.empty())
  {
    std::pair<std::list<CDA_Node*>::iterator,
              std::list<CDA_Node*>::iterator>& itp
      = iteratorStack.front();

    if (itp.first == itp.second)
    {
      iteratorStack.pop_front();
      continue;
    }

    // This is a pre-order traversal, so consider the element first...
    if ((*(itp.first))->nodeType() == iface::dom::Node::ELEMENT_NODE)
    {
      bool hit = true;
      switch (mFilterType)
      {
      case LEVEL_1_NAME_FILTER:
        if ((*(itp.first))->mNodeName != mNameFilter &&
            (*(itp.first))->mNodeName != L"*")
          hit = false;
        break;
      case LEVEL_2_NAME_FILTER:
        if ((*(itp.first))->mLocalName != mNameFilter &&
            (mNameFilter != L"*"))
          hit = false;
        if ((*(itp.first))->mNamespaceURI != mNamespaceFilter &&
            (mNamespaceFilter != L"*"))
          hit = false;
        break;
      }
      if (hit)
      {
        length++;
      }
    }

    // Next, we need to recurse...
    CDA_Node* n = *(itp.first);
    itp.first++;
    iteratorStack.push_front(std::pair<std::list<CDA_Node*>::iterator,
                             std::list<CDA_Node*>::iterator>
                             (n->mNodeList.begin(),
                              n->mNodeList.end()));
  }

  return length;
}

CDA_NamedNodeMap::CDA_NamedNodeMap(CDA_Element* aElement)
  : mElement(aElement), hintSerial(0)
{
  mElement->add_ref();
}

CDA_NamedNodeMap::~CDA_NamedNodeMap()
{
  mElement->release_ref();
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::getNamedItem(const std::wstring& name)
  throw(std::exception&)
{
  std::map<CDA_Element::LocalName, CDA_Attr*>::iterator i =
    mElement->attributeMap.find(CDA_Element::LocalName(name));
  if (i == mElement->attributeMap.end())
    return NULL;
  (*i).second->add_ref();
  return (*i).second;
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::setNamedItem(iface::dom::Node* arg)
  throw(std::exception&)
{
  CDA_Attr* attr = dynamic_cast<CDA_Attr*>(arg);
  // Event is sent for us...
  return mElement->setAttributeNode(attr);
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::removeNamedItem(const std::wstring& name)
  throw(std::exception&)
{
  // std::pair<std::wstring,std::wstring> p(L"", name);
  std::map<CDA_Element::LocalName, CDA_Attr*>::iterator
    i = mElement->attributeMap.find(CDA_Element::LocalName(name));

  if (i == mElement->attributeMap.end())
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  // Remove the child(which sorts out the refcounting)...
  ObjRef<CDA_Attr> at = (*i).second;
  mElement->removeChildPrivate(at)->release_ref();

  CDA_Element::LocalName ln((*i).first);

  mElement->attributeMap.erase(i);
  std::map<CDA_Element::QualifiedName, CDA_Attr*>::iterator j =
    mElement->attributeMapNS.find(CDA_Element::QualifiedName
                                  (at->mNamespaceURI,
                                   at->mLocalName));
  CDA_Element::QualifiedName qn((*j).first);
  mElement->attributeMapNS.erase(j);

  CDA_DOM_SomethingChanged();

  if (mElement->eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          at, at->mNodeValue.c_str(), L"", name,
                          iface::events::MutationEvent::REMOVAL);
    mElement->dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    mElement->dispatchEvent(me);
  }

  at->add_ref();
  return at.getPointer();
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::item(uint32_t index)
  throw(std::exception&)
{
  uint32_t saveIndex =  index;

  std::map<CDA_Element::QualifiedName, CDA_Attr*>::iterator i =
    mElement->attributeMapNS.begin();

  // Can we use a hint saved from last time?
  if (CDA_DOMCompareSerial(hintSerial) && hintIndex <= index)
  {
    i = hintIterator;
    index -= hintIndex;
  }

  for (; i != mElement->attributeMapNS.end(); i++)
  {
    if (index == 0)
    {
      hintSerial = gCDADOMChangeSerial;
      hintIterator = i;
      hintIndex = saveIndex;

      (*i).second->add_ref();
      return (*i).second;
    }
    index--;
  }
  return NULL;
}

uint32_t
CDA_NamedNodeMap::length()
  throw(std::exception&)
{
  return mElement->attributeMap.size();
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::getNamedItemNS(const std::wstring& namespaceURI,
                                 const std::wstring& localName)
  throw(std::exception&)
{
  std::map<CDA_Element::QualifiedName, CDA_Attr*>::iterator i =
    mElement->attributeMapNS.find(CDA_Element::QualifiedName
                                  (namespaceURI, localName));
  if (i == mElement->attributeMapNS.end())
    return NULL;
  (*i).second->add_ref();
  return (*i).second;
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::setNamedItemNS(iface::dom::Node* arg)
  throw(std::exception&)
{
  CDA_Attr* attr = dynamic_cast<CDA_Attr*>(arg);
  // Event is sent for us...
  return mElement->setAttributeNodeNS(attr);
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMap::removeNamedItemNS(const std::wstring& namespaceURI,
                                    const std::wstring& localName)
  throw(std::exception&)
{
  std::map<CDA_Element::QualifiedName, CDA_Attr*>::iterator
    i = mElement->attributeMapNS.find
    (CDA_Element::QualifiedName(namespaceURI, localName));

  if (i == mElement->attributeMapNS.end())
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);

  // Remove the child(which sorts out the refcounting)...
  ObjRef<CDA_Attr> at = (*i).second;
  mElement->removeChildPrivate(at)->release_ref();

  std::map<CDA_Element::LocalName, CDA_Attr*>::iterator j =
    mElement->attributeMap.find(CDA_Element::LocalName
                                (at->mNodeName));

  mElement->attributeMapNS.erase(i);
  mElement->attributeMap.erase(j);
  at->add_ref();

  CDA_DOM_SomethingChanged();

  if (mElement->eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          at, at->mNodeValue.c_str(), L"", localName,
                          iface::events::MutationEvent::REMOVAL);
    mElement->dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    mElement->dispatchEvent(me);
  }

  return at.getPointer();
}

CDA_NamedNodeMapDT::CDA_NamedNodeMapDT(CDA_DocumentType* aDocType,
                                       uint16_t aType)
  : mDocType(aDocType), mType(aType)
{
  mDocType->add_ref();
}

CDA_NamedNodeMapDT::~CDA_NamedNodeMapDT()
{
  mDocType->release_ref();
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::getNamedItem(const std::wstring& name)
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mDocType->mNodeList.begin();
  for (; i != mDocType->mNodeList.end(); i++)
  {
    if ((*i)->nodeType() != mType)
      continue;
    if ((*i)->mNodeName != name)
      continue;
    (*i)->add_ref();
    return (*i);
  }
  return NULL;
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::setNamedItem(iface::dom::Node* arg)
  throw(std::exception&)
{
  throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::removeNamedItem(const std::wstring& name)
  throw(std::exception&)
{
  throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::item(uint32_t index)
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mDocType->mNodeList.begin();
  for (; i != mDocType->mNodeList.end(); i++)
  {
    if ((*i)->nodeType() != mType)
      continue;
    if (index != 0)
    {
      index--;
      continue;
    }
    (*i)->add_ref();
    return *i;
  }
  return NULL;
}

uint32_t
CDA_NamedNodeMapDT::length()
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mDocType->mNodeList.begin();
  uint32_t l = 0;
  for (; i != mDocType->mNodeList.end(); i++)
  {
    if ((*i)->nodeType() != mType)
      continue;
    l++;
  }

  return l;
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::getNamedItemNS(const std::wstring& namespaceURI,
                                   const std::wstring& localName)
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mDocType->mNodeList.begin();
  for (; i != mDocType->mNodeList.end(); i++)
  {
    if ((*i)->nodeType() != mType)
      continue;
    if ((*i)->mNamespaceURI != namespaceURI)
      continue;
    if ((*i)->mLocalName != localName)
      continue;
    (*i)->add_ref();
    return (*i);
  }
  return NULL;
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::setNamedItemNS(iface::dom::Node* arg)
  throw(std::exception&)
{
  throw iface::dom::DOMException(iface::dom::NOT_SUPPORTED_ERR);
}

already_AddRefd<iface::dom::Node>
CDA_NamedNodeMapDT::removeNamedItemNS(const std::wstring& namespaceURI,
                      const std::wstring& localName)
  throw(std::exception&)
{
  throw iface::dom::DOMException(iface::dom::NOT_SUPPORTED_ERR);
}


std::wstring
CDA_CharacterData::data()
  throw(std::exception&)
{
  return mNodeValue;
}

void
CDA_CharacterData::data(const std::wstring& attr)
  throw(std::exception&)
{
  std::wstring oldData = mNodeValue;
  mNodeValue = attr;
  dispatchCharDataModified(oldData);
}

void
CDA_CharacterData::nodeValue(const std::wstring& attr)
  throw(std::exception&)
{
  std::wstring oldData = mNodeValue;
  mNodeValue = attr;
  dispatchCharDataModified(oldData);
}

uint32_t
CDA_CharacterData::length()
  throw(std::exception&)
{
  return mNodeValue.length();
}

std::wstring
CDA_CharacterData::substringData(uint32_t offset, uint32_t count)
  throw(std::exception&)
{
  try
  {
    return mNodeValue.substr(offset, count);
  }
  catch (std::out_of_range& oor)
  {
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);
  }
}

void
CDA_CharacterData::appendData(const std::wstring& arg)
  throw(std::exception&)
{
  std::wstring oldData = mNodeValue;
  mNodeValue += arg;
  dispatchCharDataModified(oldData);
}

void
CDA_CharacterData::insertData(uint32_t offset, const std::wstring& arg)
  throw(std::exception&)
{
  try
  {
    std::wstring oldData = mNodeValue;
    std::wstring t = mNodeValue.substr(offset);
    mNodeValue = mNodeValue.substr(0, offset);
    mNodeValue += arg;
    mNodeValue += t;
    dispatchCharDataModified(oldData);
  }
  catch (std::out_of_range& oor)
  {
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);
  }
}

void
CDA_CharacterData::deleteData(uint32_t offset, uint32_t count)
  throw(std::exception&)
{
  try
  {
    std::wstring oldData = mNodeValue;
    std::wstring t = mNodeValue.substr(offset + count);
    mNodeValue = mNodeValue.substr(0, offset);
    mNodeValue += t;
    dispatchCharDataModified(oldData);
  }
  catch (std::out_of_range& oor)
  {
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);
  }
}

void
CDA_CharacterData::replaceData(uint32_t offset, uint32_t count,
                               const std::wstring& arg)
  throw(std::exception&)
{
  try
  {
    std::wstring oldData = mNodeValue;
    std::wstring t = mNodeValue.substr(offset + count);
    mNodeValue = mNodeValue.substr(0, offset);
    mNodeValue += arg;
    mNodeValue += t;
    dispatchCharDataModified(oldData);
  }
  catch (std::out_of_range& oor)
  {
    throw iface::dom::DOMException(iface::dom::INVALID_ACCESS_ERR);
  }
}

void
CDA_CharacterData::dispatchCharDataModified(const std::wstring& oldValue)
{
  if (!eventsHaveEffects())
    return;

  RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
  me->initMutationEvent(L"DOMCharacterDataModified", true, false,
                        NULL, oldValue.c_str(), mNodeValue.c_str(), L"",
                        iface::events::MutationEvent::MODIFICATION);
  dispatchEvent(me);
  me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                        L"", 0);
  dispatchEvent(me);
}

already_AddRefd<CDA_Node>
CDA_Attr::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_Attr* ca = new CDA_Attr(aDoc);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  ca->mSpecified = mSpecified;
  return ca;
}

std::wstring
CDA_Attr::name()
  throw(std::exception&)
{
  return mNodeName;
}

bool
CDA_Attr::specified()
  throw(std::exception&)
{
  return mSpecified;
}

std::wstring
CDA_Attr::value()
  throw(std::exception&)
{
  return mNodeValue;
}

void
CDA_Attr::value(const std::wstring& attr)
  throw(std::exception&)
{
  mNodeValue = attr;
  mSpecified = true;

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          this, L"", mNodeValue.c_str(), mNodeName.c_str(),
                          iface::events::MutationEvent::MODIFICATION);

    CDA_Element* el = dynamic_cast<CDA_Element*>(mParent);
    if (el == NULL)
      return;

    el->dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    el->dispatchEvent(me);
  }
}

already_AddRefd<iface::dom::Element>
CDA_Attr::ownerElement()
  throw(std::exception&)
{
  iface::dom::Element* el = dynamic_cast<iface::dom::Element*>(mParent);
  if (el != NULL)
    el->add_ref();
  return el;
}

CDA_Element::~CDA_Element()
{
}

already_AddRefd<CDA_Node>
CDA_Element::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_Element* ca = CDA_NewElement(aDoc, mNamespaceURI.c_str(),
                                   mLocalName.c_str());
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  // Attributes get cloned too...
  std::map<QualifiedName, CDA_Attr*>::iterator i
    = attributeMapNS.begin();
  for (; i != attributeMapNS.end(); i++)
  {
    RETURN_INTO_OBJREF(n, CDA_Node, (*i).second->shallowCloneNode(aDoc));
    CDA_Attr* at = static_cast<CDA_Attr*>(n.getPointer());
    iface::dom::Attr* atTmp = ca->setAttributeNodeNS(at);
    assert(atTmp == NULL);
  }

  return ca;
}

already_AddRefd<iface::dom::NamedNodeMap>
CDA_Element::attributes()
  throw(std::exception&)
{
  return new CDA_NamedNodeMap(this);
}

bool
CDA_Element::hasAttributes()
  throw(std::exception&)
{
  return !attributeMap.empty();
}

std::wstring
CDA_Element::tagName()
  throw(std::exception&)
{
  return mNodeName;
}

std::wstring
CDA_Element::getAttribute(const std::wstring& name)
  throw(std::exception&)
{
  std::map<LocalName, CDA_Attr*>::iterator
    i = attributeMap.find(LocalName(name));
  if (i == attributeMap.end())
    return L"";

  return (*i).second->value();
}

void
CDA_Element::setAttribute(const std::wstring& name, const std::wstring& value)
  throw(std::exception&)
{
  std::map<LocalName, CDA_Attr*>::iterator
    i = attributeMap.find(LocalName(name));

  CDA_DOM_SomethingChanged();

  if (i == attributeMap.end())
  {
    RETURN_INTO_OBJREF(a, CDA_Attr, new CDA_Attr(mDocument));
    a->mNodeValue = value;
    a->mNodeName = name;
    insertBeforePrivate(a, NULL)->release_ref();
    attributeMapNS.insert(std::pair<QualifiedName, CDA_Attr*>
                          (QualifiedName(L"", name),
                           a));
    attributeMap.insert(std::pair<LocalName,CDA_Attr*>
                        (LocalName(name), a));

    if (eventsHaveEffects())
    {
      RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
      me->initMutationEvent(L"DOMAttrModified", true, false,
                            a, L"", value, name,
                            iface::events::MutationEvent::ADDITION);
      dispatchEvent(me);
      me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                            L"", 0);
      dispatchEvent(me);
    }
    return;
  }

  std::wstring oldValue = (*i).second->mNodeValue;
  (*i).second->mNodeValue = value;

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          (*i).second, oldValue.c_str(), value, name,
                          iface::events::MutationEvent::MODIFICATION);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }
}

void
CDA_Element::removeAttribute(const std::wstring& name)
  throw(std::exception&)
{
  std::map<LocalName, CDA_Attr*>::iterator
    i = attributeMap.find(LocalName(name));

  if (i == attributeMap.end())
  {
    // DOM doesn't say its an error to remove a non-existant attribute.
    return;
  }

  // Remove the child(which sorts out the refcounting)...
  ObjRef<CDA_Attr> at = (*i).second;
  removeChildPrivate(at)->release_ref();
  attributeMap.erase(i);

  std::map<QualifiedName, CDA_Attr*>::iterator j;
  if (at->mLocalName != L"")
    j = attributeMapNS.find
      (
       QualifiedName(at->mNamespaceURI,
                     at->mLocalName)
      );
  else
    j = attributeMapNS.find
      (
       QualifiedName(at->mNamespaceURI.c_str(),
                     at->mNodeName.c_str())
      );

  attributeMapNS.erase(j);

  CDA_DOM_SomethingChanged();

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          (*i).second, at->mNodeValue.c_str(), L"", name,
                          iface::events::MutationEvent::REMOVAL);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }
}

already_AddRefd<iface::dom::Attr>
CDA_Element::getAttributeNode(const std::wstring& name)
  throw(std::exception&)
{
  std::map<LocalName, CDA_Attr*>::iterator
    i = attributeMap.find(LocalName(name));
  if (i == attributeMap.end())
    return NULL;

  (*i).second->add_ref();
  return (*i).second;
}

already_AddRefd<iface::dom::Attr>
CDA_Element::setAttributeNode(iface::dom::Attr* inewAttr)
  throw(std::exception&)
{
  CDA_Attr* newAttr = dynamic_cast<CDA_Attr*>(inewAttr);
  if (newAttr == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
  std::wstring name = newAttr->name();
  std::map<LocalName, CDA_Attr*>::iterator
    i = attributeMap.find(LocalName(name));
  std::map<QualifiedName, CDA_Attr*>::iterator
    j = attributeMapNS.find(QualifiedName(L"", name));

  CDA_DOM_SomethingChanged();

  if (i == attributeMap.end())
  {
    insertBeforePrivate(newAttr, NULL)->release_ref();
    attributeMap.insert(std::pair<LocalName, CDA_Attr*>(name, newAttr));
    attributeMapNS.insert(std::pair<QualifiedName, CDA_Attr*>
                          (QualifiedName(L"", name), newAttr));

    if (eventsHaveEffects())
    {
      RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
      me->initMutationEvent(L"DOMAttrModified", true, false,
                            newAttr, L"", newAttr->mNodeValue.c_str(),
                            name, iface::events::MutationEvent::ADDITION);
      dispatchEvent(me);
      me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                            L"", 0);
      dispatchEvent(me);
    }
    return NULL;
  }
  ObjRef<CDA_Attr> at = (*i).second;
  removeChildPrivate(at)->release_ref();
  attributeMap.erase(i);

  if (j != attributeMapNS.end())
  {
    attributeMapNS.erase(j);
  }

  insertBeforePrivate(newAttr, NULL)->release_ref();
  attributeMap.insert(std::pair<LocalName, CDA_Attr*>
                      (LocalName(name), newAttr));
  attributeMapNS.insert(std::pair<QualifiedName, CDA_Attr*>
                        (QualifiedName(L"", name),
                         newAttr));

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          at, at->mNodeValue.c_str(), newAttr->mNodeValue.c_str(),
                          name, iface::events::MutationEvent::REMOVAL);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          newAttr, at->mNodeValue.c_str(), newAttr->mNodeValue.c_str(),
                          name, iface::events::MutationEvent::ADDITION);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }

  at->add_ref();
  return at.getPointer();
}

already_AddRefd<iface::dom::Attr>
CDA_Element::removeAttributeNode(iface::dom::Attr* ioldAttr)
  throw(std::exception&)
{
  CDA_Attr* oldAttr = dynamic_cast<CDA_Attr*>(ioldAttr);
  if (oldAttr == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
  std::wstring name = oldAttr->name();
  std::wstring lname = oldAttr->localName();
  std::wstring nsuri = oldAttr->namespaceURI();
  std::map<LocalName, CDA_Attr*>::iterator
    i = attributeMap.find(LocalName(name));
  if (i == attributeMap.end())
  {
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
  }
  ObjRef<CDA_Attr> at = (*i).second;
  LocalName ln((*i).first);
  removeChildPrivate(at)->release_ref();
  attributeMap.erase(i);

  std::map<QualifiedName, CDA_Attr*>::iterator
    j = attributeMapNS.find(QualifiedName(nsuri, lname));
  QualifiedName qn((*j).first);
  attributeMapNS.erase(j);

  CDA_DOM_SomethingChanged();

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          at, at->mNodeValue, L"",
                          name, iface::events::MutationEvent::REMOVAL);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }

  at->add_ref();
  return at.getPointer();
}

already_AddRefd<iface::dom::NodeList>
CDA_Element::getElementsByTagName(const std::wstring& name)
  throw(std::exception&)
{
  return new CDA_NodeListDFSSearch(this, name);
}

std::wstring
CDA_Element::getAttributeNS(const std::wstring& namespaceURI,
                            const std::wstring& localName)
  throw(std::exception&)
{
  std::pair<std::wstring,std::wstring> p(namespaceURI, localName);
  std::map<QualifiedName, CDA_Attr*>::iterator
    i = attributeMapNS.find(QualifiedName(namespaceURI, localName));
  if (i == attributeMapNS.end())
    return L"";

  return (*i).second->value();
}

void
CDA_Element::setAttributeNS(const std::wstring& namespaceURI,
                            const std::wstring& qualifiedName,
                            const std::wstring& value)
  throw(std::exception&)
{
  const wchar_t* localName;
  const wchar_t* pos = wcschr(qualifiedName.c_str(), L':');
  if (pos == NULL)
    localName = qualifiedName.c_str();
  else
    localName = pos + 1;

  std::map<QualifiedName, CDA_Attr*>::iterator
    i = attributeMapNS.find(QualifiedName(namespaceURI, localName));

  CDA_DOM_SomethingChanged();

  if (i == attributeMapNS.end())
  {
    RETURN_INTO_OBJREF(a, CDA_Attr, new CDA_Attr(mDocument));
    a->value(value);
    a->mNodeName = qualifiedName;
    a->mLocalName = localName;
    a->mNamespaceURI = namespaceURI;
    insertBeforePrivate(a, NULL)->release_ref();
    attributeMapNS.insert(std::pair<QualifiedName, CDA_Attr*>
                          (
                           QualifiedName(namespaceURI, localName), a
                          )
                         );


    std::map<LocalName, CDA_Attr*>::iterator
      j = attributeMap.find(LocalName(qualifiedName));
    if (j != attributeMap.end())
    {
      attributeMap.erase(j);
    }

    attributeMap.insert(std::pair<LocalName,CDA_Attr*>
                        (LocalName(qualifiedName), a));

    if (eventsHaveEffects())
    {
      RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
      me->initMutationEvent(L"DOMAttrModified", true, false,
                            a, L"", value, localName,
                            iface::events::MutationEvent::ADDITION);
      dispatchEvent(me);
      me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                            L"", 0);
      dispatchEvent(me);
    }
    return;
  }

  std::wstring oldValue = (*i).second->mNodeValue;
  (*i).second->mNodeValue = value;

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          (*i).second, oldValue.c_str(), value, localName,
                          iface::events::MutationEvent::MODIFICATION);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }
}

void
CDA_Element::removeAttributeNS(const std::wstring& namespaceURI,
                               const std::wstring& localName)
  throw(std::exception&)
{
  std::map<QualifiedName, CDA_Attr*>::iterator
    i = attributeMapNS.find
    (QualifiedName(namespaceURI, localName));

  if (i == attributeMapNS.end())
  {
    // DOM doesn't say its an error to remove a non-existant attribute.
    return;
  }

  CDA_DOM_SomethingChanged();

  // Remove the child(which sorts out the refcounting)...
  ObjRef<CDA_Attr> at = (*i).second;
  removeChildPrivate(at)->release_ref();
  QualifiedName qn((*i).first);

  std::map<LocalName, CDA_Attr*>::iterator
    j = attributeMap.find(LocalName((*i).second->mNodeName));

  ObjRef<iface::dom::Node> n = (*i).second;

  attributeMapNS.erase(i);
  LocalName ln((*j).first);
  attributeMap.erase(j);

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          n, at->mNodeValue.c_str(), L"", localName,
                          iface::events::MutationEvent::REMOVAL);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }
}

already_AddRefd<iface::dom::Attr>
CDA_Element::getAttributeNodeNS(const std::wstring& namespaceURI,
                                const std::wstring& localName)
  throw(std::exception&)
{
  std::map<QualifiedName, CDA_Attr*>::iterator
    i = attributeMapNS.find(QualifiedName(namespaceURI, localName));
  if (i == attributeMapNS.end())
    return NULL;

  (*i).second->add_ref();
  return (*i).second;
}

already_AddRefd<iface::dom::Attr>
CDA_Element::setAttributeNodeNS(iface::dom::Attr* inewAttr)
  throw(std::exception&)
{
  CDA_Attr* newAttr = dynamic_cast<CDA_Attr*>(inewAttr);
  if (newAttr == NULL)
    throw iface::dom::DOMException(iface::dom::NOT_FOUND_ERR);
  if (newAttr->mLocalName == L"")
    newAttr->mLocalName = newAttr->mNodeName;
  std::pair<std::wstring,std::wstring> p
    (newAttr->mNamespaceURI, newAttr->mLocalName);
  std::map<QualifiedName, CDA_Attr*>::iterator
    i = attributeMapNS.find
    (QualifiedName(newAttr->mNamespaceURI.c_str(),
                   newAttr->mLocalName.c_str()));

  CDA_DOM_SomethingChanged();

  if (i == attributeMapNS.end())
  {
    insertBeforePrivate(newAttr, NULL)->release_ref();
    attributeMapNS.insert
      (
       std::pair<QualifiedName, CDA_Attr*>
       (QualifiedName(newAttr->mNamespaceURI,
              newAttr->mLocalName), newAttr)
      );
    attributeMap.insert(std::pair<LocalName,CDA_Attr*>
                        (LocalName(newAttr->mNodeName),
             newAttr));

    if (eventsHaveEffects())
    {
      RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
      me->initMutationEvent(L"DOMAttrModified", true, false,
                            newAttr, L"", newAttr->mNodeValue.c_str(),
                            newAttr->mLocalName.c_str(),
                            iface::events::MutationEvent::ADDITION);
      dispatchEvent(me);
      me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                            L"", 0);
      dispatchEvent(me);
    }

    return NULL;
  }
  ObjRef<CDA_Attr> at = (*i).second;
  QualifiedName qn((*i).first);
  removeChildPrivate(at)->release_ref();
  attributeMapNS.erase(i);

  std::map<LocalName, CDA_Attr*>::iterator j =
    attributeMap.find(LocalName(at->mNodeName.c_str()));
  LocalName ln((*j).first);
  attributeMap.erase(j);

  insertBeforePrivate(newAttr, NULL)->release_ref();
  attributeMapNS.insert(std::pair<QualifiedName, CDA_Attr*>
                        (QualifiedName(newAttr->mNamespaceURI,
                                       newAttr->mLocalName),
                         newAttr));
  attributeMap.insert(std::pair<LocalName, CDA_Attr*>
                      (LocalName(at->mNodeName), newAttr));

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          at, at->mNodeValue.c_str(), newAttr->mNodeValue.c_str(),
                          newAttr->mLocalName.c_str(), iface::events::MutationEvent::REMOVAL);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMAttrModified", true, false,
                          newAttr, at->mNodeValue.c_str(), newAttr->mNodeValue.c_str(),
                          newAttr->mLocalName.c_str(),
                          iface::events::MutationEvent::ADDITION);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }

  at->add_ref();
  return at.getPointer();
}

already_AddRefd<iface::dom::NodeList>
CDA_Element::getElementsByTagNameNS(const std::wstring& namespaceURI,
                                    const std::wstring& localName)
  throw(std::exception&)
{
  return new CDA_NodeListDFSSearch(this, namespaceURI, localName);
}

bool
CDA_Element::hasAttribute(const std::wstring& name)
  throw(std::exception&)
{
  return attributeMap.find(LocalName(name)) !=
    attributeMap.end();
}

bool CDA_Element::hasAttributeNS(const std::wstring& namespaceURI,
                                 const std::wstring& localName)
  throw(std::exception&)
{
  return attributeMapNS.find(QualifiedName(namespaceURI, localName))
    != attributeMapNS.end();
}

already_AddRefd<iface::dom::Element>
CDA_Element::searchForElementById(const std::wstring& elementId)
{
  // XXX DOM says: 'Attributes with the name "ID" are not of type ID unless so
  //     defined.', but we don't deal with DTDs, so we deviate from the DOM and
  //     just assume that "id" in namespace "" is of type ID.
  RETURN_INTO_WSTRING(ourId, getAttribute(L"id"));
  if (ourId == L"")
  {
    std::wstring tmp = getAttribute(L"xml:id");
    if (tmp != L"")
      ourId = tmp;
  }
  if (ourId == elementId)
  {
    add_ref();
    return this;
  }

  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
  {
    already_AddRefd<iface::dom::Element> e = (*i)->searchForElementById(elementId);
    if (e.getPointer() != NULL)
      return e;
  }

  return NULL;
}

already_AddRefd<iface::dom::Text>
CDA_TextBase::splitText(uint32_t offset)
  throw(std::exception&)
{
  if (mParent == NULL)
    throw iface::dom::DOMException(iface::dom::INVALID_STATE_ERR);

  RETURN_INTO_OBJREF(n, CDA_Node, shallowCloneNode(mDocument));
  CDA_TextBase* tb = static_cast<CDA_TextBase*>(n.getPointer());
  std::wstring oldData;
  oldData = mNodeValue;
  tb->mNodeValue = mNodeValue.substr(offset);
  mNodeValue = mNodeValue.substr(0, offset);
  RETURN_INTO_OBJREF(n2, iface::dom::Node, nextSibling());
  mParent->insertBefore(tb, n2)->release_ref();

  dispatchCharDataModified(oldData);

  tb->add_ref();
  return tb;
}

already_AddRefd<CDA_Node>
CDA_Text::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_Text* ca = new CDA_Text(aDoc);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  ca->mNodeValue = mNodeValue;

  return ca;
}

already_AddRefd<CDA_Node>
CDA_Comment::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_Comment* ca = new CDA_Comment(aDoc);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  ca->mNodeValue = mNodeValue;

  return ca;
}

already_AddRefd<CDA_Node>
CDA_CDATASection::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_CDATASection* ca = new CDA_CDATASection(aDoc);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  ca->mNodeValue = mNodeValue;

  return ca;
}

already_AddRefd<CDA_Node>
CDA_DocumentType::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_DocumentType* ca =
    new CDA_DocumentType(aDoc, mNodeName, mPublicId, mSystemId);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeValue = mNodeValue;

  return ca;
}

std::wstring
CDA_DocumentType::name()
  throw(std::exception&)
{
  return mNodeName;
}

already_AddRefd<iface::dom::NamedNodeMap>
CDA_DocumentType::entities()
  throw(std::exception&)
{
  return new CDA_NamedNodeMapDT(this, iface::dom::Node::ENTITY_NODE);
}

already_AddRefd<iface::dom::NamedNodeMap>
CDA_DocumentType::notations()
  throw(std::exception&)
{
  return new CDA_NamedNodeMapDT(this, iface::dom::Node::NOTATION_NODE);
}

std::wstring
CDA_DocumentType::publicId()
  throw(std::exception&)
{
  return mPublicId;
}

std::wstring
CDA_DocumentType::systemId()
  throw(std::exception&)
{
  return mSystemId;
}

std::wstring
CDA_DocumentType::internalSubset()
  throw(std::exception&)
{
  // The DOM basically leaves this up to the API, and since we don't store this
  // information as it is irrelevant to CellML, lets just skip it...
  return L"";
}

already_AddRefd<CDA_Node>
CDA_Notation::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_Notation* ca = new CDA_Notation(aDoc, mPublicId, mSystemId);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  return ca;
}

std::wstring
CDA_Notation::publicId()
  throw(std::exception&)
{
  return mPublicId;
}

std::wstring
CDA_Notation::systemId()
  throw(std::exception&)
{
  return mSystemId;
}

already_AddRefd<CDA_Node>
CDA_Entity::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_Entity* ca = new CDA_Entity(aDoc, mPublicId, mSystemId,
                                  mNotationName);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  return ca;
}

std::wstring
CDA_Entity::publicId()
  throw(std::exception&)
{
  return mPublicId;
}

std::wstring
CDA_Entity::systemId()
  throw(std::exception&)
{
  return mSystemId;
}

std::wstring
CDA_Entity::notationName()
  throw(std::exception&)
{
  return mNotationName;
}

already_AddRefd<CDA_Node>
CDA_EntityReference::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_EntityReference* ca =
    new CDA_EntityReference(aDoc);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  return ca;
}

already_AddRefd<CDA_Node>
CDA_ProcessingInstruction::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_ProcessingInstruction* ca =
    new CDA_ProcessingInstruction(aDoc, mNodeName, mNodeValue);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  return ca;
}

std::wstring
CDA_ProcessingInstruction::target()
  throw(std::exception&)
{
  return mNodeName;
}

std::wstring
CDA_ProcessingInstruction::data()
  throw(std::exception&)
{
  return mNodeValue;
}

void
CDA_ProcessingInstruction::data(const std::wstring& attr)
  throw(std::exception&)
{
  std::wstring oldData = mNodeValue;
  mNodeValue = attr;

  CDA_DOM_SomethingChanged();

  if (eventsHaveEffects())
  {
    RETURN_INTO_OBJREF(me, CDA_MutationEvent, new CDA_MutationEvent());
    me->initMutationEvent(L"DOMCharacterData", true, false,
                          NULL, oldData, mNodeValue, L"",
                          iface::events::MutationEvent::MODIFICATION);
    dispatchEvent(me);
    me->initMutationEvent(L"DOMSubtreeModified", true, false, NULL, L"", L"",
                          L"", 0);
    dispatchEvent(me);
  }
}

already_AddRefd<CDA_Node>
CDA_DocumentFragment::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  CDA_DocumentFragment* ca =
    new CDA_DocumentFragment(aDoc);
  ca->mLocalName = mLocalName;
  ca->mNamespaceURI = mNamespaceURI;
  ca->mNodeName = mNodeName;
  ca->mNodeValue = mNodeValue;
  return ca;
}

CDA_Document::CDA_Document
(
 const std::wstring& namespaceURI,
 const std::wstring& qualifiedName,
 CDA_DocumentType* doctype
)
  : CDA_Node(this)
{
  const wchar_t* pos = wcschr(qualifiedName.c_str(), L':');

  // We are our own document ancestor, so fix the refcounts...
  mDocumentIsAncestor = true;
  mDocument->release_ref();

  if (doctype && doctype->mDocument != NULL)
    throw iface::dom::DOMException(iface::dom::INVALID_STATE_ERR);

  if (doctype)
  {
    doctype->mDocument = this;
    doctype->mDocumentIsAncestor = false;
    doctype->mDocument->add_ref();
  }

  if (doctype != NULL)
    insertBeforePrivate(doctype, NULL)->release_ref();

  if (pos == NULL)
    pos = qualifiedName.c_str();
  else
    pos++;

  RETURN_INTO_OBJREF(docel, CDA_Element,
                     CDA_NewElement(this, namespaceURI, pos));
  docel->mNamespaceURI = namespaceURI;
  docel->mNodeName = qualifiedName;
  docel->mLocalName = pos;

  appendChild(docel)->release_ref();
}

already_AddRefd<iface::dom::DocumentType>
CDA_Document::doctype()
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
    if ((*i)->nodeType() == iface::dom::Node::DOCUMENT_TYPE_NODE)
    {
      CDA_DocumentType* dt = static_cast<CDA_DocumentType*>(*i);
      dt->add_ref();
      return dt;
    }
  return NULL;
}

already_AddRefd<iface::dom::DOMImplementation>
CDA_Document::implementation()
  throw(std::exception&)
{
  CDA_DOMImplementation::sDOMImplementation->add_ref();
  return CDA_DOMImplementation::sDOMImplementation;
}

already_AddRefd<iface::dom::Element>
CDA_Document::documentElement()
  throw(std::exception&)
{
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
    if ((*i)->nodeType() == iface::dom::Node::ELEMENT_NODE)
    {
      CDA_Element* el = static_cast<CDA_Element*>(*i);
      el->add_ref();
      return el;
    }
  return NULL;
}

already_AddRefd<iface::dom::Element>
CDA_Document::createElement(const std::wstring& tagName)
  throw(std::exception&)
{
  CDA_Element* el = CDA_NewElement(this, L"", tagName);
  el->mNodeName = tagName;
  return el;
}

already_AddRefd<iface::dom::DocumentFragment>
CDA_Document::createDocumentFragment()
  throw(std::exception&)
{
  CDA_DocumentFragment* df = new CDA_DocumentFragment(this);
  return df;
}

already_AddRefd<iface::dom::Text>
CDA_Document::createTextNode(const std::wstring& data)
  throw(std::exception&)
{
  CDA_Text* tn = new CDA_Text(this);
  tn->mNodeValue = data;
  return tn;
}

already_AddRefd<iface::dom::Comment>
CDA_Document::createComment(const std::wstring& data)
  throw(std::exception&)
{
  CDA_Comment* cn = new CDA_Comment(this);
  cn->mNodeValue = data;
  return cn;
}

already_AddRefd<iface::dom::CDATASection>
CDA_Document::createCDATASection(const std::wstring& data)
  throw(std::exception&)
{
  CDA_CDATASection* cds = new CDA_CDATASection(this);
  cds->mNodeValue = data;
  return cds;
}

already_AddRefd<iface::dom::ProcessingInstruction>
CDA_Document::createProcessingInstruction
(
 const std::wstring& target,
 const std::wstring& data
)
  throw(std::exception&)
{
  return new CDA_ProcessingInstruction(this, target, data);
}

already_AddRefd<iface::dom::Attr>
CDA_Document::createAttribute(const std::wstring& name)
  throw(std::exception&)
{
  CDA_Attr* at = new CDA_Attr(this);
  at->mNodeName = name;
  return at;
}

already_AddRefd<iface::dom::EntityReference>
CDA_Document::createEntityReference(const std::wstring& name)
  throw(std::exception&)
{
  CDA_EntityReference* er = new CDA_EntityReference(this);
  er->mNodeName = name;
  return er;
}

already_AddRefd<iface::dom::NodeList>
CDA_Document::getElementsByTagName(const std::wstring& tagname)
  throw(std::exception&)
{
  return new CDA_NodeListDFSSearch(this, tagname);
}

already_AddRefd<iface::dom::Node>
CDA_Document::importNode(iface::dom::Node* importedNode, bool deep)
  throw(std::exception&)
{
  // XXX it would be very nice if this would work across CORBA, i.e. the
  // incoming node implements our interface but is not from this implementation.
  // In this case, we would have to do the clone ourselves using only
  // the standard interfaces.
  RETURN_INTO_OBJREF(cloned, iface::dom::Node, importedNode->cloneNode(deep));
  // Next we need to change the document all the way through...
  CDA_Node* n = dynamic_cast<CDA_Node*>(cloned.getPointer());

  // It makes no sense to change the document associated with a document...
  if (n->nodeType() == iface::dom::Node::DOCUMENT_NODE)
    throw iface::dom::DOMException(iface::dom::INVALID_MODIFICATION_ERR);

  n->recursivelyChangeDocument(this);
  n->add_ref();
  return n;
}

already_AddRefd<iface::dom::Element>
CDA_Document::createElementNS(const std::wstring& namespaceURI,
                              const std::wstring& qualifiedName)
  throw(std::exception&)
{
  const wchar_t* pos = wcschr(qualifiedName.c_str(), L':');
  if (pos == NULL)
    pos = qualifiedName.c_str();
  else
    pos++;
  CDA_Element* el = CDA_NewElement(this, namespaceURI, pos);
  el->mNamespaceURI = namespaceURI;
  el->mNodeName = qualifiedName;
  el->mLocalName = pos;
  return el;
}

already_AddRefd<iface::dom::Attr>
CDA_Document::createAttributeNS(const std::wstring& namespaceURI,
                                const std::wstring& qualifiedName)
  throw(std::exception&)
{
  CDA_Attr* at = new CDA_Attr(mDocument);
  at->mNamespaceURI = namespaceURI;
  at->mNodeName = qualifiedName;
  const wchar_t* pos = wcschr(qualifiedName.c_str(), L':');
  if (pos == NULL)
    at->mLocalName = qualifiedName;
  else
    at->mLocalName = pos + 1;
  return at;
}

already_AddRefd<iface::dom::NodeList>
CDA_Document::getElementsByTagNameNS(const std::wstring& namespaceURI,
                                     const std::wstring& localName)
  throw(std::exception&)
{
  return new CDA_NodeListDFSSearch(this, namespaceURI, localName);
}

already_AddRefd<iface::dom::Element>
CDA_Document::getElementById(const std::wstring& elementId)
  throw(std::exception&)
{
  return searchForElementById(elementId);
}

already_AddRefd<iface::events::Event>
CDA_Document::createEvent(const std::wstring& domEventType)
  throw(std::exception&)
{
  if (domEventType != L"DOMSubtreeModified" &&
      domEventType != L"DOMNodeInserted" &&
      domEventType != L"DOMNodeRemoved" &&
      domEventType != L"DOMNodeRemovedFromDocument" &&
      domEventType != L"DOMNodeInsertedIntoDocument" &&
      domEventType != L"DOMAttrModified" &&
      domEventType != L"DOMCharacterDataModified")
    throw iface::dom::DOMException(iface::dom::INVALID_STATE_ERR);

  return new CDA_MutationEvent();
}

already_AddRefd<CDA_Node>
CDA_Document::shallowCloneNode(CDA_Document* aDoc)
  throw(std::exception&)
{
  /* RETURN_INTO_OBJREFD(dt, CDA_DocumentType, doctype()); */
  CDA_Document* ca = new CDA_Document();
  ca->mLocalName = mLocalName;
  ca->mNodeValue = mNodeValue;
  return ca;
}

already_AddRefd<iface::dom::Element>
CDA_Document::searchForElementById(const std::wstring& elementId)
{
  std::list<CDA_Node*>::iterator i = mNodeList.begin();
  for (; i != mNodeList.end(); i++)
  {
    iface::dom::Element* e = (*i)->searchForElementById(elementId);
    if (e != NULL)
      return e;
  }

  return NULL;
}

std::wstring
CDA_MutationEvent::type()
  throw(std::exception&)
{
  return mType;
}

already_AddRefd<iface::events::EventTarget>
CDA_MutationEvent::target()
  throw(std::exception&)
{
  if (mTarget != NULL)
    mTarget->add_ref();
  return mTarget.getPointer();
}

already_AddRefd<iface::events::EventTarget>
CDA_MutationEvent::currentTarget()
  throw(std::exception&)
{
  if (mCurrentTarget != NULL)
    mCurrentTarget->add_ref();
  return mCurrentTarget.getPointer();
}

uint16_t
CDA_MutationEvent::eventPhase()
  throw(std::exception&)
{
  return mPhase;
}

bool
CDA_MutationEvent::bubbles()
  throw(std::exception&)
{
  return mBubbles;
}

bool
CDA_MutationEvent::cancelable()
  throw(std::exception&)
{
  return mCancelable;
}

uint64_t
CDA_MutationEvent::timeStamp()
  throw(std::exception&)
{
  return mTimeStamp;
}

void
CDA_MutationEvent::stopPropagation()
  throw(std::exception&)
{
  mPropagationStopped = true;
}

void
CDA_MutationEvent::preventDefault()
  throw(std::exception&)
{
  mCanceled = true;
}

void
CDA_MutationEvent::initEvent
(
 const std::wstring& eventTypeArg, bool canBubbleArg, bool cancelableArg
)
  throw(std::exception&)
{
  mType = eventTypeArg;
  mBubbles = canBubbleArg;
  mCancelable = cancelableArg;
}

already_AddRefd<iface::dom::Node>
CDA_MutationEvent::relatedNode()
  throw(std::exception&)
{
  if (mRelatedNode != NULL)
    mRelatedNode->add_ref();
  return mRelatedNode.getPointer();
}

std::wstring
CDA_MutationEvent::prevValue()
  throw(std::exception&)
{
  return mPrevValue;
}

std::wstring
CDA_MutationEvent::newValue()
  throw(std::exception&)
{
  return mNewValue;
}

std::wstring
CDA_MutationEvent::attrName()
  throw(std::exception&)
{
  return mAttrName;
}

uint16_t
CDA_MutationEvent::attrChange()
  throw(std::exception&)
{
  return mAttrChange;
}

void
CDA_MutationEvent::initMutationEvent
(
 const std::wstring& typeArg, bool canBubbleArg,
 bool cancelableArg, iface::dom::Node* relatedNodeArg,
 const std::wstring& prevValueArg, const std::wstring& newValueArg,
 const std::wstring& attrNameArg, uint16_t attrChangeArg
)
  throw(std::exception&)
{
  initEvent(typeArg, canBubbleArg, cancelableArg);
  mRelatedNode = relatedNodeArg;
  mPrevValue = prevValueArg;
  mNewValue = newValueArg;
  mAttrName = attrNameArg;
  mAttrChange = attrChangeArg;
}
