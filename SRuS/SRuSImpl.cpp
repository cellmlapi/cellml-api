#include "SRuSBootstrap.hpp"
#include "Utilities.hxx"
#include "IfaceCIS.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceSRuS.hxx"
#include "IfaceSProS.hxx"
#include "CellMLBootstrap.hpp"
#include "XPathBootstrap.hpp"
#include "CISBootstrap.hpp"
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <set>

class CDA_SRuSProcessor
  : public iface::SRuS::SEDMLProcessor
{
public:
  CDA_SRuSProcessor()
    : _cda_refcount(1) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::SEDMLProcessor);
  CDA_IMPL_REFCOUNT;

  bool supportsModellingLanguage(const wchar_t* aLang) throw();
  iface::SRuS::TransformedModel* buildOneModel(iface::SProS::Model* aModel)
    throw();
  iface::SRuS::TransformedModelSet* buildAllModels
    (iface::SProS::SEDMLElement* aElement) throw();
  void generateData(iface::SRuS::TransformedModelSet* aSet,
                    iface::SProS::SEDMLElement* aElement,
                    iface::SRuS::GeneratedDataMonitor* aMonitor) throw();

private:
  iface::cellml_api::CellMLElement* xmlToCellML(iface::cellml_api::Model* aModel, iface::dom::Node* aNode)
    throw();
};

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

template<typename C> class XPCOMContainerSecondRAII
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


// Find the CellMLElement corresponding to a given DOM node.
iface::cellml_api::CellMLElement*
CDA_SRuSProcessor::xmlToCellML(iface::cellml_api::Model* aModel, iface::dom::Node* aNode) throw()
{
  RETURN_INTO_OBJREF(doc, iface::dom::Document, aNode->ownerDocument());
  std::list<iface::dom::Node*> l;
  XPCOMContainerRAII<std::list<iface::dom::Node*> > lRAII(l);
  
  // Build a list giving the path through the document...
  ObjRef<iface::dom::Node> n(aNode);
  while (n)
  {
    n->add_ref();
    l.push_back(n);

    n = already_AddRefd<iface::dom::Node>(aNode->parentNode());
  }

  // Get rid of anything up to the document element...
  while (!l.empty())
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, l.front(), dom::Element);
    if (el != NULL)
      break;
    l.front()->release_ref();
    l.pop_front();
  }

  ObjRef<iface::cellml_api::CellMLElement> cur(aModel);
  while (!l.empty())
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, l.front(), dom::Element);
    // It's an attribute, text node, or processing instruction in an element...
    // just match to the element, the remaining specificity is unneeded.
    if (el == NULL)
      break;

    RETURN_INTO_OBJREF(ces, iface::cellml_api::CellMLElementSet, cur->childElements());
    RETURN_INTO_OBJREF(cei, iface::cellml_api::CellMLElementIterator, ces->iterate());
    bool found = false;
    while (true)
    {
      RETURN_INTO_OBJREF(ce, iface::cellml_api::CellMLElement, cei->next());
      if (ce == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(dce, ce, cellml_api::CellMLDOMElement);
      if (dce != NULL)
      {
        RETURN_INTO_OBJREF(del, iface::dom::Element, dce->domElement());
        if (!CDA_objcmp(del, el))
        {
          found = true;
          cur = ce;
          l.front()->release_ref();
          l.pop_front();
          break;
        }
      }
    }

    if (!found)
      return NULL;
  }

  cur->add_ref();
  return cur;
}

bool
CDA_SRuSProcessor::supportsModellingLanguage(const wchar_t* aLang)
  throw()
{
  if (!wcscmp(aLang, L"http://www.cellml.org/cellml/1.1#") ||
      !wcscmp(aLang, L"http://www.cellml.org/cellml/1.0#"))
    return true;

  // In future, make this extensible so we can support other modelling languages.
  return false;
}

