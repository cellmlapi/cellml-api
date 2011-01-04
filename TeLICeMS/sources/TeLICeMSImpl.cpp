#include "TeLICeMSImpl.hxx"
#include "CellMLBootstrap.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include <sstream>

#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"

class CDA_TeLICeMService
  : public iface::cellml_services::TeLICeMService
{
public:
  CDA_TeLICeMService()
    : _cda_refcount(1) {}

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::TeLICeMService);

  iface::cellml_services::TeLICeMModelResult* parseModel(const wchar_t* aModelText)
    throw();
  iface::cellml_services::TeLICeMMathResult* parseMaths(iface::dom::Document* aDoc, const wchar_t* aMathText)
    throw();
  wchar_t* showModel(iface::cellml_api::Model* aModel)
    throw();
  wchar_t* showMaths(iface::mathml_dom::MathMLContentElement* aMaths)
    throw();
};

int telicem_parse(
                  TeLICeMStateScan *aLexer,
                  TeLICeMSParseTarget* aParseTarget
                 );

iface::cellml_services::TeLICeMModelResult*
CDA_TeLICeMService::parseModel(const wchar_t* aModelText)
  throw()
{
  RETURN_INTO_OBJREF(cbs, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());

  // Note: by default, we create a CellML 1.0 model. If there is a need for a
  //       CellML 1.1 model (i.e. there is an import statement), then the model
  //       will be converted to CellML 1.1 as part of the importstart rule
  RETURN_INTO_OBJREF(m, iface::cellml_api::Model, cbs->createModel(L"1.0"));

  TeLICeMSParseCellML parseCellML(m);
  RETURN_INTO_OBJREF(res, CDA_TeLICeMModelResult, parseCellML.result());

  size_t l = wcslen(aModelText);
  char* s = new char[l + 1];
  if (wcstombs(s, aModelText, l + 1) != l)
  {
    delete [] s;
    res->addMessage(L"The input model text could not be converted to UTF8.");
    res->add_ref();
    return res;
  }
  std::stringstream str(s);
  delete [] s;

  TeLICeMStateScan lex(&str, &parseCellML);
  
  int ret = telicem_parse(&lex, &parseCellML);

  if (ret == 2)
    res->addMessage(L"There was insufficient memory to complete the parse.");
  else if (ret == 1)
    res->addMessage(L"Parsing failed due to a syntax error (see above).");

  res->add_ref();
  return res;
}

iface::cellml_services::TeLICeMMathResult*
CDA_TeLICeMService::parseMaths(iface::dom::Document* aDoc, const wchar_t* aMathText)
  throw()
{
  RETURN_INTO_OBJREF(el, iface::dom::Element,
                     aDoc->createElementNS(L"http://www.w3.org/1998/Math/MathML",
                                           L"math"));
  DECLARE_QUERY_INTERFACE_OBJREF(mel, el, mathml_dom::MathMLMathElement);

  TeLICeMSParseMathML parseMath(mel);
  RETURN_INTO_OBJREF(res, CDA_TeLICeMMathResult, parseMath.result());

  size_t l = wcslen(aMathText);
  char* s = new char[l + 1];
  if (wcstombs(s, aMathText, l + 1) != l)
  {
    delete [] s;
    res->addMessage(L"The input math text could not be converted to UTF8.");
    res->add_ref();
    return res;
  }
  std::stringstream str(s);
  delete [] s;

  TeLICeMStateScan lex(&str, &parseMath);
  
  int ret = telicem_parse(&lex, &parseMath);

  if (ret == 2)
    res->addMessage(L"There was insufficient memory to complete the parse.");
  else if (ret == 1)
    res->addMessage(L"Parsing failed due to a syntax error (see above).");

  res->add_ref();
  return res;
}

// Note: only works for valid CellML identifiers.
static bool
IsBuiltin(const std::wstring& aStr)
{
  static const wchar_t* idents[26][7] =
    {
      {L"and", L"as", NULL}, {L"base", L"between", NULL},
      {L"case", L"comp", NULL}, {L"def", L"dir", L"dvar", NULL},
      {L"else", L"endcomp", L"enddef", L"endrole", L"endvar", L"expo", NULL},
      {L"for", NULL}, {L"group", NULL}, {NULL},
      {L"import", L"incl", L"init", NULL}, {NULL}, {NULL}, {NULL},
      {L"map", L"math", L"model", L"mult", NULL}, {L"name", L"namespace", L"not", NULL},
      {L"off", L"or", NULL}, {L"piecewise", L"pref", L"priv", L"pub", NULL},
      {NULL}, {L"react", L"role", NULL}, {L"stoichio", NULL},
      {L"then", L"type", NULL}, {L"unit", NULL}, {L"var", L"vars", NULL},
      {L"with", NULL}, {NULL}, {NULL}, {NULL}
    };
  const wchar_t* s = aStr.c_str();
  wchar_t c = s[0];
  if (c > L'z' || c < L'a')
    return false;
  
  const wchar_t** search = idents[c - L'a'];
  for (const wchar_t** p = search; *p != NULL; p++)
    if (!wcscmp(*p, s))
      return true;
  return false;
}

static std::wstring
ShowIdentifier(const std::wstring& aStr)
{
  if (!IsBuiltin(aStr))
    return aStr;

  return L"$" + aStr;
}

static std::wstring
ShowComponentRef(const std::wstring& indent, iface::cellml_api::ComponentRef* aCR)
{
  std::wstring txt;
  RETURN_INTO_WSTRING(compname, aCR->componentName());
  txt += indent + L"comp " + ShowIdentifier(compname);
  RETURN_INTO_OBJREF(crs, iface::cellml_api::ComponentRefSet, aCR->componentRefs());
  RETURN_INTO_OBJREF(cri, iface::cellml_api::ComponentRefIterator, crs->iterateComponentRefs());
  std::wstring incl;
  std::wstring nextIndent = indent + L"  ";
  while (true)
  {
    RETURN_INTO_OBJREF(cr, iface::cellml_api::ComponentRef, cri->nextComponentRef());
    if (cr == NULL)
      break;

    incl += ShowComponentRef(nextIndent, cr);
  }
  if (incl != L"")
    txt = txt + L" incl\n" + incl + indent + L"endcomp";
  txt += L";\n";

  return txt;
}

