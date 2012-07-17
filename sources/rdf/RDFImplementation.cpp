#define MODULE_CONTAINS_RDFAPISPEC
#define MODULE_CONTAINS_xpcom
#define MODULE_CONTAINS_DOMAPISPEC
#define IN_DOMWRITER_MODULE
#define IN_DOM_MODULE
#include "RDFImplementation.hpp"
#include "Utilities.hxx"
#include "URITool.hpp"
#include "DOMBootstrap.hxx"
#include "DOMWriter.hxx"
#include "RDFBootstrap.hpp"

#define RDF_NS L"http://www.w3.org/1999/02/22-rdf-syntax-ns#"

CDA_DataSource::CDA_DataSource()
  : mTripleSet(new CDA_AllTriplesSet(this))
{
}

CDA_DataSource::~CDA_DataSource()
{
  for (std::set<CDA_RDFNode*, ptr_to_less<CDA_RDFNode> >::iterator i = mAssociatedNodes.begin();
       i != mAssociatedNodes.end();
       i++)
    delete *i;

  delete mTripleSet;
}

already_AddRefd<iface::rdf_api::URIReference>
CDA_DataSource::getURIReference(const std::wstring& aURI)
  throw(std::exception&)
{
  URI u(aURI);
  std::wstring abs(u.absoluteURI());

  // See if it already exists...
  std::map<std::wstring, CDA_URIReference*>::iterator i
    (mURIReferences.find(abs));

  if (i != mURIReferences.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We need to make a new URI reference...
  CDA_URIReference* ur = new CDA_URIReference(aURI, this);
  mURIReferences.insert(std::pair<std::wstring, CDA_URIReference*>(abs, ur));

  return ur;
}

already_AddRefd<iface::rdf_api::BlankNode>
CDA_DataSource::createBlankNode()
  throw(std::exception&)
{
  return new CDA_BlankNode(this);
}

already_AddRefd<iface::rdf_api::PlainLiteral>
CDA_DataSource::getPlainLiteral(const std::wstring& aForm,
				const std::wstring& aLanguage)
  throw(std::exception&)
{
  wstringpair p(aForm, aLanguage);
  std::map<wstringpair, CDA_PlainLiteral*>::iterator i(mPlainLiterals.find(p));
  if (i != mPlainLiterals.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We need to make a new plain literal...
  CDA_PlainLiteral* pl = new CDA_PlainLiteral(aForm, aLanguage, this);

  mPlainLiterals.insert(std::pair<wstringpair, CDA_PlainLiteral*>(p, pl));

  return pl;
}

already_AddRefd<iface::rdf_api::TypedLiteral>
CDA_DataSource::getTypedLiteral(const std::wstring& aForm,
                                const std::wstring& aDatatypeURI)
  throw(std::exception&)
{
  wstringpair p(aForm, aDatatypeURI);
  std::map<wstringpair, CDA_TypedLiteral*>::iterator i(mTypedLiterals.find(p));
  if (i != mTypedLiterals.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We need to make a new typed literal...
  CDA_TypedLiteral* tl = new CDA_TypedLiteral(aForm, aDatatypeURI, this);

  mTypedLiterals.insert(std::pair<wstringpair, CDA_TypedLiteral*>(p, tl));

  return tl;
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_DataSource::getAllTriples()
  throw(std::exception&)
{
  mTripleSet->add_ref();
  return mTripleSet;
}

void
CDA_DataSource::plainLiteralDeleted(CDA_PlainLiteral* aLit)
{
  std::map<wstringpair, CDA_PlainLiteral*>::iterator i
    (mPlainLiterals.find(wstringpair(aLit->mLexicalForm, aLit->mLanguage)));

  if (i == mPlainLiterals.end())
    return;

  mPlainLiterals.erase(i);
}

void
CDA_DataSource::typedLiteralDeleted(CDA_TypedLiteral* aLit)
{
  std::map<wstringpair, CDA_TypedLiteral*>::iterator i
    (mTypedLiterals.find(wstringpair(aLit->mLexicalForm, aLit->mTypeURI)));

  if (i == mTypedLiterals.end())
    return;

  mTypedLiterals.erase(i);
}

void
CDA_DataSource::uriReferenceDeleted(CDA_URIReference* aURIRef)
{
  std::map<std::wstring, CDA_URIReference*>::iterator i
    (mURIReferences.find(aURIRef->mURI));

  if (i == mURIReferences.end())
    return;

  mURIReferences.erase(i);
}

void
CDA_DataSource::assert
(
 CDA_Resource* aSubject,
 CDA_Resource* aPredicate,
 CDA_RDFNode* aObject
)
{
  if (aSubject->mDataSource != this ||
      aPredicate->mDataSource != this ||
      aObject->mDataSource != this)
    throw iface::rdf_api::RDFProcessingError();

  if (mTripleSet->assert(aSubject, aPredicate, aObject))
  {
    aSubject->addAssociation();
    aPredicate->addAssociation();
    aObject->addAssociation();
  }
}

void
CDA_DataSource::unassert
(
 CDA_Resource* aSubject,
 CDA_Resource* aPredicate,
 CDA_RDFNode* aObject
)
{
  if (mTripleSet->unassert(aSubject, aPredicate, aObject))
  {
    aSubject->releaseAssociation();
    aPredicate->releaseAssociation();
    aObject->releaseAssociation();
  }
}

void
CDA_DataSource::nodeAssociated(CDA_RDFNode* aNode)
{
  mAssociatedNodes.insert(aNode);
}

void
CDA_DataSource::nodeDissociated(CDA_RDFNode* aNode)
{
  mAssociatedNodes.erase(aNode);
}

CDA_RDFNode::CDA_RDFNode(CDA_DataSource* aDataSource)
  : mDataSource(aDataSource), mRefCount(0), mAssociationCount(0)
{
  add_ref();
}

CDA_RDFNode::~CDA_RDFNode()
{
}

void
CDA_RDFNode::add_ref()
  throw()
{
  mRefCount++;
  mDataSource->add_ref();
}

void
CDA_RDFNode::release_ref()
  throw()
{
  mRefCount--;

  iface::rdf_api::DataSource* ds = mDataSource;
  
  if (mRefCount == 0 && mAssociationCount == 0)
    delete this;

  ds->release_ref();
}

void
CDA_RDFNode::addAssociation()
  throw()
{
  mAssociationCount++;
  if (mAssociationCount == 1)
    mDataSource->nodeAssociated(this);
}

void
CDA_RDFNode::releaseAssociation()
  throw()
{
  mAssociationCount--;
  if (mAssociationCount == 0)
  {
    mDataSource->nodeDissociated(this);

    if (mRefCount == 0)
      delete this;
  }
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_RDFNode::getTriplesInto()
  throw(std::exception&)
{
  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, NULL, NULL, this);
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_RDFNode::getTriplesIntoByPredicate
(
 iface::rdf_api::Resource* aPredicate
)
  throw(std::exception&)
{
  CDA_Resource* pred = dynamic_cast<CDA_Resource*>(aPredicate);
  if (pred == NULL)
    throw iface::rdf_api::RDFProcessingError();

  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, NULL, pred, this);
}

CDA_Resource::CDA_Resource(CDA_DataSource* aDataSource)
  : CDA_RDFNode(aDataSource)
{
}

CDA_Resource::~CDA_Resource()
{
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_Resource::getTriplesOutOfByPredicate(iface::rdf_api::Resource* aPredicate)
  throw(std::exception&)
{
  CDA_Resource* pred = dynamic_cast<CDA_Resource*>(aPredicate);
  if (pred == NULL)
    throw iface::rdf_api::RDFProcessingError();

  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, this, pred, NULL);
}

already_AddRefd<iface::rdf_api::Triple>
CDA_Resource::getTripleOutOfByPredicate(iface::rdf_api::Resource* aPredicate)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     getTriplesOutOfByPredicate(aPredicate));
  RETURN_INTO_OBJREF(ti, iface::rdf_api::TripleEnumerator, ts->enumerateTriples());
  RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, ti->getNextTriple());
  if (t == NULL)
    throw iface::rdf_api::RDFProcessingError();

  t->add_ref();
  return t.getPointer();
}

bool
CDA_Resource::hasTripleOutOf
(
 iface::rdf_api::Resource* aPredicate,
 iface::rdf_api::Node* aObject
)
  throw(std::exception&)
{
  CDA_Resource* pred = dynamic_cast<CDA_Resource*>(aPredicate);
  CDA_RDFNode* obj = dynamic_cast<CDA_RDFNode*>(aObject);

  if (pred == NULL || obj == NULL)
    throw iface::rdf_api::RDFProcessingError();

  RETURN_INTO_OBJREF(fts, CDA_FilteringTripleSet,
                     new CDA_FilteringTripleSet(mDataSource->mTripleSet,
                                                this, pred, obj));
  RETURN_INTO_OBJREF(ti, iface::rdf_api::TripleEnumerator,
                     fts->enumerateTriples());
  RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, ti->getNextTriple());
  return (t != NULL);
}