class CDA_SRuSTransformedModel
  : public iface::SRuS::TransformedModel
{
public:
  CDA_SRuSTransformedModel(iface::dom::Document* aDoc)
    : _cda_refcount(1), mDocument(aDoc) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::TransformedModel);
  CDA_IMPL_REFCOUNT;

  iface::dom::Document* xmlDocument() throw()
  {
    mDocument->add_ref();
    return mDocument;
  }

  iface::XPCOM::IObject* modelDocument() throw(std::exception&)
  {
    ensureModelOrRaise();
    mModel->add_ref();
    return mModel;
  }

  iface::SProS::Model* sedmlModel() throw()
  {
    mSEDMLModel->add_ref();
    return mSEDMLModel;
  }

private:
  void ensureModelOrRaise()
  {
    if (mModel)
      return;

    RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader, cb->modelLoader());
    DECLARE_QUERY_INTERFACE_OBJREF(dml, ml, cellml_api::DOMModelLoader);
    try
    {
      mModel = already_AddRefd<iface::cellml_api::Model>(dml->createFromDOMDocument(mDocument));
    }
    catch (...)
    {
    }

    if (mModel == NULL)
      throw iface::SRuS::SRuSException();
  }

  ObjRef<iface::dom::Document> mDocument;
  ObjRef<iface::cellml_api::Model> mModel;
  ObjRef<iface::SProS::Model> mSEDMLModel;
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

static void processSep(iface::dom::Node* n, double& v1, double& v2)
{
  RETURN_INTO_OBJREF(cn, iface::dom::Node, n->firstChild());
  std::wstring value;
  bool seenSep = false;
  while (cn)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, cn, dom::Element);
    if (el != NULL)
    {
      RETURN_INTO_WSTRING(ln, el->localName());
      if (ln == L"sep")
      {
        RETURN_INTO_WSTRING(ns, el->localName());
        if (ns == L"http://www.w3.org/1998/Math/MathML")
        {
          v1 = stringToNumber(value);
          value = L"";
        }
      }
    }
    else
      value += stringValueOf(cn);

    cn = cn->nextSibling();
  }

  if (!seenSep)
    throw iface::SRuS::SRuSException();
  
  v2 = stringToNumber(value);
}

class NeedsAggregate
{
};

class Aggregator
{
public:
  virtual double operator()(const std::vector<double>& aV) = 0;
};

class MinAggregator
  : public Aggregator
{
public:
  double operator()(const std::vector<double>& aV)
  {
    return *std::min_element(aV.begin(), aV.end());
  }
};

class MaxAggregator
  : public Aggregator
{
public:
  double operator()(const std::vector<double>& aV)
  {
    return *std::max_element(aV.begin(), aV.end());
  }
};

class SumAggregator
  : public Aggregator
{
public:
  double operator()(const std::vector<double>& aV)
  {
    double s = 0.0;
    for (std::vector<double>::const_iterator i = aV.begin(); i != aV.end(); i++)
      s += *i;
    return s;
  }
};

class ProductAggregator
  : public Aggregator
{
public:
  double operator()(const std::vector<double>& aV)
  {
    double s = 1.0;
    for (std::vector<double>::const_iterator i = aV.begin(); i != aV.end(); i++)
      s *= *i;
    return s;
  }
};

// An evaluator for MathML expressions making use of the SED-ML subset.
class SEDMLMathEvaluator
{
public:
  SEDMLMathEvaluator() : mExploreEverything(false)
  {
  }

  void setExploreEverything(bool aExploreEverything)
  {
    mExploreEverything = aExploreEverything;
  }

  double eval(iface::mathml_dom::MathMLElement* aME)
  {
    DECLARE_QUERY_INTERFACE_OBJREF(mae, aME, mathml_dom::MathMLApplyElement);
    if (mae != NULL)
      return evalApply(mae);

    DECLARE_QUERY_INTERFACE_OBJREF(mcne, aME, mathml_dom::MathMLCnElement);
    if (mcne != NULL)
      return evalConstant(mcne);

    DECLARE_QUERY_INTERFACE_OBJREF(mcie, aME, mathml_dom::MathMLCiElement);
    if (mcie != NULL)
      return evalVariable(mcie);

    DECLARE_QUERY_INTERFACE_OBJREF(mpw, aME, mathml_dom::MathMLPiecewiseElement);
    if (mpw != NULL)
      return evalPiecewise(mpw);

    DECLARE_QUERY_INTERFACE_OBJREF(mpds, aME, mathml_dom::MathMLPredefinedSymbol);
    if (mpds != NULL)
      return evalPredefined(mpds);

    throw iface::SRuS::SRuSException();
  }