static std::wstring
ShowGroup(const std::wstring& indent, iface::cellml_api::Group* aGroup)
{
  std::wstring txt = indent + L"def group as\n";

  std::wstring nextIndent = indent + L"  ";

  RETURN_INTO_OBJREF(rs, iface::cellml_api::RelationshipRefSet,
                     aGroup->relationshipRefs());
  RETURN_INTO_OBJREF(ri, iface::cellml_api::RelationshipRefIterator,
                     rs->iterateRelationshipRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(rr, iface::cellml_api::RelationshipRef,
                       ri->nextRelationshipRef());
    if (rr == NULL)
      break;

    RETURN_INTO_WSTRING(rn, rr->relationship());
    txt += nextIndent + L"type " + ShowIdentifier(rn);
    RETURN_INTO_WSTRING(rnn, rr->relationshipNamespace());
    if (rnn != L"")
      txt += L" namespace \"" + rnn + L"\"";

    RETURN_INTO_WSTRING(rrn, rr->name());
    if (rrn != L"")
      txt += nextIndent + L" with name " + ShowIdentifier(rrn);

    txt += L";\n";
  }

  RETURN_INTO_OBJREF(crs, iface::cellml_api::ComponentRefSet, aGroup->componentRefs());
  RETURN_INTO_OBJREF(cri, iface::cellml_api::ComponentRefIterator, crs->iterateComponentRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(cr, iface::cellml_api::ComponentRef, cri->nextComponentRef());
    if (cr == NULL)
      break;

    txt += ShowComponentRef(nextIndent, cr);
  }

  txt += indent + L"enddef;\n";

  return txt;
}

static std::wstring
ShowImportComponent(const std::wstring& indent, iface::cellml_api::ImportComponent* aImportComponent)
{
  RETURN_INTO_WSTRING(nameThere, aImportComponent->componentRef());
  RETURN_INTO_WSTRING(nameHere, aImportComponent->name());
  return indent + L"comp " + ShowIdentifier(nameThere) + L" as " + ShowIdentifier(nameHere) + L";\n";
}

static std::wstring
ShowImportUnits(const std::wstring& indent, iface::cellml_api::ImportUnits* aImportUnits)
{
  RETURN_INTO_WSTRING(nameThere, aImportUnits->unitsRef());
  RETURN_INTO_WSTRING(nameHere, aImportUnits->name());
  return indent + L"unit " + ShowIdentifier(nameThere) + L" as " + ShowIdentifier(nameHere) + L";\n";
}

static std::wstring
ShowImport(const std::wstring& indent, iface::cellml_api::CellMLImport* aImport)
{
  std::wstring txt = indent + L"def import \"";
  RETURN_INTO_OBJREF(uri, iface::cellml_api::URI, aImport->xlinkHref());
  RETURN_INTO_WSTRING(uriStr, uri->asText());
  txt += uriStr + L"\" as\n";

  std::wstring nextIndent = indent + L"  ";
  RETURN_INTO_OBJREF(comps, iface::cellml_api::ImportComponentSet, aImport->components());
  RETURN_INTO_OBJREF(compi, iface::cellml_api::ImportComponentIterator, comps->iterateImportComponents());
  while (true)
  {
    RETURN_INTO_OBJREF(comp, iface::cellml_api::ImportComponent, compi->nextImportComponent());
    if (comp == NULL)
      break;

    txt += ShowImportComponent(nextIndent, comp);
  }

  RETURN_INTO_OBJREF(unitss, iface::cellml_api::ImportUnitsSet, aImport->units());
  RETURN_INTO_OBJREF(unitsi, iface::cellml_api::ImportUnitsIterator, unitss->iterateImportUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(units, iface::cellml_api::ImportUnits, unitsi->nextImportUnits());
    if (units == NULL)
      break;

    txt += ShowImportUnits(nextIndent, units);
  }

  txt += indent + L"enddef;\n";

  return txt;
}

static const wchar_t* siPrefixes[] = {
  L"yocto", L"zepto", L"atto", L"femto", L"pico", L"nano", L"micro", L"milli", L"BUG!",
  L"kilo", L"mega", L"giga", L"tera", L"peta", L"exa", L"zetta", L"yotta"
};

static std::wstring
ShowUnit(const std::wstring& indent, iface::cellml_api::Unit* aUnit)
{
  RETURN_INTO_WSTRING(u, aUnit->units());
  std::wstring txt = indent + L"unit " + ShowIdentifier(u);
  std::wstring attrs;

  long prefix = aUnit->prefix();
  if (prefix != 0)
  {
    attrs += L"pref:";
    if (prefix > -3 && prefix < 3)
    {
      if (prefix == -2)
        attrs += L"centi";
      else if (prefix == -1)
        attrs += L"deci";
      else if (prefix == 1)
        attrs += L"deca";
      else
        attrs += L"hecto";
    }
    else if (prefix > 24 || prefix < -24 || prefix % 3)
    {
      wchar_t buf[40];
      swprintf(buf, 40, L"%d", prefix);
      attrs += buf;
    }
    else // non-zero, multiple of 3 between -24 and 24 inclusive.
      attrs += siPrefixes[prefix/3 + 8];
  }

  double expo = aUnit->exponent();
  if (expo != 0.0)
  {
    if (attrs != L"")
      attrs += L", ";
    wchar_t buf[30];
    swprintf(buf, 30, L"%g", expo);
    attrs = attrs + L"expo:" + buf;
  }
  
  double mult = aUnit->multiplier();
  if (mult != 0.0)
  {
    if (attrs != L"")
      attrs += L", ";

    wchar_t buf[30];
    swprintf(buf, 30, L"%g", mult);
    attrs = attrs + L"mult:" + buf;
  }

  double offs = aUnit->offset();
  if (offs != 0.0)
  {
    if (attrs != L"")
      attrs += L", ";

    wchar_t buf[30];
    swprintf(buf, 30, L"%g", offs);
    attrs = attrs + L"off:" + buf;
  }

  if (attrs != L"")
  {
    txt += L"{" + attrs + L"}";
  }

  txt += L";\n";

  return txt;
}

