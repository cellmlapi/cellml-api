#include "CellMLImplementation.hpp"
#include "DOMWriter.hxx"
#include <memory>

#define NULL_NS L""
#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"
#define CMETA_NS L"http://www.cellml.org/metadata/1.0#"
#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"
#define RDF_NS L"http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define XLINK_NS L"http://www.w3.org/1999/xlink"

CDA_RDFXMLDOMRepresentation::CDA_RDFXMLDOMRepresentation(iface::dom::Element* idata)
  : _cda_refcount(1), datastore(idata)
{
  datastore->add_ref();
}

CDA_RDFXMLDOMRepresentation::~CDA_RDFXMLDOMRepresentation()
{
  datastore->release_ref();
}

wchar_t*
CDA_RDFXMLDOMRepresentation::type()
  throw(std::exception&)
{
  return wcsdup(L"http://www.cellml.org/RDFXML/DOM");
}

iface::dom::Element*
CDA_RDFXMLDOMRepresentation::data()
  throw(std::exception&)
{
  datastore->add_ref();
  return datastore;
}

CDA_RDFXMLStringRepresentation::CDA_RDFXMLStringRepresentation
(
 iface::dom::Element* idata
)
  : _cda_refcount(1), datastore(idata)
{
  datastore->add_ref();
}

CDA_RDFXMLStringRepresentation::~CDA_RDFXMLStringRepresentation()
{
  datastore->release_ref();
}

wchar_t*
CDA_RDFXMLStringRepresentation::type()
  throw(std::exception&)
{
  return wcsdup(L"http://www.cellml.org/RDFXML/string");
}

wchar_t*
CDA_RDFXMLStringRepresentation::serialisedData()
  throw(std::exception&)
{
  // We need to serialise the element...
  DOMWriter dw;
  std::wstring str;
  dw.writeElement(NULL, datastore, str);
  return wcsdup(str.c_str());
}

void
CDA_RDFXMLStringRepresentation::serialisedData(const wchar_t* attr)
  throw(std::exception&)
{
  // We need to parse attr, and update the element accordingly.
  return;
}

CDA_URI::CDA_URI(iface::dom::Attr* idata)
  : _cda_refcount(1), datastore(idata)
{
  datastore->add_ref();
}

CDA_URI::~CDA_URI()
{
  datastore->release_ref();
}

wchar_t*
CDA_URI::asText()
  throw(std::exception&)
{
  return datastore->value();
}

void
CDA_URI::asText(const wchar_t* attr)
  throw(std::exception&)
{
  datastore->value(attr);
}

CDA_CellMLElement::CDA_CellMLElement
(
 iface::XPCOM::IObject* parent,
 iface::dom::Element* idata
)
  : mParent(parent), datastore(idata), _cda_refcount(1),
    children(NULL)
{
  if (parent != NULL)
    parent->add_ref();
  datastore->add_ref();
}

CDA_CellMLElement::~CDA_CellMLElement()
{
  if (_cda_refcount != 0)
    printf("Warning: release_ref called too few times on %s.\n",
           typeid(this).name());

  if (datastore != NULL)
    datastore->release_ref();

  // Reference counts are shared across all connected elements and sets, so
  // when the reference count goes to zero, it is each element's responsibility
  // to destroy the children set, and the set's responsibility to destroy the
  // elements in it.
  if (children != NULL)
    delete children;

  std::map<std::wstring,iface::cellml_api::UserData*>::iterator i =
    userData.begin();
  for (; i != userData.end(); i++)
    (*i).second->release_ref();
}

wchar_t*
CDA_CellMLElement::cellmlVersion()
  throw(std::exception&)
{
  // We default to the latest known version, so that we will be backwards
  // compatible when a newer version comes out.
  const wchar_t* version = L"1.1";
  wchar_t* ns = datastore->namespaceURI();
  if (!wcscmp(ns, CELLML_1_0_NS))
    version = L"1.0";
  free(ns);
  return wcsdup(version);
}

wchar_t*
CDA_CellMLElement::cmetaId()
  throw(std::exception&)
{
  return datastore->getAttributeNS(CMETA_NS, L"id");
}

void
CDA_CellMLElement::cmetaId(const wchar_t* attr)
  throw(std::exception&)
{
  return datastore->setAttributeNS(CMETA_NS, L"id", attr);
}

iface::cellml_api::RDFRepresentation*
CDA_CellMLElement::getRDFRepresentation(const wchar_t* type)
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(cmid, datastore->getAttributeNS(CMETA_NS, L"id"));
  if (cmid == L"")
    throw iface::cellml_api::CellMLException();

  // We now have to search all RDF elements...
  CDA_CellMLElement* el = this;
  CDA_Model* m = NULL;
  while (true)
  {
    m = dynamic_cast<CDA_Model*>(el);
    if (m != NULL)
      break;
    el = dynamic_cast<CDA_CellMLElement*>(el->mParent);
    if (el == NULL)
      break;
  }
  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  RETURN_INTO_OBJREF(cnodes, iface::dom::NodeList,
                     m->datastore->childNodes());

  cmid = L"#" + cmid;
  u_int32_t i, l = cnodes->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, cnodes->item(i));

    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;

    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI != RDF_NS)
      continue;

    RETURN_INTO_WSTRING(ln, el->localName());
    if (ln != L"RDF")
      continue;

    // Next we look for descriptions...
    u_int32_t j, m = cnodes->length();
    for (j = 0; j < m; j++)
    {
      RETURN_INTO_OBJREF(cnodes2, iface::dom::NodeList,
                         el->childNodes());

      RETURN_INTO_OBJREF(n2, iface::dom::Node, cnodes2->item(j));

      DECLARE_QUERY_INTERFACE_OBJREF(el2, n2, dom::Element);
      if (el2 == NULL)
        continue;

      RETURN_INTO_WSTRING(nsURI2, el2->namespaceURI());
      if (nsURI2 != RDF_NS)
        continue;

      RETURN_INTO_WSTRING(ln2, el2->localName());

      if (ln2 != L"Description")
        continue;
      RETURN_INTO_WSTRING(about, el2->getAttributeNS(RDF_NS, L"about"));
      if (about != cmid)
        continue;

      if (!wcscmp(type, L"http://www.cellml.org/RDFXML/DOM"))
        return new CDA_RDFXMLDOMRepresentation(el2);
      else if (!wcscmp(type, L"http://www.cellml.org/RDFXML/string"))
        return new CDA_RDFXMLStringRepresentation(el2);
      else
        throw iface::cellml_api::CellMLException();
    }
  }
  throw iface::cellml_api::CellMLException();
}

iface::cellml_api::ExtensionElementList*
CDA_CellMLElement::extensionElements()
  throw(std::exception&)
{
  return new CDA_ExtensionElementList(datastore);
}

