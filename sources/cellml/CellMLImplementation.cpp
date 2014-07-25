// CellML gets linked into the same module as DOMWriter...
#define IN_DOMWRITER_MODULE
#define MODULE_CONTAINS_xpcom
#define MODULE_CONTAINS_RDFAPISPEC
#define MODULE_CONTAINS_CellMLAPISPEC
#define MODULE_CONTAINS_CellMLBootstrap
#define MODULE_CONTAINS_CellMLEvents
#define MODULE_CONTAINS_DOMAPISPEC
#define MODULE_CONTAINS_DOMevents
#define MODULE_CONTAINS_DOMevents
#define MODULE_CONTAINS_MathMLcontentAPISPEC
#include "CellMLImplementation.hpp"
#include "DOMWriter.hxx"
#ifdef ENABLE_RDF
#include "RDFBootstrap.hpp"
#endif
#include <memory>
#include <assert.h>
#include <stdio.h>

#define NULL_NS L""
#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"
#define CMETA_NS L"http://www.cellml.org/metadata/1.0#"
#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"
#define RDF_NS L"http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define XLINK_NS L"http://www.w3.org/1999/xlink"

static already_AddRefd<CDA_CellMLElement>
WrapCellMLElement(CDA_CellMLElement* newParent,
                  iface::dom::Element* el) throw(std::exception&);

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
cda_serial_t gCDAChangeSerial = 1;

// Called to update the change serial when something changes...
static void CDA_SomethingChanged()
{
  gCDAChangeSerial++;
}

// Returns true if a cache serial matches the global serial.
static bool CDA_CompareSerial(cda_serial_t aSerial)
{
  return gCDAChangeSerial == aSerial;
}

// A global change listener...
class CDAGlobalChangeListener
  : public iface::events::EventListener
{
public:
  CDA_IMPL_ID
  CDA_IMPL_QI1(events::EventListener)
  void add_ref() throw() {}
  void release_ref() throw() {}

  void handleEvent(iface::events::Event* aEvent)
    throw()
  {
    CDA_SomethingChanged();
  }
};

CDAGlobalChangeListener gCDAChangeListener;

CDA_RDFXMLDOMRepresentation::CDA_RDFXMLDOMRepresentation(CDA_Model* aModel)
  : mModel(aModel)
{
}

CDA_RDFXMLDOMRepresentation::~CDA_RDFXMLDOMRepresentation()
{
}

std::wstring
CDA_RDFXMLDOMRepresentation::type()
  throw(std::exception&)
{
  return L"http://www.cellml.org/RDFXML/DOM";
}

already_AddRefd<iface::dom::Document>
CDA_RDFXMLDOMRepresentation::data()
  throw(std::exception&)
{
  // Create a brand new document to store the data...
  RETURN_INTO_OBJREF(impl, iface::dom::DOMImplementation,
                     mModel->mDoc->implementation());
  RETURN_INTO_OBJREF(rdoc, iface::dom::Document,
                     impl->createDocument(RDF_NS, L"RDF", NULL));
  RETURN_INTO_OBJREF(rdocel, iface::dom::Element,
                     rdoc->documentElement());

  // Find our model's datastore...
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                     mModel->datastore->getElementsByTagNameNS(RDF_NS, L"RDF"));
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    // Now go through each child of the RDF:rdf element, and import into the
    // document.

    RETURN_INTO_OBJREF(nl2, iface::dom::NodeList,
                       n->childNodes());

    uint32_t j, m = nl2->length();
    for (j = 0; j < m; j++)
    {
      RETURN_INTO_OBJREF(n2, iface::dom::Node, nl2->item(j));
      if (n2->nodeType() == iface::dom::Node::ELEMENT_NODE)
      {
        RETURN_INTO_OBJREF(ln, iface::dom::Node, rdoc->importNode(n2, true));
        rdocel->appendChild(ln)->release_ref();
      }
    }
  }

  rdoc->add_ref();
  return rdoc.getPointer();
}

void
CDA_RDFXMLDOMRepresentation::data(iface::dom::Document* aData)
  throw(std::exception&)
{
  // Step one: Wipe out all RDF from the model...
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                     mModel->datastore->getElementsByTagNameNS(RDF_NS, L"RDF"));
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    // The item remains as 0, because we keep deleting the items after it...
    RETURN_INTO_OBJREF(el, iface::dom::Node, nl->item(0));
    RETURN_INTO_OBJREF(pel, iface::dom::Node, el->parentNode());
    pel->removeChild(el)->release_ref();
  }

  // Step two: Add the supplied RDF into the model...
  RETURN_INTO_OBJREF(de, iface::dom::Element, aData->documentElement());
  RETURN_INTO_OBJREF(ide, iface::dom::Node,
                     mModel->mDoc->importNode(de, true));
  mModel->datastore->appendChild(ide)->release_ref();
}

CDA_RDFXMLStringRepresentation::CDA_RDFXMLStringRepresentation
(
 CDA_Model* aModel
)
  : mModel(aModel)
{
}

CDA_RDFXMLStringRepresentation::~CDA_RDFXMLStringRepresentation()
{
}

std::wstring
CDA_RDFXMLStringRepresentation::type()
  throw(std::exception&)
{
  return L"http://www.cellml.org/RDFXML/string";
}

std::wstring
CDA_RDFXMLStringRepresentation::serialisedData()
  throw(std::exception&)
{
  // Create a brand new document to store the data...
  RETURN_INTO_OBJREF(impl, iface::dom::DOMImplementation,
                     mModel->mDoc->implementation());
  RETURN_INTO_OBJREF(rdoc, iface::dom::Document,
                     impl->createDocument(RDF_NS, L"RDF", NULL));
  RETURN_INTO_OBJREF(rdocel, iface::dom::Element,
                     rdoc->documentElement());

  // Find our model's datastore...
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                     mModel->datastore->getElementsByTagNameNS(RDF_NS, L"RDF"));
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    // Now go through each child of the RDF:rdf element, and import into the
    // document.

    RETURN_INTO_OBJREF(nl2, iface::dom::NodeList,
                       n->childNodes());

    uint32_t j, m = nl2->length();
    for (j = 0; j < m; j++)
    {
      RETURN_INTO_OBJREF(n2, iface::dom::Node, nl2->item(j));
      if (n2->nodeType() == iface::dom::Node::ELEMENT_NODE)
      {
        RETURN_INTO_OBJREF(ln, iface::dom::Node, rdoc->importNode(n2, true));
        rdocel->appendChild(ln)->release_ref();
      }
    }
  }

  // We need to serialise the element...
  DOMWriter dw;
  std::wstring str;
  dw.writeDocument(NULL, rdoc, str);
  return str;
}

void
CDA_RDFXMLStringRepresentation::serialisedData(const std::wstring& attr)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     mModel->mLoader->loadDocumentFromText(attr));

  // Step one: Wipe out all RDF from the model...
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                     mModel->datastore->getElementsByTagNameNS(RDF_NS, L"RDF"));
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    // The item remains as 0, because we keep deleting the items after it...
    RETURN_INTO_OBJREF(el, iface::dom::Node, nl->item(0));
    RETURN_INTO_OBJREF(pel, iface::dom::Node, el->parentNode());
    pel->removeChild(el)->release_ref();
  }

  // Step two: Add the supplied RDF into the model...
  RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
  RETURN_INTO_OBJREF(ide, iface::dom::Node,
                     mModel->mDoc->importNode(de, true));
  mModel->datastore->appendChild(ide)->release_ref();
}

#ifdef ENABLE_RDF
CDA_RDFAPIRepresentation::CDA_RDFAPIRepresentation(CDA_Model* aModel)
  : mModel(aModel)
{
}

std::wstring
CDA_RDFAPIRepresentation::type()
  throw(std::exception&)
{
  return L"http://www.cellml.org/RDF/API";
}

already_AddRefd<iface::rdf_api::DataSource>
CDA_RDFAPIRepresentation::source()
  throw(std::exception&)
{
  // Create a brand new document to store the data...
  RETURN_INTO_OBJREF(impl, iface::dom::DOMImplementation,
                     mModel->mDoc->implementation());
  RETURN_INTO_OBJREF(rdoc, iface::dom::Document,
                     impl->createDocument(RDF_NS, L"RDF", NULL));
  RETURN_INTO_OBJREF(rdocel, iface::dom::Element,
                     rdoc->documentElement());

  // Find our model's datastore...
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                     mModel->datastore->getElementsByTagNameNS(RDF_NS, L"RDF"));
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    // Now go through each child of the RDF:rdf element, and import into the
    // document.

    RETURN_INTO_OBJREF(nl2, iface::dom::NodeList,
                       n->childNodes());

    uint32_t j, m = nl2->length();
    for (j = 0; j < m; j++)
    {
      RETURN_INTO_OBJREF(n2, iface::dom::Node, nl2->item(j));
      if (n2->nodeType() == iface::dom::Node::ELEMENT_NODE)
      {
        RETURN_INTO_OBJREF(ln, iface::dom::Node, rdoc->importNode(n2, true));
        rdocel->appendChild(ln)->release_ref();
      }
    }
  }

  RETURN_INTO_OBJREF(bs, iface::rdf_api::Bootstrap, CreateRDFBootstrap());
  RETURN_INTO_OBJREF(ds, iface::rdf_api::DataSource, bs->createDataSource());

  RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, mModel->xmlBase());
  RETURN_INTO_WSTRING(base, bu->asText());
  bs->parseIntoDataSource(ds, rdocel, base.c_str());

  ds->add_ref();
  return ds.getPointer();
}

void
CDA_RDFAPIRepresentation::source(iface::rdf_api::DataSource* aSource)
  throw(std::exception&)
{
  // Step one: Wipe out all RDF from the model...
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                     mModel->datastore->getElementsByTagNameNS(RDF_NS, L"RDF"));
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    // The item remains as 0, because we keep deleting the items after it...
    RETURN_INTO_OBJREF(el, iface::dom::Node, nl->item(0));
    RETURN_INTO_OBJREF(pel, iface::dom::Node, el->parentNode());
    pel->removeChild(el)->release_ref();
  }

  // Step two: Convert the data-source into an RDF document...
  RETURN_INTO_OBJREF(bs, iface::rdf_api::Bootstrap, CreateRDFBootstrap());
  RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, mModel->xmlBase());
  RETURN_INTO_WSTRING(base, bu->asText());
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     bs->getDOMForDataSource(aSource, base.c_str()));

  // Step three: Add the RDF into the model...
  RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
  RETURN_INTO_OBJREF(ide, iface::dom::Node,
                     mModel->mDoc->importNode(de, true));
  mModel->datastore->appendChild(ide)->release_ref();
}
#endif

CDA_URI::CDA_URI(iface::dom::Element* aDatastore, const std::wstring& aNamespace, const std::wstring& aLocalname,
                 const std::wstring& aQualifiedName)
  : mDatastore(aDatastore), mNamespace(aNamespace), mLocalname(aLocalname),
    mQualifiedName(aQualifiedName)
{
}

CDA_URI::~CDA_URI()
{
}

std::wstring
CDA_URI::asText()
  throw(std::exception&)
{
  return mDatastore->getAttributeNS(mNamespace, mLocalname);
}

void
CDA_URI::asText(const std::wstring& attr)
  throw(std::exception&)
{
  mDatastore->setAttributeNS(mNamespace, mQualifiedName, attr);
}

CDA_CellMLElement::CDA_CellMLElement
(
 CDA_CellMLElement* parent,
 iface::dom::Element* idata
)
  : mParent(parent), datastore(idata),
    children(NULL)
{
  if (parent != NULL)
    parent->add_ref();
  else
  {
    // We have an unparented element, so changes to it are cache invalidating
    // events...
    DECLARE_QUERY_INTERFACE_OBJREF(targ, datastore, events::EventTarget);
    targ->addEventListener(L"DOMSubtreeModified",
                           &gCDAChangeListener, false);
  }

  datastore->add_ref();
}

CDA_CellMLElement::~CDA_CellMLElement()
{
  if (_cda_refcount != 0)
  {
    std::wstring nn(datastore->nodeName());
    printf("Warning: release_ref called too few times on %s (%S).\n",
           typeid(this).name(),
           nn.c_str());
    assert(0);
  }


  cleanupEvents();

  if (datastore != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, datastore, events::EventTarget);
    targ->removeEventListener(L"DOMSubtreeModified", &gCDAChangeListener,
                              false);
    datastore->release_ref();
  }

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

std::wstring
CDA_CellMLElement::cellmlVersion()
  throw(std::exception&)
{
  // We default to the latest known version, so that we will be backwards
  // compatible when a newer version comes out.
  const wchar_t* version = L"1.1";
  std::wstring ns = datastore->namespaceURI();
  if (ns == CELLML_1_0_NS)
    version = L"1.0";
  return version;
}

std::wstring
CDA_CellMLElement::cmetaId()
  throw(std::exception&)
{
  return datastore->getAttributeNS(CMETA_NS, L"id");
}

void
CDA_CellMLElement::cmetaId(const std::wstring& attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(CMETA_NS, L"id", attr);
}

already_AddRefd<iface::cellml_api::RDFRepresentation>
CDA_Model::getRDFRepresentation(const std::wstring& type)
  throw(std::exception&)
{
  if (type == L"http://www.cellml.org/RDFXML/DOM")
    return new CDA_RDFXMLDOMRepresentation(this);
  else if (type == L"http://www.cellml.org/RDFXML/string")
    return new CDA_RDFXMLStringRepresentation(this);
#ifdef ENABLE_RDF
  else if (type == L"http://www.cellml.org/RDF/API")
    return new CDA_RDFAPIRepresentation(this);
#endif

  throw iface::cellml_api::CellMLException(L"Unknown RDF representation type requested.");
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_CellMLElement::findCellMLElementFromDOMElement(iface::dom::Element* aEl)
  throw(std::exception&)
{
  if (mParent != NULL)
    return mParent->findCellMLElementFromDOMElement(aEl);

  if (children == NULL)
    children = new CDA_CellMLElementSet(this, datastore);

  return children->searchDescendents(aEl);
}

already_AddRefd<iface::cellml_api::ExtensionElementList>
CDA_CellMLElement::extensionElements()
  throw(std::exception&)
{
  return new CDA_ExtensionElementList(datastore);
}

void
CDA_CellMLElement::insertExtensionElementAfter
(
 iface::dom::Element* marker,
 iface::dom::Element* newEl
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLElement::appendExtensionElement(iface::dom::Element* x)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->appendChild(x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLElement::prependExtensionElement(iface::dom::Element* x)
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLElement::removeExtensionElement(iface::dom::Element* x)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->removeChild(x);
    n->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLElement::replaceExtensionElement
(
 iface::dom::Element* x,
 iface::dom::Element* y
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
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
    uint32_t i, l = nl->length();
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::CellMLElementSet>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
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
      throw iface::cellml_api::CellMLException(L"DOM exception caught.");

    // Does it already have a parent?
    if (el->mParent != NULL)
      throw iface::cellml_api::CellMLException(L"DOM exception caught.");

    // Adopt the node...
    el->mParent = this;
    // We adopted the node, so we also take on its references...
    uint32_t i, rc = el->_cda_refcount;
    for (i = 0; i < rc; i++)
      add_ref();

    // TODO: Perhaps we should also check the datastore ownerDocument matches?

    // Make a local CellML wrapping, if it doesn't exist already...
    if (children == NULL)
    {
      children = new CDA_CellMLElementSet(this, datastore);
    }

    // Add to our local CellML wrapping...
    children->addChildToWrapper(el);
    if (el->children)
      el->children->dumpRootCaches();

    // Add the element's backing datastore to our datastore...
    datastore->appendChild(el->datastore)->release_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
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
      throw iface::cellml_api::CellMLException(L"Element not from this implementation.");

    // Are we the correct parent?
    if (el->mParent != this)
      throw iface::cellml_api::CellMLException(L"Element from a different parent.");

    // Remove the element's backing datastore to our datastore...
    datastore->removeChild(el->datastore)->release_ref();

    // Remove from our local CellML wrapping, if it exists...
    if (children)
      children->removeChildFromWrapper(el);

    el->removeLinkFromHereToParent();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLElement::removeLinkFromHereToParent()
{
  uint32_t i, rc = _cda_refcount;
  for (i = 0; i < rc; i++)
    mParent->release_ref();

  if (_cda_refcount == 0)
    delete this;
  else
    mParent = NULL;
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
      throw iface::cellml_api::CellMLException(L"Element not from this implementation.");

    CDA_CellMLElement* elNew = dynamic_cast<CDA_CellMLElement*>(y);
    if (elNew == NULL)
      throw iface::cellml_api::CellMLException(L"Element ");

    // Are we the correct parent for the old element?
    if (elOld->mParent != this)
      throw iface::cellml_api::CellMLException(L"Element has a different parent.");

    // Does the new element already have a parent?
    if (elNew->mParent != NULL)
      throw iface::cellml_api::CellMLException(L"New element already has a parent.");

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
    uint32_t i, rc = elOld->_cda_refcount;
    // We don't need to be kept around for its references any more.
    for (i = 0; i < rc; i++)
      release_ref();

    if (rc == 0)
      delete elOld;

    // The new node now belongs to us...
    elNew->mParent = this;
    rc = elNew->_cda_refcount;
    for (i = 0; i < rc; i++)
      add_ref();
  }
  catch (iface::dom::DOMException& e)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLElement::removeByName
(
 const std::wstring& type,
 const std::wstring& name
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(elS, iface::cellml_api::CellMLElementSet, childElements());
  RETURN_INTO_OBJREF(it, CDA_CellMLElementIterator,
                     unsafe_dynamic_cast<CDA_CellMLElementIterator*>(elS->iterate().getPointer()));
  while (true)
  {
    RETURN_INTO_OBJREF(n, iface::cellml_api::CellMLElement,
                       it->next());
    if (n == NULL)
      break;

    CDA_NamedCellMLElement* el =
      dynamic_cast<CDA_NamedCellMLElement*>(n.getPointer());
    if (el != NULL)
    {
      RETURN_INTO_WSTRING(tn, el->datastore->localName());
      if (tn == type)
      {
        RETURN_INTO_WSTRING(elname, el->name());
        if (elname == name)
        {
          removeElement(n);
          break;
        }
      }
    }
  }
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_CellMLElement::parentElement()
  throw(std::exception&)
{
  iface::cellml_api::CellMLElement* el =
    dynamic_cast<iface::cellml_api::CellMLElement*>(mParent);
  if (el == NULL)
  {
    return NULL;
  }
  el->add_ref();
  return el;
}

already_AddRefd<iface::cellml_api::Model>
CDA_CellMLElement::modelElement()
  throw(std::exception&)
{
  for (ObjRef<iface::cellml_api::CellMLElement> cand(this); cand != NULL;
       cand = cand->parentElement())
  {
    ObjRef<iface::cellml_api::Model> m(QueryInterface(cand));
    if (m != NULL)
    {
      m->add_ref();
      return m.getPointer();
    }
  }
  return NULL;
}

void
CDA_CellMLElement::setUserData
(
 const std::wstring& key,
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

already_AddRefd<iface::cellml_api::UserData>
CDA_CellMLElement::getUserData(const std::wstring& key)
  throw(std::exception&)
{
  std::map<std::wstring,iface::cellml_api::UserData*>::iterator i;
  i = userData.find(key);
  if (i != userData.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }
  throw iface::cellml_api::CellMLException(L"User data not found.");
}

already_AddRefd<iface::cellml_api::UserData>
CDA_CellMLElement::getUserDataWithDefault(const std::wstring& key, iface::cellml_api::UserData* defval)
  throw(std::exception&)
{
  std::map<std::wstring,iface::cellml_api::UserData*>::iterator i;
  i = userData.find(key);
  if (i != userData.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  if (defval != NULL)
    defval->add_ref();
  return defval;
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_CellMLElement::clone(bool aDeep)
  throw(std::exception&)
{
  // Clone the underlying node...
  RETURN_INTO_OBJREF(cn, iface::dom::Node, datastore->cloneNode(aDeep));
  DECLARE_QUERY_INTERFACE_OBJREF(cel, cn, dom::Element);
  if (cel == NULL)
    return NULL;

  return WrapCellMLElement(NULL, cel);
}

std::wstring
CDA_CellMLElement::getExtensionAttributeNS(const std::wstring& aNS, const std::wstring& aLocalName)
  throw(std::exception&)
{
  // XXX should we check aNS is an extension namespace?
  return datastore->getAttributeNS(aNS, aLocalName);
}

void
CDA_CellMLElement::setExtensionAttributeNS(const std::wstring& aNS, const std::wstring& aQualifiedName,
                                           const std::wstring& aValue)
  throw(std::exception&)
{
  datastore->setAttributeNS(aNS, aQualifiedName, aValue);
}

void
CDA_CellMLElement::removeExtensionAttributeNS(const std::wstring& aNS, const std::wstring& aLocalName)
  throw(std::exception&)
{
  datastore->removeAttributeNS(aNS, aLocalName);
}

already_AddRefd<iface::cellml_api::ExtensionAttributeSet>
CDA_CellMLElement::extensionAttributes()
  throw(std::exception&)
{
  return new CDA_ExtensionAttributeSet(datastore);
}

CDA_ExtensionAttributeSet::CDA_ExtensionAttributeSet(iface::dom::Element* aDataStore)
  : mDataStore(aDataStore)
{
}

already_AddRefd<iface::cellml_api::ExtensionAttributeIterator>
CDA_ExtensionAttributeSet::iterate()
  throw(std::exception&)
{
  return new CDA_ExtensionAttributeIterator(mDataStore);
}

CDA_ExtensionAttributeIterator::CDA_ExtensionAttributeIterator(iface::dom::Element* aDataStore)
  : mNodeMap(already_AddRefd<iface::dom::NamedNodeMap>(aDataStore->attributes())),
    mPrevIndex(-1), mDone(false), eventListener(this)
{
}

CDA_ExtensionAttributeIterator::~CDA_ExtensionAttributeIterator()
{
  if (mPrev != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrev, events::EventTarget);
    targ->removeEventListener(L"DOMNodeRemoved", &eventListener, false);
  }
}

already_AddRefd<iface::dom::Attr>
CDA_ExtensionAttributeIterator::nextAttribute()
  throw(std::exception&)
{
  while (true)
  {
    RETURN_INTO_OBJREF(attr, iface::dom::Attr,
                       nextAttributeInternal());
    if (attr == NULL)
      return NULL;

    RETURN_INTO_WSTRING(nsURI, attr->namespaceURI());
    if (nsURI == L"" || nsURI == CELLML_1_0_NS || nsURI == CELLML_1_1_NS ||
        nsURI == CMETA_NS || nsURI == MATHML_NS || nsURI == RDF_NS ||
        nsURI == L"http://www.w3.org/XML/1998/namespace")
      continue;
    attr->add_ref();
    return attr.getPointer();
  }
}

already_AddRefd<iface::dom::Attr>
CDA_ExtensionAttributeIterator::nextAttributeInternal()
  throw(std::exception&)
{
  if (mDone)
    return NULL;

  if (mPrev == NULL)
  {
    mPrev = already_AddRefd<iface::dom::Node>(mNodeMap->item(0));
    if (mPrev == NULL)
      return NULL;
    mPrevIndex = 0;

    {
      DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrev, events::EventTarget);
      targ->addEventListener(L"DOMNodeRemoved", &eventListener, false);
    }

    DECLARE_QUERY_INTERFACE(ret, mPrev, dom::Attr);
    return ret;
  }

  {
    ObjRef<iface::dom::Node> curPrev;
    if (mPrevIndex != -1)
    {
      curPrev = already_AddRefd<iface::dom::Node>(mNodeMap->item(mPrevIndex));
      if (CDA_objcmp(curPrev, mPrev))
        mPrevIndex = -1;
    }

    if (mPrevIndex == -1)
    {
      for (mPrevIndex = 0;; mPrevIndex++)
      {
        curPrev = already_AddRefd<iface::dom::Node>(mNodeMap->item(mPrevIndex));
        assert(curPrev != NULL); // Node should be guaranteed to be there.

        if (!CDA_objcmp(curPrev, mPrev))
          break;
      }
    }
  }

  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrev, events::EventTarget);
    targ->removeEventListener(L"DOMNodeRemoved", &eventListener, false);
  }

  // We now have a guaranteed up to date mPrevIndex.
  mPrevIndex++;
  mPrev = already_AddRefd<iface::dom::Node>(mNodeMap->item(mPrevIndex));

  if (mPrev == NULL)
  {
    mDone = true;
    return NULL;
  }

  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrev, events::EventTarget);
    targ->addEventListener(L"DOMNodeRemoved", &eventListener, false);
  }

  DECLARE_QUERY_INTERFACE(ret, mPrev, dom::Attr);
  return ret;
}

void
CDA_ExtensionAttributeIterator::ExtensionAttributeEventListener::handleEvent
(
 iface::events::Event* evt
)
  throw(std::exception&)
{

  try
  {
    if (evt->eventPhase() != iface::events::Event::AT_TARGET)
      return;
    RETURN_INTO_OBJREF(t, iface::events::EventTarget, evt->target());

    if (mParent->mPrev == NULL || CDA_objcmp(t, mParent->mPrev))
      return;

    {
      ObjRef<iface::dom::Node> curPrev;
      if (mParent->mPrevIndex != -1)
      {
        curPrev = already_AddRefd<iface::dom::Node>(mParent->mNodeMap->item(mParent->mPrevIndex));
        if (CDA_objcmp(curPrev, mParent->mPrev))
          mParent->mPrevIndex = -1;
      }

      if (mParent->mPrevIndex == -1)
      {
        for (mParent->mPrevIndex = 0;; mParent->mPrevIndex++)
        {
          curPrev = already_AddRefd<iface::dom::Node>(mParent->mNodeMap->item(mParent->mPrevIndex));
          assert(curPrev != NULL); // Node should be guaranteed to be there.

          if (!CDA_objcmp(curPrev, mParent->mPrev))
            break;
        }
      }
    }

    {
      DECLARE_QUERY_INTERFACE_OBJREF(targ, mParent->mPrev, events::EventTarget);
      targ->removeEventListener(L"DOMNodeRemoved", this, false);
    }

    // We now have a guaranteed up to date mPrevIndex.
    mParent->mPrevIndex--;
    if (mParent->mPrevIndex == -1)
      mParent->mPrev = NULL;
    else
      mParent->mPrev = already_AddRefd<iface::dom::Node>(mParent->mNodeMap->item(mParent->mPrevIndex));

    if (mParent->mPrev != NULL)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(targ, mParent->mPrev, events::EventTarget);
      targ->addEventListener(L"DOMNodeRemoved", this, false);
    }
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

static void
recursivelyCloneFromTo(iface::cellml_api::Model* aFrom,
                       iface::cellml_api::Model* aTo)
{
  RETURN_INTO_OBJREF(cisFrom, iface::cellml_api::CellMLImportSet, aFrom->imports());
  RETURN_INTO_OBJREF(ciiFrom, iface::cellml_api::CellMLImportIterator, cisFrom->iterateImports());
  RETURN_INTO_OBJREF(cisTo, iface::cellml_api::CellMLImportSet, aTo->imports());
  RETURN_INTO_OBJREF(ciiTo, iface::cellml_api::CellMLImportIterator, cisTo->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(ciFrom, iface::cellml_api::CellMLImport, ciiFrom->nextImport());
    if (ciFrom == NULL)
      break;
    RETURN_INTO_OBJREF(ciTo, iface::cellml_api::CellMLImport, ciiTo->nextImport());

    RETURN_INTO_OBJREF(mFrom, iface::cellml_api::Model, ciFrom->importedModel());
    {
      RETURN_INTO_WSTRING(st, mFrom->serialisedText());
      ciTo->instantiateFromText(st.c_str());
    }
    RETURN_INTO_OBJREF(mTo, iface::cellml_api::Model, ciTo->importedModel());
    recursivelyCloneFromTo(mFrom, mTo);
  }
}

already_AddRefd<iface::cellml_api::Model>
CDA_Model::cloneAcrossImports()
  throw(std::exception&)
{
  fullyInstantiateImports();

  RETURN_INTO_OBJREF(cloneModelE, iface::cellml_api::CellMLElement, clone(true));
  DECLARE_QUERY_INTERFACE_OBJREF(cloneModel, cloneModelE, cellml_api::Model);
  recursivelyCloneFromTo(this, cloneModel);

  cloneModel->add_ref();
  return cloneModel.getPointer();
}

already_AddRefd<iface::dom::Element>
CDA_CellMLElement::domElement()
  throw(std::exception&)
{
  if (datastore != NULL)
    datastore->add_ref();

  return datastore;
}

CDA_NamedCellMLElement::CDA_NamedCellMLElement
(
 CDA_CellMLElement* parent,
 iface::dom::Element* idata
)
  : CDA_CellMLElement(parent, idata)
{
}

CDA_NamedCellMLElement::~CDA_NamedCellMLElement()
{
}

std::wstring
CDA_NamedCellMLElement::name()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"name");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught");
  }
}

