#include "CellMLImplementation.hpp"

#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"
#define CMETA_NS L"http://www.cellml.org/metadata/1.0#"
#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"
#define RDF_NS L"http://www.w3.org/1999/02/22-rdf-syntax-ns#"

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
  return wcsdup(L"<notimplementedyet />");
}

void
CDA_RDFXMLStringRepresentation::serialisedData(wchar_t* attr)
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
CDA_URI::asText(wchar_t* attr)
  throw(std::exception&)
{
  datastore->value(attr);
}

CDA_CellMLElement::CDA_CellMLElement
(
 iface::XPCOM::IObject* parent,
 iface::dom::Element* idata
)
  : datastore(idata), mParent(parent), _cda_refcount(1),
    userData(NULL)
{
  datastore->add_ref();
}

CDA_CellMLElement::~CDA_CellMLElement()
{
  datastore->release_ref();
}

iface::cellml_api::CellMLAttributeString
CDA_CellMLElement::cellmlVersion()
  throw(std::exception&)
{
  // We default to the latest known version, so that we will be backwards
  // compatible when a newer version comes out.
  wchar_t* version = L"1.1";
  wchar_t* ns = datastore->namespaceURI();
  if (!wcscmp(version, CELLML_1_0_NS))
    version = L"1.0";
  free(ns);
  return wcsdup(version);
}

iface::cellml_api::CellMLAttributeString
CDA_CellMLElement::cmetaId()
  throw(std::exception&)
{
  return datastore->getAttributeNS(CMETA_NS, L"id");
}

void
CDA_CellMLElement::cmetaId(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
  return datastore->setAttributeNS(CMETA_NS, L"id", attr);
}

