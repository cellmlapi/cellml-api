#define IN_XPATH_MODULE
#define MODULE_CONTAINS_xpath
#include "Utilities.hxx"
#include "Ifacexpath.hxx"
#include "IfaceDOM_events.hxx"
#include "XPathBootstrap.hpp"
#include <list>
#include <iterator>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <limits>
#include <algorithm>
#include <math.h>
#include <assert.h>

#ifdef _MSC_VER
#define finite _finite
#define isnan _isnan
#endif

template<typename C> class XPCOMContainerRAII
{
public:
  XPCOMContainerRAII(C& aCont)
    : mCont(aCont)
  {
  }

  ~XPCOMContainerRAII()
  {
    for (typename C::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i)->release_ref();
    mCont.clear();
  }

private:
  C& mCont;
};

class CDA_XPathEvaluator
  : public iface::xpath::XPathEvaluator
{
public:
  CDA_XPathEvaluator() {}
  ~CDA_XPathEvaluator() {}
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathEvaluator);

  already_AddRefd<iface::xpath::XPathExpression> createExpression(const std::wstring& aExpression,
								  iface::xpath::XPathNSResolver* aResolver)
    throw(std::exception&);
  already_AddRefd<iface::xpath::XPathNSResolver> createNSResolver(iface::dom::Node* aNodeResolver)
    throw();
  already_AddRefd<iface::xpath::XPathResult> evaluate(const std::wstring& aExpression,
						      iface::dom::Node* aContextNode,
						      iface::xpath::XPathNSResolver* aResolver,
						      uint16_t aType,
						      iface::xpath::XPathResult* aResult)
    throw(std::exception&);
};

already_AddRefd<iface::xpath::XPathEvaluator> CreateXPathEvaluator()
{
  return new CDA_XPathEvaluator();
}

template<typename C>
class XPCOMContainerFirstRAII
{
public:
  XPCOMContainerFirstRAII(C& aCont)
    : mCont(aCont)
  {
  }

  ~XPCOMContainerFirstRAII()
  {
    for (typename C::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i).first->release_ref();
    mCont.clear();
  }

private:
  C& mCont;
};

template<typename C>
class XPCOMContainerSecondRAII
{
public:
  XPCOMContainerSecondRAII(C& aCont)
    : mCont(aCont)
  {
  }

  ~XPCOMContainerSecondRAII()
  {
    for (typename C::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i).second->release_ref();
    mCont.clear();
  }

private:
  C& mCont;
};

static std::wstring stringValueOf(iface::dom::Node* n)
{
  uint16_t nt = n->nodeType();
  if (nt != iface::dom::Node::ELEMENT_NODE &&
      nt != iface::dom::Node::DOCUMENT_NODE)
  {
    RETURN_INTO_WSTRING(s, n->nodeValue());
    return s;
  }

  RETURN_INTO_OBJREF(cn, iface::dom::Node, n->firstChild());
  std::wstring value;
  while (cn)
  {
    value += stringValueOf(cn);
    cn = cn->nextSibling();
  }

  return value;
}

static double stringToNumber(const std::wstring& aNumberStr)
{
  const wchar_t* p = aNumberStr.c_str();
  while (*p == L' ' || *p == '\n' || *p == '\t' || *p == '\r')
    p++;
  if (*p == 0)
    return std::numeric_limits<double>::quiet_NaN();

  const wchar_t* endptr;
  double number = wcstod(p, (wchar_t**)&endptr);
  while (*endptr == L' ' || *endptr == L'\n' || *endptr == L'\t' || *endptr == L'\r')
    endptr++;

  if (*p != 0)
    number = std::numeric_limits<double>::quiet_NaN();

  return number;
}

class CDA_XPathResult
  : public iface::xpath::XPathResult
{
public:
  CDA_XPathResult(bool aIsInternal = true) : mIsInternal(aIsInternal),
                                             mIsInvalid(false), mELI(this) {}
  ~CDA_XPathResult() { cleanup(); }
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(xpath::XPathResult);

  bool booleanValue()
    throw(std::exception&)
  {
    ensureType(iface::xpath::XPathResult::BOOLEAN_TYPE);
    return mBoolean;
  }

  bool invalidIteratorState()
    throw(std::exception&)
  {
    return mIsInvalid;
  }

  double numberValue()
    throw(std::exception&)
  {
    ensureType(iface::xpath::XPathResult::NUMBER_TYPE);
    return mNumber;
  }

  uint16_t resultType() throw() { return mType; }

  already_AddRefd<iface::dom::Node>
  singleNodeValue() throw(std::exception&)
  {
    if (iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE != mType &&
        iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE != mType)
      throw iface::xpath::XPathException();

    if (mNodes.begin() == mNodes.end())
      return NULL;
    iface::dom::Node* n = mNodes.front();
    n->add_ref();
    return n;
  }

  uint32_t
  snapshotLength() throw(std::exception&)
  {
     if (!mIsInternal && iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE != mType &&
        iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE != mType)
      throw iface::xpath::XPathException();

     return mNodes.size();
  }

  std::wstring stringValue() throw(std::exception&)
  {
    ensureType(iface::xpath::XPathResult::STRING_TYPE);
    return mString;
  }

  already_AddRefd<iface::dom::Node> iterateNext() throw(std::exception&)
  {
    if (mType != iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE &&
        mType != iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE)
      throw iface::xpath::XPathException();

    if (mNodeIt == mNodes.end())
      return NULL;

    iface::dom::Node* n = *mNodeIt;
    mNodeIt++;

    n->add_ref();
    return n;
  }

  already_AddRefd<iface::dom::Node> snapshotItem(uint32_t aIndex) throw(std::exception&)
  {
    if (!mIsInternal && mType != iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE &&
        mType != iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE)
      throw iface::xpath::XPathException();

    if (aIndex >= mNodes.size())
      return NULL;
    iface::dom::Node* n =  mNodes[aIndex];
    n->add_ref();
    return n;
  }

  // Used internally only.
  void makeExternal()
  {
    mIsInternal = false;
    if (mOwnerDoc == NULL)
    {
      if (mNodes.begin() != mNodes.end())
      {
        QUERY_INTERFACE(mOwnerDoc, mNodes.front(), events::EventTarget);
        mOwnerDoc->addEventListener(L"DOMSubtreeModified", &mELI, false);
      }
    }
    mNodeIt = mNodes.begin();
  }

  void cleanup()
  {
    mIsInvalid = false;
    for (std::vector<iface::dom::Node*>::iterator i = mNodes.begin();
         i != mNodes.end();
         i++)
      (*i)->release_ref();
    mNodes.clear();
    mNodeIt = mNodes.begin();
    mString = L"";
    if (mOwnerDoc)
    {
      mOwnerDoc->removeEventListener(L"DOMSubtreeModified", &mELI, false);
      mOwnerDoc = NULL;
    }
  }

  void addNode(iface::dom::Node* aNode)
  {
    if (!mIsInternal && mOwnerDoc == NULL)
    {
      QUERY_INTERFACE(mOwnerDoc, aNode, events::EventTarget);
      mOwnerDoc->addEventListener(L"DOMSubtreeModified", &mELI, false);
    }

    aNode->add_ref();
    mNodes.push_back(aNode);
  }

  void coerceTo(uint16_t aNewType)
  {
    if (aNewType == iface::xpath::XPathResult::ANY_TYPE ||
        aNewType == mType)
      return;

    if (aNewType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
        aNewType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
        aNewType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
        aNewType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
        aNewType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
        aNewType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE)
    {
      // Either there is nothing to do but set the type, or the conversion
      // isn't allowed...
      if (mType != iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE &&
          mType != iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE &&
          mType != iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE &&
          mType != iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE &&
          mType != iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE &&
          mType != iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE)
        throw iface::xpath::XPathException();
      if (aNewType == ANY_UNORDERED_NODE_TYPE)
        mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;
      else
        mType = aNewType;
      return;
    }

    if (aNewType == iface::xpath::XPathResult::STRING_TYPE)
    {
      if (mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
          mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
          mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
          mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
          mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
          mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE)
      {
        mType = iface::xpath::XPathResult::STRING_TYPE;
        if (mNodes.empty())
        {
          mString = L"";
          return;
        }
        
        mString = stringValueOf(mNodes.front());
        return;
      }
      else if (mType == iface::xpath::XPathResult::NUMBER_TYPE)
      {
        mType = iface::xpath::XPathResult::STRING_TYPE;
        if (isnan(mNumber))
        {
          mString = L"NaN";
          return;
        }

        if (!finite(mNumber))
        {
          if (mNumber < 0)
            mString = L"-Infinity";
          else
            mString = L"Infinity";
        }

        wchar_t buf[100];
        any_swprintf(buf, 999, L"%f", mNumber);
        mString = buf;
        return;
      }
      else if (mType == iface::xpath::XPathResult::BOOLEAN_TYPE)
      {
        mType = iface::xpath::XPathResult::STRING_TYPE;
        if (mBoolean)
          mString = L"true";
        else
          mString = L"false";
        return;
      }
      // Unreachable? Should be - but just in case something changes.
      throw iface::xpath::XPathException();
    }

    if (aNewType == iface::xpath::XPathResult::BOOLEAN_TYPE)
    {
      if (mType == iface::xpath::XPathResult::NUMBER_TYPE)
      {
        mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        mBoolean = !isnan(mNumber) && (mNumber != 0);
        return;
      }
      else if (mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
               mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
               mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
               mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
               mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
               mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE)
      {
        mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        mBoolean = !mNodes.empty();
        return;
      }
      else if (mType == iface::xpath::XPathResult::STRING_TYPE)
      {
        mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        mBoolean = mString.size() != 0;
        return;
      }
      // Unreachable? Should be - but just in case something changes.
      throw iface::xpath::XPathException();
    }

    if (aNewType == iface::xpath::XPathResult::NUMBER_TYPE)
    {
      if (mType == iface::xpath::XPathResult::BOOLEAN_TYPE)
      {
        mType = iface::xpath::XPathResult::NUMBER_TYPE;
        mNumber = mBoolean ? 1.0 : 0.0;
        return;
      }
      else if (mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
               mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
               mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
               mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
               mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
               mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE)
      {
        coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
        return;
      }
      else if (mType == iface::xpath::XPathResult::STRING_TYPE)
      {
        mType = iface::xpath::XPathResult::NUMBER_TYPE;
        mNumber = stringToNumber(mString);
        return;
      }
      // Unreachable? Should be - but just in case something changes.
      throw iface::xpath::XPathException();
    }

    throw iface::xpath::XPathException();
  }

  uint16_t mType;
  bool mBoolean, mIsInternal;
  double mNumber;
  bool mIsInvalid;
  std::wstring mString;
  std::vector<iface::dom::Node*> mNodes;
  std::vector<iface::dom::Node*>::iterator mNodeIt;

  void
  ensureType(uint16_t match)
  {
    if (match != mType)
      throw iface::xpath::XPathException();
  }

private:
  ObjRef<iface::events::EventTarget> mOwnerDoc;
  
  class EventListenerInternal
    : public iface::events::EventListener
  {
  public:
    CDA_IMPL_QI1(events::EventListener);
    CDA_IMPL_ID;
    void add_ref() throw() {}
    void release_ref() throw() {}

    EventListenerInternal(CDA_XPathResult* aR) : mR(aR) {}

    // External interface...
    void handleEvent(iface::events::Event*)
      throw()
    {
      mR->mIsInvalid = true;
    }

  private:
    CDA_XPathResult* mR;
  };
  friend class EventListenerInternal;
  EventListenerInternal mELI;
};

