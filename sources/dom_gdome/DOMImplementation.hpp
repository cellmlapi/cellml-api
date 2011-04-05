#include <exception>
#include "cda_compiler_support.h"
#include "IfaceDOM-APISPEC.hxx"
#include "IfaceMathML-content-APISPEC.hxx"
#include <libgdome/gdome.h>
#include <libgdome/gdome-events.h>
#include "ThreadWrapper.hxx"
#include <map>
#include <string>
#include"Utilities.hxx"

class CDA_DOMImplementation
  : public CellML_DOMImplementationBase,
    public virtual iface::mathml_dom::MathMLDOMImplementation
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(dom::DOMImplementation, mathml_dom::MathMLDOMImplementation);
  CDA_IMPL_ID;

  CDA_DOMImplementation();
  CDA_DOMImplementation(GdomeDOMImplementation*);
  virtual ~CDA_DOMImplementation();

  bool hasFeature(const wchar_t* feature,
                  const wchar_t* version)
    throw(std::exception&);

  iface::dom::DocumentType* createDocumentType
  (
   const wchar_t* qualifiedName,
   const wchar_t* publicId,
   const wchar_t* systemId
  )
    throw(std::exception&);

  iface::dom::Document* createDocument
  (
   const wchar_t* namespaceURI,
   const wchar_t* qualifiedName,
   iface::dom::DocumentType* doctype
  )
    throw(std::exception&);

  iface::mathml_dom::MathMLDocument* createMathMLDocument()
    throw(std::exception&);

  /* This is a non-standard function used by the bootstrap code to load a
   * document from a URI.
   */
  iface::dom::Document* loadDocument(const wchar_t* sourceURL,
                                     std::wstring& errorMessage)
    throw(std::exception&);
private:
  GdomeDOMImplementation* impl;
};

class CDA_Node
  : public virtual iface::dom::Node
{
public:
  CDA_Node(GdomeNode* n)
    : mNode(n)
  {
    n->user_data = reinterpret_cast<void*>(this);
  }

  virtual ~CDA_Node()
  {
  }

  wchar_t* nodeName() throw(std::exception&);
  wchar_t* nodeValue() throw(std::exception&);
  void nodeValue(const wchar_t* attr) throw(std::exception&);
  uint16_t nodeType() throw(std::exception&);
  iface::dom::Node* parentNode() throw(std::exception&);
  iface::dom::NodeList* childNodes() throw(std::exception&);
  iface::dom::Node* firstChild() throw(std::exception&);
  iface::dom::Node* lastChild() throw(std::exception&);
  iface::dom::Node* previousSibling() throw(std::exception&);
  iface::dom::Node* nextSibling() throw(std::exception&);
  iface::dom::NamedNodeMap* attributes() throw(std::exception&);
  iface::dom::Document* ownerDocument() throw(std::exception&);
  iface::dom::Node* insertBefore(iface::dom::Node* newChild,
                                 iface::dom::Node* refChild)
    throw(std::exception&);
  iface::dom::Node* replaceChild(iface::dom::Node* newChild,
                                 iface::dom::Node* oldChild)
    throw(std::exception&);
  iface::dom::Node* removeChild(iface::dom::Node* oldChild)
    throw(std::exception&);
  iface::dom::Node* appendChild(iface::dom::Node* newChild)
    throw(std::exception&);
  bool hasChildNodes() throw(std::exception&);
  iface::dom::Node* cloneNode(bool deep) throw(std::exception&);
  void normalize() throw(std::exception&);
  bool isSupported(const wchar_t* feature,
                   const wchar_t* version) throw(std::exception&);
  wchar_t* namespaceURI() throw(std::exception&);
  wchar_t* prefix() throw(std::exception&);
  void prefix(const wchar_t* attr) throw(std::exception&);
  wchar_t* localName() throw(std::exception&);
  bool hasAttributes() throw(std::exception&);
  void addEventListener(const wchar_t* type,
                        iface::events::EventListener* listener,
                        bool useCapture) throw(std::exception&);
  void removeEventListener(const wchar_t* type,
                           iface::events::EventListener* listener,
                           bool useCapture) throw(std::exception&);
  bool dispatchEvent(iface::events::Event* evt) throw(std::exception&);

  CDA_IMPL_ID;

private:
  struct EventListenerData
  {
    ~EventListenerData()
    {
      if (listener != NULL)
      {
        EXCEPTION_TRY;
        gdome_evntl_unref(listener, &exc);
        // No catch, if it fails let it fail.
      }
      if (callee != NULL)
      {
        callee->release_ref();
        callee = NULL;
      }
    }
    uint32_t copies;
    GdomeEventListener* listener;
    iface::events::EventListener* callee;
  };
  static std::map<iface::events::EventListener*,struct EventListenerData*> activeEventListeners;
  static CDAMutex mStaticMutex;
public:
  GdomeNode* mNode;

  static CDA_Node* findExistingWrapper(GdomeNode* n);
};