iface::cellml_api::RDFRepresentation*
CDA_CellMLElement::getRDFRepresentation(const wchar_t* type)
  throw(std::exception&)
{
  if (!wcscmp(type, L"http://www.cellml.org/RDFXML/DOM"))
    return new CDA_RDFXMLDOMRepresentation(datastore);
  else if (!wcscmp(type, L"http://www.cellml.org/RDFXML/string"))
    return new CDA_RDFXMLStringRepresentation(datastore);
  else
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
  // Any child element which we don't recognise gets removed...
  ObjRef<iface::dom::NodeList> nl =
    already_AddRefd<iface::dom::NodeList>(datastore->childNodes());
  u_int32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    iface::dom::Node* n = nl->item(i);
    if (n->query_interface("cellml_api::CellMLElement"))
    {
      n->release_ref();
      continue;
    }
    datastore->removeChild(n)->release_ref();
    n->release_ref();
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

    // Add to our local CellML wrapping, if it exists...
    if (children)
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
    if (elNew->mParent == NULL)
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
 const iface::cellml_api::CellMLAttributeString type,
 const iface::cellml_api::CellMLAttributeString name
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
      RETURN_INTO_WSTRING(tn, el->datastore->tagName());
      if (tn == type)
      {
        RETURN_INTO_WSTRING(name, el->name());
        if (tn == name)
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
  iface::cellml_api::CellMLElement* cand = this;

  while (true)
  {
    iface::cellml_api::CellMLElement* el =
      dynamic_cast<iface::cellml_api::CellMLElement*>(mParent);
    if (el == NULL)
    {
      // We have reached a point where no parent can be found. Either we have a
      // Model, or there is no Model at the root.
      iface::cellml_api::Model* m =
        dynamic_cast<iface::cellml_api::Model*>(cand);
      if (m != NULL)
        m->add_ref();
      return m;
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
  if (userData != NULL)
    userData->release_ref();
  userData = data;
  userData->add_ref();
}

iface::cellml_api::UserData*
CDA_CellMLElement::getUserData(const wchar_t* key)
  throw(std::exception&)
{
  if (userData != NULL)
    userData->add_ref();
  return userData;
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

iface::cellml_api::CellMLAttributeString
CDA_NamedCellMLElement::name()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttribute(L"name");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
}

void
CDA_NamedCellMLElement::name
(
 iface::cellml_api::CellMLAttributeString attr
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

CDA_Model::CDA_Model(iface::dom::Document* aDoc,
                     iface::dom::Element* modelElement)
  : CDA_CellMLElement(NULL, modelElement),
    CDA_NamedCellMLElement(NULL, modelElement),
    mDoc(aDoc)
{
  mDoc->add_ref();
}

CDA_Model::~CDA_Model()
{
  mDoc->release_ref();
}

iface::cellml_api::Model*
CDA_Model::getAlternateVersion(const wchar_t* cellmlVersion)
  throw(std::exception&)
{
  try
  {
    wchar_t* new_namespace;
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
    CDA_Model* cm = new CDA_Model(newDoc, de);
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
      iface::dom::Node* tmp = origItem->cloneNode(false);
      newItem = already_AddRefd<iface::dom::Node>(tmp);
    }
    else if (!wcscmp(nsURI, CELLML_1_0_NS) || 
             !wcscmp(nsURI, CELLML_1_1_NS))
    {
      // See what type of node it is...
      switch (origItem->nodeType())
      {
      case iface::dom::Node::ELEMENT_NODE:
        {
          wchar_t* ln = origItem->localName();
          newItem = already_AddRefd<iface::dom::Node>
            (mDoc->createElementNS(const_cast<wchar_t*>(aNewNamespace),
                                   ln));
          free(ln);
          break;
        }
      case iface::dom::Node::ATTRIBUTE_NODE:
        {
          wchar_t* ln = origItem->localName();
          newItem = 
            already_AddRefd<iface::dom::Node>
            (
             mDoc->createAttributeNS
             (const_cast<wchar_t*>(aNewNamespace), ln)
            );
          free(ln);
          break;
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
    free(nsURI);
    aCopy->appendChild(&newItem);
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
      static_cast<CDA_CellMLElementSet*>((childElements()))
     )
    );

  return new CDA_GroupSet(&allChildren);
}

iface::cellml_api::CellMLImportSet*
CDA_Model::imports()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (static_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_CellMLImportSet(&allChildren);
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
    if (&attr == NULL)
    {
      attr =
        already_AddRefd<iface::dom::Attr>
        (
         mDoc->createAttributeNS(L"http://www.w3.org/XML/1998/namespace",
                                 L"xml:base")
        );
      datastore->appendChild(&attr);
    }
    return new CDA_URI(&attr);
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
     (static_cast<CDA_CellMLElementSet*>(childElements())));

  return new CDA_UnitsSet(&allChildren /*, false */);
}

iface::cellml_api::UnitsSet*
CDA_Model::childUnits()
  throw(std::exception&)
{
  //ObjRef<CDA_CellMLElementSet> allChildren
  //  (already_AddRefd<CDA_CellMLElementSet>(childElements()));
  //return new CDA_UnitsSet(allChildren, true);
#ifdef SHOW_NOT_IMPLEMENTED
#error 'childUnits is not implemented yet.'
#endif
  return NULL;
}

iface::cellml_api::CellMLComponentSet*
CDA_Model::localComponents()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (static_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_CellMLComponentSet(&allChildren);
}

iface::cellml_api::CellMLComponentSet*
CDA_Model::components()
  throw(std::exception&)
{
  //ObjRef<CDA_CellMLElementSet> allChildren
  //  (already_AddRefd<CDA_CellMLElementSet>(childElements()));
#ifdef SHOW_NOT_IMPLEMENTED
#error 'components is not implemented yet.'
#endif
  //return new CDA_ComponentSet(allChildren, true);
  return NULL;
}

iface::cellml_api::ConnectionSet*
CDA_Model::connections()
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (static_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_ConnectionSet(&allChildren);
}

iface::cellml_api::GroupSet*
CDA_Model::findGroupsWithRelationshipRefName
(
 const iface::cellml_api::CellMLAttributeString name
)
  throw(std::exception&)
{
  ObjRef<CDA_CellMLElementSet> allChildren
    (already_AddRefd<CDA_CellMLElementSet>
     (static_cast<CDA_CellMLElementSet*>(childElements())));
  return new CDA_GroupSet(&allChildren, name);
}

void
CDA_Model::fullyInstantiateImports()
  throw(std::exception&)
{
  // XXX TODO: import instantiation.
#ifdef SHOW_NOT_IMPLEMENTED
#error 'fullyInstantiateImports is not implemented yet.'
#endif
}

iface::cellml_api::Model*
CDA_Model::generateFlattenedModel()
  throw(std::exception&)
{
  // XXX TODO: model flattening.
#ifdef SHOW_NOT_IMPLEMENTED
#error 'generateFlattenedModel is not implemented yet.'
#endif
  return NULL;
}

iface::cellml_api::CellMLComponent*
CDA_Model::createComponent()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"component");
    return new CDA_CellMLComponent(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"component");
    return new CDA_ImportComponent(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"units");
    return new CDA_Units(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"units");
    return new CDA_ImportUnits(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()), L"unit");
    return new CDA_Unit(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"import");
    return new CDA_CellMLImport(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"variable");
    return new CDA_CellMLVariable(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"component_ref");
    return new CDA_ComponentRef(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"relationship_ref");
    return new CDA_RelationshipRef(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"group");
    return new CDA_Group(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"connection");
    return new CDA_Connection(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"map_variables");
    return new CDA_MapVariables(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"reaction");
    return new CDA_Reaction(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"variable_ref");
    return new CDA_ReactantVariableRef(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"variable_ref");
    return new CDA_RateVariableRef(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"role");

    newNode->setAttributeNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"role", L"reactant");
    return new CDA_ReactantRole(NULL, &newNode);
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
    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"role");

    newNode->setAttributeNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"role", L"product");
    return new CDA_ProductRole(NULL, &newNode);
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

    ObjRef<iface::dom::Element> newNode =
      mDoc->createElementNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"role");
    
    newNode->setAttributeNS(const_cast<wchar_t*>(myNamespace.c_str()),
                            L"role", L"rate");

    return new CDA_RateRole(NULL, &newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException();
  }
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
      iface::mathml_dom::MathMLElement* mme =
        dynamic_cast<iface::mathml_dom::MathMLElement*>
        (node->query_interface("mathml_dom::MathMLElement"));
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
}