void
CDA_NamedCellMLElement::name
(
 const std::wstring& attr
)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"name", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

CDA_Model::CDA_Model(iface::cellml_api::DOMURLLoader* aLoader,
                     iface::dom::Document* aDoc,
                     iface::dom::Element* modelElement)
  : CDA_CellMLElement(NULL, modelElement),
    CDA_NamedCellMLElement(NULL, modelElement),
    mLoader(aLoader), mDoc(aDoc), mNextUniqueIdentifier(1),
    mConnectionSet(NULL), mGroupSet(NULL), mImportSet(NULL),
    mComponentSet(NULL), mAllComponents(NULL), mModelComponents(NULL),
    mLocalUnits(NULL), mAllUnits(NULL), mModelUnits(NULL)
{
  mDoc->add_ref();
}

CDA_Model::~CDA_Model()
{
  // XXX this code should run in ~CDA_CellMLElement, but libxml doesn't work
  // after the document has been destroyed.
  //if (datastore != NULL)
  //{
  //  datastore->release_ref();
  //  datastore = NULL;
  // }
  //if (children != NULL)
  //{
  //  delete children;
  //  children = NULL;
  //}

  if (mConnectionSet != NULL)
    delete mConnectionSet;

  if (mGroupSet != NULL)
    delete mGroupSet;

  if (mImportSet != NULL)
    delete mImportSet;

  if (mComponentSet != NULL)
    delete mComponentSet;

  if (mAllComponents != NULL)
    delete mAllComponents;

  if (mModelComponents != NULL)
    delete mModelComponents;

  if (mLocalUnits != NULL)
    delete mLocalUnits;

  if (mAllUnits != NULL)
    delete mAllUnits;

  if (mModelUnits != NULL)
    delete mModelUnits;

  mDoc->release_ref();
}

already_AddRefd<iface::cellml_api::Model>
CDA_Model::getAlternateVersion(const std::wstring& cellmlVersion)
  throw(std::exception&)
{
  try
  {
    const wchar_t* new_namespace;
    if (cellmlVersion == L"1.0")
      new_namespace = CELLML_1_0_NS;
    else if (cellmlVersion == L"1.1")
      new_namespace = CELLML_1_1_NS;
    else
      throw iface::cellml_api::CellMLException(L"Invalid version of CellML passed to getAlternateVersion");

    RETURN_INTO_OBJREF(di, iface::dom::DOMImplementation,
                       mDoc->implementation());
    RETURN_INTO_OBJREF(newDoc, iface::dom::Document,
                       di->createDocument(new_namespace, L"model", NULL));

    // Now copy the current document into the old one...
    RecursivelyChangeVersionCopy(new_namespace, newDoc, mDoc, newDoc);

    // newDoc needs a CellML wrapper...
    RETURN_INTO_OBJREF(de, iface::dom::Element, newDoc->documentElement());
    return new CDA_Model(mLoader, newDoc, de);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_Model::RecursivelyChangeVersionCopy
(
 const std::wstring& aNewNamespace,
 iface::dom::Node* aCopy,
 iface::dom::Node* aOriginal,
 iface::dom::Document* aNewDoc
)
  throw(std::exception&)
{
  ObjRef<iface::dom::NamedNodeMap> nnm(already_AddRefd<iface::dom::NamedNodeMap>
                                       (aOriginal->attributes()));
  if (nnm)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(aCopyEl, aCopy, dom::Element);

    uint32_t i, l = nnm->length();
    for (i = 0; i < l; i++)
    {
      RETURN_INTO_OBJREF(origItemN, iface::dom::Node, nnm->item(i));
      DECLARE_QUERY_INTERFACE_OBJREF(origItem, origItemN, dom::Attr);
      RETURN_INTO_WSTRING(ln, origItem->nodeName());
      RETURN_INTO_WSTRING(nsuri, origItem->namespaceURI());
      RETURN_INTO_WSTRING(v, origItem->nodeValue());
      if (nsuri == CELLML_1_0_NS || nsuri == CELLML_1_1_NS)
        nsuri = aNewNamespace;

      aCopyEl->setAttributeNS(nsuri, ln, v);
    }
  }

  ObjRef<iface::dom::NodeList> nl(aOriginal->childNodes());
  uint32_t i, l = nl->length();
  for (i = 0; i < l; i++)
  {
    ObjRef<iface::dom::Node> origItem(already_AddRefd<iface::dom::Node>
                                      (nl->item(i)));
    ObjRef<iface::dom::Node> newItem;
    // See if we need to change the namespace...
    std::wstring nsURI = origItem->namespaceURI();
    if (nsURI == aNewNamespace ||
        (nsURI != CELLML_1_0_NS &&
         nsURI != CELLML_1_1_NS))
    {
      if (nsURI == L"http://www.w3.org/2000/xmlns/")
        continue;
      newItem = aNewDoc->importNode(origItem, false);
    }
    else
    {
      // See what type of node it is...
      switch (origItem->nodeType())
      {
      case iface::dom::Node::ELEMENT_NODE:
        {
          // Ignore the document element, it is already done
          if (aOriginal == mDoc)
          {
            // except we have to recurse on it...
            RETURN_INTO_OBJREF(n, iface::dom::Node, aNewDoc->documentElement());
            RecursivelyChangeVersionCopy(aNewNamespace, n, origItem, aNewDoc);
            continue;
          }
	  std::wstring ln = origItem->localName();
          newItem = already_AddRefd<iface::dom::Node>
            (aNewDoc->createElementNS(aNewNamespace, ln));
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
        break;

      case iface::dom::Node::ATTRIBUTE_NODE:
        continue;

      default:
        throw iface::cellml_api::CellMLException(L"Unknown type of DOM node encountered.");
      }
    }
    aCopy->appendChild(newItem)->release_ref();
    RecursivelyChangeVersionCopy(aNewNamespace, newItem, origItem, aNewDoc);
  }
}

already_AddRefd<iface::cellml_api::GroupSet>
CDA_Model::groups()
  throw(std::exception&)
{
  if (mGroupSet != NULL)
  {
    mGroupSet->add_ref();
    return mGroupSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mGroupSet = new CDA_GroupSet(this, allChildren);
  return mGroupSet;
}

already_AddRefd<iface::cellml_api::CellMLImportSet>
CDA_Model::imports()
  throw(std::exception&)
{
  if (mImportSet != NULL)
  {
    mImportSet->add_ref();
    return mImportSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mImportSet = new CDA_CellMLImportSet(this, allChildren);
  return mImportSet;
}

already_AddRefd<iface::cellml_api::URI>
CDA_Model::xmlBase()
  throw(std::exception&)
{
  return new CDA_URI(datastore, L"http://www.w3.org/XML/1998/namespace", L"base", L"xml:base");
}

// Deprecated
already_AddRefd<iface::cellml_api::URI>
CDA_Model::base_uri()
  throw(std::exception&)
{
  return xmlBase();
}

void
CDA_Model::clearXMLBase()
  throw(std::exception&)
{
  datastore->removeAttributeNS(L"http://www.w3.org/XML/1998/namespace", L"base");
}

already_AddRefd<iface::cellml_api::UnitsSet>
CDA_Model::localUnits()
  throw(std::exception&)
{
  if (mLocalUnits != NULL)
  {
    mLocalUnits->add_ref();
    return mLocalUnits;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mLocalUnits = new CDA_UnitsSet(this, allChildren /*, false */);
  return mLocalUnits;
}

already_AddRefd<iface::cellml_api::UnitsSet>
CDA_Model::modelUnits()
  throw(std::exception&)
{
  if (mModelUnits != NULL)
  {
    mModelUnits->add_ref();
    return mModelUnits;
  }

  // Call localUnits, as that is a subset of our work...
  RETURN_INTO_OBJREF(lu, iface::cellml_api::UnitsSet, localUnits());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());

  // Next construct the special set...
  mModelUnits = new CDA_AllUnitsSet(this, lu, imp, false);
  return mModelUnits;
}

already_AddRefd<iface::cellml_api::UnitsSet>
CDA_Model::allUnits()
  throw(std::exception&)
{
  if (mAllUnits != NULL)
  {
    mAllUnits->add_ref();
    return mAllUnits;
  }

  // Call localUnits, as that is a subset of our work...
  RETURN_INTO_OBJREF(lu, iface::cellml_api::UnitsSet, localUnits());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());

  // Next construct the special set...
  mAllUnits = new CDA_AllUnitsSet(this, lu, imp, true);
  return mAllUnits;
}

already_AddRefd<iface::cellml_api::CellMLComponentSet>
CDA_Model::localComponents()
  throw(std::exception&)
{
  if (mComponentSet != NULL)
  {
    mComponentSet->add_ref();
    return mComponentSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mComponentSet = new CDA_CellMLComponentSet(this, allChildren);
  return mComponentSet;
}

already_AddRefd<iface::cellml_api::CellMLComponentSet>
CDA_Model::modelComponents()
  throw(std::exception&)
{
  if (mModelComponents != NULL)
  {
    mModelComponents->add_ref();
    return mModelComponents;
  }

  // Call localComponents, as that is a subset of our work...
  RETURN_INTO_OBJREF(lc, iface::cellml_api::CellMLComponentSet, localComponents());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());

  // Next construct the special set...
  mModelComponents = new CDA_AllComponentSet(this, lc, imp, false);
  return mModelComponents;
}

already_AddRefd<iface::cellml_api::CellMLComponentSet>
CDA_Model::allComponents()
  throw(std::exception&)
{
  if (mAllComponents != NULL)
  {
    mAllComponents->add_ref();
    return mAllComponents;
  }


  // Call localComponents, as that is a subset of our work...
  RETURN_INTO_OBJREF(lc, iface::cellml_api::CellMLComponentSet, localComponents());
  // Next fetch the list of imports...
  RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImportSet, imports());

  // Next construct the special set...
  mAllComponents = new CDA_AllComponentSet(this, lc, imp, true);
  return mAllComponents;
}

already_AddRefd<iface::cellml_api::ConnectionSet>
CDA_Model::connections()
  throw(std::exception&)
{
  if (mConnectionSet != NULL)
  {
    mConnectionSet->add_ref();
    return mConnectionSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mConnectionSet = new CDA_ConnectionSet(this, allChildren);
  return mConnectionSet;
}

already_AddRefd<iface::cellml_api::GroupSet>
CDA_Model::findGroupsWithRelationshipRefName
(
 const std::wstring& name
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  return new CDA_GroupSet(allChildren, name);
}

void
CDA_Model::fullyInstantiateImports()
  throw(std::exception&)
{
  std::list<iface::cellml_api::CellMLImport*> importQueue;

  // Go through the list of imports and add them to the queue...
  RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet, imports());
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                     imps->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       impi->nextImport());
    if (imp == NULL)
      break;
    imp->add_ref();
    importQueue.push_back(imp);
  }

  while (!importQueue.empty())
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       importQueue.front());
    importQueue.pop_front();

    if (!imp->wasInstantiated())
      imp->instantiate();

    // Now that the model is loaded, add its children to the importQueue...
    CDA_Model* m =
      unsafe_dynamic_cast<CDA_Model*>(unsafe_dynamic_cast<CDA_CellMLImport*>
                                      (imp.getPointer())->mImportedModel);
    if (m == NULL)
      continue;

    // Go through the list of imports and add them to the queue...
    imps = already_AddRefd<iface::cellml_api::CellMLImportSet>(m->imports());
    impi = already_AddRefd<iface::cellml_api::CellMLImportIterator>
      (imps->iterateImports());
    while (true)
    {
      RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                         impi->nextImport());
      if (imp == NULL)
        break;
      imp->add_ref();
      importQueue.push_back(imp);
    }
  }
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_Model::clone(bool aDeep)
  throw(std::exception&)
{
  // Make a new document...
  RETURN_INTO_OBJREF(di, iface::dom::DOMImplementation,
                     mDoc->implementation());
  RETURN_INTO_WSTRING(new_namespace, datastore->namespaceURI());
  RETURN_INTO_OBJREF(newDoc, iface::dom::Document,
                     di->createDocument(new_namespace.c_str(), L"model", NULL));

  // Clone the underlying node...
  RETURN_INTO_OBJREF(jde, iface::dom::Element, newDoc->documentElement());
  newDoc->removeChild(jde)->release_ref();
  RETURN_INTO_OBJREF(cn, iface::dom::Node,
                     newDoc->importNode(datastore, aDeep));
  newDoc->appendChild(cn)->release_ref();

  DECLARE_QUERY_INTERFACE_OBJREF(cel, cn, dom::Element);
  if (cel == NULL)
    return NULL;

  return new CDA_Model(mLoader, newDoc, cel);
}