  double evalApply(iface::mathml_dom::MathMLApplyElement* mae)
  {
    RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement, mae->_cxx_operator());
    DECLARE_QUERY_INTERFACE_OBJREF(cse, op, mathml_dom::MathMLCsymbolElement);
    if (cse != NULL)
      return evalAggregate(cse, mae);

    std::vector<double> args;
    for (uint32_t i = 2, l = mae->nArguments(); i <= l; i++)
    {
      RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, mae->getArgument(i));
      args.push_back(eval(arg));
    }

    RETURN_INTO_WSTRING(opns, op->namespaceURI());
    if (opns != L"http://www.w3.org/1998/Math/MathML")
      throw iface::SRuS::SRuSException();

    RETURN_INTO_WSTRING(opln, op->localName());
    switch (opln[0])
    {
    case L'a':
      switch (opln[1])
      {
      case L'b':
        if (opln == L"abs")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return fabs(args[0]);
        }
        else
          throw iface::SRuS::SRuSException();

      case L'n':
        if (opln == L"and")
        {
          bool success = true;
          for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
            success &= ((*i) != 0.0);
          return success ? 1.0 : 0.0;
        }
        else
          throw iface::SRuS::SRuSException();

      case L'r':
        if (opln == L"arccos")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acos(args[0]);
        }
        else if (opln == L"arccosh")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acosh(args[0]);
        }
        else if (opln == L"arccot")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atan(1.0 / args[0]);
        }
        else if (opln == L"arccoth")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atanh(1.0 / args[0]);
        }
        else if (opln == L"arccsc")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asin(1.0 / args[0]);
        }
        else if (opln == L"arccsch")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asinh(1.0 / args[0]);
        }
        else if (opln == L"arcsec")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acos(1.0 / args[0]);
        }
        else if (opln == L"arcsech")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acosh(1.0 / args[0]);
        }
        else if (opln == L"arcsin")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asin(args[0]);
        }
        else if (opln == L"arcsinh")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asinh(args[0]);
        }
        else if (opln == L"arctan")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atan(args[0]);
        }
        else if (opln == L"arctanh")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atanh(args[0]);
        }
        else
          throw iface::SRuS::SRuSException();

      default:
        throw iface::SRuS::SRuSException();
      }

    case L'c':
      if (opln == L"ceiling")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return ceil(args[0]);
      }
      else if (opln == L"cos")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return cos(args[0]);
      }
      else if (opln == L"cosh")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return cosh(args[0]);
      }
      else if (opln == L"cot")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return 1.0 / tan(args[0]);
      }
      else if (opln == L"coth")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return 1.0 / tanh(args[0]);
      }
      else if (opln == L"csc")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return 1.0 / sin(args[0]);
      }
      else if (opln == L"csch")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return 1.0 / sinh(args[0]);
      }
      else
        throw iface::SRuS::SRuSException();

    case L'd':
      if (opln == L"divide")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return args[0] / args[1];
      }
      else
        throw iface::SRuS::SRuSException();

    case L'e':
      if (opln == L"eq")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return (args[0] == args[1]) ? 1.0 : 0.0;
      }
      else if (opln == L"exp")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return exp(args[0]);
      }
      else
        throw iface::SRuS::SRuSException();

    case L'f':
      if (opln == L"factorial")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        double v = 1.0;
        for (double m = args[0]; m > 0; m -= 1.0)
          v *= m;
        return v;
      }
      else if (opln == L"floor")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return floor(args[0]);
      }
      else
        throw iface::SRuS::SRuSException();

    case L'g':
      if (opln == L"geq")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return (args[0] >= args[1]) ? 1.0 : 0.0;
      }
      else if (opln == L"gt")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return (args[0] > args[1]) ? 1.0 : 0.0;
      }
      else
        throw iface::SRuS::SRuSException();

    case L'l':
      if (opln == L"leq")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return (args[0] <= args[1]) ? 1.0 : 0.0;
      }
      else if (opln == L"ln")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return log(args[0]);
      }
      else if (opln == L"log")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        RETURN_INTO_OBJREF(nl, iface::dom::NodeList, mae->childNodes());
        for (uint32_t i = 0, l = nl->length(); i < l; i++)
        {
          RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
          DECLARE_QUERY_INTERFACE_OBJREF(el, n, mathml_dom::MathMLContentContainer);
          if (el == NULL)
            continue;
          RETURN_INTO_WSTRING(ln, el->localName());
          if (ln == L"logbase")
          {
            if (el->nArguments() < 1)
              throw iface::SRuS::SRuSException();
            RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement, el->getArgument(1));
            double lb = eval(arg1);
            return log(args[0]) / log(lb);
          }
        }
        return log10(args[0]);
      }
      else if (opln == L"lt")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return (args[0] < args[1]) ? 1.0 : 0.0;
      }
      else
        throw iface::SRuS::SRuSException();

    case L'm':
      if (opln == L"minus")
      {
        if (args.size() == 1)
          return -args[0];
        else if (args.size() == 2)
          return args[0] - args[1];
        else
          throw iface::SRuS::SRuSException();
      }
      else
        throw iface::SRuS::SRuSException();

    case L'n':
      if (opln == L"neq")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return args[0] != args[1];
      }
      else if (opln == L"not")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return (args[0] == 0.0) ? 1.0 : 0.0;
      }
      else
        throw iface::SRuS::SRuSException();

    case L'o':
      if (opln == L"or")
      {
        bool any = false;
        for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
          any |= ((*i) != 0.0);
        return any ? 1.0 : 0.0;
      }
      else
        throw iface::SRuS::SRuSException();

    case L'p':
      if (opln == L"plus")
      {
        double sum = 0.0;
        for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
          sum += *i;
        return sum;
      }
      else if (opln == L"power")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return pow(args[0], args[1]);
      }
      else
        throw iface::SRuS::SRuSException();

    case L'r':
      if (opln == L"root")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        RETURN_INTO_OBJREF(nl, iface::dom::NodeList, mae->childNodes());
        for (uint32_t i = 0, l = nl->length(); i < l; i++)
        {
          RETURN_INTO_OBJREF(n, iface::dom::Node, nl->item(i));
          DECLARE_QUERY_INTERFACE_OBJREF(el, n, mathml_dom::MathMLContentContainer);
          if (el == NULL)
            continue;
          RETURN_INTO_WSTRING(ln, el->localName());
          if (ln == L"degree")
          {
            if (el->nArguments() < 1)
              throw iface::SRuS::SRuSException();
            RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement, el->getArgument(1));
            double deg = eval(arg1);
            return pow(args[0], 1.0 / deg);
          }
        }
        return sqrt(args[0]);        
      }
      else
        throw iface::SRuS::SRuSException();

    case L's':
      if (opln == L"sec")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return 1.0 / cos(args[0]);
      }
      else if (opln == L"sech")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return 1.0 / cosh(args[0]);
      }
      else if (opln == L"sin")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return sin(args[0]);
      }
      else if (opln == L"sinh")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return sinh(args[0]);
      }
      else
        throw iface::SRuS::SRuSException();

    case L't':
      if (opln == L"tan")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return tan(args[0]);
      }
      else if (opln == L"tanh")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException();
        return tanh(args[0]);
      }
      else if (opln == L"times")
      {
        double prod = 1.0;
        for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
          prod *= *i;
        return prod;
      }
      else
        throw iface::SRuS::SRuSException();

    case L'x':
      if (opln == L"xor")
      {
        if (args.size() != 2)
          throw iface::SRuS::SRuSException();
        return ((args[0] == 0) != (args[1] == 0)) ? 1.0 : 0.0;
      }
      else
        throw iface::SRuS::SRuSException();

    default:
      throw iface::SRuS::SRuSException();
    }
  }

  double evalConstant(iface::mathml_dom::MathMLCnElement* mcne)
  {
    RETURN_INTO_WSTRING(b, mcne->base());
    if (b != L"" && b != L"10")
      throw iface::SRuS::SRuSException(); // To do: bases other than 10.
    
    RETURN_INTO_WSTRING(t, mcne->type());
    if (t == L"integer" || t == L"real")
    {
      std::wstring n = stringValueOf(mcne);
      return stringToNumber(n);
    }
    else if (t == L"e-notation")
    {
      double s1, s2;
      processSep(mcne, s1, s2);
      return s1 * pow(10.0, s2);
    }
    else if (t == L"rational")
    {
      double s1, s2;
      processSep(mcne, s1, s2);
      return s1 / s2;
    }
    else
      throw iface::SRuS::SRuSException(); // complex or constant not supported (yet).
  }

  double evalVariable(iface::mathml_dom::MathMLCiElement* mcie)
  {
    std::wstring v(stringValueOf(mcie));
    std::map<std::wstring, double>::iterator i = mVariableValues.find(stringValueOf(mcie));
    if (i == mVariableValues.end())
      throw iface::SRuS::SRuSException(); // variable not found.
    
    return (*i).second;
  }

  double evalPiecewise(iface::mathml_dom::MathMLPiecewiseElement* mpe)
  {
    RETURN_INTO_OBJREF(mp, iface::mathml_dom::MathMLNodeList, mpe->pieces());
    bool gotResult = false;
    double result;
    for (uint32_t i = 0, l = mp->length(); i < l; i++)
    {
      RETURN_INTO_OBJREF(dc, iface::dom::Node, mp->item(i));
      DECLARE_QUERY_INTERFACE_OBJREF(mc, dc, mathml_dom::MathMLCaseElement);
      if (mc == NULL)
        continue;

      RETURN_INTO_OBJREF(cc, iface::mathml_dom::MathMLContentElement, mc->caseCondition());
      RETURN_INTO_OBJREF(cv, iface::mathml_dom::MathMLContentElement, mc->caseValue());
      if (eval(cc) != 0.0)
      {
        gotResult = true;
        result = eval(cv);
        if (!mExploreEverything)
          return result;
      }
      else if (mExploreEverything)
        eval(cv);
    }

    RETURN_INTO_OBJREF(ow, iface::mathml_dom::MathMLContentElement, mpe->otherwise());

    if (gotResult)
      return result;
    return eval(ow);
  }

  double evalPredefined(iface::mathml_dom::MathMLPredefinedSymbol* mpds)
  {
    RETURN_INTO_WSTRING(sn, mpds->symbolName());
    if (sn == L"true")
      return 1.0;
    else if (sn == L"false")
      return 0.0;
    else if (sn == L"notanumber")
      return std::numeric_limits<double>::quiet_NaN();
    else if (sn == L"pi")
      return 3.141592653589793;
    else if (sn == L"infinity")
      return std::numeric_limits<double>::infinity();
    else if (sn == L"exponentiale")
      return exp(1);

    throw iface::SRuS::SRuSException(); // no such predefined symbol
  }

  virtual double evalAggregate(iface::mathml_dom::MathMLCsymbolElement* aOp, iface::mathml_dom::MathMLApplyElement* aApply)
  {
    throw NeedsAggregate();
  }

  void setVariable(const std::wstring& aName, double aValue)
  {
    mVariableValues.insert(std::pair<std::wstring, double>(aName, aValue));
  }