void
CDA_Resource::createTripleOutOf
(
 iface::rdf_api::Resource* aPredicate,
 iface::rdf_api::Node* aObject
)
  throw(std::exception&)
{
  CDA_Resource* pred = dynamic_cast<CDA_Resource*>(aPredicate);
  CDA_RDFNode* obj = dynamic_cast<CDA_RDFNode*>(aObject);

  if (pred == NULL || obj == NULL)
    throw iface::rdf_api::RDFProcessingError();

  mDataSource->assert(this, pred, obj);
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_Resource::getTriplesOutOfByObject
(
 iface::rdf_api::Node* aObject
)
  throw(std::exception&)
{
  CDA_RDFNode* obj = dynamic_cast<CDA_RDFNode*>(aObject);
  if (obj == NULL)
    throw iface::rdf_api::RDFProcessingError();

  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, this, NULL, obj);
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_Resource::getTriplesWherePredicate()
  throw(std::exception&)
{
  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, NULL, this, NULL);
}

already_AddRefd<iface::rdf_api::TripleSet>
CDA_Resource::getTriplesWhereSubject()
  throw(std::exception&)
{
  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, this, NULL, NULL);
}

already_AddRefd<iface::rdf_api::Container>
CDA_Resource::correspondingContainer()
  throw(std::exception&)
{
  return new CDA_Container(mDataSource, this);
}

already_AddRefd<iface::rdf_api::Container>
CDA_Resource::findOrMakeContainer
(
 iface::rdf_api::Resource* aPredicate,
 iface::rdf_api::Resource* aContainerType
)
  throw(std::exception&)
{
  ObjRef<iface::rdf_api::Container> result;

  RETURN_INTO_OBJREF(type, iface::rdf_api::URIReference,
                     mDataSource->getURIReference(RDF_NS L"type"));

  // Find all triples with predicate...
  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     getTriplesOutOfByPredicate(aPredicate));
  RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                     ts->enumerateTriples());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
    if (t == NULL)
      break;

    RETURN_INTO_OBJREF(n, iface::rdf_api::Node, t->object());
    DECLARE_QUERY_INTERFACE_OBJREF(r, n, rdf_api::Resource);
    if (r == NULL)
      continue;

    if (!r->hasTripleOutOf(type, aContainerType))
      continue;

    if (result == NULL)
      result = already_AddRefd<iface::rdf_api::Container>
        (r->correspondingContainer());
    else
    {
      RETURN_INTO_OBJREF(c, iface::rdf_api::Container, r->correspondingContainer());
      result = already_AddRefd<iface::rdf_api::Container>(result->mergeWith(c));
    }
  }

  if (result != NULL)
  {
    result->add_ref();
    return result.getPointer();
  }
  
  // So no matching containter exists. Make one...
  RETURN_INTO_OBJREF(r, iface::rdf_api::BlankNode, mDataSource->createBlankNode());
  createTripleOutOf(aPredicate, r);
  r->createTripleOutOf(type, aContainerType);
  return r->correspondingContainer();
}

#define MAX_SEQUENTIAL_SEARCH 5

already_AddRefd<iface::rdf_api::Node>
CDA_NodeIteratorContainer::getNextNode()
  throw(std::exception&)
{
  uint32_t i;
  for (i = 0; i < MAX_SEQUENTIAL_SEARCH; i++)
  {
    const int sz = sizeof(RDF_NS) + 20;
    wchar_t buf[sz];
    any_swprintf(buf, sz, RDF_NS L"_%u", mNextIndex + i);
    RETURN_INTO_OBJREF(indexp, iface::rdf_api::URIReference,
                       mDataSource->getURIReference(buf));
    try
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple,
                         mResource->getTripleOutOfByPredicate(indexp));
      mNextIndex += i + 1;
      return t->object();
    }
    catch (iface::rdf_api::RDFProcessingError)
    {
    }
  }

  mNextIndex += MAX_SEQUENTIAL_SEARCH;
  // We failed to find anything close enough; this could be the end, or maybe
  // there is just a big gap. Check all triples and find the next if any...
  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     mResource->getTriplesWhereSubject());
  RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                     ts->enumerateTriples());
  ObjRef<iface::rdf_api::Node> closestNode;
  uint32_t smallestIndex = 0 /* Avoid spurious compiler warning. */;
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
    if (t == NULL)
      break;

    RETURN_INTO_OBJREF(p, iface::rdf_api::Resource, t->predicate());
    DECLARE_QUERY_INTERFACE_OBJREF(u, p, rdf_api::URIReference);
    if (u == NULL)
      continue;

    RETURN_INTO_WSTRING(uri, u->URI());
    uint32_t n;
    if (swscanf(uri.c_str(), RDF_NS L"_%u", &n) != 1)
      continue;

    if (n < mNextIndex)
      continue;
    if (closestNode == NULL || smallestIndex > n)
    {
      smallestIndex = n;
      closestNode = already_AddRefd<iface::rdf_api::Node>(t->object());
    }
  }

  if (closestNode != NULL)
  {
    closestNode->add_ref();
    mNextIndex = smallestIndex + 1;
  }
  return closestNode.getPointer();
}

already_AddRefd<iface::rdf_api::Resource>
CDA_Container::correspondingResource()
  throw(std::exception&)
{
  mCorrespondingResource->add_ref();
  return mCorrespondingResource.getPointer();
}

already_AddRefd<iface::rdf_api::Resource>
CDA_Container::containerType()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(type, iface::rdf_api::URIReference,
                     mDataSource->getURIReference(RDF_NS L"type"));
  try
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple,
                       mCorrespondingResource->getTripleOutOfByPredicate(type));
    RETURN_INTO_OBJREF(n, iface::rdf_api::Node, t->object());
    DECLARE_QUERY_INTERFACE_OBJREF(r, n, rdf_api::Resource);
    r->add_ref();
    return r.getPointer();
  }
  catch (iface::rdf_api::RDFProcessingError)
  {
    return NULL;
  }
}

