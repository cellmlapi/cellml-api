#define MODULE_CONTAINS_SRuS
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
#include <math.h>
#include <set>

class CDA_SRuSProcessor
  : public iface::SRuS::SEDMLProcessor
{
public:
  CDA_SRuSProcessor()
    : mRecursionDepth(0) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::SEDMLProcessor);
  CDA_IMPL_REFCOUNT;

  bool supportsModellingLanguage(const std::wstring& aLang) throw();
  already_AddRefd<iface::SRuS::TransformedModel> buildOneModel(iface::SProS::Model* aModel)
    throw(std::exception&);
  already_AddRefd<iface::SRuS::TransformedModelSet> buildAllModels
    (iface::SProS::SEDMLElement* aElement) throw();
  void generateData(iface::SRuS::TransformedModelSet* aSet,
                    iface::SProS::SEDMLElement* aElement,
                    iface::SRuS::GeneratedDataMonitor* aMonitor) throw(std::exception&);

private:
  already_AddRefd<iface::cellml_api::CellMLElement> xmlToCellML(iface::cellml_api::Model* aModel, iface::dom::Node* aNode)
    throw();
  uint32_t mRecursionDepth;
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
already_AddRefd<iface::cellml_api::CellMLElement>
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
    l.push_front(n);

    n = already_AddRefd<iface::dom::Node>(n->parentNode());
  }

  // Get rid of anything up to and including the document element...
  while (!l.empty())
  {
    DECLARE_QUERY_INTERFACE_OBJREF(el, l.front(), dom::Element);
    l.front()->release_ref();
    l.pop_front();
    if (el != NULL)
      break;
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
  return cur.getPointer();
}

bool
CDA_SRuSProcessor::supportsModellingLanguage(const std::wstring& aLang)
  throw()
{
  if (aLang == L"http://www.cellml.org/cellml/1.1#" ||
      aLang == L"http://www.cellml.org/cellml/1.0#")
    return true;

  // In future, make this extensible so we can support other modelling languages.
  return false;
}

class CDA_SRuSTransformedModel
  : public iface::SRuS::TransformedModel
{
public:
  CDA_SRuSTransformedModel(iface::dom::Document* aDoc, iface::SProS::Model* aModel)
    : mDocument(aDoc), mSEDMLModel(aModel) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::TransformedModel);
  CDA_IMPL_REFCOUNT;

  already_AddRefd<iface::dom::Document> xmlDocument() throw()
  {
    mDocument->add_ref();
    return mDocument.getPointer();
  }

  already_AddRefd<iface::XPCOM::IObject> modelDocument() throw(std::exception&)
  {
    ensureModelOrRaise();
    mModel->add_ref();
    return mModel.getPointer();
  }

  already_AddRefd<iface::SProS::Model> sedmlModel() throw()
  {
    mSEDMLModel->add_ref();
    return mSEDMLModel.getPointer();
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
        RETURN_INTO_WSTRING(ns, el->namespaceURI());
        if (ns == L"http://www.w3.org/1998/Math/MathML")
        {
          v1 = stringToNumber(value);
          seenSep = true;
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

    DECLARE_QUERY_INTERFACE_OBJREF(mme, aME, mathml_dom::MathMLMathElement);
    if (mme != NULL)
    {
      if (mme->nArguments() != 1)
        throw iface::SRuS::SRuSException();
      RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, mme->getArgument(1));
      return eval(me);
    }

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
#ifndef _MSC_VER
        else if (opln == L"arccosh")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acosh(args[0]);
        }
#endif
        else if (opln == L"arccot")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atan(1.0 / args[0]);
        }
#ifndef _MSC_VER
        else if (opln == L"arccoth")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atanh(1.0 / args[0]);
        }
#endif
        else if (opln == L"arccsc")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asin(1.0 / args[0]);
        }
#ifndef _MSC_VER
        else if (opln == L"arccsch")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asinh(1.0 / args[0]);
        }
#endif
        else if (opln == L"arcsec")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acos(1.0 / args[0]);
        }
#ifndef _MSC_VER
        else if (opln == L"arcsech")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return acosh(1.0 / args[0]);
        }
#endif
        else if (opln == L"arcsin")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asin(args[0]);
        }