protected:
  std::map<std::wstring, double> mVariableValues;
  bool mExploreEverything;
};

class SEDMLMathEvaluatorWithAggregate
  : public SEDMLMathEvaluator
{
public:
  SEDMLMathEvaluatorWithAggregate(const std::map<std::wstring, std::vector<double> >& aHistory)
    : mHistory(aHistory.begin(), aHistory.end())
  {
  }

  double evalAggregate(iface::mathml_dom::MathMLCsymbolElement* aOp,
                       iface::mathml_dom::MathMLApplyElement* aApply)
  {
    if (aApply->nArguments() != 2)
      throw iface::SRuS::SRuSException();
    RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, aApply->getArgument(2));

    RETURN_INTO_WSTRING(du, aOp->definitionURL());
    std::auto_ptr<Aggregator> ag;
    if (du == L"http: //sed -ml.org/#max")
      ag.reset(new MaxAggregator());
    else if (du == L"http: //sed -ml.org/#min")
      ag.reset(new MinAggregator());
    else if (du == L"http: //sed -ml.org/#sum")
      ag.reset(new SumAggregator());
    else if (du == L"http: //sed -ml.org/#product")
      ag.reset(new ProductAggregator());
    else
      throw iface::SRuS::SRuSException();

    std::map<std::wstring, double> vvback(mVariableValues.begin(), mVariableValues.end());
    if (mHistory.size() == 0)
      throw iface::SRuS::SRuSException(); // Should be impossible?
    uint32_t nhist = (*mHistory.begin()).second.size();

    std::vector<double> vals;
    for (uint32_t idx = 0; idx < nhist; idx++)
    {
      for (std::map<std::wstring, double>::iterator i = mVariableValues.begin();
           i != mVariableValues.end(); i++)
        (*i).second = mHistory[(*i).first][idx];
      vals.push_back(eval(arg));
    }
    mVariableValues.clear();
    mVariableValues.insert(vvback.begin(), vvback.end());

    return (*ag)(vals);
  }