void
CDA_Container::containerType(iface::rdf_api::Resource* aType)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(type, iface::rdf_api::URIReference,
                     mDataSource->getURIReference(RDF_NS L"type"));
  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     mCorrespondingResource->getTriplesOutOfByPredicate(type));
  RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                     ts->enumerateTriples());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
    if (t == NULL)
      break;

    t->unassert();
  }

  mCorrespondingResource->createTripleOutOf(type, aType);
}

already_AddRefd<iface::rdf_api::NodeIterator>
CDA_Container::iterateChildren()
  throw(std::exception&)
{
  return new CDA_NodeIteratorContainer(mDataSource, mCorrespondingResource);
}

void
CDA_Container::appendChild(iface::rdf_api::Node* aChild)
  throw(std::exception&)
{
  for (uint32_t i = 1;; i++)
  {
    const int sz = sizeof(RDF_NS) + 20;
    wchar_t buf[sz];
    any_swprintf(buf, sz, RDF_NS L"_%u", i);
    RETURN_INTO_OBJREF(indexp, iface::rdf_api::URIReference,
                       mDataSource->getURIReference(buf));
    try
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple,
                         mCorrespondingResource->getTripleOutOfByPredicate(indexp));
    }
    catch (iface::rdf_api::RDFProcessingError)
    {
      mCorrespondingResource->createTripleOutOf(indexp, aChild);
      return;
    }

  }
}

void
CDA_Container::removeChild(iface::rdf_api::Node* aChild, bool aDoRenumbering)
  throw(std::exception&)
{
  bool anotherRunNeeded;

  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     mCorrespondingResource->getTriplesWhereSubject());

  do
  {
    RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                       ts->enumerateTriples());

    anotherRunNeeded = false;

    while (true)
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
      if (t == NULL)
        break;

      RETURN_INTO_OBJREF(o, iface::rdf_api::Node, t->object());
      if (CDA_objcmp(o, aChild))
        continue;

      RETURN_INTO_OBJREF(p, iface::rdf_api::Resource, t->predicate());
      DECLARE_QUERY_INTERFACE_OBJREF(u, p, rdf_api::URIReference);
      if (u == NULL)
        continue;

      RETURN_INTO_WSTRING(uri, u->URI());
      uint32_t n;
      if (swscanf(uri.c_str(), RDF_NS L"_%u", &n) != 1)
        continue;

      t->unassert();

      // If we get here, we have removed an entry at n.
      anotherRunNeeded = true;
      if (!aDoRenumbering)
        break;

      te = already_AddRefd<iface::rdf_api::TripleEnumerator>
        (ts->enumerateTriples());
      std::list<ObjRef<iface::rdf_api::Triple> > toremove;
      std::list<std::pair<ObjRef<iface::rdf_api::URIReference>, ObjRef<iface::rdf_api::Node> > > toadd;
      while (true)
      {
        RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
        if (t == NULL)
          break;
        RETURN_INTO_OBJREF(p, iface::rdf_api::Resource, t->predicate());
        DECLARE_QUERY_INTERFACE_OBJREF(u, p, rdf_api::URIReference);
        if (u == NULL)
          continue;

        RETURN_INTO_WSTRING(uri, u->URI());
        uint32_t n2;
        if (swscanf(uri.c_str(), RDF_NS L"_%u", &n2) != 1)
          continue;
        if (n2 <= n)
          continue;
        const int sz = sizeof(RDF_NS) + 20;
        wchar_t buf[sz];
        any_swprintf(buf, sz, RDF_NS L"_%u", n2 - 1);
        RETURN_INTO_OBJREF(indexp, iface::rdf_api::URIReference,
                           mDataSource->getURIReference(buf));

        RETURN_INTO_OBJREF(o, iface::rdf_api::Node, t->object());
        toremove.push_back(t);
        toadd.push_back(std::pair<ObjRef<iface::rdf_api::URIReference>, ObjRef<iface::rdf_api::Node> >
                        (indexp, o));
      }
      for (std::list<ObjRef<iface::rdf_api::Triple> >::iterator i =
             toremove.begin();
           i != toremove.end();
           i++)
        (*i)->unassert();
      for (std::list<std::pair<ObjRef<iface::rdf_api::URIReference>, ObjRef<iface::rdf_api::Node> > >::iterator i =
             toadd.begin();
           i != toadd.end();
           i++)
        mCorrespondingResource->createTripleOutOf((*i).first, (*i).second);

      break;
    }
  }
  while (anotherRunNeeded);
}

void
CDA_Container::renumberContainer()
  throw(std::exception&)
{
  std::map<uint32_t, ObjRef<iface::rdf_api::Node> > contents;
  RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                     mCorrespondingResource->getTriplesWhereSubject());
  RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                     ts->enumerateTriples());

  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, te->getNextTriple());
    if (t == NULL)
      break;

    RETURN_INTO_OBJREF(p, iface::rdf_api::Resource, t->predicate());
    DECLARE_QUERY_INTERFACE_OBJREF(u, p, rdf_api::URIReference);
    if (u == NULL)
      continue;
    
    RETURN_INTO_WSTRING(uri, u->URI());
    uint32_t n;
    if (swscanf(uri.c_str(), RDF_NS L"_%u", &n) != 1)
      continue;

    RETURN_INTO_OBJREF(o, iface::rdf_api::Node, t->object());
    t->unassert();
    contents.insert(std::pair<uint32_t, ObjRef<iface::rdf_api::Node> >(n, o));
  }
  
  uint32_t n = 1;
  for (std::map<uint32_t, ObjRef<iface::rdf_api::Node> >::iterator i = contents.begin();
       i != contents.end(); i++, n++)
  {
    const int sz = sizeof(RDF_NS) + 20;
    wchar_t buf[sz];
    any_swprintf(buf, sz, RDF_NS L"_%u", n);
    RETURN_INTO_OBJREF(indexp, iface::rdf_api::URIReference,
                       mDataSource->getURIReference(buf));
    mCorrespondingResource->createTripleOutOf(indexp, (*i).second);
  }
}

already_AddRefd<iface::rdf_api::Container>
CDA_Container::mergeWith(iface::rdf_api::Container* aContainer)
  throw(std::exception&)
{
  return new CDA_MergedContainer(this, aContainer);
}


already_AddRefd<iface::rdf_api::Node>
CDA_NodeIteratorMergedContainer::getNextNode()
  throw(std::exception&)
{
  if (mIterator1Done)
    return mIterator2->getNextNode();
  iface::rdf_api::Node* n = mIterator1->getNextNode();
  if (n == NULL)
  {
    mIterator1Done = true;
    n = mIterator2->getNextNode();
  }

  return n;
}

already_AddRefd<iface::rdf_api::Resource>
CDA_MergedContainer::correspondingResource()
  throw(std::exception&)
{
  return mContainer1->correspondingResource();
}

already_AddRefd<iface::rdf_api::Resource>
CDA_MergedContainer::containerType()
  throw(std::exception&)
{
  return mContainer1->containerType();
}

void
CDA_MergedContainer::containerType(iface::rdf_api::Resource* aType)
  throw(std::exception&)
{
  mContainer1->containerType(aType);
}

already_AddRefd<iface::rdf_api::NodeIterator>
CDA_MergedContainer::iterateChildren()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(it1, iface::rdf_api::NodeIterator, mContainer1->iterateChildren());
  RETURN_INTO_OBJREF(it2, iface::rdf_api::NodeIterator,
                     mContainer2->iterateChildren());
  return new CDA_NodeIteratorMergedContainer(it1, it2);
}