class CDA_NodeList
  : public iface::dom::NodeList
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(dom::NodeList);
  CDA_IMPL_ID;

  CDA_NodeList(GdomeNodeList* nl);
  virtual ~CDA_NodeList();

  iface::dom::Node* item(uint32_t index) throw(std::exception&);
  uint32_t length() throw(std::exception&);

  GdomeNodeList* impl;
};

class CDA_NamedNodeMap
  : public iface::dom::NamedNodeMap
{
public:
  CDA_NamedNodeMap(GdomeNamedNodeMap* nnm);
  virtual ~CDA_NamedNodeMap();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(dom::NamedNodeMap);
  CDA_IMPL_ID;

  iface::dom::Node* getNamedItem(const wchar_t* name)
    throw(std::exception&);
  iface::dom::Node* setNamedItem(iface::dom::Node* arg)
    throw(std::exception&);
  iface::dom::Node* removeNamedItem(const wchar_t* name)
    throw(std::exception&);
  iface::dom::Node* item(uint32_t index) throw(std::exception&);
  uint32_t length() throw(std::exception&);
  iface::dom::Node* getNamedItemNS(const wchar_t* namespaceURI,
                                   const wchar_t* localName)
    throw(std::exception&);
  iface::dom::Node* setNamedItemNS(iface::dom::Node* arg)
    throw(std::exception&);
  iface::dom::Node* removeNamedItemNS(const wchar_t* namespaceURI,
                                      const wchar_t* localName)
    throw(std::exception&);

  GdomeNamedNodeMap* impl;
};

class CDA_CharacterData
  : public virtual iface::dom::CharacterData,
    public CDA_Node
{
public:
  CDA_CharacterData(GdomeNode* aNode)
    : CDA_Node(aNode) {}

  wchar_t* data() throw(std::exception&);
  void data(const wchar_t* attr) throw(std::exception&);
  uint32_t length() throw(std::exception&);
  wchar_t* substringData(uint32_t offset, uint32_t count)
    throw(std::exception&);
  void appendData(const wchar_t* arg) throw(std::exception&);
  void insertData(uint32_t offset, const wchar_t* arg)
    throw(std::exception&);
  void deleteData(uint32_t offset, uint32_t count)
    throw(std::exception&);
  void replaceData(uint32_t offset, uint32_t count,
                   const wchar_t* arg) throw(std::exception&);

  /* Implementation only... */
  virtual GdomeCharacterData* fetchCData() const = 0;
};


#define CDA_IMPL_WRAP(gdometype, cdatype) \
  static cdatype* wrap(gdometype* x) \
  { \
    cdatype* ret = static_cast<cdatype*>(findExistingWrapper(GDOME_N(x))); \
    if (ret == NULL) \
      return new cdatype(x); \
    ret->add_ref(); \
    GdomeException exc; \
    gdome_n_unref(GDOME_N(x), &exc); \
    return ret; \
  }