static std::wstring
ShowUnits(const std::wstring& indent, iface::cellml_api::Units* aUnits)
{
  std::wstring txt = indent + L"def unit ";
  RETURN_INTO_WSTRING(name, aUnits->name());
  txt += ShowIdentifier(name) + L" as ";
  if (aUnits->isBaseUnits())
    txt += L"base unit";
  else
  {
    txt += L"def\n";
    std::wstring nextIndent = indent + L"  ";

    RETURN_INTO_OBJREF(us, iface::cellml_api::UnitSet, aUnits->unitCollection());
    RETURN_INTO_OBJREF(ui, iface::cellml_api::UnitIterator, us->iterateUnits());
    while (true)
    {
      RETURN_INTO_OBJREF(u, iface::cellml_api::Unit, ui->nextUnit());
      if (u == NULL)
        break;

      txt += ShowUnit(nextIndent, u);
    }

    txt += indent + L"enddef";
  }
  txt += L";\n";

  return txt;
}

static std::wstring
ShowVariable(const std::wstring& indent, iface::cellml_api::CellMLVariable* aVariable)
{
  RETURN_INTO_WSTRING(name, aVariable->name());
  RETURN_INTO_WSTRING(units, aVariable->unitsName());
  std::wstring txt = indent + L"var " + ShowIdentifier(name) + L" : " +
    ShowIdentifier(units);
  RETURN_INTO_WSTRING(ivStr, aVariable->initialValue());

  std::wstring params = L"";
  bool needComma = false;
  if (ivStr != L"")
  {
    params += L"init: " + ivStr;
    needComma = true;
  }

  iface::cellml_api::VariableInterface i = aVariable->publicInterface();
  if (i != iface::cellml_api::INTERFACE_NONE)
  {
    if (needComma)
      params += L", ";
    else
      needComma = true;

    params += L"pub: ";
    if (i == iface::cellml_api::INTERFACE_IN)
      params += L"in";
    else if (i == iface::cellml_api::INTERFACE_OUT)
      params += L"out";
  }

  i = aVariable->privateInterface();
  if (i != iface::cellml_api::INTERFACE_NONE)
  {
    if (needComma)
      params += L", ";
    params += L"priv: ";
    if (i == iface::cellml_api::INTERFACE_IN)
      params += L"in";
    else if (i == iface::cellml_api::INTERFACE_OUT)
      params += L"out";
  }

  if (params != L"")
    txt += L" { " + params + L" }";

  txt += L";\n";
  return txt;
}