void
CDA_MergedContainer::appendChild(iface::rdf_api::Node* aChild)
  throw(std::exception&)
{
  mContainer1->appendChild(aChild);
}

void
CDA_MergedContainer::removeChild(iface::rdf_api::Node* aChild, bool aDoRenumbering)
  throw(std::exception&)
{
  mContainer1->removeChild(aChild, aDoRenumbering);
  mContainer2->removeChild(aChild, aDoRenumbering);
}

void
CDA_MergedContainer::renumberContainer()
  throw(std::exception&)
{
  mContainer1->renumberContainer();
  mContainer2->renumberContainer();
}

already_AddRefd<iface::rdf_api::Container>
CDA_MergedContainer::mergeWith(iface::rdf_api::Container* aContainer)
  throw(std::exception&)
{
  return new CDA_MergedContainer(this, aContainer);
}

CDA_BlankNode::CDA_BlankNode(CDA_DataSource* aDataSource)
  : CDA_Resource(aDataSource)
{
}

CDA_BlankNode::~CDA_BlankNode()
{
}

CDA_URIReference::CDA_URIReference
(
 const std::wstring& aURI,
 CDA_DataSource* aDataSource
)
  : CDA_Resource(aDataSource), mURI(aURI)
{
}

CDA_URIReference::~CDA_URIReference()
{
  mDataSource->uriReferenceDeleted(this);
}

std::wstring
CDA_URIReference::URI()
  throw()
{
  return mURI;
}

CDA_Literal::CDA_Literal(const std::wstring& aLexicalForm, CDA_DataSource* aDataSource)
  : CDA_RDFNode(aDataSource), mLexicalForm(aLexicalForm)
{
}

CDA_Literal::~CDA_Literal()
{
}

std::wstring
CDA_Literal::lexicalForm()
  throw()
{
  return mLexicalForm;
}

CDA_PlainLiteral::CDA_PlainLiteral(const std::wstring& aLexicalForm,
                                   const std::wstring& aLanguage,
                                   CDA_DataSource* aDataSource)
  : CDA_Literal(aLexicalForm, aDataSource), mLanguage(aLanguage)
{
}

CDA_PlainLiteral::~CDA_PlainLiteral()
{
  mDataSource->plainLiteralDeleted(this);
}

std::wstring
CDA_PlainLiteral::language()
  throw()
{
  return mLanguage;
}

CDA_TypedLiteral::CDA_TypedLiteral
(
 const std::wstring& aLexicalForm, const std::wstring& aTypeURI,
 CDA_DataSource* aDataSource
)
  : CDA_Literal(aLexicalForm, aDataSource), mTypeURI(aTypeURI)
{
}

CDA_TypedLiteral::~CDA_TypedLiteral()
{
  mDataSource->typedLiteralDeleted(this);
}

std::wstring
CDA_TypedLiteral::datatypeURI()
  throw()
{
  return mTypeURI;
}

CDA_Triple::CDA_Triple(CDA_DataSource* aDataSource, CDA_Resource* aSubject,
                       CDA_Resource* aPredicate, CDA_RDFNode* aObject)
  : mDataSource(aDataSource), mSubject(aSubject),
    mPredicate(aPredicate), mObject(aObject)
{
}

CDA_Triple::~CDA_Triple()
{
}

already_AddRefd<iface::rdf_api::Resource>
CDA_Triple::subject()
  throw(std::exception&)
{
  mSubject->add_ref();
  return mSubject.getPointer();
}

already_AddRefd<iface::rdf_api::Resource>
CDA_Triple::predicate()
  throw(std::exception&)
{
  mPredicate->add_ref();
  return mPredicate.getPointer();
}

already_AddRefd<iface::rdf_api::Node>
CDA_Triple::object()
  throw(std::exception&)
{
  mObject->add_ref();
  return mObject.getPointer();
}

void
CDA_Triple::unassert()
  throw()
{
  mDataSource->unassert(mSubject, mPredicate, mObject);
}

CDA_AllTriplesEnumerator::CDA_AllTriplesEnumerator
(
 CDA_DataSource* aDataSource,
 CDA_AllTriplesSet* aSet,
 const std::set<CDA_AllTriplesSet::RealTriple>::iterator& aPosition,
 const std::set<CDA_AllTriplesSet::RealTriple>::iterator& aEnd
)
  : mSet(aSet), mDataSource(aDataSource),
    mPosition(aPosition), mEnd(aEnd)
{
}

CDA_AllTriplesEnumerator::~CDA_AllTriplesEnumerator()
{
  mSet->enumeratorDeleted(this);
}

already_AddRefd<iface::rdf_api::Triple>
CDA_AllTriplesEnumerator::getNextTriple()
  throw(std::exception&)
{
  if (mPosition == mEnd)
  {
    return NULL;
  }

  CDA_Triple* t = new CDA_Triple(mDataSource, (*mPosition).subj,
                                 (*mPosition).pred, (*mPosition).obj);
  mPosition++;

  return t;
}

void
CDA_AllTriplesEnumerator::aboutToDelete(const std::set<CDA_AllTriplesSet::
                                        RealTriple>::iterator& aWhere)
{
  if (aWhere == mPosition)
    mPosition++;
}

CDA_AllTriplesSet::CDA_AllTriplesSet(CDA_DataSource* aDataSource)
  : mDataSource(aDataSource)
{
}

CDA_AllTriplesSet::~CDA_AllTriplesSet()
{
}

already_AddRefd<iface::rdf_api::TripleEnumerator>
CDA_AllTriplesSet::enumerateTriples()
  throw(std::exception&)
{
  CDA_AllTriplesEnumerator* te =
    new CDA_AllTriplesEnumerator(mDataSource, this, mRealTriples.begin(),
                                 mRealTriples.end());
  mLiveEnumerators.insert(te);

  return te;
}

bool
CDA_AllTriplesSet::assert
(
 CDA_Resource* aSubject,
 CDA_Resource* aPredicate,
 CDA_RDFNode* aObject
)
{
  RealTriple rt(aSubject, aPredicate, aObject);
  if (mRealTriples.count(rt))
    return false;

  mRealTriples.insert(rt);
  return true;
}

bool
CDA_AllTriplesSet::unassert
(
 CDA_Resource* aSubject,
 CDA_Resource* aPredicate,
 CDA_RDFNode* aObject
)
{
  RealTriple rt(aSubject, aPredicate, aObject);

  std::set<RealTriple>::iterator i(mRealTriples.find(rt));

  if (i == mRealTriples.end())
    return false;

  for (std::set<CDA_AllTriplesEnumerator*>::iterator j = mLiveEnumerators.begin();
       j != mLiveEnumerators.end();
       j++)
    (*j)->aboutToDelete(i);

  mRealTriples.erase(i);

  return true;
}

void
CDA_AllTriplesSet::enumeratorDeleted(CDA_AllTriplesEnumerator* aEnum)
{
  mLiveEnumerators.erase(aEnum);
}

CDA_FilteringTripleSet::CDA_FilteringTripleSet
(
 CDA_AllTriplesSet* aMasterSource,
 CDA_Resource* aSubjectFilter,
 CDA_Resource* aPredicateFilter,
 CDA_RDFNode* aObjectFilter
)
  : mMasterSource(aMasterSource),
    mSubjectFilter(aSubjectFilter), mPredicateFilter(aPredicateFilter),
    mObjectFilter(aObjectFilter)
{
}