class CDA_Attr
  : public iface::dom::Attr,
    public CDA_Node
{
public:
  CDA_Attr(GdomeAttr* at);
  CDA_IMPL_WRAP(GdomeAttr, CDA_Attr);

  virtual ~CDA_Attr();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::Node, dom::Attr)

  wchar_t* name() throw(std::exception&);
  bool specified() throw(std::exception&);
  wchar_t* value() throw(std::exception&);
  void value(const wchar_t* attr) throw(std::exception&);
  iface::dom::Element* ownerElement() throw(std::exception&);

  GdomeAttr* impl;
};

class CDA_Element
  : public virtual iface::dom::Element,
    public CDA_Node
{
public:
  CDA_Element(GdomeElement* el);
  virtual ~CDA_Element();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::Node, dom::Element)

  wchar_t* tagName()
    throw(std::exception&);
  wchar_t* getAttribute(const wchar_t* name)
    throw(std::exception&);
  void setAttribute(const wchar_t* name,
                    const wchar_t* value) throw(std::exception&);
  void removeAttribute(const wchar_t* name)
    throw(std::exception&);
  iface::dom::Attr* getAttributeNode(const wchar_t* name)
    throw(std::exception&);
  iface::dom::Attr* setAttributeNode(iface::dom::Attr* newAttr)
    throw(std::exception&);
  iface::dom::Attr* removeAttributeNode(iface::dom::Attr* oldAttr)
    throw(std::exception&);
  iface::dom::NodeList* getElementsByTagName(const wchar_t* name)
    throw(std::exception&);
  wchar_t* getAttributeNS(const wchar_t* namespaceURI,
                                       const wchar_t* localName)
    throw(std::exception&);
  void setAttributeNS(const wchar_t* namespaceURI,
                      const wchar_t* qualifiedName,
                      const wchar_t* value)
    throw(std::exception&);
  void removeAttributeNS(const wchar_t* namespaceURI,
                         const wchar_t* localName)
    throw(std::exception&);
  iface::dom::Attr* getAttributeNodeNS(const wchar_t* namespaceURI,
                                       const wchar_t* localName)
    throw(std::exception&);
  iface::dom::Attr* setAttributeNodeNS(iface::dom::Attr* newAttr)
    throw(std::exception&);
  iface::dom::NodeList*
  getElementsByTagNameNS(const wchar_t* namespaceURI,
                         const wchar_t* localName)
    throw(std::exception&);
  bool hasAttribute(const wchar_t* name) throw(std::exception&);
  bool hasAttributeNS(const wchar_t* namespaceURI,
                      const wchar_t* localName)
    throw(std::exception&);

  GdomeElement* impl;
};

class CDA_TextBase
  : public virtual iface::dom::Text,
    public CDA_CharacterData
{
public:
  CDA_TextBase(GdomeNode* aNode)
    : CDA_CharacterData(aNode) {}
  iface::dom::Text* splitText(uint32_t offset) throw(std::exception&);

  virtual GdomeText* fetchText() const = 0;
};

class CDA_Text
  : public CDA_TextBase
{
public:
  CDA_Text(GdomeText* txt);
  CDA_IMPL_WRAP(GdomeText, CDA_Text);
  virtual ~CDA_Text();
  
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(dom::Text, dom::CharacterData, dom::Node)

  GdomeText* impl;
  GdomeCharacterData* fetchCData() const;
  GdomeText* fetchText() const;
};

class CDA_Comment
  : public iface::dom::Comment,
    public CDA_CharacterData
{
public:
  CDA_Comment(GdomeComment* c);
  virtual ~CDA_Comment();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_WRAP(GdomeComment, CDA_Comment);
  CDA_IMPL_QI3(dom::Comment, dom::CharacterData, dom::Node)

  GdomeComment* impl;
  GdomeCharacterData* fetchCData() const;
};

class CDA_CDATASection
  : public iface::dom::CDATASection,
    public CDA_TextBase
{
public:
  CDA_CDATASection(GdomeCDATASection* cds);
  CDA_IMPL_WRAP(GdomeCDATASection, CDA_CDATASection);
  virtual ~CDA_CDATASection();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI4(dom::CDATASection, dom::Text, dom::CharacterData, dom::Node)

  GdomeCDATASection* impl;
  GdomeCharacterData* fetchCData() const;
  GdomeText* fetchText() const;
};