class CDA_Model_AsyncInstantiate_CommonState
  : public iface::XPCOM::IObject
{
public:
  CDA_Model_AsyncInstantiate_CommonState
  (
   iface::cellml_api::ImportInstantiationListener* aListener
  )
    : mListener(aListener)
  {
    mListener->add_ref();
  }

  virtual ~CDA_Model_AsyncInstantiate_CommonState()
  {
    mListener->release_ref();
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI0;
  CDA_IMPL_ID;

  bool mInRecursion, mContainsStale;
  uint32_t mActiveInstantiationCount;
  std::list<iface::cellml_api::CellMLImport*> importQueue;

  iface::cellml_api::ImportInstantiationListener* mListener;
};


class CDA_Model_AsyncInstantiateDoneListener
  : public iface::cellml_api::ImportInstantiationListener
{
public:
  CDA_Model_AsyncInstantiateDoneListener
  (
   CDA_Model_AsyncInstantiate_CommonState* aModel,
   iface::cellml_api::CellMLImport* aImport
  )
    : mModel(aModel), mImport(aImport)
  {
    mModel->add_ref();
    mImport->add_ref();
  }

  virtual ~CDA_Model_AsyncInstantiateDoneListener()
  {
    mModel->release_ref();
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::ImportInstantiationListener);
  CDA_IMPL_ID;

  void loadCompleted(bool aWasStale)
    throw(std::exception&)
  {
    mModel->mActiveInstantiationCount--;
    if (aWasStale)
    {
      mModel->mContainsStale = true;
      return;
    }

    // Now that the model is loaded, add its children to the importQueue...
    iface::cellml_api::Model* m =
      dynamic_cast<CDA_CellMLImport*>(mImport)->mImportedModel;
    if (m == NULL)
      return;

    // Go through the list of imports and add them to the queue...
    RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet, m->imports());
    RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                       imps->iterateImports());
    while (true)
    {
      RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                         impi->nextImport());
      if (imp == NULL)
        break;
      imp->add_ref();
      mModel->importQueue.push_back(imp);
    }

    // If another stack frame is already processing the importQueue, then there
    // is no use doing it again here and wasting stack space on top of heap
    // space for the queue...
    if (mModel->mInRecursion)
      return;

    mModel->mInRecursion = true;
    while (!mModel->importQueue.empty())
    {
      RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                         mModel->importQueue.front());
      mModel->importQueue.pop_front();
      try
      {
        RETURN_INTO_OBJREF(aidl,
                           iface::cellml_api::ImportInstantiationListener,
                           new CDA_Model_AsyncInstantiateDoneListener
                           (mModel, imp));
        mModel->mActiveInstantiationCount++;
        imp->asyncInstantiate(aidl);
      }
      catch (iface::cellml_api::CellMLException& ce)
      {
        mModel->mInRecursion = false;
        throw ce;
      }
    }
    mModel->mInRecursion = false;

    if (mModel->mActiveInstantiationCount != 0)
      return;

    // We are done...
    mModel->mListener->loadCompleted(mModel->mContainsStale);
  }

private:
  CDA_Model_AsyncInstantiate_CommonState* mModel;
  iface::cellml_api::CellMLImport* mImport;
};

void
CDA_Model::asyncFullyInstantiateImports
(iface::cellml_api::ImportInstantiationListener* listener)
  throw(std::exception&)
{
  // Create the common state...
  RETURN_INTO_OBJREF(aics, CDA_Model_AsyncInstantiate_CommonState,
                     new CDA_Model_AsyncInstantiate_CommonState(listener));

  // Go through the list of imports and add them to the queue...
  RETURN_INTO_OBJREF(imps, iface::cellml_api::CellMLImportSet, imports());
  RETURN_INTO_OBJREF(impi, iface::cellml_api::CellMLImportIterator,
                     imps->iterateImports());
  while (true)
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       impi->nextImport());
    if (imp == NULL)
      break;
    imp->add_ref();
    aics->importQueue.push_back(imp);
  }

  aics->mInRecursion = true;
  aics->mActiveInstantiationCount = 0;
  aics->mContainsStale = false;

  while (!aics->importQueue.empty())
  {
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLImport,
                       aics->importQueue.front());
    aics->importQueue.pop_front();

    try
    {
      RETURN_INTO_OBJREF(aidl, iface::cellml_api::ImportInstantiationListener,
                         new CDA_Model_AsyncInstantiateDoneListener(aics, imp));
      aics->mActiveInstantiationCount++;

      if (!imp->wasInstantiated())
        imp->asyncInstantiate(aidl);
    }
    catch (iface::cellml_api::CellMLException& ce)
    {
      aics->mInRecursion = false;
      throw ce;
    }
  }
  aics->mInRecursion = false;

  if (aics->mActiveInstantiationCount != 0)
    return;

  // We are done...
  listener->loadCompleted(aics->mContainsStale);
}

already_AddRefd<iface::cellml_api::CellMLComponent>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::ImportComponent>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught");
  }
}

already_AddRefd<iface::cellml_api::Units>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::ImportUnits>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::Unit>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::CellMLImport>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::CellMLVariable>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::ComponentRef>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::RelationshipRef>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::Group>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::Connection>
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
    CDA_Connection* c = new CDA_Connection(NULL, newNode);
    RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents, createMapComponents());
    c->addElement(mc);
    return c;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::MapComponents>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::MapVariables>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::Reaction>
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::VariableRef>
CDA_Model::createVariableRef()
  throw(std::exception&)
{
  try
  {
    // Get our namespace, and use it for the new node.
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"variable_ref"));
    return new CDA_VariableRef(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::Role>
CDA_Model::createRole()
  throw(std::exception&)
{
  // Get our namespace, and use it for the new node.
  try
  {
    RETURN_INTO_WSTRING(myNamespace, datastore->namespaceURI());
    RETURN_INTO_OBJREF(newNode, iface::dom::Element,
                       mDoc->createElementNS(myNamespace.c_str(),
                                             L"role"));
    return new CDA_Role(NULL, newNode);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

uint32_t
CDA_Model::assignUniqueIdentifier()
{
  return mNextUniqueIdentifier++;
}

already_AddRefd<iface::mathml_dom::MathMLMathElement>
CDA_Model::createMathElement()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(el, iface::dom::Element,
                     mDoc->createElementNS(MATHML_NS, L"math"));
  DECLARE_QUERY_INTERFACE(mel, el, mathml_dom::MathMLMathElement);
  return mel;
}

already_AddRefd<iface::dom::Element>
CDA_Model::createExtensionElement
(
 const std::wstring& namespaceURI,
 const std::wstring& qualifiedName
)
  throw(std::exception&)
{
  return mDoc->createElementNS(namespaceURI, qualifiedName);
}

std::wstring
CDA_Model::serialisedText()
  throw (std::exception&)
{
  DOMWriter dw;
  std::wstring str;
  dw.writeDocument(NULL, mDoc, str);
  return str;
}

CDA_MathContainer::CDA_MathContainer(CDA_CellMLElement* parent,
                                     iface::dom::Element* modelElement)
  : CDA_CellMLElement(parent, modelElement)
{
}

already_AddRefd<iface::cellml_api::MathList>
CDA_MathContainer::math()
  throw(std::exception&)
{
  try
  {
    return new CDA_MathList(datastore);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_MathContainer::addMath
(
 iface::mathml_dom::MathMLElement* x
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_MathContainer::removeMath
(
 iface::mathml_dom::MathMLElement* x
)
  throw(std::exception&)
{
  try
  {
    iface::dom::Node* n = datastore->removeChild(x);
    if (n)
      n->release_ref();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_MathContainer::replaceMath
(
 iface::mathml_dom::MathMLElement* x,
 iface::mathml_dom::MathMLElement* y
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_MathContainer::clearMath()
  throw()
{
  ObjRef<iface::dom::NodeList> nl(already_AddRefd<iface::dom::NodeList>
                                  (datastore->childNodes()));
  // Go through and find all math nodes...
  uint32_t i, l = nl->length();
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

CDA_CellMLComponent::~CDA_CellMLComponent()
{
  if (mReactionSet != NULL)
    delete mReactionSet;
  if (mVariableSet != NULL)
    delete mVariableSet;
  if (mConnectionSet != NULL)
    delete mConnectionSet;
  if (mUnitsSet != NULL)
    delete mUnitsSet;
}

already_AddRefd<iface::cellml_api::CellMLVariableSet>
CDA_CellMLComponent::variables()
  throw(std::exception&)
{
  if (mVariableSet != NULL)
  {
    mVariableSet->add_ref();
    return mVariableSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mVariableSet = new CDA_CellMLVariableSet(this, allChildren);
  return mVariableSet;
}

already_AddRefd<iface::cellml_api::UnitsSet>
CDA_CellMLComponent::units()
  throw(std::exception&)
{
  if (mUnitsSet != NULL)
  {
    mUnitsSet->add_ref();
    return mUnitsSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mUnitsSet = new CDA_UnitsSet(this, allChildren);
  return mUnitsSet;
}

already_AddRefd<iface::cellml_api::ConnectionSet>
CDA_CellMLComponent::connections()
  throw(std::exception&)
{
  if (mConnectionSet != NULL)
  {
    mConnectionSet->add_ref();
    return mConnectionSet;
  }
  mConnectionSet = new CDA_ComponentConnectionSet(this, this);
  return mConnectionSet;
}

static CDA_ComponentRef*
RecursivelySearchCR(CDA_ComponentRef *cr, const std::wstring& cn)
{
  std::wstring cn2 = cr->componentName();
  bool match = cn == cn2;

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

already_AddRefd<iface::cellml_api::CellMLComponent>
CDA_CellMLComponentGroupMixin::encapsulationParent()
  throw(std::exception&)
{
  // Find the model...
  CDA_Model* model = dynamic_cast<CDA_Model*>(mParent);
  if (model == NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(mParent);
    if (import == NULL)
      return NULL;
    model = dynamic_cast<CDA_Model*>(import->mParent);
    if (model == NULL)
      return NULL;
  }

  RETURN_INTO_WSTRING(componentName, name());
  ObjRef<iface::cellml_api::CellMLComponent> currentComponent = this;

  // We have a component. Go down the import chain and look for the
  // encapsulation parent in the top-most model.
  while (model->mParent)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(model->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException(L"Model parent is not an import.");

    RETURN_INTO_OBJREF(icomps, iface::cellml_api::ImportComponentSet,
                       import->components());
    RETURN_INTO_OBJREF(icompi, iface::cellml_api::ImportComponentIterator,
                       icomps->iterateImportComponents());
    bool wentDown = false;
    while (true)
    {
      RETURN_INTO_OBJREF(icomp, iface::cellml_api::ImportComponent,
                         icompi->nextImportComponent());
      if (icomp == NULL)
        break;
      RETURN_INTO_WSTRING(cr, icomp->componentRef());
      if (cr == componentName)
      {
        currentComponent = icomp;
	componentName = icomp->name();
        model = dynamic_cast<CDA_Model*>(import->mParent);
        if (model == NULL)
          throw iface::cellml_api::CellMLException(L"Import parent is not a model.");
        wentDown = true;
        break;
      }
    }
    if (!wentDown)
      break;
  }

  // We need to search deeply into groups...
  RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet,
                     model->groups());
  RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator,
                     groupSet->iterateGroups());

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
                           model->modelComponents());
        RETURN_INTO_OBJREF(fc, iface::cellml_api::NamedCellMLElement,
                           mc->get(parname.c_str()));

        // If it is missing, then the CellML is malformed.
        if (fc == NULL)
          throw iface::cellml_api::CellMLException(L"Could not find imported component.");

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
        CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
        if (ci->mImportedModel == NULL)
        {
          ic->add_ref();
          return ic;
        }
        model = unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);
        // If the real component isn't instantiated yet, return the
        // ImportComponent...
	parname = ic->componentRef();
      }
    }
  }

  return NULL;
}

already_AddRefd<iface::cellml_api::CellMLComponentSet>
CDA_CellMLComponentGroupMixin::encapsulationChildren()
  throw(std::exception&)
{
  // Find the model...
  CDA_Model* model = dynamic_cast<CDA_Model*>(mParent);
  if (model == NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(mParent);
    if (import == NULL)
      return new CDA_CellMLComponentEmptySet();
    model = dynamic_cast<CDA_Model*>(import->mParent);
    if (model == NULL)
      return new CDA_CellMLComponentEmptySet();
  }

  RETURN_INTO_WSTRING(componentName, name());
  ObjRef<iface::cellml_api::CellMLComponent> currentComponent = this;

  // We have a component. Go down the import chain, so we can search up...
  while (model->mParent)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(model->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of model is not import.");

    RETURN_INTO_OBJREF(icomps, iface::cellml_api::ImportComponentSet,
                       import->components());
    RETURN_INTO_OBJREF(icompi, iface::cellml_api::ImportComponentIterator,
                       icomps->iterateImportComponents());
    bool wentDown = false;
    while (true)
    {
      RETURN_INTO_OBJREF(icomp, iface::cellml_api::ImportComponent,
                         icompi->nextImportComponent());
      if (icomp == NULL)
        break;
      RETURN_INTO_WSTRING(cr, icomp->componentRef());
      if (cr == componentName)
      {
        currentComponent = icomp;
        componentName = icomp->name();
        CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>
          (unsafe_dynamic_cast<CDA_ImportComponent*>(icomp.getPointer())->mParent);
        if (imp == NULL)
          throw iface::cellml_api::CellMLException(L"Couldn't find import component.");
        model = dynamic_cast<CDA_Model*>(imp->mParent);
        if (model == NULL)
          throw iface::cellml_api::CellMLException(L"Import isn't in a model.");
        wentDown = true;
        break;
      }
    }
    if (!wentDown)
      break;
  }

  // We are now as far down the import chain as we can go. Start the search
  // from here and build our way back up...

  while (model)
  {
    // We need to search deeply into groups...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet,
                       model->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator,
                       groupSet->iterateGroups());

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
        return new CDA_CellMLComponentFromComponentRefSet(model, comprsc);
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the children. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...

    // See if this is an ImportComponent...
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
      (currentComponent.getPointer());
    if (ic == NULL)
      // If its not an import component, there is no parent...
      return new CDA_CellMLComponentEmptySet();

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL)
      throw iface::cellml_api::CellMLException(L"Can't find CellMLImport.");

    if (ci->mImportedModel == NULL)
      throw iface::cellml_api::CellMLException(L"Import not yet instantiated.");
    model = unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);

    RETURN_INTO_WSTRING(cr, ic->componentRef());

    // Search the new model for the component...
    RETURN_INTO_OBJREF(mcs, iface::cellml_api::CellMLComponentSet,
                       model->modelComponents());

    currentComponent = already_AddRefd<iface::cellml_api::CellMLComponent>
      (mcs->getComponent(cr.c_str()));
    if (currentComponent == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find imported component.");
    componentName = currentComponent->name();
  }

  return new CDA_CellMLComponentEmptySet();
}

already_AddRefd<iface::cellml_api::CellMLComponent>
CDA_CellMLComponentGroupMixin::containmentParent()
  throw(std::exception&)
{
  // Find the model...
  CDA_Model* model = dynamic_cast<CDA_Model*>(mParent);
  if (model == NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(mParent);
    if (import == NULL)
      return NULL;
    model = dynamic_cast<CDA_Model*>(import->mParent);
    if (model == NULL)
      return NULL;
  }

  RETURN_INTO_WSTRING(componentName, name());
  ObjRef<iface::cellml_api::CellMLComponent> currentComponent = this;

  // We have a component. Go down the import chain, so we can search up...
  while (model->mParent)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(model->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find import as parent of model.");

    RETURN_INTO_OBJREF(icomps, iface::cellml_api::ImportComponentSet,
                       import->components());
    RETURN_INTO_OBJREF(icompi, iface::cellml_api::ImportComponentIterator,
                       icomps->iterateImportComponents());
    bool wentDown = false;
    while (true)
    {
      RETURN_INTO_OBJREF(icomp, iface::cellml_api::ImportComponent,
                         icompi->nextImportComponent());
      if (icomp == NULL)
        break;
      RETURN_INTO_WSTRING(cr, icomp->componentRef());
      if (cr == componentName)
      {
        currentComponent = icomp;
        componentName = icomp->name();
        model = dynamic_cast<CDA_Model*>
          (dynamic_cast<CDA_ImportComponent*>(icomp.getPointer())->mParent);
        if (model == NULL)
          throw iface::cellml_api::CellMLException(L"Cannot find model as parent of component.");
        wentDown = true;
        break;
      }
    }
    if (!wentDown)
      break;
  }

  // We are now as far down the import chain as we can go. Start the search
  // from here and build our way back up...

  while (model)
  {
    // We need to search deeply into groups...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet,
                       model->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator,
                       groupSet->iterateGroups());

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
                             model->modelComponents());
          RETURN_INTO_OBJREF(fc, iface::cellml_api::NamedCellMLElement,
                             mc->get(parname.c_str()));

          // If it is missing, then the CellML is malformed.
          if (fc == NULL)
            throw iface::cellml_api::CellMLException(L"");

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
          CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);

          // If the real component isn't instantiated yet, return the
          // ImportComponent...
          if (ci->mImportedModel == NULL)
          {
            ic->add_ref();
            return ic;
          }
          model =
            unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);

          parname = ic->componentRef();
        }
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the parent. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...

    // See if this is an ImportComponent...
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
      (currentComponent.getPointer());
    if (ic == NULL)
      // If its not an import component, there is no parent...
      return NULL;

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL)
      throw iface::cellml_api::CellMLException(L"Import component has no import parent");

    if (ci->mImportedModel == NULL)
      throw iface::cellml_api::CellMLException(L"Imported model is not instantiated.");

    model = unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);

    RETURN_INTO_WSTRING(cr, ic->componentRef());

    // Search the new model for the component...
    RETURN_INTO_OBJREF(mcs, iface::cellml_api::CellMLComponentSet,
                       model->modelComponents());

    currentComponent = already_AddRefd<iface::cellml_api::CellMLComponent>
      (mcs->getComponent(cr.c_str()));
    if (currentComponent == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find import component in imported model.");
    componentName = currentComponent->name();
  }

  return NULL;
}

already_AddRefd<iface::cellml_api::CellMLComponentSet>
CDA_CellMLComponentGroupMixin::containmentChildren()
  throw(std::exception&)
{
  // Find the model...
  CDA_Model* model = dynamic_cast<CDA_Model*>(mParent);
  if (model == NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(mParent);
    if (import == NULL)
      return new CDA_CellMLComponentEmptySet();
    model = dynamic_cast<CDA_Model*>(import->mParent);
    if (model == NULL)
      return new CDA_CellMLComponentEmptySet();
  }

  RETURN_INTO_WSTRING(componentName, name());
  ObjRef<iface::cellml_api::CellMLComponent> currentComponent = this;

  // We have a component. Go down the import chain, so we can search up...
  while (model->mParent)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(model->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find model imported model is imported from.");

    RETURN_INTO_OBJREF(icomps, iface::cellml_api::ImportComponentSet,
                       import->components());
    RETURN_INTO_OBJREF(icompi, iface::cellml_api::ImportComponentIterator,
                       icomps->iterateImportComponents());
    bool wentDown = false;
    while (true)
    {
      RETURN_INTO_OBJREF(icomp, iface::cellml_api::ImportComponent,
                         icompi->nextImportComponent());
      if (icomp == NULL)
        break;
      RETURN_INTO_WSTRING(cr, icomp->componentRef());
      if (cr == componentName)
      {
        currentComponent = icomp;
        componentName = icomp->name();
        model = dynamic_cast<CDA_Model*>
          (dynamic_cast<CDA_ImportComponent*>(icomp.getPointer())->mParent);
        if (model == NULL)
          throw iface::cellml_api::CellMLException(L"Component is not embedded in a model.");
        wentDown = true;
        break;
      }
    }
    if (!wentDown)
      break;
  }

  // We are now as far down the import chain as we can go. Start the search
  // from here and build our way back up...

  while (model)
  {
    // We need to search deeply into groups...
    RETURN_INTO_OBJREF(groupSet, iface::cellml_api::GroupSet,
                       model->groups());
    RETURN_INTO_OBJREF(groupIt, iface::cellml_api::GroupIterator,
                       groupSet->iterateGroups());

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
        return new CDA_CellMLComponentFromComponentRefSet(model, comprsc);
      }
    }

    // If we get here, the current model does not contain the definition of
    // where to find the children. However, it may be the case that the component
    // is imported, and the encapsulation parent is defined in the imported
    // model. We firstly need to look in imports for one that imports this
    // component...

    // See if this is an ImportComponent...
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
      (currentComponent.getPointer());
    if (ic == NULL)
      // If its not an import component, there is no parent...
      return new CDA_CellMLComponentEmptySet();

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find import from import component.");

    if (ci->mImportedModel == NULL)
      throw iface::cellml_api::CellMLException(L"Import is not instantiated.");

    model = unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);

    RETURN_INTO_WSTRING(cr, ic->componentRef());

    // Search the new model for the component...
    RETURN_INTO_OBJREF(mcs, iface::cellml_api::CellMLComponentSet,
                       model->modelComponents());

    currentComponent = already_AddRefd<iface::cellml_api::CellMLComponent>
      (mcs->getComponent(cr.c_str()));
    if (currentComponent == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find component named in import.");
    componentName = currentComponent->name();
  }

  return new CDA_CellMLComponentEmptySet();
}