class CDA_XPathContext
  : public iface::XPCOM::IObject
{
public:
  CDA_XPathContext(iface::dom::Node* aNode, uint32_t aPos, uint32_t aSize,
                   std::map<std::wstring, std::wstring>& aNS)
    : mNode(aNode), mContextSize(aSize), mContextPos(aPos),
      mNamespaceMap(aNS)
  {
  }

  CDA_XPathContext(iface::dom::Node* aNode, uint32_t aPos, uint32_t aSize)
    : mNode(aNode), mContextSize(aSize), mContextPos(aPos)
  {
  }
  
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;
  
  ObjRef<iface::dom::Node> mNode;
  uint32_t mContextSize, mContextPos;
  std::map<std::wstring, std::wstring> mNamespaceMap;
};

class CDA_XPathExpr
  : public virtual iface::XPCOM::IObject, public iface::xpath::XPathExpression
{
public:
  CDA_XPathExpr() {};

  already_AddRefd<iface::xpath::XPathResult> evaluate(iface::dom::Node* aN, uint16_t aCoerceTo, iface::xpath::XPathResult* aR)
    throw(std::exception&)
  {
    CDA_XPathContext ctx(aN, 1, 1);
    RETURN_INTO_OBJREF(r, CDA_XPathResult, eval(ctx));
    r->coerceTo(aCoerceTo);
    r->makeExternal();

    r->add_ref();
    return r.getPointer();
  }

  virtual already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aCtx) = 0;
};

class CDA_XPathPath
  : public iface::XPCOM::IObject
{
public:
  CDA_XPathPath() {}

  virtual already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aCtx, CDA_XPathResult* aStarting) = 0;
};

class CDA_XPathNodeTest
  : public iface::XPCOM::IObject
{
public:
  CDA_XPathNodeTest() {};

  virtual bool eval(CDA_XPathContext& aCtx, iface::dom::Node* aNode) = 0;
};

class CDA_XPathOrExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathOrExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2) : mExpr1(e1), mExpr2(e2) {};

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aCtx)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aCtx));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aCtx));
    r1->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);

    r1->mBoolean = r1->mBoolean || r2->mBoolean;

    r1->add_ref();
    return r1.getPointer();
  }

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathAndExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathAndExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2) {}

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aCtx)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aCtx));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aCtx));
    r1->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);

    r1->mBoolean = r1->mBoolean && r2->mBoolean;

    r1->add_ref();
    return r1.getPointer();
  }

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathComparisonExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathComparisonExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2) {};

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aCtx)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aCtx));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aCtx));

    bool result = compareRaw(r1, r2);

    r1->mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
    r1->mBoolean = result;

    r1->add_ref();
    return r1.getPointer();
  }

  bool compareRaw(CDA_XPathResult* e1, CDA_XPathResult* e2)
  {
    bool n1 = (e1->mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
               e1->mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
               e1->mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
               e1->mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
               e1->mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
               e1->mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE);
    bool n2 = (e2->mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
               e2->mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
               e2->mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
               e2->mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
               e2->mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
               e2->mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE);
    if (!n1 && !n2)
      return compareNotSet(e1, e2);

    if (n1 && n2)
    {
      std::set<std::wstring> set1, set2;
      for (std::vector<iface::dom::Node*>::iterator i = e1->mNodes.begin(); 
           i != e1->mNodes.end(); i++)
        set1.insert(stringValueOf(*i));
      for (std::vector<iface::dom::Node*>::iterator i = e2->mNodes.begin(); 
           i != e2->mNodes.end(); i++)
        set2.insert(stringValueOf(*i));
      if (set1.empty() || set2.empty())
        return false;
      return compareSets(set1, set2);
    }

    if (e1->mType == iface::xpath::XPathResult::NUMBER_TYPE)
    {
      for (std::vector<iface::dom::Node*>::iterator i = e2->mNodes.begin();
           i != e2->mNodes.end(); i++)
        if (compareNumbers(e1->mNumber, stringToNumber(stringValueOf(*i))))
          return true;
      return false;
    }

    if (e2->mType == iface::xpath::XPathResult::NUMBER_TYPE)
    {
      for (std::vector<iface::dom::Node*>::iterator i = e1->mNodes.begin();
           i != e1->mNodes.end(); i++)
        if (compareNumbers(stringToNumber(stringValueOf(*i)), e2->mNumber))
          return true;
      return false;
    }

    if (e1->mType == iface::xpath::XPathResult::STRING_TYPE)
    {
      for (std::vector<iface::dom::Node*>::iterator i = e2->mNodes.begin();
           i != e2->mNodes.end(); i++)
        if (compareStrings(e1->mString, stringValueOf(*i)))
          return true;
      return false;
    }

    if (e2->mType == iface::xpath::XPathResult::STRING_TYPE)
    {
      for (std::vector<iface::dom::Node*>::iterator i = e1->mNodes.begin();
           i != e1->mNodes.end(); i++)
        if (compareStrings(stringValueOf(*i), e2->mString))
          return true;
      return false;
    }

    if (e1->mType == iface::xpath::XPathResult::BOOLEAN_TYPE)
    {
      bool val = !e1->mNodes.empty();
      return compareBoolean(val, e2->mBoolean);
    }

    if (e2->mType == iface::xpath::XPathResult::BOOLEAN_TYPE)
    {
      bool val = !e2->mNodes.empty();
      return compareBoolean(e1->mBoolean, val);
    }

    // Should be unreachable unless a new type is added.
    assert(0);
    return false;
  }

  virtual bool
  compareNotSet(CDA_XPathResult* e1, CDA_XPathResult* e2)
  {
    if (e1->mType == iface::xpath::XPathResult::BOOLEAN_TYPE)
    {
      e2->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
      return compareBoolean(e1->mBoolean, e2->mBoolean);
    }

    if (e2->mType == iface::xpath::XPathResult::BOOLEAN_TYPE)
    {
      e1->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
      return compareBoolean(e1->mBoolean, e2->mBoolean);
    }

    if (e1->mType == iface::xpath::XPathResult::NUMBER_TYPE)
    {
      e2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
      return compareNumbers(e1->mNumber, e2->mNumber);
    }

    if (e2->mType == iface::xpath::XPathResult::NUMBER_TYPE)
    {
      e1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
      return compareBoolean(e1->mNumber, e2->mNumber);
    }
    
    e1->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
    e2->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
    return compareStrings(e1->mString, e2->mString);
  }

  virtual bool compareNumbers(double aN1, double aN2) = 0;
  virtual bool
  compareStrings(const std::wstring& aStr1, const std::wstring& aStr2)
  {
    return compareNumbers(stringToNumber(aStr1), stringToNumber(aStr2));
  }

  virtual bool
  compareBoolean(bool aB1, bool aB2)
  {
    return compareNumbers(aB1 ? 1 : 0, aB2 ? 1 : 0);
  }

  virtual bool
  compareSets(std::set<std::wstring>& aS1, std::set<std::wstring>& aS2) = 0;

protected:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathOrderExpr
  : public CDA_XPathComparisonExpr
{
public:
  CDA_XPathOrderExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathComparisonExpr(e1, e2)
  {
  }
  
  bool
  compareNotSet(CDA_XPathResult* r1, CDA_XPathResult* r2)
  {
    r1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    return compareNumbers(r1->mNumber, r2->mNumber);
  }

  virtual bool
  compareSets(std::set<std::wstring>& aSet1, std::set<std::wstring>& aSet2)
  {
    std::set<double> set1, set2;
    for (std::set<std::wstring>::iterator i = aSet1.begin(); i != aSet1.end(); i++)
      set1.insert(stringToNumber(*i));
    for (std::set<std::wstring>::iterator i = aSet2.begin(); i != aSet2.end(); i++)
      set2.insert(stringToNumber(*i));
    return compareNumberSets(set1, set2);
  }

  virtual bool compareNumberSets(std::set<double>& aSet1, std::set<double>& aSet2) = 0;
};

class CDA_XPathEqualityExpr
  : public CDA_XPathComparisonExpr
{
public:
  CDA_XPathEqualityExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathComparisonExpr(e1, e2) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  bool compareSets(std::set<std::wstring>& aS1, std::set<std::wstring>& aS2)
  {
    // Any two strings can be equal, so do a set intersection...
    std::set<std::wstring> intersect;
    std::set_intersection(aS1.begin(), aS1.end(), aS2.begin(), aS2.end(),
                          std::inserter(intersect, intersect.end()));
    return !intersect.empty();
  }

  bool compareNumbers(double aNum1, double aNum2)
  {
    return aNum1 == aNum2;
  }

  bool compareStrings(const std::wstring& aStr1, const std::wstring& aStr2)
  {
    return aStr1 == aStr2;
  }
};

class CDA_XPathNotEqualExpr
  : public CDA_XPathComparisonExpr
{
public:
  CDA_XPathNotEqualExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathComparisonExpr(mExpr1, mExpr2) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  bool compareSets(std::set<std::wstring>& aS1, std::set<std::wstring>& aS2)
  {
    // True unless all strings are present in both...
    return aS1 != aS2;
  }

  bool compareNumbers(double aNum1, double aNum2)
  {
    return aNum1 != aNum2;
  }

  bool compareStrings(const std::wstring& aStr1, const std::wstring& aStr2)
  {
    return aStr1 != aStr2;
  }

};

class CDA_XPathLessThanExpr
  : public CDA_XPathOrderExpr
{
public:
  CDA_XPathLessThanExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathOrderExpr(e1, e2) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  bool compareNumberSets(std::set<double>& aS1, std::set<double>& aS2)
  {
    // We compare the lowest in aS1 with the highest in aS2.
    // Sets are guaranteed to have at least one member.
    return *aS1.begin() < *aS2.rbegin();
  }

  bool compareNumbers(double aNum1, double aNum2)
  {
    return aNum1 < aNum2;
  }
};

class CDA_XPathLessEqualExpr
  : public CDA_XPathOrderExpr
{
public:
  CDA_XPathLessEqualExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathOrderExpr(e1, e2) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  bool compareNumberSets(std::set<double>& aS1, std::set<double>& aS2)
  {
    // We compare the lowest in aS1 with the highest in aS2.
    // Sets are guaranteed to have at least one member.
    return *aS1.begin() <= *aS2.rbegin();
  }

  bool compareNumbers(double aNum1, double aNum2)
  {
    return aNum1 <= aNum2;
  }
};

class CDA_XPathGreaterThanExpr
  : public CDA_XPathOrderExpr
{
public:
  CDA_XPathGreaterThanExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathOrderExpr(e1, e2) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  bool compareNumberSets(std::set<double>& aS1, std::set<double>& aS2)
  {
    // We compare the highest in aS1 with the lowest in aS2.
    // Sets are guaranteed to have at least one member.
    return *aS1.rbegin() > *aS2.begin();
  }

  bool compareNumbers(double aNum1, double aNum2)
  {
    return aNum1 > aNum2;
  }
};

