#include "cda_config.h"
#include "Utilities.hxx"
#include "IfaceRDF_APISPEC.hxx"
#include <map>
#include <string>
#include <set>

class CDA_PlainLiteral;
class CDA_TypedLiteral;
class CDA_URIReference;
class CDA_AllTriplesSet;
class CDA_Resource;
class CDA_RDFNode;

#undef assert

class CDA_DataSource
  : public iface::rdf_api::DataSource
{
public:
  CDA_DataSource();
  ~CDA_DataSource();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(rdf_api::DataSource);
  CDA_IMPL_ID;

  iface::rdf_api::URIReference* getURIReference(const wchar_t* aURI)
    throw(std::exception&);
  iface::rdf_api::BlankNode* createBlankNode()
    throw(std::exception&);
  iface::rdf_api::PlainLiteral* getPlainLiteral(const wchar_t* aForm,
                                                const wchar_t* aLanguage)
    throw(std::exception&);
  iface::rdf_api::TypedLiteral* getTypedLiteral(const wchar_t* aForm,
                                                const wchar_t* datatypeURI)
    throw(std::exception&);
  iface::rdf_api::TripleSet* getAllTriples()
    throw(std::exception&);

  // Implementation use only...
  void plainLiteralDeleted(CDA_PlainLiteral* aLit);
  void typedLiteralDeleted(CDA_TypedLiteral* aLit);
  void uriReferenceDeleted(CDA_URIReference* aURIRef);
  void assert(CDA_Resource* aSubject, CDA_Resource* aPredicate, CDA_RDFNode* aObject);
  void unassert(CDA_Resource* aSubject, CDA_Resource* aPredicate, CDA_RDFNode* aObject);

  void nodeAssociated(CDA_RDFNode* aNode);
  void nodeDissociated(CDA_RDFNode* aNode);

  CDA_AllTriplesSet* mTripleSet;

private:
  typedef std::pair<std::wstring, std::wstring> wstringpair;
  std::map<wstringpair, CDA_TypedLiteral*> mTypedLiterals;
  std::map<wstringpair, CDA_PlainLiteral*> mPlainLiterals;
  std::map<std::wstring, CDA_URIReference*> mURIReferences;
  std::set<CDA_RDFNode*> mAssociatedNodes;
};

class CDA_RDFNode
  : public virtual iface::rdf_api::Node
{
public:
  CDA_RDFNode(CDA_DataSource* aDataSource);
  ~CDA_RDFNode();

  iface::rdf_api::TripleSet* getTriplesInto()
    throw(std::exception&);
  iface::rdf_api::TripleSet* getTriplesIntoByPredicate(iface::rdf_api::Resource*
                                                       aPredicate)
    throw(std::exception&);

  void add_ref() throw();
  void release_ref() throw();

  // Implementation use only...
  void addAssociation() throw();
  void releaseAssociation() throw();

  CDA_DataSource* mDataSource;
private:
  uint32_t mRefCount, mAssociationCount;
};

class CDA_Resource
  : public CDA_RDFNode, public virtual iface::rdf_api::Resource
{
public:
  CDA_Resource(CDA_DataSource* aDataSource);
  ~CDA_Resource();

  iface::rdf_api::TripleSet* getTriplesOutOfByPredicate(iface::rdf_api::Resource* aPredicate)
    throw(std::exception&);
  iface::rdf_api::Triple* getTripleOutOfByPredicate
  (iface::rdf_api::Resource* aPredicate)
    throw(std::exception&);
  bool hasTripleOutOf(iface::rdf_api::Resource* aPredicate,
                      iface::rdf_api::Node* aObject)
    throw(std::exception&);
  void createTripleOutOf(iface::rdf_api::Resource* aPredicate,
                         iface::rdf_api::Node* aObject)
    throw(std::exception&);
  iface::rdf_api::TripleSet* getTriplesOutOfByObject
  (iface::rdf_api::Node* aObject)
    throw(std::exception&);
  iface::rdf_api::TripleSet* getTriplesWherePredicate()
    throw(std::exception&);
  iface::rdf_api::TripleSet* getTriplesWhereSubject()
    throw(std::exception&);
  iface::rdf_api::Container* correspondingContainer()
    throw(std::exception&);
  iface::rdf_api::Container* findOrMakeContainer(iface::rdf_api::Resource* aPredicate,
                                                 iface::rdf_api::Resource* aContainerType)
    throw(std::exception&);
};