already_AddRefd<iface::cellml_api::ReactionSet>
CDA_CellMLComponent::reactions()
  throw(std::exception&)
{
  if (mReactionSet != NULL)
  {
    mReactionSet->add_ref();
    return mReactionSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mReactionSet = new CDA_ReactionSet(this, allChildren);
  return mReactionSet;
}

uint32_t
CDA_CellMLComponent::importNumber()
  throw(std::exception&)
{
  // Get our model...
  RETURN_INTO_OBJREFUD(modelEl, CDA_Model, modelElement());
  if (modelEl == NULL)
    return 0;
  CDA_CellMLImport* impEl = dynamic_cast<CDA_CellMLImport*>(modelEl->mParent);
  if (impEl == NULL)
    return 0;

  return impEl->uniqueIdentifier();
}

CDA_UnitsBase::~CDA_UnitsBase()
{
  if (mUnitSet != NULL)
    delete mUnitSet;
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
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::UnitSet>
CDA_UnitsBase::unitCollection()
  throw(std::exception&)
{
  if (mUnitSet)
  {
    mUnitSet->add_ref();
    return mUnitSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mUnitSet = new CDA_UnitSet(this, allChildren);
  return mUnitSet;
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
        throw iface::cellml_api::CellMLException(L"Malformed units prefix.");
      return ret;
    }
    int32_t lowerBound = 0;
    int32_t upperBound = sizeof(PrefixTable) / sizeof(PrefixTable[0]);
    while (lowerBound <= upperBound)
    {
      int32_t choice = (upperBound + lowerBound) / 2;
      int32_t cval = wcscmp(PrefixTable[choice].prefix, prefixWC);
      if (cval == 0)
        return PrefixTable[choice].value;
      else if (cval < 0)
        // The prefix in the table is less than the input, so the input must be
        // after choice...
        lowerBound = choice + 1;
      else
        upperBound = choice - 1;
    }
    throw iface::cellml_api::CellMLException(L"Prefix is neither number nor valid SI prefix.");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_Unit::prefix(int32_t attr)
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

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
    wchar_t buf[24];
    any_swprintf(buf, 24, L"%d", attr);
    buf[23] = 0;
    datastore->setAttribute(L"prefix", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

double
CDA_Unit::multiplier()
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    std::wstring mupstr = datastore->getAttributeNS(NULL_NS, L"multiplier");
    if (mupstr.empty())
      return 1.0;
    wchar_t* endstr;
    double val = wcstod(mupstr.c_str(), &endstr);
    bool invalid = false;
    if (*endstr != 0)
      invalid = true;

    if (invalid)
      throw iface::cellml_api::CellMLException(L"Invalid multiplier string on units.");

    return val;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_Unit::multiplier(double attr)
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  try
  {
    if (attr == 1.0)
    {
      datastore->removeAttribute(L"multiplier");
      return;
    }

    wchar_t buf[24];
    any_swprintf(buf, 24, L"%g", attr);
    buf[23] = 0;
    datastore->setAttribute(L"multiplier", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

double
CDA_Unit::offset()
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI());
    std::wstring offstr = datastore->getAttributeNS(NULL_NS, L"offset");
    if (offstr == L"")
      return 0.0;

    wchar_t* endstr;
    double val = wcstod(offstr.c_str(), &endstr);
    bool invalid = false;
    if (*endstr != 0)
      invalid = true;

    if (invalid)
      throw iface::cellml_api::CellMLException(L"Units offset is invalid.");

    return val;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
  }
}

void
CDA_Unit::offset(double attr)
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  try
  {
    if (attr == 0.0)
    {
      datastore->removeAttribute(L"offset");
      return;
    }

    wchar_t buf[24];
    any_swprintf(buf, 24, L"%g", attr);
    buf[23] = 0;
    datastore->setAttribute(L"offset", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
  }
}

double
CDA_Unit::exponent()
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  try
  {
    std::wstring nsURI = datastore->namespaceURI();
    std::wstring expstr = datastore->getAttributeNS(NULL_NS, L"exponent");
    if (expstr == L"")
      return 1.0;

    wchar_t* endstr;
    double val = wcstod(expstr.c_str(), &endstr);
    bool invalid = false;
    if (*endstr != 0)
      invalid = true;

    if (invalid)
      throw iface::cellml_api::CellMLException(L"Invalid exponent on units element.");

    return val;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
  }
}

void
CDA_Unit::exponent(double attr)
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  try
  {
    if (attr == 1.0)
    {
      datastore->removeAttribute(L"exponent");
      return;
    }

    wchar_t buf[24];
    any_swprintf(buf, 24, L"%g", attr);
    buf[23] = 0;
    datastore->setAttribute(L"exponent", buf);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
  }
}

std::wstring
CDA_Unit::units()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"units");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
  }
}

void
CDA_Unit::units(const std::wstring& attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"units", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
  }
}

CDA_CellMLImport::~CDA_CellMLImport()
{
  if (mImportedModel)
    delete mImportedModel;
  if (mImportConnectionSet)
    delete mImportConnectionSet;
  if (mImportComponentSet)
    delete mImportComponentSet;
  if (mImportUnitsSet)
    delete mImportUnitsSet;
}

already_AddRefd<iface::cellml_api::URI>
CDA_CellMLImport::xlinkHref()
  throw(std::exception&)
{
  return new CDA_URI(datastore, XLINK_NS, L"href", L"xlink:href");
}