class CDA_XPathGreaterEqualExpr
  : public CDA_XPathOrderExpr
{
public:
  CDA_XPathGreaterEqualExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : CDA_XPathOrderExpr(e1, e2) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  bool compareNumberSets(std::set<double>& aS1, std::set<double>& aS2)
  {
    // We compare the highest in aS1 with the lowest in aS2.
    // Sets are guaranteed to have at least one member.
    return *aS1.rbegin() >= *aS2.begin();
  }

  bool compareNumbers(double aNum1, double aNum2)
  {
    return aNum1 >= aNum2;
  }
};

class CDA_XPathPlusExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathPlusExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2)
  {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult>
  eval(CDA_XPathContext& aCtx)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aCtx));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aCtx));
    r1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);

    r1->mNumber = r1->mNumber + r2->mNumber;
    r1->add_ref();
    return r1.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathMinusExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathMinusExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2) {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult>
  eval(CDA_XPathContext& aCtx)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aCtx));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aCtx));
    r1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);

    r1->mNumber = r1->mNumber - r2->mNumber;
    r1->add_ref();
    return r1.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathTimesExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathTimesExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2) {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult>
  eval(CDA_XPathContext& aCtx)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aCtx));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aCtx));
    r1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);

    r1->mNumber = r1->mNumber * r2->mNumber;
    r1->add_ref();
    return r1.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathDivExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathDivExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2) {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult>
  eval(CDA_XPathContext& aContext)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aContext));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aContext));
    r1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);

    r1->mNumber = r1->mNumber / r2->mNumber;
    r1->add_ref();
    return r1.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathModExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathModExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2)
    : mExpr1(e1), mExpr2(e2) {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult>
  eval(CDA_XPathContext& aContext)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aContext));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aContext));
    r1->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
    r2->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);

    // Truncating remainder (per the spec).
    r1->mNumber = r1->mNumber - floor(r1->mNumber / r2->mNumber) * r2->mNumber;
    r1->add_ref();
    return r1.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathUnaryMinusExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathUnaryMinusExpr(CDA_XPathExpr* e)
    : mExpr(e) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult>
  eval(CDA_XPathContext& aContext)
  {
    RETURN_INTO_OBJREF(r, CDA_XPathResult, mExpr->eval(aContext));
    r->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);

    r->mNumber = -r->mNumber;
    r->add_ref();
    return r.getPointer();
  }
  
private:
  ObjRef<CDA_XPathExpr> mExpr;
};

class CDA_XPathUnionExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathUnionExpr(CDA_XPathExpr* e, CDA_XPathExpr* e2)
    : mExpr1(e), mExpr2(e)
  {
  }

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    RETURN_INTO_OBJREF(r1, CDA_XPathResult, mExpr1->eval(aContext));
    RETURN_INTO_OBJREF(r2, CDA_XPathResult, mExpr2->eval(aContext));

    if (!(r1->mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
          r1->mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
          r1->mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
          r1->mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
          r1->mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
          r1->mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE))
      throw iface::xpath::XPathException();

    if (!(r2->mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
          r2->mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
          r2->mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
          r2->mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
          r2->mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
          r2->mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE))
      throw iface::xpath::XPathException();
    
    for (std::vector<iface::dom::Node*>::iterator i = r2->mNodes.begin();
         i != r2->mNodes.end(); i++)
    {
      r1->addNode(*i);
    }

    r1->add_ref();
    return r1.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr1, mExpr2;
};

class CDA_XPathUseLocationExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathUseLocationExpr(CDA_XPathPath* p) : mPath(p) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    // Make a set using the node...
    RETURN_INTO_OBJREF(r, CDA_XPathResult, new CDA_XPathResult());
    r->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;
    r->addNode(aContext.mNode);
    return mPath->eval(aContext, r);
  }

private:
  ObjRef<CDA_XPathPath> mPath;
};

class CDA_XPathApplyFilterExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathApplyFilterExpr(CDA_XPathExpr* e, CDA_XPathPath* p) : mExpr(e), mPath(p) {};

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    RETURN_INTO_OBJREF(r, CDA_XPathResult, mExpr->eval(aContext));
    return mPath->eval(aContext, r);
  }

private:
  ObjRef<CDA_XPathExpr> mExpr;
  ObjRef<CDA_XPathPath> mPath;
};

class CDA_XPathApplyPredicateExpr
  : public CDA_XPathExpr
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);
  CDA_XPathApplyPredicateExpr(CDA_XPathExpr* ex, CDA_XPathExpr* pred) :
    mExpr(ex), mPred(pred) {};

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    RETURN_INTO_OBJREF(r, CDA_XPathResult, mExpr->eval(aContext));
    uint16_t t = r->mType;

    r->coerceTo(iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE);

    RETURN_INTO_OBJREF(rn, CDA_XPathResult, new CDA_XPathResult());
    rn->mType = t;

    CDA_XPathContext newCtx(NULL, 0, r->mNodes.size());

    for (std::vector<iface::dom::Node*>::iterator i(r->mNodes.begin());
         i != r->mNodes.end(); i++)
    {
      newCtx.mNode = *i;
      newCtx.mContextPos++;
      RETURN_INTO_OBJREF(ri, CDA_XPathResult, mPred->eval(newCtx));
      ri->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
      if (ri->mBoolean)
        rn->addNode(*i);
    }
    
    rn->add_ref();
    return rn.getPointer();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr, mPred;
};

class CDA_XPathVariableReferenceExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathVariableReferenceExpr(const std::wstring& aNamespace, const std::wstring& aReference)
  {};
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    // XXX this is host language independent, but this is a host language independent
    //     API, so we need to provide a way to set variable values.
    throw new iface::xpath::XPathException();
  }

private:
  ObjRef<CDA_XPathExpr> mExpr;
  ObjRef<CDA_XPathPath> mPath;
};

class CDA_XPathLiteralExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathLiteralExpr(const std::wstring& aLit) : mLit(aLit) {};
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    CDA_XPathResult* r = new CDA_XPathResult();
    r->mType = iface::xpath::XPathResult::STRING_TYPE;
    r->mString = mLit;

    return r;
  }

private:
  std::wstring mLit;
};

class CDA_XPathNumberExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathNumberExpr(double aNum) : mNum(aNum) {};
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    CDA_XPathResult* r = new CDA_XPathResult();
    r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
    r->mNumber = mNum;

    return r;
  }

private:
  double mNum;
};

static void splitWST(const std::wstring& aStr, std::list<std::wstring>& aAddTo)
{
  std::wstring cur;

  for (std::wstring::const_iterator i(aStr.begin()); i != aStr.end(); i++)
  {
    wchar_t c = *i;
    bool wasWS = false;
    while (c == L' ' || c == L'\t' || c == L'\r' || c == L'\n')
    {
      wasWS = true;
      i++;
      c = *i;
    }
    if (wasWS && cur != L"")
    {
      aAddTo.push_back(cur);
      cur = L"";
    }

    cur += c;
  }

  if (cur != L"")
    aAddTo.push_back(cur);
}

static double inline myround(double x)
{
  return (static_cast<int>(x >= 0 ? (x + 0.5) : (x - 0.5)));
}

