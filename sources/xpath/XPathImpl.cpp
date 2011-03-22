#include "XPathImpl.hxx"

class CDA_XPathExpr
{
public:
  CDA_XPathExpr();
};

class CDA_XPathPath
{
public:
  CDA_XPathPath();
};
class CDA_XPathNodeTest
{
public:
  CDA_XPathNodeTest();
};


class CDA_XPathOrExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathOrExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathAndExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathAndExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathEqualityExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathEqualityExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathNotEqualExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathNotEqualExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathLessThanExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathLessThanExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathLessEqualExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathLessEqualExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathGreaterThanExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathGreaterThanExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathGreaterEqualExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathGreaterEqualExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathPlusExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathPlusExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathMinusExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathMinusExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathTimesExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathTimesExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathDivExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathDivExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathModExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathModExpr(CDA_XPathExpr* e1, CDA_XPathExpr* e2);
};
class CDA_XPathUnaryMinusExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathUnaryMinusExpr(CDA_XPathExpr* e);
};
class CDA_XPathUnionExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathUnionExpr(CDA_XPathExpr* e);
};
class CDA_XPathUseLocationExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathUseLocationExpr(CDA_XPathPath* e);
};
class CDA_XPathApplyFilterExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathApplyFilterExpr(CDA_XPathExpr* f, CDA_XPathPath* p);
};
class CDA_XPathApplyPredicateExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathApplyPredicateExpr(CDA_XPathExpr* ex, CDA_XPathExpr* pred);
};
class CDA_XPathVariableReferenceExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathVariableReferenceExpr(const std::wstring& aReference);
};
class CDA_XPathLiteralExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathLiteralExpr(const wchar_t* aLit);
};
class CDA_XPathNumberExpr
  : public CDA_XPathExpr
{
public:
  CDA_XPathNumberExpr(double aNum);
};
class CDA_XPathFunctionCallExpr
  : public CDA_XPathExpr
{
public:
  // Note: aArgs goes away after the call - must copy and add_ref()...
  CDA_XPathFunctionCallExpr(const std::wstring& aName, const std::list<CDA_XPathExpr*>& aArgs);
};
class CDA_XPathRoot
  : public CDA_XPathPath
{
public:
  // Indicates aPath is absolute (relative to root rather than context).
  CDA_XPathRoot(CDA_XPathPath* aPath);
};
class CDA_XPathApplyPredicatePath
  : public CDA_XPathPath
{
public:
  CDA_XPathApplyPredicatePath(CDA_XPathPath* path, CDA_XPathExpr* pred);
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

class CDA_XPathApplyStepPath
  : public CDA_XPathPath
{
public:
  CDA_XPathApplyStepPath(CDA_XPathPath* aPath, CDA_XPathAxis aAxis, CDA_XPathNodeTest* aTest);
};
class CDA_XPathNameTest
  : public CDA_XPathNodeTest
{
public:
  CDA_XPathNameTest(const std::wstring& aNamespaceMatch, const std::wstring& aLocalnameMatch);
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
  CDA_XPathNodeTypeTest(CDA_XPathNodeType aNodeType);
};
class CDA_XPathProccessingInstructionTest
  : public CDA_XPathNodeTest
{
public:
  CDA_XPathProcessingInstructionTest(const std::wstring& aLiteralMatch);
};


template
class XPCOMContainerRAII<typename C>
{
public:
  XPCOMContainerRAII(C& aCont)
    : mCont(aCont)
  {
  }

  ~XPCOMContainerRAII()
  {
    for (C::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i)->release_ref();
    mCont.clear();
  }

private:
  C& mCont;
};

template
class XPCOMContainerFirstRAII<typename C>
{
public:
  XPCOMContainerFirstRAII(C& aCont)
    : mCont(aCont)
  {
  }

  ~XPCOMContainerFirstRAII()
  {
    for (C::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i).first->release_ref();
    mCont.clear();
  }

private:
  C& mCont;
};

static const char const * kEqualToken = "=";
static const char const * kNotEqualToken = "!=";
static const char const * kLessThanToken = "<";
static const char const * kLessEqualToken = "<=";
static const char const * kGreaterThanToken = ">";
static const char const * kGreaterEqualToken = ">=";
static const char const * kPlusToken = "+";
static const char const * kMinusToken = "-";
static const char const * kMultiplyToken = "*";
static const char const * kDivToken = "div";
static const char const * kModToken = "mod";

class CDA_XPathParserContext
{
public:
  CDA_XPathParserContext(const std::wstring& aParseStr) : mParseStr(aParseStr), mParseIt(aParseStr.end()) {}
  ~CDA_XPathParserContext() {}

  // We use recursive-descent parsing since using tools like Bison is difficult for
  // potentially UCS4 encoded data.
  XPathExpr* parse()
  {
    std::list<XPathExpr*> mExprs;
    XPCOMContainerRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parseAndExpr();
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
        back.add_ref();
        return back;
      }
      else
        mExprs.push_back(ex);
      consumeName(); // or
    }
  }