class CDA_NodeIteratorContainer
  : public iface::rdf_api::NodeIterator
{
public:
  CDA_NodeIteratorContainer(CDA_DataSource* aDataSource,
                            CDA_Resource* aResource)
    : _cda_refcount(1), mNextIndex(1), mDataSource(aDataSource), mResource(aResource)
  {
  }

  ~CDA_NodeIteratorContainer() {}

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(rdf_api::NodeIterator);

  iface::rdf_api::Node* getNextNode()
    throw(std::exception&);

private:
  uint32_t mNextIndex;
  ObjRef<CDA_DataSource> mDataSource;
  ObjRef<CDA_Resource> mResource;
};

class CDA_Container
  : public iface::rdf_api::Container
{
public:
  CDA_Container(CDA_DataSource* aDataSource,
                CDA_Resource* aCorrespondingResource)
    : _cda_refcount(1),
      mDataSource(aDataSource), mCorrespondingResource(aCorrespondingResource)
  {}
  ~CDA_Container() {};

  CDA_IMPL_QI1(rdf_api::Container);
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  iface::rdf_api::Resource* correspondingResource()
    throw(std::exception&);
  iface::rdf_api::Resource* containerType()
    throw(std::exception&);
  void containerType(iface::rdf_api::Resource* aType)
    throw(std::exception&);
  iface::rdf_api::NodeIterator* iterateChildren()
    throw(std::exception&);
  void appendChild(iface::rdf_api::Node* aChild)
    throw(std::exception&);
  void removeChild(iface::rdf_api::Node*, bool aDoRenumbering)
    throw(std::exception&);
  void renumberContainer()
    throw(std::exception&);
  iface::rdf_api::Container* mergeWith(iface::rdf_api::Container* aContainer)
    throw(std::exception&);

private:
  ObjRef<CDA_DataSource> mDataSource;
  ObjRef<CDA_Resource> mCorrespondingResource;
};

class CDA_NodeIteratorMergedContainer
  : public iface::rdf_api::NodeIterator
{
public:
  CDA_NodeIteratorMergedContainer(iface::rdf_api::NodeIterator * it1,
                                  iface::rdf_api::NodeIterator * it2)
    : _cda_refcount(1), mIterator1(it1), mIterator2(it2), mIterator1Done(false)
  {
  }

  ~CDA_NodeIteratorMergedContainer() {}
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(rdf_api::NodeIterator);

  iface::rdf_api::Node* getNextNode() throw(std::exception&);

private:
  ObjRef<iface::rdf_api::NodeIterator> mIterator1, mIterator2;
  bool mIterator1Done;
};

class CDA_MergedContainer
  : public iface::rdf_api::Container
{
public:
  CDA_MergedContainer(iface::rdf_api::Container* aC1,
                      iface::rdf_api::Container* aC2)
    : _cda_refcount(1), mContainer1(aC1), mContainer2(aC2) {}
  ~CDA_MergedContainer() {};

  CDA_IMPL_QI1(rdf_api::Container);
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  iface::rdf_api::Resource* correspondingResource()
    throw(std::exception&);
  iface::rdf_api::Resource* containerType()
    throw(std::exception&);
  void containerType(iface::rdf_api::Resource* aType)
    throw(std::exception&);
  iface::rdf_api::NodeIterator* iterateChildren()
    throw(std::exception&);
  void appendChild(iface::rdf_api::Node* aChild)
    throw(std::exception&);
  void removeChild(iface::rdf_api::Node*, bool aDoRenumbering)
    throw(std::exception&);
  void renumberContainer()
    throw(std::exception&);
  iface::rdf_api::Container* mergeWith(iface::rdf_api::Container* aContainer)
    throw(std::exception&);

private:
  ObjRef<iface::rdf_api::Container> mContainer1, mContainer2;
};