struct OperatorInformation
{
  const wchar_t* mathmlName, * telicemName;
  uint32_t expressionPrec, argPrec;
  enum ProcessType {
    PREORDER, SIMPLE_INORDER, DERIVATIVE_LIKE
  } processType;
} operInfo[] = {
  {L"abs",              L"abs",              9, 0, OperatorInformation::PREORDER},
  {L"and",              L"and",              2, 2, OperatorInformation::SIMPLE_INORDER},
  {L"approx",           L"approx",           9, 0, OperatorInformation::PREORDER},
  {L"arccos",           L"arccos",           9, 0, OperatorInformation::PREORDER},
  {L"arccosh",          L"arccosh",          9, 0, OperatorInformation::PREORDER},
  {L"arccot",           L"arccot",           9, 0, OperatorInformation::PREORDER},
  {L"arccoth",          L"arccoth",          9, 0, OperatorInformation::PREORDER},
  {L"arccsc",           L"arccsc",           9, 0, OperatorInformation::PREORDER},
  {L"arccsch",          L"arccsch",          9, 0, OperatorInformation::PREORDER},
  {L"arcsec",           L"arcsec",           9, 0, OperatorInformation::PREORDER},
  {L"arcsech",          L"arcsech",          9, 0, OperatorInformation::PREORDER},
  {L"arcsin",           L"arcsin",           9, 0, OperatorInformation::PREORDER},
  {L"arcsinh",          L"arcsinh",          9, 0, OperatorInformation::PREORDER},
  {L"arctan",           L"arctan",           9, 0, OperatorInformation::PREORDER},
  {L"arctanh",          L"arctanh",          9, 0, OperatorInformation::PREORDER},
  {L"arg",              L"arg",              9, 0, OperatorInformation::PREORDER},
  {L"card",             L"card",             9, 0, OperatorInformation::PREORDER},
  {L"cartesianproduct", L"cartesianproduct", 9, 0, OperatorInformation::PREORDER},
  {L"ceiling",          L"ceiling",          9, 0, OperatorInformation::PREORDER},
  {L"conjugate",        L"conjugate",        9, 0, OperatorInformation::PREORDER},
  {L"cos",              L"cos",              9, 0, OperatorInformation::PREORDER},
  {L"cosh",             L"cosh",             9, 0, OperatorInformation::PREORDER},
  {L"cot",              L"cot", 9, 0, OperatorInformation::PREORDER},
  {L"coth",             L"coth", 9, 0, OperatorInformation::PREORDER},
  {L"csc",              L"csc", 9, 0, OperatorInformation::PREORDER},
  {L"csch",             L"csch", 9, 0, OperatorInformation::PREORDER},
  {L"curl",             L"curl",             9, 0, OperatorInformation::PREORDER},
  {L"determinant",      L"determinant", 9, 0, OperatorInformation::PREORDER},
  {L"diff",             L"d", 6, 0, OperatorInformation::DERIVATIVE_LIKE},
  {L"divergence",       L"divergence", 10, 0, OperatorInformation::PREORDER},
  {L"divide",           L"/", 4, 4, OperatorInformation::SIMPLE_INORDER},
  {L"eq",               L"=", 5, 5, OperatorInformation::SIMPLE_INORDER},
  {L"equivalent",       L"equivalent", 10, 0, OperatorInformation::PREORDER},
  {L"exists",           L"exists", 10, 0, OperatorInformation::PREORDER},
  {L"exp",              L"exp", 10, 0, OperatorInformation::PREORDER},
  {L"factorial",        L"factorial", 10, 0, OperatorInformation::PREORDER},
  {L"factorof",         L"factorof", 10, 0, OperatorInformation::PREORDER},
  {L"floor",            L"floor", 10, 0, OperatorInformation::PREORDER},
  {L"forall",           L"forall", 10, 0, OperatorInformation::PREORDER},
  {L"gcd",              L"gcd", 10, 0, OperatorInformation::PREORDER},
  {L"geq",              L"geq", 10, 0, OperatorInformation::PREORDER},
  {L"grad",             L"grad", 10, 0, OperatorInformation::PREORDER},
  {L"gt",               L">", 5, 5, OperatorInformation::SIMPLE_INORDER},
  {L"imaginary",        L"imaginary", 10, 0, OperatorInformation::PREORDER},
  {L"implies",          L"implies", 10, 0, OperatorInformation::PREORDER},
  {L"in",               L"in", 10, 0, OperatorInformation::PREORDER},
  {L"int",              L"int", 10, 0, OperatorInformation::PREORDER},
  {L"intersect",        L"intersect", 10, 0, OperatorInformation::PREORDER},
  {L"laplacian",        L"laplacian", 10, 0, OperatorInformation::PREORDER},
  {L"lcm",              L"lcm", 10, 0, OperatorInformation::PREORDER},
  {L"leq",              L"<=", 5, 5, OperatorInformation::SIMPLE_INORDER},
  {L"list",             L"list", 10, 0, OperatorInformation::PREORDER},
  {L"ln",               L"ln", 10, 0, OperatorInformation::PREORDER},
  {L"log",              L"log", 10, 0, OperatorInformation::PREORDER},
  {L"lowlimit",         L"lowlimit", 10, 0, OperatorInformation::PREORDER},
  {L"lt",               L"<", 5, 5, OperatorInformation::SIMPLE_INORDER},
  {L"max",              L"max", 10, 0, OperatorInformation::PREORDER},
  {L"mean",             L"mean", 10, 0, OperatorInformation::PREORDER},
  {L"median",           L"median", 10, 0, OperatorInformation::PREORDER},
  {L"min",              L"min", 10, 0, OperatorInformation::PREORDER},
  {L"minus",            L"-", 3, 3, OperatorInformation::SIMPLE_INORDER},
  {L"mode",             L"mode", 10, 0, OperatorInformation::PREORDER},
  {L"moment",           L"moment", 10, 0, OperatorInformation::PREORDER},
  {L"neq",              L"!=", 5, 5, OperatorInformation::SIMPLE_INORDER},
  {L"not",              L"not", 10, 0, OperatorInformation::PREORDER},
  {L"notin",            L"notin", 10, 0, OperatorInformation::PREORDER},
  {L"notprsubset",      L"notprsubset", 10, 0, OperatorInformation::PREORDER},
  {L"notsubset",        L"notsubset", 10, 0, OperatorInformation::PREORDER},
  {L"or",               L"or", 1, 1, OperatorInformation::SIMPLE_INORDER},
  {L"outerproduct",     L"outerproduct", 10, 0, OperatorInformation::PREORDER},
  {L"partialdiff",      L"partialdiff", 10, 0, OperatorInformation::DERIVATIVE_LIKE},
  {L"plus",             L"+", 3, 3, OperatorInformation::SIMPLE_INORDER},
  {L"power",            L"power", 10, 0, OperatorInformation::PREORDER},
  {L"product",          L"product", 10, 0, OperatorInformation::PREORDER},
  {L"prsubset",         L"prsubset", 10, 0, OperatorInformation::PREORDER},
  {L"quotient",         L"quotient", 10, 0, OperatorInformation::PREORDER},
  {L"real",             L"real", 10, 0, OperatorInformation::PREORDER},
  {L"rem",              L"rem", 10, 0, OperatorInformation::PREORDER},
  {L"root",             L"root", 10, 0, OperatorInformation::PREORDER},
  {L"scalarproduct",    L"scalarproduct", 10, 0, OperatorInformation::PREORDER},
  {L"sdev",             L"sdev", 10, 0, OperatorInformation::PREORDER},
  {L"sec",              L"sec", 10, 0, OperatorInformation::PREORDER},
  {L"sech",             L"sech", 10, 0, OperatorInformation::PREORDER},
  {L"selector",         L"selector", 10, 0, OperatorInformation::PREORDER},
  {L"set",              L"set", 10, 0, OperatorInformation::PREORDER},
  {L"setdiff",          L"setdiff", 10, 0, OperatorInformation::PREORDER},
  {L"sin",              L"sin", 10, 0, OperatorInformation::PREORDER},
  {L"sinh",             L"sinh", 10, 0, OperatorInformation::PREORDER},
  {L"subset",           L"subset", 10, 0, OperatorInformation::PREORDER},
  {L"sum",              L"sum", 10, 0, OperatorInformation::PREORDER},
  {L"tan",              L"tan", 10, 0, OperatorInformation::PREORDER},
  {L"tanh",             L"tanh", 10, 0, OperatorInformation::PREORDER},
  {L"times",            L"*", 4, 4, OperatorInformation::SIMPLE_INORDER},
  {L"transpose",        L"transpose", 10, 0, OperatorInformation::PREORDER},
  {L"union",            L"union", 10, 0, OperatorInformation::PREORDER},
  {L"uplimit",          L"uplimit", 10, 0, OperatorInformation::PREORDER},
  {L"variance",         L"variance", 10, 0, OperatorInformation::PREORDER},
  {L"vectorproduct",    L"vectorproduct", 10, 0, OperatorInformation::PREORDER},
  {L"xor",              L"xor", 10, 0, OperatorInformation::PREORDER}
};

OperatorInformation*
LookupOperatorByMathMLName(const wchar_t* aOpName)
{
  size_t lowerBound = 0, upperBound = sizeof(operInfo) / sizeof(OperatorInformation) - 1;
  while (upperBound > lowerBound)
  {
    size_t guess = (lowerBound + upperBound) / 2;
    int ret = wcscmp(aOpName, operInfo[guess].mathmlName);
    if (ret == 0)
      return operInfo + guess;
    if (ret < 0)
      upperBound = guess - 1;
    else
      lowerBound = guess + 1;
  }

  if (upperBound == lowerBound && !wcscmp(aOpName, operInfo[lowerBound].mathmlName))
    return operInfo + lowerBound;

  return NULL;
}