class CDA_XPathFunctionCallExpr
  : public CDA_XPathExpr
{
public:
  // Note: aArgs goes away after the call - must copy and add_ref()...
  CDA_XPathFunctionCallExpr(const std::wstring& aNSURI, const std::wstring& aName, const std::list<CDA_XPathExpr*>& aArgs)
    : mNSURI(aNSURI), mName(aName), mArgs(aArgs.begin(), aArgs.end()) {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathExpression);

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext)
  {
    std::list<CDA_XPathResult*> args;
    XPCOMContainerRAII<std::list<CDA_XPathResult*> > argsRAII(args);

    for (std::list<CDA_XPathExpr*>::iterator i = mArgs.begin();
         i != mArgs.end(); i++)
      args.push_back((*i)->eval(aContext));

    // Currently only the default functions (in the empty namespace) are implemented.
    if (mNSURI != L"")
      throw iface::xpath::XPathException();
    switch (mName[0])
    {
    case L'b':
      if (mName == L"boolean")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
        arg->add_ref();
        return arg;
      }
      break;

    case L'c':
      if (mName == L"ceiling")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
        arg->mNumber = ceil(arg->mNumber);
        arg->add_ref();
        return arg;
      }
      else if (mName == L"concat")
      {
        if (args.size() < 2)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        for (std::list<CDA_XPathResult*>::iterator i = ++args.begin(); i != args.end(); i++)
        {
          (*i)->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
          arg->mString += (*i)->mString;
        }
        arg->add_ref();
        return arg;
      }
      else if (mName == L"contains")
      {
        if (args.size() != 2)
          throw iface::xpath::XPathException();
        CDA_XPathResult* haystack(args.front()), * needle(*++args.begin());
        haystack->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        needle->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        bool v = wcsstr(needle->mString.c_str(), haystack->mString.c_str()) != NULL;
        needle->cleanup();
        needle->mBoolean = v;
        needle->mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        needle->add_ref();
        return needle;
      }
      else if (mName == L"count")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE);

        double sz = arg->mNodes.size();

        arg->cleanup();
        arg->mType = iface::xpath::XPathResult::NUMBER_TYPE; 
        arg->mNumber = sz;
        arg->add_ref();
        return arg;
      }
      break;

    case L'f':
      if (mName == L"false")
      {
        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        r->mBoolean = false;

        return r;
      }
      else if (mName == L"floor")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
        arg->mNumber = floor(arg->mNumber);
        arg->add_ref();
        return arg;
      }
      break;

    case L'i':
      if (mName == L"id")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();

        CDA_XPathResult* arg(args.front());
        std::list<std::wstring> ids;
        if (arg->mType == iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE ||
            arg->mType == iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE ||
            arg->mType == iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE ||
            arg->mType == iface::xpath::XPathResult::ORDERED_NODE_SNAPSHOT_TYPE ||
            arg->mType == iface::xpath::XPathResult::ANY_UNORDERED_NODE_TYPE ||
            arg->mType == iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE)
        {
          for (std::vector<iface::dom::Node*>::iterator i(arg->mNodes.begin());
               i != arg->mNodes.end(); i++)
            splitWST(stringValueOf(*i), ids);
        }
        else
        {
          arg->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
          splitWST(arg->mString, ids);
        }

        arg->cleanup();
        arg->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;

        RETURN_INTO_OBJREF(doc, iface::dom::Document, aContext.mNode->ownerDocument());
        for (std::list<std::wstring>::iterator i = ids.begin(); i != ids.end(); i++)
        {
          RETURN_INTO_OBJREF(el, iface::dom::Element, doc->getElementById((*i).c_str()));
          if (el != NULL)
            arg->addNode(el);
        }

        arg->add_ref();
        return arg;
      }
      break;

    case L'l':
      if (mName == L"lang")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();

        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::STRING_TYPE);

        // Find the context language...
        ObjRef<iface::dom::Node> n(aContext.mNode);
        std::wstring l;
        while (n)
        {
          DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
          if (el != NULL)
          {
            l = el->getAttributeNS(L"http://www.w3.org/XML/1998/namespace", L"lang");
            if (l != L"")
              break;
          }
          n = already_AddRefd<iface::dom::Node>(n->parentNode());
        }

        CDA_XPathResult* res = new CDA_XPathResult();
        res->mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        if (l == L"")
        {
          res->mBoolean = false;
          return res;
        }

        size_t len(arg->mString.length());
        if (l == arg->mString ||
            (l.substr(0, len) == arg->mString && l[len] == L'-'))
          res->mBoolean = true;
        else
          res->mBoolean = false;

        return res;
      }
      else if (mName == L"last")
      {
        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        r->mNumber = aContext.mContextSize;
        return r;
      }
      else if (mName == L"local-name")
      {
        ObjRef<iface::dom::Node> n;
        if (args.size() == 1)
        {
          CDA_XPathResult* r1 = args.front();
          if (r1->mNodes.empty())
          {
            CDA_XPathResult* r = new CDA_XPathResult();
            r->mType = iface::xpath::XPathResult::STRING_TYPE;
            r->mString = L"";
            return r;
          }
          n = r1->mNodes.front();
        }
        else if (args.size() > 1)
          throw iface::xpath::XPathException();
        else
          n = aContext.mNode;

        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        r->mString = n->localName();
        return r;
      }
      break;

    case L'n':
      if (mName == L"name")
      {
        ObjRef<iface::dom::Node> n;
        if (args.size() == 1)
        {
          CDA_XPathResult* r1 = args.front();
          if (r1->mNodes.empty())
          {
            CDA_XPathResult* r = new CDA_XPathResult();
            r->mType = iface::xpath::XPathResult::STRING_TYPE;
            r->mString = L"";
            return r;
          }
          n = r1->mNodes.front();
        }
        else if (args.size() > 1)
          throw iface::xpath::XPathException();
        else
          n = aContext.mNode;

        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        r->mString = n->nodeName();
        return r;
      }
      else if (mName == L"namespace-uri")
      {
        ObjRef<iface::dom::Node> n;
        if (args.size() == 1)
        {
          CDA_XPathResult* r1 = args.front();
          if (r1->mNodes.empty())
          {
            CDA_XPathResult* r = new CDA_XPathResult();
            r->mType = iface::xpath::XPathResult::STRING_TYPE;
            r->mString = L"";
            return r;
          }
          n = r1->mNodes.front();
        }
        else if (args.size() > 1)
          throw iface::xpath::XPathException();
        else
          n = aContext.mNode;

        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        r->mString = n->namespaceURI();
        return r;
      }
      else if (mName == L"normalize-space")
      {
        std::wstring str;
        if (args.size() == 1)
        {
          CDA_XPathResult* arg = args.front();
          arg->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
          str = arg->mString;
        }
        else if (args.size() == 0)
          str = stringValueOf(aContext.mNode);
        else
          throw iface::xpath::XPathException();

        bool keep = false;
        std::wstring normalised;
        for (std::wstring::iterator i = str.begin(); i != str.end(); i++)
        {
          wchar_t c = *i;
          if (c == L' ' || c == L'\t' || c == L'\n' || c == L'\r')
          {
            if (keep)
              normalised += L' ';
            keep = false;
          }
          else
          {
            normalised += c;
            keep = true;
          }
        }
        size_t len(normalised.length());
        if (len != 0 && normalised[len - 1] == ' ')
          normalised = normalised.substr(0, len - 1);

        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::STRING_TYPE;
        r->mString = normalised;
        return r;
      }
      else if (mName == L"not")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
        arg->mBoolean = !arg->mBoolean;
        arg->add_ref();
        return arg;
      }
      else if (mName == L"number")
      {
        ObjRef<CDA_XPathResult> r;

        if (args.size() == 1)
          r = args.front();
        else if (args.size() == 0)
        {
          r = already_AddRefd<CDA_XPathResult>(new CDA_XPathResult());
          r->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;
          r->addNode(aContext.mNode);
        }
        else
          throw iface::xpath::XPathException();
        
        r->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
        r->add_ref();
        return r.getPointer();
      }
      break;

    case L'p':
      if (mName == L"position")
      {
        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        r->mNumber = aContext.mContextPos;
        return r;
      }
      break;

    case L'r':
      if (mName == L"round")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg(args.front());
        arg->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
        arg->mNumber = myround(arg->mNumber);
        arg->add_ref();
        return arg;
      }
      break;

    case L's':
      if (mName == L"starts-with")
      {
        if (args.size() != 2)
          throw iface::xpath::XPathException();

        CDA_XPathResult* longs(args.front()), * shorts(*++args.begin());
        longs->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        shorts->coerceTo(iface::xpath::XPathResult::STRING_TYPE);

        bool startswith = wcsncmp(longs->mString.c_str(), shorts->mString.c_str(), shorts->mString.size());

        longs->cleanup();
        longs->mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        longs->mBoolean = startswith;
      }
      else if (mName == L"string")
      {
        ObjRef<CDA_XPathResult> r;

        if (args.size() == 1)
          r = args.front();
        else if (args.size() == 0)
        {
          r = already_AddRefd<CDA_XPathResult>(new CDA_XPathResult());
          r->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;
          r->addNode(aContext.mNode);
        }
        else
          throw iface::xpath::XPathException();
        
        r->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        r->add_ref();
        return r.getPointer();
      }
      else if (mName == L"string-length")
      {
        std::wstring str;
        if (args.size() == 1)
        {
          CDA_XPathResult* arg = args.front();
          arg->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
          str = arg->mString;
        }
        else if (args.size() == 0)
          str = stringValueOf(aContext.mNode);
        else
          throw iface::xpath::XPathException();

        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        r->mNumber = str.size();
        return r;
      }
      else if (mName == L"substring")
      {
        if (args.size() < 2 || args.size() > 3)
          throw iface::xpath::XPathException();

        std::list<CDA_XPathResult*>::iterator i = args.begin();
        CDA_XPathResult* strr = *i++;
        strr->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        const std::wstring& str = strr->mString;

        CDA_XPathResult* offsetr = *i++;
        offsetr->coerceTo(iface::xpath::XPathResult::NUMBER_TYPE);
        uint32_t offset = static_cast<uint32_t>(myround(offsetr->mNumber));
        
        uint32_t length;
        if (args.size() == 2)
          length = str.length();
        else
        {
          CDA_XPathResult* lengthr = *i++;
          length = static_cast<uint32_t>(myround(lengthr->mNumber));
        }

        try
        {
          std::wstring ss = str.substr(offset, length);
          strr->cleanup();
          strr->mString = ss;
        }
        catch (std::exception&)
        {
          throw iface::xpath::XPathException();
        }

        strr->add_ref();
        return strr;
      }
      else if (mName == L"substring-after")
      {
        if (args.size() != 2)
          throw iface::xpath::XPathException();
        CDA_XPathResult* haystack(args.front()), * needle(*++args.begin());
        needle->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        haystack->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        const wchar_t* sub(wcsstr(needle->mString.c_str(), haystack->mString.c_str()));
        if (sub == NULL)
        {
          haystack->cleanup();
          haystack->add_ref();
          return haystack;
        }

        sub += needle->mString.size();
        needle->mString = sub;
        needle->add_ref();
        return needle;
      }
      else if (mName == L"substring-before")
      {
        if (args.size() != 2)
          throw iface::xpath::XPathException();
        CDA_XPathResult* haystack(args.front()), * needle(*++args.begin());
        needle->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        haystack->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        const wchar_t* sub(wcsstr(needle->mString.c_str(), haystack->mString.c_str()));
        if (sub == NULL)
        {
          haystack->cleanup();
          haystack->add_ref();
          return haystack;
        }

        needle->mString = haystack->mString.substr(0, sub - haystack->mString.c_str());
        needle->add_ref();
        return needle;
      }
      else if (mName == L"sum")
      {
        if (args.size() != 1)
          throw iface::xpath::XPathException();
        CDA_XPathResult* arg = args.front();
        arg->coerceTo(iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE);
        double sum = 0.0;
        for (std::vector<iface::dom::Node*>::iterator i = arg->mNodes.begin();
             i != arg->mNodes.end(); i++)
        {
          sum += stringToNumber(stringValueOf(*i));
        }

        arg->cleanup();
        arg->mType = iface::xpath::XPathResult::NUMBER_TYPE;
        arg->mNumber = sum;
        arg->add_ref();
        return arg;
      }
      break;

    case L't':
      if (mName == L"translate")
      {
        if (args.size() != 3)
          throw iface::xpath::XPathException();

        std::list<CDA_XPathResult*>::iterator i = args.begin();
        CDA_XPathResult* strr = *i++;
        strr->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        const std::wstring& str = strr->mString;

        CDA_XPathResult* oldr = *i++;
        oldr->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        const std::wstring& old = oldr->mString;

        CDA_XPathResult* newr = *i++;
        newr->coerceTo(iface::xpath::XPathResult::STRING_TYPE);
        const std::wstring& news = newr->mString;

        std::map<wchar_t, wchar_t> translate;
        for (std::wstring::const_iterator oi = old.begin(), ni = news.begin(); oi != old.end(); oi++)
        {
          if (ni != news.end())
          {
            translate.insert(std::pair<wchar_t, wchar_t>(*oi, *ni));
            ni++;
          }
          else
            translate.insert(std::pair<wchar_t, wchar_t>(*oi, 0));
        }

        std::wstring output;
        for (std::wstring::const_iterator si = str.begin(); si != str.end(); si++)
        {
          std::map<wchar_t, wchar_t>::iterator ti(translate.find(*si));
          if (ti == translate.end())
            output += *si;
          else if ((*ti).second == 0)
            ;
          else
            output += (*ti).second;
        }

        strr->cleanup();
        strr->mString = output;
      }
      else if (mName == L"true")
      {
        CDA_XPathResult* r = new CDA_XPathResult();
        r->mType = iface::xpath::XPathResult::BOOLEAN_TYPE;
        r->mBoolean = true;

        return r;
      }
      break;

    default:
      ;
    }
    
    throw iface::xpath::XPathException();
  }

private:
  double mNum;
  std::wstring mNSURI, mName;
  std::list<CDA_XPathExpr*> mArgs;
};

typedef enum {
  CDA_XPathAxisAncestor,
  CDA_XPathAxisAncestorOrSelf,
  CDA_XPathAxisAttribute,
  CDA_XPathAxisChild,
  CDA_XPathAxisDescendant,
  CDA_XPathAxisDescendantOrSelf,
  CDA_XPathAxisFollowing,
  CDA_XPathAxisFollowingSibling,
  CDA_XPathAxisNamespace,
  CDA_XPathAxisParent,
  CDA_XPathAxisPreceding,
  CDA_XPathAxisPrecedingSibling,
  CDA_XPathAxisSelf
} CDA_XPathAxis;

class CDA_XPathRoot
  : public CDA_XPathPath
{
public:
  // Indicates aPath is absolute (relative to root rather than context).
  CDA_XPathRoot(CDA_XPathPath* aPath) : mPath(aPath)  {};
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI0;

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext, CDA_XPathResult* aInput)
  {
    RETURN_INTO_OBJREF(doc, iface::dom::Document, aContext.mNode->ownerDocument());
    aInput->cleanup();
    aInput->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;
    aInput->addNode(doc);

    if (mPath == NULL)
    {
      aInput->add_ref();
      return aInput;
    }
    else
      return mPath->eval(aContext, aInput);
  }

private:
  ObjRef<CDA_XPathPath> mPath;
};