already_AddRefd<iface::cellml_api::ImportComponentSet>
CDA_CellMLImport::components()
  throw(std::exception&)
{
  if (mImportComponentSet != NULL)
  {
    mImportComponentSet->add_ref();
    return mImportComponentSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mImportComponentSet = new CDA_ImportComponentSet(this, allChildren);
  return mImportComponentSet;
}

already_AddRefd<iface::cellml_api::ImportUnitsSet>
CDA_CellMLImport::units()
  throw(std::exception&)
{
  if (mImportUnitsSet != NULL)
  {
    mImportUnitsSet->add_ref();
    return mImportUnitsSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mImportUnitsSet = new CDA_ImportUnitsSet(this, allChildren);
  return mImportUnitsSet;
}

already_AddRefd<iface::cellml_api::ConnectionSet>
CDA_CellMLImport::importedConnections()
  throw(std::exception&)
{
  if (mImportConnectionSet)
  {
    mImportConnectionSet->add_ref();
    return mImportConnectionSet;
  }
  mImportConnectionSet = new CDA_ImportConnectionSet(this, this);
  return mImportConnectionSet;
}

static void
CDA_MakeURLAbsolute(CDA_Model* aModel, std::wstring& aURL)
  throw (std::exception&)
{
  // It may already be an absolute URL...
  if (aURL.find(L"://") != std::wstring::npos)
    return;

  // See if we can get an xml:base...
  RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, aModel->xmlBase());
  RETURN_INTO_WSTRING(base, bu->asText());

  if (aURL.find(L"://") != std::wstring::npos)
    return;

  // See if it is a '/' type URL...
  if (aURL[0] == L'/')
  {
    size_t pos = base.find(L"://");
    // Bail if we are trying to resolve relative to a relative URL...
    if (pos == std::wstring::npos)
      return;

    // Assume protocol://host/path, where host may be zero length e.g. file:///
    pos = base.find(L"/", pos + 3);
    std::wstring absURL;
    if (pos == std::wstring::npos)
      absURL = base;
    else
      // Don't include the slash, only everything up to it...
      absURL = base.substr(0, pos);
    absURL += aURL;
    aURL.assign(absURL);
    return;
  }

  // No point trying to deal with a zero-length base URI.
  if (base.length() == 0)
    return;

  // If it is a ? type URL, just take everything after the ? off the base.
  if (aURL[0] == L'?')
  {
    size_t pos = base.find(L'?');
    base = base.substr(0, pos);
    base += aURL;
    aURL.assign(base);
    return;
  }

  // If it is a # type URL, just take everything after the # off the base.
  if (aURL[0] == L'#')
  {
    size_t pos = base.find(L'#');
    base = base.substr(0, pos);
    base += aURL;
    aURL.assign(base);
    return;
  }

  // It is a completely relative URL.
  // See if base ends in a /...
  size_t pos = base.find(L"://");
  if (base[base.length() - 1] != L'/')
  {
    // aURL last component needs to be removed...
    size_t tpos = base.rfind(L"/");
    if (tpos == std::wstring::npos || tpos < pos + 3)
      base += L"/";
    else
      base = base.substr(0, tpos + 1);
  }
  base += aURL;

  // Substitute [^/]*/../ => / and /./ => /
  size_t prepathlength = 3;
  size_t pos2 = base.find(L"/", pos + 3);

  if (pos2 != std::wstring::npos)
    prepathlength = pos2 - pos + 1;

  pos += prepathlength;

  aURL.assign(base.substr(0, pos));

  std::list<std::wstring> pathComponents;
  bool last = false;

  do
  {
    pos2 = base.find(L"/", pos);
    if (pos2 == std::wstring::npos)
    {
      last = true;
      pos2 = base.length();
    }

    // Don't bother putting an empty path component for //
    if (pos2 != pos)
    {
      std::wstring str = base.substr(pos, pos2 - pos);
      if (str == L"..")
      {
        if (!pathComponents.empty())
          pathComponents.pop_back();
      }
      else if (str == L".")
        ;
      else
        pathComponents.push_back(str);
    }
    pos = pos2 + 1;
  }
  while (!last);

  bool first = true;

  // Now go through the path components and make a path...
  std::list<std::wstring>::iterator i;
  for (i = pathComponents.begin(); i != pathComponents.end(); i++)
  {
    if (first)
      first = false;
    else
      aURL += L'/';

    aURL += *i;
  }
  if (base[base.length() - 1] == L'/')
    aURL += L'/';
}

void
CDA_CellMLImport::instantiate()
  throw(std::exception&)
{
  // If this import has already been instantiated, throw an exception....
  if (mImportedModel != NULL)
    throw iface::cellml_api::CellMLException(L"Model is already instantiated.");

  // We need to get hold of the top level CellML model...
  CDA_CellMLElement *lastEl = NULL, *nextEl;

  nextEl = dynamic_cast<CDA_CellMLElement*>(mParent);
  CDA_Model* rootModel = dynamic_cast<CDA_Model*>(mParent);
  if (rootModel == NULL)
  {
    while (nextEl != NULL)
    {
      lastEl = nextEl;
      rootModel = dynamic_cast<CDA_Model*>(lastEl->mParent);
      if (rootModel != NULL)
        break;
      nextEl = dynamic_cast<CDA_CellMLElement*>(lastEl->mParent);
    }
  }
  if (rootModel == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find root model in hierarchy.");

  // Get the URL...
  RETURN_INTO_OBJREF(url, iface::cellml_api::URI, xlinkHref());
  RETURN_INTO_WSTRING(urlStr, url->asText());

  CDA_MakeURLAbsolute(rootModel, urlStr);

  // We now have a root model, and so we also have a loader...
  RETURN_INTO_OBJREF(dd, iface::dom::Document,
                     rootModel->mLoader->loadDocument(urlStr.c_str()));

  try
  {
    RETURN_INTO_OBJREF(modelEl, iface::dom::Element, dd->documentElement());
    if (modelEl == NULL)
      throw iface::cellml_api::CellMLException(L"Document has no document element.");

    // Check it is a CellML model...
    RETURN_INTO_WSTRING(nsURI, modelEl->namespaceURI());
    if (nsURI != CELLML_1_0_NS &&
        nsURI != CELLML_1_1_NS)
      throw iface::cellml_api::CellMLException(L"Unrecognised namespace in imported model.");

    RETURN_INTO_WSTRING(modName, modelEl->localName());
    if (modName != L"model")
      throw iface::cellml_api::CellMLException(L"Unrecognised localName in imported model.");

    ObjRef<CDA_Model> cm = already_AddRefd<CDA_Model>(new CDA_Model(rootModel->mLoader, dd, modelEl));
    RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, cm->xmlBase());
    RETURN_INTO_WSTRING(base, bu->asText());
    if (base == L"")
      bu->asText(urlStr.c_str());
    mImportedModel = cm;

    // Adjust the refcounts to leave the importedModel completely dependent on
    // the rest of the tree...
    cm->mParent = this;
    // We increment our refcount(and our other ancestors') by one...
    add_ref();

    // As cm goes out of scope, cm's(and its ancestors', which includes us, as we
    // are cm's parent), refcount is decremented by 1. cm's refcount is now 0,
    // and our/our ancestors' refcounts are back to what they were before the
    // add_ref().
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLImport::instantiateFromText(const std::wstring& aText)
  throw(std::exception&)
{
  // If this import has already been instantiated, throw an exception....
  if (mImportedModel != NULL)
    throw iface::cellml_api::CellMLException(L"Model is already instantiated.");

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
    throw iface::cellml_api::CellMLException(L"Cannot find root model in import hierarchy.");

  // We now have a root model, and so we also have a loader...
  RETURN_INTO_OBJREF(dd, iface::dom::Document,
                     rootModel->mLoader->loadDocumentFromText(aText));

  try
  {
    RETURN_INTO_OBJREF(modelEl, iface::dom::Element, dd->documentElement());
    if (modelEl == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find document element.");

    // Check it is a CellML model...
    RETURN_INTO_WSTRING(nsURI, modelEl->namespaceURI());
    if (nsURI != CELLML_1_0_NS &&
        nsURI != CELLML_1_1_NS)
      throw iface::cellml_api::CellMLException(L"Imported model has invalid namespace.");

    RETURN_INTO_WSTRING(modName, modelEl->localName());
    if (modName != L"model")
      throw iface::cellml_api::CellMLException(L"Invalid model has invalid localName.");

    CDA_Model* cm = new CDA_Model(rootModel->mLoader, dd, modelEl);
    mImportedModel = cm;

    // Adjust the refcounts to leave the importedModel completely dependent on
    // the rest of the tree...
    cm->mParent = this;
    // We increment our refcount(and our other ancestors') by one...
    add_ref();
    // This will decrement cm's(and its ancestors', which includes us, as we
    // are cm's parent), refcount by 1. cm's refcount is now 0, and our/our
    // ancestors' refcounts are back to what they were before the add_ref().
    cm->release_ref();
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }

}

class CDA_CellMLImport_DocumentLoadedListener
  : public iface::cellml_api::DocumentLoadedListener
{
public:
  CDA_CellMLImport_DocumentLoadedListener
  (
   iface::cellml_api::CellMLImport* aImport,
   iface::cellml_api::ImportInstantiationListener* aListener,
   iface::cellml_api::DOMURLLoader* aLoader,
   std::wstring& aURL
  )
    : mImport(aImport), mListener(aListener),
      mLoader(aLoader), mURL(aURL)
  {
    mImport->add_ref();
    mListener->add_ref();
    mLoader->add_ref();
  }

  virtual ~CDA_CellMLImport_DocumentLoadedListener()
  {
    mImport->release_ref();
    mListener->release_ref();
    mLoader->release_ref();
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::DocumentLoadedListener);
  CDA_IMPL_ID;

  void loadCompleted(iface::dom::Document* dd)
    throw(std::exception&)
  {
    try
    {
      RETURN_INTO_OBJREF(modelEl, iface::dom::Element, dd->documentElement());
      if (modelEl == NULL)
        throw iface::cellml_api::CellMLException(L"Cannot find document element.");

      // Check it is a CellML model...
      RETURN_INTO_WSTRING(nsURI, modelEl->namespaceURI());
      if (nsURI != CELLML_1_0_NS &&
          nsURI != CELLML_1_1_NS)
        throw iface::cellml_api::CellMLException(L"Loaded model has invalid namespace.");

      RETURN_INTO_WSTRING(modName, modelEl->localName());
      if (modName != L"model")
        throw iface::cellml_api::CellMLException(L"Loaded model has invalid document element local name.");

      // Now check the URL of the xlink:href didn't change.
      RETURN_INTO_OBJREF(hrefURL, iface::cellml_api::URI,
                         mImport->xlinkHref());
      RETURN_INTO_WSTRING(URL, hrefURL->asText());
      // See if the URL is still valid...
      if (mURL != URL)
      {
        mListener->loadCompleted(true);
        return;
      }

      // Check no other load beat us to it...
      if (dynamic_cast<CDA_CellMLImport*>(mImport)->mImportedModel)
      {
        mListener->loadCompleted(false);
        return;
      }

      CDA_Model* cm = new CDA_Model(mLoader, dd, modelEl);
      dynamic_cast<CDA_CellMLImport*>(mImport)->mImportedModel = cm;

      // Adjust the refcounts to leave the importedModel completely dependent on
      // the rest of the tree...
      cm->mParent = unsafe_dynamic_cast<CDA_CellMLImport*>(mImport);
      // We increment our refcount(and our other ancestors') by one...
      add_ref();
      // This will decrement cm's(and its ancestors', which includes us, as we
      // are cm's parent), refcount by 1. cm's refcount is now 0, and our/our
      // ancestors' refcounts are back to what they were before the add_ref().
      cm->release_ref();

      mListener->loadCompleted(false);
    }
    catch (iface::dom::DOMException& de)
    {
      throw iface::cellml_api::CellMLException(L"DOM exception caught.");
    }
  }

private:
  iface::cellml_api::CellMLImport* mImport;
  iface::cellml_api::ImportInstantiationListener* mListener;
  iface::cellml_api::DOMURLLoader* mLoader;
  std::wstring mURL;
};

void
CDA_CellMLImport::asyncInstantiate
(
 iface::cellml_api::ImportInstantiationListener* listener
)
  throw(std::exception&)
{
  // If this import has already been instantiated, throw an exception....
  if (mImportedModel != NULL)
    throw iface::cellml_api::CellMLException(L"Already instantiated.");

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
    throw iface::cellml_api::CellMLException(L"Could not find the top-level of the CellML model.");

  // Get the URL...
  RETURN_INTO_OBJREF(url, iface::cellml_api::URI, xlinkHref());
  RETURN_INTO_WSTRING(urlStr, url->asText());

  CDA_MakeURLAbsolute(rootModel, urlStr);

  // We now have a root model, and so we also have a loader...
  RETURN_INTO_OBJREF(cidll, iface::cellml_api::DocumentLoadedListener,
                     new CDA_CellMLImport_DocumentLoadedListener
                     (
                      this, listener, rootModel->mLoader, urlStr
                     )
                    );
  rootModel->mLoader->asyncLoadDocument(urlStr.c_str(), cidll);
}

bool
CDA_CellMLImport::wasInstantiated()
  throw(std::exception&)
{
  return (mImportedModel != NULL);
}

uint32_t
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
    throw iface::cellml_api::CellMLException(L"Root of fragment is not CellML model.");

  if (m != lastIdentifierModel())
  {
    mUniqueIdentifier = m->assignUniqueIdentifier();
    lastIdentifierModel = m;
  }

  return mUniqueIdentifier;
}

already_AddRefd<iface::cellml_api::Model>
CDA_CellMLImport::importedModel()
  throw(std::exception&)
{
  if (mImportedModel != NULL)
    mImportedModel->add_ref();
  return mImportedModel;
}

void
CDA_CellMLImport::uninstantiate()
  throw(std::exception&)
{
  if (mImportedModel == NULL)
    return;

  unsafe_dynamic_cast<CDA_Model*>(mImportedModel)
    ->removeLinkFromHereToParent();

  mImportedModel = NULL;
}

CDA_CellMLComponent*
CDA_ImportComponent::fetchDefinition()
  throw(std::exception&)
{
  // Fetch the name in the parent...
  CDA_CellMLImport* cmi = dynamic_cast<CDA_CellMLImport*>(mParent);
  if (cmi == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of import component isn't import.");

  // We need imports to be instantiated for this to work, too...
  if (cmi->mImportedModel == NULL)
    throw iface::cellml_api::CellMLException(L"Import is not instantiated.");

  // Fetch the components...
  RETURN_INTO_WSTRING(compName, componentRef());

  // Now search for it in the imported model...
  CDA_Model* m = unsafe_dynamic_cast<CDA_Model*>(cmi->mImportedModel);

  RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                     m->modelComponents());
  RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                     mc->getComponent(compName.c_str()));
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find component referenced in import.");

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

already_AddRefd<iface::cellml_api::CellMLVariableSet>
CDA_ImportComponent::variables()
  throw(std::exception&)
{
  return fetchDefinition()->variables();
}

already_AddRefd<iface::cellml_api::UnitsSet>
CDA_ImportComponent::units()
  throw(std::exception&)
{
  return fetchDefinition()->units();
}

already_AddRefd<iface::cellml_api::ConnectionSet>
CDA_ImportComponent::connections()
  throw(std::exception&)
{
  return fetchDefinition()->connections();
}

already_AddRefd<iface::cellml_api::ReactionSet>
CDA_ImportComponent::reactions()
  throw(std::exception&)
{
  return fetchDefinition()->reactions();
}

already_AddRefd<iface::cellml_api::MathList>
CDA_ImportComponent::math()
  throw(std::exception&)
{
  return fetchDefinition()->math();
}

void
CDA_ImportComponent::addMath(iface::cellml_api::MathMLElement el)
  throw(std::exception&)
{
  fetchDefinition()->addMath(el);
}

void
CDA_ImportComponent::removeMath(iface::cellml_api::MathMLElement el)
  throw(std::exception&)
{
  fetchDefinition()->removeMath(el);
}

void
CDA_ImportComponent::replaceMath
(
 iface::cellml_api::MathMLElement x,
 iface::cellml_api::MathMLElement y
)
  throw(std::exception&)
{
  fetchDefinition()->replaceMath(x, y);
}

void
CDA_ImportComponent::clearMath()
  throw(std::exception&)
{
  fetchDefinition()->clearMath();
}

uint32_t
CDA_ImportComponent::importNumber()
  throw(std::exception&)
{
  return fetchDefinition()->importNumber();
#if 0
  // Get our model...
  ObjRef<CDA_Model> modelEl(already_AddRefd<CDA_Model>
                            (dynamic_cast<CDA_Model*>(modelElement())));
  if (modelEl == NULL)
    return 0;
  CDA_CellMLImport* impEl = dynamic_cast<CDA_CellMLImport*>(modelEl->mParent);
  if (impEl == NULL)
    return 0;

  return impEl->uniqueIdentifier();
#endif
}

std::wstring
CDA_ImportComponent::componentRef()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"component_ref");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_ImportComponent::componentRef(const std::wstring& attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"component_ref", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

CDA_Units*
CDA_ImportUnits::fetchDefinition()
  throw(std::exception&)
{
  // Fetch the name in the parent...
  CDA_CellMLImport* cmi = dynamic_cast<CDA_CellMLImport*>(mParent);
  if (cmi == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of import units is not a CellML import.");

  // We need imports to be instantiated for this to work, too...
  if (cmi->mImportedModel == NULL)
    throw iface::cellml_api::CellMLException(L"Import is not instantiated.");

  // Fetch the components...
  RETURN_INTO_WSTRING(unitsName, unitsRef());

  // Now search for it in the imported model...
  CDA_Model* m = unsafe_dynamic_cast<CDA_Model*>(cmi->mImportedModel);

  RETURN_INTO_OBJREF(mu, iface::cellml_api::UnitsSet,
                     m->modelUnits());
  RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                     mu->getUnits(unitsName.c_str()));
  if (u == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find units that were imported.");

  CDA_ImportUnits* iu =
    dynamic_cast<CDA_ImportUnits*>(u.getPointer());
  if (iu != NULL)
  {
    // Just recurse to finish the work...
    return iu->fetchDefinition();
  }

  // We are documented as not calling add_ref. This design might need to be
  // changed for threadsafety later.
  return unsafe_dynamic_cast<CDA_Units*>(u.getPointer());
}


std::wstring
CDA_ImportUnits::unitsRef()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"units_ref");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_ImportUnits::unitsRef(const std::wstring& attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"units_ref", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

bool
CDA_ImportUnits::isBaseUnits()
  throw(std::exception&)
{
  return fetchDefinition()->isBaseUnits();
}

void
CDA_ImportUnits::isBaseUnits(bool attr)
  throw(std::exception&)
{
  fetchDefinition()->isBaseUnits(attr);
}

already_AddRefd<iface::cellml_api::UnitSet>
CDA_ImportUnits::unitCollection()
  throw(std::exception&)
{
  return fetchDefinition()->unitCollection();
}

CDA_CellMLVariable::~CDA_CellMLVariable()
{
  if (mConnectedCellMLVariableSet != NULL)
    delete mConnectedCellMLVariableSet;
}

std::wstring
CDA_CellMLVariable::initialValue()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"initial_value");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLVariable::initialValue
(
 const std::wstring& attr
)
  throw(std::exception&)
{
  try
  {
    if (attr == L"")
      datastore->removeAttribute(L"initial_value");
    else
      datastore->setAttribute(L"initial_value", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

bool
CDA_CellMLVariable::initialValueFromVariable()
  throw(std::exception&)
{
  std::wstring iv = initialValue();
  const wchar_t* ivS = iv.c_str();
  wchar_t* ivE;

  wcstod(ivS, &ivE);
  return (ivE == ivS);
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDA_CellMLVariable::initialValueVariable()
  throw(std::exception&)
{
  std::wstring iv = initialValue();
  ObjRef<iface::cellml_api::CellMLComponent> comp(QueryInterface(parentElement()));
  if (comp == NULL)
    return NULL;

  ObjRef<iface::cellml_api::CellMLVariableSet> vs(comp->variables());
  return vs->getVariable(iv);
}

void
CDA_CellMLVariable::initialValueVariable(iface::cellml_api::CellMLVariable* aValue)
  throw(std::exception&)
{
  if (aValue == NULL)
  {
    datastore->removeAttribute(L"initial_value");
    return;
  }

  CDA_CellMLVariable* cv = dynamic_cast<CDA_CellMLVariable*>(aValue);
  if (cv == NULL)
    throw iface::cellml_api::CellMLException(L"CellML variable not from this implementation.");
  ObjRef<iface::dom::Node> mycomp(datastore->parentNode());
  if (mycomp == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of variable is not a component.");
  ObjRef<iface::dom::Node> theircomp(cv->datastore->parentNode());
  if (CDA_objcmp(mycomp, theircomp))
    throw iface::cellml_api::CellMLException(L"Cannot set initial_value to a variable in another component.");
  std::wstring compName = aValue->name();
  initialValue(compName);
}

double
CDA_CellMLVariable::initialValueValue()
  throw(std::exception&)
{
  std::wstring iv = initialValue();
  const wchar_t* ivS = iv.c_str();
  wchar_t * ivE;
  double v = wcstod(ivS, &ivE);
  if (ivE == ivS)
    throw iface::cellml_api::CellMLException(L"Cannot parse initial value as a double.");

  return v;
}

void
CDA_CellMLVariable::initialValueValue(double aValue)
  throw(std::exception&)
{
  wchar_t buf[30];
  any_swprintf(buf, 24, L"%g", aValue);
  buf[29] = 0;

  initialValue(buf);
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
    throw iface::cellml_api::CellMLException(L"private_interface has invalid value.");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
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
      throw iface::cellml_api::CellMLException(L"private_interface set to invalid value.");
    }

    datastore->setAttribute(L"private_interface", str);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
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
    throw iface::cellml_api::CellMLException(L"public_interface has invalid value.");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception raised.");
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
      throw iface::cellml_api::CellMLException(L"public_interface set to invalid value.");
    }

    datastore->setAttribute(L"public_interface", str);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::CellMLVariableSet>
CDA_CellMLVariable::connectedVariables()
  throw(std::exception&)
{
  if (mConnectedCellMLVariableSet != NULL)
  {
    mConnectedCellMLVariableSet->add_ref();
    return mConnectedCellMLVariableSet;
  }
  mConnectedCellMLVariableSet = new CDA_ConnectedCellMLVariableSet(this, this);
  return mConnectedCellMLVariableSet;
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDA_CellMLVariable::sourceVariable()
  throw(std::exception&)
{
  // Test for the easy special case where this is the source variable...
  if (publicInterface() != iface::cellml_api::INTERFACE_IN &&
      privateInterface() != iface::cellml_api::INTERFACE_IN)
  {
    add_ref();
    return this;
  }

  // Find all connected variables...
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                     connectedVariables());
  RETURN_INTO_OBJREF(cvi, iface::cellml_api::CellMLVariableIterator,
                     cvs->iterateVariables());
  while (true)
  {
    RETURN_INTO_OBJREF(v, iface::cellml_api::CellMLVariable, cvi->nextVariable());
    if (v == NULL)
    {
      return NULL;
    }

    if ((v->publicInterface() != iface::cellml_api::INTERFACE_IN) &&
        (v->privateInterface() != iface::cellml_api::INTERFACE_IN))
    {
      v->add_ref();
      return v.getPointer();
    }
  }
}

std::wstring
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

std::wstring
CDA_CellMLVariable::unitsName()
  throw(std::exception&)
{
  try
  {
    return datastore->getAttributeNS(NULL_NS, L"units");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLVariable::unitsName(const std::wstring& aUnitsName)
  throw(std::exception&)
{
  try
  {
    datastore->setAttributeNS(NULL_NS, L"units", aUnitsName);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::Units>
CDA_CellMLVariable::unitsElement()
  throw(std::exception&)
{
  try
  {
    // Go up to the component...
    CDA_CellMLComponent* comp
      = dynamic_cast<CDA_CellMLComponent*>(mParent);
    if (comp == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of variable is not units.");

    RETURN_INTO_OBJREF(unitsSet, iface::cellml_api::UnitsSet, comp->units());
    RETURN_INTO_WSTRING(uName, unitsName());
    if (uName == L"")
      throw iface::cellml_api::CellMLException(L"Units name is blank.");
    RETURN_INTO_OBJREF(units, iface::cellml_api::Units,
                       unitsSet->getUnits(uName.c_str()));
    if (units != NULL)
    {
      units->add_ref();
      return units.getPointer();
    }

    // Not in the component, find the model...
    CDA_Model* model
      = dynamic_cast<CDA_Model*>(comp->mParent);

    if (model == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of component is not model.");

    unitsSet = already_AddRefd<iface::cellml_api::UnitsSet>
      (model->modelUnits());
    units = already_AddRefd<iface::cellml_api::Units>
      (unitsSet->getUnits(uName.c_str()));
    if (units != NULL)
    {
      units->add_ref();
      return units.getPointer();
    }
    throw iface::cellml_api::CellMLException(L"Cannot find units named on variable.");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_CellMLVariable::unitsElement(iface::cellml_api::Units* aUnits)
  throw(std::exception&)
{
  try
  {
    CDA_CellMLElement* units = dynamic_cast<CDA_CellMLElement*>(aUnits);
    if (units == NULL)
      throw iface::cellml_api::CellMLException(L"Units is not from this implementation.");
    CDA_CellMLElement* ce = dynamic_cast<CDA_CellMLElement*>(units->mParent);
    if (ce == NULL)
      throw iface::cellml_api::CellMLException(L"Units element has no parent.");
    CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>(ce);
    if (imp != NULL)
      ce = dynamic_cast<CDA_CellMLElement*>(imp->mParent);

    CDA_CellMLElement* comp = dynamic_cast<CDA_CellMLElement*>(mParent);
    if (comp == NULL)
      throw iface::cellml_api::CellMLException(L"Variable element has no parent.");
    if (comp != ce)
    {
      CDA_CellMLElement* model = dynamic_cast<CDA_CellMLElement*>
        (comp->mParent);
      if (model == NULL || model != ce)
        throw iface::cellml_api::CellMLException(L"Variable not in a model.");
    }

    // Well, the units is usable here, so get the name...
    RETURN_INTO_WSTRING(name, aUnits->name());

    // Set the attribute...
    datastore->setAttributeNS(NULL_NS, L"units", name.c_str());
  }
  catch (iface::dom::DOMException& de)
  {
  }
}

CDA_ComponentRef::~CDA_ComponentRef()
{
  if (mCRSet != NULL)
    delete mCRSet;
}

std::wstring
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_ComponentRef::componentName(const std::wstring& attr)
  throw(std::exception&)
{
  try
  {
    datastore->setAttribute(L"component", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::ComponentRefSet>
CDA_ComponentRef::componentRefs()
  throw(std::exception&)
{
  if (mCRSet != NULL)
  {
    mCRSet->add_ref();
    return mCRSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mCRSet = new CDA_ComponentRefSet(this, allChildren.getPointer());
  return mCRSet;
}

already_AddRefd<iface::cellml_api::ComponentRef>
CDA_ComponentRef::parentComponentRef()
  throw(std::exception&)
{
  iface::cellml_api::ComponentRef* pcr =
    dynamic_cast<iface::cellml_api::ComponentRef*>(mParent);
  if (pcr != NULL)
    pcr->add_ref();
  return pcr;
}

already_AddRefd<iface::cellml_api::Group>
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

std::wstring
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
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_RelationshipRef::name(const std::wstring& attr)
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(nsURI, datastore->namespaceURI())
    datastore->setAttributeNS(NULL_NS, L"name", attr);
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

std::wstring
CDA_RelationshipRef::relationship()
  throw(std::exception&)
{
  try
  {
    // We are looking for relationships in any namespace, so we need to
    // go through all nodes...
    RETURN_INTO_OBJREF(cn, iface::dom::NamedNodeMap, datastore->attributes());
    uint32_t l = cn->length();
    uint32_t i;
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
	std::wstring str = at->nodeName();
        ln = str;
      }
      if (ln != L"relationship")
        continue;

      return at->value();
    }
    throw iface::cellml_api::CellMLException(L"Cannot find a relationship attribute.");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

std::wstring
CDA_RelationshipRef::relationshipNamespace()
  throw(std::exception&)
{
  try
  {
    // We are looking for relationships in any namespace, so we need to
    // go through all nodes...
    RETURN_INTO_OBJREF(cn, iface::dom::NamedNodeMap, datastore->attributes());
    uint32_t l = cn->length();
    uint32_t i;
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
        ln = at->nodeName();

      if (ln != L"relationship")
        continue;

      return at->namespaceURI();
    }
    throw iface::cellml_api::CellMLException(L"Cannot find a relationship attribute.");
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_RelationshipRef::setRelationshipName(const std::wstring& namespaceURI,
                                         const std::wstring& name)
  throw(std::exception&)
{
  try
  {
    // We are looking for relationships in any namespace, so we need to
    // go through all nodes...
    RETURN_INTO_OBJREF(cn, iface::dom::NamedNodeMap, datastore->attributes());
    uint32_t l = cn->length();
    uint32_t i;
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
      if (ln == L"")
        ln = at->nodeName();
      if (ln != L"relationship")
      {
        i++;
        continue;
      }

      // Remove the attribute...
      datastore->removeAttributeNode(at)->release_ref();
    }

    datastore->setAttributeNS(namespaceURI, L"relationship", name);
    return;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

CDA_Group::~CDA_Group()
{
  if (mRRSet != NULL)
    delete mRRSet;
  if (mCRSet != NULL)
    delete mCRSet;
}

already_AddRefd<iface::cellml_api::RelationshipRefSet>
CDA_Group::relationshipRefs()
  throw(std::exception&)
{
  if (mRRSet != NULL)
  {
    mRRSet->add_ref();
    return mRRSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mRRSet = new CDA_RelationshipRefSet(this, allChildren);
  return mRRSet;
}

already_AddRefd<iface::cellml_api::ComponentRefSet>
CDA_Group::componentRefs()
  throw(std::exception&)
{
  if (mCRSet != NULL)
  {
    mCRSet->add_ref();
    return mCRSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mCRSet = new CDA_ComponentRefSet(this, allChildren);
  return mCRSet;
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
    if (rr->relationship() == L"encapsulation")
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
    if (rr->relationship() == L"containment")
      return true;
  }
}

CDA_Connection::~CDA_Connection()
{
  if (mMVS != NULL)
    delete mMVS;
}

already_AddRefd<iface::cellml_api::MapComponents>
CDA_Connection::componentMapping()
  throw(std::exception&)
{
  if (CDA_CompareSerial(mCacheSerial))
  {
    mMapComponents->add_ref();
    return mMapComponents;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet,
                       childElements());
  RETURN_INTO_OBJREF(allChildrenIt, iface::cellml_api::CellMLElementIterator,
                     allChildren->iterate());
  while (true)
  {
    RETURN_INTO_OBJREF(child, iface::cellml_api::CellMLElement,
                       allChildrenIt->next());
    if (child == NULL)
    {
      // No MapComponents. Make one...
      RETURN_INTO_OBJREFUD(me, CDA_Model, modelElement());
      if (me == NULL)
        throw iface::cellml_api::CellMLException(L"Connection not in model.");
      RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                         me->createMapComponents());
      addElement(mc);
      iface::cellml_api::MapComponents* rmc = mc;
      rmc->add_ref();
      mMapComponents = rmc;
      mCacheSerial = gCDAChangeSerial;
      return rmc;
    }
    iface::cellml_api::MapComponents* mc =
      dynamic_cast<iface::cellml_api::MapComponents*>(child.getPointer());
    if (mc != NULL)
    {
      mc->add_ref();
      mMapComponents = mc;
      mCacheSerial = gCDAChangeSerial;
      return mc;
    }
  }
}

already_AddRefd<iface::cellml_api::MapVariablesSet>
CDA_Connection::variableMappings()
  throw(std::exception&)
{
  if (mMVS != NULL)
  {
    mMVS->add_ref();
    return mMVS;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mMVS = new CDA_MapVariablesSet(this, allChildren);
  return mMVS;
}

std::wstring
CDA_MapComponents::firstComponentName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"component_1");
}

void
CDA_MapComponents::firstComponentName(const std::wstring& attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"component_1", attr);
}

std::wstring
CDA_MapComponents::secondComponentName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"component_2");
}

void
CDA_MapComponents::secondComponentName(const std::wstring& attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"component_2", attr);
}

already_AddRefd<iface::cellml_api::CellMLComponent>
CDA_MapComponents::firstComponent()
  throw(std::exception&)
{
  // Get the connection...
  CDA_Connection* c =
    dynamic_cast<CDA_Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of map_components isn't a connection.");

  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of connection isn't model.");

  RETURN_INTO_WSTRING(cn, firstComponentName());
  if (cn == L"")
    return NULL;

  while (true)
  {
    RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                       m->modelComponents());
    RETURN_INTO_OBJREF(ce, iface::cellml_api::NamedCellMLElement,
                       mc->get(cn.c_str()));
    if (ce == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find component named in connection.");

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
    if (ci == NULL || ci->mImportedModel == NULL)
    {
      iface::cellml_api::CellMLComponent* cc =
        dynamic_cast<iface::cellml_api::CellMLComponent*>(ce.getPointer());
      cc->add_ref();
      return cc;
    }

    m = unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);
    std::wstring icrn = ic->componentRef();
    cn = icrn;
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
    throw iface::cellml_api::CellMLException(L"Parent of map_components isn't connection.");

  CDA_Model* thisModel = dynamic_cast<CDA_Model*>(connection->mParent);
  if (thisModel == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of connection isn't model.");

  CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>(attr);
  CDA_Model* m = NULL, *mOrig = NULL;
  if (ic != NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of import component isn't import.");

    m = dynamic_cast<CDA_Model*>(import->mParent);
    mOrig = m;
    while (m != thisModel)
    {
      m = unsafe_dynamic_cast<CDA_Model*>(import->mImportedModel);
      if (m == NULL)
        // Don't return yet, we might find the model up the import chain.
        break;

      RETURN_INTO_WSTRING(compName, ic->componentRef());
      RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                         m->modelComponents());
      comp = already_AddRefd<iface::cellml_api::CellMLComponent>
        (dynamic_cast<iface::cellml_api::CellMLComponent*>
         (mc->get(compName).getPointer()));
      if (comp == NULL)
        throw iface::cellml_api::CellMLException(L"Cannot find component named in import component.");
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
        throw iface::cellml_api::CellMLException(L"Cannot find importing model.");

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
          throw iface::cellml_api::CellMLException(L"Cannot find matching import component.");
        RETURN_INTO_WSTRING(compR, impc->componentRef());
        if (compR == compName)
        {
          comp = static_cast<iface::cellml_api::CellMLComponent*>
            (impc.getPointer());
          compName = impc->name();
          m = dynamic_cast<CDA_Model*>(ci->mParent);
          if (m == NULL)
            throw iface::cellml_api::CellMLException(L"Parent of import isn't model.");
          break;
        }
      }
    }
  }

  // We just need to set the component_1 attr...
  RETURN_INTO_WSTRING(cn, comp->name());
  datastore->setAttributeNS(NULL_NS, L"component_1", cn);
}

already_AddRefd<iface::cellml_api::CellMLComponent>
CDA_MapComponents::secondComponent()
  throw(std::exception&)
{
  // Get the connection...
  CDA_Connection* c =
    dynamic_cast<CDA_Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of map_components isn't a connection.");

  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of connection isn't model.");

  RETURN_INTO_WSTRING(cn, secondComponentName());
  if (cn == L"")
    return NULL;

  while (true)
  {
    RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                       m->modelComponents());
    RETURN_INTO_OBJREF(ce, iface::cellml_api::NamedCellMLElement,
                       mc->get(cn));
    if (ce == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find component named in map_components.");

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
    if (ci == NULL || ci->mImportedModel == NULL)
    {
      iface::cellml_api::CellMLComponent* cc =
        dynamic_cast<iface::cellml_api::CellMLComponent*>(ce.getPointer());
      cc->add_ref();
      return cc;
    }

    m = unsafe_dynamic_cast<CDA_Model*>(ci->mImportedModel);
    cn = ic->componentRef();
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
    throw iface::cellml_api::CellMLException(L"Parent of map_components isn't a connection.");

  CDA_Model* thisModel = dynamic_cast<CDA_Model*>(connection->mParent);
  if (thisModel == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of connection isn't model.");

  CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>(attr);
  CDA_Model* m = NULL, *mOrig = NULL;
  if (ic != NULL)
  {
    CDA_CellMLImport* import = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (import == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find imported model.");

    m = dynamic_cast<CDA_Model*>(import->mParent);
    mOrig = m;
    while (m != thisModel)
    {
      m = unsafe_dynamic_cast<CDA_Model*>(import->mImportedModel);
      if (m == NULL)
        // Don't return yet, we might find the model up the import chain.
        break;

      RETURN_INTO_WSTRING(compName, ic->componentRef());
      RETURN_INTO_OBJREF(mc, iface::cellml_api::CellMLComponentSet,
                         m->modelComponents());
      comp = already_AddRefd<iface::cellml_api::CellMLComponent>
        (dynamic_cast<iface::cellml_api::CellMLComponent*>
         (mc->get(compName).getPointer()));
      if (comp == NULL)
        throw iface::cellml_api::CellMLException(L"Cannot find component referenced by map_components.");
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
        throw iface::cellml_api::CellMLException(L"Cannot find importing model.");

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
          throw iface::cellml_api::CellMLException(L"Cannot find import component for map_components.");
        RETURN_INTO_WSTRING(compR, impc->componentRef());
        if (compR == compName)
        {
          comp = static_cast<iface::cellml_api::CellMLComponent*>
            (impc.getPointer());
          compName = impc->name();
          m = dynamic_cast<CDA_Model*>(ci->mParent);
          if (m == NULL)
            throw iface::cellml_api::CellMLException(L"Parent of import element isn't a model.");
          break;
        }
      }
    }
  }

  // We just need to set the component_1 attr...
  RETURN_INTO_WSTRING(cn, comp->name());
  datastore->setAttributeNS(NULL_NS, L"component_2", cn);
}

std::wstring
CDA_MapVariables::firstVariableName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"variable_1");
}

void
CDA_MapVariables::firstVariableName(const std::wstring& attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"variable_1", attr);
}

std::wstring
CDA_MapVariables::secondVariableName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"variable_2");
}

void
CDA_MapVariables::secondVariableName(const std::wstring& attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"variable_2", attr);
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDA_MapVariables::firstVariable()
  throw(std::exception&)
{
  // See if we have a connection...
  iface::cellml_api::Connection* c =
    dynamic_cast<iface::cellml_api::Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of map_variables isn't a connection.");

  // Next find the component...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     c->componentMapping());
  RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                     mc->firstComponent());
  if (comp == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find first component.");

  // and look in the component for the variable...
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                     comp->variables());
  RETURN_INTO_WSTRING(fvn, firstVariableName())
  CDA_CellMLVariable* v =
    unsafe_dynamic_cast<CDA_CellMLVariable*>(cvs->get(fvn.c_str()).getPointer());

  // The CellML is malformed if the referenced variable doesn't exist.
  if (v == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find mapped variable in component.");

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
    throw iface::cellml_api::CellMLException(L"Parent of map_variables isn't a connection.");

  // Next, find our parent...
  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of connection isn't a model.");

  // To connect variables, the component needs to be available under some name
  // in the model. See if we can find the appropriate name...
  CDA_CellMLVariable* v = dynamic_cast<CDA_CellMLVariable*>(attr);
  if (v == NULL)
    throw iface::cellml_api::CellMLException(L"Variable is not from this implementation.");

  ObjRef<iface::cellml_api::CellMLComponent> comp =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(v->mParent);
  if (comp == NULL)
    throw iface::cellml_api::CellMLException(L"Variable doesn't belong to any component.");

  // This component is guaranteed not to be an import component, or it wouldn't
  // have variables. But the component we are looking for could be this one, or
  // it could be up the import chain.
  CDA_Model* compModel = dynamic_cast<CDA_Model*>
    (unsafe_dynamic_cast<CDA_CellMLComponent*>(comp.getPointer())->mParent);
  if (compModel == NULL)
    throw iface::cellml_api::CellMLException(L"Component doesn't belong to any model.");

  std::wstring compName = comp->name();

  while (compModel != m)
  {

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(compModel->mParent);

    // If we reached the root, the variable cannot be added to this connection.
    if (ci == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find common ancestor model between variable to set on map_cvariables and map_variables.");

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
        throw iface::cellml_api::CellMLException(L"Could not find import component matching component name.");
      RETURN_INTO_WSTRING(compRef, ic->componentRef());
      if (compRef == compName)
      {
        comp = dynamic_cast<iface::cellml_api::CellMLComponent*>
          (ic.getPointer());
        compModel = dynamic_cast<CDA_Model*>(ci->mParent);
        compName = comp->name();
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
    throw iface::cellml_api::CellMLException(L"Cannot set firstVariable without setting the correct component.");

  // Everything is now ready to set the variable...
  RETURN_INTO_WSTRING(vname, attr->name());
  firstVariableName(vname.c_str());
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDA_MapVariables::secondVariable()
  throw(std::exception&)
{
  // See if we have a connection...
  iface::cellml_api::Connection* c =
    dynamic_cast<iface::cellml_api::Connection*>(mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of map_variables is not a connection.");

  // Next find the component...
  RETURN_INTO_OBJREF(mc, iface::cellml_api::MapComponents,
                     c->componentMapping());
  RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                     mc->secondComponent());
  if (comp == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot fetch the second component for a connection.");

  // and look in the component for the variable...
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet,
                     comp->variables());
  RETURN_INTO_WSTRING(fvn, secondVariableName())
  iface::cellml_api::CellMLVariable* v =
    dynamic_cast<iface::cellml_api::CellMLVariable*>(cvs->get(fvn).getPointer());

  // The CellML is malformed if the referenced variable doesn't exist.
  if (v == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find second_variable named in map_variables.");

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
    throw iface::cellml_api::CellMLException(L"Parent of map_variables is not a connection.");

  // Next, find our parent...
  CDA_Model* m =
    dynamic_cast<CDA_Model*>(c->mParent);

  if (m == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of connection is not a model.");

  // To connect variables, the component needs to be available under some name
  // in the model. See if we can find the appropriate name...
  CDA_CellMLVariable* v = dynamic_cast<CDA_CellMLVariable*>(attr);
  if (v == NULL)
    throw iface::cellml_api::CellMLException(L"Variable is not from this implementation.");

  ObjRef<iface::cellml_api::CellMLComponent> comp =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(v->mParent);
  if (comp == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of variable is not a CellML component.");

  // This component is guaranteed not to be an import component, or it wouldn't
  // have variables. But the component we are looking for could be this one, or
  // it could be up the import chain.
  CDA_Model* compModel = dynamic_cast<CDA_Model*>
    (dynamic_cast<CDA_CellMLComponent*>(comp.getPointer())->mParent);
  if (compModel == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of component is not a model.");

  std::wstring compName = comp->name();

  while (compModel != m)
  {

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(compModel->mParent);

    // If we reached the root, the variable cannot be added to this connection.
    if (ci == NULL)
      throw iface::cellml_api::CellMLException(L"Variable doesn't share a common model ancestor with map_variables.");

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
        throw iface::cellml_api::CellMLException(L"Cannot find a component matching the variable component.");
      RETURN_INTO_WSTRING(compRef, ic->componentRef());
      if (compRef == compName)
      {
        comp = dynamic_cast<iface::cellml_api::CellMLComponent*>
          (ic.getPointer());
        compModel = dynamic_cast<CDA_Model*>(ci->mParent);
        compName = comp->name();
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
    throw iface::cellml_api::CellMLException(L"Cannot set second_variable on a component without first setting the component to be the correct component.");

  // Everything is now ready to set the variable...
  RETURN_INTO_WSTRING(vname, attr->name());
  secondVariableName(vname.c_str());
}

CDA_Reaction::~CDA_Reaction()
{
  if (mVariableRefSet != NULL)
    delete mVariableRefSet;
}

already_AddRefd<iface::cellml_api::VariableRefSet>
CDA_Reaction::variableReferences()
  throw(std::exception&)
{
  if (mVariableRefSet != NULL)
  {
    mVariableRefSet->add_ref();
    return mVariableRefSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mVariableRefSet = new CDA_VariableRefSet(this, allChildren);
  return mVariableRefSet;
}

bool
CDA_Reaction::reversible()
  throw(std::exception&)
{
  std::wstring str = datastore->getAttributeNS(NULL_NS, L"reversible");
  bool rev = true;
  if (str == L"no")
    rev = false;

  return rev;
}

void
CDA_Reaction::reversible(bool attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"reversible", attr ? L"yes" : L"no");
}

already_AddRefd<iface::cellml_api::VariableRef>
CDA_Reaction::getVariableRef(const std::wstring& varName, bool create)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(vrs, iface::cellml_api::VariableRefSet,
                     variableReferences());
  RETURN_INTO_OBJREF(vri, iface::cellml_api::VariableRefIterator,
                     vrs->iterateVariableRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(vr, iface::cellml_api::VariableRef,
                       vri->nextVariableRef());
    if (vr == NULL)
    {
      if (!create)
        return NULL;

      // Get our model...
      RETURN_INTO_OBJREF(me, iface::cellml_api::Model, modelElement());

      vr = already_AddRefd<iface::cellml_api::VariableRef>
        (me->createVariableRef());
      vr->variableName(varName);
      addElement(vr);
      vr->add_ref();
      return vr.getPointer();
    }

    RETURN_INTO_WSTRING(vn, vr->variableName());
    if (vn == varName)
    {
      vr->add_ref();
      return vr.getPointer();
    }
  }
}

already_AddRefd<iface::cellml_api::Role>
CDA_Reaction::getRoleByDeltaVariable(const std::wstring& deltav)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(vrs, iface::cellml_api::VariableRefSet,
                     variableReferences());
  RETURN_INTO_OBJREF(vri, iface::cellml_api::VariableRefIterator,
                     vrs->iterateVariableRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(vr, iface::cellml_api::VariableRef,
                       vri->nextVariableRef());
    if (vr == NULL)
      return NULL;

    RETURN_INTO_OBJREF(vrs, iface::cellml_api::VariableRefSet,
                       variableReferences());
    RETURN_INTO_OBJREF(vri, iface::cellml_api::VariableRefIterator,
                       vrs->iterateVariableRefs());
    RETURN_INTO_OBJREF(rs, iface::cellml_api::RoleSet, vr->roles());
    RETURN_INTO_OBJREF(ri, iface::cellml_api::RoleIterator, rs->iterateRoles());
    while (true)
    {
      RETURN_INTO_OBJREF(role, iface::cellml_api::Role, ri->nextRole());
      if (role == NULL)
        break;
      RETURN_INTO_WSTRING(dvn, role->deltaVariableName());
      if (dvn == deltav)
      {
        role->add_ref();
        return role.getPointer();
      }
    }
  }
}

CDA_VariableRef::~CDA_VariableRef()
{
  if (mRoleSet != NULL)
    delete mRoleSet;
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDA_VariableRef::variable()
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(vn, datastore->getAttributeNS(NULL_NS, L"variable"));

  // Find the component...
  CDA_Reaction* r =
    dynamic_cast<CDA_Reaction*>(mParent);
  if (r == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of variable_ref is not a reaction.");
  iface::cellml_api::CellMLComponent* c =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(r->mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of reaction is not a component.");

  // Find the variable...
  RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet, c->variables());
  RETURN_INTO_OBJREF(v, iface::cellml_api::NamedCellMLElement,
                     vs->get(vn.c_str()));
  if (v == NULL)
    throw iface::cellml_api::CellMLException(L"Cannot find variable referenced by variable_ref.");

  iface::cellml_api::CellMLVariable* rv =
    dynamic_cast<iface::cellml_api::CellMLVariable*>(v.getPointer());
  rv->add_ref();

  return rv;
}

void CDA_VariableRef::variable(iface::cellml_api::CellMLVariable* v) throw(std::exception&)
{
  // Check the variable is in the same component...
  CDA_CellMLVariable* cv = dynamic_cast<CDA_CellMLVariable*>(v);
  if (cv == NULL)
    throw iface::cellml_api::CellMLException(L"Variable is not from this implementation.");
  CDA_CellMLElement* o1 = cv->mParent;
  CDA_Reaction* r = dynamic_cast<CDA_Reaction*>(mParent);

  if (o1 == NULL)
    throw iface::cellml_api::CellMLException(L"Variable has no parent.");

  if (r == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of variable_ref isn't a reaction.");

  if (r->mParent == NULL)
    throw iface::cellml_api::CellMLException(L"Reaction has no parent.");

  if (CDA_objcmp(r->mParent, o1) != 0)
    throw iface::cellml_api::CellMLException(L"Reaction and variable are not in the same component.");

  RETURN_INTO_WSTRING(vn, v->name());

  variableName(vn.c_str());
}

std::wstring
CDA_VariableRef::variableName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"variable");
}

void
CDA_VariableRef::variableName(const std::wstring& varName)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"variable", varName);
}

already_AddRefd<iface::cellml_api::RoleSet>
CDA_VariableRef::roles()
  throw(std::exception&)
{
  if (mRoleSet != NULL)
  {
    mRoleSet->add_ref();
    return mRoleSet;
  }

  RETURN_INTO_OBJREFUD(allChildren, CDA_CellMLElementSet, childElements());

  mRoleSet = new CDA_RoleSet(this, allChildren);
  return mRoleSet;
}

iface::cellml_api::Role::RoleType
CDA_Role::variableRole()
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(role, datastore->getAttributeNS(NULL_NS, L"role"));
  if (role == L"")
    throw iface::cellml_api::CellMLException(L"role attribute is empty.");
  wchar_t c = role[0];
  // ACTIVATOR CATALYST INHIBITOR MODIFIER PRODUCT RATE REACTANT
  if (c >= L'm')
  {
    if (c == L'm')
    {
      if (role == L"modifier")
        return iface::cellml_api::Role::MODIFIER;
    }
    else if (c == L'p')
    {
      if (role == L"product")
        return iface::cellml_api::Role::PRODUCT;
    }
    else if (c == L'r')
    {
      if (role == L"rate")
        return iface::cellml_api::Role::RATE;
      else if (role == L"reactant")
        return iface::cellml_api::Role::REACTANT;
    }
  }
  else
  {
    if (c == L'a')
    {
      if (role == L"activator")
        return iface::cellml_api::Role::ACTIVATOR;
    }
    else if (c == L'c')
    {
      if (role == L"catalyst")
        return iface::cellml_api::Role::CATALYST;
    }
    else if (c == L'i' && role == L"inhibitor")
      return iface::cellml_api::Role::INHIBITOR;
  }
  throw iface::cellml_api::CellMLException(L"role has invalid value.");
}

void
CDA_Role::variableRole
(
 iface::cellml_api::Role::RoleType attr
)
  throw(std::exception&)
{
  const wchar_t* str;
  switch (attr)
  {
  case iface::cellml_api::Role::REACTANT:
    str = L"reactant";
    break;
  case iface::cellml_api::Role::PRODUCT:
    str = L"product";
    break;
  case iface::cellml_api::Role::RATE:
    str = L"rate";
    break;
  case iface::cellml_api::Role::CATALYST:
    str = L"catalyst";
    break;
  case iface::cellml_api::Role::ACTIVATOR:
    str = L"activator";
    break;
  case iface::cellml_api::Role::INHIBITOR:
    str = L"inhibitor";
    break;
  case iface::cellml_api::Role::MODIFIER:
    str = L"modifier";
    break;
  default:
    throw iface::cellml_api::CellMLException(L"Role set to invalid value.");
  }

  datastore->setAttributeNS(NULL_NS, L"role", str);
}

iface::cellml_api::Role::DirectionType
CDA_Role::direction()
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(str, datastore->getAttributeNS(NULL_NS, L"direction"));
  if (str == L"" || str == L"forward")
    return iface::cellml_api::Role::FORWARD;
  else if (str == L"reverse")
    return iface::cellml_api::Role::REVERSE;
  else if (str == L"both")
    return iface::cellml_api::Role::BOTH;
  throw iface::cellml_api::CellMLException(L"Direction attribute has invalid value.");
}

void
CDA_Role::direction(iface::cellml_api::Role::DirectionType dt)
  throw(std::exception&)
{
  const wchar_t* str;
  switch (dt)
  {
  case iface::cellml_api::Role::FORWARD:
    str = L"forward";
    break;
  case iface::cellml_api::Role::REVERSE:
    str = L"reverse";
    break;
  case iface::cellml_api::Role::BOTH:
    str = L"both";
    break;
  default:
    throw iface::cellml_api::CellMLException(L"Direction set to invalid value.");
  }

  datastore->setAttributeNS(NULL_NS, L"direction", str);
}

double
CDA_Role::stoichiometry()
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  double s;
  RETURN_INTO_WSTRING(str, datastore->getAttributeNS(NULL_NS,
                                                     L"stoichiometry"));
  s = wcstod(str.c_str(), NULL);
  return s;
}

void
CDA_Role::stoichiometry(double attr)
  throw(std::exception&)
{
  // Scoped locale change.
  CNumericLocale locobj;

  wchar_t buf[40];
  any_swprintf(buf, 40, L"%g", attr);
  buf[39] = 0;
  datastore->setAttributeNS(NULL_NS, L"stoichiometry", buf);
}

already_AddRefd<iface::cellml_api::CellMLVariable>
CDA_Role::deltaVariable()
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(dvn, deltaVariableName());

  CDA_VariableRef* vr = dynamic_cast<CDA_VariableRef*>(mParent);
  if (vr == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of role isn't a variable_ref.");
  CDA_Reaction* r = dynamic_cast<CDA_Reaction*>(vr->mParent);
  if (r == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of variable_ref isn't a reaction.");
  iface::cellml_api::CellMLComponent* c =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(r->mParent);
  if (c == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of reaction isn't a component.");

  RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet, c->variables());
  return vs->getVariable(dvn.c_str());
}

void
CDA_Role::deltaVariable(iface::cellml_api::CellMLVariable* attr)
  throw(std::exception&)
{
  CDA_VariableRef* vr = dynamic_cast<CDA_VariableRef*>(mParent);
  if (vr == NULL)
    throw iface::cellml_api::CellMLException(L"The parent of role isn't a variable_ref.");
  CDA_Reaction* r = dynamic_cast<CDA_Reaction*>(vr->mParent);
  if (r == NULL)
    throw iface::cellml_api::CellMLException(L"The parent of variable_ref isn't a reaction.");
  iface::cellml_api::CellMLComponent* c =
    dynamic_cast<iface::cellml_api::CellMLComponent*>(r->mParent);

  CDA_CellMLVariable* cv = dynamic_cast<CDA_CellMLVariable*>(attr);
  if (cv == NULL || cv->mParent == NULL)
    throw iface::cellml_api::CellMLException(L"Variable not from this implementation.");

  if (CDA_objcmp(cv->mParent, c) != 0)
  {
    throw iface::cellml_api::CellMLException(L"delta_variable is not from the same component as role.");
  }

  RETURN_INTO_WSTRING(dvn, cv->name());
  deltaVariableName(dvn.c_str());
}

std::wstring
CDA_Role::deltaVariableName()
  throw(std::exception&)
{
  return datastore->getAttributeNS(NULL_NS, L"delta_variable");
}

void
CDA_Role::deltaVariableName(const std::wstring& attr)
  throw(std::exception&)
{
  datastore->setAttributeNS(NULL_NS, L"delta_variable", attr);
}

uint32_t
CDA_CellMLElementSetUseIteratorMixin::length()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(cei, iface::cellml_api::CellMLElementIterator, iterate());

  uint32_t length = 0;
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
    if (ce == NULL)
      return false;
    if (CDA_objcmp(ce, x) == 0)
      return true;
  }
}

CDA_DOMElementIteratorBase::CDA_DOMElementIteratorBase
(
 iface::dom::Element* parentElement
)
  : mPrevElement(NULL), mNextElement(NULL), mParentElement(parentElement),
    icml(this)
{
  mParentElement->add_ref();
  mNodeList = mParentElement->childNodes();
  DECLARE_QUERY_INTERFACE_OBJREF(targ, mParentElement, events::EventTarget);
  targ->addEventListener(L"DOMNodeInserted", &icml, false);
}

CDA_DOMElementIteratorBase::~CDA_DOMElementIteratorBase()
{
  mNodeList->release_ref();

  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mParentElement, events::EventTarget);
    targ->removeEventListener(L"DOMNodeInserted", &icml, false);
  }
  mParentElement->release_ref();

  if (mNextElement != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mNextElement, events::EventTarget);
    targ->removeEventListener(L"DOMNodeRemoved", &icml, false);
    mNextElement->release_ref();
    mNextElement = NULL;
  }
  if (mPrevElement != NULL)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(targ, mPrevElement, events::EventTarget);
    targ->removeEventListener(L"DOMNodeRemoved", &icml, false);
    mPrevElement->release_ref();
  }
}

already_AddRefd<iface::dom::Element>
CDA_DOMElementIteratorBase::fetchNextElement()
{
  try
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
      mPrevElement->release_ref();
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

    return mPrevElement;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::dom::Element>
CDA_DOMElementIteratorBase::fetchNextElement(const std::wstring& aWantEl)
{
  try
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
      mPrevElement->release_ref();
      mPrevElement = mNextElement;
      mNextElement = NULL;
    }

    // We now have a valid previous element, which will be our return value.
    // However, to maintain our assumptions, we need to find mNextElement.
    RETURN_INTO_OBJREF(nodeHit, iface::dom::Node, mPrevElement->nextSibling());

    while (nodeHit != NULL)
    {
      RETURN_INTO_WSTRING(elN, nodeHit->localName());
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

    return mPrevElement;
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

void
CDA_DOMElementIteratorBase::IteratorChildrenModificationListener::
handleEvent(iface::events::Event* evt)
  throw(std::exception&)
{
  try
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
        printf("Warning: Unexpected event sent to handler.\n");
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
        mIterator->mPrevElement->release_ref();
        mIterator->mPrevElement = NULL;
        while (true)
        {
          if (nodeHit == NULL)
          {
            // If we just deleted the first element, reset to the initial
            // iterator state...
            if (mIterator->mNextElement != NULL)
            {
              mIterator->mNextElement->release_ref();
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
        mIterator->mNextElement->release_ref();
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
          dynamic_cast<void*>(mIterator->mParentElement))
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

        if (curE == NULL)
          printf("Something is wrong: we got from the previous node to the "
                 "last node without passing the next node, but the next node "
                 "is non-null!\n");
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
        mIterator->mNextElement->release_ref();
      }
      mIterator->mNextElement = curE;
      mIterator->mNextElement->add_ref();
      DECLARE_QUERY_INTERFACE_OBJREF(targ, mIterator->mNextElement,
                                     events::EventTarget);
      targ->addEventListener(L"DOMNodeRemoved", this, false);
    }
  }
  catch (iface::dom::DOMException& de)
  {
    throw iface::cellml_api::CellMLException(L"DOM exception caught.");
  }
}

already_AddRefd<iface::cellml_api::CellMLElementIterator>
CDA_AllUnitsSet::iterate()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(localIterator, iface::cellml_api::CellMLElementIterator,
                     mLocalSet->iterate());
  RETURN_INTO_OBJREF(importIterator, iface::cellml_api::CellMLElementIterator,
                     mImportSet->iterate());
  return new CDA_AllUnitsIterator(localIterator, importIterator,
                                  mRecurseIntoImports);
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_AllUnitsIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    RETURN_INTO_OBJREF(n, iface::cellml_api::CellMLElement, mLocalIterator->next());
    if (n != NULL)
    {
      if (mRecurseIntoImports)
      {
        CDA_ImportUnits* iu = dynamic_cast<CDA_ImportUnits*>(n.getPointer());
        if (iu != NULL)
        {
          // We have an import units. Ignore it unless we can't go any
          // deeper due to non-instantiated imports.
          if ((dynamic_cast<CDA_CellMLImport*>(iu->mParent))->mImportedModel
              != NULL)
          {
            continue;
          }
        }
      }
      n->add_ref();
      return n.getPointer();
    }

    // We have run out of elements, so we have to find the next import.
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLElement,
                       mImportIterator->next());

    // If there are no more imports left, we are done...
    if (imp == NULL)
      return NULL;

    CDA_CellMLImport* impintern =
      unsafe_dynamic_cast<CDA_CellMLImport*>(imp.getPointer());
    // If the import hasn't been instantiated, iterate its ImportUnits
    if (!mRecurseIntoImports || impintern->mImportedModel == NULL)
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
                       impintern->mImportedModel->allUnits());

    mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
      (cu->iterate());
  }
}

already_AddRefd<iface::cellml_api::CellMLElementIterator>
CDA_AllComponentSet::iterate()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(localIterator, iface::cellml_api::CellMLElementIterator,
                     mLocalSet->iterate());
  RETURN_INTO_OBJREF(importIterator, iface::cellml_api::CellMLElementIterator,
                     mImportSet->iterate());
  return new CDA_AllComponentIterator(localIterator, importIterator,
                                      mRecurseIntoImports);
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_AllComponentIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    RETURN_INTO_OBJREF(n, iface::cellml_api::CellMLElement,
                       mLocalIterator->next());
    if (n != NULL)
    {
      if (mRecurseIntoImports)
      {

        CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
          (n.getPointer());
        if (ic != NULL)
        {
          // We have an import units. Ignore it unless we can't go any
          // deeper due to non-instantiated imports.
          if ((dynamic_cast<CDA_CellMLImport*>(ic->mParent))->mImportedModel
              != NULL)
          {
            continue;
          }
        }
      }
      n->add_ref();
      return n.getPointer();
    }

    // We have run out of elements, so we have to find the next import.
    RETURN_INTO_OBJREF(imp, iface::cellml_api::CellMLElement,
                       mImportIterator->next());

    // If there are no more imports left, we are done...
    if (imp == NULL)
      return NULL;

    CDA_CellMLImport* impintern =
      unsafe_dynamic_cast<CDA_CellMLImport*>(imp.getPointer());
    // If the import hasn't been instantiated, iterate the ImportComponents.
    if (!mRecurseIntoImports || impintern->mImportedModel == NULL)
    {
      RETURN_INTO_OBJREF(cu, iface::cellml_api::CellMLElementSet,
                         impintern->components());

      mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
        (cu->iterate());
      continue;
    }
    // We have found an instantiated import, so we need to iterate its
    // childUnits. The use of allComponents instead of localComponents is
    // important, as that means the iterator will produce a pre-order traversal
    // of the entire tree. We re-use mLocalIterator to store this.
    RETURN_INTO_OBJREF(cu, iface::cellml_api::CellMLElementSet,
                       impintern->mImportedModel->allComponents());

    mLocalIterator = already_AddRefd<iface::cellml_api::CellMLElementIterator>
      (cu->iterate());
  }
}

already_AddRefd<iface::cellml_api::CellMLElementIterator>
CDA_CellMLComponentFromComponentRefSet::iterate()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(compRefIterator, iface::cellml_api::ComponentRefIterator,
                     mCompRefSet->iterateComponentRefs());
  return new CDA_CellMLComponentFromComponentRefIterator
    (mModel, compRefIterator);
}

already_AddRefd<iface::cellml_api::CellMLElement>
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
      throw iface::cellml_api::CellMLException(L"Cannot find component named in component_ref.");

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
      dynamic_cast<CDA_CellMLImport*>(ic->mParent)->mImportedModel;

    // Make do with the import component if not instantiated.
    if (currentModel == NULL)
    {
      iface::cellml_api::CellMLComponent* rc = c;
      rc->add_ref();
      return rc;
    }

    compName = ic->componentRef();
  }
}

already_AddRefd<iface::cellml_api::CellMLElementIterator>
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
    throw iface::cellml_api::CellMLException(L"Parent of variable isn't a component.");

  CDA_Model* m = dynamic_cast<CDA_Model*>(comp->mParent);
  if (m == NULL)
    throw iface::cellml_api::CellMLException(L"Parent of component isn't model.");

  RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet, m->connections());
  RETURN_INTO_OBJREF(ci, iface::cellml_api::ConnectionIterator,
                     cs->iterateConnections());

  VariableStackFrame* topFrame = new VariableStackFrame();
  topFrame->whichVariable = aConnectedToWhat;
  topFrame->whichComponent = comp;
  topFrame->whichCompLevel = topFrame->whichComponent;
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
      throw iface::cellml_api::CellMLException(L"Parent of import component isn't an import.");

    if (ci->mImportedModel == NULL)
      throw iface::cellml_api::CellMLException(L"Import is not instantiated.");

    RETURN_INTO_WSTRING(compRef, ic->componentRef());

    RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                       ci->mImportedModel->modelComponents());
    comp1 = already_AddRefd<iface::cellml_api::CellMLComponent>
      (cs->getComponent(compRef.c_str()));

    if (comp1 == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find component named in import.");
  }
  while (true)
  {
    CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
      (comp2.getPointer());
    if (ic == NULL)
      break;

    CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
    if (ci == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of import component is not import.");

    if (ci->mImportedModel == NULL)
      throw iface::cellml_api::CellMLException(L"Import is not instantiated.");

    RETURN_INTO_WSTRING(compRef, ic->componentRef());

    RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                       ci->mImportedModel->modelComponents());
    comp2 = already_AddRefd<iface::cellml_api::CellMLComponent>
      (cs->getComponent(compRef.c_str()));

    if (comp2 == NULL)
      throw iface::cellml_api::CellMLException(L"Cannot find component referenced in import.");
  }

  // We now have the two real components. Dynamic cast to the implementation
  // class and see if they are identical.
  return (dynamic_cast<void*>(comp1.getPointer()) ==
          dynamic_cast<void*>(comp2.getPointer()));
}

