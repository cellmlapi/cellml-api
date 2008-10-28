#define MODULE_CONTAINS_RDFAPISPEC
#define IN_DOMWRITER_MODULE
#define IN_DOM_MODULE
#include "RDFImplementation.hpp"
#include "Utilities.hxx"
#include "URITool.hpp"
#include "DOMBootstrap.hxx"
#include "DOMWriter.hxx"
#include "RDFBootstrap.hpp"

#ifdef _WIN32
#define swprintf _snwprintf
#endif

CDA_DataSource::CDA_DataSource()
  : _cda_refcount(1), mTripleSet(new CDA_AllTriplesSet(this))
{
}

CDA_DataSource::~CDA_DataSource()
{
  for (std::set<CDA_RDFNode*>::iterator i = mAssociatedNodes.begin();
       i != mAssociatedNodes.end();
       i++)
    delete *i;

  delete mTripleSet;
}

iface::rdf_api::URIReference*
CDA_DataSource::getURIReference(const wchar_t* aURI)
  throw(std::exception&)
{
  // See if it already exists...
  std::map<std::wstring, CDA_URIReference*>::iterator i
    (mURIReferences.find(aURI));

  if (i != mURIReferences.end())
  {
    (*i).second->add_ref();
    return (*i).second;
  }

  // We need to make a new URI reference...
  CDA_URIReference* ur = new CDA_URIReference(aURI, this);
  mURIReferences.insert(std::pair<std::wstring, CDA_URIReference*>(aURI, ur));

  return ur;
}

iface::rdf_api::BlankNode*
CDA_DataSource::createBlankNode()
  throw(std::exception&)
{
  return new CDA_BlankNode(this);
}

iface::rdf_api::PlainLiteral*
CDA_DataSource::getPlainLiteral(const wchar_t* aForm,
                                  const wchar_t* aLanguage)
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

iface::rdf_api::TypedLiteral*
CDA_DataSource::getTypedLiteral(const wchar_t* aForm,
                                const wchar_t* aDatatypeURI)
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

iface::rdf_api::TripleSet*
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

iface::rdf_api::TripleSet*
CDA_RDFNode::getTriplesInto()
  throw(std::exception&)
{
  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, NULL, NULL, this);
}

iface::rdf_api::TripleSet*
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

iface::rdf_api::TripleSet*
CDA_Resource::getTriplesOutOfByPredicate(iface::rdf_api::Resource* aPredicate)
  throw(std::exception&)
{
  CDA_Resource* pred = dynamic_cast<CDA_Resource*>(aPredicate);
  if (pred == NULL)
    throw iface::rdf_api::RDFProcessingError();

  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, this, pred, NULL);
}

iface::rdf_api::Triple*
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
  return t;
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

iface::rdf_api::TripleSet*
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

iface::rdf_api::TripleSet*
CDA_Resource::getTriplesWherePredicate()
  throw(std::exception&)
{
  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, NULL, this, NULL);
}

iface::rdf_api::TripleSet*
CDA_Resource::getTriplesWhereSubject()
  throw(std::exception&)
{
  return new CDA_FilteringTripleSet(mDataSource->mTripleSet, this, NULL, NULL);
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
 const wchar_t* aURI,
 CDA_DataSource* aDataSource
)
  : CDA_Resource(aDataSource), mURI(aURI)
{
}

CDA_URIReference::~CDA_URIReference()
{
  mDataSource->uriReferenceDeleted(this);
}

wchar_t*
CDA_URIReference::URI()
  throw()
{
  return CDA_wcsdup(mURI.c_str());
}

CDA_Literal::CDA_Literal(const wchar_t* aLexicalForm, CDA_DataSource* aDataSource)
  : CDA_RDFNode(aDataSource), mLexicalForm(aLexicalForm)
{
}

CDA_Literal::~CDA_Literal()
{
}

wchar_t*
CDA_Literal::lexicalForm()
  throw()
{
  return CDA_wcsdup(mLexicalForm.c_str());
}

CDA_PlainLiteral::CDA_PlainLiteral(const wchar_t* aLexicalForm,
                                   const wchar_t* aLanguage,
                                   CDA_DataSource* aDataSource)
  : CDA_Literal(aLexicalForm, aDataSource), mLanguage(aLanguage)
{
}

CDA_PlainLiteral::~CDA_PlainLiteral()
{
  mDataSource->plainLiteralDeleted(this);
}

wchar_t*
CDA_PlainLiteral::language()
  throw()
{
  return CDA_wcsdup(mLanguage.c_str());
}

CDA_TypedLiteral::CDA_TypedLiteral
(
 const wchar_t* aLexicalForm, const wchar_t* aTypeURI,
 CDA_DataSource* aDataSource
)
  : CDA_Literal(aLexicalForm, aDataSource), mTypeURI(aTypeURI)
{
}

CDA_TypedLiteral::~CDA_TypedLiteral()
{
  mDataSource->typedLiteralDeleted(this);
}

wchar_t*
CDA_TypedLiteral::datatypeURI()
  throw()
{
  return CDA_wcsdup(mTypeURI.c_str());
}

CDA_Triple::CDA_Triple(CDA_DataSource* aDataSource, CDA_Resource* aSubject,
                       CDA_Resource* aPredicate, CDA_RDFNode* aObject)
  : _cda_refcount(1), mDataSource(aDataSource), mSubject(aSubject),
    mPredicate(aPredicate), mObject(aObject)
{
}

CDA_Triple::~CDA_Triple()
{
}

iface::rdf_api::Resource*
CDA_Triple::subject()
  throw(std::exception&)
{
  mSubject->add_ref();
  return mSubject;
}