class CDA_XPathApplyPredicatePath
  : public CDA_XPathPath
{
public:
  CDA_XPathApplyPredicatePath(CDA_XPathPath* aPath, CDA_XPathExpr* aExpr, CDA_XPathAxis aAxis)
    : mPath(aPath), mExpr(aExpr)
  {
    if (aAxis == CDA_XPathAxisAncestor || aAxis == CDA_XPathAxisAncestorOrSelf ||
        aAxis == CDA_XPathAxisPreceding || aAxis == CDA_XPathAxisPrecedingSibling)
      mReverse = true;
    else
      mReverse = false;
  }

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI0;

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext, CDA_XPathResult* aInput)
  {
    CDA_XPathContext ctx(NULL, mReverse ? aInput->mNodes.size() + 1 : 0,
                         aInput->mNodes.size());
    RETURN_INTO_OBJREF(output, CDA_XPathResult, new CDA_XPathResult());
    output->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;

    for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin();
         i != aInput->mNodes.end(); i++)
    {
      if (mReverse)
        ctx.mContextPos--;
      else
        ctx.mContextPos++;

      ctx.mNode = *i;
      RETURN_INTO_OBJREF(er, CDA_XPathResult, mExpr->eval(ctx));
      er->coerceTo(iface::xpath::XPathResult::BOOLEAN_TYPE);
      if (er->mBoolean)
        output->addNode(*i);
    }

    if (mExpr == NULL)
      output->add_ref();

    if (mPath == NULL)
    {
      output->add_ref();
      return output.getPointer();
    }
    else
      return mPath->eval(aContext, output);
  }

private:
  ObjRef<CDA_XPathPath> mPath;
  ObjRef<CDA_XPathExpr> mExpr;
  bool mReverse;
};

class CDA_XPathApplyStepPath
  : public CDA_XPathPath
{
public:
  CDA_XPathApplyStepPath(CDA_XPathPath* aPath, CDA_XPathAxis aAxis, CDA_XPathNodeTest* aTest)
    : mPath(aPath), mAxis(aAxis), mTest(aTest) {};

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;

  already_AddRefd<CDA_XPathResult> eval(CDA_XPathContext& aContext, CDA_XPathResult* aInput)
  {
    std::set<iface::dom::Node*> seen;
    RETURN_INTO_OBJREF(r, CDA_XPathResult, new CDA_XPathResult());
    r->mType = iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE;

    switch (mAxis)
    {
    case CDA_XPathAxisSelf:
      for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin(); i != aInput->mNodes.end(); i++)
        if (performTestOn(aContext, *i))
        {
          r->addNode(*i);
        }
      break;

    case CDA_XPathAxisAncestor:
    case CDA_XPathAxisAncestorOrSelf:
      for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin(); i != aInput->mNodes.end(); i++)
      {
        std::list<iface::dom::Node*> toDo;

        // Go to the root or the topmost not yet seen...
        ObjRef<iface::dom::Node> n(*i), np;
        while (n)
        {
          if (seen.count(n) != 0)
            break;
          seen.insert(n);

          toDo.push_back(n);
          n = already_AddRefd<iface::dom::Node>(n->parentNode());
        }
        if (toDo.empty())
          continue;
        if (mAxis == CDA_XPathAxisAncestor)
          toDo.pop_front();

        for (std::list<iface::dom::Node*>::reverse_iterator tdi = toDo.rbegin();
             tdi != toDo.rend(); tdi++)
          if (performTestOn(aContext, *tdi))
            r->addNode(*tdi);
      }
      break;

    case CDA_XPathAxisAttribute:
      for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin(); i != aInput->mNodes.end(); i++)
      {
        RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, (*i)->attributes());
        uint32_t l = nnm->length();
        for (uint32_t i = 0; i < l; i++)
        {
          RETURN_INTO_OBJREF(item, iface::dom::Node, nnm->item(i));
          if (performTestOn(aContext, item))
            r->addNode(item);
        }
      }
      break;

    case CDA_XPathAxisNamespace:
      for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin(); i != aInput->mNodes.end(); i++)
      {
        std::map<std::wstring, iface::dom::Node*> currentNodes;
        XPCOMContainerSecondRAII<std::map<std::wstring, iface::dom::Node*> > currentNodesRAII(currentNodes);
        for (ObjRef<iface::dom::Node> n = *i; n;
             n = already_AddRefd<iface::dom::Node>(n->parentNode()))
        {
          RETURN_INTO_OBJREF(nnm, iface::dom::NamedNodeMap, (*i)->attributes());
          uint32_t l = nnm->length();
          for (uint32_t i = 0; i < l; i++)
          {
            RETURN_INTO_OBJREF(item, iface::dom::Node, nnm->item(i));
            RETURN_INTO_WSTRING(ln, item->localName());
            RETURN_INTO_WSTRING(nsURI, item->namespaceURI());

            if (!((nsURI == L"" && ln == L"xmlns") ||
                  nsURI == L"http://www.w3.org/2000/xmlns/"))
              continue;

            if (nsURI == L"")
              ln = L"";

            if (currentNodes.count(ln))
              continue;

            item->add_ref();
            currentNodes.insert(std::pair<std::wstring, iface::dom::Node*>(ln, item));

            if (performTestOn(aContext, item))
              r->addNode(item);
          }
        }
      }
      break;

    case CDA_XPathAxisChild:
      for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin(); i != aInput->mNodes.end(); i++)
      {
        RETURN_INTO_OBJREF(n, iface::dom::Node, (*i)->firstChild());
        for (; n; n = already_AddRefd<iface::dom::Node>(n->nextSibling()))
        {
          if (performTestOn(aContext, n))
            r->addNode(n);
        }
      }
      break;

    case CDA_XPathAxisDescendant:
    case CDA_XPathAxisDescendantOrSelf:
      {
        std::list<iface::dom::Node*> stack;
        XPCOMContainerRAII<std::list<iface::dom::Node*> > stackRAII(stack);
        for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin(); i != aInput->mNodes.end(); i++)
        {
          if (mAxis == CDA_XPathAxisDescendantOrSelf)
          {
            (*i)->add_ref();
            stack.push_back(*i);
          }
          else
          {
            iface::dom::Node* n = (*i)->lastChild();
            while (n)
            {
              stack.push_back(n);
              n = n->previousSibling();
            }
          }

          while (!stack.empty())
          {
            RETURN_INTO_OBJREF(n, iface::dom::Node, stack.back());
            stack.pop_back();
            
            if (seen.count(n) != 0)
              continue;
            seen.insert(n);
            
            if (performTestOn(aContext, n))
              r->addNode(n);
            
            iface::dom::Node* nc = n->lastChild();
            while (nc)
            {
              stack.push_back(nc);
              nc = nc->previousSibling();
            }
          }
        }
      }
      break;

    case CDA_XPathAxisFollowing:
      if (!aInput->mNodes.empty())
      {
        std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin();
        iface::dom::Node* everythingAfter = *i;

        for (i++; i != aInput->mNodes.end(); i++)
        {
          iface::dom::Node* cand = *i;
          bool found = false;
          ObjRef<iface::dom::Node> ca(cand);
          for (; ca; ca = already_AddRefd<iface::dom::Node>(ca->parentNode()))
          {
            if (!CDA_objcmp(ca, everythingAfter))
            {
              found = true;
              everythingAfter = cand;
              break;
            }
          }
          if (!found)
            break;
        }

        ObjRef<iface::dom::Node> n = everythingAfter;
        while (n)
        {
          RETURN_INTO_OBJREF(n2, iface::dom::Node, n->nextSibling());
          if (n2 != NULL)
            n = n2;
          else
          {
            n = already_AddRefd<iface::dom::Node>(n->parentNode());
            continue;
          }

          std::list<iface::dom::Node*> stack;
          XPCOMContainerRAII<std::list<iface::dom::Node*> > stackRAII(stack);
          n->add_ref();
          stack.push_back(n);
          while (!stack.empty())
          {
            ObjRef<iface::dom::Node> n(already_AddRefd<iface::dom::Node>(stack.back()));
            stack.pop_back();
            
            if (seen.count(n) != 0)
              continue;
            seen.insert(n);
            
            if (performTestOn(aContext, n))
              r->addNode(n);
            
            iface::dom::Node* nc = n->lastChild();
            while (nc)
            {
              stack.push_back(nc);
              nc = nc->previousSibling();
            }
          }
        }
      }
      break;
      
    case CDA_XPathAxisFollowingSibling:
      {
        std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin();
        std::list<iface::dom::Node*> stack;
        XPCOMContainerRAII<std::list<iface::dom::Node*> > stackRAII(stack);

        while (true)
        {
          if (stack.empty())
          {
            if (i == aInput->mNodes.end())
              break;
            else
            {
              (*i)->add_ref();
              stack.push_back(*i);
              i++;
            }
          }
          ObjRef<iface::dom::Node> b(already_AddRefd<iface::dom::Node>(stack.back()));
          stack.pop_back();

          if (i != aInput->mNodes.end())
          {
            for (ObjRef<iface::dom::Node> n(*i); n;
                 n = already_AddRefd<iface::dom::Node>(n->parentNode()))
              if (!CDA_objcmp(n, b))
              {
                (*i)->add_ref();
                stack.push_back(*i);
                i++;
                continue;
              }
          }

          b = already_AddRefd<iface::dom::Node>(b->nextSibling());
          if (b != NULL)
          {
            if (seen.count(b) == 0)
            {
              seen.insert(b);

              if (performTestOn(aContext, b))
                r->addNode(b);

              b->add_ref();
              stack.push_back(b);
            }
          }
        }
      }
      break;

    case CDA_XPathAxisParent:
      for (std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.begin();
           i != aInput->mNodes.end(); i++)
      {
        RETURN_INTO_OBJREF(n, iface::dom::Node, (*i)->parentNode());
        if (seen.count(n) != 0)
          continue;
        seen.insert(n);

        for (ObjRef<iface::dom::Node> np(n->parentNode()); np;
             np = already_AddRefd<iface::dom::Node>(np->parentNode()))
        {
          if (seen.count(np) != 0)
            break;
          seen.insert(np);

          for (std::vector<iface::dom::Node*>::iterator j = i;
               j != aInput->mNodes.end(); j++)
          {
            RETURN_INTO_OBJREF(jp, iface::dom::Node, (*j)->parentNode());
            if (!CDA_objcmp(jp, np))
              r->addNode(np);
          }
        }
        r->addNode(n);
      }
      break;

    case CDA_XPathAxisPreceding:
      if (!aInput->mNodes.empty())
      {
        ObjRef<iface::dom::Node> n = *--aInput->mNodes.end();
        while (n)
        {
          RETURN_INTO_OBJREF(n2, iface::dom::Node, n->previousSibling());
          if (n2 != NULL)
            n = n2;
          else
          {
            n = already_AddRefd<iface::dom::Node>(n->parentNode());
            continue;
          }

          std::list<iface::dom::Node*> stack;
          XPCOMContainerRAII<std::list<iface::dom::Node*> > stackRAII(stack);
          n->add_ref();
          stack.push_back(n);
          while (!stack.empty())
          {
            ObjRef<iface::dom::Node> n(already_AddRefd<iface::dom::Node>(stack.back()));
            stack.pop_back();
            
            if (seen.count(n) != 0)
              continue;
            seen.insert(n);
            
            if (performTestOn(aContext, n))
            {
              n->add_ref();
              r->mNodes.insert(r->mNodes.begin(), n);
            }
            
            iface::dom::Node* nc = n->firstChild();
            while (nc)
            {
              stack.push_back(nc);
              nc = nc->nextSibling();
            }
          }
        }
      }
      break;

    case CDA_XPathAxisPrecedingSibling:
      {
        std::vector<iface::dom::Node*>::iterator i = aInput->mNodes.end();
        std::list<iface::dom::Node*> stack;
        XPCOMContainerRAII<std::list<iface::dom::Node*> > stackRAII(stack);

        while (true)
        {
          if (stack.empty())
          {
            if (i == aInput->mNodes.begin())
              break;
            else
            {
              i--;
              (*i)->add_ref();
              stack.push_back(*i);
            }
          }
          ObjRef<iface::dom::Node> b(already_AddRefd<iface::dom::Node>(stack.back()));
          stack.pop_back();

          if (i != aInput->mNodes.begin())
          {
            for (ObjRef<iface::dom::Node> n(*(i - 1)); n;
                 n = already_AddRefd<iface::dom::Node>(n->parentNode()))
              if (!CDA_objcmp(n, b))
              {
                i--;
                (*i)->add_ref();
                stack.push_back(*i);
                continue;
              }
          }

          b = already_AddRefd<iface::dom::Node>(b->previousSibling());
          if (b != NULL)
          {
            if (seen.count(b) == 0)
            {
              seen.insert(b);
              if (performTestOn(aContext, b))
              {
                b->add_ref();
                r->mNodes.insert(r->mNodes.begin(), b);
              }
              b->add_ref();
              stack.push_back(b);
            }
          }
        }
      }
      break;
    }

    if (mPath == NULL)
    {
      r->add_ref();
      return r.getPointer();
    }
    else
      return mPath->eval(aContext, r);
  }