void
CDA_CellMLElement::insertExtensionElementAfter
(
 const iface::cellml_api::ExtensionElement marker,
 const iface::cellml_api::ExtensionElement newEl
)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = marker->nextSibling();
    if (n == NULL)
    {
      n = datastore->appendChild(newEl);
    }
    else
    {
      iface::dom::Node* n2 = datastore->insertBefore(newEl, n);
      n2->release_ref();
    }
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::appendExtensionElement(const iface::cellml_api::ExtensionElement x)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->appendChild(x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::prependExtensionElement(const iface::cellml_api::ExtensionElement x)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->firstChild();
    if (n == NULL)
      n = datastore->appendChild(x);
    else
    {
      iface::dom::Node* n2 = datastore->insertBefore(x, n);
      n2->release_ref();
    }
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::removeExtensionElement(const iface::cellml_api::ExtensionElement x)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->removeChild(x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::replaceExtensionElement
(
 const iface::cellml_api::ExtensionElement x,
 const iface::cellml_api::ExtensionElement y
)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->replaceChild(y, x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::clearExtensionElements()
  throw(std::exception&)
{
  try
  {
    // Any child element which we don't recognise gets removed...
    RETURN_INTO_OBJREF(nl, iface::dom::NodeList, datastore->childNodes());
    u_int32_t i, l = nl->length();
    for (i = 0; i < l;)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
      // All CellML is definitely safe...
      RETURN_INTO_WSTRING(nsURI, n->namespaceURI());
      if (nsURI == CELLML_1_0_NS || nsURI == CELLML_1_1_NS ||
          nsURI == CMETA_NS || nsURI == MATHML_NS || nsURI == RDF_NS)
      {
        i++;
        continue;
      }
      datastore->removeChild(n)->release_ref();
      l--;
    }
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLElementSet*
CDA_CellMLElement::childElements()
  throw(std::exception&)
{
  try
  {
    if (children == NULL)
    {
      children = new CDA_CellMLElementSet(this, datastore);
    }
    children->add_ref();
    return children;
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::addElement(iface::cellml_api::CellMLElement* x)
  throw(std::exception&)
{
  try
  {
    // Does the element come from this implementation?
    CDA_CellMLElement* el = dynamic_cast<CDA_CellMLElement*>(x);
    if (el == NULL)
      throw iface::cellml_api::CellMLException();
    
    // Does it already have a parent?
    if (el->mParent != NULL)
      throw iface::cellml_api::CellMLException();
    
    // Adopt the node...
    el->mParent = this;
    // We adopted the node, so we also take on its references...
    u_int32_t i;
    for (i = 0; i < el->_cda_refcount; i++)
      add_ref();

    // TODO: Perhaps we should also check the datastore ownerDocument matches?

    // Make a local CellML wrapping, if it doesn't exist already...
    if (children == NULL)
    {
      children = new CDA_CellMLElementSet(this, datastore);
    }

    // Add to our local CellML wrapping...
    children->addChildToWrapper(el);

    // Add the element's backing datastore to our datastore...
    datastore->appendChild(el->datastore)->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::removeElement(iface::cellml_api::CellMLElement* x)
  throw(std::exception&)
{
  try
  {
    // Does the element come from this implementation?
    CDA_CellMLElement* el = dynamic_cast<CDA_CellMLElement*>(x);
    if (el == NULL)
      throw iface::cellml_api::CellMLException();
    
    // Are we the correct parent?
    if (el->mParent != this)
      throw iface::cellml_api::CellMLException();
    
    // Remove the element's backing datastore to our datastore...
    datastore->removeChild(el->datastore)->release_ref();

    // Remove from our local CellML wrapping, if it exists...
    if (children)
      children->removeChildFromWrapper(el);

    // The node is now orphaned...
    el->mParent = NULL;
    u_int32_t i;
    // We don't need to be kept around for its references any more.
    for (i = 0; i < el->_cda_refcount; i++)
      release_ref();

    if (el->_cda_refcount == 0)
      delete el;
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::replaceElement(iface::cellml_api::CellMLElement* x,
                                  iface::cellml_api::CellMLElement* y)
  throw(std::exception&)
{
  try
  {
    // Do the elements come from this implementation?
    CDA_CellMLElement* elOld = dynamic_cast<CDA_CellMLElement*>(x);
    if (elOld == NULL)
      throw iface::cellml_api::CellMLException();

    CDA_CellMLElement* elNew = dynamic_cast<CDA_CellMLElement*>(y);
    if (elNew == NULL)
      throw iface::cellml_api::CellMLException();
    
    // Are we the correct parent for the old element?
    if (elOld->mParent != this)
      throw iface::cellml_api::CellMLException();
    
    // Does the new element already have a parent?
    if (elNew->mParent != NULL)
      throw iface::cellml_api::CellMLException();

    if (children)
    {
      // Remove the old element and add the new element to our local CellML
      // wrapping, if it exists...
      children->removeChildFromWrapper(elOld);
      children->addChildToWrapper(elNew);
    }

    // Replace on the DOM...
    datastore->replaceChild(elNew->datastore, elOld->datastore)->release_ref();

    // The old node is now orphaned...
    elOld->mParent = NULL;
    u_int32_t i;
    // We don't need to be kept around for its references any more.
    for (i = 0; i < elOld->_cda_refcount; i++)
      release_ref();

    if (elOld->_cda_refcount == 0)
      delete elOld;

    // The new node now belongs to us...
    elNew->mParent = this;
    for (i = 0; i < elNew->_cda_refcount; i++)
      add_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLElement::removeByName
(
 const wchar_t* type,
 const wchar_t* name
)
  throw(std::exception&)
{
  iface::cellml_api::CellMLElementSet* elS = childElements();
  iface::cellml_api::CellMLElementIterator* it = elS->iterate();
  iface::cellml_api::CellMLElement* n;
  while ((n = it->next()) != NULL)
  {
    bool removeThis = false;
    CDA_NamedCellMLElement* el = dynamic_cast<CDA_NamedCellMLElement*>(n);
    if (el != NULL)
    {
      RETURN_INTO_WSTRING(tn, el->datastore->localName());
      if (tn == type)
      {
        RETURN_INTO_WSTRING(elname, el->name());
        if (elname == name)
          removeThis = true;
      }
    }
    if (removeThis)
      removeElement(n);
    n->release_ref();
    if (removeThis)
      break;
  }
  it->release_ref();
  elS->release_ref();
}

bool
CDA_CellMLElement::validate()
  throw(std::exception&)
{
  // XXX TODO this needs to be done.
#ifdef SHOW_NOT_IMPLEMENTED
#error 'validate is not implemented yet.'
#endif
  return true;
}

iface::cellml_api::CellMLElement*
CDA_CellMLElement::parentElement()
  throw(std::exception&)
{
  iface::cellml_api::CellMLElement* el =
    dynamic_cast<iface::cellml_api::CellMLElement*>(mParent);
  if (el == NULL)
    return NULL;
  el->add_ref();
  return el;
}

iface::cellml_api::Model*
CDA_CellMLElement::modelElement()
  throw(std::exception&)
{
  CDA_CellMLElement* cand = this;

  while (true)
  {
    iface::cellml_api::Model* m =
      dynamic_cast<iface::cellml_api::Model*>(cand);
    if (m != NULL)
    {
      m->add_ref();
      return m;
    }

    CDA_CellMLElement* el =
      dynamic_cast<CDA_CellMLElement*>(cand->mParent);

    if (el == NULL)
    {
      // We have reached a point where no parent can be found, and we did not
      // hit a model on the way. Therefore, we are looking at a fragment which
      // does not have a modelElement.
      return NULL;
    }
    cand = el;
  }
}

void
CDA_CellMLElement::setUserData
(
 const wchar_t* key,
 iface::cellml_api::UserData* data
)
  throw(std::exception&)
{
  std::map<std::wstring,iface::cellml_api::UserData*>::iterator i;
  i = userData.find(key);
  if (i != userData.end())
  {
    (*i).second->release_ref();
    userData.erase(i);
  }

  if (data == NULL)
    return;

  data->add_ref();
  userData.insert(std::pair<std::wstring,iface::cellml_api::UserData*>
                  (key, data));
}

iface::cellml_api::UserData*
CDA_CellMLElement::getUserData(const wchar_t* key)
  throw(std::exception&)
{
  std::map<std::wstring,iface::cellml_api::UserData*>::iterator i;
  i = userData.find(key);
  if (i != userData.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }
  throw iface::cellml_api::CellMLException();
}

CDA_NamedCellMLElement::CDA_NamedCellMLElement
(
 iface::XPCOM::IObject* parent,
 iface::dom::Element* idata
)
  : CDA_CellMLElement(parent, idata)
{
}

CDA_NamedCellMLElement::~CDA_NamedCellMLElement()
{
}

wchar_t*
CDA_NamedCellMLElement::name()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"name");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_NamedCellMLElement::name
(
 const wchar_t* attr
)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"name", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

CDA_Model::CDA_Model(iface::cellml_api::DOMURLLoader* aLoader,
                     iface::dom::Document* aDoc,
                     iface::dom::Element* modelElement)
  : CDA_CellMLElement(NULL, modelElement),
    CDA_NamedCellMLElement(NULL, modelElement),
    mLoader(aLoader), mDoc(aDoc), mNextUniqueIdentifier(1)
{
  mDoc->add_ref();
}

CDA_Model::~CDA_Model()
{
  // XXX this code should run in ~CDA_CellMLElement, but libxml doesn't work
  // after the document has been destroyed.
  if (datastore != NULL)
  {
    datastore->release_ref();
    datastore = NULL;
  }
  if (children != NULL)
  {
    delete children;
    children = NULL;
  }

  mDoc->release_ref();
}

iface::cellml_api::Model*
CDA_Model::getAlternateVersion(const wchar_t* cellmlVersion)
  throw(std::exception&)
{
  try
  {
    const wchar_t* new_namespace;
    if (!wcscmp(cellmlVersion, L"1.0"))
      new_namespace = CELLML_1_0_NS;
    else if (!wcscmp(cellmlVersion, L"1.1"))
      new_namespace = CELLML_1_1_NS;
    else
      throw iface::cellml_api::CellMLException();
    
    iface::dom::DOMImplementation* di = mDoc->implementation();
    iface::dom::DocumentType* dt = mDoc->doctype();
    iface::dom::Document* newDoc =
      di->createDocument(new_namespace, L"model", dt);
    dt->release_ref();
    di->release_ref();

    // Now copy the current document into the old one...
    RecursivelyChangeVersionCopy(new_namespace, newDoc, mDoc);

    // newDoc needs a CellML wrapper...
    iface::dom::Element* de = mDoc->documentElement();
    CDA_Model* cm = new CDA_Model(mLoader, newDoc, de);
    de->release_ref();
    return cm;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_Model::RecursivelyChangeVersionCopy
(
 const wchar_t* aNewNamespace,
 iface::dom::Node* aCopy,
 iface::dom::Node* aOriginal
)
  throw(std::exception&)
{
  ObjRef<iface::dom::NodeList> nl(already_AddRefd<iface::dom::NodeList>(aOriginal->childNodes()));
  u_int32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    ObjRef<iface::dom::Node> origItem(already_AddRefd<iface::dom::Node>(nl->item(i)));
    ObjRef<iface::dom::Node> newItem;
    // See if we need to change the namespace...
    wchar_t* nsURI = newItem->namespaceURI();
    if (!wcscmp(nsURI, aNewNamespace))
    {
        free(nsURI);
      iface::dom::Node* tmp = origItem->cloneNode(false);
      newItem = already_AddRefd<iface::dom::Node>(tmp);
    }
    else if (!wcscmp(nsURI, CELLML_1_0_NS) || 
             !wcscmp(nsURI, CELLML_1_1_NS))
    {
      free(nsURI);
      // See what type of node it is...
      switch (origItem->nodeType())
      {
      case iface::dom::Node::ELEMENT_NODE:
        {
          wchar_t* ln = origItem->localName();
          newItem = already_AddRefd<iface::dom::Node>
            (mDoc->createElementNS(aNewNamespace, ln));
          free(ln);
          break;
        }
      case iface::dom::Node::ATTRIBUTE_NODE:
        {
          wchar_t* ln = origItem->localName();
          iface::dom::Attr* newAttr = 
            already_AddRefd<iface::dom::Attr>
            (
             mDoc->createAttributeNS
             (NULL_NS, ln)
            );
          free(ln);
          DECLARE_QUERY_INTERFACE_OBJREF(aCopyEl, aCopy, dom::Element);
          if (aCopyEl != NULL)
            aCopyEl->setAttributeNodeNS(newAttr);
          continue;
        }
      case iface::dom::Node::TEXT_NODE:
      case iface::dom::Node::CDATA_SECTION_NODE:
      case iface::dom::Node::ENTITY_REFERENCE_NODE:
      case iface::dom::Node::ENTITY_NODE:
      case iface::dom::Node::PROCESSING_INSTRUCTION_NODE:
      case iface::dom::Node::COMMENT_NODE:
      case iface::dom::Node::NOTATION_NODE:
          newItem = already_AddRefd<iface::dom::Node>(origItem->cloneNode(false));

      default:
        throw iface::cellml_api::CellMLException();
      }

    }
    aCopy->appendChild(newItem);
  }
}

iface::cellml_api::GroupSet*
CDA_Model::groups()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_GroupSet(allChildren);
}

iface::cellml_api::CellMLImportSet*
CDA_Model::imports()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (dynamic_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_CellMLImportSet(allChildren);
}

iface::cellml_api::URI*
CDA_Model::base_uri()
  throw(std::exception&)
{
  // Find the xml:base attribute...
  try
  {
    ObjRef<iface::dom::Attr> attr
      (already_AddRefd<iface::dom::Attr>
       (
        datastore->getAttributeNodeNS(L"http://www.w3.org/XML/1998/namespace",
                                      L"base")
       ));
    if (attr == NULL)
    {
      attr =
        already_AddRefd<iface::dom::Attr>
        (
         mDoc->createAttributeNS(L"http://www.w3.org/XML/1998/namespace",
                                 L"xml:base")
        );
      datastore->setAttributeNodeNS(attr);
    }
    return new CDA_URI(attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::UnitsSet*
CDA_Model::localUnits()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (dynamic_cast<CDA_CellMLElementSet*>(childElements())));

  return new CDA_UnitsSet(allChildren /*, false */);
}

iface::cellml_api::UnitsSet*
CDA_Model::modelUnits()
  throw(std::exception&)
{
  // Call localUnits, as that is a subset of our work...
  RETURN_INTO_OBJREF(lu, iface::cellml_api::UnitsSet, localUnits());
  // Now fetch the iterator...
  RETURN_INTO_OBJREF(lui, iface::cellml_api::CellMLElementIterator, lu->iterate());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());
  // And the import iterator...
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLElementIterator,
                     imp->iterate());

  // Next construct the special set...
  return new CDA_AllUnitsSet(lui, impi, false);
}

iface::cellml_api::UnitsSet*
CDA_Model::allUnits()
  throw(std::exception&)
{
  // Call localUnits, as that is a subset of our work...
  RETURN_INTO_OBJREF(lu, iface::cellml_api::UnitsSet, localUnits());
  // Now fetch the iterator...
  RETURN_INTO_OBJREF(lui, iface::cellml_api::CellMLElementIterator, lu->iterate());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());
  // And the import iterator...
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLElementIterator,
                     imp->iterate());

  // Next construct the special set...
  return new CDA_AllUnitsSet(lui, impi, true);
}

iface::cellml_api::CellMLComponentSet*
CDA_Model::localComponents()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (dynamic_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_CellMLComponentSet(allChildren);
}

iface::cellml_api::CellMLComponentSet*
CDA_Model::modelComponents()
  throw(std::exception&)
{
  // Call localComponents, as that is a subset of our work...
  RETURN_INTO_OBJREF(lc, iface::cellml_api::CellMLComponentSet, localComponents());
  // Now fetch the iterator...
  RETURN_INTO_OBJREF(lci, iface::cellml_api::CellMLElementIterator, lc->iterate());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());
  // And the import iterator...
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLElementIterator,
                     imp->iterate());

  // Next construct the special set...
  return new CDA_AllComponentSet(lci, impi, false);
}

iface::cellml_api::CellMLComponentSet*
CDA_Model::allComponents()
  throw(std::exception&)
{
  // Call localComponents, as that is a subset of our work...
  RETURN_INTO_OBJREF(lc, iface::cellml_api::CellMLComponentSet, localComponents());
  // Now fetch the iterator...
  RETURN_INTO_OBJREF(lci, iface::cellml_api::CellMLElementIterator, lc->iterate());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());
  // And the import iterator...
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLElementIterator,
                     imp->iterate());

  // Next construct the special set...
  return new CDA_AllComponentSet(lci, impi, true);
}

iface::cellml_api::ConnectionSet*
CDA_Model::connections()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (dynamic_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_ConnectionSet(allChildren);
}

iface::cellml_api::GroupSet*
CDA_Model::findGroupsWithRelationshipRefName
(
 const wchar_t* name
)
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (dynamic_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_GroupSet(allChildren, name);
}

void
CDA_Model::fullyInstantiateImports()
  throw(std::exception&)
{
  // Tell each import to instantiate itself...
  RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet, imports());
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                     imps->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       impi->nextImport());
    if (imp == NULL)
      break;
    imp->fullyInstantiate();
  }
}

iface::cellml_api::Model*
CDA_Model::generateFlattenedModel()
  throw(std::exception&)
{
#if 0
  // Firstly create a document...
  RETURN_INTO_OBJREF(di, iface::dom::DOMImplementation, mDoc->implementation());
  RETURN_INTO_OBJREF(dt, iface::dom::DocumentType, mDoc->doctype());
  // Just because its flat doesn't mean it has to be CellML 1.0.
  RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI());
  RETURN_INTO_OBJREF(newDoc, iface::dom::Document,
                     di->createDocument(nsURI.c_str(), L"model", dt));

  // Get the document element...
  RETURN_INTO_OBJREF(de, iface::dom::Element, mDoc->documentElement());

  // Append copies of all local components, building up a name map as we go...
  std::map<std::wstring,int> newComponentNames;
  std::map<std::pair<iface::cellml_api::Model*, std::wstring>, std::wstring>
    componentNameMap;

  RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                     localComponents());
  RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                     cs->iterateComponents());
  while (true)
  {
    RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                       ci->nextComponent());
    if (c == NULL)
      break;

    RETURN_INTO_OBJREF(cdnc, iface::dom::Node,
                       dynamic_cast<CDA_CellMLComponent*>(c)->
                       datastore->cloneNode(true));
    RETURN_INTO_WSTRING(cn, c->name());
    newComponentNames.insert(std::pair<std::wstring,int>(cn, 1));
    componentNameMap.insert(std::pair<std::pair<iface::cellml_api::Model*,
                                                std::wstring>,
                                      std::wstring>
                            (std::pair<iface::cellml_api::Model*,std::wstring>
                             (this, cn), cn));
    de->appendChild(cdnc)->release_ref();
  }

  // Likewise for units...
  std::map<std::wstring,int> newUnitsNames;
  std::map<std::pair<iface::cellml_api::Model*, std::wstring>, std::wstring>
    unitsNameMap;

  RETURN_INTO_OBJREF(us, iface::cellml_api::UnitsSet,
                     localUnits());
  RETURN_INTO_OBJREF(ui, iface::cellml_api::UnitsIterator,
                     us->iterateUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                       ui->nextUnits());
    if (u == NULL)
      break;

    RETURN_INTO_OBJREF(cdnc, iface::dom::Node,
                       dynamic_cast<CDA_Units*>(u)->
                       datastore->cloneNode(true));
    RETURN_INTO_WSTRING(cn, u->name());
    newUnitsNames.insert(std::pair<std::wstring,int>(cn, 1));
    de->appendChild(cdnc)->release_ref();
  }

  // Any import components mentioned in the base model are always included.
  std::list<CDA_ImportComponent*> importComponentQueue;
  std::list<CDA_Model*> modelsForConnectionProcessingQueue;
  
  // 
  

  // Create a CellML wrapper...
  RETURN_INTO_OBJREF(cm, CDA_Model, new CDA_Model(mLoader, newDoc, de));

  cm->add_ref();
  return cm;
#endif

  // This is not implemented yet, and may never be.
  throw iface::cellml_api::CellMLException();
}