static std::wstring
StripWhitespace(const std::wstring& aStr)
{
  int i, j;
  for (i = 0; i < static_cast<int>(aStr.length()); i++)
    if (aStr[i] != ' ' && aStr[i] != '\t' && aStr[i] != '\r' &&
        aStr[i] != '\n')
      break;
  for (j = aStr.length() - 1; j >= i; j--)
    if (aStr[j] != ' ' && aStr[j] != '\t' && aStr[j] != '\r' &&
        aStr[j] != '\n')
      break;

  return aStr.substr(i, j - i + 1);
}

static std::wstring
ShowAttributes(const std::map<std::wstring, std::wstring>& aAttrMap)
{
  std::wstring txt;
  bool more = false;

  for (std::map<std::wstring, std::wstring>::const_iterator i = aAttrMap.begin();
       i != aAttrMap.end(); i++)
  {
    if (more)
      txt += L",";
    else
      more = true;

    txt += (*i).first + L": \"" + (*i).second + L"\"";
  }

  if (txt != L"")
    return L"{" + txt + L"}";
  return txt;
}

static void
AddContentElementAttributes(iface::mathml_dom::MathMLContentElement* aMaths,
                            std::map<std::wstring, std::wstring> & aAttrs)
{
#define CopyOneAttribute(x) \
  RETURN_INTO_WSTRING(x, aMaths->x()); \
  if (x != L"") \
    aAttrs.insert(std::pair<std::wstring, std::wstring>(L## #x, x));
  CopyOneAttribute(className);
  CopyOneAttribute(mathElementStyle);
  CopyOneAttribute(id);
  CopyOneAttribute(xref);
  CopyOneAttribute(href);
}

static void
AddContentTokenAttributes(iface::mathml_dom::MathMLContentToken* aMaths,
                          std::map<std::wstring, std::wstring> & aAttrs)
{
  CopyOneAttribute(definitionURL);
  CopyOneAttribute(encoding);
}

static std::wstring
ShowMathExpression(std::wstring aIndent, iface::mathml_dom::MathMLContentElement* aMaths,
                   uint32_t aBracketPrec)
{
  std::wstring txt;

  DECLARE_QUERY_INTERFACE_OBJREF(mae, aMaths, mathml_dom::MathMLApplyElement);
  if (mae == NULL)
  {
    // Perhaps it's a ci element...
    DECLARE_QUERY_INTERFACE_OBJREF(mce, aMaths, mathml_dom::MathMLCiElement);
    if (mce == NULL)
    {
      // Maybe a constant (cn)?
      DECLARE_QUERY_INTERFACE_OBJREF(mcne, aMaths, mathml_dom::MathMLCnElement);
      if (mcne == NULL)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(mpe, aMaths, mathml_dom::MathMLPiecewiseElement);
        if (mpe == NULL)
        {
          // Perhaps a MathML predefined element?
          DECLARE_QUERY_INTERFACE_OBJREF(mpds, aMaths, mathml_dom::MathMLPredefinedSymbol);
          if (mpds == NULL)
            // We can't handle this - just say unknown-element for now.
            txt += L"unknown-element";
          else
          {
            RETURN_INTO_WSTRING(sName, mpds->symbolName());
            txt += sName;
            std::map<std::wstring, std::wstring> attrs;
            RETURN_INTO_WSTRING(du, mpds->definitionURL());
            if (du != L"")
              attrs.insert(std::pair<std::wstring, std::wstring>(L"definitionURL", du));
            RETURN_INTO_WSTRING(enc, mpds->encoding());
            if (enc != L"")
              attrs.insert(std::pair<std::wstring, std::wstring>(L"encoding", enc));
            AddContentElementAttributes(mpds, attrs);
            txt += ShowAttributes(attrs);
          }
        }
        else
        {
          txt += L"piecewise";
          std::map<std::wstring, std::wstring> attrs;
          AddContentElementAttributes(mpe, attrs);
          txt += ShowAttributes(attrs);

          txt += L"(\n";
          uint32_t i;
          std::wstring newIndent = aIndent + L"  ";
          for (i = 1; ; i++)
          {
            RETURN_INTO_OBJREF(caseEl, iface::mathml_dom::MathMLCaseElement,
                               mpe->getCase(i));
            
            if (caseEl == NULL)
              break;
            RETURN_INTO_OBJREF(caseValue, iface::mathml_dom::MathMLContentElement,
                               caseEl->caseValue());
            RETURN_INTO_OBJREF(caseCond, iface::mathml_dom::MathMLContentElement,
                               caseEl->caseCondition());
            txt += newIndent + L"case ";
            std::map<std::wstring, std::wstring> cattrs;
            AddContentElementAttributes(caseEl, cattrs);
            txt += ShowAttributes(cattrs);
            
            txt += ShowMathExpression(newIndent, caseCond, 0) + L" then " +
                   ShowMathExpression(newIndent, caseValue, 0) + L"\n";
          }
          RETURN_INTO_OBJREF(owmce, iface::mathml_dom::MathMLContentElement, mpe->otherwise());
          // TODO: get attributes off parent ('otherwise' element itself)?
          if (owmce != NULL)
            txt += newIndent + L"else " + ShowMathExpression(newIndent, owmce, 0) + L"\n";
          txt += aIndent + L")";
        }
      }
      else
      {
        RETURN_INTO_WSTRING(constType, mcne->type());

        RETURN_INTO_OBJREF(mantTextN, iface::dom::Node, mcne->getArgument(1));
        DECLARE_QUERY_INTERFACE_OBJREF(tn, mantTextN, dom::Text);
        RETURN_INTO_WSTRING(mantv, tn->data());
        RETURN_INTO_OBJREF(expTextN, iface::dom::Node, mcne->getArgument(2));

        txt += StripWhitespace(mantv);

        if (expTextN)
        {
          DECLARE_QUERY_INTERFACE_OBJREF(tn2, expTextN, dom::Text);
          RETURN_INTO_WSTRING(expv, tn2->data());
          txt += L"E" + StripWhitespace(expv);
        }

        std::map<std::wstring, std::wstring> attrs;
        if (constType != L"")
          attrs.insert(std::pair<std::wstring, std::wstring>(L"type", constType));
        RETURN_INTO_WSTRING(basev, mcne->getAttributeNS(L"", L"base"));
        if (basev != L"")
          attrs.insert(std::pair<std::wstring, std::wstring>(L"base", basev));
        RETURN_INTO_WSTRING(units, mcne->getAttributeNS(CELLML_1_0_NS, L"units"));
        if (units == L"")
        {
          RETURN_INTO_WSTRING(units1, mcne->getAttributeNS(CELLML_1_1_NS, L"units"));
          units = units1;
        }
        attrs.insert(std::pair<std::wstring, std::wstring>(L"unit", units));

        AddContentTokenAttributes(mcne, attrs);
        txt += ShowAttributes(attrs);
      }
    }
    else
    {
      std::map<std::wstring, std::wstring> attrs;
      RETURN_INTO_OBJREF(mceT, iface::dom::Node, mce->getArgument(1));
      DECLARE_QUERY_INTERFACE_OBJREF(tn, mceT, dom::Text);
      RETURN_INTO_WSTRING(mceS, tn->data());
      
      txt += ShowIdentifier(StripWhitespace(mceS));

      RETURN_INTO_WSTRING(type, mce->type());
      if (type != L"")
        attrs.insert(std::pair<std::wstring, std::wstring>(L"type", type));

      AddContentTokenAttributes(mce, attrs);
      txt += ShowAttributes(attrs);
    }
  }
  else
  {
    // We now have an apply element...

    RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                       mae->_cxx_operator());
    
    DECLARE_QUERY_INTERFACE_OBJREF(pds, op, mathml_dom::MathMLPredefinedSymbol);

    if (pds == NULL)
      txt += L"apply-unknown-operator";
    else
    {
      RETURN_INTO_WSTRING(sName, pds->symbolName());
      // Look up the symbol...
      OperatorInformation* op = LookupOperatorByMathMLName(sName.c_str());
      if (op == NULL)
      {
        txt += L"apply-unknown-operator";
      }
      else
      {
        std::map<std::wstring, std::wstring> attrs;
        RETURN_INTO_WSTRING(du, pds->definitionURL());
        if (du != L"")
          attrs.insert(std::pair<std::wstring, std::wstring>(L"definitionURL", du));
        RETURN_INTO_WSTRING(enc, pds->encoding());
        if (enc != L"")
          attrs.insert(std::pair<std::wstring, std::wstring>(L"encoding", enc));
        txt += ShowAttributes(attrs);
        AddContentElementAttributes(mae, attrs);
        std::wstring attrStr = ShowAttributes(attrs);

        if (op->expressionPrec < aBracketPrec)
          txt += L"(";

        uint32_t n = mae->nArguments();

        OperatorInformation::ProcessType pt = op->processType;

        if (n == 1 && !wcscmp(L"-", op->telicemName))
          pt = OperatorInformation::PREORDER;

        switch (pt)
        {
        case OperatorInformation::PREORDER:
          {
            txt += op->telicemName;
            txt += attrStr;
            txt += L"(";
            for (uint32_t i = 2; i <= n; i++)
            {
              RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, mae->getArgument(i));
              DECLARE_QUERY_INTERFACE_OBJREF(content, arg, mathml_dom::MathMLContentElement);
              txt += ShowMathExpression(aIndent, content, op->argPrec);
              if (i != n)
                txt += L", ";
            }
            txt += L")";
          }
          break;

        case OperatorInformation::SIMPLE_INORDER:
          {
            for (uint32_t i = 2; i <= n; i++)
            {
              RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, mae->getArgument(i));
              DECLARE_QUERY_INTERFACE_OBJREF(content, arg, mathml_dom::MathMLContentElement);
              txt += ShowMathExpression(aIndent, content, op->argPrec);
              if (i != n)
                txt += (std::wstring(L" ")) + op->telicemName + attrStr + L" ";
            }
          }
          break;
          
        case OperatorInformation::DERIVATIVE_LIKE:
          {
            RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement, mae->getArgument(2));
            DECLARE_QUERY_INTERFACE_OBJREF(content, arg, mathml_dom::MathMLContentElement);
            RETURN_INTO_OBJREF(bvar, iface::mathml_dom::MathMLElement, mae->getBoundVariable(1));
            DECLARE_QUERY_INTERFACE_OBJREF(bcontent, bvar, mathml_dom::MathMLContentElement);
            
            txt += std::wstring(op->telicemName) + L"(" + ShowMathExpression(aIndent, content, op->argPrec) +
              L")/" + op->telicemName + L"(" + ShowMathExpression(aIndent, bcontent, op->argPrec) + L")";
            txt += attrStr;
          }
          break;
        }

        if (op->expressionPrec < aBracketPrec)
          txt += L")";
      }
    }
  }

  return txt;
}