private:
  ObjRef<CDA_XPathPath> mPath;
  CDA_XPathAxis mAxis;
  ObjRef<CDA_XPathNodeTest> mTest;

  bool performTestOn(CDA_XPathContext& aContext, iface::dom::Node* aNode)
  {
    return mTest->eval(aContext, aNode);
  }
};

class CDA_XPathNameTest
  : public CDA_XPathNodeTest
{
public:
  CDA_XPathNameTest(const std::wstring& aNamespaceMatch, const std::wstring& aLocalnameMatch)
    : mNamespaceMatch(aNamespaceMatch), mLocalnameMatch(aLocalnameMatch) {};
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;

  virtual bool eval(CDA_XPathContext& aCtx, iface::dom::Node* aNode)
  {
    RETURN_INTO_WSTRING(ln, aNode->localName());
    RETURN_INTO_WSTRING(ns, aNode->namespaceURI());
    return ((mNamespaceMatch == L"*" || ns == mNamespaceMatch) &&
            (mLocalnameMatch == L"*" || ln == mLocalnameMatch));
  }

private:
  std::wstring mNamespaceMatch, mLocalnameMatch;
};

typedef enum {
  CDA_XPathNodeComment,
  CDA_XPathNodeText,
  CDA_XPathNodeProcessingInstruction,
  CDA_XPathNodeNode
} CDA_XPathNodeType;

class CDA_XPathNodeTypeTest
  : public CDA_XPathNodeTest
{
public:
  CDA_XPathNodeTypeTest(CDA_XPathNodeType aNodeType)
    : mNodeType(aNodeType) {}
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;

  virtual bool eval(CDA_XPathContext& aCtx, iface::dom::Node* aNode)
  {
    switch (mNodeType)
    {
    case CDA_XPathNodeComment:
      return (aNode->nodeType() == iface::dom::Node::COMMENT_NODE);
    case CDA_XPathNodeText:
      return (aNode->nodeType() == iface::dom::Node::TEXT_NODE ||
              aNode->nodeType() == iface::dom::Node::CDATA_SECTION_NODE);
    case CDA_XPathNodeProcessingInstruction:
      return (aNode->nodeType() == iface::dom::Node::PROCESSING_INSTRUCTION_NODE);
    case CDA_XPathNodeNode:
      return true;
    }
    return false;
  }

private:
  CDA_XPathNodeType mNodeType;
};

class CDA_XPathProcessingInstructionTest
  : public CDA_XPathNodeTest
{
public:
  CDA_XPathProcessingInstructionTest(const std::wstring& aLiteralMatch)
    : mLiteralMatch(aLiteralMatch) {}
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;

  virtual bool eval(CDA_XPathContext& aCtx, iface::dom::Node* aNode)
  {
    if (aNode->nodeType() != iface::dom::Node::PROCESSING_INSTRUCTION_NODE)
      return false;

    DECLARE_QUERY_INTERFACE_OBJREF(pi, aNode, dom::ProcessingInstruction);
    RETURN_INTO_WSTRING(t, pi->target());
    return (t == mLiteralMatch);
  }

private:
  std::wstring mLiteralMatch;
};

static const char* const kEqualToken = "=";
static const char* const kNotEqualToken = "!=";
static const char* const kLessThanToken = "<";
static const char* const kLessEqualToken = "<=";
static const char* const kGreaterThanToken = ">";
static const char* const kGreaterEqualToken = ">=";
static const char* const kPlusToken = "+";
static const char* const kMinusToken = "-";
static const char* const kMultiplyToken = "*";
static const char* const kDivToken = "div";
static const char* const kModToken = "mod";

// This is just a convenience class, it gets broken down into applications of NodePaths
// and Predicates.
class CDA_XPathStep 
  : public iface::XPCOM::IObject
{
public:
  CDA_XPathStep(CDA_XPathAxis aAxis, CDA_XPathNodeTest* aTest)
    : mAxis(aAxis), mTest(aTest), mPredicatesRAII(mPredicates) {}

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI0;

  CDA_XPathAxis mAxis;
  ObjRef<CDA_XPathNodeTest> mTest;
  std::list<CDA_XPathExpr*> mPredicates;
  XPCOMContainerRAII<std::list<CDA_XPathExpr*> > mPredicatesRAII;

  already_AddRefd<CDA_XPathPath> makePath(CDA_XPathPath* aInner)
  {
    ObjRef<CDA_XPathPath> inner(aInner);
    std::list<CDA_XPathExpr*>::iterator ei = mPredicates.end();
    while (true)
    {
      if (ei == mPredicates.begin())
        break;
      ei--;
      inner = already_AddRefd<CDA_XPathPath>(new CDA_XPathApplyPredicatePath(inner, *ei, mAxis));
    }
    return new CDA_XPathApplyStepPath(inner, mAxis, mTest);
  }
};

class CDA_XPathParserContext
{
public:
  CDA_XPathParserContext(const std::wstring& aParseStr,
                         iface::xpath::XPathNSResolver* aResolver) :
    mParseStr(aParseStr), mResolver(aResolver)
  {
    mParseIt = mParseStr.begin();
  }
  ~CDA_XPathParserContext() {}

  // We use recursive-descent parsing since using tools like Bison is difficult for
  // potentially UCS4 encoded data.
  already_AddRefd<CDA_XPathExpr> parse()
  {
    std::list<CDA_XPathExpr*> mExprs;
    XPCOMContainerRAII<std::list<CDA_XPathExpr*> > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parseAndExpr();
      if (ex == NULL)
        return NULL;
      if (!expectName(L"or"))
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back());
          mExprs.pop_back();
          back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathOrExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      else
        mExprs.push_back(ex);
      consumeName(); // or
    }
  }