iface::cellml_api::UnitsSet*
CDA_CellMLComponent::units()
  throw(std::exception&)
{
}

iface::cellml_api::ConnectionSet*
CDA_CellMLComponent::connections()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLComponent*
CDA_CellMLComponent::encapsulationParent()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLComponentSet*
CDA_CellMLComponent::encapsulationChildren()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLComponent*
CDA_CellMLComponent::containmentParent()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLComponentSet*
CDA_CellMLComponent::containmentChildren()
  throw(std::exception&)
{
}

u_int32_t
CDA_CellMLComponent::importNumber()
  throw(std::exception&)
{
}

bool
CDA_UnitsBase::isBaseUnits()
  throw(std::exception&)
{
}

void
CDA_UnitsBase::isBaseUnits(bool attr)
  throw(std::exception&)
{
}

iface::cellml_api::UnitSet*
CDA_UnitsBase::unitCollection()
  throw(std::exception&)
{
}

int32_t
CDA_Unit::prefix()
  throw(std::exception&)
{
}

void
CDA_Unit::prefix(int32_t attr)
  throw(std::exception&)
{
}

double
CDA_Unit::multiplier()
  throw(std::exception&)
{
}

void
CDA_Unit::multiplier(double attr)
  throw(std::exception&)
{
}

double
CDA_Unit::offset()
  throw(std::exception&)
{
}

void
CDA_Unit::offset(double attr)
  throw(std::exception&)
{
}

double
CDA_Unit::exponent()
  throw(std::exception&)
{
}