already_AddRefd<iface::cellml_api::CellMLElement>
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
        // No more connections in the current model. But, the model could be
        // imported, and the connection may be at the next level up...
        CDA_CellMLElement* el = dynamic_cast<CDA_CellMLElement*>
          (topFrame->whichCompLevel.getPointer());
        CDA_Model* m =
          dynamic_cast<CDA_Model*>(el->mParent);
        if (m == NULL)
        {
          CDA_CellMLImport* impold =
          dynamic_cast<CDA_CellMLImport*>(el->mParent);
          m = dynamic_cast<CDA_Model*>(impold->mParent);
        }
        if (m != NULL)
        {
          CDA_CellMLImport* imp =
            dynamic_cast<CDA_CellMLImport*>(m->mParent);
          if (imp != NULL)
          {
            // Check for our name in the import...
            RETURN_INTO_OBJREF(c, iface::cellml_api::ImportComponentSet,
                               imp->components());
            RETURN_INTO_WSTRING(n, topFrame->whichCompLevel->name());
            RETURN_INTO_OBJREF(ci, iface::cellml_api::ImportComponentIterator,
                               c->iterateImportComponents());
            ObjRef<iface::cellml_api::ImportComponent> comp;
            while (true)
            {
              comp = already_AddRefd<iface::cellml_api::ImportComponent>
                (ci->nextImportComponent());
              if (comp == NULL)
                break;
              RETURN_INTO_WSTRING(cr, comp->componentRef());
              if (cr == n)
                break;
            }
            if (comp != NULL)
            {
              CDA_Model* m = dynamic_cast<CDA_Model*>(imp->mParent);
              if (m != NULL)
              {
                RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet,
                                   m->connections());
                RETURN_INTO_OBJREF(ci, iface::cellml_api::ConnectionIterator,
                                   cs->iterateConnections());
                topFrame->connectionIterator = ci;
                topFrame->whichCompLevel = comp;
                continue;
              }
            }
          }
        }

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
      {
        continue;
      }
      mConsider1 = (CompareComponentsImportAware
                    (c1, topFrame->whichComponent));

      RETURN_INTO_OBJREF(c2, iface::cellml_api::CellMLComponent,
                         cm->secondComponent());
      if (c2 == NULL)
      {
        continue;
      }
      mConsider2 = (CompareComponentsImportAware
                    (c2, topFrame->whichComponent));

      // There is no point even looking at this connection if neither component
      // matches.
      if (!mConsider1 && !mConsider2)
      {
        continue;
      }

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
    if (mv == NULL)
    {
      topFrame->mapVariableIterator = NULL;
      continue;
    }

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
        // See if variable_1 has already been found...
        vother = already_AddRefd<iface::cellml_api::CellMLVariable>
          (mv->firstVariable());
        if (vother == NULL)
          continue;
      }
    }
    if (vother == NULL)
      continue;

    CDA_CellMLVariable *votherImpl =
      unsafe_dynamic_cast<CDA_CellMLVariable*>(vother.getPointer());
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
      throw iface::cellml_api::CellMLException(L"Parent of variable isn't a component.");

    CDA_Model* m = dynamic_cast<CDA_Model*>(comp->mParent);
    if (m == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of component isn't a model.");

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
    newFrame->whichCompLevel = newFrame->whichComponent;
    variableStack.push_front(newFrame);

    // Finally, return the variable we found...
    vother->add_ref();
    return vother.getPointer();
  }
}

