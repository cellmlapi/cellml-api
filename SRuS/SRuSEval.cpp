#define MODULE_CONTAINS_SRuS

#include "SRuSImpl.hxx"

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
    throw iface::SRuS::SRuSException(L"Expected a <sep/> in cn element but didn't find one.");

  v2 = stringToNumber(value);
}


SEDMLMathEvaluator::SEDMLMathEvaluator()
  : mExploreEverything(false)
{
}

void
SEDMLMathEvaluator::setExploreEverything(bool aExploreEverything)
{
  mExploreEverything = aExploreEverything;
}

double
SEDMLMathEvaluator::eval(iface::mathml_dom::MathMLElement* aME)
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
      throw iface::SRuS::SRuSException(L"Found a MathML math element with the wrong number of children (i.e. not exactly one)");
    RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, mme->getArgument(1));
    return eval(me);
  }

  throw iface::SRuS::SRuSException(L"Unhandled type of MathML element");
}

double
SEDMLMathEvaluator::evalApply(iface::mathml_dom::MathMLApplyElement* mae)
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
    throw iface::SRuS::SRuSException(L"Found a math operator not in the MathML namespace");

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
          throw iface::SRuS::SRuSException(L"Found an abs with the wrong number of arguments.");
        return std::abs(args[0]);
      }
      else
        throw iface::SRuS::SRuSException(L"Unknown operator");

    case L'n':
      if (opln == L"and")
      {
        bool success = true;
        for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
          success &= ((*i) != 0.0);
        return success ? 1.0 : 0.0;
      }
      else
        throw iface::SRuS::SRuSException(L"Unknown operator");

    case L'r':
      if (opln == L"arccos")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arccos with the wrong number of arguments.");
        return std::acos(args[0]);
      }
#ifndef _MSC_VER
      else if (opln == L"arccosh")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arccosh with the wrong number of arguments.");
        return acosh(args[0]);
      }
#endif
      else if (opln == L"arccot")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arccot with the wrong number of arguments.");
        return std::atan(1.0 / args[0]);
      }
#ifndef _MSC_VER
      else if (opln == L"arccoth")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arccoth with the wrong number of arguments.");
        return atanh(1.0 / args[0]);
      }
#endif
      else if (opln == L"arccsc")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arccsc with the wrong number of arguments.");
        return std::asin(1.0 / args[0]);
      }
#ifndef _MSC_VER
      else if (opln == L"arccsch")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arccsch with the wrong number of arguments.");
        return asinh(1.0 / args[0]);
      }
#endif
      else if (opln == L"arcsec")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arcsec with the wrong number of arguments.");
        return std::acos(1.0 / args[0]);
      }
#ifndef _MSC_VER
      else if (opln == L"arcsech")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arcsech with the wrong number of arguments.");
        return acosh(1.0 / args[0]);
      }
#endif
      else if (opln == L"arcsin")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arcsin with the wrong number of arguments.");
        return std::asin(args[0]);
      }
#ifndef _MSC_VER
      else if (opln == L"arcsinh")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arcsinh with the wrong number of arguments.");
        return asinh(args[0]);
      }
#endif
      else if (opln == L"arctan")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arctan with the wrong number of arguments.");
        return std::atan(args[0]);
      }
#ifndef _MSC_VER
      else if (opln == L"arctanh")
      {
        if (args.size() != 1)
          throw iface::SRuS::SRuSException(L"Found an arctanh with the wrong number of arguments.");
        return atanh(args[0]);
      }
