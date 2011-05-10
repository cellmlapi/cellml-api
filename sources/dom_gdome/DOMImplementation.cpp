#include "Utilities.hxx"
#include "DOMImplementation.hpp"
#include <string>
#include "libxml2/libxml/xmlerror.h"

std::map<iface::events::EventListener*,CDA_Node::EventListenerData*> CDA_Node::activeEventListeners;
CDAMutex CDA_Node::mStaticMutex;

CDA_Node*
CDA_Node::findExistingWrapper(GdomeNode* n)
{
  return reinterpret_cast<CDA_Node*>(n->user_data);
}

CDA_DOMImplementation::CDA_DOMImplementation()
  : _cda_refcount(1), impl(gdome_di_mkref())
{
  /* The docs say this is okay, although it doesn't look thread safe... */
  if (!g_thread_supported())
    g_thread_init(NULL);
}

CDA_DOMImplementation::CDA_DOMImplementation(GdomeDOMImplementation* iimpl)
  : _cda_refcount(1), impl(iimpl)
{
  /* The docs say this is okay, although it doesn't look thread safe... */
  if (!g_thread_supported())
    g_thread_init(NULL);
}

CDA_DOMImplementation::~CDA_DOMImplementation()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_di_unref(impl, &exc);
}

bool
CDA_DOMImplementation::hasFeature
(
 const wchar_t* feature,
 const wchar_t* version
)
  throw(std::exception&)
{
  if (!wcscmp(feature, L"org.w3c.dom.mathml"))
  {
    return !wcscmp(version, L"2.0");
  }

  EXCEPTION_TRY;
  TRDOMSTRING(feature);
  TRDOMSTRING(version);
  bool ret = gdome_di_hasFeature(impl, gdfeature, gdversion, &exc);
  DDOMSTRING(feature);
  DDOMSTRING(version);
  EXCEPTION_CATCH;
  
  return ret;
}

iface::dom::DocumentType*
CDA_DOMImplementation::createDocumentType
(
 const wchar_t* qualifiedName,
 const wchar_t* publicId,
 const wchar_t* systemId
)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  TRDOMSTRING(qualifiedName);
  TRDOMSTRING(publicId);
  TRDOMSTRING(systemId);
  GdomeDocumentType* gdt =
    gdome_di_createDocumentType(impl, gdqualifiedName, gdpublicId,
                                gdsystemId, &exc);
  DDOMSTRING(qualifiedName);
  DDOMSTRING(publicId);
  DDOMSTRING(systemId);
  EXCEPTION_CATCH;
  
  if (gdt == NULL)
    return NULL;
  return CDA_DocumentType::wrap(gdt);
}

iface::dom::Document*
CDA_DOMImplementation::createDocument
(
 const wchar_t* namespaceURI,
 const wchar_t* qualifiedName,
 iface::dom::DocumentType* doctype
)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(qualifiedName);
  LOCALCONVERT_NULLOK(doctype, DocumentType)

  GdomeDocument* gd =
    gdome_di_createDocument(impl, gdnamespaceURI, gdqualifiedName,
                            ldoctype ? ldoctype->impl : NULL, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(qualifiedName);
  EXCEPTION_CATCH;
  
  return CDA_WrapDocument(gd);
}

iface::dom::Document*
CDA_DOMImplementation::loadDocument(const wchar_t* sourceURL,
                                    std::wstring& errorMessage)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  char* sourceURL8 = g_wchar_to_char((const gwchar_t*)sourceURL, -1, NULL,
                                     NULL, NULL);
  errorMessage = L"";

  GdomeDocument* gd =
    gdome_di_createDocFromURI(impl, sourceURL8, GDOME_LOAD_PARSING, &exc);
  free(sourceURL8);
  if (gd != NULL && GDOME_EXCEPTION_CODE(exc) == GDOME_NOEXCEPTION_ERR)
    return CDA_WrapDocument(gd);

  // Otherwise, we have an exception to deal with. We do this by creating a
  // fake document to hold the exception information.
  xmlErrorPtr ep = xmlGetLastError();
  switch (ep->domain)
  {
  case XML_FROM_PARSER:
  case XML_FROM_TREE:
  case XML_FROM_NAMESPACE:
  case XML_FROM_HTML:
    {
      wchar_t buf[27];
      errorMessage = L"badxml/";
      // badxml/line/column/msg
      swprintf(buf, 27, L"%u/%u/", ep->line, ep->int2);
      errorMessage += buf;
      if (ep->message)
      {
        wchar_t* msg = (wchar_t*)g_char_to_wchar(ep->message, -1, NULL
                                                 POSSIBLE_EXTRA_NULLS);
        errorMessage += msg;
        free(msg);
      }
    }
    break;
  case XML_FROM_MEMORY:
    errorMessage = L"nomemory";
    break;
  case XML_FROM_IO:
    // This could also result from a malformed URL. We need better detection.
    errorMessage = L"ioerror";
    break;
  case XML_FROM_FTP:
  case XML_FROM_HTTP:
    // or servererror?
    errorMessage = L"badurl";
    break;
  default:
    errorMessage = L"othererror";
    break;
  }
  return NULL;
}