CDA_FilteringTripleSet::~CDA_FilteringTripleSet()
{
}

already_AddRefd<iface::rdf_api::TripleEnumerator>
CDA_FilteringTripleSet::enumerateTriples()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(ti, iface::rdf_api::TripleEnumerator,
                     mMasterSource->enumerateTriples());
  return new CDA_FilteringTripleEnumerator(ti, mSubjectFilter, mPredicateFilter,
                                           mObjectFilter);
}

CDA_FilteringTripleEnumerator::CDA_FilteringTripleEnumerator
(
 iface::rdf_api::TripleEnumerator* aMasterEnum, CDA_Resource* aSubjectFilter,
 CDA_Resource* aPredicateFilter, CDA_RDFNode* aObjectFilter
)
  : mMasterEnum(aMasterEnum), mSubjectFilter(aSubjectFilter),
    mPredicateFilter(aPredicateFilter), mObjectFilter(aObjectFilter)
{
}

CDA_FilteringTripleEnumerator::~CDA_FilteringTripleEnumerator()
{
}

already_AddRefd<iface::rdf_api::Triple>
CDA_FilteringTripleEnumerator::getNextTriple()
  throw(std::exception&)
{
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::rdf_api::Triple, mMasterEnum->getNextTriple());
    if (t == NULL)
      return NULL;

    if (mSubjectFilter)
    {
      RETURN_INTO_OBJREF(s, iface::rdf_api::Resource, t->subject());
      if (CDA_objcmp(s, mSubjectFilter))
        continue;
    }
    if (mPredicateFilter)
    {
      RETURN_INTO_OBJREF(p, iface::rdf_api::Resource, t->predicate());
      if (CDA_objcmp(p, mPredicateFilter))
        continue;
    }
    if (mObjectFilter)
    {
      RETURN_INTO_OBJREF(o, iface::rdf_api::Node, t->object());
      if (CDA_objcmp(o, mObjectFilter))
        continue;
    }

    t->add_ref();
    return t.getPointer();
  }
}

CDA_RDFBootstrap::CDA_RDFBootstrap()
{
}

CDA_RDFBootstrap::~CDA_RDFBootstrap()
{
}

already_AddRefd<iface::rdf_api::DataSource>
CDA_RDFBootstrap::createDataSource()
  throw(std::exception&)
{
  return new CDA_DataSource();
}

/* We should really support xml:lang properly. But for now it is hardcoded. */
#define LANGUAGE L"en"

static std::wstring
getLanguage(iface::dom::Element* aElement)
{
  for (ObjRef<iface::dom::Node> n = aElement;
       n != NULL;
       n = already_AddRefd<iface::dom::Node>(n->parentNode()))
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      break;

    if (el->hasAttribute(L"xml:lang"))
      return el->getAttribute(L"xml:lang");
  }

  return LANGUAGE;
}

static URI
getBaseURI(const URI& aDefault, iface::dom::Element* aElement)
{
  for (ObjRef<iface::dom::Node> n = aElement;
       n != NULL;
       n = already_AddRefd<iface::dom::Node>(n->parentNode()))
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
    if (el == NULL)
      break;

    if (el->hasAttribute(L"xml:base"))
    {
      RETURN_INTO_WSTRING(xb, el->getAttribute(L"xml:base"));
      return URI(xb);
    }
  }

  return aDefault;
}

class RDFParseContext
{
public:
  RDFParseContext(iface::rdf_api::DataSource* aDataSource, const std::wstring& aDocURI)
    : mDataSource(aDataSource), mBase(aDocURI)
  {
#define RESOURCE(n, t) m##n##Resource = already_AddRefd<iface::rdf_api::Resource> \
      (aDataSource->getURIReference(RDF_NS L## #t))
    RESOURCE(Type, type);
    RESOURCE(Subj, subject);
    RESOURCE(Pred, predicate);
    RESOURCE(Obj, object);
    RESOURCE(First, first);
    RESOURCE(Rest, rest);
    RESOURCE(Nil, nil);
    RESOURCE(Statement, Statement);
#undef RESOURCE
  }

  ~RDFParseContext()
  {
    for (std::map<std::wstring, iface::rdf_api::BlankNode*>::iterator i =
           mBlankNodes.begin();
         i != mBlankNodes.end(); i++)
      (*i).second->release_ref();
  }

  void
  parseNodeElementList(iface::dom::Element* aElement)
  {
    RETURN_INTO_OBJREF(nl, iface::dom::NodeList, aElement->childNodes());
    for (uint32_t i = 0; i < nl->length(); i++)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
      // If we were validating, we would check for non-whitespace text nodes,
      // and give an error. But since we aren't, ignore anything that isn't an
      // element.
      DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
      if (el == NULL)
        continue;
      
      parseNodeElement(el);
    }
  }

  void
  parseNodeElement
  (
   iface::dom::Element* aElement,
   iface::rdf_api::Resource** aSubject = NULL
  )
  {
    std::wstring eURI(getNodeURI(aElement));
    if (!isNodeElementURI(eURI))
    {
      if (aSubject)
        *aSubject = NULL;
      return;
    }

    ObjRef<iface::rdf_api::Resource> subject;

    // Look for rdf:ID...
    {
      RETURN_INTO_WSTRING(id, aElement->getAttributeNS(RDF_NS, L"ID"));
      if (id != L"")
      {
        URI abs(getBaseURI(mBase, aElement), std::wstring(L"#") + id);
        subject = already_AddRefd<iface::rdf_api::Resource>
          (mDataSource->getURIReference(abs.absoluteURI().c_str()));
      }
    }

    if (subject == NULL)
    {
      RETURN_INTO_WSTRING(nodeID, aElement->getAttributeNS(RDF_NS, L"nodeID"));
      if (nodeID != L"")
        subject = already_AddRefd<iface::rdf_api::Resource>
          (findOrCreateBlankNodeById(nodeID));
    }

    if (subject == NULL)
    {
      if (aElement->hasAttributeNS(RDF_NS, L"about"))
      {
        RETURN_INTO_WSTRING(about, aElement->getAttributeNS(RDF_NS, L"about"));
        URI abs(getBaseURI(mBase, aElement), about);
        subject = already_AddRefd<iface::rdf_api::Resource>
          (mDataSource->getURIReference(abs.absoluteURI().c_str()));
      }
    }

    if (subject == NULL)
      subject = already_AddRefd<iface::rdf_api::Resource>
        (mDataSource->createBlankNode());

    // We are now guaranteed to have a subject...

    if (eURI != RDF_NS L"Description")
    {
      RETURN_INTO_OBJREF(obj, iface::rdf_api::Resource,
                         mDataSource->getURIReference(eURI.c_str()));
      subject->createTripleOutOf(mTypeResource, obj);
    }

    RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, aElement->attributes());
    for (uint32_t i = 0, l = nnm->length(); i < l; i++)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));
      std::wstring atURI(getNodeURI(n));
      if (atURI == RDF_NS L"type")
      {
        RETURN_INTO_WSTRING(v, n->nodeValue());
        URI abs(getBaseURI(mBase, aElement), v);
        RETURN_INTO_OBJREF(obj, iface::rdf_api::Resource,
                           mDataSource->getURIReference(abs.absoluteURI().c_str()));
        subject->createTripleOutOf(mTypeResource, obj);
      }
      else if (isPropertyAttributeURI(atURI))
      {
        RETURN_INTO_WSTRING(v, n->nodeValue());
        RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                           mDataSource->getURIReference(atURI.c_str()));
        RETURN_INTO_WSTRING(lang, getLanguage(aElement));
        RETURN_INTO_OBJREF(obj, iface::rdf_api::Node,
                           mDataSource->getPlainLiteral(v.c_str(),
                                                        lang.c_str()));
        subject->createTripleOutOf(pred, obj);
      }
    }

    parsePropertyElementList(aElement, subject);

    if (aSubject != NULL)
    {
      subject->add_ref();
      *aSubject = subject;
    }
  }