#endif
      else
        throw iface::SRuS::SRuSException(L"Unknown operator.");

    default:
      throw iface::SRuS::SRuSException(L"Unknown operator.");
    }

  case L'c':
    if (opln == L"ceiling")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a ceiling with the wrong number of arguments.");
      return std::ceil(args[0]);
    }
    else if (opln == L"cos")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a cos with the wrong number of arguments.");
      return std::cos(args[0]);
    }
    else if (opln == L"cosh")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a cosh with the wrong number of arguments.");
      return std::cosh(args[0]);
    }
    else if (opln == L"cot")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a cot with the wrong number of arguments.");
      return 1.0 / std::tan(args[0]);
    }
    else if (opln == L"coth")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a coth with the wrong number of arguments.");
      return 1.0 / std::tanh(args[0]);
    }
    else if (opln == L"csc")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a csc with the wrong number of arguments.");
      return 1.0 / std::sin(args[0]);
    }
    else if (opln == L"csch")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a csch with the wrong number of arguments.");
      return 1.0 / std::sinh(args[0]);
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'd':
    if (opln == L"divide")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found a divide with the wrong number of arguments.");
      return args[0] / args[1];
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'e':
    if (opln == L"eq")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found an eq with the wrong number of arguments.");
      return (args[0] == args[1]) ? 1.0 : 0.0;
    }
    else if (opln == L"exp")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found an exp with the wrong number of arguments.");
      return std::exp(args[0]);
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'f':
    if (opln == L"factorial")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a factorial with the wrong number of arguments.");
      double v = 1.0;
      for (double m = args[0]; m > 0; m -= 1.0)
        v *= m;
      return v;
    }
    else if (opln == L"floor")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a floor with the wrong number of arguments.");
      return std::floor(args[0]);
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'g':
    if (opln == L"geq")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found a geq with the wrong number of arguments.");
      return (args[0] >= args[1]) ? 1.0 : 0.0;
    }
    else if (opln == L"gt")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found a gt with the wrong number of arguments.");
      return (args[0] > args[1]) ? 1.0 : 0.0;
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'l':
    if (opln == L"leq")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found a leq with the wrong number of arguments.");
      return (args[0] <= args[1]) ? 1.0 : 0.0;
    }
    else if (opln == L"ln")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a ln with the wrong number of arguments.");
      return std::log(args[0]);
    }
    else if (opln == L"log")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a log with the wrong number of arguments.");
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
            throw iface::SRuS::SRuSException(L"Found a logbase with the wrong number of arguments.");
          RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement, el->getArgument(1));
          double lb = eval(arg1);
          return std::log(args[0]) / log(lb);
        }
      }
      return std::log10(args[0]);
    }
    else if (opln == L"lt")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found a lt with the wrong number of arguments.");
      return (args[0] < args[1]) ? 1.0 : 0.0;
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'm':
    if (opln == L"minus")
    {
      if (args.size() == 1)
        return -args[0];
      else if (args.size() == 2)
        return args[0] - args[1];
      else
        throw iface::SRuS::SRuSException(L"Found a minus with the wrong number of arguments.");
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'n':
    if (opln == L"neq")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found an neq with the wrong number of arguments.");
      return args[0] != args[1];
    }
    else if (opln == L"not")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a not with the wrong number of arguments.");
      return (args[0] == 0.0) ? 1.0 : 0.0;
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'o':
    if (opln == L"or")
    {
      bool any = false;
      for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
        any |= ((*i) != 0.0);
      return any ? 1.0 : 0.0;
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

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
        throw iface::SRuS::SRuSException(L"Found a power with the wrong number of arguments.");
      return std::pow(args[0], args[1]);
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'r':
    if (opln == L"root")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a root with the wrong number of arguments.");
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
            throw iface::SRuS::SRuSException(L"Found a degree with the wrong number of arguments.");
          RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement, el->getArgument(1));
          double deg = eval(arg1);
          return std::pow(args[0], 1.0 / deg);
        }
      }
      return std::sqrt(args[0]);
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L's':
    if (opln == L"sec")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a sec with the wrong number of arguments.");
      return 1.0 / std::cos(args[0]);
    }
    else if (opln == L"sech")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a sech with the wrong number of arguments.");
      return 1.0 / std::cosh(args[0]);
    }
    else if (opln == L"sin")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a sin with the wrong number of arguments.");
      return std::sin(args[0]);
    }
    else if (opln == L"sinh")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a sinh with the wrong number of arguments.");
      return std::sinh(args[0]);
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L't':
    if (opln == L"tan")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a tan with the wrong number of arguments.");
      return std::tan(args[0]);
    }
    else if (opln == L"tanh")
    {
      if (args.size() != 1)
        throw iface::SRuS::SRuSException(L"Found a tanh with the wrong number of arguments.");
      return std::tanh(args[0]);
    }
    else if (opln == L"times")
    {
      double prod = 1.0;
      for (std::vector<double>::iterator i = args.begin(); i != args.end(); i++)
        prod *= *i;
      return prod;
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  case L'x':
    if (opln == L"xor")
    {
      if (args.size() != 2)
        throw iface::SRuS::SRuSException(L"Found an xor with the wrong number of arguments.");
      return ((args[0] == 0) != (args[1] == 0)) ? 1.0 : 0.0;
    }
    else
      throw iface::SRuS::SRuSException(L"Unknown operator.");

  default:
    throw iface::SRuS::SRuSException(L"Unknown operator.");
  }
}