private:
  std::map<std::wstring, std::vector<double> > mHistory;
};

iface::SRuS::TransformedModel*
CDA_SRuSProcessor::buildOneModel(iface::SProS::Model* aModel)
  throw()
{
  try
  {
    RETURN_INTO_WSTRING(lang, aModel->language());
    if (lang != L"urn:sedml:language:cellML" &&
        lang != L"urn:sedml:language:cellml.1_0" &&
        lang != L"urn:sedml:language:cellml.1_1")
    {
      // To do: Support other languages here...
      throw iface::SRuS::SRuSException();
    }

    // Get the URI...
    RETURN_INTO_WSTRING(uri, aModel->source());
    
    // Load it...
    RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    RETURN_INTO_OBJREF(ml, iface::cellml_api::DOMURLLoader, cb->localURLLoader());
    RETURN_INTO_OBJREF(doc, iface::dom::Document, ml->loadDocument(uri.c_str()));
    
    RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
    RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
    RETURN_INTO_OBJREF(resolver, iface::xpath::XPathNSResolver,
                       xe->createNSResolver(de));
    
    // Apply the transformations to the document...
    RETURN_INTO_OBJREF(cs, iface::SProS::ChangeSet, aModel->changes());
    RETURN_INTO_OBJREF(ci, iface::SProS::ChangeIterator, cs->iterateChanges());
    while (true)
    {
      RETURN_INTO_OBJREF(c, iface::SProS::Change, ci->nextChange());
      if (c == NULL)
        break;
      
      RETURN_INTO_WSTRING(targ, c->target());
      RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                         xe->evaluate(targ.c_str(), doc, resolver,
                                      iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE,
                                      NULL));
      
      RETURN_INTO_OBJREF(n, iface::dom::Node, xr->iterateNext());
      if (n == NULL)
        throw iface::SRuS::SRuSException();
      
      RETURN_INTO_OBJREF(n2, iface::dom::Node, xr->iterateNext());
      if (n2)
        throw iface::SRuS::SRuSException();

      DECLARE_QUERY_INTERFACE_OBJREF(ca, c, SProS::ChangeAttribute);
      if (ca != NULL)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(at, n, dom::Attr);
        if (at == NULL) // XPath target is not an attribute?
          throw iface::SRuS::SRuSException();
        RETURN_INTO_WSTRING(v, ca->newValue());
        at->value(v.c_str());
        continue;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(ax, c, SProS::AddXML);
      if (ax != NULL)
      {
        RETURN_INTO_OBJREF(nl, iface::dom::NodeList, ax->anyXML());
        RETURN_INTO_OBJREF(np, iface::dom::Node, n->parentNode());

        RETURN_INTO_OBJREF(axnl, iface::dom::NodeList, ax->anyXML());
        for (uint32_t i = 0, l = axnl->length(); i < l; i++)
        {
          RETURN_INTO_OBJREF(nn, iface::dom::Node, axnl->item(i));
          np->insertBefore(nn, n)->release_ref();
        }

        // Deliberate fall-through, ChangeXML is a specific type of AddXML.
      }

      DECLARE_QUERY_INTERFACE_OBJREF(cx, c, SProS::ChangeXML);
      DECLARE_QUERY_INTERFACE_OBJREF(rx, c, SProS::RemoveXML);
      if (cx != NULL || rx != NULL)
      {
        RETURN_INTO_OBJREF(np, iface::dom::Node, n->parentNode());
        RETURN_INTO_OBJREF(tmp, iface::dom::Node, np->removeChild(n));
        continue;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(cc, c, SProS::ComputeChange);
      if (cc != NULL)
      {
        // To do: implement ComputeChange.
        throw iface::SRuS::SRuSException();
      }

      if (ax != NULL)
        continue;
    }

    return new CDA_SRuSTransformedModel(doc);
  }
  catch (...)
  {
    throw iface::SRuS::SRuSException();
  }
}