#ifndef _MSC_VER
        else if (opln == L"arcsinh")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return asinh(args[0]);
        }
#endif
        else if (opln == L"arctan")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atan(args[0]);
        }
#ifndef _MSC_VER
        else if (opln == L"arctanh")
        {
          if (args.size() != 1)
            throw iface::SRuS::SRuSException();
          return atanh(args[0]);
        }
#endif
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
    double result = 0 /* Assignment avoids compiler warning. */;
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
      return exp(1.0);

    throw iface::SRuS::SRuSException(); // no such predefined symbol
  }

  virtual double evalAggregate(iface::mathml_dom::MathMLCsymbolElement* aOp, iface::mathml_dom::MathMLApplyElement* aApply)
  {
    throw NeedsAggregate();
  }

  void setVariable(const std::wstring& aName, double aValue)
  {
    std::map<std::wstring, double>::iterator i = mVariableValues.find(aName);
    if (i == mVariableValues.end())
      mVariableValues.insert(std::pair<std::wstring, double>(aName, aValue));
    else
      (*i).second = aValue;
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

class CDA_ScopedIncrement
{
public:
  CDA_ScopedIncrement(uint32_t& aInt)
    : mInt(aInt)
  {
    mInt++;
  }

  ~CDA_ScopedIncrement()
  {
    mInt--;
  }

private:
  uint32_t& mInt;
};

already_AddRefd<iface::SRuS::TransformedModel>
CDA_SRuSProcessor::buildOneModel(iface::SProS::Model* aModel)
  throw(std::exception&)
{
  try
  {
    RETURN_INTO_WSTRING(lang, aModel->language());
    if (lang != L"urn:sedml:language:cellml" &&
        lang != L"urn:sedml:language:cellml.1_0" &&
        lang != L"urn:sedml:language:cellml.1_1")
    {
      // To do: Support other languages here...
      throw iface::SRuS::SRuSException();
    }

    // Get the URI...
    RETURN_INTO_WSTRING(uri, aModel->source());

    RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    ObjRef<iface::dom::Document> doc;

    // Check if it is an identifier for another model...
    RETURN_INTO_OBJREF(cmb, iface::SProS::Base, aModel->parent());
    DECLARE_QUERY_INTERFACE_OBJREF(cm, cmb, SProS::SEDMLElement);
    CDA_ScopedIncrement rdsi(mRecursionDepth);
    if (cm != NULL && mRecursionDepth <= 50)
    {
      RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, cm->models());
      RETURN_INTO_OBJREF(refmod, iface::SProS::Model, ms->getModelByIdentifier(uri.c_str()));
      if (refmod != NULL)
      {
        RETURN_INTO_OBJREF(tmrefmod, iface::SRuS::TransformedModel, buildOneModel(refmod));
        doc = already_AddRefd<iface::dom::Document>(tmrefmod->xmlDocument());
      }
    }
    if (doc == NULL)
    {
      RETURN_INTO_OBJREF(seb, iface::SProS::Base, aModel->parent());
      DECLARE_QUERY_INTERFACE_OBJREF(se, seb, SProS::SEDMLElement);
      RETURN_INTO_WSTRING(baseURL, se->originalURL());
      RETURN_INTO_WSTRING(absURI, cb->makeURLAbsolute(baseURL.c_str(), uri.c_str()));
      uri = absURI;

      // Load it...
      RETURN_INTO_OBJREF(ml, iface::cellml_api::DOMURLLoader, cb->localURLLoader());
      doc = already_AddRefd<iface::dom::Document>(ml->loadDocument(uri.c_str()));
      if (doc != NULL)
      {
        RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
        de->setAttributeNS(L"http://www.w3.org/XML/1998/namespace", L"base", uri.c_str());
      }
    }
    
    RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
    
    // Apply the transformations to the document...
    RETURN_INTO_OBJREF(cs, iface::SProS::ChangeSet, aModel->changes());
    RETURN_INTO_OBJREF(ci, iface::SProS::ChangeIterator, cs->iterateChanges());
    while (true)
    {
      RETURN_INTO_OBJREF(c, iface::SProS::Change, ci->nextChange());
      if (c == NULL)
        break;

      RETURN_INTO_OBJREF(de, iface::dom::Element, c->domElement());
      RETURN_INTO_OBJREF(resolver, iface::xpath::XPathNSResolver,
                         xe->createNSResolver(de));

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
        SEDMLMathEvaluator sme;

        // Set parameters...
        RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, cc->parameters());
        RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
        while (true)
        {
          RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
          if (p == NULL)
            break;
          
          RETURN_INTO_WSTRING(pid, p->id());
          sme.setVariable(pid, p->value());
        }

        // Set variables...
        RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet, cc->variables());
        RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator, vs->iterateVariables());
        while (true)
        {
          RETURN_INTO_OBJREF(v, iface::SProS::Variable, vi->nextVariable());
          if (v == NULL)
            break;

          RETURN_INTO_WSTRING(targ, v->target());
          RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                             xe->evaluate(targ.c_str(), doc, resolver,
                                          iface::xpath::XPathResult::NUMBER_TYPE, NULL));
          RETURN_INTO_WSTRING(vid, v->id());
          sme.setVariable(vid, xr->numberValue());
        }

        RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, cc->math());
        double v = sme.eval(m);

        RETURN_INTO_WSTRING(targ, cc->target());
        RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                           xe->evaluate(targ.c_str(), doc, resolver,
                                        iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE, NULL));
        RETURN_INTO_OBJREF(n, iface::dom::Node, xr->singleNodeValue());
        wchar_t vstr[30];
        any_swprintf(vstr, 30, L"%f", v);
        n->nodeValue(vstr);
      }
    }

    return new CDA_SRuSTransformedModel(doc, aModel);
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
    : mTransformedRAII(mTransformed) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::TransformedModelSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length() throw()
  {
    return mTransformed.size();
  }

  already_AddRefd<iface::SRuS::TransformedModel> item(uint32_t aIdx)
    throw(std::exception&)
  {
    if (aIdx >= length())
      throw iface::SRuS::SRuSException();

    iface::SRuS::TransformedModel* ret = mTransformed[aIdx];
    ret->add_ref();
    return ret;
  }

  already_AddRefd<iface::SRuS::TransformedModel> getItemByID(const std::wstring& aMatchID)
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