iface::cellml_api::CellMLComponent*
CDA_Model::createComponent()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"component"));
    return new CDA_CellMLComponent(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ImportComponent*
CDA_Model::createImportComponent()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"component"));
    return new CDA_ImportComponent(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::Units*
CDA_Model::createUnits()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"units"));
    return new CDA_Units(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ImportUnits*
CDA_Model::createImportUnits()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"units"));
    return new CDA_ImportUnits(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::Unit*
CDA_Model::createUnit()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"unit"));
    return new CDA_Unit(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLImport*
CDA_Model::createCellMLImport()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"import"));
    return new CDA_CellMLImport(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLVariable*
CDA_Model::createCellMLVariable()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"variable"));
    return new CDA_CellMLVariable(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ComponentRef*
CDA_Model::createComponentRef()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"component_ref"));
    return new CDA_ComponentRef(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::RelationshipRef*
CDA_Model::createRelationshipRef()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"relationship_ref"));
    return new CDA_RelationshipRef(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::Group*
CDA_Model::createGroup()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"group"));
    return new CDA_Group(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::Connection*
CDA_Model::createConnection()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"connection"));
    return new CDA_Connection(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::MapComponents*
CDA_Model::createMapComponents()
  throw(std::exception&)
{
  // Get our namespace, and use it for the new node.
  try
  {
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"map_components"));
    return new CDA_MapComponents(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::MapVariables*
CDA_Model::createMapVariables()
  throw(std::exception&)
{
  // Get our namespace, and use it for the new node.
  try
  {
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"map_variables"));
    return new CDA_MapVariables(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::Reaction*
CDA_Model::createReaction()
  throw(std::exception&)
{
  // Get our namespace, and use it for the new node.
  try
  {
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"reaction"));
    return new CDA_Reaction(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ReactantVariableRef*
CDA_Model::createReactantVariableRef()
  throw(std::exception&)
{
  // Get our namespace, and use it for the new node.
  try
  {
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"variable_ref"));
    return new CDA_ReactantVariableRef(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::RateVariableRef*
CDA_Model::createRateVariableRef()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"variable_ref"));
    return new CDA_RateVariableRef(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ReactantRole*
CDA_Model::createReactantRole()
  throw(std::exception&)
{
  // Get our namespace, and use it for the new node.
  try
  {
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"role"));
    newNode->setAttributeNS(NULL_NS, L"role", L"reactant");
    return new CDA_ReactantRole(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ProductRole*
CDA_Model::createProductRole()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"role"));
    newNode->setAttributeNS(NULL_NS,
                            L"role", L"product");
    return new CDA_ProductRole(NULL, newNode);
  }
  catch(iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::RateRole*
CDA_Model::createRateRole()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());

    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"role"));    
    newNode->setAttributeNS(NULL_NS,
                            L"role", L"rate");

    return new CDA_RateRole(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

u_int32_t
CDA_Model::assignUniqueIdentifier()
{
  return mNextUniqueIdentifier++;
}

CDA_MathContainer::CDA_MathContainer(iface::XPCOM::IObject* parent,
                                     iface::dom::Element* modelElement)
  : CDA_CellMLElement(parent, modelElement)
{
}

iface::cellml_api::MathList*
CDA_MathContainer::math()
  throw(std::exception&)
{
  try
  {
    return new CDA_MathList(datastore);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_MathContainer::addMath
(
 const iface::cellml_api::MathMLElement x
)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->appendChild(x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_MathContainer::removeMath
(
 const iface::cellml_api::MathMLElement x
)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->removeChild(x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_MathContainer::replaceMath
(
 const iface::cellml_api::MathMLElement x,
 const iface::cellml_api::MathMLElement y
)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->replaceChild(y, x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_MathContainer::clearMath()
  throw(std::exception&)
{
  try
  {
    ObjRef<iface::dom::NodeList> nl(already_AddRefd<iface::dom::NodeList>
                                    (datastore->childNodes()));
    // Go through and find all math nodes...
    u_int32_t i, l = nl->length();
    for (i = 0; i < l; i++)
    {
      ObjRef<iface::dom::Node> node
        (already_AddRefd<iface::dom::Node>(nl->item(i)));
      // See if it is a MathML element...
      DECLARE_QUERY_INTERFACE_OBJREF(mme, node, mathml_dom::MathMLElement);
      if (mme == NULL)
        continue;
      // It is a MathML element. Remove it...
      removeMath(mme);
    }
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLVariableSet*
CDA_CellMLComponent::variables()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_CellMLVariableSet(allChildren);
}


iface::cellml_api::UnitsSet*
CDA_CellMLComponent::units()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_UnitsSet(allChildren);
}

iface::cellml_api::ConnectionSet*
CDA_CellMLComponent::connections()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_ConnectionSet(allChildren);
}

static CDA_ComponentRef*
RecursivelySearchCR(CDA_ComponentRef *cr, const wchar_t *cn)
{
  wchar_t* cn2 = cr->componentName();
  int match = (wcscmp(cn, cn2));
  free(cn2);

  if (match)
    return cr;

  RETURN_INTO_OBJREF(crs, iface::cellml_api::ComponentRefSet,
                     cr->componentRefs());
  RETURN_INTO_OBJREF(cri, iface::cellml_api::ComponentRefIterator,
                     crs->iterateComponentRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(cr2, iface::cellml_api::ComponentRef,
                       cri->nextComponentRef());
    if (cr2 == NULL)
      break;

    CDA_ComponentRef* hit =
      RecursivelySearchCR(static_cast<CDA_ComponentRef*>(cr2.getPointer()),
                          cn);
    if (hit != NULL)
      return hit;
  }

  return NULL;
}

iface::cellml_api::CellMLComponent*
CDA_CellMLComponentGroupMixin::encapsulationParent()
  throw(std::exception&)
{
  // Find the name in this model...
  RETURN_INTO_WSTRING(componentName, name());

  CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>(mParent);
  CDA_Model* currentModel = dynamic_cast<CDA_Model*>(mParent);
  if (imp != NULL)
    currentModel = dynamic_cast<CDA_Model*>(imp->mParent);
  while (currentModel)
  {
    // We need to search for groups and recurse into them...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet, currentModel->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator, groupSet->iterateGroups());

    // We now have the group iterator,
    while (true)
    {
      RETURN_INTO_OBJREF(group, iface::cellml_api::Group, groupIt->nextGroup());
      if (group == NULL)
        break;
      if (!group->isEncapsulation())
        continue;

      RETURN_INTO_OBJREF(comprs, iface::cellml_api::ComponentRefSet,
                         group->componentRefs());
      RETURN_INTO_OBJREF(compri, iface::cellml_api::ComponentRefIterator,
                         comprs->iterateComponentRefs());
      while (true)
      {
        RETURN_INTO_OBJREF(compr, iface::cellml_api::ComponentRef,
                           compri->nextComponentRef());
        // See if we can find an appropriate relationship ref...
        if (compr == NULL)
          break;

        CDA_ComponentRef* crf =
          RecursivelySearchCR(static_cast<CDA_ComponentRef*>
                              (compr.getPointer()),
                              componentName.c_str());

        if (crf == NULL)
          continue;

        // See if the parent is a relationship ref too...
        CDA_ComponentRef* crp =
          dynamic_cast<CDA_ComponentRef*>(crf->mParent);
        if (crp == NULL)
          continue;

        // crp is the parent of this relationship ref. Get the name.
        RETURN_INTO_WSTRING(parname, crp->componentName());

        while (true)
        {
          // Find the component in this model...
          RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                             currentModel->modelComponents());
          RETURN_INTO_OBJREF(fc, iface::cellml_api::NamedCellMLElement,
                             mc->get(componentName.c_str()));
          
          // If it is missing, then the CellML is malformed.
          if (fc == NULL)
            throw iface::cellml_api::CellMLException();

          // See if it is an ImportComponent...
          CDA_ImportComponent* ic =
            dynamic_cast<CDA_ImportComponent*>(fc.getPointer());
          if (ic == NULL)
          {
            iface::cellml_api::CellMLComponent* fcr = 
              dynamic_cast<iface::cellml_api::CellMLComponent*>
              (fc.getPointer());
            fcr->add_ref();
            return fcr;
          }
          // We now have an ImportComponent, but we want the real one if we can
          // get it.
          currentModel =
            dynamic_cast<CDA_Model*>(
            dynamic_cast<CDA_CellMLImport*>(ic->mParent)->importedModel);
          // If the real component isn't instantiated yet, return the
          // ImportComponent...
          if (currentModel == NULL)
          {
            ic->add_ref();
            return ic;
          }
          wchar_t *cr = ic->componentRef();
          componentName = cr;
          free(cr);
        }
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the parent. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...
    RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet,
                       currentModel->imports());
    RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                       imps->iterateImports());

    bool foundImport = false;
    while (true)
    {
      RETURN_INTO_OBJREFD(ni, CDA_CellMLImport, impi->nextImport());
      if (ni == NULL)
        break;

      RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                         ni->components());
      RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                         cs->iterateComponents());
      while (true)
      {
        RETURN_INTO_OBJREFD(c, iface::cellml_api::ImportComponent,
                            ci->nextComponent());
        if (c == NULL)
          break;
        RETURN_INTO_WSTRING(cn, c->name());
        if (cn != componentName)
          continue;

        wchar_t* cr = c->componentRef();
        componentName = cr;
        free(cr);

        currentModel = dynamic_cast<CDA_Model*>(ni->importedModel);
        if (currentModel == NULL)
          throw iface::cellml_api::CellMLException();
        foundImport = true;
        break;
      }
      if (foundImport)
        break;
    }

    // It looks like there is no import parent...
    if (!foundImport)
      return NULL;
  }

  return NULL;
}

iface::cellml_api::CellMLComponentSet*
CDA_CellMLComponentGroupMixin::encapsulationChildren()
  throw(std::exception&)
{
  // Find the name in this model...
  RETURN_INTO_WSTRING(componentName, name());

  CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>(mParent);
  CDA_Model* currentModel = dynamic_cast<CDA_Model*>(mParent);
  if (imp != NULL)
    currentModel = dynamic_cast<CDA_Model*>(imp->mParent);

  while (currentModel)
  {
    // We need to search for groups and recurse into them...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet, currentModel->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator, groupSet->iterateGroups());

    // We now have the group iterator,
    while (true)
    {
      RETURN_INTO_OBJREF(group, iface::cellml_api::Group, groupIt->nextGroup());
      if (group == NULL)
        break;
      if (!group->isEncapsulation())
        continue;

      RETURN_INTO_OBJREF(comprs, iface::cellml_api::ComponentRefSet,
                         group->componentRefs());
      RETURN_INTO_OBJREF(compri, iface::cellml_api::ComponentRefIterator,
                         comprs->iterateComponentRefs());
      while (true)
      {
        RETURN_INTO_OBJREF(compr, iface::cellml_api::ComponentRef,
                           compri->nextComponentRef());
        // See if we can find an appropriate relationship ref...
        if (compr == NULL)
          break;

        CDA_ComponentRef* crf =
          RecursivelySearchCR(static_cast<CDA_ComponentRef*>
                              (compr.getPointer()),
                              componentName.c_str());

        if (crf == NULL)
          continue;

        // See if the component ref has children...
        RETURN_INTO_OBJREF(comprsc, iface::cellml_api::ComponentRefSet,
                           crf->componentRefs());
        // If it has no children, we need to consider other component refs...
        if (comprsc->length() == 0)
          continue;
        RETURN_INTO_OBJREF(compric, iface::cellml_api::ComponentRefIterator,
                           comprsc->iterateComponentRefs());
        return new CDA_CellMLComponentFromComponentRefSet(currentModel, compric);
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the children. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...
    RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet,
                       currentModel->imports());
    RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                       imps->iterateImports());

    bool foundImport = false;
    while (true)
    {
      RETURN_INTO_OBJREFD(ni, CDA_CellMLImport, impi->nextImport());
      if (ni == NULL)
        break;

      RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                         ni->components());
      RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                         cs->iterateComponents());
      while (true)
      {
        RETURN_INTO_OBJREFD(c, iface::cellml_api::ImportComponent,
                            ci->nextComponent());
        if (c == NULL)
          break;
        RETURN_INTO_WSTRING(cn, c->name());
        if (cn != componentName)
          continue;

        wchar_t* cr = c->componentRef();
        componentName = cr;
        free(cr);

        currentModel = dynamic_cast<CDA_Model*>(ni->importedModel);
        if (currentModel == NULL)
          throw iface::cellml_api::CellMLException();
        foundImport = true;
        break;
      }
      if (foundImport)
        break;
    }

    // It looks like there is no import parent...
    if (!foundImport)
      return NULL;
  }

  return NULL;
}

iface::cellml_api::CellMLComponent*
CDA_CellMLComponentGroupMixin::containmentParent()
  throw(std::exception&)
{
  // Find the name in this model...
  RETURN_INTO_WSTRING(componentName, name());

  CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>(mParent);
  CDA_Model* currentModel = dynamic_cast<CDA_Model*>(mParent);
  if (imp != NULL)
    currentModel = dynamic_cast<CDA_Model*>(imp->mParent);

  while (currentModel)
  {
    // We need to search for groups and recurse into them...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet, currentModel->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator, groupSet->iterateGroups());

    // We now have the group iterator,
    while (true)
    {
      RETURN_INTO_OBJREF(group, iface::cellml_api::Group, groupIt->nextGroup());
      if (group == NULL)
        break;
      if (!group->isContainment())
        continue;

      RETURN_INTO_OBJREF(comprs, iface::cellml_api::ComponentRefSet,
                         group->componentRefs());
      RETURN_INTO_OBJREF(compri, iface::cellml_api::ComponentRefIterator,
                         comprs->iterateComponentRefs());
      while (true)
      {
        RETURN_INTO_OBJREF(compr, iface::cellml_api::ComponentRef,
                           compri->nextComponentRef());
        // See if we can find an appropriate relationship ref...
        if (compr == NULL)
          break;

        CDA_ComponentRef* crf =
          RecursivelySearchCR(static_cast<CDA_ComponentRef*>
                              (compr.getPointer()), componentName.c_str());

        if (crf == NULL)
          continue;

        // See if the parent is a relationship ref too...
        CDA_ComponentRef* crp =
          dynamic_cast<CDA_ComponentRef*>(crf->mParent);
        if (crp == NULL)
          continue;

        // crp is the parent of this relationship ref. Get the name.
        RETURN_INTO_WSTRING(parname, crp->componentName());

        while (true)
        {
          // Find the component in this model...
          RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                             currentModel->modelComponents());
          RETURN_INTO_OBJREF(fc, iface::cellml_api::NamedCellMLElement,
                             mc->get(componentName.c_str()));
          
          // If it is missing, then the CellML is malformed.
          if (fc == NULL)
            throw iface::cellml_api::CellMLException();

          // See if it is an ImportComponent...
          CDA_ImportComponent* ic =
            dynamic_cast<CDA_ImportComponent*>(fc.getPointer());
          if (ic == NULL)
          {
            iface::cellml_api::CellMLComponent* fcr = 
              dynamic_cast<iface::cellml_api::CellMLComponent*>(fc.getPointer());
            fcr->add_ref();
            return fcr;
          }
          // We now have an ImportComponent, but we want the real one if we can
          // get it.
          currentModel =
            dynamic_cast<CDA_Model*>(
                                     dynamic_cast<CDA_CellMLImport*>
                                     (ic->mParent)->importedModel
                                    );
          // If the real component isn't instantiated yet, return the
          // ImportComponent...
          if (currentModel == NULL)
          {
            ic->add_ref();
            return ic;
          }
          wchar_t *cr = ic->componentRef();
          componentName = cr;
          free(cr);
        }
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the parent. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...
    RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet,
                       currentModel->imports());
    RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                       imps->iterateImports());

    bool foundImport = false;
    while (true)
    {
      RETURN_INTO_OBJREFD(ni, CDA_CellMLImport, impi->nextImport());
      if (ni == NULL)
        break;

      RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                         ni->components());
      RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                         cs->iterateComponents());
      while (true)
      {
        RETURN_INTO_OBJREFD(c, iface::cellml_api::ImportComponent,
                            ci->nextComponent());
        if (c == NULL)
          break;
        RETURN_INTO_WSTRING(cn, c->name());
        if (cn != componentName)
          continue;

        wchar_t* cr = c->componentRef();
        componentName = cr;
        free(cr);

        currentModel = dynamic_cast<CDA_Model*>(ni->importedModel);
        if (currentModel == NULL)
          throw iface::cellml_api::CellMLException();
        foundImport = true;
        break;
      }
      if (foundImport)
        break;
    }

    // It looks like there is no import parent...
    if (!foundImport)
      return NULL;
  }

  return NULL;
}

iface::cellml_api::CellMLComponentSet*
CDA_CellMLComponentGroupMixin::containmentChildren()
  throw(std::exception&)
{
  // Find the name in this model...
  RETURN_INTO_WSTRING(componentName, name());

  CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>(mParent);
  CDA_Model* currentModel = dynamic_cast<CDA_Model*>(mParent);
  if (imp != NULL)
    currentModel = dynamic_cast<CDA_Model*>(imp->mParent);

  while (currentModel)
  {
    // We need to search for groups and recurse into them...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet, currentModel->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator, groupSet->iterateGroups());

    // We now have the group iterator,
    while (true)
    {
      RETURN_INTO_OBJREF(group, iface::cellml_api::Group, groupIt->nextGroup());
      if (group == NULL)
        break;
      if (!group->isContainment())
        continue;

      RETURN_INTO_OBJREF(comprs, iface::cellml_api::ComponentRefSet,
                         group->componentRefs());
      RETURN_INTO_OBJREF(compri, iface::cellml_api::ComponentRefIterator,
                         comprs->iterateComponentRefs());
      while (true)
      {
        RETURN_INTO_OBJREF(compr, iface::cellml_api::ComponentRef,
                           compri->nextComponentRef());
        // See if we can find an appropriate relationship ref...
        if (compr == NULL)
          break;

        CDA_ComponentRef* crf =
          RecursivelySearchCR(static_cast<CDA_ComponentRef*>
                              (compr.getPointer()), componentName.c_str());

        if (crf == NULL)
          continue;

        // See if the component ref has children...
        RETURN_INTO_OBJREF(comprsc, iface::cellml_api::ComponentRefSet,
                           crf->componentRefs());
        // If it has no children, we need to consider other component refs...
        if (comprsc->length() == 0)
          continue;
        RETURN_INTO_OBJREF(compric, iface::cellml_api::ComponentRefIterator,
                           comprsc->iterateComponentRefs());
        return new CDA_CellMLComponentFromComponentRefSet
          (currentModel, compric);
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the children. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...
    RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet,
                       currentModel->imports());
    RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                       imps->iterateImports());

    bool foundImport = false;
    while (true)
    {
      RETURN_INTO_OBJREFD(ni, CDA_CellMLImport, impi->nextImport());
      if (ni == NULL)
        break;

      RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                         ni->components());
      RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                         cs->iterateComponents());
      while (true)
      {
        RETURN_INTO_OBJREFD(c, iface::cellml_api::ImportComponent,
                            ci->nextComponent());
        if (c == NULL)
          break;
        RETURN_INTO_WSTRING(cn, c->name());
        if (cn != componentName)
          continue;

        wchar_t* cr = c->componentRef();
        componentName = cr;
        free(cr);

        currentModel = dynamic_cast<CDA_Model*>(ni->importedModel);
        if (currentModel == NULL)
          throw iface::cellml_api::CellMLException();
        foundImport = true;
        break;
      }
      if (foundImport)
        break;
    }

    // It looks like there is no import parent...
    if (!foundImport)
      return NULL;
  }

  return NULL;
}

u_int32_t
CDA_CellMLComponent::importNumber()
  throw(std::exception&)
{
  // Get our model...
  ObjRef<CDA_Model> modelEl(already_AddRefd<CDA_Model>
                            (dynamic_cast<CDA_Model*>(modelElement())));
  if (modelEl == NULL)
    return 0;
  CDA_CellMLImport* impEl = dynamic_cast<CDA_CellMLImport*>(modelEl->mParent);
  if (impEl == NULL)
    return 0;
  
  return impEl->uniqueIdentifier();
}