private:
  std::wstring mParseStr;
  std::wstring::const_iterator mParseIt;
  ObjRef<iface::xpath::XPathNSResolver> mResolver;

  already_AddRefd<CDA_XPathExpr>
  parseAndExpr()
  {
    std::list<CDA_XPathExpr*> mExprs;
    XPCOMContainerRAII<std::list<CDA_XPathExpr*> > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parseEqualityExpr();
      if (ex == NULL)
        return NULL;
      if (!expectName(L"and"))
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back());
          mExprs.pop_back();
          back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathAndExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      else
        mExprs.push_back(ex);
      consumeName(); // and
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parseEqualityExpr()
  {
    std::list<std::pair<CDA_XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII<std::list<std::pair<CDA_XPathExpr*, const char*> > > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parseRelationalExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"=")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kEqualToken));
      else if (n == L"!=")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kNotEqualToken));
      else
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back().first);
          const char* token = mExprs.back().second;
          mExprs.pop_back();
          if (token == kEqualToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathEqualityExpr(n, back));
          else
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathNotEqualExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      consumeName(); // either = or !=
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parseRelationalExpr()
  {
    std::list<std::pair<CDA_XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII<std::list<std::pair<CDA_XPathExpr*, const char*> > > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parseAdditiveExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"<")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kLessThanToken));
      else if (n == L">")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kGreaterThanToken));
      else if (n == L"<=")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kLessEqualToken));
      else if (n == L">=")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kGreaterEqualToken));
      else
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back().first);
          const char* token = mExprs.back().second;
          mExprs.pop_back();
          if (token == kLessThanToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathLessThanExpr(n, back));
          else if (token == kGreaterThanToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathGreaterThanExpr(n, back));
          else if (token == kLessEqualToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathLessEqualExpr(n, back));
          else
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathGreaterEqualExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      consumeName(); // <, >, <=, or >=
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parseAdditiveExpr()
  {
    std::list<std::pair<CDA_XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII<std::list<std::pair<CDA_XPathExpr*, const char*> > > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parseMultiplicativeExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"+")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kPlusToken));
      else if (n == L"-")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kMinusToken));
      else
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back().first);
          const char* token = mExprs.back().second;
          mExprs.pop_back();
          if (token == kPlusToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathPlusExpr(n, back));
          else
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathMinusExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      consumeName(); // + or -
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parseMultiplicativeExpr()
  {
    std::list<std::pair<CDA_XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII<std::list<std::pair<CDA_XPathExpr*, const char*> > > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parseUnaryExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"*")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kMultiplyToken));
      else if (n == L"div")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kDivToken));
      else if (n == L"mod")
        mExprs.push_back(std::pair<CDA_XPathExpr*, const char*>(ex, kModToken));
      else
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back().first);
          const char* token = mExprs.back().second;
          mExprs.pop_back();
          if (token == kMultiplyToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathTimesExpr(n, back));
          else if (token == kDivToken)
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathDivExpr(n, back));
          else
            back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathModExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      consumeName(); // *, div, or mod
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parseUnaryExpr()
  {
    bool unaryMinus = false;
    while (expectName(L"-"))
    {
      unaryMinus = !unaryMinus;
      consumeName();
    }

    if (!unaryMinus)
      return parseUnionExpr();
    else
    {
      RETURN_INTO_OBJREF(ex, CDA_XPathExpr, parseUnionExpr());
      if (ex == NULL)
        return NULL;
      return new CDA_XPathUnaryMinusExpr(ex);
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parseUnionExpr()
  {
    std::list<CDA_XPathExpr*> mExprs;
    XPCOMContainerRAII<std::list<CDA_XPathExpr*> > xcrpath(mExprs);

    while (true)
    {
      CDA_XPathExpr* ex = parsePathExpr();
      if (ex == NULL)
        return NULL;
      if (!expectName(L"|"))
      {
        RETURN_INTO_OBJREF(back, CDA_XPathExpr, ex);
        while (!mExprs.empty())
        {
          RETURN_INTO_OBJREF(n, CDA_XPathExpr, mExprs.back());
          mExprs.pop_back();
          back = already_AddRefd<CDA_XPathExpr>(new CDA_XPathUnionExpr(n, back));
        }
        back->add_ref();
        return back.getPointer();
      }
      else
        mExprs.push_back(ex);
      consumeName(); // |
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parsePathExpr()
  {
    {
      std::wstring::const_iterator tParseIt = mParseIt;
      RETURN_INTO_OBJREF(loc, CDA_XPathPath, parseLocationPath());
      if (loc != NULL)
        return new CDA_XPathUseLocationExpr(loc);

      // parseLocationPath can consume tokens and fail, so back that out...
      mParseIt = tParseIt;
    }

    RETURN_INTO_OBJREF(fex, CDA_XPathExpr, parseFilterExpr());
    if (fex == NULL)
      return NULL;
    RETURN_INTO_WSTRING(n, peekName());
    bool doubleSlash = false;
    if (n == L"/")
      ;
    else if (n == L"//")
      doubleSlash = true;
    else
    {
      fex->add_ref();
      return fex.getPointer();
    }
    consumeName();

    RETURN_INTO_OBJREF(rlp, CDA_XPathPath, parseRelativeLocationPath());
    if (rlp == NULL)
      return NULL;

    if (!doubleSlash)
      return new CDA_XPathApplyFilterExpr(fex, rlp);

    RETURN_INTO_OBJREF(ntnn, CDA_XPathNodeTest, new CDA_XPathNodeTypeTest(CDA_XPathNodeNode));
    RETURN_INTO_OBJREF(asp, CDA_XPathPath,
                       new CDA_XPathApplyStepPath(rlp, CDA_XPathAxisDescendantOrSelf, ntnn));
    return new CDA_XPathApplyFilterExpr(fex, asp);
  }

  already_AddRefd<CDA_XPathExpr>
  parseFilterExpr()
  {
    RETURN_INTO_OBJREF(pe, CDA_XPathExpr, parsePrimaryExpr());
    if (pe == NULL)
      return NULL;

    while (true)
    {
      if (!expectName(L"["))
      {
        pe->add_ref();
        return pe.getPointer();
      }
      consumeName();

      RETURN_INTO_OBJREF(pred, CDA_XPathExpr, parse());
      if (pred == NULL)
        return NULL;
      if (!expectName(L"]"))
        return NULL;
      consumeName();

      pe = already_AddRefd<CDA_XPathExpr>(new CDA_XPathApplyPredicateExpr(pe, pred));
    }
  }

  already_AddRefd<CDA_XPathExpr>
  parsePrimaryExpr()
  {
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L"(")
    {
      consumeName();
      RETURN_INTO_OBJREF(expr, CDA_XPathExpr, parse());
      if (expr == NULL || !expectName(L")"))
        return NULL;
      expr->add_ref();
      return expr.getPointer();
    }
    else  if (n == L"$")
    {
      consumeName();
      std::wstring nsURI;
      bool success = false;
      RETURN_INTO_WSTRING(localName, parseQName(nsURI, success));
      if (!success)
        return NULL;
      return new CDA_XPathVariableReferenceExpr(nsURI, localName);
    }

    {
      bool success = false;
      std::wstring lit(parseLiteral(success));
      if (success)
        return new CDA_XPathLiteralExpr(lit);
    }

    {
      bool success = false;
      double num = parseNumber(success);
      if (success)
        return new CDA_XPathNumberExpr(num);
    }

    std::wstring nsURI;
    bool success = false;
    std::wstring funcName = parseFunctionName(nsURI, success);
    if (!success)
      return NULL;

    if (!expectName(L"("))
      return NULL;

    std::list<CDA_XPathExpr*> exprs;
    XPCOMContainerRAII<std::list<CDA_XPathExpr*> > xcrpath(exprs);
    CDA_XPathExpr* ex = parse();
    if (ex != NULL)
    {
      exprs.push_back(ex);
      
      while (true)
      {
        if (!expectName(L","))
          break;

        ex = parse();
        if (ex == NULL)
          return NULL;
        exprs.push_back(ex);
      }
    }

    if (!expectName(L")"))
      return NULL;

    return new CDA_XPathFunctionCallExpr(nsURI, funcName, exprs);
  }

  already_AddRefd<CDA_XPathPath>
  parseLocationPath()
  {
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L"//")
    {
      consumeName();

      RETURN_INTO_OBJREF(rlp, CDA_XPathPath, parseRelativeLocationPath());
      if (rlp == NULL)
        return NULL;
      RETURN_INTO_OBJREF(ntnn, CDA_XPathNodeTest,
                         new CDA_XPathNodeTypeTest(CDA_XPathNodeNode));
      RETURN_INTO_OBJREF(asp, CDA_XPathPath,
                         new CDA_XPathApplyStepPath
                         (rlp, CDA_XPathAxisDescendantOrSelf, ntnn));
      return new CDA_XPathRoot(asp);
    }
    
    if (n == L"/")
    {
      consumeName();
      RETURN_INTO_OBJREF(rlp, CDA_XPathPath, parseRelativeLocationPath());
      return new CDA_XPathRoot(rlp);
    }

    return parseRelativeLocationPath();
  }

  already_AddRefd<CDA_XPathPath>
  parseRelativeLocationPath()
  {
    std::list<CDA_XPathStep*> steps;
    XPCOMContainerRAII<std::list<CDA_XPathStep*> > stepraii(steps);

    while (true)
    {
      {
        CDA_XPathStep* s = parseStep();
        if (s == NULL)
          return NULL;
        steps.push_back(s);
      }

      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"//")
      {
        consumeName();
        RETURN_INTO_OBJREF(ntnn, CDA_XPathNodeTest, new CDA_XPathNodeTypeTest(CDA_XPathNodeNode));
        steps.push_back(new CDA_XPathStep(CDA_XPathAxisDescendantOrSelf, ntnn));
      }
      else if (n == L"/")
      {
        consumeName();
      }
      else
      {
        ObjRef<CDA_XPathPath> innerPath;

        while (!steps.empty())
        {
          RETURN_INTO_OBJREF(st, CDA_XPathStep, steps.back());
          steps.pop_back();
          innerPath = already_AddRefd<CDA_XPathPath>(st->makePath(innerPath));
        }

        if (innerPath)
          innerPath->add_ref();
        return innerPath.getPointer();
      }
    }
  }

  bool parseAxisSpecifier(CDA_XPathAxis& aAxis)
  {
    RETURN_INTO_WSTRING(n, peekName());
    consumeName();
    if (n == L"@")
    {
      aAxis = CDA_XPathAxisAttribute;
      return true;
    }

    RETURN_INTO_WSTRING(sep, peekName());
    if (sep != L"::")
    {
      unconsume(n);
      return false;
    }
    consumeName();

    switch (n[0])
    {
    case L'a':
      if (n == L"ancestor")
      {
        aAxis = CDA_XPathAxisAncestor;
        return true;
      }
      if (n == L"ancestor-or-self")
      {
        aAxis = CDA_XPathAxisAncestorOrSelf;
        return true;
      }
      if (n == L"attribute")
      {
        aAxis = CDA_XPathAxisAttribute;
        return true;
      }
      unconsume(sep);
      unconsume(n);
      return false;
    case L'c':
      if (n == L"child")
      {
        aAxis = CDA_XPathAxisChild;
        return true;
      }
      unconsume(sep);
      unconsume(n);
      return false;
    case L'd':
      if (n == L"descendant")
      {
        aAxis = CDA_XPathAxisDescendant;
        return true;
      }
      if (n == L"descendant-or-self")
      {
        aAxis = CDA_XPathAxisDescendantOrSelf;
        return true;
      }
      unconsume(sep);
      unconsume(n);
      return false;
    case L'f':
      if (n == L"following")
      {
        aAxis = CDA_XPathAxisFollowing;
        return true;
      }
      if (n == L"following-sibling")
      {
        aAxis = CDA_XPathAxisFollowingSibling;
        return true;
      }
      unconsume(sep);
      unconsume(n);
      return false;
    case L'n':
      if (n == L"namespace")
      {
        aAxis = CDA_XPathAxisNamespace;
        return true;
      }
      unconsume(sep);
      unconsume(n);
      return false;
    case L'p':
      if (n == L"parent")
      {
        aAxis = CDA_XPathAxisParent;
        return true;
      }
      if (n == L"preceding")
      {
        aAxis = CDA_XPathAxisPreceding;
        return true;
      }
      if (n == L"preceding-sibling")
      {
        aAxis = CDA_XPathAxisPrecedingSibling;
        return true;
      }
      unconsume(sep);
      unconsume(n);
      return false;
    case 's':
      if (n == L"self")
      {
        aAxis = CDA_XPathAxisSelf;
        return true;
      }
    default:
      unconsume(sep);
      unconsume(n);
      return false;
    }
  }

  already_AddRefd<CDA_XPathStep>
  parseStep()
  {
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L".")
    {
      // Abbreviated Step .
      RETURN_INTO_OBJREF(ntnn, CDA_XPathNodeTest, new CDA_XPathNodeTypeTest(CDA_XPathNodeNode));
      return new CDA_XPathStep(CDA_XPathAxisSelf, ntnn);
    }
    else if (n == L"..")
    {
      // Abbreviated Step ..
      RETURN_INTO_OBJREF(ntnn, CDA_XPathNodeTest, new CDA_XPathNodeTypeTest(CDA_XPathNodeNode));
      return new CDA_XPathStep(CDA_XPathAxisParent, ntnn);
    }
    else
    {
      CDA_XPathAxis axis = CDA_XPathAxisChild;
      // We ignore failure and let XPathAxisChild prevail if there is no match.
      parseAxisSpecifier(axis);
      RETURN_INTO_OBJREF(nt, CDA_XPathNodeTest, parseNodeTest());
      if (nt == NULL)
        return NULL;
      RETURN_INTO_OBJREF(step, CDA_XPathStep, new CDA_XPathStep(axis, nt));
      
      while (true)
      {
        if (!expectName(L"["))
        {
          step->add_ref();
          return step.getPointer();
        }

        RETURN_INTO_OBJREF(expr, CDA_XPathExpr, parse());
        if (expr == NULL)
          return NULL;

        if (!expectName(L"]"))
          return NULL;
        
        expr->add_ref();
        step->mPredicates.push_back(expr);
      }
    }
  }

  already_AddRefd<CDA_XPathNodeTest>
  parseNodeTest()
  {
    // NameTest, NodeType, or processing-instruction(literal)
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L"processing-instruction")
    {
      consumeName();
      if (!expectName(L"("))
        return NULL;
      bool success = false;
      std::wstring l = parseLiteral(success);
      CDA_XPathNodeTest* ret;
      if (!success)
        ret = new CDA_XPathNodeTypeTest(CDA_XPathNodeProcessingInstruction);
      else
        ret = new CDA_XPathProcessingInstructionTest(l);

      if (!expectName(L")"))
        return NULL;

      return ret;
    }
    
    CDA_XPathNodeType nt = CDA_XPathNodeNode;
    if (parseNodeType(nt))
    {
      if (!expectName(L"("))
        return NULL;
      if (!expectName(L")"))
        return NULL;
      return new CDA_XPathNodeTypeTest(nt);
    }

    return parseNameTest();
  }

  bool
  parseNodeType(CDA_XPathNodeType& aNT)
  {
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L"comment")
    {
      aNT = CDA_XPathNodeComment;
      consumeName();
      return true;
    }
    if (n == L"text")
    {
      aNT = CDA_XPathNodeText;
      consumeName();
      return true;
    }
    if (n == L"processing-instruction")
    {
      aNT = CDA_XPathNodeProcessingInstruction;
      consumeName();
      return true;
    }
    if (n == L"node")
    {
      aNT = CDA_XPathNodeNode;
      consumeName();
      return true;
    }

    return false;
  }

  already_AddRefd<CDA_XPathNameTest>
  parseNameTest()
  {
    bool success = false;
    std::wstring ns = parseNCName(success);
    if (!success)
    {
      if (!expectName(L"*"))
        return NULL;
      return new CDA_XPathNameTest(L"*", L"*");
    }

    if (!expectName(L":"))
    {
      RETURN_INTO_WSTRING(pn, peekName());
      if (pn == L"(")
        return NULL;
      return new CDA_XPathNameTest(L"*", ns);
    }

    ns = mResolver->lookupNamespaceURI(ns);

    success = false;
    std::wstring p = parseNCName(success);
    if (!success)
    {
      if (!expectName(L"*"))
        return NULL;

      RETURN_INTO_WSTRING(pn, peekName());
      if (pn == L"(")
        return NULL;

      return new CDA_XPathNameTest(ns, L"*");
    }

    CDA_XPathNameTest* nt = new CDA_XPathNameTest(ns, p);

    RETURN_INTO_WSTRING(pn, peekName());
    if (pn == L"(")
      return NULL;

    return nt;
  }

  struct GroupInfo { char c, g, ng; bool singleIfFirst; };

  uint32_t
  nameLength()
  {
    static const GroupInfo kGroup[] =
     {
       {'/', '/', '/', false},
       {'"', '"', 0, false},
       {'\'', '\'', 0, false},
       {'=', '=', 0, false},
       {'!', '!', '=', false},
       {'<', '<', '=', false},
       {'>', '<', '=', false},
       {'+', '+', 0, false},
       {'-', 'A', 'A', true},
       {'*', '*', 0, false},
       {'|', '|', 0, false},
       {'[', '[', 0, false},
       {']', ']', 0, false},
       {'(', '(', 0, false},
       {')', ')', 0, false},
       {'$', '$', 0, false},
       {'\"', '\"', 0, false},
       {'@', '@', 0, false},
       {':', ':', ':', false},
       {'.', '.', '.', false},
       {' ', ' ', ' ', false},
       {'\t', ' ', ' ', false},
       {'\r', ' ', ' ', false},
       {'\n', ' ', ' ', false}
     };
    bool isFirst = true;
    static const GroupInfo defgroup = {'A', 'A', 'A', false};

    const GroupInfo* lastGroup = NULL, * thisGroup;
    std::wstring::const_iterator tParseIt = mParseIt;
    while (true)
    {
      thisGroup = &defgroup;
      if (tParseIt == mParseStr.end())
        break;
      for (size_t i = 0; i < sizeof(kGroup) / sizeof(GroupInfo); i++)
      {
        if (kGroup[i].c == *tParseIt)
        {
          thisGroup = &kGroup[i];
          break;
        }
      }
      if (lastGroup != NULL)
      {
        if (lastGroup->singleIfFirst && isFirst)
          break;
        isFirst = false;
        if (lastGroup->ng != thisGroup->g)
          break;
      }
      tParseIt++;
      lastGroup = thisGroup;
    }

    return tParseIt - mParseIt;
  }

  std::wstring
  peekName()
  {
    consumeWhitespace();

    std::wstring ret(mParseIt, mParseIt + nameLength());
    return ret;
  }

  void
  consumeName()
  {
    mParseIt += nameLength();
  }

  bool
  expectName(const std::wstring& aName)
  {
    consumeWhitespace();

    size_t l = nameLength();
    if (l == aName.length() &&
        std::wstring(mParseIt, mParseIt + l) == aName)
    {
      mParseIt += l;
      return true;
    }
    return false;
  }

  std::wstring
  parseNCName(bool& aSuccess)
  {
    consumeWhitespace();

    std::wstring::const_iterator tParseIt = mParseIt;
    if (tParseIt == mParseStr.end())
    {
      aSuccess = false;
      return L"";
    }

    // NameStartChar - ':'
    wchar_t c = *tParseIt;
    if (!((c >= L'A' && c <= L'Z') || (c == L'_') || (c >= L'a' && c <= L'z') ||
          (c >= 0xC0 && c <= 0xD6) || (c >= 0xD8 && c <= 0xF6) ||
          (c >= 0xF8 && c <= 0x2FF) || (c >= 0x370 && c <= 0x37D) ||
          (c >= 0x37F && c <= 0x1FFF) || (c >= 0x200C && c <= 0x200D) ||
          (c >= 0x2070 && c <= 0x218F) || (c >= 0x2C00 && c <= 0x2FEF) ||
          (c >= 0x3001 && c <= 0xD7FF) || (c >= 0xF900 && c <= 0xFDCF) ||
          (c >= 0xFDF0 && c != 0xFFFE && c != 0xFFFF)))
    {
      aSuccess = false;
      return L"";
    }
    tParseIt++;

    while (true)
    {
      if (tParseIt == mParseStr.end())
      {
        std::wstring s(mParseIt, tParseIt);
        mParseIt = tParseIt;
        aSuccess = true;
        return s;
      }
      wchar_t c = *tParseIt;
      if (!((c >= L'A' && c <= L'Z') || (c == L'_') || (c == L'-') || (c == L'.') ||
            (c >= '0' && c <= '9') || (c >= L'a' && c <= L'z') || (c == 0xB7) ||
            (c >= 0xC0 && c <= 0xD6) || (c >= 0xD8 && c <= 0xF6) ||
            (c >= 0xF8 && c <= 0x37D) ||
            (c >= 0x37F && c <= 0x1FFF) || (c >= 0x200C && c <= 0x200D) ||
            (c >= 0x203F && c <= 0x2040) ||
            (c >= 0x2070 && c <= 0x218F) || (c >= 0x2C00 && c <= 0x2FEF) ||
            (c >= 0x3001 && c <= 0xD7FF) || (c >= 0xF900 && c <= 0xFDCF) ||
            (c >= 0xFDF0 && c != 0xFFFE && c != 0xFFFF)))
      {
        std::wstring s(mParseIt, tParseIt);
        mParseIt = tParseIt;
        aSuccess = true;
        return s;
      }
      tParseIt++;
    }
  }

  void
  unconsume(const std::wstring& aStr)
  {
    mParseIt -= aStr.length();
  }

  std::wstring
  parseQName(std::wstring& aNSURI, bool& aSuccess)
  {
    std::wstring fst = parseNCName(aSuccess);
    if (!aSuccess)
      return fst;

    consumeWhitespace();

    if (mParseIt == mParseStr.end() || *mParseIt != L':')
    {
      aNSURI = L"";
      return fst;
    }

    consumeWhitespace();

    mParseIt++;

    std::wstring snd = parseNCName(aSuccess);
    if (!aSuccess)
    {
      mParseIt--;
      aNSURI = L"";
      return fst;
    }

    aNSURI = mResolver->lookupNamespaceURI(fst);
    return snd;
  }

  std::wstring
  parseLiteral(bool& aSuccess)
  {
    consumeWhitespace();

    if (mParseIt == mParseStr.end())
    {
      aSuccess = false;
      return L"";
    }

    wchar_t e = *mParseIt;
    if (e != L'\'' && e != L'"')
    {
      aSuccess = false;
      return L"";
    }

    std::wstring::const_iterator tParseIt = mParseIt;
    tParseIt++;

    while (tParseIt != mParseStr.end())
    {
      if (*tParseIt == e)
      {
        std::wstring res(mParseIt + 1, tParseIt);
        mParseIt = tParseIt;
        mParseIt++;

        aSuccess = true;
        return res;
      }

      tParseIt++;
    }

    // Unterminated string...
    aSuccess = false;
    return L"";
  }

  double parseNumber(bool& aSuccess)
  {
    consumeWhitespace();

    aSuccess = false;
    bool postDP = false;
    double value = 0.0, mup = 1.0;
    std::wstring::const_iterator tParseIt = mParseIt;

    while (tParseIt != mParseStr.end())
    {
      wchar_t c = *tParseIt;
      if (postDP && c == L'.')
        break;
      if (c == L'.')
      {
        postDP = true;
        tParseIt++;
        continue;
      }
      if (c < L'0' || c > L'9')
        break;
      int v = c - '0';
      if (!postDP)
        value *= 10;
      else
        mup /= 10.0;

      value += v * mup;
      tParseIt++;
    }

    if (tParseIt != mParseIt)
    {
      aSuccess = true;
      mParseIt = tParseIt;
    }

    return value;
  }

  std::wstring
  parseFunctionName(std::wstring& aNSURI, bool& aSuccess)
  {
    std::wstring::const_iterator tParseIt;
    CDA_XPathNodeType nt;
    bool v = parseNodeType(nt);
    if (v)
    {
      mParseIt = tParseIt;
      aSuccess = false;
      return L"";
    }
    
    return parseQName(aNSURI, aSuccess);
  }

  void consumeWhitespace()
  {
    while (mParseIt != mParseStr.end())
    {
      wchar_t c = *mParseIt;
      if (c == L' ' || c == L'\n' || c == L'\t' || c == L'\r')
        mParseIt++;
      else
        break;
    }
  }
};