CDA_CellMLElementIterator::CDA_CellMLElementIterator
(
 iface::dom::Element* parentElement,
 CDA_CellMLElementSet* ownerSet
)
  : CDA_DOMElementIteratorBase(parentElement),
    parentSet(ownerSet)
{
  parentSet->add_ref();
}

CDA_CellMLElementIterator::~CDA_CellMLElementIterator()
{
  parentSet->release_ref();
}

static already_AddRefd<CDA_CellMLElement>
WrapCellMLElement(CDA_CellMLElement* newParent,
                  iface::dom::Element* el)
  throw(std::exception&)
{
  RETURN_INTO_WSTRING(ln, el->localName());
  RETURN_INTO_OBJREF(pn, iface::dom::Node, el->parentNode());
  std::wstring pln;
  if (pn != NULL)
    pln = pn->localName();

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
  else if (ln == L"variable_ref")
  {
    return new CDA_VariableRef(newParent, el);
  }
  else if (ln == L"role")
  {
    return new CDA_Role(newParent, el);
  }
  // Role goes here once we fix everything up.
  else
  {
    // We are being asked to wrap something that shouldn't be here. What should
    // we do? For now I will raise a CellMLException.
    throw iface::cellml_api::CellMLException(L"Attempt to wrap invalid type of element as CellML element.");
  }
}

already_AddRefd<iface::cellml_api::CellMLElement>
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
  std::map<iface::dom::Element*,CDA_CellMLElement*, XPCOMComparator>
     ::iterator i = parentSet->childMap.find(el);
  if (i != parentSet->childMap.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We have an unwrapped element, so we need to wrap it...
  CDA_CellMLElement* cel =
    WrapCellMLElement(parentSet->mParent, el);
  parentSet->addChildToWrapper(cel);

  return cel;
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_CellMLElementIterator::next(const std::wstring& aWantEl)
  throw(std::exception&)
{
  iface::dom::Element* el;
  while (true)
  {
    el = fetchNextElement(aWantEl);
    if (el == NULL)
    {
      return NULL;
    }
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS || nsURI == CELLML_1_1_NS)
      break;
  }

  // We have an element. Now go back to the set, and look in the map...
  std::map<iface::dom::Element*,CDA_CellMLElement*, XPCOMComparator>
     ::iterator i = parentSet->childMap.find(el);
  if (i != parentSet->childMap.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We have an unwrapped element, so we need to wrap it...
  CDA_CellMLElement* cel =
    WrapCellMLElement(parentSet->mParent, el);
  parentSet->addChildToWrapper(cel);

  return cel;
}

already_AddRefd<iface::mathml_dom::MathMLElement>
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
{
  nl = el->childNodes();
}

CDA_ExtensionElementList::~CDA_ExtensionElementList()
{
  nl->release_ref();
}

uint32_t
CDA_ExtensionElementList::length()
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  uint32_t le = 0, l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS ||
        nsURI == CELLML_1_1_NS ||
        nsURI == RDF_NS ||
        nsURI == MATHML_NS)
      continue;

    le++;
  }

  return le;
}

bool
CDA_ExtensionElementList::contains(iface::dom::Element* x)
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  uint32_t l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;

    if (CDA_objcmp(el, x) == 0)
      return true;
  }

  return false;
}

int32_t
CDA_ExtensionElementList::getIndexOf(iface::dom::Element* x)
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  uint32_t le = 0, l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS ||
        nsURI == CELLML_1_1_NS ||
        nsURI == RDF_NS ||
        nsURI == MATHML_NS)
      continue;

    if (el == x)
      return le;

    le++;
  }

  return -1;
}

already_AddRefd<iface::dom::Element>
CDA_ExtensionElementList::getAt(uint32_t index)
  throw(std::exception&)
{
  // Anything except CellML 1.0 / CellML 1.1 / RDF elements...
  uint32_t l = nl->length(), i;
  for (i = 0; i < l; i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      continue;
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    if (nsURI == CELLML_1_0_NS ||
        nsURI == CELLML_1_1_NS ||
        nsURI == RDF_NS ||
        nsURI == MATHML_NS)
      continue;

    if (index == 0)
    {
      el->add_ref();
      return el.getPointer();
    }

    index--;
  }

  return NULL;
}

CDA_MathList::CDA_MathList(iface::dom::Element* aParentEl)
  : mParentEl(aParentEl)
{
  mParentEl->add_ref();
}

CDA_MathList::~CDA_MathList()
{
  mParentEl->release_ref();
}

uint32_t
CDA_MathList::length()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathMLElementIterator, iterate());
  uint32_t l = 0;
  while (true)
  {
    RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, ml->next());
    if (me == NULL)
      return l;
    l++;
  }
}

bool
CDA_MathList::contains(iface::mathml_dom::MathMLElement* x)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathMLElementIterator, iterate());
  while (true)
  {
    RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, ml->next());
    if (me == NULL)
      return false;
    iface::mathml_dom::MathMLElement* xme =
      static_cast<iface::mathml_dom::MathMLElement*>(x);
    if (CDA_objcmp(me, xme) == 0)
      return true;
  }
}

already_AddRefd<iface::cellml_api::MathMLElementIterator>
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
  : mParent(parent), mElement(parentEl), descendentSerial(0)
{
  // Note: The reference count starts at zero, because an Element is
  // permanently part of an Element, and so needs no refcount when it is
  // constructed.
  --_cda_refcount;
}

CDA_CellMLElementSet::~CDA_CellMLElementSet()
{
  if (_cda_refcount != 0)
    printf("Warning: release_ref called too few times on %s.\n",
           typeid(this).name());

  std::map<iface::dom::Element*,CDA_CellMLElement*,XPCOMComparator>::iterator
    i;

  for (i = childMap.begin(); i != childMap.end(); i++)
    delete (*i).second;
}

already_AddRefd<iface::cellml_api::CellMLElementIterator>
CDA_CellMLElementSet::iterate()
  throw(std::exception&)
{
  return new CDA_CellMLElementIterator(mElement, this);
}

void
CDA_CellMLElementSet::addChildToWrapper(CDA_CellMLElement* el)
{
  DECLARE_QUERY_INTERFACE_OBJREF(targ, el->datastore, events::EventTarget);
  targ->removeEventListener(L"DOMSubtreeModified", &gCDAChangeListener, false);
  childMap.insert(std::pair<iface::dom::Element*,
                            CDA_CellMLElement*>
                  (el->datastore, el));
}

void
CDA_CellMLElementSet::removeChildFromWrapper(CDA_CellMLElement* el)
{
  DECLARE_QUERY_INTERFACE_OBJREF(targ, el->datastore, events::EventTarget);
  targ->addEventListener(L"DOMSubtreeModified", &gCDAChangeListener, false);
  childMap.erase(el->datastore);
}

void
CDA_CellMLElementSet::dumpRootCaches()
{
  descendentMap.clear();
}

iface::cellml_api::CellMLElement*
CDA_CellMLElementSet::searchDescendents(iface::dom::Element* aEl)
{
  if (descendentSerial != gCDAChangeSerial)
  {
    dumpRootCaches();
    populateDescendentCache(descendentMap);
    descendentSerial = gCDAChangeSerial;
  }

  std::map<iface::dom::Element*, CDA_CellMLElement*,XPCOMComparator>::iterator
    i(descendentMap.find(aEl));
  if (i == descendentMap.end())
    return NULL;

  i->second->add_ref();
  return i->second;
}

void
CDA_CellMLElementSet::populateDescendentCache(std::map<iface::dom::Element*,CDA_CellMLElement*,XPCOMComparator>& aMap)
{
  ObjRef<iface::cellml_api::CellMLElementIterator> it(iterate());
  ObjRef<iface::cellml_api::CellMLElement> el;
  do
  {
    el = it->next();
  } while (el);

  std::map<iface::dom::Element*,CDA_CellMLElement*,XPCOMComparator>::iterator i;
  for (i = childMap.begin(); i != childMap.end(); i++)
  {
    aMap.insert(*i);
    if (i->second->children == NULL)
      i->second->children = new CDA_CellMLElementSet(i->second, i->first);
    i->second->children->populateDescendentCache(aMap);
  }
}

already_AddRefd<iface::cellml_api::NamedCellMLElement>
CDA_NamedCellMLElementSetBase::get(const std::wstring& name)
  throw(std::exception&)
{
  bool multipleCalls = false;
  if (CDA_CompareSerial(mCacheSerial))
  {
    std::map<std::wstring, iface::cellml_api::NamedCellMLElement*>::iterator i
      = mMap.find(name);
    if (i != mMap.end())
    {
      (*i).second->add_ref();
      return (*i).second;
    }
    if (mCacheComplete)
      return NULL;
    multipleCalls = true;
  }
  else
  {
    mCacheSerial = gCDAChangeSerial;
    mCacheComplete = false;
    mHighWaterMark = 0;
    mMap.clear();
  }

  RETURN_INTO_OBJREF(elIt, iface::cellml_api::CellMLElementIterator, iterate());
  uint32_t count = 0;
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, elIt->next());
    if (el == NULL)
    {
      mCacheComplete = true;
      return NULL;
    }
    DECLARE_QUERY_INTERFACE_OBJREF(nel, el, cellml_api::NamedCellMLElement);

    // If nel == null, you are missing a NamedCellMLElement QI item on your
    // implementation.

    std::wstring n = nel->name();
    bool match = n == name;
    count++;
    if (count > mHighWaterMark)
    {
      mMap.insert(std::pair<std::wstring,
                            iface::cellml_api::NamedCellMLElement*>(n, nel));
      mHighWaterMark = count;
    }

    if (match)
    {
      // If we have had more than one call, then as a heuristic assume we will
      // have more, and finish the cache just so we don't have to keep going
      // back to the start...
      if (multipleCalls)
      {
        while (true)
        {
          RETURN_INTO_OBJREF(tel, iface::cellml_api::CellMLElement,
                             elIt->next());
          if (tel == NULL)
            break;
          count++;
          if (count > mHighWaterMark)
          {
            DECLARE_QUERY_INTERFACE_OBJREF(tnel, tel,
                                           cellml_api::NamedCellMLElement);
            // If tnel == null, you are missing a NamedCellMLElement QI item on your
            // implementation.
	    std::wstring tn = tnel->name();
            mMap.insert(std::pair<std::wstring,
                        iface::cellml_api::NamedCellMLElement*>(tn, tnel));
            mHighWaterMark = count;
          }
        }
        mCacheComplete = true;
      }

      nel->add_ref();
      return nel.getPointer();
    }
  }
}