class CDA_SRuSTransformedModelSet
  : public iface::SRuS::TransformedModelSet
{
public:
  CDA_SRuSTransformedModelSet()
    : _cda_refcount(1), mTransformedRAII(mTransformed) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::TransformedModelSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length() throw()
  {
    return mTransformed.size();
  }

  iface::SRuS::TransformedModel* item(uint32_t aIdx)
    throw(std::exception&)
  {
    if (aIdx >= length())
      throw iface::SRuS::SRuSException();

    iface::SRuS::TransformedModel* ret = mTransformed[aIdx];
    ret->add_ref();
    return ret;
  }

  iface::SRuS::TransformedModel* getItemByID(const wchar_t* aMatchID)
    throw(std::exception&)
  {
    for (std::vector<iface::SRuS::TransformedModel*>::iterator i = mTransformed.begin();
         i != mTransformed.end(); i++)
    {
      RETURN_INTO_OBJREF(sm, iface::SProS::Model, (*i)->sedmlModel());
      RETURN_INTO_WSTRING(idStr, sm->id());
      if (idStr == aMatchID)
      {
        (*i)->add_ref();
        return (*i);
      }
    }

    return NULL;
  }

  void addTransformed(iface::SRuS::TransformedModel* aTransformed)
  {
    aTransformed->add_ref();
    mTransformed.push_back(aTransformed);
  }

private:
  std::vector<iface::SRuS::TransformedModel*> mTransformed;
  XPCOMContainerRAII<std::vector<iface::SRuS::TransformedModel*> > mTransformedRAII;
};