static std::wstring
ShowMathElementWithDef(const std::wstring& indent,
                       iface::mathml_dom::MathMLMathElement* aMaths)
{
  std::wstring txt = indent + L"def math as\n";

  std::wstring newIndent = indent + L"  ";

  uint32_t na = aMaths->nArguments();
  for (uint32_t i = 1; i <= na; i++)
  {
    RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                       aMaths->getArgument(i));
    DECLARE_QUERY_INTERFACE_OBJREF(mce, arg, mathml_dom::MathMLContentElement);
    txt += newIndent + ShowMathExpression(newIndent, mce, 0) + L";\n";
  }

  txt += indent + L"enddef;\n";
  return txt;
}

static std::wstring
ShowRole(const std::wstring& indent, iface::cellml_api::Role* aRole)
{
  std::wstring txt = indent + L"role ";
  iface::cellml_api::Role::RoleType rt = aRole->variableRole();
  switch (rt)
  {
  case iface::cellml_api::Role::REACTANT:
    txt += L"reactant";
    break;
  case iface::cellml_api::Role::PRODUCT:
    txt += L"product";
    break;
  case iface::cellml_api::Role::RATE:
    txt += L"rate";
    break;
  case iface::cellml_api::Role::CATALYST:
    txt += L"catalyst";
    break;
  case iface::cellml_api::Role::ACTIVATOR:
    txt += L"activator";
    break;
  case iface::cellml_api::Role::INHIBITOR:
    txt += L"inhibitor";
    break;
  case iface::cellml_api::Role::MODIFIER:
    txt += L"modifier";
    break;
  }
  
  std::wstring attrs;
  iface::cellml_api::Role::DirectionType dir = aRole->direction();
  if (dir != iface::cellml_api::Role::FORWARD)
  {
    attrs += L"dir: ";
    if (dir == iface::cellml_api::Role::REVERSE)
      attrs += L"reverse";
    else
      attrs += L"both";
  }

  double stoich = aRole->stoichiometry();
  if (stoich != 1.0)
  {
    if (attrs != L"")
      attrs += L", ";
    wchar_t buf[40];
    swprintf(buf, 40, L"%g", stoich);
    attrs += L"stoichio: ";
    attrs += buf;
  }

  RETURN_INTO_WSTRING(deltaVarName, aRole->deltaVariableName());
  if (deltaVarName != L"")
  {
    if (attrs != L"")
      attrs += L", ";
    attrs += L"dvar: " + ShowIdentifier(deltaVarName);
  }

  if (attrs != L"")
    txt += L"{ " + attrs + L" }";

  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathList, aRole->math());
  if (ml->length() > 0)
  {
    txt += L" with\n";
    std::wstring nextIndent = indent + L"  ";
    RETURN_INTO_OBJREF(mei, iface::cellml_api::MathMLElementIterator, ml->iterate());
    while (true)
    {
      RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, mei->next());
      if (me == NULL)
        break;
      DECLARE_QUERY_INTERFACE_OBJREF(mme, me, mathml_dom::MathMLMathElement);

      if (mme != NULL)
        txt += ShowMathElementWithDef(nextIndent, mme);
    }
    txt += indent + L"endrole";
  }
  
  txt += L";\n";

  return txt;
}