class CDA_BlankNode
  : public CDA_Resource, public iface::rdf_api::BlankNode
{
public:
  CDA_BlankNode(CDA_DataSource* aDataSource);
  ~CDA_BlankNode();

  CDA_IMPL_QI3(rdf_api::Node, rdf_api::Resource, rdf_api::BlankNode);
  CDA_IMPL_ID;
};

class CDA_URIReference
  : public CDA_Resource, public iface::rdf_api::URIReference
{
public:
  CDA_URIReference(const wchar_t* aURI, CDA_DataSource* aDataSource);
  ~CDA_URIReference();

  CDA_IMPL_QI3(rdf_api::Node, rdf_api::Resource, rdf_api::URIReference);
  CDA_IMPL_ID;

  wchar_t* URI() throw();

  // Implementation use only...
  std::wstring mURI;
};

class CDA_Literal
  : public CDA_RDFNode, public virtual iface::rdf_api::Literal
{
public:
  CDA_Literal(const wchar_t* aLexicalForm, CDA_DataSource* aDataSource);
  ~CDA_Literal();

  wchar_t* lexicalForm() throw();

  // Implementation use only...
  std::wstring mLexicalForm;
};

class CDA_PlainLiteral
  : public CDA_Literal, public iface::rdf_api::PlainLiteral
{
public:
  CDA_PlainLiteral(const wchar_t* aLexicalForm, const wchar_t* aLanguage,
                   CDA_DataSource* aDataSource);
  ~CDA_PlainLiteral();

  CDA_IMPL_QI3(rdf_api::Node, rdf_api::Literal, rdf_api::PlainLiteral);
  CDA_IMPL_ID;

  wchar_t* language() throw();

  // Implementation use only...
  std::wstring mLanguage;
};

class CDA_TypedLiteral
  : public CDA_Literal, public iface::rdf_api::TypedLiteral
{
public:
  CDA_TypedLiteral(const wchar_t* aLexicalForm, const wchar_t* aTypeURI,
                   CDA_DataSource* aDataSource);
  ~CDA_TypedLiteral();

  CDA_IMPL_QI3(rdf_api::Node, rdf_api::Literal, rdf_api::TypedLiteral);
  CDA_IMPL_ID;

  wchar_t* datatypeURI() throw();

  // Implementation use only...
  std::wstring mTypeURI;
};

class CDA_Triple
  : public iface::rdf_api::Triple
{
public:
  CDA_Triple(CDA_DataSource* aDataSource, CDA_Resource* aSubject,
             CDA_Resource* aPredicate, CDA_RDFNode* aObject);
  ~CDA_Triple();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(rdf_api::Triple);
  CDA_IMPL_ID;

  iface::rdf_api::Resource* subject() throw(std::exception&);
  iface::rdf_api::Resource* predicate() throw(std::exception&);
  iface::rdf_api::Node* object() throw(std::exception&);
  void unassert() throw();

private:
  ObjRef<CDA_DataSource> mDataSource;
  ObjRef<CDA_Resource> mSubject, mPredicate;
  ObjRef<CDA_RDFNode> mObject;
};

class CDA_AllTriplesEnumerator;

class CDA_AllTriplesSet
  : public iface::rdf_api::TripleSet
{
public:
  CDA_AllTriplesSet(CDA_DataSource* aDataSource);
  ~CDA_AllTriplesSet();

  void add_ref()
    throw()
  {
    mDataSource->add_ref();
  }

  void release_ref()
    throw()
  {
    mDataSource->release_ref();
  }

  CDA_IMPL_QI1(rdf_api::TripleSet);
  CDA_IMPL_ID;

  iface::rdf_api::TripleEnumerator* enumerateTriples()
    throw(std::exception&);

  // Implementation use only...
  bool assert(CDA_Resource* aSubject,
              CDA_Resource* aPredicate,
              CDA_RDFNode* aObject);
  bool unassert(CDA_Resource* aSubject,
                CDA_Resource* aPredicate,
                CDA_RDFNode* aObject);
  void enumeratorDeleted(CDA_AllTriplesEnumerator* aEnum);

  struct RealTriple
  {
    RealTriple(CDA_Resource* aSubj, CDA_Resource* aPred, CDA_RDFNode* aObj)
      : subj(aSubj), pred(aPred), obj(aObj)
    {
    }

    CDA_Resource * subj, * pred;
    CDA_RDFNode* obj;

    bool
    operator<(const RealTriple& aRT) const
    {
      if (subj == aRT.subj)
      {
        if (pred == aRT.pred)
          return obj < aRT.obj;
        else
          return pred < aRT.pred;
      }
      return subj < aRT.subj;
    }
  };

private:
  std::set<RealTriple> mRealTriples;
  std::set<CDA_AllTriplesEnumerator*> mLiveEnumerators;
  CDA_DataSource* mDataSource;
};