bool
CDA_UnitsBase::isBaseUnits()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    RETURN_INTO_WSTRING(baseUnits, datastore->getAttributeNS(NULL_NS, L"base_units"));
    if (baseUnits == L"yes")
      return true;
    return false;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_UnitsBase::isBaseUnits(bool attr)
  throw(std::exception&)
{
  try
  {
    const wchar_t* str = attr ? L"yes" : L"no";
    datastore->setAttribute(L"base_units", str);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::UnitSet*
CDA_UnitsBase::unitCollection()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_UnitSet(allChildren);
}

static struct
{
  const wchar_t* prefix;
  int32_t value;
} PrefixTable[] =
{
  {L"atto", -18},
  {L"centi", -2},
  {L"deci", -1},
  {L"deka", 1},
  {L"exa", 18},
  {L"femto", -15},
  {L"giga", 9},
  {L"hecto", 2},
  {L"kilo", 3},
  {L"mega", 6},
  {L"micro", -6},
  {L"milli", -3},
  {L"nano", -9},
  {L"peta", 15},
  {L"pico", -12},
  {L"tera", 12},
  {L"yocto", -24},
  {L"yotta", 24},
  {L"zepto", -21},
  {L"zetta", 21}
};

static struct
{
  const wchar_t* prefix;
  int32_t value;
} PrefixNumberTable[] =
{
  {L"yocto", -24},
  {NULL, -23},
  {NULL, -22},
  {L"zepto", -21},
  {NULL, -20},
  {NULL, -19},
  {L"atto", -18},
  {NULL, -17},
  {NULL, -16},
  {L"femto", -15},
  {NULL, -14},
  {NULL, -13},
  {L"pico", -12},
  {NULL, -11},
  {NULL, -10},
  {L"nano", -9},
  {NULL, -8},
  {NULL, -7},
  {L"micro", -6},
  {NULL, -5},
  {NULL, -4},
  {L"milli", -3},
  {L"centi", -2},
  {L"deci", -1},
  {NULL, 0},
  {L"deka", 1},
  {L"hecto", 2},
  {L"kilo", 3},
  {NULL, 4},
  {NULL, 5},
  {L"mega", 6},
  {NULL, 7},
  {NULL, 8},
  {L"giga", 9},
  {NULL, 10},
  {NULL, 11},
  {L"tera", 12},
  {NULL, 13},
  {NULL, 14},
  {L"peta", 15},
  {NULL, 16},
  {NULL, 17},
  {L"exa", 18},
  {NULL, 19},
  {NULL, 20},
  {L"zetta", 21},
  {NULL, 22},
  {NULL, 23},
  {L"yotta", 24}
};

int32_t
CDA_Unit::prefix()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    RETURN_INTO_WSTRING(prefix, datastore->getAttributeNS(NULL_NS, L"prefix"));

    if (prefix == L"")
      return 0;

    const wchar_t* prefixWC = prefix.c_str();
    if ((prefixWC[0] >= '0' && prefixWC[0] <= '9') ||
        (prefixWC[0] == '-' && prefixWC[1] >= '0' && prefixWC[1] <= '9'))
    {
      wchar_t* endPtr;
      int32_t ret = wcstol(prefixWC, &endPtr, 10);
      if (*endPtr != 0)
        throw iface::cellml_api::CellMLException();
      return ret;
    }
    int32_t lowerBound = 0;
    int32_t upperBound = sizeof(PrefixTable) / sizeof(PrefixTable[0]);
    while (lowerBound <= upperBound)
    {
      int32_t choice = (upperBound + lowerBound) / 2;
      int32_t cval = wcscasecmp(PrefixTable[choice].prefix, prefixWC);
      if (cval == 0)
        return PrefixTable[choice].value;
      else if (cval < 0)
        // The prefix in the table is less than the input, so the input must be
        // after choice...
        lowerBound = choice + 1;
      else
        upperBound = choice - 1;
    }
    throw iface::cellml_api::CellMLException();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_Unit::prefix(int32_t attr)
  throw(std::exception&)
{
  try
  {
    if (attr == 0)
    {
      datastore->removeAttribute(L"prefix");
      return;
    }

    if (attr >= -24 && attr <= 24 &&
        PrefixNumberTable[attr + 24].prefix != NULL)
    {
      datastore->setAttribute(L"prefix", PrefixNumberTable[attr + 24].prefix);
      return;
    }

    // We have a non-SI prefix, so we need to express it as a number...
    wchar_t buf[12];
    swprintf(buf, 12, L"%d", attr);
    datastore->setAttribute(L"prefix", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

double
CDA_Unit::multiplier()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    wchar_t* mupstr = datastore->getAttributeNS(NULL_NS, L"multiplier");
    if (mupstr[0] == 0)
    {
      free(mupstr);
      return 1.0;
    }
    wchar_t* endstr;
    double val = wcstod(mupstr, &endstr);
    bool invalid = false;
    if (*endstr == 0)
      invalid = true;
    free(mupstr);

    if (invalid)
      throw iface::cellml_api::CellMLException();

    return val;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_Unit::multiplier(double attr)
  throw(std::exception&)
{
  try
  {
    if (attr == 1.0)
    {
      datastore->removeAttribute(L"multiplier");
      return;
    }

    wchar_t buf[24];
    swprintf(buf, 12, L"%g", attr);
    datastore->setAttribute(L"multipler", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

double
CDA_Unit::offset()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    wchar_t* offstr = datastore->getAttributeNS(NULL_NS, L"offset");
    if (offstr[0] == 0)
    {
      free(offstr);
      return 0.0;
    }
    wchar_t* endstr;
    double val = wcstod(offstr, &endstr);
    bool invalid = false;
    if (*endstr == 0)
      invalid = true;
    free(offstr);

    if (invalid)
      throw iface::cellml_api::CellMLException();

    return val;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_Unit::offset(double attr)
  throw(std::exception&)
{
  try
  {
    if (attr == 0.0)
    {
      datastore->removeAttribute(L"offset");
      return;
    }

    wchar_t buf[24];
    swprintf(buf, 12, L"%g", attr);
    datastore->setAttribute(L"offset", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

double
CDA_Unit::exponent()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    wchar_t* expstr = datastore->getAttributeNS(NULL_NS, L"exponent");
    if (expstr[0] == 0)
    {
      free(expstr);
      return 0.0;
    }
    wchar_t* endstr;
    double val = wcstod(expstr, &endstr);
    bool invalid = false;
    if (*endstr == 0)
      invalid = true;
    free(expstr);

    if (invalid)
      throw iface::cellml_api::CellMLException();

    return val;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_Unit::exponent(double attr)
  throw(std::exception&)
{
  try
  {
    if (attr == 1.0)
    {
      datastore->removeAttribute(L"exponent");
      return;
    }

    wchar_t buf[24];
    swprintf(buf, 12, L"%g", attr);
    datastore->setAttribute(L"exponent", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

wchar_t*
CDA_Unit::units()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    return datastore->getAttributeNS(NULL_NS, L"units");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_Unit::units(const wchar_t* attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"units", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::URI*
CDA_CellMLImport::xlinkHref()
  throw(std::exception&)
{
  try
  {
    ObjRef<iface::dom::Attr> attrNode =
      already_AddRefd<iface::dom::Attr>
      (datastore->getAttributeNodeNS(XLINK_NS, L"href"));
    if (attrNode == NULL)
    {
      RETURN_INTO_OBJREF(doc, iface::dom::Document, datastore->ownerDocument());
      attrNode = already_AddRefd<iface::dom::Attr>
        (doc->createAttributeNS(XLINK_NS, L"href"));
    }
    return new CDA_URI(attrNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ImportComponentSet*
CDA_CellMLImport::components()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_ImportComponentSet(allChildren);
}

iface::cellml_api::ImportUnitsSet*
CDA_CellMLImport::units()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_ImportUnitsSet(allChildren);
}

iface::cellml_api::ConnectionSet*
CDA_CellMLImport::importedConnections()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );
  return new CDA_ConnectionSet(allChildren);
}

void
CDA_CellMLImport::fullyInstantiate()
  throw(std::exception&)
{
  // If this import has already been instantiated, throw an exception....
  if (importedModel != NULL)
    throw iface::cellml_api::CellMLException();

  // We need to get hold of the top level CellML model...
  CDA_CellMLElement *lastEl = NULL, *nextEl;

  nextEl = dynamic_cast<CDA_CellMLElement*>(mParent);
  while (nextEl != NULL)
  {
    lastEl = nextEl;
    nextEl = dynamic_cast<CDA_CellMLElement*>(lastEl->mParent);
  }
  CDA_Model *rootModel = dynamic_cast<CDA_Model*>(lastEl);
  if (rootModel == NULL)
    throw iface::cellml_api::CellMLException();

  // Get the URL...
  RETURN_INTO_OBJREF(url, iface::cellml_api::URI, xlinkHref());
  RETURN_INTO_WSTRING(urlStr, url->asText());

  // We now have a root model, and so we also have a loader...
  RETURN_INTO_OBJREF(dd, iface::dom::Document,
                     rootModel->mLoader->loadDocument(urlStr.c_str()));

  try
  {
    RETURN_INTO_OBJREF(modelEl, iface::dom::Element,
                      dd->documentElement());
    if (modelEl == NULL)
      throw iface::cellml_api::CellMLException();

    // Check it is a CellML model...
    RETURN_INTO_WSTRING(nsURI, modelEl->namespaceURI());
    if (nsURI != CELLML_1_0_NS &&
        nsURI != CELLML_1_1_NS)
      throw iface::cellml_api::CellMLException();

    RETURN_INTO_WSTRING(modName, modelEl->localName());
    if (modName != L"model")
      throw iface::cellml_api::CellMLException();

    CDA_Model* cm = new CDA_Model(rootModel->mLoader, dd, modelEl);
    importedModel = cm;

    // Adjust the refcounts to leave the importedModel completely dependent on
    // the rest of the tree...
    cm->mParent = this;
    add_ref();
    cm->release_ref();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

bool
CDA_CellMLImport::wasFullyInstantiated()
  throw(std::exception&)
{
  return (importedModel != NULL);
}

u_int32_t
CDA_CellMLImport::uniqueIdentifier()
  throw(std::exception&)
{
  // Find the true of the parent tree...
  CDA_CellMLElement* possibleRoot, *lastChoice = NULL;

  possibleRoot = dynamic_cast<CDA_CellMLElement*>(mParent);
  while (possibleRoot != NULL)
  {
    lastChoice = possibleRoot;
    possibleRoot = dynamic_cast<CDA_CellMLElement*>(possibleRoot->mParent);
  }

  CDA_Model* m =
    dynamic_cast<CDA_Model*>(lastChoice);

  // If our root is not a model, we give an exception...
  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  if (m != lastIdentifierModel())
  {
    mUniqueIdentifier = m->assignUniqueIdentifier();
    lastIdentifierModel = m;
  }

  return mUniqueIdentifier;
}

CDA_CellMLComponent*
CDA_ImportComponent::fetchDefinition()
  throw(std::exception&)
{
  // Fetch the name in the parent...
  CDA_CellMLImport* cmi = dynamic_cast<CDA_CellMLImport*>(mParent);
  if (cmi == NULL)
    throw iface::cellml_api::CellMLException();

  // We need imports to be instantiated for this to work, too...
  if (cmi->importedModel == NULL)
    throw iface::cellml_api::CellMLException();

  // Fetch the components...
  RETURN_INTO_WSTRING(compName, componentRef());

  // Now search for it in the imported model...
  CDA_Model* m = dynamic_cast<CDA_Model*>(cmi->importedModel);

  RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                     m->modelComponents());
  RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                     mc->getComponent(compName.c_str()));
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_ImportComponent* ic =
    dynamic_cast<CDA_ImportComponent*>(c.getPointer());
  if (ic != NULL)
  {
    // Just recurse to finish the work...
    return ic->fetchDefinition();
  }

  // We are documented as not calling add_ref. This design might need to be
  // changed for threadsafety later.
  return dynamic_cast<CDA_CellMLComponent*>(c.getPointer());
}

iface::cellml_api::CellMLVariableSet*
CDA_ImportComponent::variables()
  throw(std::exception&)
{
  return fetchDefinition()->variables();
}

iface::cellml_api::UnitsSet*
CDA_ImportComponent::units()
  throw(std::exception&)
{
  return fetchDefinition()->units();
}

iface::cellml_api::ConnectionSet*
CDA_ImportComponent::connections()
  throw(std::exception&)
{
  return fetchDefinition()->connections();
}

u_int32_t
CDA_ImportComponent::importNumber()
  throw(std::exception&)
{
  // Get our model...
  ObjRef<CDA_Model> modelEl(already_AddRefd<CDA_Model>
                            (dynamic_cast<CDA_Model*>(modelElement())));
  if (modelEl == NULL)
    return 0;
  CDA_CellMLImport* impEl = dynamic_cast<CDA_CellMLImport*>(modelEl->mParent);
  if (impEl == NULL)
    return 0;
  
  return impEl->uniqueIdentifier();
}

wchar_t*
CDA_ImportComponent::componentRef()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    return datastore->getAttributeNS(NULL_NS, L"component_ref");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_ImportComponent::componentRef(const wchar_t* attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"component_ref", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

wchar_t*
CDA_ImportUnits::unitsRef()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    return datastore->getAttributeNS(NULL_NS, L"units_ref");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_ImportUnits::unitsRef(const wchar_t* attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"units_ref", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

wchar_t*
CDA_CellMLVariable::initialValue()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    return datastore->getAttributeNS(NULL_NS, L"initial_value");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLVariable::initialValue
(
 const wchar_t* attr
)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"initial_value", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::VariableInterface
CDA_CellMLVariable::privateInterface()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    RETURN_INTO_WSTRING(privint,
                        datastore->getAttributeNS(NULL_NS,
                                                  L"private_interface"));
    if (privint == L"")
      return iface::cellml_api::INTERFACE_NONE;
    if (privint == L"in")
      return iface::cellml_api::INTERFACE_IN;
    else if (privint == L"out")
      return iface::cellml_api::INTERFACE_OUT;
    else if (privint == L"none")
      return iface::cellml_api::INTERFACE_NONE;
    throw iface::cellml_api::CellMLException();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLVariable::privateInterface(iface::cellml_api::VariableInterface attr)
  throw(std::exception&)
{
  try
  {
    if (attr == iface::cellml_api::INTERFACE_NONE)
    {
      datastore->removeAttribute(L"private_interface");
      return;
    }

    const wchar_t* str;
    switch (attr)
    {
    case iface::cellml_api::INTERFACE_IN:
      str = L"in";
      break;
    case iface::cellml_api::INTERFACE_OUT:
      str = L"out";
      break;
    default:
      throw iface::cellml_api::CellMLException();
    }

    datastore->setAttribute(L"private_interface", str);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::VariableInterface
CDA_CellMLVariable::publicInterface()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    RETURN_INTO_WSTRING(pubint,
                        datastore->getAttributeNS(NULL_NS,
                                                  L"public_interface"));
    if (pubint == L"")
      return iface::cellml_api::INTERFACE_NONE;
    if (pubint == L"in")
      return iface::cellml_api::INTERFACE_IN;
    else if (pubint == L"out")
      return iface::cellml_api::INTERFACE_OUT;
    else if (pubint == L"none")
      return iface::cellml_api::INTERFACE_NONE;
    throw iface::cellml_api::CellMLException();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_CellMLVariable::publicInterface(iface::cellml_api::VariableInterface attr)
  throw(std::exception&)
{
  try
  {
    if (attr == iface::cellml_api::INTERFACE_NONE)
    {
      datastore->removeAttribute(L"public_interface");
      return;
    }

    const wchar_t* str;
    switch (attr)
    {
    case iface::cellml_api::INTERFACE_IN:
      str = L"in";
      break;
    case iface::cellml_api::INTERFACE_OUT:
      str = L"out";
      break;
    default:
      throw iface::cellml_api::CellMLException();
    }

    datastore->setAttribute(L"public_interface", str);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLVariableSet*
CDA_CellMLVariable::connectedVariables()
  throw(std::exception&)
{
  return new CDA_ConnectedCellMLVariableSet(this);
}

iface::cellml_api::CellMLVariable*
CDA_CellMLVariable::sourceVariable()
  throw(std::exception&)
{
  // Find all connected variables...
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                     connectedVariables());
  RETURN_INTO_OBJREF(cvi, iface::cellml_api::CellMLVariableIterator,
                     cvs->iterateVariables());
  while (true)
  {
    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable, cvi->nextVariable());
    if (v == NULL)
      return NULL;
    if ((v->publicInterface() != iface::cellml_api::INTERFACE_IN) &&
        (v->privateInterface() != iface::cellml_api::INTERFACE_IN))
    {
      v->add_ref();
      return v.getPointer();
    }
  }
}

wchar_t*
CDA_CellMLVariable::componentName()
  throw(std::exception&)
{
  try
  {
    ObjRef<iface::cellml_api::CellMLElement>
      pe(already_AddRefd<iface::cellml_api::CellMLElement>
         (
          static_cast<iface::cellml_api::CellMLElement*>(parentElement())
         )
        );

    DECLARE_QUERY_INTERFACE_OBJREF(cc, pe, cellml_api::CellMLComponent);
    return cc->name();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

wchar_t*
CDA_ComponentRef::componentName()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    return datastore->getAttributeNS(NULL_NS, L"component");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_ComponentRef::componentName(const wchar_t* attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"component", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::ComponentRefSet*
CDA_ComponentRef::componentRefs()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_ComponentRefSet(allChildren.getPointer());
}

iface::cellml_api::ComponentRef*
CDA_ComponentRef::parentComponentRef()
  throw(std::exception&)
{
  iface::cellml_api::ComponentRef* pcr =
    dynamic_cast<iface::cellml_api::ComponentRef*>(mParent);
  if (pcr != NULL)
    pcr->add_ref();
  return pcr;
}

iface::cellml_api::Group*
CDA_ComponentRef::parentGroup()
  throw(std::exception&)
{
  CDA_ComponentRef* topref = NULL;
  CDA_ComponentRef* nextRef = this;
  do
  {
    topref = nextRef;
    nextRef = dynamic_cast<CDA_ComponentRef*>(topref->mParent);
  }
  while (nextRef);

  iface::cellml_api::Group* g =
    dynamic_cast<iface::cellml_api::Group*>(topref->mParent);

  if (g != NULL)
    g->add_ref();
  return g;
}

wchar_t*
CDA_RelationshipRef::name()
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    return datastore->getAttributeNS(NULL_NS, L"name");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_RelationshipRef::name(const wchar_t* attr)
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    datastore->setAttributeNS(NULL_NS, L"name", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

wchar_t*
CDA_RelationshipRef::relationship()
  throw(std::exception&)
{
  try
  {
    // We are looking for relationships in any namespace, so we need to
    // go through all nodes...
    RETURN_INTO_OBJREF(cn, iface::dom::NamedNodeMap, datastore->attributes());
    u_int32_t l = cn->length();
    u_int32_t i;
    for (i = 0; i < l; i++)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, cn->item(i));
      if (n == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(at, n, dom::Attr);
      if (at == NULL)
        continue;

      // We have now found the attribute.
      RETURN_INTO_WSTRING(ln, at->localName());
      if (ln == L"")
      {
        wchar_t* str = at->nodeName();
        ln = str;
        free(str);
      }
      if (ln != L"relationship")
        continue;

      return at->value();
    }
    throw iface::cellml_api::CellMLException();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

wchar_t*
CDA_RelationshipRef::relationshipNamespace()
  throw(std::exception&)
{
  try
  {
    // We are looking for relationships in any namespace, so we need to
    // go through all nodes...
    RETURN_INTO_OBJREF(cn, iface::dom::NodeList, datastore->childNodes());
    u_int32_t l = cn->length();
    u_int32_t i;
    for (i = 0; i < l; i++)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, cn->item(i));
      if (n == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(at, n, dom::Attr);
      if (at == NULL)
        continue;

      // We have now found the attribute.
      RETURN_INTO_WSTRING(ln, at->localName());
      if (ln != L"relationship")
        continue;

      return at->namespaceURI();
    }
    throw iface::cellml_api::CellMLException();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_RelationshipRef::setRelationshipName(const wchar_t* namespaceURI,
                                         const wchar_t* name)
  throw(std::exception&)
{
  try
  {
    // We are looking for relationships in any namespace, so we need to
    // go through all nodes...
    RETURN_INTO_OBJREF(cn, iface::dom::NodeList, datastore->childNodes());
    u_int32_t l = cn->length();
    u_int32_t i;
    for (i = 0; i < l;)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, cn->item(i));
      if (n == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(at, n, dom::Attr);
      if (at == NULL)
      {
        i++;
        continue;
      }

      // We have now found the attribute.
      RETURN_INTO_WSTRING(ln, at->localName());
      if (ln != L"relationship")
      {
        i++;
        continue;
      }

      // Remove the attribute...
      datastore->removeChild(at);
    }

    datastore->setAttributeNS(NULL_NS, L"relationship", name);
    return;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::RelationshipRefSet*
CDA_Group::relationshipRefs()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_RelationshipRefSet(allChildren);
}

iface::cellml_api::ComponentRefSet*
CDA_Group::componentRefs()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_ComponentRefSet(allChildren);
}

bool
CDA_Group::isEncapsulation()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(rrl, iface::cellml_api::RelationshipRefSet,
                     relationshipRefs());
  RETURN_INTO_OBJREF(rri, iface::cellml_api::RelationshipRefIterator,
                     rrl->iterateRelationshipRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(rr, iface::cellml_api::RelationshipRef,
                       rri->nextRelationshipRef());
    if (rr == NULL)
      return false;
    wchar_t* s = rr->relationship();
    bool match = !wcscmp(s, L"encapsulation");
    free(s);
    if (match)
      return true;
  }
}

bool
CDA_Group::isContainment()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(rrl, iface::cellml_api::RelationshipRefSet,
                     relationshipRefs());
  RETURN_INTO_OBJREF(rri, iface::cellml_api::RelationshipRefIterator,
                     rrl->iterateRelationshipRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(rr, iface::cellml_api::RelationshipRef,
                       rri->nextRelationshipRef());
    if (rr == NULL)
      return false;
    wchar_t* s = rr->relationship();
    bool match = !wcscmp(s, L"containment");
    free(s);
    if (match)
      return true;
  }
}

iface::cellml_api::MapComponents*
CDA_Connection::componentMapping()
  throw(std::exception&)
{
  RETURN_INTO_OBJREFD(allChildren, iface::cellml_api::CellMLElementSet,
                      childElements());
  RETURN_INTO_OBJREF(allChildrenIt, iface::cellml_api::CellMLElementIterator,
                     allChildren->iterate());
  while (true)
  {
    RETURN_INTO_OBJREFD(child, iface::cellml_api::CellMLElement,
                       allChildrenIt->next());
    if (child == NULL)
    {
      // No MapComponents. Make one...
      RETURN_INTO_OBJREFD(me, CDA_Model, modelElement());
      if (me == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                         me->createMapComponents());
      addElement(mc);
      iface::cellml_api::MapComponents* rmc = mc;
      rmc->add_ref();
      return rmc;
    }
    iface::cellml_api::MapComponents* mc =
      dynamic_cast<iface::cellml_api::MapComponents*>(child.getPointer());
    if (mc != NULL)
    {
      mc->add_ref();
      return mc;
    }
  }
}

iface::cellml_api::MapVariablesSet*
CDA_Connection::variableMappings()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_MapVariablesSet(allChildren);
}

wchar_t*
CDA_MapComponents::firstComponentName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"component_1");
}

void
CDA_MapComponents::firstComponentName(const wchar_t* attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"component_1", attr);
}

wchar_t*
CDA_MapComponents::secondComponentName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"component_2");
}

void 
CDA_MapComponents::secondComponentName(const wchar_t* attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"component_2", attr);
}

iface::cellml_api::CellMLComponent*
CDA_MapComponents::firstComponent()
  throw(std::exception&)
{
  // Get the connection...
  CDA_Connection* c =
    dynamic_cast<CDA_Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  RETURN_INTO_WSTRING(cn, firstComponentName());
  if (cn == L"")
    return NULL;

  while (true)
  {
    RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet, 
                       m->modelComponents());
    RETURN_INTO_OBJREF(mci, iface::cellml_api::CellMLComponentIterator,
                       mc->iterateComponents());
    RETURN_INTO_OBJREF(ce, iface::cellml_api::NamedCellMLElement,
                       mc->get(cn.c_str()));
    if (ce == NULL)
      throw iface::cellml_api::CellMLException();
    
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>(ce.getPointer());
    if (ic == NULL)
    {
      iface::cellml_api::CellMLComponent* cc =
        dynamic_cast<iface::cellml_api::CellMLComponent*>(ce.getPointer());
      cc->add_ref();
      return cc;
    }

    // Its an import component. See if there is an import...
    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL || ci->importedModel == NULL)
    {
      iface::cellml_api::CellMLComponent* cc =
        dynamic_cast<iface::cellml_api::CellMLComponent*>(ce.getPointer());
      cc->add_ref();
      return cc;
    }

    m = dynamic_cast<CDA_Model*>(ci->importedModel);
    wchar_t* icrn = ic->componentRef();
    cn = icrn;
    free(icrn);
  }
}

void
CDA_MapComponents::firstComponent(iface::cellml_api::CellMLComponent* attr)
  throw(std::exception&)
{
  // CellMLComponent might not be in the correct model. We will not
  // automatically import to make this work, but we can go up the parent chain
  // to check if this component is already here. If attr is an ImportComponent
  // we will also walk down the chain to find the right model...
  ObjRef<iface::cellml_api::CellMLComponent> comp(attr);

  CDA_Connection* connection = dynamic_cast<CDA_Connection*>(mParent);

  if (connection == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_Model* thisModel = dynamic_cast<CDA_Model*>(connection->mParent);
  if (thisModel == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>(attr);
  CDA_Model* m = NULL, *mOrig = NULL;
  if (ic != NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException();

    m = dynamic_cast<CDA_Model*>(import->mParent);
    mOrig = m;
    while (m != thisModel)
    {
      m = dynamic_cast<CDA_Model*>(import->importedModel);
      if (m == NULL)
        // Don't return yet, we might find the model up the import chain.
        break;

      RETURN_INTO_WSTRING(compName, ic->componentRef());
      RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                         m->modelComponents());
      comp = already_AddRefd<iface::cellml_api::CellMLComponent>
        (dynamic_cast<iface::cellml_api::CellMLComponent*>
         (mc->get(compName.c_str())));
      if (comp == NULL)
        throw iface::cellml_api::CellMLException();
      // If we have reached the real component, then break...
      ic = dynamic_cast<CDA_ImportComponent*>(comp.getPointer());
      if (ic == NULL)
        break;
      import = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    }
  }
  if (m != thisModel)
  {
    if (mOrig == NULL)
      m = dynamic_cast<CDA_Model*>(dynamic_cast<CDA_CellMLComponent*>
                                   (comp.getPointer())->mParent);
    else
      m = mOrig;

    RETURN_INTO_WSTRING(compName, attr->name());

    while (m != thisModel)
    {
      // We haven't found the appropriate component yet. Head up the import chain
      // towards the root model...
      CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(m->mParent);
      if (ci == NULL)
        throw iface::cellml_api::CellMLException();

      // Look in the CellMLImport for a component referencing the name...
      RETURN_INTO_OBJREF(compset, iface::cellml_api::ImportComponentSet,
                         ci->components());
      RETURN_INTO_OBJREF(compi, iface::cellml_api::ImportComponentIterator,
                         compset->iterateImportComponents());
      while (true)
      {
        RETURN_INTO_OBJREF(impc, iface::cellml_api::ImportComponent,
                           compi->nextImportComponent());
        if (impc == NULL)
          throw iface::cellml_api::CellMLException();
        RETURN_INTO_WSTRING(compR, impc->componentRef());
        if (compR == compName)
        {
          comp = dynamic_cast<iface::cellml_api::CellMLComponent*>
            (impc.getPointer());
          compName = impc->name();
          m = dynamic_cast<CDA_Model*>(ci->mParent);
          if (m == NULL)
            throw iface::cellml_api::CellMLException();
          break;
        }
      }
    }
  }

  // We just need to set the component_1 attr...
  RETURN_INTO_WSTRING(cn, comp->name());
  datastore->setAttributeNS(NULL_NS, L"component_1", cn.c_str());
}

iface::cellml_api::CellMLComponent*
CDA_MapComponents::secondComponent()
  throw(std::exception&)
{
  // Get the connection...
  CDA_Connection* c =
    dynamic_cast<CDA_Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  RETURN_INTO_WSTRING(cn, secondComponentName());
  if (cn == L"")
    return NULL;

  while (true)
  {
    RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet, 
                       m->modelComponents());
    RETURN_INTO_OBJREF(mci, iface::cellml_api::CellMLComponentIterator,
                       mc->iterateComponents());
    RETURN_INTO_OBJREF(ce, iface::cellml_api::NamedCellMLElement,
                       mc->get(cn.c_str()));
    if (ce == NULL)
      throw iface::cellml_api::CellMLException();
    
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>(ce.getPointer());
    if (ic == NULL)
    {
      iface::cellml_api::CellMLComponent* cc =
        dynamic_cast<iface::cellml_api::CellMLComponent*>(ce.getPointer());
      cc->add_ref();
      return cc;
    }

    // Its an import component. See if there is an import...
    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL || ci->importedModel == NULL)
    {
      iface::cellml_api::CellMLComponent* cc =
        dynamic_cast<iface::cellml_api::CellMLComponent*>(ce.getPointer());
      cc->add_ref();
      return cc;
    }

    m = dynamic_cast<CDA_Model*>(ci->importedModel);
    wchar_t* icrn = ic->componentRef();
    cn = icrn;
    free(icrn);
  }
}

void
CDA_MapComponents::secondComponent(iface::cellml_api::CellMLComponent* attr)
  throw(std::exception&)
{
  // CellMLComponent might not be in the correct model. We will not
  // automatically import to make this work, but we can go up the parent chain
  // to check if this component is already here. If attr is an ImportComponent
  // we will also walk down the chain to find the right model...
  ObjRef<iface::cellml_api::CellMLComponent> comp(attr);

  CDA_Connection* connection = dynamic_cast<CDA_Connection*>(mParent);

  if (connection == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_Model* thisModel = dynamic_cast<CDA_Model*>(connection->mParent);
  if (thisModel == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>(attr);
  CDA_Model* m = NULL, *mOrig = NULL;
  if (ic != NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException();

    m = dynamic_cast<CDA_Model*>(import->mParent);
    mOrig = m;
    while (m != thisModel)
    {
      m = dynamic_cast<CDA_Model*>(import->importedModel);
      if (m == NULL)
        // Don't return yet, we might find the model up the import chain.
        break;

      RETURN_INTO_WSTRING(compName, ic->componentRef());
      RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                         m->modelComponents());
      comp = already_AddRefd<iface::cellml_api::CellMLComponent>
        (dynamic_cast<iface::cellml_api::CellMLComponent*>
         (mc->get(compName.c_str())));
      if (comp == NULL)
        throw iface::cellml_api::CellMLException();
      // If we have reached the real component, then break...
      ic = dynamic_cast<CDA_ImportComponent*>(comp.getPointer());
      if (ic == NULL)
        break;
      import = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    }
  }
  if (m != thisModel)
  {
    if (mOrig == NULL)
      m = dynamic_cast<CDA_Model*>(dynamic_cast<CDA_CellMLComponent*>
                                   (comp.getPointer())->mParent);
    else
      m = mOrig;

    RETURN_INTO_WSTRING(compName, attr->name());

    while (m != thisModel)
    {
      // We haven't found the appropriate component yet. Head up the import chain
      // towards the root model...
      CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(m->mParent);
      if (ci == NULL)
        throw iface::cellml_api::CellMLException();

      // Look in the CellMLImport for a component referencing the name...
      RETURN_INTO_OBJREF(compset, iface::cellml_api::ImportComponentSet,
                         ci->components());
      RETURN_INTO_OBJREF(compi, iface::cellml_api::ImportComponentIterator,
                         compset->iterateImportComponents());
      while (true)
      {
        RETURN_INTO_OBJREF(impc, iface::cellml_api::ImportComponent,
                           compi->nextImportComponent());
        if (impc == NULL)
          throw iface::cellml_api::CellMLException();
        RETURN_INTO_WSTRING(compR, impc->componentRef());
        if (compR == compName)
        {
          comp = dynamic_cast<iface::cellml_api::CellMLComponent*>
            (impc.getPointer());
          compName = impc->name();
          m = dynamic_cast<CDA_Model*>(ci->mParent);
          if (m == NULL)
            throw iface::cellml_api::CellMLException();
          break;
        }
      }
    }
  }

  // We just need to set the component_1 attr...
  RETURN_INTO_WSTRING(cn, comp->name());
  datastore->setAttributeNS(NULL_NS, L"component_2", cn.c_str());
}

wchar_t*
CDA_MapVariables::firstVariableName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"variable_1");
}

void
CDA_MapVariables::firstVariableName(const wchar_t* attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"variable_1", attr);
}

wchar_t*
CDA_MapVariables::secondVariableName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"variable_2");
}

void
CDA_MapVariables::secondVariableName(const wchar_t* attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"variable_2", attr);
}

iface::cellml_api::CellMLVariable*
CDA_MapVariables::firstVariable()
  throw(std::exception&)
{
  // See if we have a connection...
  iface::cellml_api::Connection* c =
    dynamic_cast<iface::cellml_api::Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  // Next find the component...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     c->componentMapping());
  RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                     mc->firstComponent());

  // and look in the component for the variable...
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                     comp->variables());
  RETURN_INTO_WSTRING(fvn, firstVariableName())
  iface::cellml_api::CellMLVariable* v =
    dynamic_cast<iface::cellml_api::CellMLVariable*>(cvs->get(fvn.c_str()));

  // The CellML is malformed if the referenced variable doesn't exist.
  if (v == NULL)
    throw iface::cellml_api::CellMLException();

  // already has been addrefd.
  return v;
}

void
CDA_MapVariables::firstVariable(iface::cellml_api::CellMLVariable* attr)
  throw(std::exception&)
{
  // See if we have a connection...
  CDA_Connection* c =
    dynamic_cast<CDA_Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  // Next, find our parent...
  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  // To connect variables, the component needs to be available under some name
  // in the model. See if we can find the appropriate name...
  CDA_CellMLVariable* v = dynamic_cast<CDA_CellMLVariable*>(attr);
  if (v == NULL)
    throw iface::cellml_api::CellMLException();

  ObjRef<iface::cellml_api::CellMLComponent> comp =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(v->mParent);
  if (comp == NULL)
    throw iface::cellml_api::CellMLException();

  // This component is guaranteed not to be an import component, or it wouldn't
  // have variables. But the component we are looking for could be this one, or
  // it could be up the import chain.
  CDA_Model* compModel = dynamic_cast<CDA_Model*>
    (dynamic_cast<CDA_CellMLComponent*>(comp.getPointer())->mParent);
  if (compModel == NULL)
    throw iface::cellml_api::CellMLException();

  std::wstring compName;
  wchar_t* cn = comp->name();
  compName = cn;
  free(cn);

  while (compModel != m)
  {

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(compModel->mParent);

    // If we reached the root, the variable cannot be added to this connection.
    if (ci == NULL)
      throw iface::cellml_api::CellMLException();

    // See if this variable is imported here...
    RETURN_INTO_OBJREF(ics, iface::cellml_api::ImportComponentSet,
                       ci->components());
    RETURN_INTO_OBJREF(ici, iface::cellml_api::ImportComponentIterator,
                       ics->iterateImportComponents());
    while (true)
    {
      RETURN_INTO_OBJREF(ic, iface::cellml_api::ImportComponent,
                         ici->nextImportComponent());
      if (ic == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_WSTRING(compRef, ic->componentRef());
      if (compRef == compName)
      {
        comp = dynamic_cast<iface::cellml_api::CellMLComponent*>
          (ic.getPointer());
        compModel = dynamic_cast<CDA_Model*>(ci->mParent);
        wchar_t* cn = comp->name();
        compName = cn;
        free(cn);
        break;
      }
    }
  }

  // Next find the component mapping...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     c->componentMapping());

  // If we already have set the component, it had better match...
  RETURN_INTO_WSTRING(existingCN, mc->firstComponentName());
  if (existingCN == L"")
  {
    mc->firstComponentName(compName.c_str());
  }
  else if (existingCN != compName)
    throw iface::cellml_api::CellMLException();

  // Everything is now ready to set the variable...
  RETURN_INTO_WSTRING(vname, attr->name());
  firstVariableName(vname.c_str());
}

iface::cellml_api::CellMLVariable*
CDA_MapVariables::secondVariable()
  throw(std::exception&)
{
  // See if we have a connection...
  iface::cellml_api::Connection* c =
    dynamic_cast<iface::cellml_api::Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  // Next find the component...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     c->componentMapping());
  RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                     mc->secondComponent());

  // and look in the component for the variable...
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                     comp->variables());
  RETURN_INTO_WSTRING(fvn, secondVariableName())
  iface::cellml_api::CellMLVariable* v =
    dynamic_cast<iface::cellml_api::CellMLVariable*>(cvs->get(fvn.c_str()));

  // The CellML is malformed if the referenced variable doesn't exist.
  if (v == NULL)
    throw iface::cellml_api::CellMLException();

  // already has been addrefd.
  return v;
}

void
CDA_MapVariables::secondVariable(iface::cellml_api::CellMLVariable* attr)
  throw(std::exception&)
{
  // See if we have a connection...
  CDA_Connection* c =
    dynamic_cast<CDA_Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  // Next, find our parent...
  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  // To connect variables, the component needs to be available under some name
  // in the model. See if we can find the appropriate name...
  CDA_CellMLVariable* v = dynamic_cast<CDA_CellMLVariable*>(attr);
  if (v == NULL)
    throw iface::cellml_api::CellMLException();

  ObjRef<iface::cellml_api::CellMLComponent> comp =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(v->mParent);
  if (comp == NULL)
    throw iface::cellml_api::CellMLException();

  // This component is guaranteed not to be an import component, or it wouldn't
  // have variables. But the component we are looking for could be this one, or
  // it could be up the import chain.
  CDA_Model* compModel = dynamic_cast<CDA_Model*>
    (dynamic_cast<CDA_CellMLComponent*>(comp.getPointer())->mParent);
  if (compModel == NULL)
    throw iface::cellml_api::CellMLException();

  std::wstring compName;
  wchar_t* cn = comp->name();
  compName = cn;
  free(cn);

  while (compModel != m)
  {

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(compModel->mParent);

    // If we reached the root, the variable cannot be added to this connection.
    if (ci == NULL)
      throw iface::cellml_api::CellMLException();

    // See if this variable is imported here...
    RETURN_INTO_OBJREF(ics, iface::cellml_api::ImportComponentSet,
                       ci->components());
    RETURN_INTO_OBJREF(ici, iface::cellml_api::ImportComponentIterator,
                       ics->iterateImportComponents());
    while (true)
    {
      RETURN_INTO_OBJREF(ic, iface::cellml_api::ImportComponent,
                         ici->nextImportComponent());
      if (ic == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_WSTRING(compRef, ic->componentRef());
      if (compRef == compName)
      {
        comp = dynamic_cast<iface::cellml_api::CellMLComponent*>
          (ic.getPointer());
        compModel = dynamic_cast<CDA_Model*>(ci->mParent);
        wchar_t* cn = comp->name();
        compName = cn;
        free(cn);
        break;
      }
    }
  }

  // Next find the component mapping...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     c->componentMapping());

  // If we already have set the component, it had better match...
  RETURN_INTO_WSTRING(existingCN, mc->secondComponentName());
  if (existingCN == L"")
  {
    mc->secondComponentName(compName.c_str());
  }
  else if (existingCN != compName)
    throw iface::cellml_api::CellMLException();

  // Everything is now ready to set the variable...
  RETURN_INTO_WSTRING(vname, attr->name());
  secondVariableName(vname.c_str());
}

iface::cellml_api::ReactantVariableRefSet*
CDA_Reaction::reactants()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_ReactantVariableRefSet(allChildren);
}

iface::cellml_api::ProductVariableRefSet*
CDA_Reaction::products()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (
     already_AddRefd<CDA_CellMLElementSet>
     (
      dynamic_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_ProductVariableRefSet(allChildren);
}

iface::cellml_api::RateVariableRef*
CDA_Reaction::rate()
  throw(std::exception&)
{
  RETURN_INTO_OBJREFD(allChildren, iface::cellml_api::CellMLElementSet,
                      childElements());
  RETURN_INTO_OBJREF(allChildrenIt, iface::cellml_api::CellMLElementIterator,
                     allChildren->iterate());
  while (true)
  {
    RETURN_INTO_OBJREFD(child, iface::cellml_api::CellMLElement,
                       allChildrenIt->next());
    if (child == NULL)
    {
      // No RateVariableRef. Make one...
      RETURN_INTO_OBJREFD(me, CDA_Model, modelElement());
      if (me == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_OBJREF(rvr, iface::cellml_api::RateVariableRef,
                         me->createRateVariableRef());
      addElement(rvr);
      iface::cellml_api::RateVariableRef* rrvr = rvr;
      rrvr->add_ref();
      return rrvr;
    }
    iface::cellml_api::RateVariableRef* rvr =
      dynamic_cast<iface::cellml_api::RateVariableRef*>(child.getPointer());
    if (rvr != NULL)
    {
      rvr->add_ref();
      return rvr;
    }
  }
}

iface::cellml_api::CellMLVariable*
CDA_VariableRef::variable()
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(vn, datastore->getAttributeNS(NULL_NS, L"variable"));

  // Find the component...
  iface::cellml_api::CellMLComponent* c =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException();

  // Find the variable...
  RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet, c->variables());
  RETURN_INTO_OBJREF(v, iface::cellml_api::NamedCellMLElement,
                     vs->get(vn.c_str()));
  if (v == NULL)
    throw iface::cellml_api::CellMLException();

  iface::cellml_api::CellMLVariable* rv =
    dynamic_cast<iface::cellml_api::CellMLVariable*>(v.getPointer());
  rv->add_ref();

  return rv;
}

iface::cellml_api::ReactantRole*
CDA_ReactantVariableRef::role() throw(std::exception&)
{
  // XXX there can be more than one role! API needs fixing.
  RETURN_INTO_OBJREFD(allChildren, iface::cellml_api::CellMLElementSet,
                      childElements());
  RETURN_INTO_OBJREF(allChildrenIt, iface::cellml_api::CellMLElementIterator,
                     allChildren->iterate());
  while (true)
  {
    RETURN_INTO_OBJREFD(child, iface::cellml_api::CellMLElement,
                       allChildrenIt->next());
    if (child == NULL)
    {
      RETURN_INTO_OBJREFD(me, CDA_Model, modelElement());
      if (me == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_OBJREF(rvr, iface::cellml_api::ReactantRole,
                         me->createReactantRole());
      addElement(rvr);
      iface::cellml_api::ReactantRole* rrvr = rvr;
      rrvr->add_ref();
      return rrvr;
    }
    iface::cellml_api::ReactantRole* rvr =
      dynamic_cast<iface::cellml_api::ReactantRole*>(child.getPointer());
    if (rvr != NULL)
    {
      rvr->add_ref();
      return rvr;
    }
  }
}

void
CDA_ReactantVariableRef::role(iface::cellml_api::ReactantRole* attr)
  throw(std::exception&)
{
  addElement(attr);
}

wchar_t*
CDA_ReactantVariableRef::refType()
  throw(std::exception&)
{
  return wcsdup(L"reactant");
}

iface::cellml_api::ProductRole*
CDA_ProductVariableRef::role()
  throw(std::exception&)
{
  // XXX there can be more than one role! API needs fixing.
  RETURN_INTO_OBJREFD(allChildren, iface::cellml_api::CellMLElementSet,
                      childElements());
  RETURN_INTO_OBJREF(allChildrenIt, iface::cellml_api::CellMLElementIterator,
                     allChildren->iterate());
  while (true)
  {
    RETURN_INTO_OBJREFD(child, iface::cellml_api::CellMLElement,
                       allChildrenIt->next());
    if (child == NULL)
    {
      RETURN_INTO_OBJREFD(me, CDA_Model, modelElement());
      if (me == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_OBJREF(rvr, iface::cellml_api::ProductRole,
                         me->createProductRole());
      addElement(rvr);
      iface::cellml_api::ProductRole* rrvr = rvr;
      rrvr->add_ref();
      return rrvr;
    }
    iface::cellml_api::ProductRole* rvr =
      dynamic_cast<iface::cellml_api::ProductRole*>(child.getPointer());
    if (rvr != NULL)
    {
      rvr->add_ref();
      return rvr;
    }
  }
}

void
CDA_ProductVariableRef::role(iface::cellml_api::ProductRole* attr)
  throw(std::exception&)
{
  addElement(attr);
}

wchar_t*
CDA_ProductVariableRef::refType()
  throw(std::exception&)
{
  return wcsdup(L"product");
}

iface::cellml_api::RateRole*
CDA_RateVariableRef::role() throw(std::exception&)
{
  // XXX there can be more than one role! API needs fixing.
  RETURN_INTO_OBJREFD(allChildren, iface::cellml_api::CellMLElementSet,
                      childElements());
  RETURN_INTO_OBJREF(allChildrenIt, iface::cellml_api::CellMLElementIterator,
                     allChildren->iterate());
  while (true)
  {
    RETURN_INTO_OBJREFD(child, iface::cellml_api::CellMLElement,
                       allChildrenIt->next());
    if (child == NULL)
    {
      RETURN_INTO_OBJREFD(me, CDA_Model, modelElement());
      if (me == NULL)
        throw iface::cellml_api::CellMLException();
      RETURN_INTO_OBJREF(rvr, iface::cellml_api::RateRole,
                         me->createRateRole());
      addElement(rvr);
      iface::cellml_api::RateRole* rrvr = rvr;
      rrvr->add_ref();
      return rrvr;
    }
    iface::cellml_api::RateRole* rvr =
      dynamic_cast<iface::cellml_api::RateRole*>(child.getPointer());
    if (rvr != NULL)
    {
      rvr->add_ref();
      return rvr;
    }
  }
}

void
CDA_RateVariableRef::role(iface::cellml_api::RateRole* attr)
  throw(std::exception&)
{
  addElement(attr);
}

wchar_t*
CDA_RateVariableRef::refType()
  throw(std::exception&)
{
  return wcsdup(L"rate");
}

wchar_t*
CDA_ReactantRole::roleType()
  throw(std::exception&)
{
  return wcsdup(L"reactant");
}

wchar_t*
CDA_ProductRole::roleType()
  throw(std::exception&)
{
  return wcsdup(L"product");
}

wchar_t*
CDA_RateRole::roleType()
  throw(std::exception&)
{
  return wcsdup(L"rate");
}

u_int32_t
CDA_CellMLElementSetUseIteratorMixin::length()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(cei, iface::cellml_api::CellMLElementIterator, iterate());
  
  u_int32_t length = 0;
  while (true)
  {
    RETURN_INTO_OBJREF(ce, iface::cellml_api::CellMLElement, cei->next());
    if (ce == NULL)
    {
      return length;
    }
    length++;
  }
}

bool
CDA_CellMLElementSetUseIteratorMixin::contains(iface::cellml_api::CellMLElement* x)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(cei, iface::cellml_api::CellMLElementIterator, iterate());
  
  while (true)
  {
    RETURN_INTO_OBJREF(ce, iface::cellml_api::CellMLElement, cei->next());
    if (ce == x)
      return true;
    else if (ce == NULL)
      return false;
  }
}

CDA_DOMElementIteratorBase::CDA_DOMElementIteratorBase
(
 iface::dom::Element* parentElement
)
  : mPrevElement(NULL), mNextElement(NULL), mParentElement(parentElement)
{
  mParentElement->add_ref();
  mNodeList = mParentElement->childNodes();
}

CDA_DOMElementIteratorBase::~CDA_DOMElementIteratorBase()
{
  mNodeList->release_ref();
  mParentElement->release_ref();
  if (mPrevElement != NULL)
  {
    mPrevElement->release_ref();
  }
}

void
CDA_DOMElementIteratorBase::registerListener()
{
  mParentElement->addEventListener(L"DOMNodeInserted", this, false);
  // Don't let the listener queue own us, we will deregister automatically.
  release_ref();
}

void
CDA_DOMElementIteratorBase::deregisterListener()
{
  mParentElement->removeEventListener(L"DOMNodeInserted", this, false);
  if (mNextElement != NULL)
  {
    mNextElement->removeEventListener(L"DOMNodeRemoved", this, false);
    mNextElement->release_ref();
    mNextElement = NULL;
  }
}

iface::dom::Element*
CDA_DOMElementIteratorBase::fetchNextElement()
{
  try
  {
    if (mPrevElement == NULL)
    {
      // Search for the first element...
      u_int32_t i;
      u_int32_t l = mNodeList->length();
      for (i = 0; i < l; i++)
      {
        ObjRef<iface::dom::Node> nodeHit
          (already_AddRefd<iface::dom::Node>(mNodeList->item(i)));
        QUERY_INTERFACE(mPrevElement, nodeHit, dom::Element);
        if (mPrevElement != NULL)
          break;
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
      mPrevElement->release_ref();
      mPrevElement = mNextElement;
      mNextElement->removeEventListener(L"DOMNodeRemoved", this, false);
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
        mNextElement->addEventListener(L"DOMNodeRemoved", this, false);
        break;
      }
      nodeHit = already_AddRefd<iface::dom::Node>(nodeHit->nextSibling());
    }
    
    return mPrevElement;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_DOMElementIteratorBase::handleEvent(iface::events::Event* evt)
  throw(std::exception&)
{
  try
  {
    bool isRemoval = false, isInsertion = false;
    wchar_t* et = evt->type();
    if (!wcscmp(et, L"DOMNodeRemoved"))
      isRemoval = true;
    else if (!wcscmp(et, L"DOMNodeInserted"))
      isInsertion = true;
    free(et);
    
    // We only care about insertions and removals...
    if (!isRemoval && !isInsertion)
      return;
    
    if (isRemoval)
    {
      if (evt->eventPhase() != iface::events::Event::AT_TARGET)
        return;

      // The next node is about to be removed. Advance to a later next...
      ObjRef<iface::dom::Node> nodeHit = mPrevElement->nextSibling();
      while (nodeHit != NULL)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(mNextElement, nodeHit, dom::Element);
        if (mNextElement != NULL)
        {
          mNextElement->addEventListener(L"DOMNodeRemoved", this, false);
          mNextElement->add_ref();
          break;
        }
        nodeHit = nodeHit->nextSibling();
      }
    }
    else if (isInsertion)
    {
      // Convert to a mutation event...
      DECLARE_QUERY_INTERFACE_OBJREF(mevt, evt, events::MutationEvent);

      ObjRef<iface::dom::Node> rn =
        already_AddRefd<iface::dom::Node>(mevt->relatedNode());
      if (!isEqualAfterLeftQI(rn, mParentElement, "dom::Element"))
        return;
      
      ObjRef<iface::dom::Node> tn =
        already_AddRefd<iface::dom::Node>(mevt->target());
      DECLARE_QUERY_INTERFACE_OBJREF(te, tn, dom::Element);
      
      if (te == NULL)
        return;

      // See if target lies between mPrevElement and mNextElement...
      ObjRef<iface::dom::Node> curN
        (already_AddRefd<iface::dom::Node>(mPrevElement->nextSibling()));
      DECLARE_QUERY_INTERFACE_OBJREF(curE, curN, dom::Element)
      while ((curE != mNextElement ||
              (mNextElement == NULL && curN != NULL)) &&
             curE != te)
      {
        curN = already_AddRefd<iface::dom::Node>(curN->nextSibling());
        QUERY_INTERFACE(curE, curN, dom::Element);
      }
      // If the target is not in the relevant range, just return...
      if (curE == mNextElement)
        return;

      // The current next element is no longer next...
      if (mNextElement)
      {
        mNextElement->removeEventListener(L"DOMNodeRemoved", this, false);
        mNextElement->release_ref();
      }
      mNextElement = curE;
      mNextElement->add_ref();
      mNextElement->addEventListener(L"DOMNodeRemoved", this, false);
    }
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLElementIterator*
CDA_AllUnitsSet::iterate()
  throw(std::exception&)
{
  return new CDA_AllUnitsIterator(mLocalIterator, mImportIterator,
                                  mRecurseIntoImports);
}

iface::cellml_api::CellMLElement*
CDA_AllUnitsIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    iface::cellml_api::CellMLElement* n = mLocalIterator->next();
    if (n != NULL)
      return n;
      
    // We have run out of elements, so we have to find the next import.
    iface::cellml_api::CellMLElement* imp = mImportIterator->next();

    // If there are no more imports left, we are done...
    if (imp == NULL)
      return NULL;

    ObjRef<CDA_CellMLImport> impintern
      (already_AddRefd<CDA_CellMLImport>(dynamic_cast<CDA_CellMLImport*>(imp)));
    // If the import hasn't been instantiated, iterate its ImportUnits
    if (!mRecurseIntoImports || impintern->importedModel == NULL)
    {
      RETURN_INTO_OBJREF(cu, iface::cellml_api::CellMLElementSet,
                         impintern->units());

      mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
        (cu->iterate());
      continue;
    }

    // We have found an instantiated import, so we need to iterate its
    // childUnits. The use of childUnits instead of localUnits is important,
    // as that means the iterator will produce a pre-order traversal of the
    // entire tree. We re-use mLocalIterator to store this.
    RETURN_INTO_OBJREF(cu, iface::cellml_api::CellMLElementSet,
                       impintern->importedModel->allUnits());

    mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
      (cu->iterate());
  }
}

iface::cellml_api::CellMLElementIterator*
CDA_AllComponentSet::iterate()
  throw(std::exception&)
{
  return new CDA_AllComponentIterator(mLocalIterator, mImportIterator,
                                      mRecurseIntoImports);
}

iface::cellml_api::CellMLElement*
CDA_AllComponentIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    iface::cellml_api::CellMLElement* n = mLocalIterator->next();
    if (n != NULL)
      return n;

    // We have run out of elements, so we have to find the next import.
    iface::cellml_api::CellMLElement* imp = mImportIterator->next();

    // If there are no more imports left, we are done...
    if (imp == NULL)
      return NULL;

    ObjRef<CDA_CellMLImport> impintern
      (already_AddRefd<CDA_CellMLImport>(dynamic_cast<CDA_CellMLImport*>(imp)));
    // If the import hasn't been instantiated, iterate the ImportComponents.
    if (!mRecurseIntoImports || impintern->importedModel == NULL)
    {
      RETURN_INTO_OBJREF(cu, iface::cellml_api::CellMLElementSet,
                         impintern->components());

      mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
        (cu->iterate());
      continue;
    }
    // We have found an instantiated import, so we need to iterate its
    // childUnits. The use of allComponents instead of localComponents is important,
    // as that means the iterator will produce a pre-order traversal of the
    // entire tree. We re-use mLocalIterator to store this.
    RETURN_INTO_OBJREF(cu, iface::cellml_api::CellMLElementSet,
                       impintern->importedModel->allComponents());

    mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
      (cu->iterate());
  }
}