static std::wstring
ShowVariableRef(const std::wstring& indent, iface::cellml_api::VariableRef* aRef)
{
  RETURN_INTO_WSTRING(vname, aRef->variableName());
  std::wstring txt = indent + L"var " + vname + L" with\n";
  std::wstring nextIndent = indent + L"  ";
  RETURN_INTO_OBJREF(rs, iface::cellml_api::RoleSet, aRef->roles());
  RETURN_INTO_OBJREF(ri, iface::cellml_api::RoleIterator, rs->iterateRoles());
  while (true)
  {
    RETURN_INTO_OBJREF(r, iface::cellml_api::Role, ri->nextRole());
    if (r == NULL)
      break;

    txt += ShowRole(nextIndent, r);
  }
  txt += indent + L"endvar;\n";

  return txt;
}

static std::wstring
ShowReaction(const std::wstring& indent, iface::cellml_api::Reaction* aReaction)
{
  std::wstring txt = indent + L"def ";
  if (aReaction->reversible())
    txt += L"rev";
  else
    txt += L"irrev";
  txt += L" reaction for\n";

  std::wstring nextIndent = indent + L"  ";
  RETURN_INTO_OBJREF(vrs, iface::cellml_api::VariableRefSet, aReaction->variableReferences());
  RETURN_INTO_OBJREF(vri, iface::cellml_api::VariableRefIterator, vrs->iterateVariableRefs());
  while (true)
  {
    RETURN_INTO_OBJREF(vr, iface::cellml_api::VariableRef, vri->nextVariableRef());
    if (vr == NULL)
      break;

    txt += ShowVariableRef(nextIndent, vr);
  }

  txt += indent + L"enddef;\n";
  return txt;
}

static std::wstring
ShowComponent(const std::wstring& indent, iface::cellml_api::CellMLComponent* aComponent)
{
  std::wstring txt = indent + L"def comp ";
  RETURN_INTO_WSTRING(name, aComponent->name());
  txt += ShowIdentifier(name) + L" as\n";

  std::wstring nextIndent = indent + L"  ";
  RETURN_INTO_OBJREF(cvs, iface::cellml_api::CellMLVariableSet, aComponent->variables());
  RETURN_INTO_OBJREF(cvi, iface::cellml_api::CellMLVariableIterator, cvs->iterateVariables());
  while (true)
  {
    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, cvi->nextVariable());
    if (cv == NULL)
      break;
    txt += ShowVariable(nextIndent, cv);
  }

  RETURN_INTO_OBJREF(cus, iface::cellml_api::UnitsSet, aComponent->units());
  RETURN_INTO_OBJREF(cui, iface::cellml_api::UnitsIterator, cus->iterateUnits());
  while (true)
  {
    RETURN_INTO_OBJREF(cu, iface::cellml_api::Units, cui->nextUnits());
    if (cu == NULL)
      break;
    txt += ShowUnits(nextIndent, cu);
  }

  RETURN_INTO_OBJREF(rs, iface::cellml_api::ReactionSet, aComponent->reactions());
  RETURN_INTO_OBJREF(ri, iface::cellml_api::ReactionIterator, rs->iterateReactions());
  while (true)
  {
    RETURN_INTO_OBJREF(r, iface::cellml_api::Reaction, ri->nextReaction());
    if (r == NULL)
      break;
    txt += ShowReaction(nextIndent, r);
  }

  RETURN_INTO_OBJREF(ml, iface::cellml_api::MathList, aComponent->math());
  RETURN_INTO_OBJREF(mei, iface::cellml_api::MathMLElementIterator, ml->iterate());
  while (true)
  {
    RETURN_INTO_OBJREF(me, iface::mathml_dom::MathMLElement, mei->next());
    if (me == NULL)
      break;
    DECLARE_QUERY_INTERFACE_OBJREF(mme, me, mathml_dom::MathMLMathElement);
    
    if (mme != NULL)
      txt += ShowMathElementWithDef(nextIndent, mme);
  }
  
  txt += indent + L"enddef;\n";

  return txt;
}