iface::SRuS::TransformedModelSet*
CDA_SRuSProcessor::buildAllModels(iface::SProS::SEDMLElement* aElement)
  throw()
{
  RETURN_INTO_OBJREF(tms, CDA_SRuSTransformedModelSet, new CDA_SRuSTransformedModelSet());
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, aElement->models());
  RETURN_INTO_OBJREF(mi, iface::SProS::ModelIterator, ms->iterateModels());
  while (true)
  {
    RETURN_INTO_OBJREF(m, iface::SProS::Model, mi->nextModel());
    if (m == NULL)
      break;

    RETURN_INTO_OBJREF(tm, iface::SRuS::TransformedModel, buildOneModel(m));
    tms->addTransformed(tm);
  }

  tms->add_ref();
  return tms;
}

class CDA_SRuSGeneratedData
  : public iface::SRuS::GeneratedData
{
public:
  CDA_SRuSGeneratedData(iface::SProS::DataGenerator* aDG)
    : _cda_refcount(1), mDataGenerator(aDG) {}
  
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::GeneratedData);
  CDA_IMPL_REFCOUNT;

  iface::SProS::DataGenerator* sedmlDataGenerator() throw()
  {
    mDataGenerator->add_ref();
    return mDataGenerator;
  }
  uint32_t length() throw()
  {
    return mData.size();
  }

  double dataPoint(uint32_t idex)
    throw(std::exception&)
  {
    if (idex >= length())
      throw iface::SRuS::SRuSException();

    return mData[idex];
  }

  std::vector<double> mData;
  ObjRef<iface::SProS::DataGenerator> mDataGenerator;
};

class CDA_SRuSGeneratedDataSet
  : public iface::SRuS::GeneratedDataSet
{
public:
  CDA_SRuSGeneratedDataSet()
    : _cda_refcount(1), mDataRAII(mData) {}
  
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::GeneratedDataSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length()
    throw()
  {
    return mData.size();
  }

  iface::SRuS::GeneratedData*
  item(uint32_t aIdx)
    throw(std::exception&)
  {
    if (aIdx >= length())
      throw iface::SRuS::SRuSException();

    iface::SRuS::GeneratedData* d = mData[aIdx];
    d->add_ref();
    return d;
  }

private:
  std::vector<iface::SRuS::GeneratedData*> mData;
  XPCOMContainerRAII<std::vector<iface::SRuS::GeneratedData*> > mDataRAII;
};