wchar_t*
CDA_Node::nodeName()
  throw(std::exception&)
{
  GdomeDOMString* str;
  EXCEPTION_TRY;
  str = gdome_n_nodeName(mNode, &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(str);
  return cxxstr;
}

wchar_t*
CDA_Node::nodeValue()
  throw(std::exception&)
{
  GdomeDOMString *str;
  EXCEPTION_TRY;
  str = gdome_n_nodeValue(mNode, &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(str);
  return cxxstr;
}

void
CDA_Node::nodeValue(const wchar_t* attr)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  TRDOMSTRING(attr);
  gdome_n_set_nodeValue(mNode, gdattr, &exc);
  DDOMSTRING(attr);
  EXCEPTION_CATCH;
}

uint16_t
CDA_Node::nodeType()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  uint16_t ret = gdome_n_nodeType(mNode, &exc);
  EXCEPTION_CATCH;
  return ret;
}

iface::dom::Node*
CDA_Node::parentNode()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_parentNode(mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::NodeList*
CDA_Node::childNodes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNodeList* ret = gdome_n_childNodes(mNode, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NodeList(ret);
}

iface::dom::Node*
CDA_Node::firstChild()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_firstChild(mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::lastChild()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_lastChild(mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::previousSibling()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_previousSibling(mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::nextSibling()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_nextSibling(mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::NamedNodeMap*
CDA_Node::attributes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNamedNodeMap* ret = gdome_n_attributes(mNode, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NamedNodeMap(ret);
}

iface::dom::Document*
CDA_Node::ownerDocument()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDocument* ret = gdome_n_ownerDocument(mNode, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return CDA_WrapDocument(ret);
}

iface::dom::Node*
CDA_Node::insertBefore(iface::dom::Node* newChild,
                       iface::dom::Node* refChild)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  LOCALCONVERT(newChild, Node);
  LOCALCONVERT(refChild, Node);
  GdomeNode* ret = gdome_n_insertBefore(mNode, lnewChild->mNode,
                                        lrefChild->mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::replaceChild(iface::dom::Node* newChild,
                       iface::dom::Node* oldChild)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  LOCALCONVERT(newChild, Node);
  LOCALCONVERT(oldChild, Node);
  GdomeNode* ret = gdome_n_replaceChild(mNode, lnewChild->mNode,
                                        loldChild->mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::removeChild(iface::dom::Node* oldChild)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  LOCALCONVERT(oldChild, Node);
  GdomeNode* ret = gdome_n_removeChild(mNode, loldChild->mNode,
                                       &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::appendChild(iface::dom::Node* newChild)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  LOCALCONVERT(newChild, Node);
  GdomeNode* ret = gdome_n_appendChild(mNode, lnewChild->mNode,
                                       &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

bool
CDA_Node::hasChildNodes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_n_hasChildNodes(mNode, &exc);
  EXCEPTION_CATCH;

  return ret;
}

iface::dom::Node*
CDA_Node::cloneNode(bool deep)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_cloneNode(mNode, deep, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

void
CDA_Node::normalize()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_n_normalize(mNode, &exc);
  EXCEPTION_CATCH;
}

bool
CDA_Node::isSupported
(
 const wchar_t* feature,
 const wchar_t* version
)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  TRDOMSTRING(feature);
  TRDOMSTRING(version);
  bool ret = gdome_n_isSupported(mNode, gdfeature, gdversion, &exc);
  DDOMSTRING(feature);
  DDOMSTRING(version);
  EXCEPTION_CATCH;

  return ret;
}

wchar_t*
CDA_Node::namespaceURI()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_n_namespaceURI(mNode, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

wchar_t*
CDA_Node::prefix()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_n_prefix(mNode, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

void
CDA_Node::prefix
(
 const wchar_t* attr
)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  TRDOMSTRING(attr);
  gdome_n_set_prefix(mNode, gdattr, &exc);
  DDOMSTRING(attr);
  EXCEPTION_CATCH;
}

wchar_t*
CDA_Node::localName()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_n_localName(mNode, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

bool
CDA_Node::hasAttributes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_n_hasAttributes(mNode, &exc);
  EXCEPTION_CATCH;

  return ret;
}

static void cda_process_event(GdomeEventListener *self,
                              GdomeEvent *event,
                              GdomeException *exc)
{
  iface::events::EventListener* l =
    reinterpret_cast<iface::events::EventListener*>(gdome_evntl_get_priv(self));
  gdome_evnt_ref(event, exc);
  RETURN_INTO_OBJREF(e, iface::events::Event, CDA_WrapEvent(event));
  l->handleEvent(e);
}

void
CDA_Node::addEventListener(const wchar_t* type,
                           iface::events::EventListener* listener,
                           bool useCapture)
  throw(std::exception&)
{
  CDALock scopedLock(mStaticMutex);

  // Look up the listener in the AEL table...
  std::map<iface::events::EventListener*,struct EventListenerData*>::iterator i =
    activeEventListeners.find(listener);
  EventListenerData* ed;
  if (i == activeEventListeners.end())
  {
    ed = new EventListenerData;
    ed->copies = 0;
    // The EventListenerData is guaranteed to outlive both the Gdome listener
    // and the iface::events::EventListener structure, so there is no need to
    // call AddRef().
    ed->listener = gdome_evntl_mkref(cda_process_event, listener);
    ed->callee = listener;
    activeEventListeners.insert(std::pair<iface::events::EventListener*,
                                struct EventListenerData*>(listener, ed));
    listener->add_ref();
  }
  else
  {
    ed = (*i).second;
  }

  TRDOMSTRING(type);
  EXCEPTION_TRY;
  gdome_n_addEventListener(mNode, gdtype, ed->listener, useCapture,
                           &exc);
  DDOMSTRING(type);

  if (GDOME_EXCEPTION_CODE(exc) != GDOME_NOEXCEPTION_ERR)
  {
    if (ed->copies == 0)
    {
      activeEventListeners.erase(listener);
      delete ed;
    }
    throw iface::dom::DOMException();
  }

  // Increment the number of copies...
  ed->copies++;
}

void
CDA_Node::removeEventListener(const wchar_t* type,
                              iface::events::EventListener* listener,
                              bool useCapture)
  throw(std::exception&)
{
  CDALock scopedLock(mStaticMutex);
  std::map<iface::events::EventListener*,struct EventListenerData*>::iterator i =
    activeEventListeners.find(listener);
  EventListenerData* ed;
  if (i == activeEventListeners.end())
    throw iface::dom::DOMException();

  ed = (*i).second;

  TRDOMSTRING(type);
  EXCEPTION_TRY;
  gdome_n_removeEventListener(mNode, gdtype, ed->listener, useCapture,
                              &exc);
  DDOMSTRING(type);
  EXCEPTION_CATCH;

  // If we have removed the last copy, destroy it...
  if (--ed->copies == 0)
  {
    activeEventListeners.erase(listener);
    delete ed;
  }
}

bool
CDA_Node::dispatchEvent(iface::events::Event* evt)
  throw(std::exception&)
{
  LOCALCONVERT(evt, Event);
  EXCEPTION_TRY;
  bool ret = gdome_n_dispatchEvent(mNode, levt->fetchEvent(), &exc);
  EXCEPTION_CATCH;

  return ret;
}

#if 0 // If ever re-enabled, this has to be fixed, because compare changed to
      // id.
int32_t
CDA_Node::compare(iface::XPCOM::IObject* obj)
  throw(std::exception&)
{
  CDA_Node* cn = dynamic_cast<CDA_Node*>(obj);
  // It is essential that the this comes first as the ordering for distinct
  // objects must maintain the reflexive property.
  if (cn == NULL)
    return reinterpret_cast<char*>(this) - reinterpret_cast<char*>(obj);

  return reinterpret_cast<char*>(mNode) -
    reinterpret_cast<char*>(cn->mNode);
}
#endif

CDA_NodeList::CDA_NodeList(GdomeNodeList* nl)
  : _cda_refcount(1), impl(nl)
{
}

CDA_NodeList::~CDA_NodeList()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_nl_unref(impl, &exc);
}

iface::dom::Node*
CDA_NodeList::item(uint32_t index)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nl_item(impl, index, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

uint32_t
CDA_NodeList::length()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  uint32_t ret = gdome_nl_length(impl, &exc);
  EXCEPTION_CATCH;

  return ret;
}

CDA_NamedNodeMap::CDA_NamedNodeMap(GdomeNamedNodeMap* nnm)
  : _cda_refcount(1), impl(nnm)
{
}

CDA_NamedNodeMap::~CDA_NamedNodeMap()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_nnm_unref(impl, &exc);
}

iface::dom::Node*
CDA_NamedNodeMap::getNamedItem(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_getNamedItem(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_NamedNodeMap::setNamedItem(iface::dom::Node* arg)
  throw(std::exception&)
{
  LOCALCONVERT(arg, Node);
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_setNamedItem(impl, larg->mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_NamedNodeMap::removeNamedItem(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_removeNamedItem(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_NamedNodeMap::item(uint32_t index)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_item(impl, index, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

uint32_t
CDA_NamedNodeMap::length()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  uint32_t ret = gdome_nnm_length(impl, &exc);
  EXCEPTION_CATCH;

  return ret;
}

iface::dom::Node*
CDA_NamedNodeMap::getNamedItemNS(const wchar_t* namespaceURI,
                                 const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_getNamedItemNS(impl, gdnamespaceURI, gdlocalName,
                                            &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_NamedNodeMap::setNamedItemNS(iface::dom::Node* arg)
  throw(std::exception&)
{
  LOCALCONVERT(arg, Node);
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_setNamedItemNS(impl, larg->mNode, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_NamedNodeMap::removeNamedItemNS(const wchar_t* namespaceURI,
                                    const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_removeNamedItemNS(impl, gdnamespaceURI,
                                               gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

wchar_t*
CDA_CharacterData::data()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_cd_data(fetchCData(), &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

void
CDA_CharacterData::data(const wchar_t* d)
  throw(std::exception&)
{
  TRDOMSTRING(d);
  EXCEPTION_TRY;
  gdome_cd_set_data(fetchCData(), gdd, &exc);
  DDOMSTRING(d);
  EXCEPTION_CATCH;
}


uint32_t
CDA_CharacterData::length()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  uint32_t ret = gdome_cd_length(fetchCData(), &exc);
  EXCEPTION_CATCH;  

  return ret;
}

wchar_t*
CDA_CharacterData::substringData(uint32_t offset, uint32_t count)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret =
    gdome_cd_substringData(fetchCData(), offset, count, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

void
CDA_CharacterData::appendData(const wchar_t* arg)
  throw(std::exception&)
{
  TRDOMSTRING(arg);

  EXCEPTION_TRY;
  gdome_cd_appendData(fetchCData(), gdarg, &exc);
  DDOMSTRING(arg);
  EXCEPTION_CATCH;
}

void
CDA_CharacterData::insertData(uint32_t offset, const wchar_t* arg)
  throw(std::exception&)
{
  TRDOMSTRING(arg);
  EXCEPTION_TRY;
  gdome_cd_insertData(fetchCData(), offset, gdarg, &exc);
  DDOMSTRING(arg);
  EXCEPTION_CATCH;
}

void
CDA_CharacterData::deleteData(uint32_t offset, uint32_t count)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_cd_deleteData(fetchCData(), offset, count, &exc);
  EXCEPTION_CATCH;
}

void
CDA_CharacterData::replaceData(uint32_t offset, uint32_t count,
                               const wchar_t* arg)
  throw(std::exception&)
{
  TRDOMSTRING(arg);
  EXCEPTION_TRY;
  gdome_cd_replaceData(fetchCData(), offset, count, gdarg, &exc);
  DDOMSTRING(arg);
  EXCEPTION_CATCH;
}

CDA_Attr::CDA_Attr(GdomeAttr* at)
  : CDA_Node(GDOME_N(at)), _cda_refcount(1), impl(at)
{
}

CDA_Attr::~CDA_Attr()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_a_unref(impl, &exc);
}

wchar_t*
CDA_Attr::name()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_a_name(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}


bool
CDA_Attr::specified()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_a_specified(impl, &exc);
  EXCEPTION_CATCH;

  return ret;
}

wchar_t*
CDA_Attr::value()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_a_value(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

void
CDA_Attr::value(const wchar_t* attr)
  throw(std::exception&)
{
  TRDOMSTRING(attr);
  EXCEPTION_TRY;
  gdome_a_set_value(impl, gdattr, &exc);
  DDOMSTRING(attr);
  EXCEPTION_CATCH;
}

iface::dom::Element*
CDA_Attr::ownerElement()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeElement* ret = gdome_a_ownerElement(impl, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapElement(ret);
}

CDA_Element::CDA_Element(GdomeElement* el)
  : CDA_Node(GDOME_N(el)), _cda_refcount(1), impl(el)
{
}

CDA_Element::~CDA_Element()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_el_unref(impl, &exc);
}

wchar_t*
CDA_Element::tagName()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_el_tagName(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

wchar_t*
CDA_Element::getAttribute(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_el_getAttribute(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

void
CDA_Element::setAttribute(const wchar_t* name,
                          const wchar_t* value)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  TRDOMSTRING(value);
  EXCEPTION_TRY;
  gdome_el_setAttribute(impl, gdname, gdvalue, &exc);
  DDOMSTRING(name);
  DDOMSTRING(value);
  EXCEPTION_CATCH;
}

void
CDA_Element::removeAttribute(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  gdome_el_removeAttribute(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;
}

iface::dom::Attr*
CDA_Element::getAttributeNode(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_getAttributeNode(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return CDA_Attr::wrap(ret);
}

iface::dom::Attr*
 CDA_Element::setAttributeNode(iface::dom::Attr* newAttr)
  throw(std::exception&)
{
  LOCALCONVERT(newAttr, Attr);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_setAttributeNode(impl, lnewAttr->impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
  {
    gdome_a_ref(lnewAttr->impl, &exc);
    return NULL;
  }
  return CDA_Attr::wrap(ret);
}

iface::dom::Attr* CDA_Element::removeAttributeNode(iface::dom::Attr* oldAttr)
  throw(std::exception&)
{
  LOCALCONVERT(oldAttr, Attr);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_removeAttributeNode(impl, loldAttr->impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return CDA_Attr::wrap(ret);
}

iface::dom::NodeList* CDA_Element::getElementsByTagName(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeNodeList* ret = gdome_el_getElementsByTagName(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NodeList(ret);
}

wchar_t*
CDA_Element::getAttributeNS(const wchar_t* namespaceURI,
                            const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeDOMString* ret =
    gdome_el_getAttributeNS(impl, gdnamespaceURI, gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

void
CDA_Element::setAttributeNS(const wchar_t* namespaceURI,
                            const wchar_t* qualifiedName,
                            const wchar_t* value)
  throw(std::exception&)
{ 
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(qualifiedName);
  TRDOMSTRING(value);
  EXCEPTION_TRY;
  gdome_el_setAttributeNS(impl, gdnamespaceURI, gdqualifiedName, gdvalue,
                          &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(qualifiedName);
  DDOMSTRING(value);
  EXCEPTION_CATCH;
}

void
CDA_Element::removeAttributeNS(const wchar_t* namespaceURI,
                               const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  gdome_el_removeAttributeNS(impl, gdnamespaceURI, gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;
}

iface::dom::Attr*
CDA_Element::getAttributeNodeNS(const wchar_t* namespaceURI,
                                const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_getAttributeNodeNS(impl, gdnamespaceURI,
                                               gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return CDA_Attr::wrap(ret);
}

iface::dom::Attr* CDA_Element::setAttributeNodeNS(iface::dom::Attr* newAttr)
  throw(std::exception&)
{
  LOCALCONVERT(newAttr, Attr);
  EXCEPTION_TRY;
  GdomeAttr* ret =
    gdome_el_setAttributeNodeNS(impl, lnewAttr->impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
  {
    return NULL;
  }

  return CDA_Attr::wrap(ret);
}

iface::dom::NodeList*
CDA_Element::getElementsByTagNameNS(const wchar_t* namespaceURI,
                                    const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeNodeList* ret =
    gdome_el_getElementsByTagNameNS(impl, gdnamespaceURI, gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NodeList(ret);
}

bool
CDA_Element::hasAttribute(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  bool ret = gdome_el_hasAttribute(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  return ret;
}

bool
CDA_Element::hasAttributeNS(const wchar_t* namespaceURI,
                            const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  bool ret = gdome_el_hasAttributeNS(impl, gdnamespaceURI, gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  return ret;
}

iface::dom::Text*
CDA_TextBase::splitText(uint32_t offset)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeText* ret = gdome_t_splitText(fetchText(), offset, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return CDA_Text::wrap(ret);
}

CDA_Text::CDA_Text(GdomeText* txt)
  : CDA_TextBase(GDOME_N(txt)), _cda_refcount(1), impl(txt)
{
}

CDA_Text::~CDA_Text()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_t_unref(impl, &exc);
}

GdomeCharacterData*
CDA_Text::fetchCData() const
{
  return GDOME_CD(impl);
}

GdomeText*
CDA_Text::fetchText() const
{
  return impl;
}

CDA_Comment::CDA_Comment(GdomeComment* c)
  : CDA_CharacterData(GDOME_N(c)), _cda_refcount(1), impl(c)
{
}

CDA_Comment::~CDA_Comment()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_c_unref(impl, &exc);
}

GdomeCharacterData*
CDA_Comment::fetchCData() const
{
  return GDOME_CD(impl);
}

CDA_CDATASection::CDA_CDATASection(GdomeCDATASection* cds)
  : CDA_TextBase(GDOME_N(cds)), _cda_refcount(1), impl(cds)
{
}

CDA_CDATASection::~CDA_CDATASection()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_cds_unref(impl, &exc);
}

GdomeCharacterData*
CDA_CDATASection::fetchCData() const
{
  return GDOME_CD(impl);
}

GdomeText*
CDA_CDATASection::fetchText() const
{
  return GDOME_T(impl);
}

CDA_DocumentType::CDA_DocumentType(GdomeDocumentType* dt)
  : CDA_Node(GDOME_N(dt)), _cda_refcount(1), impl(dt)
{
}

CDA_DocumentType::~CDA_DocumentType()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_dt_unref(impl, &exc);
}

wchar_t*
CDA_DocumentType::name()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_dt_name(impl, &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(ret);

  return cxxret;
}

iface::dom::NamedNodeMap*
CDA_DocumentType::entities()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNamedNodeMap* ret = gdome_dt_entities(impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NamedNodeMap(ret);
}

iface::dom::NamedNodeMap*
CDA_DocumentType::notations()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNamedNodeMap* ret = gdome_dt_notations(impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NamedNodeMap(ret);
}

wchar_t*
CDA_DocumentType::publicId()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_dt_publicId(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

wchar_t*
CDA_DocumentType::systemId()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_dt_systemId(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

wchar_t*
CDA_DocumentType::internalSubset()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_dt_internalSubset(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

CDA_Notation::CDA_Notation(GdomeNotation* nt)
  : CDA_Node(GDOME_N(nt)), _cda_refcount(1), impl(nt)
{
}

CDA_Notation::~CDA_Notation()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_not_unref(impl, &exc);
}

wchar_t*
CDA_Notation::publicId()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_not_publicId(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

wchar_t*
CDA_Notation::systemId()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_not_systemId(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

CDA_Entity::CDA_Entity(GdomeEntity* ent)
  : CDA_Node(GDOME_N(ent)), _cda_refcount(1), impl(ent)
{
}

CDA_Entity::~CDA_Entity()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_ent_unref(impl, &exc);
}

wchar_t*
CDA_Entity::publicId()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_ent_publicId(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

wchar_t*
CDA_Entity::systemId()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_ent_systemId(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

wchar_t*
CDA_Entity::notationName()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_ent_notationName(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

CDA_EntityReference::CDA_EntityReference(GdomeEntityReference* ent)
  : CDA_Node(GDOME_N(ent)), _cda_refcount(1), impl(ent)
{
}

CDA_EntityReference::~CDA_EntityReference()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_er_unref(impl, &exc);
}

CDA_ProcessingInstruction::CDA_ProcessingInstruction
(GdomeProcessingInstruction* pri)
  : CDA_Node(GDOME_N(pri)), _cda_refcount(1), impl(pri)
{
}

CDA_ProcessingInstruction::~CDA_ProcessingInstruction()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_pi_unref(impl, &exc);
}

wchar_t*
CDA_ProcessingInstruction::target()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_pi_target(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

wchar_t*
CDA_ProcessingInstruction::data()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_pi_data(impl, &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);
  return cxxret;
}

void
CDA_ProcessingInstruction::data(const wchar_t* attr)
  throw(std::exception&)
{
  TRDOMSTRING(attr);
  EXCEPTION_TRY;
  gdome_pi_set_data(impl, gdattr, &exc);
  DDOMSTRING(attr);
  EXCEPTION_CATCH;
}

CDA_DocumentFragment::CDA_DocumentFragment(GdomeDocumentFragment* df)
  : CDA_Node(GDOME_N(df)), _cda_refcount(1), impl(df)
{
}

CDA_DocumentFragment::~CDA_DocumentFragment()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_df_unref(impl, &exc);
}

CDA_Document::CDA_Document(GdomeDocument* doc)
  : CDA_Node(GDOME_N(doc)), _cda_refcount(1), impl(doc)
{
}

CDA_Document::~CDA_Document()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_doc_unref(impl, &exc);
}

iface::dom::DocumentType*
CDA_Document::doctype()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDocumentType* ret = gdome_doc_doctype(impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return CDA_DocumentType::wrap(ret);
}

iface::dom::DOMImplementation*
CDA_Document::implementation()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMImplementation* ret = gdome_doc_implementation(impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_DOMImplementation(ret);
}

iface::dom::Element*
CDA_Document::documentElement()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeElement* ret = gdome_doc_documentElement(impl, &exc);
  EXCEPTION_CATCH;
  return CDA_WrapElement(ret);
}

iface::dom::Element*
CDA_Document::createElement(const wchar_t* tagName)
  throw(std::exception&)
{
  TRDOMSTRING(tagName);
  EXCEPTION_TRY;
  GdomeElement* ret = gdome_doc_createElement(impl, gdtagName, &exc);
  DDOMSTRING(tagName);
  EXCEPTION_CATCH;
  // No namespace => No special type.

  if (ret == NULL)
    return NULL;
  return new CDA_Element(ret);
}

iface::dom::DocumentFragment*
CDA_Document::createDocumentFragment()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDocumentFragment* ret = gdome_doc_createDocumentFragment(impl, &exc);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_DocumentFragment(ret);
}

iface::dom::Text*
CDA_Document::createTextNode(const wchar_t* data)
  throw(std::exception&)
{
  TRDOMSTRING(data);
  EXCEPTION_TRY;
  GdomeText* ret = gdome_doc_createTextNode(impl, gddata, &exc);
  DDOMSTRING(data);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_Text(ret);
}

iface::dom::Comment*
CDA_Document::createComment(const wchar_t* data)
  throw(std::exception&)
{
  TRDOMSTRING(data);
  EXCEPTION_TRY;
  GdomeComment* ret = gdome_doc_createComment(impl, gddata, &exc);
  DDOMSTRING(data);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_Comment(ret);
}

iface::dom::CDATASection*
CDA_Document::createCDATASection(const wchar_t* data)
  throw(std::exception&)
{
  TRDOMSTRING(data);
  EXCEPTION_TRY;
  GdomeCDATASection* ret = gdome_doc_createCDATASection(impl, gddata, &exc);
  DDOMSTRING(data);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_CDATASection(ret);
}

iface::dom::ProcessingInstruction*
CDA_Document::createProcessingInstruction
(const wchar_t* target, const wchar_t* data)
  throw(std::exception&)
{
  TRDOMSTRING(target);
  TRDOMSTRING(data);
  EXCEPTION_TRY;
  GdomeProcessingInstruction* ret =
    gdome_doc_createProcessingInstruction(impl, gdtarget, gddata, &exc);
  DDOMSTRING(target);
  DDOMSTRING(data);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_ProcessingInstruction(ret);
}

iface::dom::Attr*
CDA_Document::createAttribute(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_doc_createAttribute(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_Attr(ret);
}

iface::dom::EntityReference*
CDA_Document::createEntityReference
(const wchar_t* name) throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeEntityReference* ret = gdome_doc_createEntityReference(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_EntityReference(ret);
}

iface::dom::NodeList*
CDA_Document::getElementsByTagName(const wchar_t* tagname)
  throw(std::exception&)
{
  TRDOMSTRING(tagname);
  EXCEPTION_TRY;
  GdomeNodeList* ret = gdome_doc_getElementsByTagName(impl, gdtagname, &exc);
  DDOMSTRING(tagname);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NodeList(ret);
}

iface::dom::Node*
CDA_Document::importNode(iface::dom::Node* importedNode, bool deep)
  throw(std::exception&)
{
  LOCALCONVERT(importedNode, Node)
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_doc_importNode(impl, limportedNode->mNode, deep, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Element*
CDA_Document::createElementNS(const wchar_t* namespaceURI,
                              const wchar_t* qualifiedName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(qualifiedName);
  EXCEPTION_TRY;
  GdomeElement* ret = gdome_doc_createElementNS(impl, gdnamespaceURI, gdqualifiedName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(qualifiedName);
  EXCEPTION_CATCH;

  return CDA_WrapElement(ret);
}

iface::dom::Attr*
CDA_Document::createAttributeNS(const wchar_t* namespaceURI,
                                const wchar_t* qualifiedName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(qualifiedName);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_doc_createAttributeNS(impl, gdnamespaceURI, gdqualifiedName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(qualifiedName);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_Attr(ret);
}

iface::dom::NodeList*
CDA_Document::getElementsByTagNameNS
(const wchar_t* namespaceURI,
 const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING_EMPTYNULL(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeNodeList* ret = gdome_doc_getElementsByTagNameNS(impl, gdnamespaceURI,
                                                        gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  if (ret == NULL)
    return NULL;
  return new CDA_NodeList(ret);
}

iface::dom::Element*
CDA_Document::getElementById(const wchar_t* elementId)
  throw(std::exception&)
{
  TRDOMSTRING(elementId);
  EXCEPTION_TRY;
  GdomeElement* ret = gdome_doc_getElementById(impl, gdelementId, &exc);
  DDOMSTRING(elementId);
  EXCEPTION_CATCH;

  return CDA_WrapElement(ret);
}

iface::events::Event*
CDA_Document::createEvent(const wchar_t* domEventType)
  throw(std::exception&)
{
  TRDOMSTRING(domEventType);
  EXCEPTION_TRY;
  GdomeEvent* ret = gdome_doc_createEvent(impl, gddomEventType, &exc);
  DDOMSTRING(domEventType);
  EXCEPTION_CATCH;

  return CDA_WrapEvent(ret);
}
  
iface::events::DOMString
CDA_EventBase::type()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_evnt_type(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(ret);
  return cxxret;
}

iface::dom::Node*
CDA_EventBase::target()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_evnt_target(fetchEvent(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_EventBase::currentTarget()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_evnt_currentTarget(fetchEvent(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

uint16_t
CDA_EventBase::eventPhase()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  uint16_t ret = gdome_evnt_eventPhase(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

bool
CDA_EventBase::bubbles()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_evnt_bubbles(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

bool
CDA_EventBase::cancelable()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_evnt_cancelable(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

iface::events::DOMTimeStamp
CDA_EventBase::timeStamp()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  iface::events::DOMTimeStamp ret = gdome_evnt_timeStamp(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

void
CDA_EventBase::stopPropagation()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_evnt_stopPropagation(fetchEvent(), &exc);
  EXCEPTION_CATCH;
}

void
CDA_EventBase::preventDefault()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_evnt_preventDefault(fetchEvent(), &exc);
  EXCEPTION_CATCH;
}

void
CDA_EventBase::initEvent
(
 const wchar_t* eventTypeArg, bool canBubbleArg,
 bool cancelableArg
)
  throw(std::exception&)
{
  TRDOMSTRING(eventTypeArg);
  EXCEPTION_TRY;
  gdome_evnt_initEvent(fetchEvent(), gdeventTypeArg, canBubbleArg, cancelableArg, &exc);
  DDOMSTRING(eventTypeArg);
  EXCEPTION_CATCH;
}

CDA_Event::CDA_Event(GdomeEvent* evt)
  : _cda_refcount(1), impl(evt)
{
}

CDA_Event::~CDA_Event()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_evnt_unref(impl, &exc);
}

GdomeEvent*
CDA_Event::fetchEvent()
  const
{
  return impl;
}

CDA_MutationEvent::CDA_MutationEvent(GdomeMutationEvent* me)
  : _cda_refcount(1), impl(me)
{
}

CDA_MutationEvent::~CDA_MutationEvent()
{
  EXCEPTION_TRY;
  impl->user_data = NULL;
  gdome_mevnt_unref(impl, &exc);
}

iface::events::Node
CDA_MutationEvent::relatedNode()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_mevnt_relatedNode(impl, &exc);
  EXCEPTION_CATCH;
  return CDA_WrapNode(ret);
}

iface::events::DOMString
CDA_MutationEvent::prevValue()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_mevnt_prevValue(impl, &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(ret);
  return cxxret;
}

iface::events::DOMString
CDA_MutationEvent::newValue()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_mevnt_newValue(impl, &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(ret);
  return cxxret;
}

iface::events::DOMString
CDA_MutationEvent::attrName()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_mevnt_attrName(impl, &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(ret);
  return cxxret;
}

uint16_t
CDA_MutationEvent::attrChange()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  uint16_t ret = gdome_mevnt_attrChange(impl, &exc);
  EXCEPTION_CATCH;
  return ret;
}

void
CDA_MutationEvent::initMutationEvent
(
 const wchar_t* typeArg,
 bool canBubbleArg, bool cancelableArg,
 iface::events::Node relatedNodeArg,
 const wchar_t* prevValueArg,
 const wchar_t* newValueArg,
 const wchar_t* attrNameArg,
 uint16_t attrChangeArg
)
  throw(std::exception&)
{
  TRDOMSTRING(typeArg);
  TRDOMSTRING(prevValueArg);
  TRDOMSTRING(newValueArg);
  TRDOMSTRING(attrNameArg);
  LOCALCONVERT(relatedNodeArg, Node);
  EXCEPTION_TRY;
  gdome_mevnt_initMutationEvent(impl, gdtypeArg, canBubbleArg, cancelableArg,
                                lrelatedNodeArg->mNode, gdprevValueArg,
                                gdnewValueArg, gdattrNameArg, attrChangeArg,
                                &exc);
  DDOMSTRING(typeArg);
  DDOMSTRING(prevValueArg);
  DDOMSTRING(newValueArg);
  DDOMSTRING(attrNameArg);
  EXCEPTION_CATCH;
}

GdomeEvent*
CDA_MutationEvent::fetchEvent() const
{
  return GDOME_EVNT(impl);
}

iface::dom::Node*
CDA_WrapNode(GdomeNode* n)
{
  if (n == NULL)
    return NULL;
  // We need to figure out what type of node it is.
  EXCEPTION_TRY;
  switch (gdome_n_nodeType(n, &exc))
  {
  case GDOME_ELEMENT_NODE:
  case GDOME_XPATH_NAMESPACE_NODE:
    return CDA_WrapElement(GDOME_EL(n));
  case GDOME_ATTRIBUTE_NODE:
    return CDA_Attr::wrap(GDOME_A(n));
  case GDOME_TEXT_NODE:
    return CDA_Text::wrap(GDOME_T(n));
  case GDOME_CDATA_SECTION_NODE:
    return CDA_CDATASection::wrap(GDOME_CDS(n));
  case GDOME_ENTITY_REFERENCE_NODE:
    return CDA_EntityReference::wrap(GDOME_ER(n));
  case GDOME_ENTITY_NODE:
    return CDA_Entity::wrap(GDOME_ENT(n));
  case GDOME_PROCESSING_INSTRUCTION_NODE:
    return CDA_ProcessingInstruction::wrap(GDOME_PI(n));
  case GDOME_COMMENT_NODE:
    return CDA_Comment::wrap(GDOME_C(n));
  case GDOME_DOCUMENT_NODE:
    return CDA_WrapDocument(GDOME_DOC(n));
  case GDOME_DOCUMENT_TYPE_NODE:
    return CDA_DocumentType::wrap(GDOME_DT(n));
  case GDOME_DOCUMENT_FRAGMENT_NODE:
    return CDA_DocumentFragment::wrap(GDOME_DF(n));
  case GDOME_NOTATION_NODE:
    return CDA_Notation::wrap(GDOME_NOT(n));
  }
  // This is not supposed to happen.
  return NULL;
}

struct CDARegisteredNamespace
{
public:
  CDARegisteredNamespace(std::string name,
                         iface::dom::Element* (*iwrapElement)(GdomeElement*),
                         iface::dom::Document* (*iwrapDocument)(GdomeDocument*)
                        );
  iface::dom::Element* (*wrapElement)(GdomeElement*);
  iface::dom::Document* (*wrapDocument)(GdomeDocument*);
};

std::map<std::string,CDARegisteredNamespace*> ElementFactories;

CDARegisteredNamespace::CDARegisteredNamespace
(
 std::string name,
 iface::dom::Element* (*iwrapElement)(GdomeElement*),
 iface::dom::Document* (*iwrapDocument)(GdomeDocument*)
)
  : wrapElement(iwrapElement), wrapDocument(iwrapDocument)
{
  ElementFactories.insert(std::pair<std::string,CDARegisteredNamespace*>
                          (name, this));
}

iface::dom::Element*
CDA_WrapElement(GdomeElement* el)
{
  if (el == NULL)
    return NULL;

  // If it already exists, skip all this...
  CDA_Element* ret = 
    static_cast<CDA_Element*>(CDA_Node::findExistingWrapper(GDOME_N(el)));
  if (ret != NULL)
  {
    ret->add_ref();
    GdomeException exc;
    gdome_el_unref(el, &exc);
    return ret;
  }

  // Figure out the namespace...
  GdomeException exc;

  GdomeDOMString* ds = gdome_el_namespaceURI(el, &exc);
  if (ds == NULL)
    return new CDA_Element(el);

  // We now need to look it up in the constructor list...
  std::string namespaceURI(ds->str);
  gdome_str_unref(ds);
  std::map<std::string,CDARegisteredNamespace*>::iterator i;
  i = ElementFactories.find(namespaceURI);
  if (i == ElementFactories.end() ||
      ((*i).second)->wrapElement == NULL)
    return new CDA_Element(el);

  return ((*i).second)->wrapElement(el);
}

iface::dom::Document*
CDA_WrapDocument(GdomeDocument* el)
{
  if (el == NULL)
    return NULL;

  // If it already exists, skip all this...
  CDA_Document* ret = 
    static_cast<CDA_Document*>(CDA_Node::findExistingWrapper(GDOME_N(el)));
  if (ret != NULL)
  {
    ret->add_ref();
    GdomeException exc;
    gdome_doc_unref(el, &exc);
    return ret;
  }

  // Find the document element, if there is one...
  GdomeException exc;
  GdomeElement* de = gdome_doc_documentElement(el, &exc);
  if (de == NULL)
  {
    return new CDA_Document(el);
  }

  // Figure out the namespace...
  GdomeDOMString* ds = gdome_el_namespaceURI(de, &exc);
  if (ds == NULL)
  {
    gdome_el_unref(de, &exc);
    return new CDA_Document(el);
  }
  gdome_el_unref(de, &exc);

  // We now need to look it up in the constructor list...
  std::string namespaceURI(ds->str);
  gdome_str_unref(ds);

  std::map<std::string,CDARegisteredNamespace*>::iterator i;
  i = ElementFactories.find(namespaceURI);
  if (i == ElementFactories.end() ||
      ((*i).second)->wrapDocument == NULL)
  {
    return new CDA_Document(el);
  }

  return ((*i).second)->wrapDocument(el);
}

iface::events::Event*
CDA_WrapEvent(GdomeEvent* evt)
{
  if (evt == NULL)
    return NULL;

  GdomeDOMString* str;

  if (evt == NULL)
    return NULL;
  // We need to figure out what type of node it is.
  EXCEPTION_TRY;
  str = gdome_evnt_type(evt, &exc);
  EXCEPTION_CATCH;

  bool isMutation = false;
  if (!strcmp(str->str, "DOMSubtreeModified") ||
      !strcmp(str->str, "DOMNodeInserted") ||
      !strcmp(str->str, "DOMNodeRemoved") ||
      !strcmp(str->str, "DOMNodeRemovedFromDocument") ||
      !strcmp(str->str, "DOMNodeInsertedIntoDocument") ||
      !strcmp(str->str, "DOMAttrModified") ||
      !strcmp(str->str, "DOMCharacterDataModified"))
    isMutation = true;
  gdome_str_unref(str);

  if (isMutation)
    return new CDA_MutationEvent(GDOME_MEVNT(evt));
  else
    return new CDA_Event(evt);
}

// Add-on: MathML support...
extern iface::dom::Element* WrapMathMLElement(GdomeElement* el);
extern iface::dom::Document* WrapMathMLDocument(GdomeDocument* doc);

CDARegisteredNamespace _regn("http://www.w3.org/1998/Math/MathML",
                             WrapMathMLElement, WrapMathMLDocument);