void
CDA_Unit::exponent(double attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_Unit::units()
  throw(std::exception&)
{
}

void
CDA_Unit::units(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::URI*
CDA_CellMLImport::xlinkHref()
  throw(std::exception&)
{
}

iface::cellml_api::ImportComponentSet*
CDA_CellMLImport::components()
  throw(std::exception&)
{
}

iface::cellml_api::ImportUnitsSet*
CDA_CellMLImport::units()
  throw(std::exception&)
{
}

iface::cellml_api::ConnectionSet*
CDA_CellMLImport::importedConnections()
  throw(std::exception&)
{
}

void
CDA_CellMLImport::fullyInstantiate()
  throw(std::exception&)
{
}

bool
CDA_CellMLImport::wasFullyInstantiated()
  throw(std::exception&)
{
}

u_int32_t
CDA_CellMLImport::uniqueIdentifier()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_ImportUnits::componentRef()
  throw(std::exception&)
{
}

void
CDA_ImportUnits::componentRef(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

void
CDA_ImportUnits::fullyInstantiate()
  throw(std::exception&)
{
}

bool
CDA_ImportUnits::wasFullyInstantiated()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_CellMLVariable::initialValue()
  throw(std::exception&)
{
}

void
CDA_CellMLVariable::initialValue
(
 iface::cellml_api::CellMLAttributeString attr
)
  throw(std::exception&)
{
}

iface::cellml_api::VariableInterface
CDA_CellMLVariable::privateInterface()
  throw(std::exception&)
{
}

void
CDA_CellMLVariable::privateInterface(iface::cellml_api::VariableInterface attr)
  throw(std::exception&)
{
}

iface::cellml_api::VariableInterface
CDA_CellMLVariable::publicInterface()
  throw(std::exception&)
{
}

void
CDA_CellMLVariable::publicInterface(iface::cellml_api::VariableInterface attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLVariableSet*
CDA_CellMLVariable::connectedVariables()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLVariable*
CDA_CellMLVariable::sourceVariable()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_ComponentRef::componentName()
  throw(std::exception&)
{
}

void
CDA_ComponentRef::componentName(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::ComponentRefSet*
CDA_ComponentRef::componentRefs()
  throw(std::exception&)
{
}

iface::cellml_api::ComponentRef*
CDA_ComponentRef::parentComponentRef()
  throw(std::exception&)
{
}

iface::cellml_api::Group*
CDA_ComponentRef::parentGroup()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_RelationshipRef::name()
  throw(std::exception&)
{
}

void
CDA_RelationshipRef::name(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_RelationshipRef::relationship()
  throw(std::exception&)
{
}

void
CDA_RelationshipRef::relationship
(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::URI*
CDA_RelationshipRef::relationship_namespace()
  throw(std::exception&)
{
}

iface::cellml_api::RelationshipRefSet*
CDA_Group::relationshipRefs()
  throw(std::exception&)
{
}

void
CDA_Group::relationshipRefs(iface::cellml_api::RelationshipRefSet* attr)
  throw(std::exception&)
{
}

iface::cellml_api::ComponentRefSet*
CDA_Group::componentRefs()
  throw(std::exception&)
{
}

void
CDA_Group::componentRefs(iface::cellml_api::ComponentRefSet* attr)
  throw(std::exception&)
{
}

bool
CDA_Group::isEncapsulation()
  throw(std::exception&)
{
}

bool
CDA_Group::isContainment()
  throw(std::exception&)
{
}

iface::cellml_api::MapComponents*
CDA_Connection::componentMapping()
  throw(std::exception&)
{
}

iface::cellml_api::MapVariablesSet*
CDA_Connection::variableMappings()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_MapComponents::firstComponentName()
  throw(std::exception&)
{
}

void
CDA_MapComponents::firstComponentName(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_MapComponents::secondComponentName()
  throw(std::exception&)
{
}

void 
CDA_MapComponents::secondComponentName(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLComponent*
CDA_MapComponents::firstComponent()
  throw(std::exception&)
{
}

void
CDA_MapComponents::firstComponent(iface::cellml_api::CellMLComponent* attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLComponent*
CDA_MapComponents::secondComponent()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_MapVariables::firstVariableName()
  throw(std::exception&)
{
}

void
CDA_MapVariables::firstVariableName(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLAttributeString
CDA_MapVariables::secondVariableName()
  throw(std::exception&)
{
}

void
CDA_MapVariables::secondVariableName(iface::cellml_api::CellMLAttributeString attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLVariable*
CDA_MapVariables::firstVariable()
  throw(std::exception&)
{
}

void
CDA_MapVariables::firstVariable(iface::cellml_api::CellMLVariable* attr)
  throw(std::exception&)
{
}

iface::cellml_api::CellMLVariable*
CDA_MapVariables::secondVariable()
  throw(std::exception&)
{
}
void
CDA_MapVariables::secondVariable(iface::cellml_api::CellMLVariable* attr)
  throw(std::exception&)
{
}

iface::cellml_api::ReactantVariableRefSet*
CDA_Reaction::reactants()
  throw(std::exception&)
{
}

iface::cellml_api::ProductVariableRefSet*
CDA_Reaction::products()
  throw(std::exception&)
{
}

iface::cellml_api::RateVariableRef*
CDA_Reaction::rate()
  throw(std::exception&)
{
}

iface::cellml_api::CellMLVariable*
CDA_VariableRef::variable()
  throw(std::exception&)
{
}

wchar_t*
CDA_VariableRef::refType()
  throw(std::exception&)
{
}

iface::cellml_api::ReactantRole*
CDA_ReactantVariableRef::role() throw(std::exception&)
{
}

void
CDA_ReactantVariableRef::role(iface::cellml_api::ReactantRole* attr)
  throw(std::exception&)
{
}

iface::cellml_api::ProductRole*
CDA_ProductVariableRef::role()
  throw(std::exception&)
{
}

void
CDA_ProductVariableRef::role(iface::cellml_api::ProductRole* attr)
  throw(std::exception&)
{
}

iface::cellml_api::RateRole*
CDA_RateVariableRef::role() throw(std::exception&)
{
}

void
CDA_RateVariableRef::role(iface::cellml_api::RateRole* attr)
  throw(std::exception&)
{
}

wchar_t*
CDA_Role::roleType()
  throw(std::exception&)
{
}

CDA_DOMElementIteratorBase::CDA_DOMElementIteratorBase
(
 iface::dom::Element* parentElement
)
  : mPrevElement(NULL), mNextElement(NULL), mParentElement(parentElement)
{
  mParentElement->add_ref();
  mNodeList = mParentElement->childNodes();
  mParentElement->addEventListener(L"DOMNodeInserted", this, false);
}

CDA_DOMElementIteratorBase::~CDA_DOMElementIteratorBase()
{
  mNodeList->release_ref();
  mParentElement->release_ref();
  if (mPrevElement != NULL)
  {
    mPrevElement->release_ref();
  }
  if (mNextElement != NULL)
  {
    mNextElement->removeEventListener(L"DOMNodeRemoved", this, false);
    mNextElement->release_ref();
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
      for (i = 0; i < mNodeList->length(); i++)
      {
        ObjRef<iface::dom::Node> nodeHit
          (already_AddRefd<iface::dom::Node>(mNodeList->item(i)));
        mPrevElement =
          dynamic_cast<iface::dom::Element*>
          (nodeHit->query_interface("cellml_api::CellMLElement"));
        if (mPrevElement != NULL)
        {
          mPrevElement->add_ref();
          break;
        }
      }
      if (mPrevElement == NULL)
        return NULL;
    }
    else
    {
      // Once mNextElement is NULL, we are at the end until more elements are
      // inserted.
      if (mNextElement == NULL)
        return NULL;
      mPrevElement->release_ref();
      mPrevElement = mNextElement;
      mNextElement->removeEventListener(L"DOMNodeRemoved", this, false);
      mNextElement = NULL;
    }

    // We now have a valid previous element, which will be our return value.
    // However, to maintain our assumptions, we need to find mNextElement.
    ObjRef<iface::dom::Node> nodeHit = mPrevElement->nextSibling();
    while (&nodeHit != NULL)
    {
      mNextElement =
        dynamic_cast<iface::dom::Element*>
        (nodeHit->query_interface("cellml_api::CellMLElement"));
      if (mNextElement != NULL)
      {
        mNextElement->addEventListener(L"DOMNodeRemoved", this, false);
        mNextElement->add_ref();
        break;
      }
      nodeHit = nodeHit->nextSibling();
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
      while (nodeHit.getPointer() != NULL)
      {
        mNextElement =
          dynamic_cast<iface::dom::Element*>
          (nodeHit->query_interface("dom::Element"));
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
      iface::events::MutationEvent* mevt =
        dynamic_cast<iface::events::MutationEvent*>(evt->query_interface("events::MutationEvent"));

      ObjRef<iface::dom::Node> rn =
        already_AddRefd<iface::dom::Node>(mevt->relatedNode());
      if (dynamic_cast<iface::dom::Element*>
          (rn->query_interface("dom::Element")) != mParentElement)
        return;
      
      ObjRef<iface::dom::Node> tn =
        already_AddRefd<iface::dom::Node>(mevt->target());
      iface::dom::Element* te = dynamic_cast<iface::dom::Element*>
        (tn->query_interface("dom::Element"));
      
      if (te == NULL)
        return;

      // See if target lies between mPrevElement and mNextElement...
      ObjRef<iface::dom::Node> curN
        (already_AddRefd<iface::dom::Node>(mPrevElement->nextSibling()));
      iface::dom::Element* curE =
        dynamic_cast<iface::dom::Element*>(curN->query_interface("dom::Element"));
      while ((curE != mNextElement ||
              (mNextElement == NULL && &curN != NULL)) &&
             curE != te)
      {
        curN = already_AddRefd<iface::dom::Node>(curN->nextSibling());
        curE = dynamic_cast<iface::dom::Element*>
          (curN->query_interface("dom::Element"));
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