iface::cellml_api::CellMLElementIterator*
CDA_CellMLComponentFromComponentRefSet::iterate()
  throw(std::exception&)
{
  return new CDA_CellMLComponentFromComponentRefIterator
    (mModel, mCompRefIterator);
}

iface::cellml_api::CellMLElement*
CDA_CellMLComponentFromComponentRefIterator::next()
  throw(std::exception&)
{
  // Find the next component ref...
  RETURN_INTO_OBJREF(cr, iface::cellml_api::ComponentRef,
                     mCompRefIterator->nextComponentRef());
  if (cr == NULL)
    return NULL;

  RETURN_INTO_WSTRING(compName, cr->componentName());

  iface::cellml_api::Model* currentModel = mModel;
  // We now have the component name, and we need to map it to a component...
  while (true)
  {
    RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                       currentModel->modelComponents());
    RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                       mc->getComponent(compName.c_str()));
    if (c == NULL)
      throw iface::cellml_api::CellMLException();

    // We have a component, but it could be an ImportComponent...
    CDA_ImportComponent* ic =
      dynamic_cast<CDA_ImportComponent*>(c.getPointer());
    if (ic == NULL)
    {
      iface::cellml_api::CellMLComponent* rc = c;
      rc->add_ref();
      return rc;
    }
    
    currentModel =
      dynamic_cast<CDA_CellMLImport*>(ic->mParent)->importedModel;

    // Make do with the import component if not instantiated.
    if (currentModel == NULL)
    {
      iface::cellml_api::CellMLComponent* rc = c;
      rc->add_ref();
      return rc;
    }

    wchar_t * crwc = ic->componentRef();
    compName = crwc;
    free(crwc);
  }
}