void
CDA_SRuSProcessor::generateData
(
 iface::SRuS::TransformedModelSet* aSet,
 iface::SProS::SEDMLElement* aElement,
 iface::SRuS::GeneratedDataMonitor* aMonitor
)
  throw()
{
  RETURN_INTO_OBJREF(is, iface::cellml_services::CellMLIntegrationService,
                     CreateIntegrationService());

  std::map<std::string, iface::SRuS::TransformedModel*> modelsById;
  XPCOMContainerSecondRAII<std::map<std::string, iface::SRuS::TransformedModel*> > modelsByIdRAII(modelsById);
  std::map<std::string, iface::SProS::Task*> tasksById;
  XPCOMContainerSecondRAII<std::map<std::string, iface::SProS::Task*> > tasksByIdRAII(tasksById);

  std::set<std::string> activeTasks;

  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet, aElement->tasks());
  RETURN_INTO_OBJREF(ti, iface::SProS::TaskIterator, ts->iterateTasks());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::SProS::Task, ti->nextTask());
    if (t == NULL)
      break;
    char* idStr = t->objid();
    t->add_ref();
    tasksById.insert(std::pair<std::string, iface::SProS::Task*>(idStr, t));
    free(idStr);
  }

  for (uint32_t i = 0, l = aSet->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(tm, iface::SRuS::TransformedModel, aSet->item(i));
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, tm->sedmlModel());
    char* idStr = sm->objid();
    tm->add_ref();
    modelsById.insert(std::pair<std::string, iface::SRuS::TransformedModel*>(idStr, tm));
    free(idStr);
  }

  RETURN_INTO_OBJREF(gens, iface::SProS::DataGeneratorSet,
                     aElement->generators());
  RETURN_INTO_OBJREF(geni, iface::SProS::DataGeneratorIterator,
                     gens->iterateDataGenerators());

  while (true)
  {
    RETURN_INTO_OBJREF(gen, iface::SProS::DataGenerator,
                       geni->nextDataGenerator());
    if (gen == NULL)
      break;
    RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet,
                       gen->variables());
    RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator,
                       vs->iterateVariables());
    while (true)
    {
      RETURN_INTO_OBJREF(v, iface::SProS::Variable, vi->nextVariable());
      if (v == NULL)
        break;

      RETURN_INTO_OBJREF(t, iface::SProS::Task, v->taskReference());
      char* idS = t->objid();
      activeTasks.insert(idS);
      free(idS);
    }
  }

  for (std::set<std::string>::iterator i = activeTasks.begin();
       i != activeTasks.end(); i++)
  {
    iface::SProS::Task* t = tasksById[(*i)];
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, t->modelReference());
    if (sm == NULL)
      throw iface::SRuS::SRuSException();

    char* idS = sm->objid();
    iface::SRuS::TransformedModel* tm = modelsById[idS];
    free(idS);

    RETURN_INTO_OBJREF(sim, iface::SProS::Simulation, t->simulationReference());
    if (sim == NULL)
      throw iface::SRuS::SRuSException();

    RETURN_INTO_WSTRING(ksid, sim->algorithmKisaoID());

    // To do: other language support.
    RETURN_INTO_OBJREF(mo, iface::XPCOM::IObject, tm->modelDocument());
    DECLARE_QUERY_INTERFACE_OBJREF(m, mo, iface::cellml_api::Model);
    if (m == NULL)
      throw iface::SRuS::SRuSException();

  }
}

class CDA_SRuSBootstrap
  : public iface::SRuS::Bootstrap
{
public:
  CDA_SRuSBootstrap()
    : _cda_refcount(1) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::Bootstrap);
  CDA_IMPL_REFCOUNT;

  iface::SRuS::SEDMLProcessor* makeDefaultProcessor()
    throw()
  {
    return new CDA_SRuSProcessor();
  }
};

iface::SRuS::Bootstrap*
CreateSRuSBootstrap() throw()
{
  return new CDA_SRuSBootstrap();
}