static std::wstring
ShowMapVariables(const std::wstring& indent, iface::cellml_api::MapVariables* aMapVars)
{
  RETURN_INTO_WSTRING(fvn, aMapVars->firstVariableName());
  RETURN_INTO_WSTRING(svn, aMapVars->secondVariableName());

  return indent + L"vars " + fvn + L" and " + svn + L";\n";
}

static std::wstring
ShowConnection(const std::wstring& indent, iface::cellml_api::Connection* aConnection)
{
  std::wstring txt;

  txt += indent + L"def map between ";
  RETURN_INTO_OBJREF(cms, iface::cellml_api::MapComponents,
                     aConnection->componentMapping());
  RETURN_INTO_WSTRING(fcn, cms->firstComponentName());
  RETURN_INTO_WSTRING(scn, cms->secondComponentName());
  txt += fcn + L" and " + scn + L" for\n";
  
  std::wstring nextIndent = indent + L"  ";
  RETURN_INTO_OBJREF(vms, iface::cellml_api::MapVariablesSet,
                     aConnection->variableMappings());
  RETURN_INTO_OBJREF(vmi, iface::cellml_api::MapVariablesIterator,
                     vms->iterateMapVariables());
  while (true)
  {
    RETURN_INTO_OBJREF(mv, iface::cellml_api::MapVariables, vmi->nextMapVariables());
    if (mv == NULL)
      break;

    txt += ShowMapVariables(nextIndent, mv);
  }

  txt += indent + L"enddef;\n";

  return txt;
}

static std::wstring
ShowModel(const std::wstring& indent, iface::cellml_api::Model* aModel)
{
  std::wstring txt = indent + L"def model ";
  RETURN_INTO_WSTRING(n, aModel->name());
  txt += ShowIdentifier(n);
  txt += L" as\n";
  std::wstring nextIndent = indent + L"  ";

  {
    RETURN_INTO_OBJREF(gs, iface::cellml_api::GroupSet,
                       aModel->groups());
    RETURN_INTO_OBJREF(gi, iface::cellml_api::GroupIterator,
                       gs->iterateGroups());
    while (true)
    {
      RETURN_INTO_OBJREF(g, iface::cellml_api::Group,
                         gi->nextGroup());
      if (g == NULL)
        break;
      txt += ShowGroup(nextIndent, g);
    }
  }

  {
    RETURN_INTO_OBJREF(is, iface::cellml_api::CellMLImportSet,
                       aModel->imports());
    RETURN_INTO_OBJREF(ii, iface::cellml_api::CellMLImportIterator,
                       is->iterateImports());
    while (true)
    {
      RETURN_INTO_OBJREF(i, iface::cellml_api::CellMLImport,
                         ii->nextImport());
      if (i == NULL)
        break;
      txt += ShowImport(nextIndent, i);
    }
  }

  {
    RETURN_INTO_OBJREF(us, iface::cellml_api::UnitsSet,
                       aModel->localUnits());
    RETURN_INTO_OBJREF(ui, iface::cellml_api::UnitsIterator,
                       us->iterateUnits());
    while (true)
    {
      RETURN_INTO_OBJREF(u, iface::cellml_api::Units,
                         ui->nextUnits());
      if (u == NULL)
        break;
      txt += ShowUnits(nextIndent, u);
    }
  }

  {
    RETURN_INTO_OBJREF(cs, iface::cellml_api::CellMLComponentSet,
                       aModel->localComponents());
    RETURN_INTO_OBJREF(ci, iface::cellml_api::CellMLComponentIterator,
                       cs->iterateComponents());
    while (true)
    {
      RETURN_INTO_OBJREF(c, iface::cellml_api::CellMLComponent,
                         ci->nextComponent());
      if (c == NULL)
        break;
      txt += ShowComponent(nextIndent, c);
    }
  }

  {
    RETURN_INTO_OBJREF(cs, iface::cellml_api::ConnectionSet,
                       aModel->connections());
    RETURN_INTO_OBJREF(ci, iface::cellml_api::ConnectionIterator,
                       cs->iterateConnections());
    while (true)
    {
      RETURN_INTO_OBJREF(c, iface::cellml_api::Connection,
                         ci->nextConnection());
      if (c == NULL)
        break;
      txt += ShowConnection(nextIndent, c);
    }
  }
  txt += indent + L"enddef;\n";

  return txt;
}

wchar_t*
CDA_TeLICeMService::showModel(iface::cellml_api::Model* aModel)
  throw()
{
  std::wstring txt = ShowModel(L"", aModel);
  return CDA_wcsdup(txt.c_str());
}

wchar_t*
CDA_TeLICeMService::showMaths(iface::mathml_dom::MathMLContentElement* aMaths)
  throw()
{
  std::wstring txt = ShowMathExpression(L"", aMaths, 0);
  return CDA_wcsdup(txt.c_str());
}

TeLICeMStateScan::TeLICeMStateScan(std::stringstream* aStr,
                                   TeLICeMSParseTarget* aTarg)
  : TeLICeMFlexLexer(aStr), mTarget(aTarg), mEOF(false)
{
}

void
TeLICeMStateScan::adjustColumn()
{
  mTarget->mColumn += yyleng;
}

void
TeLICeMStateScan::newRow()
{
  mTarget->mColumn = 0;
  mTarget->mRow++;
}

void
TeLICeMStateScan::lexerError(const char* m)
{
  mTarget->mResult->addMessage(convertStringToWString(m));
  wchar_t buf[80];
  swprintf(buf, 80, L"at line %d, column %d", mTarget->mRow, mTarget->mColumn);
  mTarget->mResult->addMessage(buf);
}

int telicem_lex(TeLICeMSLValue* aLValue, TeLICeMStateScan* aLexer)
{
  aLexer->mLValue = aLValue;

  int val;
  while (true)
  {
    while (true)
    {
      val = aLexer->yylex();
      
      if (val == MAGIC_NEWLINE)
        aLexer->newRow();
      else
        break;
    }
    aLexer->adjustColumn();

    if (val != MAGIC_WHITESPACE)
      break;
  }

  aLexer->mEOF = (val == 0);

  return val;
}

iface::cellml_services::TeLICeMService*
CreateTeLICeMService(void)
{
  return new CDA_TeLICeMService();
}