already_AddRefd<iface::SRuS::TransformedModelSet>
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
  return tms.getPointer();
}

class CDA_SRuSGeneratedData
  : public iface::SRuS::GeneratedData
{
public:
  CDA_SRuSGeneratedData(iface::SProS::DataGenerator* aDG)
    : mDataGenerator(aDG) {}
  
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::GeneratedData);
  CDA_IMPL_REFCOUNT;

  already_AddRefd<iface::SProS::DataGenerator> sedmlDataGenerator() throw()
  {
    mDataGenerator->add_ref();
    return mDataGenerator.getPointer();
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
    : mDataRAII(mData) {}
  
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::GeneratedDataSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length()
    throw()
  {
    return mData.size();
  }

  already_AddRefd<iface::SRuS::GeneratedData>
  item(uint32_t aIdx)
    throw(std::exception&)
  {
    if (aIdx >= length())
      throw iface::SRuS::SRuSException();

    iface::SRuS::GeneratedData* d = mData[aIdx];
    d->add_ref();
    return d;
  }

  std::vector<iface::SRuS::GeneratedData*> mData;
private:
  XPCOMContainerRAII<std::vector<iface::SRuS::GeneratedData*> > mDataRAII;
};

class CDA_SRuSResultBridge
  : public iface::cellml_services::IntegrationProgressObserver
{
public:
  CDA_SRuSResultBridge(iface::cellml_services::CellMLIntegrationRun* aRun,
                       iface::SRuS::GeneratedDataMonitor* aMonitor,
                       iface::cellml_services::CodeInformation* aCodeInfo,
                       const std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >&
                       aVarInfoByDataGeneratorId,
                       const std::map<std::wstring, iface::SProS::DataGenerator*>& aDataGeneratorsById)
    : mRun(aRun), mMonitor(aMonitor), mCodeInfo(aCodeInfo),
      mAggregateMode(0), mVarInfoByDataGeneratorId(aVarInfoByDataGeneratorId),
      mDataGeneratorsById(aDataGeneratorsById),
      mDataGeneratorsByIdRAII(mDataGeneratorsById), mTotalN(0)
  {
    uint32_t aic = mCodeInfo->algebraicIndexCount();
    uint32_t ric = mCodeInfo->rateIndexCount();
    mRecSize = 2 * ric + aic + 1;

    for (std::map<std::wstring, iface::SProS::DataGenerator*>::iterator i =
           mDataGeneratorsById.begin(); i != mDataGeneratorsById.end(); i++)
      (*i).second->add_ref();
  }

  ~CDA_SRuSResultBridge()
  {
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::IntegrationProgressObserver);

  void computedConstants(const std::vector<double>& aValues) throw()
  {
    mConstants = aValues;
  }

  void done()
    throw(std::exception&)
  {
    try
    {
      if (mAggregateMode == 1)
      {
        RETURN_INTO_OBJREF(gds, CDA_SRuSGeneratedDataSet, new CDA_SRuSGeneratedDataSet());
        for (std::map<std::wstring, std::map<std::wstring, std::vector<double> > >::iterator i =
               mAggregateData.begin(); i != mAggregateData.end(); i++)
        {
          iface::SProS::DataGenerator* dg = mDataGeneratorsById[(*i).first];
          RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, dg->math());
          
          SEDMLMathEvaluatorWithAggregate smea((*i).second);
          // Set parameters too...
          RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());
          RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
          while (true)
          {
            RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
            if (p == NULL)
              break;
            
            RETURN_INTO_WSTRING(pid, p->id());
            smea.setVariable(pid, p->value());
          }
          
          RETURN_INTO_OBJREF(gd, CDA_SRuSGeneratedData, new CDA_SRuSGeneratedData(dg));
          for (uint32_t j = 0; j < mTotalN; j++)
          {
            for (std::map<std::wstring, std::vector<double> >::iterator vi = (*i).second.begin();
                 vi != (*i).second.end(); vi++)
              smea.setVariable((*vi).first, (*vi).second[j]);
            gd->mData.push_back(smea.eval(m));
          }
          
          gd->add_ref();
          gds->mData.push_back(gd);
        }
      }
      mMonitor->done();
    }
    catch (...) {}

    mRun = NULL;
  }

  void failed(const std::string& aErrorMessage)
    throw(std::exception&)
  {
    try
    {
      mMonitor->failure(aErrorMessage);
    }
    catch (...)
    {
    }

    mRun = NULL;
  }

  void results(const std::vector<double>& state)
    throw(std::exception&)
  {
    uint32_t n = state.size() / mRecSize;
    mTotalN += n;
    if (mAggregateMode == 1)
    {
      for (std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator i =
             mVarInfoByDataGeneratorId.begin(); i != mVarInfoByDataGeneratorId.end(); i++)
        for (std::list<std::pair<std::wstring, int32_t> >::iterator li = (*i).second.begin();
             li != (*i).second.end(); li++)
        {
          int32_t idx = (*li).second;
          std::vector<double>& l = mAggregateData[(*i).first][(*li).first];
          for (uint32_t j = 0; j < n; j++)
            l.push_back(idx < 0 ? mConstants[-1 - idx] : state[j * mRecSize + idx]);
        }
      return;
    }

    SEDMLMathEvaluator sme;
    if (mAggregateMode == 0)
      sme.setExploreEverything(true);

    RETURN_INTO_OBJREF(gds, CDA_SRuSGeneratedDataSet, new CDA_SRuSGeneratedDataSet());

    for (std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator i =
           mVarInfoByDataGeneratorId.begin(); i != mVarInfoByDataGeneratorId.end(); i++)
    {
      iface::SProS::DataGenerator* dg = mDataGeneratorsById[(*i).first];
      // Set parameters too...
      RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());
      RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());
      while (true)
      {
        RETURN_INTO_OBJREF(p, iface::SProS::Parameter, pi->nextParameter());
        if (p == NULL)
          break;
        
          RETURN_INTO_WSTRING(pid, p->id());
          sme.setVariable(pid, p->value());
      }

      RETURN_INTO_OBJREF(gd, CDA_SRuSGeneratedData, new CDA_SRuSGeneratedData(dg));

      for (uint32_t j = 0; j < n; j++)
      {
        for (std::list<std::pair<std::wstring, int32_t> >::iterator li = (*i).second.begin();
             li != (*i).second.end(); li++)
          sme.setVariable((*li).first, state[j * mRecSize + (*li).second]);
        
        try
        {
          RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLMathElement, dg->math());
          double v = sme.eval(m);
          gd->mData.push_back(v);
        }
        catch (NeedsAggregate&)
        {
          mAggregateMode = 1;
          results(state);
          return;
        }
      }

      gd->add_ref();
      gds->mData.push_back(gd);
    }

    mMonitor->progress(gds);
  }