private:
  iface::rdf_api::DataSource* mDataSource;
  ObjRef<iface::rdf_api::Resource> mTypeResource, mSubjResource, mPredResource,
    mObjResource, mFirstResource, mRestResource, mNilResource,
    mStatementResource;
  URI mBase;

  std::map<std::wstring, iface::rdf_api::BlankNode*> mBlankNodes;

  std::wstring getNodeURI(iface::dom::Node* aNode)
  {
    RETURN_INTO_WSTRING(ns, aNode->namespaceURI());
    RETURN_INTO_WSTRING(ln, aNode->localName());

    URI u(ns+ln);
    return u.absoluteURI();
  }

  bool isXMLAttributeURI(const std::wstring& aURI)
  {
    return (aURI.substr(0, 36) == L"http://www.w3.org/XML/1998/namespace") ||
      (aURI.substr(0, 29) == L"http://www.w3.org/2000/xmlns/") || aURI == L"xmlns";
  }

  bool isRDFTerm(const std::wstring& aURI)
  {
    return (aURI.substr(0, sizeof(RDF_NS)/sizeof(RDF_NS[0]) - 1) == RDF_NS);
  }

  bool isCoreSyntaxTerm(const std::wstring& aURI)
  {
#define T(x) (aURI == RDF_NS L## #x)
    if (T(RDF) || T(ID) || T(about) || T(parseType) || T(resource) || T(nodeID)
        || T(datatype))
      return true;
    return false;
  }

  bool isSyntaxTerm(const std::wstring& aURI)
  {
    if (isCoreSyntaxTerm(aURI) || T(Description) || T(li))
      return true;

    return false;
  }

  bool isOldTerm(const std::wstring& aURI)
  {
    if (T(aboutEach) || T(aboutEachPrefix) || T(bagID))
      return true;

    return false;
  }

  bool isNodeElementURI(const std::wstring& aURI)
  {
    if (!isRDFTerm(aURI))
      return true;
    if (isCoreSyntaxTerm(aURI) || isOldTerm(aURI) || T(li))
      return false;

    return true;
  }

  bool isPropertyElementURI(const std::wstring& aURI)
  {
    if (!isRDFTerm(aURI))
      return true;
    if (isCoreSyntaxTerm(aURI) || isOldTerm(aURI) || T(Description))
      return false;

    return true;
  }

  bool isPropertyAttributeURI(const std::wstring& aURI)
  {
    if (!isRDFTerm(aURI))
      return !isXMLAttributeURI(aURI);
    if (isSyntaxTerm(aURI) || isOldTerm(aURI))
      return false;

    return true;
  }