#define SIMPLE_SET_ITERATORFETCH(setname, iteratorname, ifacename, funcname) \
already_AddRefd<iface::cellml_api::CellMLElementIterator> \
setname::iterate() \
  throw(std::exception&) \
{ \
  CDA_CellMLElementIterator* ei = dynamic_cast<CDA_CellMLElementIterator*>(mInner->iterate().getPointer()); \
  iface::cellml_api::CellMLElementIterator* it = new iteratorname(ei); \
  ei->release_ref(); \
  return it; \
} \
already_AddRefd<ifacename>			\
setname::funcname() \
  throw(std::exception&) \
{ \
  return static_cast<ifacename*>(unsafe_dynamic_cast<iteratorname*>(iterate().getPointer())); \
}

#define SIMPLE_SET_ITERATORFETCH_GET_B(setname, iteratorname, ifacename, funcname, basename, getname) \
SIMPLE_SET_ITERATORFETCH(setname, iteratorname, ifacename, funcname) \
already_AddRefd<basename> \
setname::getname(const std::wstring& name) \
  throw(std::exception&) \
{ \
  return dynamic_cast<basename*>(get(name).getPointer()); \
}

#define SIMPLE_SET_ITERATORFETCH_GET(setname, iteratorname, ifacename, funcname, classname, basename, getname) \
SIMPLE_SET_ITERATORFETCH(setname, iteratorname, ifacename, funcname) \
already_AddRefd<basename>					     \
setname::getname(const std::wstring& name)				     \
  throw(std::exception&) \
{ \
  return static_cast<basename*>(unsafe_dynamic_cast<classname*>(get(name).getPointer())); \
}

SIMPLE_SET_ITERATORFETCH_GET_B
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
 CDA_ImportComponent,
 iface::cellml_api::ImportComponent,
 getImportComponent
);

SIMPLE_SET_ITERATORFETCH_GET
(
 CDA_CellMLVariableSet,
 CDA_CellMLVariableIterator,
 iface::cellml_api::CellMLVariableIterator,
 iterateVariables,
 CDA_CellMLVariable,
 iface::cellml_api::CellMLVariable,
 getVariable
);

SIMPLE_SET_ITERATORFETCH_GET_B
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
 CDA_ImportUnits,
 iface::cellml_api::ImportUnits,
 getImportUnits
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_CellMLImportSet,
 CDA_CellMLImportIterator,
 iface::cellml_api::CellMLImportIterator,
 iterateImports
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

SIMPLE_SET_ITERATORFETCH
(
 CDA_ReactionSet,
 CDA_ReactionIterator,
 iface::cellml_api::ReactionIterator,
 iterateReactions
);

already_AddRefd<iface::cellml_api::CellMLElementIterator>
CDA_GroupSet::iterate()
  throw(std::exception&)
{
  CDA_CellMLElementIterator* ei =
    unsafe_dynamic_cast<CDA_CellMLElementIterator*>
    (mInner->iterate().getPointer());
  iface::cellml_api::CellMLElementIterator* it =
    new CDA_GroupIterator(ei, filterByRRName,
                          mFilterRRName.c_str());
  ei->release_ref();
  return it;
}

already_AddRefd<iface::cellml_api::GroupIterator>
CDA_GroupSet::iterateGroups()
  throw(std::exception&)
{
  return static_cast<iface::cellml_api::GroupIterator*>(unsafe_dynamic_cast<CDA_GroupIterator*>(iterate().getPointer()));
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
 CDA_VariableRefSet,
 CDA_VariableRefIterator,
 iface::cellml_api::VariableRefIterator,
 iterateVariableRefs
);

SIMPLE_SET_ITERATORFETCH
(
 CDA_RoleSet,
 CDA_RoleIterator,
 iface::cellml_api::RoleIterator,
 iterateRoles
);

#define SIMPLE_ITERATOR_NEXTONLY(itname, ifacename, nextname, restrict) \
already_AddRefd<iface::cellml_api::CellMLElement>			\
itname::next() \
  throw(std::exception&) \
{ \
  while (true) \
  { \
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, mInner->next restrict); \
    if (el == NULL) \
      return NULL; \
    iface::cellml_api::ifacename* targEl; \
    QUERY_INTERFACE(targEl, el, cellml_api::ifacename); \
    if (targEl == NULL) \
      continue; \
    return targEl; \
  } \
} \

#define SIMPLE_ITERATOR_NEXT_B(itname, ifacename, nextname, restrict) \
SIMPLE_ITERATOR_NEXTONLY(itname, ifacename, nextname, restrict) \
already_AddRefd<iface::cellml_api::ifacename>			\
itname::nextname() \
  throw(std::exception&) \
{ \
  return dynamic_cast<iface::cellml_api::ifacename*>(next().getPointer()); \
}

#define SIMPLE_ITERATOR_NEXT(itname, ifacename, nextname, restrict) \
SIMPLE_ITERATOR_NEXTONLY(itname, ifacename, nextname, restrict) \
already_AddRefd<iface::cellml_api::ifacename>			\
itname::nextname() \
  throw(std::exception&) \
{ \
  return static_cast<iface::cellml_api::ifacename*>(unsafe_dynamic_cast<CDA_##ifacename*>(next().getPointer())); \
}

SIMPLE_ITERATOR_NEXT_B(CDA_CellMLComponentIteratorBase, CellMLComponent,
                       nextComponent, (L"component"));
SIMPLE_ITERATOR_NEXT(CDA_ImportComponentIterator, ImportComponent,
                     nextImportComponent, (L"component"));
SIMPLE_ITERATOR_NEXT(CDA_CellMLVariableIterator, CellMLVariable,
                     nextVariable, (L"variable"));
SIMPLE_ITERATOR_NEXT_B(CDA_UnitsIteratorBase, Units,
                       nextUnits, (L"units"));
SIMPLE_ITERATOR_NEXT(CDA_ImportUnitsIterator, ImportUnits,
                     nextImportUnits, (L"units"));
SIMPLE_ITERATOR_NEXT(CDA_CellMLImportIterator, CellMLImport,
                     nextImport, (L"import"));
SIMPLE_ITERATOR_NEXT(CDA_UnitIterator, Unit,
                     nextUnit, (L"unit"));
SIMPLE_ITERATOR_NEXT(CDA_ConnectionIterator, Connection,
                     nextConnection, (L"connection"));
SIMPLE_ITERATOR_NEXT(CDA_ReactionIterator, Reaction, nextReaction,
                     (L"reaction"));

static bool
DoesGroupHaveRelationshipRef(iface::dom::Element* el, const std::wstring& rrname)
{
  RETURN_INTO_OBJREF(cn, iface::dom::NodeList, el->childNodes());
  uint32_t i, l = cn->length();
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

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_GroupIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, mInner->next(L"group"));
    if (el == NULL)
      return NULL;
    DECLARE_QUERY_INTERFACE_OBJREF(targEl, el, cellml_api::Group);
    if (targEl == NULL)
      continue;

    CDA_Group* g = unsafe_dynamic_cast<CDA_Group*>(targEl.getPointer());

    if (filterByRRName &&
        !DoesGroupHaveRelationshipRef(g->datastore, mFilterRRName))
      continue;

    targEl->add_ref();
    return targEl.getPointer();
  }
}

already_AddRefd<iface::cellml_api::Group>
CDA_GroupIterator::nextGroup()
  throw(std::exception&)
{
  return static_cast<iface::cellml_api::Group*>(unsafe_dynamic_cast<CDA_Group*>(next().getPointer()));
}

SIMPLE_ITERATOR_NEXT(CDA_RelationshipRefIterator, RelationshipRef,
                     nextRelationshipRef, (L"relationship_ref"));
SIMPLE_ITERATOR_NEXT(CDA_ComponentRefIterator, ComponentRef,
                     nextComponentRef, (L"component_ref"));
SIMPLE_ITERATOR_NEXT(CDA_MapVariablesIterator, MapVariables,
                     nextMapVariables, (L"map_variables"));

// Deprecated method name...
already_AddRefd<iface::cellml_api::MapVariables>
CDA_MapVariablesIterator::nextMapVariable()
  throw (std::exception&)
{
  return nextMapVariables();
}

SIMPLE_ITERATOR_NEXT(CDA_VariableRefIterator, VariableRef,
                     nextVariableRef, (L"variable_ref"));
SIMPLE_ITERATOR_NEXT(CDA_RoleIterator, Role,
                     nextRole, (L"role"));

already_AddRefd<iface::cellml_api::GroupSet>
CDA_GroupSet::getSubsetInvolvingRelationship(const std::wstring& relName)
  throw(std::exception&)
{
  return new CDA_GroupSet(mInner, relName);
}

already_AddRefd<iface::cellml_api::GroupSet>
CDA_GroupSet::subsetInvolvingEncapsulation()
  throw(std::exception&)
{
  return new CDA_GroupSet(mInner, L"encapsulation");
}

already_AddRefd<iface::cellml_api::GroupSet>
CDA_GroupSet::subsetInvolvingContainment()
  throw(std::exception&)
{
  return new CDA_GroupSet(mInner, L"containment");
}

CDA_ComponentConnectionIterator::CDA_ComponentConnectionIterator
(
 iface::cellml_api::CellMLComponent *aWhatComponent
)
  : CDA_ConnectionIterator(NULL), mWhatComponent(NULL),
    mConnectionIterator(NULL)
{
  ObjRef<CDA_NamedCellMLElement> comp =
    (dynamic_cast<CDA_NamedCellMLElement*>(aWhatComponent));
  // We firstly ascend from this component to the top-most level at which it is
  // imported...
  while (true)
  {
    CDA_Model* m = dynamic_cast<CDA_Model*>(comp->mParent);
    if (m == NULL)
    {
      CDA_CellMLImport* impt = dynamic_cast<CDA_CellMLImport*>(comp->mParent);
      if (impt == NULL)
        throw iface::cellml_api::CellMLException(L"Parent of import component isn't an import.");
      m = dynamic_cast<CDA_Model*>(impt->mParent);
      if (m == NULL)
        throw iface::cellml_api::CellMLException(L"Parent of import is not a model.");
    }
    CDA_CellMLImport* imp = dynamic_cast<CDA_CellMLImport*>(m->mParent);
    if (imp == NULL)
      break;

    // Now we check the import for the component we want...
    RETURN_INTO_OBJREF(ics, iface::cellml_api::ImportComponentSet,
                       imp->components());
    RETURN_INTO_WSTRING(name, comp->name());
    RETURN_INTO_OBJREF(ici, iface::cellml_api::ImportComponentIterator,
                       ics->iterateImportComponents());
    bool found = false;
    while (true)
    {
      RETURN_INTO_OBJREF(ic, iface::cellml_api::ImportComponent,
                         ici->nextImportComponent());
      if (ic == NULL)
        break;
      RETURN_INTO_WSTRING(crname, ic->componentRef());
      if (crname != name)
        continue;
      comp = dynamic_cast<CDA_NamedCellMLElement*>
        (ic.getPointer());
      found = true;
      break;
    }
    if (!found)
      throw iface::cellml_api::CellMLException(L"Could not find component.");
  }
  mWhatComponent = dynamic_cast<iface::cellml_api::CellMLComponent*>
    (comp.getPointer());
  mWhatComponent->add_ref();

  // We now have a component, so we can get an iterator...
  CDA_Model* m =
    dynamic_cast<CDA_Model*>(comp->mParent);
  if (m == NULL)
  {
    CDA_CellMLImport* impt = dynamic_cast<CDA_CellMLImport*>(comp->mParent);
    if (impt == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of import component isn't an import.");
    m = dynamic_cast<CDA_Model*>(impt->mParent);
    if (m == NULL)
      throw iface::cellml_api::CellMLException(L"Parent of import isn't a model.");
  }
  RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet, m->connections());
  mConnectionIterator = cs->iterateConnections();
}

CDA_ComponentConnectionIterator::~CDA_ComponentConnectionIterator()
{
  if (mWhatComponent != NULL)
    mWhatComponent->release_ref();
  if (mConnectionIterator != NULL)
    mConnectionIterator->release_ref();
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_ComponentConnectionIterator::next()
  throw(std::exception&)
{
  while (true)
  {
    if (mConnectionIterator == NULL)
      return NULL;
    RETURN_INTO_OBJREF(c, iface::cellml_api::Connection,
                       mConnectionIterator->nextConnection());
    if (c == NULL)
    {
      // No more connections in this model. Try up one in the import chain...
      CDA_ImportComponent* ic = dynamic_cast<CDA_ImportComponent*>
        (mWhatComponent);
      if (ic == NULL)
        return NULL;

      CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(ic->mParent);
      if (ci == NULL)
        // Can't see it due to model structure. Should this raise an exception?
        return NULL;

      CDA_Model* m = dynamic_cast<CDA_Model*>(ci->mImportedModel);
      if (m == NULL)
        // Can't go any further because imports not instantiated...
        return NULL;

      RETURN_INTO_WSTRING(name, ic->componentRef());
      RETURN_INTO_OBJREF(comps, iface::cellml_api::CellMLComponentSet,
                         m->modelComponents());
      RETURN_INTO_OBJREF(comp, iface::cellml_api::CellMLComponent,
                         comps->getComponent(name.c_str()));
      mWhatComponent->release_ref();
      mWhatComponent = comp;
      mWhatComponent->add_ref();

      // We now have a component, so we can get an iterator...
      RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet, m->connections());
      mConnectionIterator->release_ref();
      mConnectionIterator = cs->iterateConnections();
      continue;
    }

    // If we get here, c is a connection that could apply to this component.
    // Check the names match...
    RETURN_INTO_WSTRING(myName, mWhatComponent->name());
    RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                       c->componentMapping());
    RETURN_INTO_WSTRING(comp1, cm->firstComponentName());
    RETURN_INTO_WSTRING(comp2, cm->secondComponentName());
    if (comp1 == myName || comp2 == myName)
    {
      c->add_ref();
      return c.getPointer();
    }

  }
}

/**
 * Determines if a R(aImport, aComponent) holds.
 * R(import, component) holds iff
 *   R(import, component->encapsulationParent) holds, or
 *   import references component(directly or via a chain of imports) as an
 *   import component.
 */
static bool
IsComponentRelatedToImport
(
 iface::cellml_api::CellMLImport* aImport,
 iface::cellml_api::CellMLComponent* aComponent
)
{
  ObjRef<iface::cellml_api::CellMLComponent> component = aComponent;

  while (component)
  {
    ObjRef<iface::cellml_api::CellMLComponent> ctmp = component;
    while (ctmp)
    {
      CDA_NamedCellMLElement* nce =
        dynamic_cast<CDA_NamedCellMLElement*>(ctmp.getPointer());
      if (nce == NULL)
        throw iface::cellml_api::CellMLException(L"Component isn't from this implementation.");
      RETURN_INTO_WSTRING(name, nce->name());

      CDA_Model* m = dynamic_cast<CDA_Model*>(nce->mParent);
      if (m == NULL)
      {
        CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(nce->mParent);
        if (ci == NULL)
          throw iface::cellml_api::CellMLException(L"Parent of import component isn't import.");

        m = dynamic_cast<CDA_Model*>(ci->mParent);
        if (m == NULL)
          throw iface::cellml_api::CellMLException(L"Parent of import isn't a model.");
      }

      CDA_CellMLImport* ci = dynamic_cast<CDA_CellMLImport*>(m->mParent);
      if (ci == NULL)
        break;

      // Look for this component in this import...
      RETURN_INTO_OBJREF(ics, iface::cellml_api::ImportComponentSet,
                         ci->components());
      RETURN_INTO_OBJREF(ici, iface::cellml_api::ImportComponentIterator,
                         ics->iterateImportComponents());
      bool notFound = false;
      while (true)
      {
        RETURN_INTO_OBJREF(ic, iface::cellml_api::ImportComponent,
                           ici->nextImportComponent());
        if (ic == NULL)
        {
          notFound = true;
          break;
        }
        RETURN_INTO_WSTRING(cr, ic->componentRef());
        if (cr == name)
        {
          ctmp = ic;
          break;
        }
      }

      if (notFound)
        break;

      if (CDA_objcmp(ci, aImport) == 0)
        return true;
    }
    component = already_AddRefd<iface::cellml_api::CellMLComponent>
      (component->encapsulationParent());
  }
  return false;
}

CDA_ImportConnectionIterator::~CDA_ImportConnectionIterator()
{
  while (!importStack.empty())
  {
    delete importStack.front();
    importStack.pop_front();
  }
}

void
CDA_ImportConnectionIterator::pushStackFrame
(
 iface::cellml_api::CellMLImport* aImport
)
  throw(std::exception&)
{
  CDA_CellMLImport* ic = dynamic_cast<CDA_CellMLImport*>(aImport);
  if (ic == NULL)
    throw iface::cellml_api::CellMLException(L"Import isn't from this implementation.");

  iface::cellml_api::Model* m = ic->mImportedModel;
  if (m == NULL)
    throw iface::cellml_api::CellMLException(L"Import isn't instantiated.");

  RETURN_INTO_OBJREF(cis, iface::cellml_api::CellMLImportSet, m->imports());
  RETURN_INTO_OBJREF(importIterator, iface::cellml_api::CellMLImportIterator,
                     cis->iterateImports());
  RETURN_INTO_OBJREF(conns, iface::cellml_api::ConnectionSet,
                     m->connections());
  RETURN_INTO_OBJREF(connectionIterator, iface::cellml_api::ConnectionIterator,
                     conns->iterateConnections());

  ImportStackFrame* isf = new ImportStackFrame();
  isf->mState = ImportStackFrame::DEEP_CONNECTIONS;
  isf->mImportIterator = importIterator;
  isf->mConnectionIterator = connectionIterator;
  importStack.push_front(isf);
}

already_AddRefd<iface::cellml_api::CellMLElement>
CDA_ImportConnectionIterator::next()
  throw(std::exception&)
{
  while (!importStack.empty())
  {
    ImportStackFrame* isf = importStack.front();

    while (isf->mState == ImportStackFrame::DEEP_CONNECTIONS)
    {
      RETURN_INTO_OBJREF(import, iface::cellml_api::CellMLImport,
                         isf->mImportIterator->nextImport());
      if (import == NULL)
      {
        isf->mState = ImportStackFrame::SHALLOW_CONNECTIONS;
        break;
      }
      // Now see if there is any point going into this import...
      bool hasSuitableComponent = false;
      RETURN_INTO_OBJREF(comps, iface::cellml_api::ImportComponentSet,
                         import->components());
      RETURN_INTO_OBJREF(compi, iface::cellml_api::ImportComponentIterator,
                         comps->iterateImportComponents());
      while (true)
      {
        RETURN_INTO_OBJREF(comp, iface::cellml_api::ImportComponent,
                           compi->nextImportComponent());
        if (comp == NULL)
          break;
        if (IsComponentRelatedToImport(mImport, comp))
        {
          hasSuitableComponent = true;
          break;
        }
      }
      if (!hasSuitableComponent)
        continue;

      // We now have a suitable import, so it is the next stack frame...
      pushStackFrame(import);
      isf = importStack.front();
      continue;
    }

    // We can't go deeper, so deal with this stack frame...
    while (true)
    {
      RETURN_INTO_OBJREF(conn, iface::cellml_api::Connection,
                         isf->mConnectionIterator->nextConnection());
      if (conn == NULL)
        break;

      RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                         conn->componentMapping());

      // See if this connection is a candidate to be returned...
      RETURN_INTO_OBJREF(c1, iface::cellml_api::CellMLComponent,
                         cm->firstComponent());
      if (!IsComponentRelatedToImport(mImport, c1))
        continue;
      RETURN_INTO_OBJREF(c2, iface::cellml_api::CellMLComponent,
                         cm->secondComponent());
      if (!IsComponentRelatedToImport(mImport, c2))
        continue;

      // We have a hit, so return it...
      conn->add_ref();
      return conn.getPointer();
    }

    // Nothing further in this stack frame. Remove it...
    delete isf;
    importStack.pop_front();
  }
  // If we get here, there is nothing left to return...
  return NULL;
}