class CDA_DocumentType
  : public CDA_Node,
    public virtual iface::dom::DocumentType
{
public:
  CDA_DocumentType(GdomeDocumentType* dt);
  CDA_IMPL_WRAP(GdomeDocumentType, CDA_DocumentType);
  virtual ~CDA_DocumentType();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::DocumentType, dom::Node)

  wchar_t* name() throw(std::exception&);
  iface::dom::NamedNodeMap* entities() throw(std::exception&);
  iface::dom::NamedNodeMap* notations() throw(std::exception&);
  wchar_t* publicId() throw(std::exception&);
  wchar_t* systemId() throw(std::exception&);
  wchar_t* internalSubset() throw(std::exception&);

  GdomeDocumentType* impl;
};

class CDA_Notation
  : public iface::dom::Notation,
    public CDA_Node
{
public:
  CDA_Notation(GdomeNotation* nt);
  CDA_IMPL_WRAP(GdomeNotation, CDA_Notation);
  virtual ~CDA_Notation();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::Notation, dom::Node)

  wchar_t* publicId() throw(std::exception&);
  wchar_t* systemId() throw(std::exception&);

  GdomeNotation* impl;
};

class CDA_Entity
  : public iface::dom::Entity,
    public CDA_Node
{
public:
  CDA_Entity(GdomeEntity* ent);
  CDA_IMPL_WRAP(GdomeEntity, CDA_Entity);
  virtual ~CDA_Entity();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::Entity, dom::Node)

  wchar_t* publicId() throw(std::exception&);
  wchar_t* systemId() throw(std::exception&);
  wchar_t* notationName() throw(std::exception&);

  GdomeEntity* impl;
};

class CDA_EntityReference
  : public iface::dom::EntityReference,
    public CDA_Node
{
public:
  CDA_EntityReference(GdomeEntityReference* ent);
  CDA_IMPL_WRAP(GdomeEntityReference, CDA_EntityReference);
  virtual ~CDA_EntityReference();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::EntityReference, dom::Node)

  GdomeEntityReference* impl;
};

class CDA_ProcessingInstruction
  : public iface::dom::ProcessingInstruction,
    public CDA_Node
{
public:
  CDA_ProcessingInstruction(GdomeProcessingInstruction* pri);
  CDA_IMPL_WRAP(GdomeProcessingInstruction, CDA_ProcessingInstruction);
  virtual ~CDA_ProcessingInstruction();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::ProcessingInstruction, dom::Node)

  wchar_t* target() throw(std::exception&);
  wchar_t* data() throw(std::exception&);
  void data(const wchar_t* attr) throw(std::exception&);

  GdomeProcessingInstruction* impl;
};

class CDA_DocumentFragment
  : public iface::dom::DocumentFragment,
    public CDA_Node
{
public:
  CDA_DocumentFragment(GdomeDocumentFragment* df);
  CDA_IMPL_WRAP(GdomeDocumentFragment, CDA_DocumentFragment);
  virtual ~CDA_DocumentFragment();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(dom::DocumentFragment, dom::Node)

  GdomeDocumentFragment* impl;
};