class CDA_AllTriplesEnumerator
  : public iface::rdf_api::TripleEnumerator
{
public:
  CDA_AllTriplesEnumerator(
                           CDA_DataSource* aDataSource,
                           CDA_AllTriplesSet* aSet,
                           const std::set<CDA_AllTriplesSet::RealTriple>::iterator& aPosition,
                           const std::set<CDA_AllTriplesSet::RealTriple>::iterator& aEnd
                          );
  ~CDA_AllTriplesEnumerator();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(rdf_api::TripleEnumerator);
  CDA_IMPL_ID;

  iface::rdf_api::Triple* getNextTriple() throw(std::exception&);
  void aboutToDelete(const std::set<CDA_AllTriplesSet::RealTriple>::iterator& aWhere);

private:
  ObjRef<CDA_AllTriplesSet> mSet;
  CDA_DataSource* mDataSource;
  std::set<CDA_AllTriplesSet::RealTriple>::iterator mPosition, mEnd;
};

class CDA_FilteringTripleSet
  : public iface::rdf_api::TripleSet
{
public:
  CDA_FilteringTripleSet(CDA_AllTriplesSet* aMasterSource, CDA_Resource* aSubjectFilter,
                         CDA_Resource* aPredicateFilter, CDA_RDFNode* aObjectFilter);
  ~CDA_FilteringTripleSet();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(rdf_api::TripleSet);
  CDA_IMPL_ID;
  
  iface::rdf_api::TripleEnumerator* enumerateTriples()
    throw(std::exception&);
private:
  ObjRef<CDA_AllTriplesSet> mMasterSource;
  ObjRef<CDA_Resource> mSubjectFilter, mPredicateFilter;
  ObjRef<CDA_RDFNode> mObjectFilter;
};

class CDA_FilteringTripleEnumerator
  : public iface::rdf_api::TripleEnumerator
{
public:
  CDA_FilteringTripleEnumerator(iface::rdf_api::TripleEnumerator* aMasterEnum,
                                CDA_Resource* aSubjectFilter,
                                CDA_Resource* aPredicateFilter, CDA_RDFNode* aObjectFilter);
  ~CDA_FilteringTripleEnumerator();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(rdf_api::TripleEnumerator);
  CDA_IMPL_ID;

  iface::rdf_api::Triple* getNextTriple() throw(std::exception&);

private:
  ObjRef<iface::rdf_api::TripleEnumerator> mMasterEnum;
  ObjRef<CDA_Resource> mSubjectFilter, mPredicateFilter;
  ObjRef<CDA_RDFNode> mObjectFilter;
};

class CDA_RDFBootstrap
  : public iface::rdf_api::Bootstrap
{
public:
  CDA_RDFBootstrap();
  ~CDA_RDFBootstrap();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(rdf_api::Bootstrap);

  iface::rdf_api::DataSource* createDataSource()
    throw(std::exception&);
  void parseIntoDataSource(iface::rdf_api::DataSource* aDataSource,
                           iface::dom::Element* aRoot,
                           const wchar_t* aBaseURI)
    throw(std::exception&);

  iface::dom::Document* getDOMForDataSource
  (
   iface::rdf_api::DataSource* aDataSource,
   const wchar_t* aBaseURI
  )
    throw(std::exception&);

  wchar_t* serialiseDataSource
  (
   iface::rdf_api::DataSource* aDataSource,
   const wchar_t* aBaseURI
  )
    throw(std::exception&);
};