class CDA_XPathNSResolver
  : public iface::xpath::XPathNSResolver
{
public:
  CDA_XPathNSResolver(iface::dom::Node* aNode)
    : mNode(aNode)
  {
  }

  std::wstring
  lookupNamespaceURI(const std::wstring& aPrefix)
    throw()
  {
    bool isEmpty = aPrefix == L"";

    RETURN_INTO_OBJREF(n, iface::dom::Node, mNode->parentNode());
    for (; n; n = already_AddRefd<iface::dom::Node>(n->parentNode()))
    {
      DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
      if (el == NULL)
        break;

      RETURN_INTO_WSTRING(ns, el->getAttributeNS(isEmpty ? L"" : L"http://www.w3.org/2000/xmlns/",
                                                isEmpty ? L"xmlns" : aPrefix));
      if (ns != L"")
        return ns;
    }
    return L"";
  }

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathNSResolver);

private:
  ObjRef<iface::dom::Node> mNode;
};

already_AddRefd<iface::xpath::XPathExpression>
CDA_XPathEvaluator::createExpression
(const std::wstring& aExpression, iface::xpath::XPathNSResolver* aResolver)
  throw(std::exception&)
{
  CDA_XPathParserContext pc(aExpression, aResolver);
  return pc.parse();
}

already_AddRefd<iface::xpath::XPathNSResolver>
CDA_XPathEvaluator::createNSResolver
(
 iface::dom::Node* aNode
)
  throw()
{
  return new CDA_XPathNSResolver(aNode);
}

already_AddRefd<iface::xpath::XPathResult>
CDA_XPathEvaluator::evaluate(
 const std::wstring& aExpr, iface::dom::Node* aContext,
 iface::xpath::XPathNSResolver* aResolver,
 uint16_t aType, iface::xpath::XPathResult* aResult)
  throw(std::exception&)
{
  CDA_XPathParserContext pc(aExpr, aResolver);
  RETURN_INTO_OBJREF(cxe, CDA_XPathExpr, pc.parse());
  if (cxe == NULL)
    throw iface::xpath::XPathException();

  return cxe->evaluate(aContext, aType, aResult);
}