double
SEDMLMathEvaluator::evalConstant(iface::mathml_dom::MathMLCnElement* mcne)
{
  RETURN_INTO_WSTRING(b, mcne->base());
  if (b != L"" && b != L"10")
    throw iface::SRuS::SRuSException(L"Invalid radix for constant"); // To do: bases other than 10.

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
    return s1 * std::pow(10.0, s2);
  }
  else if (t == L"rational")
  {
    double s1, s2;
    processSep(mcne, s1, s2);
    return s1 / s2;
  }
  else
    throw iface::SRuS::SRuSException(L"Unknown / unsupported constant type."); // complex or constant not supported (yet).
}

double
SEDMLMathEvaluator::evalVariable(iface::mathml_dom::MathMLCiElement* mcie)
{
  std::wstring v(stringValueOf(mcie));
  std::map<std::wstring, double>::iterator i = mVariableValues.find(stringValueOf(mcie));
  if (i == mVariableValues.end())
    throw iface::SRuS::SRuSException(L"Reference to variable that couldn't be found."); // variable not found.

  return (*i).second;
}

double
SEDMLMathEvaluator::evalPiecewise(iface::mathml_dom::MathMLPiecewiseElement* mpe)
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

double
SEDMLMathEvaluator::evalPredefined(iface::mathml_dom::MathMLPredefinedSymbol* mpds)
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
    return std::exp(1.0);

  throw iface::SRuS::SRuSException(L"Unknown predefined symbol."); // no such predefined symbol
}

double
SEDMLMathEvaluator::evalAggregate(iface::mathml_dom::MathMLCsymbolElement* aOp, iface::mathml_dom::MathMLApplyElement* aApply)
{
  throw NeedsAggregate();
}

void
SEDMLMathEvaluator::setVariable(const std::wstring& aName, double aValue)
{
  std::map<std::wstring, double>::iterator i = mVariableValues.find(aName);
  if (i == mVariableValues.end())
    mVariableValues.insert(std::pair<std::wstring, double>(aName, aValue));
  else
    (*i).second = aValue;
}

SEDMLMathEvaluatorWithAggregate::SEDMLMathEvaluatorWithAggregate(const std::map<std::wstring, std::vector<double> >& aHistory)
  : mHistory(aHistory.begin(), aHistory.end())
{
}

double
SEDMLMathEvaluatorWithAggregate::evalAggregate
(iface::mathml_dom::MathMLCsymbolElement* aOp,
 iface::mathml_dom::MathMLApplyElement* aApply)
{
  if (aApply->nArguments() != 2)
    throw iface::SRuS::SRuSException(L"Invalid number of arguments to aggregate operator");
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
    throw iface::SRuS::SRuSException(L"Unknown aggregate operator.");

  std::map<std::wstring, double> vvback(mVariableValues.begin(), mVariableValues.end());
  if (mHistory.size() == 0)
    throw iface::SRuS::SRuSException(L"No history found"); // Should be impossible?
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