private:
  ObjRef<iface::cellml_services::CellMLIntegrationRun> mRun;
  ObjRef<iface::SRuS::GeneratedDataMonitor> mMonitor;
  ObjRef<iface::cellml_services::CodeInformation> mCodeInfo;
  // 0: Unknown. 1: Need aggregate. -1: Don't need aggregate.
  int mAggregateMode;
  std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
    mVarInfoByDataGeneratorId;
  std::vector<double> mConstants;
  std::map<std::wstring, std::map<std::wstring, std::vector<double> > > mAggregateData;
  std::map<std::wstring, iface::SProS::DataGenerator*> mDataGeneratorsById;
  XPCOMContainerSecondRAII<std::map<std::wstring, iface::SProS::DataGenerator*> > mDataGeneratorsByIdRAII;
  uint32_t mRecSize, mTotalN;
};

class CDA_SRuSContinueAtStartTime
  : public iface::cellml_services::IntegrationProgressObserver
{
public:
  CDA_SRuSContinueAtStartTime(iface::cellml_services::CellMLIntegrationRun* aRun,
                              iface::SRuS::GeneratedDataMonitor* aMonitor,
                              iface::cellml_services::CodeInformation* aCodeInfo,
                              const std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >&
                                aVarInfoByDataGeneratorId,
                              std::map<std::wstring, iface::SProS::DataGenerator*>& aDataGeneratorsById
                             )
    : mRun(aRun), mMonitor(aMonitor), mCodeInfo(aCodeInfo),
      mVarInfoByDataGeneratorId(aVarInfoByDataGeneratorId),
      mDataGeneratorsById(aDataGeneratorsById),
      mDataGeneratorsByIdRAII(mDataGeneratorsById)
  {
    uint32_t aic = mCodeInfo->algebraicIndexCount();
    uint32_t ric = mCodeInfo->rateIndexCount();
    mRecSize = 2 * ric + aic + 1;
    mRow = new double[mRecSize];

    for (std::map<std::wstring, iface::SProS::DataGenerator*>::iterator i =
           mDataGeneratorsById.begin(); i != mDataGeneratorsById.end(); i++)
      (*i).second->add_ref();
  }

  ~CDA_SRuSContinueAtStartTime()
  {
    delete [] mRow;
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::IntegrationProgressObserver);

  void computedConstants(const std::vector<double>&) throw()
  {
  }

  void failed(const std::string& aMessage)
    throw (std::exception&)
  {
    mMonitor->failure(aMessage);
  }

  void results(const std::vector<double>& state)
    throw (std::exception&)
  {
    uint32_t nRows = state.size() / mRecSize;
    uint32_t offs = (nRows - 1) * mRecSize;
    for (uint32_t i = 0; i < mRecSize; i++)
      mRow[i] = state[i + offs];
  }

  void done()
    throw (std::exception&)
  {
    // Start the main run...
    RETURN_INTO_OBJREF(rb, CDA_SRuSResultBridge,
                       new CDA_SRuSResultBridge(mRun, mMonitor, mCodeInfo, mVarInfoByDataGeneratorId, mDataGeneratorsById));
    mRun->setProgressObserver(rb);
    // Time, States, Rates, Algebraic
    RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator,
                       mCodeInfo->iterateTargets());
    while (true)
    {
      RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget, cti->nextComputationTarget());
      if (ct == NULL)
        break;

      if (ct->type() == iface::cellml_services::STATE_VARIABLE)
        mRun->setOverride(iface::cellml_services::STATE_VARIABLE,
                          ct->assignedIndex(), mRow[1 + ct->assignedIndex()]);
    }
    mRun->start();
  }