#undef T

  already_AddRefd<iface::rdf_api::BlankNode>
  findOrCreateBlankNodeById(const std::wstring& aNodeID)
  {
    std::map<std::wstring, iface::rdf_api::BlankNode*>::iterator i
      (mBlankNodes.find(aNodeID));
    if (i != mBlankNodes.end())
    {
      (*i).second->add_ref();
      return (*i).second;
    }

    iface::rdf_api::BlankNode* bn = mDataSource->createBlankNode();
    mBlankNodes.insert(std::pair<std::wstring, iface::rdf_api::BlankNode*>(aNodeID, bn));
    bn->add_ref();
    return bn;
  }

  void
  parsePropertyElementList
  (
   iface::dom::Element* aElement,
   iface::rdf_api::Resource* aSubject
  )
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, aElement->firstChild());
    uint32_t liCounter = 1;
    for (; n; n = already_AddRefd<iface::dom::Node>(n->nextSibling()))
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
      if (el != NULL)
      {
        std::wstring uri(getNodeURI(el));
        if (uri == RDF_NS L"li")
        {
          uri = RDF_NS L"_";
          wchar_t buf[20];
          any_swprintf(buf, 20, L"%u", liCounter++);
          uri += buf;
        }

        parsePropertyElement(el, uri, aSubject);
      }
    }
  }

  void
  parsePropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject
  )
  {
    // First up, look at the parseType...
    RETURN_INTO_WSTRING(parseType, aElement->getAttributeNS(RDF_NS, L"parseType"));
    if (parseType != L"")
    {
      if (parseType == L"Resource")
        parseTypeResourcePropertyElement(aElement, aURI, aSubject);
      else if (parseType == L"Collection")
        parseTypeCollectionPropertyElement(aElement, aURI, aSubject);
      else
        parseTypeLiteralPropertyElement(aElement, aURI, aSubject);

      return;
    }

    // See what type of children are present to decide between resource,
    // literal and empty.
    std::wstring text;

    for (
         ObjRef<iface::dom::Node> n = already_AddRefd<iface::dom::Node>
           (aElement->firstChild());
         n != NULL;
         n = already_AddRefd<iface::dom::Node>(n->nextSibling())
        )
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
      if (el != NULL)
      {
        parseResourcePropertyElement(aElement, aURI, aSubject, el);
        return;
      }
      else
      {
        DECLARE_QUERY_INTERFACE_OBJREF(t, n, dom::Text);
        if (t != NULL)
        {
          RETURN_INTO_WSTRING(d, t->data());
          text += d;
        }
      }
    }

    for (std::wstring::iterator i = text.begin(); i != text.end(); i++)
    {
      if ((*i) == L' ' || (*i) == L'\t' || (*i) == L'\r' || (*i) == L'\n')
        continue;

      parseLiteralPropertyElement(aElement, aURI, aSubject, text);
      return;
    }

    parseEmptyPropertyElement(aElement, aURI, aSubject);
  }

  void
  createPossiblyReifiedTriple
  (
   iface::dom::Element* aElement,
   iface::rdf_api::Resource* aSubject,
   iface::rdf_api::Resource* aPredicate,
   iface::rdf_api::Node* aObject
  )
  {
    // Firstly, create the unreified triple...
    aSubject->createTripleOutOf(aPredicate, aObject);

    // See if there is an rdf:ID attribute...
    RETURN_INTO_WSTRING(id, aElement->getAttributeNS(RDF_NS, L"ID"));
    if (id == L"")
      return;

    URI abs(getBaseURI(mBase, aElement), std::wstring(L"#") + id);
    RETURN_INTO_OBJREF(subj, iface::rdf_api::Resource,
                       mDataSource->getURIReference(abs.absoluteURI().c_str()));

    subj->createTripleOutOf(mSubjResource, aSubject);
    subj->createTripleOutOf(mPredResource, aPredicate);
    subj->createTripleOutOf(mObjResource, aObject);
    subj->createTripleOutOf(mTypeResource, mStatementResource);
  }

  void
  parseResourcePropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject,
   iface::dom::Element* aChildEl
  )
  {
    if (!isPropertyElementURI(aURI))
      return;

    RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                       mDataSource->getURIReference(aURI.c_str()));

    iface::rdf_api::Resource* tobj;
    parseNodeElement(aChildEl, &tobj);
    if (tobj == NULL)
      return;
    RETURN_INTO_OBJREF(obj, iface::rdf_api::Resource, tobj);

    createPossiblyReifiedTriple(aElement, aSubject, pred, obj);
  }

  void
  parseLiteralPropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject,
   std::wstring& aText
  )
  {
    if (!isPropertyElementURI(aURI))
      return;

    RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                       mDataSource->getURIReference(aURI.c_str()));

    ObjRef<iface::rdf_api::Node> obj;
    RETURN_INTO_WSTRING(dt, aElement->getAttributeNS(RDF_NS, L"datatype"));
    
    if (dt == L"")
    {
      RETURN_INTO_WSTRING(lang, getLanguage(aElement));
      obj = already_AddRefd<iface::rdf_api::Node>
        (mDataSource->getPlainLiteral(aText.c_str(), lang.c_str()));
    }
    else
      obj = already_AddRefd<iface::rdf_api::Node>
        (mDataSource->getTypedLiteral(aText.c_str(), dt.c_str()));

    createPossiblyReifiedTriple(aElement, aSubject, pred, obj);
  }

  void
  parseEmptyPropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject
  )
  {
    RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                       mDataSource->getURIReference(aURI.c_str()));

    bool foundNonIDAttribute = false;
    // See if there are any attribute children other than ID...
    RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, aElement->attributes());
    for (uint32_t i = 0, l = nnm->length(); i < l; i++)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));
      std::wstring atURI(getNodeURI(n));
      if (atURI != RDF_NS L"ID" &&
          atURI.substr(0, 36) != L"http://www.w3.org/XML/1998/namespace" &&
          atURI.substr(0, 29) != L"http://www.w3.org/2000/xmlns/"
          )
      {
        foundNonIDAttribute = true;
        break;
      }
    }

    if (!foundNonIDAttribute)
    {
      RETURN_INTO_WSTRING(lang, getLanguage(aElement));
      RETURN_INTO_OBJREF(obj, iface::rdf_api::PlainLiteral,
                         mDataSource->getPlainLiteral(L"", lang.c_str()));
      createPossiblyReifiedTriple(aElement, aSubject, pred, obj);
      return;
    }

    ObjRef<iface::rdf_api::Resource> r;
    RETURN_INTO_WSTRING(res, aElement->getAttributeNS(RDF_NS, L"resource"));
    if (res != L"")
    {
      URI abs(getBaseURI(mBase, aElement), res);
      r = already_AddRefd<iface::rdf_api::Resource>
        (mDataSource->getURIReference(abs.absoluteURI().c_str()));
    }
    else
    {
      RETURN_INTO_WSTRING(nodeID, aElement->getAttributeNS(RDF_NS, L"nodeID"));
      if (nodeID != L"")
        r = already_AddRefd<iface::rdf_api::Resource>
          (findOrCreateBlankNodeById(nodeID));
      else
        r = already_AddRefd<iface::rdf_api::Resource>
          (mDataSource->createBlankNode());
    }

    for (uint32_t i = 0, l = nnm->length(); i < l; i++)
    {
      RETURN_INTO_OBJREF(n, iface::dom::Node, nnm->item(i));
      std::wstring atURI(getNodeURI(n));

      if (!isPropertyAttributeURI(atURI))
        continue;

      RETURN_INTO_WSTRING(val, n->nodeValue());

      if (atURI == RDF_NS L"type")
      {
        URI abs(getBaseURI(mBase, aElement), val);
        RETURN_INTO_OBJREF(obj, iface::rdf_api::Resource,
                           mDataSource->getURIReference(abs.absoluteURI().
                                                        c_str()));
        r->createTripleOutOf(mTypeResource, obj);
      }
      else
      {
        RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                           mDataSource->getURIReference(atURI.c_str()));
        RETURN_INTO_WSTRING(lang, getLanguage(aElement));
        RETURN_INTO_OBJREF(obj, iface::rdf_api::Literal,
                           mDataSource->getPlainLiteral(val.c_str(),
                                                        lang.c_str()));
        r->createTripleOutOf(pred, obj);
      }
    }

    createPossiblyReifiedTriple(aElement, aSubject, pred, r);
  }

  void
  parseTypeResourcePropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject
  )
  {
    RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                       mDataSource->getURIReference(aURI.c_str()));
    RETURN_INTO_OBJREF(node, iface::rdf_api::Resource,
                       mDataSource->createBlankNode());
    createPossiblyReifiedTriple(aElement, aSubject, pred, node);
    parsePropertyElementList(aElement, node);
  }

  void
  parseTypeCollectionPropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject
  )
  {
    bool considerReify = true;
    ObjRef<iface::rdf_api::Resource> subjIn(aSubject);
    RETURN_INTO_OBJREF(predIn, iface::rdf_api::Resource,
                       mDataSource->getURIReference(aURI.c_str()));

    for (
         ObjRef<iface::dom::Node> n
           (already_AddRefd<iface::dom::Node>(aElement->firstChild()));
         n != NULL;
         n = already_AddRefd<iface::dom::Node>(n->nextSibling())
        )
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
      if (el == NULL)
        continue;

      iface::rdf_api::Resource* tmp;
      parseNodeElement(el, &tmp);
      RETURN_INTO_OBJREF(member, iface::rdf_api::Node, tmp);

      RETURN_INTO_OBJREF(link, iface::rdf_api::Resource,
                         mDataSource->createBlankNode());

      if (considerReify)
        createPossiblyReifiedTriple(aElement, subjIn, predIn, link);
      else
        subjIn->createTripleOutOf(predIn, link);

      link->createTripleOutOf(mFirstResource, member);

      predIn = mRestResource;
      subjIn = link;
      considerReify = false;
    }

    if (considerReify)
      createPossiblyReifiedTriple(aElement, subjIn, predIn, mNilResource);
    else
      subjIn->createTripleOutOf(predIn, mNilResource);
  }

  void
  parseTypeLiteralPropertyElement
  (
   iface::dom::Element* aElement,
   const std::wstring& aURI,
   iface::rdf_api::Resource* aSubject
  )
  {
    // Re-serialise aElement's children...
    // XXX we need to make DOMWriter comply with XC14N for this to be
    // strictly correct.
    DOMWriter dw;
    std::wstring text;
    for (ObjRef<iface::dom::Node> n
           (already_AddRefd<iface::dom::Node>(aElement->firstChild()));
         n != NULL;
         n = already_AddRefd<iface::dom::Node>(n->nextSibling())
        )
      dw.writeNode(NULL, n, text);


    RETURN_INTO_OBJREF(pred, iface::rdf_api::Resource,
                       mDataSource->getURIReference(aURI.c_str()));
    RETURN_INTO_OBJREF(obj, iface::rdf_api::Literal,
                       mDataSource->getTypedLiteral(text.c_str(),
                                                    RDF_NS L"XMLLiteral"));
    
    createPossiblyReifiedTriple(aElement, aSubject, pred, obj);
  }
};

void
CDA_RDFBootstrap::parseIntoDataSource
(
 iface::rdf_api::DataSource* aDataSource,
 iface::dom::Element* aElement,
 const std::wstring& aBaseURI
)
  throw(std::exception&)
{
  RDFParseContext parseContext(aDataSource, aBaseURI);

  RETURN_INTO_WSTRING(ns, aElement->namespaceURI());
  if (ns != RDF_NS)
  {
    parseContext.parseNodeElement(aElement);
    return;
  }

  RETURN_INTO_WSTRING(ln, aElement->localName());
  if (ln != L"RDF")
  {
    parseContext.parseNodeElement(aElement);
    return;
  }

  parseContext.parseNodeElementList(aElement);
}