iface::cellml_api::CellMLElementIterator*
CDA_ConnectedCellMLVariableSet::iterate()
  throw(std::exception&)
{
  return new CDA_ConnectedCellMLVariableIterator(mConnectedToWhat);
}

CDA_ConnectedCellMLVariableIterator::CDA_ConnectedCellMLVariableIterator
(CDA_CellMLVariable* aConnectedToWhat)
  : CDA_CellMLVariableIterator(NULL)
{
  CDA_CellMLComponent* comp =
    dynamic_cast<CDA_CellMLComponent*>(aConnectedToWhat->mParent);
  if (comp == NULL)
    throw iface::cellml_api::CellMLException();

  CDA_Model* m = dynamic_cast<CDA_Model*>(comp->mParent);
  if (m == NULL)
    throw iface::cellml_api::CellMLException();

  RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet, m->connections());
  RETURN_INTO_OBJREF(ci, iface::cellml_api::ConnectionIterator,
                     cs->iterateConnections());
  
  VariableStackFrame* topFrame = new VariableStackFrame();
  topFrame->whichVariable = aConnectedToWhat;
  topFrame->whichComponent = comp;
  topFrame->connectionIterator = ci;
  variableStack.push_front(topFrame);
}

CDA_ConnectedCellMLVariableIterator::~CDA_ConnectedCellMLVariableIterator()
{
  while (variableStack.size() > 0)
  {
    delete variableStack.front();
    variableStack.pop_front();
  }
}

