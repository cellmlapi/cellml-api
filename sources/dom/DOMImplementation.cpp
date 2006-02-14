#include "Utilities.hxx"
#include "DOMImplementation.hxx"
#include <string>

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
    return !!wcscmp(version, L"2.0");
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
  
  return new CDA_DocumentType(gdt);
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
  TRDOMSTRING(namespaceURI);
  TRDOMSTRING(qualifiedName);
  LOCALCONVERT(doctype, DocumentType)

  GdomeDocument* gd =
    gdome_di_createDocument(impl, gdnamespaceURI, gdqualifiedName,
                            ldoctype->impl, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(qualifiedName);
  EXCEPTION_CATCH;
  
  return CDA_WrapDocument(gd);
}

wchar_t*
CDA_Node::nodeName()
  throw(std::exception&)
{
  GdomeDOMString* str;
  EXCEPTION_TRY;
  str = gdome_n_nodeName(fetchNode(), &exc);
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
  str = gdome_n_nodeValue(fetchNode(), &exc);
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
  gdome_n_set_nodeValue(fetchNode(), gdattr, &exc);
  DDOMSTRING(attr);
  EXCEPTION_CATCH;
}

u_int16_t
CDA_Node::nodeType()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  u_int16_t ret = gdome_n_nodeType(fetchNode(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

iface::dom::Node*
CDA_Node::parentNode()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_parentNode(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::firstChild()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_firstChild(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::lastChild()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_lastChild(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::previousSibling()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_previousSibling(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::nextSibling()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_nextSibling(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::NamedNodeMap*
CDA_Node::attributes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNamedNodeMap* ret = gdome_n_attributes(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return new CDA_NamedNodeMap(ret);
}

iface::dom::Document*
CDA_Node::ownerDocument()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDocument* ret = gdome_n_ownerDocument(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return new CDA_Document(ret);
}

iface::dom::Node*
CDA_Node::insertBefore(iface::dom::Node* newChild,
                       iface::dom::Node* refChild)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  LOCALCONVERT(newChild, Node);
  LOCALCONVERT(refChild, Node);
  GdomeNode* ret = gdome_n_insertBefore(fetchNode(), lnewChild->fetchNode(),
                                        lrefChild->fetchNode(), &exc);
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
  GdomeNode* ret = gdome_n_replaceChild(fetchNode(), lnewChild->fetchNode(),
                                        loldChild->fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_Node::removeChild(iface::dom::Node* oldChild)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  LOCALCONVERT(oldChild, Node);
  GdomeNode* ret = gdome_n_removeChild(fetchNode(), loldChild->fetchNode(),
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
  GdomeNode* ret = gdome_n_appendChild(fetchNode(), lnewChild->fetchNode(),
                                       &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

bool
CDA_Node::hasChildNodes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_n_hasChildNodes(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return ret;
}

iface::dom::Node*
CDA_Node::cloneNode(bool deep)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_n_cloneNode(fetchNode(), deep, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

void
CDA_Node::normalize()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_n_normalize(fetchNode(), &exc);
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
  bool ret = gdome_n_isSupported(fetchNode(), gdfeature, gdversion, &exc);
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
  GdomeDOMString* ret = gdome_n_namespaceURI(fetchNode(), &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

wchar_t*
CDA_Node::prefix()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_n_prefix(fetchNode(), &exc);
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
  gdome_n_set_prefix(fetchNode(), gdattr, &exc);
  DDOMSTRING(attr);
  EXCEPTION_CATCH;
}

wchar_t*
CDA_Node::localName()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_n_localName(fetchNode(), &exc);
  EXCEPTION_CATCH;

  TRGDOMSTRING(ret);

  return cxxret;
}

bool
CDA_Node::hasAttributes()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_n_hasAttributes(fetchNode(), &exc);
  EXCEPTION_CATCH;

  return ret;
}

static void cda_process_event(GdomeEventListener *self,
                              GdomeEvent *event,
                              GdomeException *exc)
{
  iface::events::EventListener* l =
    reinterpret_cast<iface::events::EventListener*>(gdome_evntl_get_priv(self));
  l->handleEvent(CDA_WrapEvent(event));
}

void
CDA_Node::addEventListener(const wchar_t* type,
                           iface::events::EventListener* listener,
                           bool useCapture)
  throw(std::exception&)
{
  CDALock scopedLock(aelprotect);

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
    listener->add_ref();
  }
  else
  {
    ed = (*i).second;
  }

  TRDOMSTRING(type);
  EXCEPTION_TRY;
  gdome_n_addEventListener(fetchNode(), gdtype, ed->listener, useCapture,
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
  CDALock scopedLock(aelprotect);
  std::map<iface::events::EventListener*,struct EventListenerData*>::iterator i =
    activeEventListeners.find(listener);
  EventListenerData* ed;
  if (i == activeEventListeners.end())
    throw iface::dom::DOMException();

  ed = (*i).second;

  TRDOMSTRING(type);
  EXCEPTION_TRY;
  gdome_n_removeEventListener(fetchNode(), gdtype, ed->listener, useCapture,
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
  bool ret = gdome_n_dispatchEvent(fetchNode(), levt->fetchEvent(), &exc);
  EXCEPTION_CATCH;

  return ret;
}

CDA_NodeList::CDA_NodeList(GdomeNodeList* nl)
  : _cda_refcount(1), impl(nl)
{
}

CDA_NodeList::~CDA_NodeList()
{
  EXCEPTION_TRY;
  gdome_nl_unref(impl, &exc);
}

iface::dom::Node*
CDA_NodeList::item(u_int32_t index)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nl_item(impl, index, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

u_int32_t
CDA_NodeList::length()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  u_int32_t ret = gdome_nl_length(impl, &exc);
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
  GdomeNode* ret = gdome_nnm_setNamedItem(impl, larg->fetchNode(), &exc);
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
CDA_NamedNodeMap::item(u_int32_t index)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_nnm_item(impl, index, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

u_int32_t
CDA_NamedNodeMap::length()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  u_int32_t ret = gdome_nnm_length(impl, &exc);
  EXCEPTION_CATCH;

  return ret;
}

iface::dom::Node*
CDA_NamedNodeMap::getNamedItemNS(const wchar_t* namespaceURI,
                                 const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING(namespaceURI);
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
  GdomeNode* ret = gdome_nnm_setNamedItemNS(impl, larg->fetchNode(), &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Node*
CDA_NamedNodeMap::removeNamedItemNS(const wchar_t* namespaceURI,
                                    const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING(namespaceURI);
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

u_int32_t
CDA_CharacterData::length()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  u_int32_t ret = gdome_cd_length(fetchCData(), &exc);
  EXCEPTION_CATCH;  

  return ret;
}

wchar_t*
CDA_CharacterData::substringData(u_int32_t offset, u_int32_t count)
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
CDA_CharacterData::insertData(u_int32_t offset, const wchar_t* arg)
  throw(std::exception&)
{
  TRDOMSTRING(arg);
  EXCEPTION_TRY;
  gdome_cd_insertData(fetchCData(), offset, gdarg, &exc);
  DDOMSTRING(arg);
  EXCEPTION_CATCH;
}

void
CDA_CharacterData::deleteData(u_int32_t offset, u_int32_t count)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_cd_deleteData(fetchCData(), offset, count, &exc);
  EXCEPTION_CATCH;
}

void
CDA_CharacterData::replaceData(u_int32_t offset, u_int32_t count,
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
  : _cda_refcount(1), impl(at)
{
}

CDA_Attr::~CDA_Attr()
{
  EXCEPTION_TRY;
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
  : _cda_refcount(1), impl(el)
{
}

CDA_Element::~CDA_Element()
{
  EXCEPTION_TRY;
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

  return new CDA_Attr(ret);
}

iface::dom::Attr*
 CDA_Element::setAttributeNode(iface::dom::Attr* newAttr)
  throw(std::exception&)
{
  LOCALCONVERT(newAttr, Attr);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_setAttributeNode(impl, lnewAttr->impl, &exc);
  EXCEPTION_CATCH;

  return new CDA_Attr(ret);
}

iface::dom::Attr* CDA_Element::removeAttributeNode(iface::dom::Attr* oldAttr)
  throw(std::exception&)
{
  LOCALCONVERT(oldAttr, Attr);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_removeAttributeNode(impl, loldAttr->impl, &exc);
  EXCEPTION_CATCH;

  return new CDA_Attr(ret);
}

iface::dom::NodeList* CDA_Element::getElementsByTagName(const wchar_t* name)
  throw(std::exception&)
{
  TRDOMSTRING(name);
  EXCEPTION_TRY;
  GdomeNodeList* ret = gdome_el_getElementsByTagName(impl, gdname, &exc);
  DDOMSTRING(name);
  EXCEPTION_CATCH;

  return new CDA_NodeList(ret);
}

wchar_t*
CDA_Element::getAttributeNS(const wchar_t* namespaceURI,
                            const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING(namespaceURI);
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
  TRDOMSTRING(namespaceURI);
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
  TRDOMSTRING(namespaceURI);
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
  TRDOMSTRING(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_el_getAttributeNodeNS(impl, gdnamespaceURI,
                                               gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  return new CDA_Attr(ret);
}

iface::dom::Attr* CDA_Element::setAttributeNodeNS(iface::dom::Attr* newAttr)
  throw(std::exception&)
{
  LOCALCONVERT(newAttr, Attr);
  EXCEPTION_TRY;
  GdomeAttr* ret =
    gdome_el_setAttributeNodeNS(impl, lnewAttr->impl, &exc);
  EXCEPTION_CATCH;

  return new CDA_Attr(ret);
}

iface::dom::NodeList*
CDA_Element::getElementsByTagNameNS(const wchar_t* namespaceURI,
                                    const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeNodeList* ret =
    gdome_el_getElementsByTagNameNS(impl, gdnamespaceURI, gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

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
  TRDOMSTRING(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  bool ret = gdome_el_hasAttributeNS(impl, gdnamespaceURI, gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

  return ret;
}

GdomeNode*
CDA_Element::fetchNode() const
{
  return GDOME_N(impl);
}

iface::dom::Text*
CDA_TextBase::splitText(u_int32_t offset)
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeText* ret = gdome_t_splitText(fetchText(), offset, &exc);
  EXCEPTION_CATCH;

  return new CDA_Text(ret);
}

CDA_Text::CDA_Text(GdomeText* txt)
  : _cda_refcount(1), impl(txt)
{
}

CDA_Text::~CDA_Text()
{
  EXCEPTION_TRY;
  gdome_t_unref(impl, &exc);
}

GdomeNode*
CDA_Text::fetchNode() const
{
  return GDOME_N(impl);
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
  : _cda_refcount(1), impl(c)
{
}

CDA_Comment::~CDA_Comment()
{
  EXCEPTION_TRY;
  gdome_c_unref(impl, &exc);
}

GdomeNode*
CDA_Comment::fetchNode() const
{
  return GDOME_N(impl);
}

GdomeCharacterData*
CDA_Comment::fetchCData() const
{
  return GDOME_CD(impl);
}

CDA_CDATASection::CDA_CDATASection(GdomeCDATASection* cds)
  : _cda_refcount(1), impl(cds)
{
}

CDA_CDATASection::~CDA_CDATASection()
{
  EXCEPTION_TRY;
  gdome_cds_unref(impl, &exc);
}

GdomeNode*
CDA_CDATASection::fetchNode() const
{
  return GDOME_N(impl);
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
  : _cda_refcount(1), impl(dt)
{
}

CDA_DocumentType::~CDA_DocumentType()
{
  EXCEPTION_TRY;
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

  return new CDA_NamedNodeMap(ret);
}

iface::dom::NamedNodeMap*
CDA_DocumentType::notations()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeNamedNodeMap* ret = gdome_dt_notations(impl, &exc);
  EXCEPTION_CATCH;

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

GdomeNode*
CDA_DocumentType::fetchNode() const
{
  return GDOME_N(impl);
}

CDA_Notation::CDA_Notation(GdomeNotation* nt)
  : _cda_refcount(1), impl(nt)
{
}

CDA_Notation::~CDA_Notation()
{
  EXCEPTION_TRY;
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

GdomeNode*
CDA_Notation::fetchNode() const
{
  return GDOME_N(impl);
}

CDA_Entity::CDA_Entity(GdomeEntity* ent)
  : _cda_refcount(1), impl(ent)
{
}

CDA_Entity::~CDA_Entity()
{
  EXCEPTION_TRY;
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

GdomeNode*
CDA_Entity::fetchNode() const
{
  return GDOME_N(impl);
}

CDA_EntityReference::CDA_EntityReference(GdomeEntityReference* ent)
  : _cda_refcount(1), impl(ent)
{
}

CDA_EntityReference::~CDA_EntityReference()
{
  EXCEPTION_TRY;
  gdome_er_unref(impl, &exc);
}

GdomeNode*
CDA_EntityReference::fetchNode() const
{
  return GDOME_N(impl);
}

CDA_ProcessingInstruction::CDA_ProcessingInstruction
(GdomeProcessingInstruction* pri)
  : _cda_refcount(1), impl(pri)
{
}

CDA_ProcessingInstruction::~CDA_ProcessingInstruction()
{
  EXCEPTION_TRY;
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
  : _cda_refcount(1), impl(df)
{
}

CDA_DocumentFragment::~CDA_DocumentFragment()
{
  EXCEPTION_TRY;
  gdome_df_unref(impl, &exc);
}

GdomeNode*
CDA_DocumentFragment::fetchNode() const
{
  return GDOME_N(impl);
}

CDA_Document::CDA_Document(GdomeDocument* doc)
  : _cda_refcount(1), impl(doc)
{
}

CDA_Document::~CDA_Document()
{
  EXCEPTION_TRY;
  gdome_doc_unref(impl, &exc);
}

iface::dom::DocumentType*
CDA_Document::doctype()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDocumentType* ret = gdome_doc_doctype(impl, &exc);
  EXCEPTION_CATCH;
  return new CDA_DocumentType(ret);
}

iface::dom::DOMImplementation*
CDA_Document::implementation()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMImplementation* ret = gdome_doc_implementation(impl, &exc);
  EXCEPTION_CATCH;
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
  return new CDA_Element(ret);
}

iface::dom::DocumentFragment*
CDA_Document::createDocumentFragment()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDocumentFragment* ret = gdome_doc_createDocumentFragment(impl, &exc);
  EXCEPTION_CATCH;
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
  return new CDA_NodeList(ret);
}

iface::dom::Node*
CDA_Document::importNode(iface::dom::Node* importedNode, bool deep)
  throw(std::exception&)
{
  LOCALCONVERT(importedNode, Node)
  EXCEPTION_TRY;
  GdomeNode* ret = gdome_doc_importNode(impl, limportedNode->fetchNode(), deep, &exc);
  EXCEPTION_CATCH;

  return CDA_WrapNode(ret);
}

iface::dom::Element*
CDA_Document::createElementNS(const wchar_t* namespaceURI,
                              const wchar_t* qualifiedName)
  throw(std::exception&)
{
  TRDOMSTRING(namespaceURI);
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
  TRDOMSTRING(namespaceURI);
  TRDOMSTRING(qualifiedName);
  EXCEPTION_TRY;
  GdomeAttr* ret = gdome_doc_createAttributeNS(impl, gdnamespaceURI, gdqualifiedName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(qualifiedName);
  EXCEPTION_CATCH;

  return new CDA_Attr(ret);
}

iface::dom::NodeList*
CDA_Document::getElementsByTagNameNS
(const wchar_t* namespaceURI,
 const wchar_t* localName)
  throw(std::exception&)
{
  TRDOMSTRING(namespaceURI);
  TRDOMSTRING(localName);
  EXCEPTION_TRY;
  GdomeNodeList* ret = gdome_doc_getElementsByTagNameNS(impl, gdnamespaceURI,
                                                        gdlocalName, &exc);
  DDOMSTRING(namespaceURI);
  DDOMSTRING(localName);
  EXCEPTION_CATCH;

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
  
GdomeNode*
CDA_Document::fetchNode() const
{
  return GDOME_N(impl);
}

iface::events::DOMString
CDA_Event::type()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  GdomeDOMString* ret = gdome_evnt_type(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  TRGDOMSTRING(ret);
  return cxxret;
}

u_int16_t
CDA_Event::eventPhase()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  u_int16_t ret = gdome_evnt_eventPhase(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

bool
CDA_Event::bubbles()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_evnt_bubbles(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

bool
CDA_Event::cancelable()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  bool ret = gdome_evnt_cancelable(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

iface::events::DOMTimeStamp
CDA_Event::timeStamp()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  iface::events::DOMTimeStamp ret = gdome_evnt_timeStamp(fetchEvent(), &exc);
  EXCEPTION_CATCH;
  return ret;
}

void
CDA_Event::stopPropagation()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_evnt_stopPropagation(fetchEvent(), &exc);
  EXCEPTION_CATCH;
}

void
CDA_Event::preventDefault()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  gdome_evnt_preventDefault(fetchEvent(), &exc);
  EXCEPTION_CATCH;
}

void
CDA_Event::initEvent
(
 const iface::events::DOMString eventTypeArg, bool canBubbleArg,
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

CDA_MutationEvent::CDA_MutationEvent(GdomeMutationEvent* me)
  : _cda_refcount(1), impl(me)
{
}

CDA_MutationEvent::~CDA_MutationEvent()
{
  EXCEPTION_TRY;
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

u_int16_t
CDA_MutationEvent::attrChange()
  throw(std::exception&)
{
  EXCEPTION_TRY;
  u_int16_t ret = gdome_mevnt_attrChange(impl, &exc);
  EXCEPTION_CATCH;
  return ret;
}

void
CDA_MutationEvent::initMutationEvent
(
 const iface::events::DOMString typeArg,
 bool canBubbleArg, bool cancelableArg,
 iface::events::Node relatedNodeArg,
 const iface::events::DOMString prevValueArg,
 const iface::events::DOMString newValueArg,
 const iface::events::DOMString attrNameArg,
 u_int16_t attrChangeArg
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
                                lrelatedNodeArg->fetchNode(), gdprevValueArg,
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
    return new CDA_Attr(GDOME_A(n));
  case GDOME_TEXT_NODE:
    return new CDA_Text(GDOME_T(n));
  case GDOME_CDATA_SECTION_NODE:
    return new CDA_CDATASection(GDOME_CDS(n));
  case GDOME_ENTITY_REFERENCE_NODE:
    return new CDA_EntityReference(GDOME_ER(n));
  case GDOME_ENTITY_NODE:
    return new CDA_Entity(GDOME_ENT(n));
  case GDOME_PROCESSING_INSTRUCTION_NODE:
    return new CDA_ProcessingInstruction(GDOME_PI(n));
  case GDOME_COMMENT_NODE:
    return new CDA_Comment(GDOME_C(n));
  case GDOME_DOCUMENT_NODE:
    return CDA_WrapDocument(GDOME_DOC(n));
  case GDOME_DOCUMENT_TYPE_NODE:
    return new CDA_DocumentType(GDOME_DT(n));
  case GDOME_DOCUMENT_FRAGMENT_NODE:
    return new CDA_DocumentFragment(GDOME_DF(n));
  case GDOME_NOTATION_NODE:
    return new CDA_Notation(GDOME_NOT(n));
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
  // Figure out the namespace...
  GdomeException exc;
  GdomeDOMString* ds = gdome_el_namespaceURI(el, &exc);
  if (ds == NULL)
    return new CDA_Element(el);

  // We now need to look it up in the constructor list...
  std::string namespaceURI(ds->str);
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
  // Figure out the namespace...
  GdomeException exc;
  GdomeDOMString* ds = gdome_doc_namespaceURI(el, &exc);
  if (ds == NULL)
    return new CDA_Document(el);

  // We now need to look it up in the constructor list...
  std::string namespaceURI(ds->str);
  std::map<std::string,CDARegisteredNamespace*>::iterator i;
  i = ElementFactories.find(namespaceURI);
  if (i == ElementFactories.end() ||
      ((*i).second)->wrapDocument == NULL)
    return new CDA_Document(el);

  return ((*i).second)->wrapDocument(el);
}

// Add-on: MathML support...
extern iface::dom::Element* WrapMathMLElement(GdomeElement* el);
extern iface::dom::Document* WrapMathMLDocument(GdomeDocument* doc);

CDARegisteredNamespace _regn("http://www.w3.org/1998/Math/MathML",
                             WrapMathMLElement, WrapMathMLDocument);