iface::rdf_api::Resource*
CDA_Triple::predicate()
  throw(std::exception&)
{
  mPredicate->add_ref();
  return mPredicate;
}

iface::rdf_api::Node*
CDA_Triple::object()
  throw(std::exception&)
{
  mObject->add_ref();
  return mObject;
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
  : _cda_refcount(1), mSet(aSet), mDataSource(aDataSource),
    mPosition(aPosition), mEnd(aEnd)
{
}

CDA_AllTriplesEnumerator::~CDA_AllTriplesEnumerator()
{
  mSet->enumeratorDeleted(this);
}

iface::rdf_api::Triple*
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

iface::rdf_api::TripleEnumerator*
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
  : _cda_refcount(1), mMasterSource(aMasterSource),
    mSubjectFilter(aSubjectFilter), mPredicateFilter(aPredicateFilter),
    mObjectFilter(aObjectFilter)
{
}

CDA_FilteringTripleSet::~CDA_FilteringTripleSet()
{
}

iface::rdf_api::TripleEnumerator*
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
  : _cda_refcount(1), mMasterEnum(aMasterEnum), mSubjectFilter(aSubjectFilter),
    mPredicateFilter(aPredicateFilter), mObjectFilter(aObjectFilter)
{
}

CDA_FilteringTripleEnumerator::~CDA_FilteringTripleEnumerator()
{
}

iface::rdf_api::Triple*
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
    return t;
  }
}

CDA_RDFBootstrap::CDA_RDFBootstrap()
  : _cda_refcount(1)
{
}

CDA_RDFBootstrap::~CDA_RDFBootstrap()
{
}

iface::rdf_api::DataSource*
CDA_RDFBootstrap::createDataSource()
  throw(std::exception&)
{
  return new CDA_DataSource();
}

#define RDF_NS L"http://www.w3.org/1999/02/22-rdf-syntax-ns#"
/* We should really support xml:lang properly. But for now it is hardcoded. */
#define LANGUAGE L"en"

static wchar_t*
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

  return CDA_wcsdup(LANGUAGE);
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
    return ns + ln;
  }

  bool isXMLAttributeURI(const std::wstring& aURI)
  {
    return (aURI.substr(0, 36) == L"http://www.w3.org/XML/1998/namespace");
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

  iface::rdf_api::BlankNode*
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
          swprintf(buf, 20, L"%u", liCounter++);
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
          atURI.substr(0, 36) != L"http://www.w3.org/XML/1998/namespace")
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
 const wchar_t* aBaseURI
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

    std::set<iface::rdf_api::Resource*, XPCOMComparator> subjects;
    while (true)
    {
      RETURN_INTO_OBJREF(t, iface::rdf_api::Triple,
                         te->getNextTriple());
      if (t == NULL)
        break;

      RETURN_INTO_OBJREF(subj, iface::rdf_api::Resource, t->subject());
      subjects.insert(subj);
    }

    for (std::set<iface::rdf_api::Resource*, XPCOMComparator>::iterator i
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
              propEl->setAttributeNS(RDF_NS, L"parseType", L"Literal");
              continue;
            }

            propEl->setAttributeNS(RDF_NS, L"datatype", dtu.c_str());
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
    char* tmp = n->objid();
    std::string objId(tmp);
    free(tmp);

    std::map<std::string, std::wstring>::iterator i = mNodeIds.find(objId);
    if (i != mNodeIds.end())
    {
      return (*i).second;
    }

    wchar_t nodeId[20];
    swprintf(nodeId, 20, L"n%u", ++mLastId);

    mNodeIds.insert(std::pair<std::string, std::wstring>(objId, nodeId));

    return nodeId;
  }

  iface::dom::Element*
  createNodeElementForResource(iface::rdf_api::Resource* res)
  {
    RETURN_INTO_OBJREF(descr, iface::dom::Element,
                       mDoc->createElementNS(RDF_NS, L"Description"));
    DECLARE_QUERY_INTERFACE_OBJREF(uri, res, rdf_api::URIReference);
    if (uri != NULL)
    {
      RETURN_INTO_WSTRING(uSubjStr, uri->URI());
      URI uSubj(uSubjStr);
      descr->setAttributeNS(RDF_NS, L"about",
                            uSubj.relativeURI(mBase).c_str());
    }
    else
    {
      // It must be a blank node. 
      descr->setAttributeNS(RDF_NS, L"nodeID",
                            getOrAssignNodeID(res).c_str());
    }

    descr->add_ref();
    return descr;
  }

  uint32_t mLastId;
  std::map<std::string, std::wstring> mNodeIds;
};

iface::dom::Document*
CDA_RDFBootstrap::getDOMForDataSource
(
 iface::rdf_api::DataSource* aDataSource,
 const wchar_t* aBaseURI
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(impl, iface::dom::DOMImplementation, CreateDOMImplementation());
  RETURN_INTO_OBJREF(rdoc, iface::dom::Document,
                     impl->createDocument(RDF_NS, L"RDF", NULL));
  RETURN_INTO_OBJREF(rdocel, iface::dom::Element,
                     rdoc->documentElement());
  RDFDOMBuilder b(rdoc, rdocel, aDataSource, aBaseURI);

  rdoc->add_ref();
  return rdoc;
}

wchar_t*
CDA_RDFBootstrap::serialiseDataSource
(
 iface::rdf_api::DataSource* aDataSource,
 const wchar_t* aBaseURI
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     getDOMForDataSource(aDataSource, aBaseURI));
  DOMWriter dw;
  std::wstring str;
  dw.writeDocument(NULL, doc, str);
  return CDA_wcsdup(str.c_str());
}

iface::rdf_api::Bootstrap*
CreateRDFBootstrap()
{
  return new CDA_RDFBootstrap();
}