static bool
CompareComponentsImportAware(iface::cellml_api::CellMLComponent* aComp1,
                             iface::cellml_api::CellMLComponent* aComp2)
  throw(std::exception&)
{
  ObjRef<iface::cellml_api::CellMLComponent> comp1(aComp1);
  ObjRef<iface::cellml_api::CellMLComponent> comp2(aComp2);
  while (true)
  {
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
      (comp1.getPointer());
    if (ic == NULL)
      break;

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL)
      throw iface::cellml_api::CellMLException();

    if (ci->importedModel == NULL)
      throw iface::cellml_api::CellMLException();

    RETURN_INTO_WSTRING(compRef, ic->componentRef());

    RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                       ci->importedModel->modelComponents());
    comp1 = already_AddRefd<iface::cellml_api::CellMLComponent>
      (cs->getComponent(compRef.c_str()));

    if (comp1 == NULL)
      throw iface::cellml_api::CellMLException();
  }
  while (true)
  {
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
      (comp2.getPointer());
    if (ic == NULL)
      break;

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL)
      throw iface::cellml_api::CellMLException();

    if (ci->importedModel == NULL)
      throw iface::cellml_api::CellMLException();

    RETURN_INTO_WSTRING(compRef, ic->componentRef());

    RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                       ci->importedModel->modelComponents());
    comp2 = already_AddRefd<iface::cellml_api::CellMLComponent>
      (cs->getComponent(compRef.c_str()));

    if (comp2 == NULL)
      throw iface::cellml_api::CellMLException();
  }

  // We now have the two real components. Dynamic cast to the implementation
  // class and see if they are identical.
  return (dynamic_cast<CDA_CellMLComponent*>(comp1.getPointer()) ==
          dynamic_cast<CDA_CellMLComponent*>(comp2.getPointer()));
}

iface::cellml_api::CellMLElement*
CDA_ConnectedCellMLVariableIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    if (variableStack.size() == 0)
      return NULL;
    VariableStackFrame* topFrame = variableStack.front();
    if (topFrame->mapVariableIterator == NULL)
    {
      RETURN_INTO_OBJREF(conn, iface::cellml_api::Connection,
                         topFrame->connectionIterator->nextConnection());
      if (conn == NULL)
      {
        // If there are no more connections, we have to go to the next frame in the
        // VariableStackFrame...
        variableStack.pop_front();
        delete topFrame;
        continue;
      }

      // See if one or the other of the components matches the current one...
      RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                         conn->componentMapping());
      RETURN_INTO_OBJREF(c1, iface::cellml_api::CellMLComponent,
                         cm->firstComponent());
      if (c1 == NULL)
        continue;
      mConsider1 = (CompareComponentsImportAware
                    (c1, topFrame->whichComponent));

      RETURN_INTO_OBJREF(c2, iface::cellml_api::CellMLComponent,
                         cm->firstComponent());
      if (c2 == NULL)
        continue;
      mConsider2 = (CompareComponentsImportAware
                    (c2, topFrame->whichComponent));

      // There is no point even looking at this connection if neither component
      // matches.
      if (!mConsider1 && !mConsider2)
        continue;

      RETURN_INTO_OBJREF(vms, iface::cellml_api::MapVariablesSet,
                         conn->variableMappings());
      topFrame->mapVariableIterator =
        already_AddRefd<iface::cellml_api::MapVariablesIterator>
        (static_cast<iface::cellml_api::MapVariablesIterator*>
         (vms->iterateMapVariables()));
    }

    RETURN_INTO_WSTRING(targetVn, topFrame->whichVariable->name());
    
    // Now fetch the MapVariables
    RETURN_INTO_OBJREF(mv, iface::cellml_api::MapVariables,
                       topFrame->mapVariableIterator->nextMapVariable());

    // See if variable_1 is the variable of interest...
    ObjRef<iface::cellml_api::CellMLVariable> vother;
    if (mConsider1)
    {
      RETURN_INTO_WSTRING(vn, mv->firstVariableName());
      if (vn == targetVn)
      {
        // See if variable_2 has already been found...
        vother = already_AddRefd<iface::cellml_api::CellMLVariable>
          (mv->secondVariable());
        if (vother == NULL)
          continue;
      }
    }
    if (mConsider2 && vother == NULL)
    {
      RETURN_INTO_WSTRING(vn, mv->secondVariableName());
      if (vn == targetVn)
      {
        // See if variable_2 has already been found...
        vother = already_AddRefd<iface::cellml_api::CellMLVariable>
          (mv->firstVariable());
        if (vother == NULL)
          continue;
      }
    }
    if (vother == NULL)
      continue;

    CDA_CellMLVariable *votherImpl =
      dynamic_cast<CDA_CellMLVariable*>(vother.getPointer());
    // Now go down the variableStack and look for matching variables.
    std::list<VariableStackFrame*>::iterator vsi;
    for (vsi = variableStack.begin(); vsi != variableStack.end(); vsi++)
      if ((*vsi)->whichVariable == votherImpl)
        break;
    if (vsi != variableStack.end())
      continue;

    // If we get here, we have the next return value. However, we need to
    // firstly set up another stack frame for next time, as we are
    // performing a depth first search for variables.
    CDA_CellMLComponent* comp =
      dynamic_cast<CDA_CellMLComponent*>(votherImpl->mParent);
    if (comp == NULL)
      throw iface::cellml_api::CellMLException();
    
    CDA_Model* m = dynamic_cast<CDA_Model*>(comp->mParent);
    if (m == NULL)
      throw iface::cellml_api::CellMLException();
    
    RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet,
                       m->connections());
    RETURN_INTO_OBJREF(ci, iface::cellml_api::ConnectionIterator,
                       cs->iterateConnections());
    
    VariableStackFrame* newFrame = new VariableStackFrame;
    newFrame->whichVariable = votherImpl;
    // This should be safe, as we wouldn't get votherImpl if it lacked a
    // component.
    newFrame->whichComponent = dynamic_cast<CDA_CellMLComponent*>
      (votherImpl->mParent);
    newFrame->connectionIterator = ci;
    variableStack.push_front(newFrame);

    // Finally, return the variable we found...
    vother->add_ref();
    return vother;
  }
}

CDA_CellMLElementIterator::CDA_CellMLElementIterator
(
 iface::dom::Element* parentElement,
 CDA_CellMLElementSet* ownerSet
)
  : CDA_DOMElementIteratorBase(parentElement),
    _cda_refcount(1), parentSet(ownerSet)
{
  parentSet->add_ref();
  registerListener();
}

CDA_CellMLElementIterator::~CDA_CellMLElementIterator()
{
  deregisterListener();
  parentSet->release_ref();
}