private:
  const std::wstring& mParseStr;
  std::wstring::const_iterator mParseIt;

  XPathExpr*
  parseAndExpr()
  {
    std::list<XPathExpr*> mExprs;
    XPCOMContainerRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parseEqualityExpr();
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
        back.add_ref();
        return back;
      }
      else
        mExprs.push_back(ex);
      consumeName(); // and
    }
  }

  XPathExpr*
  parseEqualityExpr()
  {
    std::list<std::pair<XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parseRelationalExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"=")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kEqualToken));
      else if (n == L"!=")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kNotEqualToken));
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
        back.add_ref();
        return back;
      }
      consumeName(); // either = or !=
    }
  }

  XPathExpr*
  parseRelationalExpr()
  {
    std::list<std::pair<XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parseAdditiveExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"<")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kLessThanToken));
      else if (n == L">")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kGreaterThanToken));
      else if (n == L"<=")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kLessEqualToken));
      else if (n == L">=")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kGreaterEqualToken));
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
        back.add_ref();
        return back;
      }
      consumeName(); // <, >, <=, or >=
    }
  }

  XPathExpr*
  parseAdditiveExpr()
  {
    std::list<std::pair<XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parseMultiplicativeExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"+")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kPlusToken));
      else if (n == L"-")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kMinusToken));
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
        back.add_ref();
        return back;
      }
      consumeName(); // + or -
    }
  }

  XPathExpr*
  parseMultiplicativeExpr()
  {
    std::list<std::pair<XPathExpr*, const char*> > mExprs;
    XPCOMContainerFirstRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parseUnaryExpr();
      if (ex == NULL)
        return NULL;
      RETURN_INTO_WSTRING(n, peekName());
      if (n == L"*")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kMultiplyToken));
      else if (n == L"div")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kDivToken));
      else if (n == L"mod")
        mExprs.push_back(std::pair<XPathExpr*, const char*>(ex, kModToken));
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
        back.add_ref();
        return back;
      }
      consumeName(); // *, div, or mod
    }
  }

  XPathExpr*
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
      RETURN_INTO_OBJREF(ex, XPathExpr, parseUnionExpr());
      return new CDA_XPathUnaryMinusExpr(ex);
    }
  }

  XPathExpr*
  parseUnionExpr()
  {
    std::list<XPathExpr*> mExprs;
    XPCOMContainerRAII xcrpath(mExprs);

    while (true)
    {
      XPathExpr* ex = parsePathExpr();
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
        back.add_ref();
        return back;
      }
      else
        mExprs.push_back(ex);
      consumeName(); // |
    }
  }

  XPathExpr*
  parsePathExpr()
  {
    {
      RETURN_INTO_OBJREF(loc, XPathPath, parseLocationPath());
      if (loc != NULL)
        return new CDA_XPathUseLocationExpr(loc);
    }

    RETURN_INTO_OBJREF(fex, XPathExpr, parseFilterExpr());
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
      return fex;
    }
    consumeName();

    RETURN_INTO_OBJREF(rlp, CDA_XPathPath, parseRelativeLocationPath());
    if (rlp == NULL)
      return NULL;

    if (!doubleSlash)
      return new CDA_ApplyFilterExpr(fex, rlp);

    RETURN_INTO_OBJREF(ntnn, CDA_XPathNodeTest, new CDA_XPathNodeTypeTest(CDA_XPathNodeNode));
    RETURN_INTO_OBJREF(asp, CDA_XPathPath,
                       new CDA_XPathApplyStepPath(rlp, CDA_XPathAxisDescendantOrSelf, ntnn));
    return new CDA_ApplyFilterExpr(fex, asp);
  }

  XPathExpr*
  parseFilterExpr()
  {
    RETURN_INTO_OBJREF(pe, XPathExpr, parsePrimaryExpr());
    if (pe == NULL)
      return NULL;

    while (true)
    {
      if (!expectName("["))
      {
        pe->add_ref();
        return pe;
      }
      consumeName();

      RETURN_INTO_OBJREF(pred, XPathExpr, parse());
      if (!expectName("]"))
        return NULL;
      consumeName();

      pe = already_AddRefd<XPathExpr>(new CDA_XPathApplyPredicateExpr(pe, pred));
    }
  }

  XPathExpr*
  parsePrimaryExpr()
  {
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L"(")
    {
      consumeName();
      RETURN_INTO_OBJREF(expr, XPathExpr, parse());
      if (!expectName(L")"))
        return NULL;
      expr->add_ref();
      return expr;
    }
    else  if (n == L"$")
    {
      consumeName();
      RETURN_INTO_WSTRING(varName, parseQName());
      return new CDA_XPathVariableReferenceExpr(varName);
    }

    {
      wchar_t* lit = parseLiteral();
      if (lit != NULL)
        return CDA_XPathLiteralExpr(lit);
    }

    {
      bool success = false;
      double num = parseNumber(success);
      if (success)
        return CDA_XPathNumberExpr(num);
    }

    RETURN_INTO_WSTRING(funcName, parseFunctionName());
    if (!expectName(L"("))
      return NULL;
    consumeName();

    std::list<XPathExpr*> exprs;
    XPCOMContainerRAII xcrpath(mExprs);
    XPathExpr* ex = parse();
    if (ex != NULL)
    {
      exprs.push_back(ex);
      
      while (true)
      {
        if (!expectName(L","))
          break;
        consumeName();

        ex = parse();
        if (ex == NULL)
          return NULL;
        exprs.push_back(ex);
      }
    }

    if (!expectName(L")"))
      return NULL;
    consumeName();

    return new CDA_XPathFunctionCallExpr(funcName, exprs);
  }

  XPathPath*
  parseLocationPath()
  {
    RETURN_INTO_WSTRING(n, peekName());
    if (n == L"//")
    {
      consumeName();

      RETURN_INTO_OBJREF(rlp, CDA_XPathPath, parseRelativeLocationPath());
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

  XPathPath*
  parseRelativeLocationPath()
  {
    // Parse a 
  }
};

iface::xpath::XPathExpression*
CDA_XPathEvaluator::createExpression
(const wchar_t* aExpression, iface::xpath::XPathNSResolver* aResolver)
  throw(std::exception&)
{
  
}