class CDA_Document
  : public virtual iface::dom::Document,
    public CDA_Node
{
public:
  CDA_Document(GdomeDocument* doc);
  virtual ~CDA_Document();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_WRAP(GdomeDocument, CDA_Document);
  CDA_IMPL_QI2(dom::Document, dom::Node)

  iface::dom::DocumentType* doctype()
    throw(std::exception&);
  iface::dom::DOMImplementation* implementation()
    throw(std::exception&);
  iface::dom::Element* documentElement()
    throw(std::exception&);
  iface::dom::Element* createElement(const wchar_t* tagName)
    throw(std::exception&);
  iface::dom::DocumentFragment* createDocumentFragment()
    throw(std::exception&);
  iface::dom::Text* createTextNode(const wchar_t* data)
    throw(std::exception&);
  iface::dom::Comment* createComment(const wchar_t* data)
    throw(std::exception&);
  iface::dom::CDATASection* createCDATASection(const wchar_t* data)
    throw(std::exception&);
  iface::dom::ProcessingInstruction* createProcessingInstruction
  (const wchar_t* target, const wchar_t* data)
    throw(std::exception&);
  iface::dom::Attr* createAttribute(const wchar_t* name)
    throw(std::exception&);
  iface::dom::EntityReference* createEntityReference
    (const wchar_t* name) throw(std::exception&);
  iface::dom::NodeList* getElementsByTagName
    (const wchar_t* tagname) throw(std::exception&);
  iface::dom::Node* importNode(iface::dom::Node* importedNode, bool deep)
    throw(std::exception&);
  iface::dom::Element*
    createElementNS(const wchar_t* namespaceURI,
                    const wchar_t* qualifiedName)
    throw(std::exception&);
  iface::dom::Attr* createAttributeNS(const wchar_t* namespaceURI,
                                      const wchar_t* qualifiedName)
    throw(std::exception&);
  iface::dom::NodeList* getElementsByTagNameNS
    (const wchar_t* namespaceURI,
     const wchar_t* localName) throw(std::exception&);
  iface::dom::Element* getElementById(const wchar_t* elementId)
    throw(std::exception&);
  iface::events::Event* createEvent(const wchar_t* domEventType)
    throw(std::exception&);
  
  GdomeDocument* impl;
};

class CDA_EventBase
  : public virtual iface::events::Event
{
public:
  iface::events::DOMString type() throw(std::exception&);
  iface::dom::Node* target() throw(std::exception&);
  iface::dom::Node* currentTarget() throw(std::exception&);
  uint16_t eventPhase() throw(std::exception&);
  bool bubbles() throw(std::exception&);
  bool cancelable() throw(std::exception&);
  iface::events::DOMTimeStamp timeStamp() throw(std::exception&);
  void stopPropagation() throw(std::exception&);
  void preventDefault() throw(std::exception&);
  void initEvent(const wchar_t* eventTypeArg,
                 bool canBubbleArg, bool cancelableArg) throw(std::exception&);

  virtual GdomeEvent* fetchEvent() const = 0;
};

class CDA_Event
  : public CDA_EventBase
{
public:
  CDA_Event(GdomeEvent* evt);
  virtual ~CDA_Event();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(events::Event);
  CDA_IMPL_ID;

  GdomeEvent* fetchEvent() const;
private:
  GdomeEvent* impl;
};

class CDA_MutationEvent
  : public iface::events::MutationEvent,
    public CDA_EventBase
{
public:
  CDA_MutationEvent(GdomeMutationEvent* me);
  virtual ~CDA_MutationEvent();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(events::Event, events::MutationEvent)
  CDA_IMPL_ID;

  iface::events::Node relatedNode() throw(std::exception&);
  iface::events::DOMString prevValue() throw(std::exception&);
  iface::events::DOMString newValue() throw(std::exception&);
  iface::events::DOMString attrName() throw(std::exception&);
  uint16_t attrChange() throw(std::exception&);
  void initMutationEvent(const wchar_t* typeArg,
                         bool canBubbleArg, bool cancelableArg,
                         iface::events::Node relatedNodeArg,
                         const wchar_t* prevValueArg,
                         const wchar_t* newValueArg,
                         const wchar_t* attrNameArg,
                         uint16_t attrChangeArg)
    throw(std::exception&);

  GdomeMutationEvent* impl;
  GdomeEvent* fetchEvent() const;
};

iface::dom::Node* CDA_WrapNode(GdomeNode* n);
iface::dom::Element* CDA_WrapElement(GdomeElement* e);
iface::dom::Document* CDA_WrapDocument(GdomeDocument* d);
iface::events::Event* CDA_WrapEvent(GdomeEvent* ev);