static iface::cellml_api::CellMLElement*
WrapCellMLElement(iface::XPCOM::IObject* newParent,
                  iface::dom::Element* el)
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(ln, el->localName());
  RETURN_INTO_OBJREF(pn, iface::dom::Node, el->parentNode());
  RETURN_INTO_WSTRING(pln, pn->localName());

  if (ln == L"component")
  {
    if (pln == L"import")
      return new CDA_ImportComponent(newParent, el);
    else
      return new CDA_CellMLComponent(newParent, el);
  }
  else if (ln == L"units")
  {
    if (pln == L"import")
      return new CDA_ImportUnits(newParent, el);
    else
      return new CDA_Units(newParent, el);
  }
  else if (ln == L"unit")
  {
    return new CDA_Unit(newParent, el);
  }
  else if (ln == L"import")
  {
    return new CDA_CellMLImport(newParent, el);
  }
  else if (ln == L"variable")
  {
    return new CDA_CellMLVariable(newParent, el);
  }
  else if (ln == L"component_ref")
  {
    return new CDA_ComponentRef(newParent, el);
  }
  else if (ln == L"relationship_ref")
  {
    return new CDA_RelationshipRef(newParent, el);
  }
  else if (ln == L"group")
  {
    return new CDA_Group(newParent, el);
  }
  else if (ln == L"connection")
  {
    return new CDA_Connection(newParent, el);
  }
  else if (ln == L"map_components")
  {
    return new CDA_MapComponents(newParent, el);
  }
  else if (ln == L"map_variables")
  {
    return new CDA_MapVariables(newParent, el);
  }
  else if (ln == L"reaction")
  {
    return new CDA_Reaction(newParent, el);
  }
  // This part of the API is broken because the interfaces are bad. Need to
  // redesign them.
  //else if (ln == L"variable_ref")
  //{
  //  return new CDA_VariableRef(newParent, el);
  //}
  // Role goes here once we fix everything up.
  else
  {
    // We are being asked to wrap something that shouldn't be here. What should
    // we do? For now I will raise a CellMLException.
    throw iface::cellml_api::CellMLException();
  }
}

iface::cellml_api::CellMLElement*
CDA_CellMLElementIterator::next()
  throw(std::exception&)
{
  iface::dom::Element* el;
  while (true)
  {
    el = fetchNextElement();
    if (el == NULL)
    {
      return NULL;
    }
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS || nsURI == CELLML_1_1_NS)
      break;
  }

  // We have an element. Now go back to the set, and look in the map...
  std::map<iface::dom::Element*,iface::cellml_api::CellMLElement*>
     ::iterator i = parentSet->childMap.find(el);
  if (i != parentSet->childMap.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We have an unwrapped element, so we need to wrap it...
  iface::cellml_api::CellMLElement* cel =
    WrapCellMLElement(parentSet->mParent, el);
  parentSet->addChildToWrapper(cel);

  return cel;
}

iface::cellml_api::MathMLElement
CDA_MathMLElementIterator::next()
  throw(std::exception&)
{
  iface::dom::Element* el;
  iface::mathml_dom::MathMLElement* mel;
  while (true)
  {
    el = fetchNextElement();
    if (el == NULL)
      return NULL;
    QUERY_INTERFACE(mel, el, mathml_dom::MathMLElement);
    if (mel)
    {
      return mel;
    }
  }
}

CDA_ExtensionElementList::CDA_ExtensionElementList(iface::dom::Element* el)
  : _cda_refcount(1)
{
  nl = el->childNodes();
}

CDA_ExtensionElementList::~CDA_ExtensionElementList()
{
  nl->release_ref();
}

u_int32_t
CDA_ExtensionElementList::length()
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  u_int32_t le = 0, l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS ||
        nsURI == CELLML_1_1_NS ||
        nsURI == RDF_NS)
      continue;

    le++;
  }

  return le;
}

bool
CDA_ExtensionElementList::contains(const iface::cellml_api::ExtensionElement x)
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  u_int32_t l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;

    if (el == x)
      return true;
  }

  return false;  
}

int32_t
CDA_ExtensionElementList::getIndexOf(const iface::cellml_api::ExtensionElement x)
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  u_int32_t le = 0, l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS ||
        nsURI == CELLML_1_1_NS ||
        nsURI == RDF_NS)
      continue;


    if (el == x)
      return le;

    le++;
  }

  return -1;
}

iface::cellml_api::ExtensionElement
CDA_ExtensionElementList::getAt(u_int32_t index)
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  u_int32_t l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS ||
        nsURI == CELLML_1_1_NS ||
        nsURI == RDF_NS)
      continue;

    if (index == 0)
    {
      el->add_ref();
      return el;
    }

    index--;
  }

  return NULL;
}

CDA_MathList::CDA_MathList(iface::dom::Element* aParentEl)
  : _cda_refcount(1), mParentEl(aParentEl)
{
  mParentEl->add_ref();
}

CDA_MathList::~CDA_MathList()
{
}

u_int32_t
CDA_MathList::length()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathMLElementIterator, iterate());
  u_int32_t l = 0;
  while (true)
  {
    RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, ml->next());
    if (me == NULL)
      return l;
    l++;
  }
}

bool
CDA_MathList::contains(const iface::cellml_api::MathMLElement x)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathMLElementIterator, iterate());
  while (true)
  {
    RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, ml->next());
    if (me == NULL)
      return false;
    iface::mathml_dom::MathMLElement* xme = (iface::mathml_dom::MathMLElement*)x;
    if (me == xme)
      return true;
  }
}

iface::cellml_api::MathMLElementIterator*
CDA_MathList::iterate()
  throw(std::exception&)
{
  return new CDA_MathMLElementIterator(mParentEl);
}

CDA_CellMLElementSet::CDA_CellMLElementSet
(
 CDA_CellMLElement* parent,
 iface::dom::Element* parentEl
)
  : mParent(parent), mElement(parentEl),
    // Note: The reference count starts at zero, because an Element is
    // permanently part of an Element, and so needs no refcount when it is
    // constructed.
    _cda_refcount(0)
{
}

CDA_CellMLElementSet::~CDA_CellMLElementSet()
{
  if (_cda_refcount != 0)
    printf("Warning: release_ref called too few times on %s.\n",
           typeid(this).name());

  std::map<iface::dom::Element*,iface::cellml_api::CellMLElement*>::iterator
    i;

  for (i = childMap.begin(); i != childMap.end(); i++)
    delete (*i).second;
}

iface::cellml_api::CellMLElementIterator*
CDA_CellMLElementSet::iterate()
  throw(std::exception&)
{
  return new CDA_CellMLElementIterator(mElement, this);
}

void
CDA_CellMLElementSet::addChildToWrapper(iface::cellml_api::CellMLElement* el)
{
  childMap.insert(std::pair<iface::dom::Element*,
                            iface::cellml_api::CellMLElement*>
                  (dynamic_cast<CDA_CellMLElement*>(el)->datastore, el));
}

void
CDA_CellMLElementSet::removeChildFromWrapper(iface::cellml_api::CellMLElement* el)
{
  childMap.erase(dynamic_cast<CDA_CellMLElement*>(el)->datastore);
}

iface::cellml_api::NamedCellMLElement*
CDA_NamedCellMLElementSetBase::get(const wchar_t* name)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(elIt, iface::cellml_api::CellMLElementIterator, iterate());
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, elIt->next());
    if (el == NULL)
      return NULL;
    iface::cellml_api::NamedCellMLElement* nel =
      dynamic_cast<iface::cellml_api::NamedCellMLElement*>(el.getPointer());
    wchar_t* n = nel->name();
    bool match = !wcscmp(name, n);
    free(n);

    if (match)
    {
      nel->add_ref();
      return nel;
    }
  }
}

#define SIMPLE_SET_ITERATORFETCH(setname, iteratorname, ifacename, funcname) \
iface::cellml_api::CellMLElementIterator* \
setname::iterate() \
  throw(std::exception&) \
{ \
  CDA_CellMLElementIterator* ei = dynamic_cast<CDA_CellMLElementIterator*>(mInner->iterate()); \
  iface::cellml_api::CellMLElementIterator* it = new iteratorname(ei); \
  ei->release_ref(); \
  return it; \
} \
ifacename* \
setname::funcname() \
  throw(std::exception&) \
{ \
  return dynamic_cast<ifacename*>(iterate()); \
}

#define SIMPLE_SET_ITERATORFETCH_GET(setname, iteratorname, ifacename, funcname, basename, getname) \
SIMPLE_SET_ITERATORFETCH(setname, iteratorname, ifacename, funcname) \
basename* \
setname::getname(const wchar_t* name) \
  throw(std::exception&) \
{ \
  return dynamic_cast<basename*>(get(name)); \
}

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_CellMLComponentSetBase,
 CDA_CellMLComponentIterator,
 iface::cellml_api::CellMLComponentIterator,
 iterateComponents,
 iface::cellml_api::CellMLComponent,
 getComponent
);

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_ImportComponentSet,
 CDA_ImportComponentIterator,
 iface::cellml_api::ImportComponentIterator,
 iterateImportComponents,
 iface::cellml_api::ImportComponent,
 getImportComponent
);

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_CellMLVariableSet,
 CDA_CellMLVariableIterator,
 iface::cellml_api::CellMLVariableIterator,
 iterateVariables,
 iface::cellml_api::CellMLVariable,
 getVariable
);

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_UnitsSetBase,
 CDA_UnitsIterator,
 iface::cellml_api::UnitsIterator,
 iterateUnits,
 iface::cellml_api::Units,
 getUnits
);

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_ImportUnitsSet,
 CDA_ImportUnitsIterator,
 iface::cellml_api::ImportUnitsIterator,
 iterateImportUnits,
 iface::cellml_api::ImportUnits,
 getImportUnits
);

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_CellMLImportSet,
 CDA_CellMLImportIterator,
 iface::cellml_api::CellMLImportIterator,
 iterateImports,
 iface::cellml_api::CellMLImport,
 getImport
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_UnitSet,
 CDA_UnitIterator,
 iface::cellml_api::UnitIterator,
 iterateUnits
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_ConnectionSet,
 CDA_ConnectionIterator,
 iface::cellml_api::ConnectionIterator,
 iterateConnections
);

iface::cellml_api::CellMLElementIterator*
CDA_GroupSet::iterate()
  throw(std::exception&)
{
  CDA_CellMLElementIterator* ei =
    dynamic_cast<CDA_CellMLElementIterator*>
    (mInner->iterate());
  iface::cellml_api::CellMLElementIterator* it = 
    new CDA_GroupIterator(ei, filterByRRName,
                          mFilterRRName.c_str());
  ei->release_ref();
  return it;
}

iface::cellml_api::GroupIterator*
CDA_GroupSet::iterateGroups()
  throw(std::exception&)
{
  return dynamic_cast<iface::cellml_api::GroupIterator*>(iterate());
}

SIMPLE_SET_ITERATORFETCH
(
 CDA_RelationshipRefSet,
 CDA_RelationshipRefIterator,
 iface::cellml_api::RelationshipRefIterator,
 iterateRelationshipRefs
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_ComponentRefSet,
 CDA_ComponentRefIterator,
 iface::cellml_api::ComponentRefIterator,
 iterateComponentRefs
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_MapVariablesSet,
 CDA_MapVariablesIterator,
 iface::cellml_api::MapVariablesIterator,
 iterateMapVariables
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_VariableRefSetBase,
 CDA_VariableRefIterator,
 iface::cellml_api::VariableRefIterator,
 iterateVariableRefs
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_ReactantVariableRefSet,
 CDA_ReactantVariableRefIterator,
 iface::cellml_api::ReactantVariableRefIterator,
 iterateReactantVariableRefs
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_ProductVariableRefSet,
 CDA_ProductVariableRefIterator,
 iface::cellml_api::ProductVariableRefIterator,
 iterateProductVariableRefs
);

#define SIMPLE_ITERATOR_NEXT(itname, ifacename, nextname) \
iface::cellml_api::CellMLElement* \
itname::next() \
  throw(std::exception&) \
{ \
  while (true) \
  { \
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, mInner->next()); \
    if (el == NULL) \
      return NULL; \
    iface::cellml_api::ifacename* targEl = \
      dynamic_cast<iface::cellml_api::ifacename*>(el.getPointer()); \
    if (targEl == NULL) \
      continue; \
    targEl->add_ref(); \
    return targEl; \
  } \
} \
iface::cellml_api::ifacename*\
itname::nextname() \
  throw(std::exception&) \
{ \
  return dynamic_cast<iface::cellml_api::ifacename*>(next()); \
}

SIMPLE_ITERATOR_NEXT(CDA_CellMLComponentIteratorBase, CellMLComponent,
                     nextComponent);
SIMPLE_ITERATOR_NEXT(CDA_ImportComponentIterator, ImportComponent,
                     nextImportComponent);
SIMPLE_ITERATOR_NEXT(CDA_CellMLVariableIterator, CellMLVariable,
                     nextVariable);
SIMPLE_ITERATOR_NEXT(CDA_UnitsIteratorBase, Units,
                     nextUnits);
SIMPLE_ITERATOR_NEXT(CDA_ImportUnitsIterator, ImportUnits,
                     nextImportUnits);
SIMPLE_ITERATOR_NEXT(CDA_CellMLImportIterator, CellMLImport,
                     nextImport);
SIMPLE_ITERATOR_NEXT(CDA_UnitIterator, Unit,
                     nextUnit);
SIMPLE_ITERATOR_NEXT(CDA_ConnectionIterator, Connection,
                     nextConnection);

static bool
DoesGroupHaveRelationshipRef(iface::dom::Element* el, const std::wstring& rrname)
{
  RETURN_INTO_OBJREF(cn, iface::dom::NodeList, el->childNodes());
  u_int32_t i, l = cn->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(cni, iface::dom::Node, cn->item(i));
    if (cni == NULL)
      return false;
    DECLARE_QUERY_INTERFACE_OBJREF(el, cni, dom::Element);
    if (el == NULL)
      continue;

    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI != CELLML_1_0_NS && 
        nsURI != CELLML_1_1_NS)
      continue;

    RETURN_INTO_WSTRING(ln, el->localName());
    if (ln != L"relationship_ref")
      continue;

    RETURN_INTO_WSTRING(rn, el->getAttributeNS(NULL_NS, L"relationship"));
    if (rn != rrname)
      continue;

    return true;
  }
  return false;
}

iface::cellml_api::CellMLElement* \
CDA_GroupIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, mInner->next());
    if (el == NULL)
      return NULL;
    CDA_Group* targEl =
      dynamic_cast<CDA_Group*>(el.getPointer());
    if (targEl == NULL)
      continue;

    if (filterByRRName && 
        !DoesGroupHaveRelationshipRef(targEl->datastore, mFilterRRName))
      continue;

    targEl->add_ref();
    return targEl;
  }
}

iface::cellml_api::Group*
CDA_GroupIterator::nextGroup()
  throw(std::exception&)
{
  return dynamic_cast<iface::cellml_api::Group*>(next());
}

SIMPLE_ITERATOR_NEXT(CDA_RelationshipRefIterator, RelationshipRef,
                     nextRelationshipRef);
SIMPLE_ITERATOR_NEXT(CDA_ComponentRefIterator, ComponentRef,
                     nextComponentRef);
SIMPLE_ITERATOR_NEXT(CDA_MapVariablesIterator, MapVariables,
                     nextMapVariable);
SIMPLE_ITERATOR_NEXT(CDA_VariableRefIteratorBase, VariableRef,
                     nextVariableRef);
SIMPLE_ITERATOR_NEXT(CDA_ReactantVariableRefIterator, ReactantVariableRef,
                     nextReactantVariableRef);
SIMPLE_ITERATOR_NEXT(CDA_ProductVariableRefIterator, ProductVariableRef,
                     nextProductVariableRef);

iface::cellml_api::GroupSet*
CDA_GroupSet::getSubsetInvolvingRelationship(const wchar_t* relName)
  throw(std::exception&)
{
  return new CDA_GroupSet(mInner, relName);
}

iface::cellml_api::GroupSet*
CDA_GroupSet::subsetInvolvingEncapsulation()
  throw(std::exception&)
{
  return new CDA_GroupSet(mInner, L"encapsulation");
}

iface::cellml_api::GroupSet*
CDA_GroupSet::subsetInvolvingContainment()
  throw(std::exception&)
{
  return new CDA_GroupSet(mInner, L"containment");
}