class RDFDOMBuilder
{
public:
  RDFDOMBuilder(
                iface::dom::Document* aRDFDoc,
                iface::dom::Element* aRDFEl,
                iface::rdf_api::DataSource* aDataSource,
                const std::wstring& aBaseURI
               )
    : mBase(aBaseURI), mDoc(aRDFDoc),
      mRDFEl(aRDFEl), mDataSource(aDataSource), mLastId(0)
  {
    RETURN_INTO_OBJREF(ts, iface::rdf_api::TripleSet,
                       mDataSource->getAllTriples());
    RETURN_INTO_OBJREF(te, iface::rdf_api::TripleEnumerator,
                       ts->enumerateTriples());

    std::set<CDA_Resource*, ptr_to_less<CDA_Resource> > subjects;
    while (true)
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple,
                         te->getNextTriple());
      if (t == NULL)
        break;

      RETURN_INTO_OBJREF(subj, iface::rdf_api::Resource, t->subject());
      subjects.insert(unsafe_dynamic_cast<CDA_Resource*>(subj.getPointer()));
    }

    for (std::set<CDA_Resource*, ptr_to_less<CDA_Resource> >::iterator i
           (subjects.begin());
         i != subjects.end();
         i++)
    {
      RETURN_INTO_OBJREF(descr, iface::dom::Element,
                         createNodeElementForResource(*i));
      mRDFEl->appendChild(descr)->release_ref();
      RETURN_INTO_OBJREF(sts, iface::rdf_api::TripleSet,
                         (*i)->getTriplesWhereSubject());
      RETURN_INTO_OBJREF(ste, iface::rdf_api::TripleEnumerator,
                         sts->enumerateTriples());
      while (true)
      {
        RETURN_INTO_OBJREF(t, iface::rdf_api::Triple,
                           ste->getNextTriple());
        if (t == NULL)
          break;

        RETURN_INTO_OBJREF(predr, iface::rdf_api::Resource,
                           t->predicate());
        DECLARE_QUERY_INTERFACE_OBJREF(pred, predr, rdf_api::URIReference);
        // If predicate is a blank node, we can't serialise it so don't try...
        if (pred == NULL)
          continue;
        
        RETURN_INTO_WSTRING(predURI, pred->URI());

        // Find the last non-alphanum character...
        size_t pos = predURI.length() - 1;
        for (; pos > 0; pos--)
        {
          wchar_t c = predURI[pos];
          if (!((c >= L'A' && c <= L'Z') ||
                (c >= L'a' && c <= L'z') ||
                (c >= L'0' && c <= L'9') ||
                c == L'_')
             )
            break;
        }

        std::wstring ns = predURI.substr(0, pos + 1);
        std::wstring ln = predURI.substr(pos + 1);

        RETURN_INTO_OBJREF(propEl, iface::dom::Element,
                           mDoc->createElementNS(ns.c_str(), ln.c_str()));
        descr->appendChild(propEl)->release_ref();

        // Look at the type of object...
        RETURN_INTO_OBJREF(objn, iface::rdf_api::Node, t->object());
        DECLARE_QUERY_INTERFACE_OBJREF(objl, objn, rdf_api::Literal);

        if (objl != NULL)
        {
          RETURN_INTO_WSTRING(lf, objl->lexicalForm());
          DECLARE_QUERY_INTERFACE_OBJREF(objtl, objl, rdf_api::TypedLiteral);
          if (objtl != NULL)
          {
            RETURN_INTO_WSTRING(dtu, objtl->datatypeURI());
            if (dtu == RDF_NS L"XMLLiteral" && lf == L"")
            {
              propEl->setAttributeNS(RDF_NS, L"rdf:parseType", L"Literal");
              continue;
            }

            propEl->setAttributeNS(RDF_NS, L"rdf:datatype", dtu.c_str());
          }
          else
          {
            DECLARE_QUERY_INTERFACE_OBJREF(objpl, objl, rdf_api::PlainLiteral);
            if (objpl != NULL)
            {
              RETURN_INTO_WSTRING(l, objpl->language());
              propEl->setAttributeNS(L"http://www.w3.org/XML/1998/namespace",
                                     L"xml:lang", l.c_str());
            }
          }

          RETURN_INTO_OBJREF(tn, iface::dom::Text,
                             mDoc->createTextNode(lf.c_str()));
          propEl->appendChild(tn)->release_ref();
        }
        else
        {
          DECLARE_QUERY_INTERFACE_OBJREF(objr, objn, rdf_api::Resource);
          RETURN_INTO_OBJREF(obje, iface::dom::Element,
                             createNodeElementForResource(objr));
          propEl->appendChild(obje)->release_ref();
        }
      }
    }
  }

private:
  URI mBase;
  iface::dom::Document* mDoc;
  iface::dom::Element* mRDFEl;
  iface::rdf_api::DataSource* mDataSource;

  std::wstring
  getOrAssignNodeID(iface::rdf_api::Node* n)
  {
    std::string objId(n->objid());

    std::map<std::string, std::wstring>::iterator i = mNodeIds.find(objId);
    if (i != mNodeIds.end())
    {
      return (*i).second;
    }

    wchar_t nodeId[20];
    any_swprintf(nodeId, 20, L"n%u", ++mLastId);

    mNodeIds.insert(std::pair<std::string, std::wstring>(objId, nodeId));

    return nodeId;
  }

  already_AddRefd<iface::dom::Element>
  createNodeElementForResource(iface::rdf_api::Resource* res)
  {
    RETURN_INTO_OBJREF(descr, iface::dom::Element,
                       mDoc->createElementNS(RDF_NS, L"rdf:Description"));
    DECLARE_QUERY_INTERFACE_OBJREF(uri, res, rdf_api::URIReference);
    if (uri != NULL)
    {
      RETURN_INTO_WSTRING(uSubjStr, uri->URI());
      URI uSubj(uSubjStr);
      descr->setAttributeNS(RDF_NS, L"rdf:about",
                            uSubj.relativeURI(mBase).c_str());
    }
    else
    {
      // It must be a blank node. 
      descr->setAttributeNS(RDF_NS, L"rdf:nodeID",
                            getOrAssignNodeID(res).c_str());
    }

    descr->add_ref();
    return descr.getPointer();
  }

  uint32_t mLastId;
  std::map<std::string, std::wstring> mNodeIds;
};

already_AddRefd<iface::dom::Document>
CDA_RDFBootstrap::getDOMForDataSource
(
 iface::rdf_api::DataSource* aDataSource,
 const std::wstring& aBaseURI
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(impl, iface::dom::DOMImplementation, CreateDOMImplementation());
  RETURN_INTO_OBJREF(rdoc, iface::dom::Document,
                     impl->createDocument(RDF_NS, L"rdf:RDF", NULL));
  RETURN_INTO_OBJREF(rdocel, iface::dom::Element,
                     rdoc->documentElement());
  RDFDOMBuilder b(rdoc, rdocel, aDataSource, aBaseURI);

  rdoc->add_ref();
  return rdoc.getPointer();
}

std::wstring
CDA_RDFBootstrap::serialiseDataSource
(
 iface::rdf_api::DataSource* aDataSource,
 const std::wstring& aBaseURI
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     getDOMForDataSource(aDataSource, aBaseURI));
  DOMWriter dw;
  std::wstring str;
  dw.writeDocument(NULL, doc, str);
  return str;
}

already_AddRefd<iface::rdf_api::Bootstrap>
CreateRDFBootstrap()
{
  return new CDA_RDFBootstrap();
}