private:
  ObjRef<iface::cellml_services::CellMLIntegrationRun> mRun;
  ObjRef<iface::SRuS::GeneratedDataMonitor> mMonitor;
  ObjRef<iface::cellml_services::CodeInformation> mCodeInfo;
  uint32_t mRecSize;
  double* mRow;
  std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
    mVarInfoByDataGeneratorId;
  std::map<std::wstring, iface::SProS::DataGenerator*>
    mDataGeneratorsById;
  XPCOMContainerSecondRAII<std::map<std::wstring, iface::SProS::DataGenerator*> >
    mDataGeneratorsByIdRAII;
};

void
CDA_SRuSProcessor::generateData
(
 iface::SRuS::TransformedModelSet* aSet,
 iface::SProS::SEDMLElement* aElement,
 iface::SRuS::GeneratedDataMonitor* aMonitor
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(is, iface::cellml_services::CellMLIntegrationService,
                     CreateIntegrationService());

  std::map<std::wstring, iface::SRuS::TransformedModel*> modelsById;
  XPCOMContainerSecondRAII<std::map<std::wstring, iface::SRuS::TransformedModel*> > modelsByIdRAII(modelsById);
  std::map<std::wstring, iface::SProS::Task*> tasksById;
  XPCOMContainerSecondRAII<std::map<std::wstring, iface::SProS::Task*> > tasksByIdRAII(tasksById);
  
  std::set<std::wstring> activeTasks;
  std::multimap<std::wstring, std::wstring> dataGeneratorIdsByTaskId;

  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet, aElement->tasks());
  RETURN_INTO_OBJREF(ti, iface::SProS::TaskIterator, ts->iterateTasks());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::SProS::Task, ti->nextTask());
    if (t == NULL)
      break;
    std::wstring idStr = t->id();
    t->add_ref();
    tasksById.insert(std::pair<std::wstring, iface::SProS::Task*>(idStr, t));
  }

  for (uint32_t i = 0, l = aSet->length(); i < l; i++)
  {
    RETURN_INTO_OBJREF(tm, iface::SRuS::TransformedModel, aSet->item(i));
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, tm->sedmlModel());
    std::wstring idStr = sm->id();
    tm->add_ref();
    modelsById.insert(std::pair<std::wstring, iface::SRuS::TransformedModel*>(idStr, tm));
  }

  RETURN_INTO_OBJREF(gens, iface::SProS::DataGeneratorSet,
                     aElement->generators());
  RETURN_INTO_OBJREF(geni, iface::SProS::DataGeneratorIterator,
                     gens->iterateDataGenerators());

  std::map<std::wstring, iface::SProS::DataGenerator*> dataGeneratorsById;
  XPCOMContainerSecondRAII<std::map<std::wstring, iface::SProS::DataGenerator*> > dataGeneratorsByIdRAII(dataGeneratorsById);
  while (true)
  {
    RETURN_INTO_OBJREF(gen, iface::SProS::DataGenerator,
                       geni->nextDataGenerator());
    if (gen == NULL)
      break;

    std::wstring dgid(gen->id());
    gen->add_ref();
    dataGeneratorsById.insert(std::pair<std::wstring, iface::SProS::DataGenerator*>(dgid, gen));

    RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet,
                       gen->variables());
    RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator,
                       vs->iterateVariables());

    bool didAdd = false;
    while (true)
    {
      RETURN_INTO_OBJREF(v, iface::SProS::Variable, vi->nextVariable());
      if (v == NULL)
        break;

      RETURN_INTO_OBJREF(t, iface::SProS::Task, v->taskReference());
      std::wstring idS = t->id();
      activeTasks.insert(idS);
      if (!didAdd)
      {
        didAdd = true;
        std::wstring dgidS = gen->id();
        dataGeneratorIdsByTaskId.insert(std::pair<std::wstring, std::wstring>(idS, dgidS));
      }
    }
  }

  for (std::set<std::wstring>::iterator i = activeTasks.begin();
       i != activeTasks.end(); i++)
  {
    iface::SProS::Task* t = tasksById[(*i)];
    RETURN_INTO_OBJREF(sm, iface::SProS::Model, t->modelReference());
    if (sm == NULL)
      throw iface::SRuS::SRuSException();

    std::wstring idS = sm->id();
    iface::SRuS::TransformedModel* tm = modelsById[idS];

    RETURN_INTO_WSTRING(taskId, t->id());

    RETURN_INTO_OBJREF(sim, iface::SProS::Simulation, t->simulationReference());
    if (sim == NULL)
      throw iface::SRuS::SRuSException();

    RETURN_INTO_WSTRING(ksid, sim->algorithmKisaoID());
    DECLARE_QUERY_INTERFACE_OBJREF(utc, sim, SProS::UniformTimeCourse);
    // To do: things other than uniform time course.
    if (utc == NULL)
      throw iface::SRuS::SRuSException();

    // To do: other language support.
    RETURN_INTO_OBJREF(mo, iface::XPCOM::IObject, tm->modelDocument());
    DECLARE_QUERY_INTERFACE_OBJREF(m, mo, cellml_api::Model);
    if (m == NULL)
      throw iface::SRuS::SRuSException();

    RETURN_INTO_OBJREF(doc, iface::dom::Document, tm->xmlDocument());

    ObjRef<iface::cellml_services::CodeInformation> ci;

    if (ksid == L"https://computation.llnl.gov/casc/sundials/documentation/ida_guide/") //IDA
    {
      RETURN_INTO_OBJREF(cmDAE, iface::cellml_services::DAESolverCompiledModel,
                         is->compileModelDAE(m));
      ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmDAE->codeInformation());
    }
    else
    {
      RETURN_INTO_OBJREF(cmODE, iface::cellml_services::ODESolverCompiledModel,
                         is->compileModelODE(m));

      ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmODE->codeInformation());
    }


    std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > > variableInfoIdxByDataGeneratorId;
    // For each DataGenerator...
    for (std::multimap<std::wstring, std::wstring>::iterator dgi = dataGeneratorIdsByTaskId.find(taskId);
         dgi != dataGeneratorIdsByTaskId.end() && (*dgi).first == taskId; dgi++)
    {
      std::wstring dgid = (*dgi).second;
      iface::SProS::DataGenerator* dg = dataGeneratorsById[dgid];
      RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet, dg->variables());
      RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator, vs->iterateVariables());
      while (true)
      {
        RETURN_INTO_OBJREF(sv, iface::SProS::Variable, vi->nextVariable());
        if (sv == NULL)
          break;

        RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
        RETURN_INTO_OBJREF(de, iface::dom::Element, sv->domElement());
        RETURN_INTO_OBJREF(resolver, iface::xpath::XPathNSResolver,
                           xe->createNSResolver(de));
        RETURN_INTO_WSTRING(expr, sv->target());
        RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                           xe->evaluate(expr.c_str(), doc, resolver,
                                        iface::xpath::XPathResult::FIRST_ORDERED_NODE_TYPE,
                                        NULL));
        RETURN_INTO_OBJREF(n, iface::dom::Node, xr->singleNodeValue());
        if (n == NULL)
          continue;

        RETURN_INTO_OBJREF(el, iface::cellml_api::CellMLElement, xmlToCellML(m, n));
        DECLARE_QUERY_INTERFACE_OBJREF(cv, el, cellml_api::CellMLVariable);
        if (cv == NULL)
          continue;

        RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator, ci->iterateTargets());
        while (true)
        {
          RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget, cti->nextComputationTarget());
          if (ct == NULL)
            break;
          if (ct->degree() != 0)
            continue;
          
          RETURN_INTO_OBJREF(ctv, iface::cellml_api::CellMLVariable, ct->variable());
          if (!CDA_objcmp(ctv, cv))
          {
            int32_t idx = ct->assignedIndex();
            iface::cellml_services::VariableEvaluationType t(ct->type());
            switch (t)
            {
            case iface::cellml_services::VARIABLE_OF_INTEGRATION:
              idx = 0;
              break;
            case iface::cellml_services::CONSTANT:
              idx = -1 - idx;
              break;
            case iface::cellml_services::STATE_VARIABLE:
            case iface::cellml_services::PSEUDOSTATE_VARIABLE:
              idx++;
              break;
            case iface::cellml_services::ALGEBRAIC:
              idx += 1 + ci->rateIndexCount() * 2;
              break;
            default:
              throw iface::SRuS::SRuSException();
            }
            RETURN_INTO_WSTRING(svid, sv->id());
            
            std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >::iterator
              vi(variableInfoIdxByDataGeneratorId.find(dgid));
            if (vi == variableInfoIdxByDataGeneratorId.end())
            {
              std::list<std::pair<std::wstring, int32_t> > l;
              l.push_back(std::pair<std::wstring, int32_t>(svid, idx));
              variableInfoIdxByDataGeneratorId.insert(
                std::pair<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
                (dgid, l)
                                                     );
            }
            else
              (*vi).second.push_back(std::pair<std::wstring, uint32_t>(svid, idx));
            break;
          }
        }
      }
    }

    double it = utc->initialTime(), ost = utc->outputStartTime(), oet = utc->outputEndTime();

    uint32_t nSamples = 1;
    DECLARE_QUERY_INTERFACE_OBJREF(ssi, utc, SProS::SamplingSensitivityAnalysis);
    if (ssi != NULL)
      nSamples = ssi->numberOfSamples();

    for (uint32_t sampleNo = 0; sampleNo < nSamples; sampleNo++)
    {
      ObjRef<iface::cellml_services::CellMLIntegrationRun> cir1, cir2;

    // Now we need to look at the KISAO term...
    // XXX this is not a proper KISAO term, but a placeholder until KISAO adds one for IDA.
      if (ksid == L"https://computation.llnl.gov/casc/sundials/documentation/ida_guide/") //IDA
      {
        RETURN_INTO_OBJREF(cmDAE, iface::cellml_services::DAESolverCompiledModel,
                           is->compileModelDAE(m));
        ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmDAE->codeInformation());
        cir1 = already_AddRefd<iface::cellml_services::DAESolverRun>(is->createDAEIntegrationRun(cmDAE));
        cir2 = already_AddRefd<iface::cellml_services::DAESolverRun>(is->createDAEIntegrationRun(cmDAE));
      }
      else
      {
        RETURN_INTO_OBJREF(cmODE, iface::cellml_services::ODESolverCompiledModel,
                           is->compileModelODE(m));
        iface::cellml_services::ODESolverRun* ciro1 = is->createODEIntegrationRun(cmODE);
        iface::cellml_services::ODESolverRun* ciro2 = is->createODEIntegrationRun(cmODE);
        ci = already_AddRefd<iface::cellml_services::CodeInformation>(cmODE->codeInformation());
        cir1 = already_AddRefd<iface::cellml_services::ODESolverRun>(ciro1);
        cir2 = already_AddRefd<iface::cellml_services::ODESolverRun>(ciro2);
        
        if (ksid == L"KISAO:0000019") // CVODE
        {
          ciro1->stepType(iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE);
          ciro2->stepType(iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE);
        }
        else if (ksid == L"KISAO:0000032")
        {
          ciro1->stepType(iface::cellml_services::RUNGE_KUTTA_4);
          ciro2->stepType(iface::cellml_services::RUNGE_KUTTA_4);
        }
        else if (ksid == L"KISAO:0000086")
        {
          ciro1->stepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5);
          ciro2->stepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5);
        }
        else if (ksid == L"KISAO:0000030")
        {
          ciro1->stepType(iface::cellml_services::GEAR_1);
          ciro2->stepType(iface::cellml_services::GEAR_1);
        }
        else // it's a term we don't handle.
          throw iface::SRuS::SRuSException();
      }

      // Run a simulation to bring the system to the start time...
      RETURN_INTO_OBJREF(cast, CDA_SRuSContinueAtStartTime,
                         new CDA_SRuSContinueAtStartTime(cir2, aMonitor, ci,
                                                         variableInfoIdxByDataGeneratorId,
                                                         dataGeneratorsById));
      cir2->setResultRange(ost, oet, 0);
      cir2->setTabulationStepControl((oet - ost) / utc->numberOfPoints(), true);
      
      cir1->setResultRange(it, ost, 1);
      cir1->setProgressObserver(cast);
      cir1->start();
    }
  }
}

class CDA_SRuSBootstrap
  : public iface::SRuS::Bootstrap
{
public:
  CDA_SRuSBootstrap()
  {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::Bootstrap);
  CDA_IMPL_REFCOUNT;

  already_AddRefd<iface::SRuS::SEDMLProcessor> makeDefaultProcessor()
    throw()
  {
    return new CDA_SRuSProcessor();
  }
};

already_AddRefd<iface::SRuS::Bootstrap>
CreateSRuSBootstrap() throw()
{
  return new CDA_SRuSBootstrap();
}
