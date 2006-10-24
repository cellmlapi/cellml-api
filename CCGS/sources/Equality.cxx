#include "Equality.hxx"
#include "CodeGenerationState.hxx"
#include <set>
#include <sstream>
#include <iomanip>

void
TrimString(std::wstring& aStr)
{
  uint32_t wsFront = 0;
  uint32_t l = aStr.size();
  while (wsFront < l && wcschr(L" \t\r\n", aStr[wsFront]))
    wsFront++;
  if (wsFront == l)
  {
    aStr = L"";
    return;
  }
  aStr = aStr.substr(wsFront);
  l = aStr.size() - 1;
  while (l > 0 && wcschr(L" \t\r\n", aStr[l]))
    l--;
  aStr = aStr.substr(0, l + 1);
}

Equation::Equation()
  : _cda_refcount(1), mTriggersNewtonRaphson(false)
{
}

Equation::~Equation()
{
  while (!equal.empty())
  {
    equal.front().second->release_ref();
    equal.pop_front();
  }
}


void
Equation::AddPart(iface::cellml_api::CellMLComponent* aComp,
                  iface::mathml_dom::MathMLElement* aEl)
{
  // See if the part is an <apply><diff/>...</apply>...
  DECLARE_QUERY_INTERFACE_OBJREF(apply, aEl, mathml_dom::MathMLApplyElement);
  if (apply != NULL)
  {
    RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                       apply->_cxx_operator());
    DECLARE_QUERY_INTERFACE_OBJREF(oppds, op,
                                   mathml_dom::MathMLPredefinedSymbol);
    if (oppds != NULL)
    {
      RETURN_INTO_WSTRING(sn, oppds->symbolName());
      if (sn == L"diff")
      {
        mDiff = aEl;
        if (mDiffCI)
          throw CodeGenerationError
            (L"Equating two or more differentials not supported");
        if (apply->nArguments() != 2)
          throw CodeGenerationError
            (L"Expected diff operator to take exactly one argument.");
        RETURN_INTO_OBJREF(diffVar, iface::mathml_dom::MathMLElement,
                           apply->getArgument(2));
        DECLARE_QUERY_INTERFACE_OBJREF(diffCI, diffVar,
                                       mathml_dom::MathMLCiElement);
        if (diffCI == NULL)
          throw CodeGenerationError
            (L"Expected diff operator argument to be a ci element.");
        mDiffCI = diffCI;

        if (apply->nBoundVariables() != 1)
          throw CodeGenerationError
            (L"Expected diff operator to have exactly 1 bound variable.");
        RETURN_INTO_OBJREF(bv, iface::mathml_dom::MathMLBvarElement,
                           apply->getBoundVariable(1));
        if (bv->nArguments() != 1)
          throw CodeGenerationError
            (L"Expected bvar to have exactly one argument.");
        RETURN_INTO_OBJREF(bvel, iface::mathml_dom::MathMLElement,
                           bv->getArgument(1));
        DECLARE_QUERY_INTERFACE_OBJREF(bvci, bvel,
                                       mathml_dom::MathMLCiElement);
        mBoundCI = bvci;
        return;
      }
    }
  }

  // aComp is on the component list, but need to add_ref aEl...
  aEl->add_ref();
  equal.push_back(std::pair<iface::cellml_api::CellMLComponent*,
                            iface::mathml_dom::MathMLElement*>(aComp, aEl));
}

bool
Equation::AllowsComputationFrom(std::list<VariableInformation*>& vi)
{
  return true;
}

static struct OpInfo
{
  const wchar_t* opName;
  enum
  {
    UNARY, /* One arg, prestring value poststring */
    BINARY, /* Two args, prestring value1 sepstring value2 poststring */
    /* n args, prestring value1 sepstring value2 ... sepstring valuen
     * poststring */
    N_ARY,
    /* n args, prestring n sepstring value1 sepstring value2 ... sepstring
     * valuen poststring */
    N_ARY_WITH_COUNT,
    /* n args, prestring value1 sepstring value2 default value2 sepstring
     * value3 ... default valuen-1 sepstring valuen poststring */
    N_ARY_CHAIN,
    /* prestring sepstring value poststring or
     * prestring value1 sepstring value2 poststring */
    UNARY_OR_BINARY,
    /* prestring value sepstring degree poststring or
     * prestring value sepstring default poststring */
    UNARY_WITH_DEGREE,
    /* prestring value sepstring logbase poststring or
     * prestring value sepstring default poststring */
    UNARY_WITH_LOGBASE,
    /* Creates a new function with the subexpression in it, and then writes
     * defint(funcptr, BOUND, CONSTANTS, RATES, VARIABLES, bvar_idx)
     */
    DEFINITE_INTEGRAL
  } opType;
  const char* prestring;
  const char* sepstring;
  const char* poststring;
#define NEVER "This code is never generated, report a bug if seen."
#define INORDER(x) "(",x,")"
#define FUNC(x) x "(",",",")"
#define FUNC_INV(x) "(1.0 / " x "(",",","))"
#define FUNC_ARG_INV(x) x "(1.0 / ", NEVER, ")"
  // The default for the root etc...
  const char* defaultValue;
  // The significance of ordering...
  enum
  {
    INSIGNIFICANT,
    SIGNIFICANT
  } Order;
  // The minimum number of children to make it worth storing results...
  uint32_t storeCutoff;
} OperatorInformation[] =
{
  {L"abs", OpInfo::UNARY, FUNC("fabs"), "", OpInfo::INSIGNIFICANT, 0},
  {L"and", OpInfo::N_ARY, INORDER("&&"), "", OpInfo::INSIGNIFICANT, 3},
  {L"arccos", OpInfo::UNARY, FUNC("acos"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arccosh", OpInfo::UNARY, FUNC("acosh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arccot", OpInfo::UNARY, FUNC_ARG_INV("atan"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arccoth", OpInfo::UNARY, FUNC_ARG_INV("atanh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arccsc", OpInfo::UNARY, FUNC_ARG_INV("asin"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arccsch", OpInfo::UNARY, FUNC_ARG_INV("asinh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arcsec", OpInfo::UNARY, FUNC_ARG_INV("acos"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arcsech", OpInfo::UNARY, FUNC_ARG_INV("acosh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arcsin", OpInfo::UNARY, FUNC("asin"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arcsinh", OpInfo::UNARY, FUNC("asinh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arctan", OpInfo::UNARY, FUNC("atan"), "", OpInfo::INSIGNIFICANT, 0},
  {L"arctanh", OpInfo::UNARY, FUNC("atanh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"ceiling", OpInfo::UNARY, FUNC("ceil"), "", OpInfo::INSIGNIFICANT, 0},
  {L"cos", OpInfo::UNARY, FUNC("cos"), "", OpInfo::INSIGNIFICANT, 0},
  {L"cosh", OpInfo::UNARY, FUNC("cosh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"cot", OpInfo::UNARY, FUNC_INV("tan"), "", OpInfo::INSIGNIFICANT, 0},
  {L"coth", OpInfo::UNARY, FUNC_INV("tanh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"csc", OpInfo::UNARY, FUNC_INV("sin"), "", OpInfo::INSIGNIFICANT, 0},
  {L"csch", OpInfo::UNARY, FUNC_INV("sinh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"divide",OpInfo::BINARY,INORDER("/"), "", OpInfo::SIGNIFICANT, 0},
  {L"eq", OpInfo::N_ARY_CHAIN, "((", "==", "))", ") && (", OpInfo::INSIGNIFICANT, 3},
  {L"exp", OpInfo::UNARY, FUNC("exp"), "", OpInfo::INSIGNIFICANT, 0},
  {L"factorial", OpInfo::UNARY, FUNC("factorial"), "", OpInfo::SIGNIFICANT, 0},
  {L"factorof", OpInfo::BINARY, "((((int)", ")%((int)", ")) == 0)", "", OpInfo::SIGNIFICANT, 0},
  {L"floor", OpInfo::UNARY, FUNC("floor"), "", OpInfo::INSIGNIFICANT, 0},
  {L"gcd", OpInfo::N_ARY_WITH_COUNT, FUNC("gcd_multi"), "", OpInfo::INSIGNIFICANT, 0},
  {L"geq", OpInfo::N_ARY_CHAIN, "((", ">=", "))", ") && (", OpInfo::SIGNIFICANT, 3},
  {L"gt", OpInfo::N_ARY_CHAIN, "((", ">", "))", ") && (", OpInfo::SIGNIFICANT, 3},
  {L"implies", OpInfo::BINARY, "(!(", ") || ", ")", "", OpInfo::SIGNIFICANT, 0},
  {L"int", OpInfo::DEFINITE_INTEGRAL, "", "", "", "", OpInfo::SIGNIFICANT, 0},
  {L"lcm", OpInfo::N_ARY_WITH_COUNT, FUNC("lcm_multi"), "", OpInfo::INSIGNIFICANT, 0},
  {L"leq", OpInfo::N_ARY_CHAIN, "((", "<=", "))", ") && (", OpInfo::SIGNIFICANT, 3},
  {L"ln", OpInfo::UNARY, FUNC("log"), "", OpInfo::INSIGNIFICANT, 0},
  {L"log", OpInfo::UNARY_WITH_LOGBASE, FUNC("arbitrary_log"), "10", OpInfo::INSIGNIFICANT, 0},
  {L"lt", OpInfo::N_ARY_CHAIN, "((", "<", "))", ") && (", OpInfo::SIGNIFICANT, 3},
  {L"max", OpInfo::N_ARY_WITH_COUNT, FUNC("multi_max"), "", OpInfo::INSIGNIFICANT, 0},
  {L"min", OpInfo::N_ARY_WITH_COUNT, FUNC("multi_min"), "", OpInfo::SIGNIFICANT, 0},
  {L"minus", OpInfo::UNARY_OR_BINARY, "(", "-", ")", "", OpInfo::SIGNIFICANT, 2},
  {L"neq", OpInfo::BINARY, INORDER("!="), "", OpInfo::INSIGNIFICANT, 3},
  {L"not", OpInfo::UNARY, "!(", NEVER, ")", "", OpInfo::INSIGNIFICANT, 2},
  {L"or", OpInfo::N_ARY, INORDER("||"), "", OpInfo::INSIGNIFICANT, 3},
  {L"plus", OpInfo::N_ARY, INORDER("+"), "", OpInfo::INSIGNIFICANT, 3},
  {L"power", OpInfo::BINARY, FUNC("pow"), "", OpInfo::SIGNIFICANT, 0},
  {L"quotient", OpInfo::BINARY, "(((int)", ") / ((int)", "))", "", OpInfo::SIGNIFICANT, 0},
  {L"rem", OpInfo::BINARY, "(((int)", ") % ((int)", "))", "", OpInfo::SIGNIFICANT, 0},
  {L"root", OpInfo::UNARY_WITH_DEGREE, "pow(", ", 1.0 / (", "))", "2.0", OpInfo::SIGNIFICANT, 0},
  {L"sec", OpInfo::UNARY, FUNC_INV("cos"), "", OpInfo::INSIGNIFICANT, 0},
  {L"sech", OpInfo::UNARY, FUNC_INV("cosh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"sin", OpInfo::UNARY, FUNC("sin"), "", OpInfo::INSIGNIFICANT, 0},
  {L"sinh", OpInfo::UNARY, FUNC("sinh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"tan", OpInfo::UNARY, FUNC("tan"), "", OpInfo::INSIGNIFICANT, 0},
  {L"tanh", OpInfo::UNARY, FUNC("tanh"), "", OpInfo::INSIGNIFICANT, 0},
  {L"times", OpInfo::N_ARY, INORDER("*"), "", OpInfo::INSIGNIFICANT, 3},
  {L"xor", OpInfo::N_ARY, INORDER("^"), "", OpInfo::INSIGNIFICANT, 3}
};
#define OPERATOR_TABLE_SIZE (sizeof(OperatorInformation)/sizeof(OperatorInformation[0]))

#define CONTEXT_DIFF 1
#define CONTEXT_BVAR 2
#define CONTEXT_INT 4
#define CONTEXT_DEEP 8
#define CONTEXT_DIFFBVAR (CONTEXT_DIFF|CONTEXT_BVAR)
#define CONTEXT_INTBVAR (CONTEXT_INT|CONTEXT_BVAR)

iface::cellml_api::CellMLVariable*
FindVariableForCI(iface::cellml_api::CellMLComponent* aComp,
                  iface::mathml_dom::MathMLCiElement* aCi)
{
  // We hit a variable. Get the name...
  RETURN_INTO_OBJREF(n, iface::dom::Node, aCi->getArgument(1));
  if (n == NULL)
    throw CodeGenerationError(L"Found an empty ci");
  RETURN_INTO_WSTRING(v, n->nodeValue());
  TrimString(v);

  // Ask the component for the variable...
  RETURN_INTO_OBJREF(vs, iface::cellml_api::CellMLVariableSet,
                     aComp->variables());
  iface::cellml_api::CellMLVariable* var = vs->getVariable(v.c_str());

  if (var == NULL)
  {
    std::wstring emsg = L"Variable ";
    emsg += v;
    emsg += L" is referenced in mathematics, but was not found in the "
      L"component.";
    throw CodeGenerationError(emsg);
  }

  return var;
}

bool
RecursivelyCountUnknown
(
 CodeGenerationState* aCGS,
 iface::cellml_api::CellMLComponent* aComponent,
 iface::mathml_dom::MathMLElement* aElement,
 std::set<VariableInformation*>& aAvailable,
 std::set<VariableInformation*>& aWanted,
 uint32_t& wantedCount,
 ObjRef<iface::mathml_dom::MathMLCiElement>& aWantedCi,
 VariableInformation*& aWantedVI,
 uint32_t contextFlags
)
{
  DECLARE_QUERY_INTERFACE_OBJREF(ci, aElement, mathml_dom::MathMLCiElement);
  if (ci != NULL)
  {
    RETURN_INTO_OBJREF(var, iface::cellml_api::CellMLVariable,
                       FindVariableForCI(aComponent, ci));
    VariableInformation* vi = aCGS->FindOrAddVariableInformation(var);

    if (aWanted.count(vi))
    {
      if (wantedCount)
      {
        // See if it is the same variable...
        char* id1 = vi->GetSourceVariable()->objid();
        char* id2 = aWantedVI->GetSourceVariable()->objid();
        int cmp = strcmp(id1, id2);
        free(id1);
        free(id2);
        if (cmp)
          return false;
      }
      wantedCount++;
      aWantedCi = ci;
      aWantedVI = vi;
    }
    else if (aAvailable.count(vi) == 0)
      return false;
    return true;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(apply, aElement,
                                 mathml_dom::MathMLApplyElement);
  if (apply != NULL)
  {
    uint32_t n = apply->nArguments();
    if (n <= 1)
      throw CodeGenerationError(L"Don't know how to deal with apply with one "
                                L"or fewer arguments only.");
    RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                       apply->_cxx_operator());
    DECLARE_QUERY_INTERFACE_OBJREF(oppd, op,
                                   mathml_dom::MathMLPredefinedSymbol);
    if (oppd == NULL)
      throw CodeGenerationError(L"You can only apply a built-in function.");
    RETURN_INTO_WSTRING(sn, oppd->symbolName());
    // See if it is a diff or int...
    if (sn == L"diff")
      contextFlags |= CONTEXT_DIFF;
    else if (sn == L"int")
      contextFlags |= CONTEXT_INT;
    uint32_t nb = apply->nBoundVariables(), i;
    for (i = 1; i <= nb; i++)
    {
      RETURN_INTO_OBJREF(bve, iface::mathml_dom::MathMLBvarElement,
                         apply->getBoundVariable(i));
      if (!RecursivelyCountUnknown(aCGS, aComponent, bve, aAvailable,
                                   aWanted, wantedCount, aWantedCi,
                                   aWantedVI, contextFlags))
        return false;
    }

    for (i = 1; i <= n; i++)
    {
      RETURN_INTO_OBJREF(a, iface::mathml_dom::MathMLElement,
                         apply->getArgument(i));
      if (!RecursivelyCountUnknown(aCGS, aComponent, a, aAvailable,
                                   aWanted, wantedCount, aWantedCi,
                                   aWantedVI, contextFlags))
        return false;
    }

    // Also, need to go into uplimit, lowlimit, degree, and logbase...
    RETURN_INTO_OBJREF(nl, iface::dom::NodeList, apply->childNodes());
    n = nl->length();
    for (i = 0; i < n; i++)
    {
      RETURN_INTO_OBJREF(it, iface::dom::Node, nl->item(i));
      if (it == NULL)
        break;
      RETURN_INTO_WSTRING(nsuri, it->namespaceURI());
      RETURN_INTO_WSTRING(ln, it->localName());
      if (nsuri != L"http://www.w3.org/1998/Math/MathML")
        continue;
      if (ln != L"uplimit" && ln != L"lowlimit" && ln != L"degree" &&
          ln != L"logbase")
        continue;
      RETURN_INTO_OBJREF(nl2, iface::dom::NodeList, it->childNodes());
      uint32_t j, m = nl2->length();
      for (j = 0; j < m; j++)
      {
        RETURN_INTO_OBJREF(it2, iface::dom::Node, nl2->item(j));
        if (it2 == NULL)
          break;
        DECLARE_QUERY_INTERFACE_OBJREF(it2el, it2, mathml_dom::MathMLElement);
        if (it2el == NULL)
          continue;
        if (!RecursivelyCountUnknown(aCGS, aComponent, it2el, aAvailable,
                                     aWanted, wantedCount, aWantedCi,
                                     aWantedVI, contextFlags))
          return false;
      }
    }

    return true;
  }

  // It could also be a bvar...
  DECLARE_QUERY_INTERFACE_OBJREF(bv, aElement,
                                 mathml_dom::MathMLBvarElement);
  if (bv != NULL)
  {
    uint32_t n = bv->nArguments();
    if (n != 1)
      throw CodeGenerationError(L"Don't know how to deal with bvar with "
                                L"anything other than one argument.");
    RETURN_INTO_OBJREF(bvc, iface::mathml_dom::MathMLElement,
                       bv->getArgument(1));
    DECLARE_QUERY_INTERFACE_OBJREF(bvci, bvc, mathml_dom::MathMLCiElement);
    if (bvci == NULL)
      throw CodeGenerationError(L"Cannot cope with bvar children which are "
                                L"not ci elements.");
    if (!RecursivelyCountUnknown(aCGS, aComponent, bvci, aAvailable,
                                 aWanted, wantedCount, aWantedCi,
                                 aWantedVI, contextFlags | CONTEXT_BVAR))
      return false;
    return true;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(pw, aElement,
                                 mathml_dom::MathMLPiecewiseElement);
  if (pw != NULL)
  {
    RETURN_INTO_OBJREF(pl, iface::mathml_dom::MathMLNodeList,
                       pw->pieces());
    uint32_t i, l = pl->length();
    for (i = 1; i <= l; i++)
    {
      RETURN_INTO_OBJREF(p, iface::mathml_dom::MathMLCaseElement,
                         pw->getCase(i));
      RETURN_INTO_OBJREF(cc, iface::mathml_dom::MathMLElement,
                         p->caseCondition());
      RETURN_INTO_OBJREF(cv, iface::mathml_dom::MathMLElement,
                         p->caseValue());
      if (!RecursivelyCountUnknown(aCGS, aComponent, cc, aAvailable,
                                   aWanted, wantedCount, aWantedCi,
                                   aWantedVI, contextFlags))
        return false;
      if (!RecursivelyCountUnknown(aCGS, aComponent, cv, aAvailable,
                                   aWanted, wantedCount, aWantedCi,
                                   aWantedVI, contextFlags))
        return false;
    }
    try
    {
      RETURN_INTO_OBJREF(ow, iface::mathml_dom::MathMLElement,
                         pw->otherwise());
      if (ow != NULL)
      {
        if (!RecursivelyCountUnknown(aCGS, aComponent, ow, aAvailable,
                                     aWanted, wantedCount, aWantedCi,
                                     aWantedVI, contextFlags))
          return false;
      }
    }
    catch (iface::dom::DOMException& de) {}
  }
  return true;
}

OpInfo*
GetOperatorInformation(iface::mathml_dom::MathMLPredefinedSymbol* aSymbol)
{
  wchar_t* sn = aSymbol->symbolName();
  uint32_t upper = OPERATOR_TABLE_SIZE - 1, lower = 0;
  OpInfo* ret = NULL;
  do
  {
    uint32_t mid = (upper + lower) / 2;
    int cmp = wcscmp(OperatorInformation[mid].opName, sn);
    if (cmp == 0)
    {
      ret = OperatorInformation + mid;
      break;
    }
    else if (upper == lower)
      break;
    else if (cmp < 0)
      lower = mid + 1;
    else
    {
      if (mid == lower)
        break;
      upper = mid - 1;
    }
  }
  while (true);
  free(sn);
  return ret;
}

void
GenerateExpression
(
 CodeGenerationState* aCGS,
 iface::cellml_api::CellMLComponent* aComponent,
 iface::mathml_dom::MathMLElement* aExpr,
 std::stringstream& expression,
 std::stringstream& supplementaryFunctions,
 bool aHaveBound
)
{
  {
    // Is it a CI...
    DECLARE_QUERY_INTERFACE_OBJREF(ci, aExpr, mathml_dom::MathMLCiElement);
    if (ci != NULL)
    {
      RETURN_INTO_OBJREF(vloc, iface::cellml_api::CellMLVariable,
                         FindVariableForCI(aComponent, ci));
      VariableInformation* svi = aCGS->FindOrAddVariableInformation(vloc);
      // Get conversion factor information...
      double offset;
      double factor = aCGS->GetConversion(svi->GetSourceVariable(), vloc,
                                          offset);

      if (offset != 0.0)
        expression << "(";
      if (factor != 1.0)
        expression << "(";

      switch (svi->GetArray())
      {
      case VariableInformation::INDEPENDENT:
        expression << "BOUND[" << svi->GetIndex() << "]";
        break;
      case VariableInformation::DEPENDENT_AND_RATE:
      case VariableInformation::DEPENDENT:
        expression << "VARIABLES[" << svi->GetIndex() << "]";
        break;
      case VariableInformation::CONSTANT:
        expression << "CONSTANTS[" << svi->GetIndex() << "]";
        break;
      }

      if (factor != 1.0)
        expression << ") * " << std::setiosflags(std::ios_base::showpoint) << factor;
      if (offset != 0.0)
        expression << ") + " << std::setiosflags(std::ios_base::showpoint) << offset;
      return;
    }
  }

  {
    // Maybe a constant?
    DECLARE_QUERY_INTERFACE_OBJREF(cn, aExpr, mathml_dom::MathMLCnElement);
    if (cn != NULL)
    {
      RETURN_INTO_WSTRING(units,
                          cn->getAttributeNS(L"http://www.cellml.org/cellml/1.0#",
                                             L"units"));
      if (units == L"")
      {
        wchar_t* tmp = cn->getAttributeNS(L"http://www.cellml.org/cellml/1.1#",
                                          L"units");
        units = tmp;
        free(tmp);
        if (units == L"")
          throw CodeGenerationError(L"Found a MathML cn with no units "
                                    L"(or units in wrong namespace).");
      }
      // Check the validity of the units...
      if (!aCGS->UnitsValid(aComponent, units))
      {
        RETURN_INTO_WSTRING(compn, aComponent->name());
        std::wstring aMsg = L"Units ";
        aMsg += units;
        aMsg += L" are not defined in the context of component ";
        aMsg += compn;
        aMsg += L" but are used as the units on a constant.";
        throw CodeGenerationError(aMsg);
      }

      if (cn->nargs() != 1)
      {
        std::wstring aMsg = L"The code generator can only handle cn elements"
          L"with exactly one part. The error occurred while processing "
          L"component ";
        RETURN_INTO_WSTRING(compn, aComponent->name());
        aMsg += compn;
        aMsg += L".";
        throw CodeGenerationError(aMsg);
      }

      RETURN_INTO_OBJREF(cnn, iface::dom::Node, cn->getArgument(1));
      DECLARE_QUERY_INTERFACE_OBJREF(tn, cnn, dom::Text);
      if (tn == NULL)
      {
        std::wstring aMsg = L"The content of a <cn> element must be text. "
          L"The error occurred while processing component ";
        RETURN_INTO_WSTRING(compn, aComponent->name());
        aMsg += compn;
        aMsg += L".";
        throw CodeGenerationError(aMsg);
      }
      RETURN_INTO_WSTRING(cd, tn->data());
      TrimString(cd);
      const wchar_t* str1;
      wchar_t* str2;
      str1 = cd.c_str();
      double val = wcstod(str1, &str2);
      if (*str2 != 0)
      {
        std::wstring aMsg = L"The content of a <cn> element must be "
          L"a floating point number within the ranges allowed for an IEEE754"
          L"double precision numbers. The error occured while processing "
          L"component ";
        RETURN_INTO_WSTRING(compn, aComponent->name());
        aMsg += compn;
        aMsg += L".";
        throw CodeGenerationError(aMsg);
      }
      expression << std::setiosflags(std::ios_base::showpoint) << val;
      return;
    }
  }

  // An apply?
  {
    DECLARE_QUERY_INTERFACE_OBJREF(apply, aExpr,
                                   mathml_dom::MathMLApplyElement);
    if (apply != NULL)
    {
      RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                         apply->_cxx_operator());
      DECLARE_QUERY_INTERFACE_OBJREF(oppd, op,
                                     mathml_dom::MathMLPredefinedSymbol);
      // TODO: Support user defined functions.
      if (oppd == NULL)
        throw CodeGenerationError(L"You can only apply a built-in function.");

      // See if we support the symbol...
      OpInfo* opi = GetOperatorInformation(oppd);
      if (opi == NULL)
      {
        std::wstring aMsg = L"Unsupported predefined operator ";
        RETURN_INTO_WSTRING(opn, oppd->symbolName());
        aMsg += opn;
        aMsg += L" while processing component ";
        RETURN_INTO_WSTRING(compn, aComponent->name());
        aMsg += compn;
        aMsg += L".";
        throw CodeGenerationError(aMsg);
      }

      expression << opi->prestring;
      
      switch (opi->opType)
      {
      case OpInfo::UNARY:
        if (apply->nArguments() != 2)
        {
          std::wstring aMsg = L"Operator ";
          RETURN_INTO_WSTRING(opn, oppd->symbolName());
          aMsg += opn;
          aMsg += L" is unary, so needs exactly one argument (in addition to "
            L"the operator). Error occurred while processing component ";
          RETURN_INTO_WSTRING(compn, aComponent->name());
          aMsg += compn;
          aMsg += L".";
          throw CodeGenerationError(aMsg);
        }
        {
          RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                             apply->getArgument(2));
          GenerateExpression(aCGS, aComponent, arg, expression,
                             supplementaryFunctions, aHaveBound);
        }
        break;
      case OpInfo::BINARY:
        if (apply->nArguments() != 3)
        {
          std::wstring aMsg = L"Operator ";
          RETURN_INTO_WSTRING(opn, oppd->symbolName());
          aMsg += opn;
          aMsg += L" is binary, so needs exactly two arguments (in addition to "
            L"the operator). Error occurred while processing component ";
          RETURN_INTO_WSTRING(compn, aComponent->name());
          aMsg += compn;
          aMsg += L".";
          throw CodeGenerationError(aMsg);
        }
        {
          RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement,
                             apply->getArgument(2));
          RETURN_INTO_OBJREF(arg2, iface::mathml_dom::MathMLElement,
                             apply->getArgument(3));
          GenerateExpression(aCGS, aComponent, arg1, expression,
                             supplementaryFunctions, aHaveBound);
          expression << opi->sepstring;
          GenerateExpression(aCGS, aComponent, arg2, expression,
                             supplementaryFunctions, aHaveBound);
        }
        break;
      case OpInfo::N_ARY:
        {
          uint32_t curarg, maxarg = apply->nArguments();
          for (curarg = 2; curarg <= maxarg; curarg++)
          {
            if (curarg > 2)
              expression << opi->sepstring;
            RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                               apply->getArgument(curarg));
            GenerateExpression(aCGS, aComponent, arg, expression,
                               supplementaryFunctions, aHaveBound);
          }
        }
        break;
      case OpInfo::N_ARY_WITH_COUNT:
        {
          uint32_t curarg, maxarg = apply->nArguments();
          expression << (maxarg - 1);
          for (curarg = 2; curarg <= maxarg; curarg++)
          {
            expression << opi->sepstring;
            RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                               apply->getArgument(curarg));
            GenerateExpression(aCGS, aComponent, arg, expression,
                               supplementaryFunctions, aHaveBound);
          }
        }
        break;
      case OpInfo::N_ARY_CHAIN:
        {
          uint32_t curarg, maxarg = apply->nArguments();
          if (maxarg == 2)
          {
            std::wstring aMsg = L"Operator ";
            RETURN_INTO_WSTRING(opn, oppd->symbolName());
            aMsg += opn;
            aMsg += L" is needs more than one argument (in addition to "
              L"the operator). Error occurred while processing component ";
            RETURN_INTO_WSTRING(compn, aComponent->name());
            aMsg += compn;
            aMsg += L".";
            throw CodeGenerationError(aMsg);
          }
          for (curarg = 2; curarg < maxarg; curarg++)
          {
            if (curarg > 2)
              expression << opi->defaultValue;
            RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement,
                               apply->getArgument(curarg));
            RETURN_INTO_OBJREF(arg2, iface::mathml_dom::MathMLElement,
                               apply->getArgument(curarg + 1));
            GenerateExpression(aCGS, aComponent, arg1, expression,
                               supplementaryFunctions, aHaveBound);
            expression << opi->sepstring;
            GenerateExpression(aCGS, aComponent, arg2, expression,
                               supplementaryFunctions, aHaveBound);
          }
        }
        break;
      case OpInfo::UNARY_OR_BINARY:
        {
          if (apply->nArguments() == 2)
          {
            RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                               apply->getArgument(2));
            expression << opi->sepstring;
            GenerateExpression(aCGS, aComponent, arg, expression,
                               supplementaryFunctions, aHaveBound);
          }
          else if (apply->nArguments() == 3)
          {
            RETURN_INTO_OBJREF(arg1, iface::mathml_dom::MathMLElement,
                               apply->getArgument(2));
            RETURN_INTO_OBJREF(arg2, iface::mathml_dom::MathMLElement,
                               apply->getArgument(3));
            GenerateExpression(aCGS, aComponent, arg1, expression,
                               supplementaryFunctions, aHaveBound);
            expression << opi->sepstring;
            GenerateExpression(aCGS, aComponent, arg2, expression,
                               supplementaryFunctions, aHaveBound);
          }
          else
          {
            std::wstring aMsg = L"Operator ";
            RETURN_INTO_WSTRING(opn, oppd->symbolName());
            aMsg += opn;
            aMsg += L" needs one or two arguments (in addition to "
            L"the operator). Error occurred while processing component ";
            RETURN_INTO_WSTRING(compn, aComponent->name());
            aMsg += compn;
            aMsg += L".";
            throw CodeGenerationError(aMsg);
          }
        }
        break;
      case OpInfo::UNARY_WITH_DEGREE:
        {
          // It is unfortunate that we have to do this, but...
          RETURN_INTO_OBJREF(nl, iface::dom::NodeList, apply->childNodes());
          ObjRef<iface::mathml_dom::MathMLElement> degree_expr;
          uint32_t ac = nl->length(), ai;
          for (ai = 0; ai < ac; ai++)
          {
            RETURN_INTO_OBJREF(dn, iface::dom::Node, nl->item(ai));
            if (dn == NULL)
              break;
            DECLARE_QUERY_INTERFACE_OBJREF(el, dn, dom::Element);
            if (el == NULL)
              continue;
            RETURN_INTO_WSTRING(nsuri, el->namespaceURI());
            RETURN_INTO_WSTRING(ln, el->localName());
            if (nsuri == L"http://www.w3.org/1998/Math/MathML")
              continue;
            if (ln != L"degree")
              continue;
            nl = already_AddRefd<iface::dom::NodeList>(el->childNodes());
            ac = nl->length();
            for (ai = 0; ai < ac; ai++)
            {
              RETURN_INTO_OBJREF(deg, iface::dom::Node, nl->item(ai));
              if (dn == NULL)
                break;
              DECLARE_QUERY_INTERFACE(degree_expr, deg,
                                      mathml_dom::MathMLElement);
              if (degree_expr != NULL)
                break;
            }
            break;
          }

          RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                             apply->getArgument(2));
          GenerateExpression(aCGS, aComponent, arg, expression,
                             supplementaryFunctions, aHaveBound);
          expression << opi->sepstring;
          if (degree_expr == NULL)
            expression << opi->defaultValue;
          else
          {
            GenerateExpression(aCGS, aComponent, degree_expr, expression,
                               supplementaryFunctions, aHaveBound);
          }
        }
        break;
      case OpInfo::UNARY_WITH_LOGBASE:
        {
          // It is unfortunate that we have to do this, but...
          RETURN_INTO_OBJREF(nl, iface::dom::NodeList, apply->childNodes());
          ObjRef<iface::mathml_dom::MathMLElement> logbase_expr;
          uint32_t ac = nl->length(), ai;
          for (ai = 0; ai < ac; ai++)
          {
            RETURN_INTO_OBJREF(dn, iface::dom::Node, nl->item(ai));
            if (dn == NULL)
              break;
            DECLARE_QUERY_INTERFACE_OBJREF(el, dn, dom::Element);
            if (el == NULL)
              continue;
            RETURN_INTO_WSTRING(nsuri, el->namespaceURI());
            RETURN_INTO_WSTRING(ln, el->localName());
            if (nsuri == L"http://www.w3.org/1998/Math/MathML")
              continue;
            if (ln != L"logbase")
              continue;
            nl = already_AddRefd<iface::dom::NodeList>(el->childNodes());
            ac = nl->length();
            for (ai = 0; ai < ac; ai++)
            {
              RETURN_INTO_OBJREF(deg, iface::dom::Node, nl->item(ai));
              if (dn == NULL)
                break;
              DECLARE_QUERY_INTERFACE(logbase_expr, deg,
                                      mathml_dom::MathMLElement);
              if (logbase_expr != NULL)
                break;
            }
            break;
          }

          RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                             apply->getArgument(2));
          GenerateExpression(aCGS, aComponent, arg, expression,
                             supplementaryFunctions, aHaveBound);
          expression << opi->sepstring;
          if (logbase_expr == NULL)
            expression << opi->defaultValue;
          else
          {
            GenerateExpression(aCGS, aComponent, logbase_expr, expression,
                               supplementaryFunctions, aHaveBound);
          }
        }
        break;
      case OpInfo::DEFINITE_INTEGRAL:
        {
          if (apply->nArguments() != 2)
          {
            std::wstring aMsg = L"Operator ";
            RETURN_INTO_WSTRING(opn, oppd->symbolName());
            aMsg += opn;
            aMsg += L" is unary, so needs exactly one argument (in addition "
              L"to the operator). Error occurred while processing component ";
            RETURN_INTO_WSTRING(compn, aComponent->name());
            aMsg += compn;
            aMsg += L".";
            throw CodeGenerationError(aMsg);
          }
          // Find the bound variable...
          if (apply->nBoundVariables() != 1)
            throw CodeGenerationError
              (
               L"Only one bound variable per int application is supported. If "
               L"you are attempting to write a multiple integral, you should "
               L"use multiple nested <apply><int/> constructs, each with its "
               L"own uplimit & lowlimit."
              );

          RETURN_INTO_OBJREF(bvar, iface::mathml_dom::MathMLBvarElement,
                             apply->getBoundVariable(1));
          if (bvar->nArguments() != 1)
            throw CodeGenerationError(L"bvar can only have one argument.");
          RETURN_INTO_OBJREF(bvciel, iface::mathml_dom::MathMLElement,
                             bvar->getArgument(1));
          DECLARE_QUERY_INTERFACE_OBJREF(bvci, bvciel,
                                         mathml_dom::MathMLCiElement);
          if (bvci == NULL)
            throw CodeGenerationError(L"Bound variable argument must be a ci "
                                      L"element.");
          // Now get the VariableInformation...
          VariableInformation* bvcivi;
          RETURN_INTO_OBJREF(bvv, iface::cellml_api::CellMLVariable,
                             FindVariableForCI(aComponent, bvci));
          
          bvcivi = aCGS->FindOrAddVariableInformation(bvv);
          if (bvcivi == NULL)
            throw CodeGenerationError
              (L"No varinfo found for bvar ci source(bug).");

          uint32_t bvarId = bvcivi->GetIndex();

          RETURN_INTO_OBJREF(upl, iface::mathml_dom::MathMLElement,
                             apply->upLimit());
          RETURN_INTO_OBJREF(lowl, iface::mathml_dom::MathMLElement,
                             apply->lowLimit());
          if (upl == NULL || upl == NULL)
          {
            std::wstring aMsg = L"Int elements need uplimit and lowlimit, "
              L"because only definite integrals are supported. Error "
              L"occurred while processing component ";
            RETURN_INTO_WSTRING(compn, aComponent->name());
            aMsg += compn;
            aMsg += L".";
            throw CodeGenerationError(aMsg);
          }
          DECLARE_QUERY_INTERFACE_OBJREF(uplcc, upl,
                                         mathml_dom::MathMLContentContainer);
          DECLARE_QUERY_INTERFACE_OBJREF(lowlcc, lowl,
                                         mathml_dom::MathMLContentContainer);
          if (uplcc->nArguments() != 1)
          {
            std::wstring aMsg = L"Uplimit needs exactly one argument, in "
              L"component ";
            RETURN_INTO_WSTRING(compn, aComponent->name());
            aMsg += compn;
            aMsg += L".";
            throw CodeGenerationError(aMsg);
          }
          upl = already_AddRefd<iface::mathml_dom::MathMLElement>
            (uplcc->getArgument(1));
          if (lowlcc->nArguments() != 1)
          {
            std::wstring aMsg = L"Lowlimit needs exactly one argument, in "
              L"component ";
            RETURN_INTO_WSTRING(compn, aComponent->name());
            aMsg += compn;
            aMsg += L".";
            throw CodeGenerationError(aMsg);
          }
          lowl = already_AddRefd<iface::mathml_dom::MathMLElement>
            (lowlcc->getArgument(1));

          RETURN_INTO_OBJREF(arg, iface::mathml_dom::MathMLElement,
                             apply->getArgument(2));
          std::stringstream func;
          uint32_t id;
          func << "double integrand_" << (id = aCGS->AssignFunctionId())
               << "(double* CONSTANTS, double* VARIABLES"
               << ", double* BOUND"
               << ")" << std::endl
               << "{" << std::endl
               << "  return (";
          GenerateExpression(aCGS, aComponent, arg, func,
                             supplementaryFunctions, aHaveBound);
          func << ");" << std::endl << "}" << std::endl;
          supplementaryFunctions << func.str();
          expression << "defint(integrand_" << id << ", CONSTANTS, VARIABLES, ";
          if (aHaveBound)
            expression << "BOUND, ";
          else
            expression << "NULL, ";
          expression << bvarId << ", ";
          GenerateExpression(aCGS, aComponent, lowl, expression,
                             supplementaryFunctions, aHaveBound);
          expression << ", ";
          GenerateExpression(aCGS, aComponent, upl, expression,
                             supplementaryFunctions, aHaveBound);
          expression << ")";
        }
        break;
      }

      expression << opi->poststring;
      return;
    }
  }

  // A piecewise?
  {
    DECLARE_QUERY_INTERFACE_OBJREF(piecewise, aExpr,
                                   mathml_dom::MathMLPiecewiseElement);
    if (piecewise != NULL)
    {
      RETURN_INTO_OBJREF(pl, iface::mathml_dom::MathMLNodeList,
                         piecewise->pieces());
      uint32_t i, l = pl->length();
      for (i = 1; i <= l; i++)
      {
        RETURN_INTO_OBJREF(p, iface::mathml_dom::MathMLCaseElement,
                           piecewise->getCase(i));
        RETURN_INTO_OBJREF(cc, iface::mathml_dom::MathMLElement,
                           p->caseCondition());
        RETURN_INTO_OBJREF(cv, iface::mathml_dom::MathMLElement,
                           p->caseValue());
        expression << "(";
        GenerateExpression(aCGS, aComponent, cc, expression,
                           supplementaryFunctions, aHaveBound);
        expression << ") ? (";
        GenerateExpression(aCGS, aComponent, cv, expression,
                           supplementaryFunctions, aHaveBound);
        expression << ") : ";
      }

      try
      {
        RETURN_INTO_OBJREF(otherwise, iface::mathml_dom::MathMLElement,
                           (piecewise->otherwise()));
        expression << "(";
        GenerateExpression(aCGS, aComponent, otherwise, expression,
                           supplementaryFunctions, aHaveBound);
        expression << ")";
      }
      catch (iface::cellml_api::CellMLException&)
      {
        // Most portable way to get a nan...
        expression << "(0.0/0.0)";
      }
      return;
    }
  }

  // A built-in constant?
  {
    DECLARE_QUERY_INTERFACE_OBJREF(pds,  aExpr,
                                   mathml_dom::MathMLPredefinedSymbol);
    if (pds != NULL)
    {
      RETURN_INTO_WSTRING(sn, pds->symbolName());

      if (sn == L"exponentiale")
      {
        expression << "2.71828182845905";
        return;
      }
      else if (sn == L"notanumber")
      {
        expression << "(0.0/0.0)";
        return;
      }
      else if (sn == L"true")
      {
        expression << "1.0";
        return;
      }
      else if (sn == L"false")
      {
        expression << "0.0";
        return;
      }
      else if (sn == L"pi")
      {
        expression << "3.14159265358979";
        return;
      }
      else if (sn == L"eulergamma")
      {
        expression << "0.577215664901533";
        return;
      }
      else if (sn == L"infinity")
      {
        expression << "(1.0/0.0)";
        return;
      }

      // otherwise don't return...
    }
  }

  // We have found some maths we can't deal with...
  std::wstring aMsg = L"Cannot deal with MathML element ";
  RETURN_INTO_WSTRING(ln, aExpr->localName());
  aMsg += ln;
  aMsg += L" in a context expecting a double, in component ";
  RETURN_INTO_WSTRING(cn, aComponent->name());
  aMsg += cn;
  aMsg += L".";
  throw CodeGenerationError(aMsg);
}

void
ProcessInitialValue(
                    CodeGenerationState* aCGS,
                    std::list<InitialAssignment>& aInitial,
                    VariableInformation* vi,
                    iface::cellml_api::CellMLVariable *var,
                    iface::cellml_api::CellMLVariable *sv,
                    uint32_t depth
                   )
{
  if (depth > 100)
  {
    throw CodeGenerationError(L"A sequence of >100 initial value assignments "
                              L"from other variables was found. You probably "
                              L"created a loop.");
  }

  if (vi->IsFlagged(VariableInformation::SEEN_INITIAL_VALUE))
    return;
  vi->SetFlag(VariableInformation::SEEN_INITIAL_VALUE);

  RETURN_INTO_WSTRING(iv, sv->initialValue());
  if (iv != L"")
  {
    // See if it is a double...
    const wchar_t* ivs = iv.c_str();
    wchar_t* ivend;
    double val = wcstod(ivs, &ivend);
    if (*ivend == 0)
    {
      vi->SetFlag(VariableInformation::HAS_INITIAL_VALUE);
      vi->SetInitialValue(val);
    }
    else
    {
      vi->SetFlag(VariableInformation::HAS_INITIAL_ASSIGNMENT);
      InitialAssignment ia;
      ia.destination = vi;
      // Find the component...
      RETURN_INTO_OBJREF(compel, iface::cellml_api::CellMLElement,
                         sv->parentElement());
      DECLARE_QUERY_INTERFACE_OBJREF(comp, compel,
                                     cellml_api::CellMLComponent);
      if (comp == NULL)
        throw CodeGenerationError(L"Parent of variable isn't a component.");
      // Search for the variable...
      RETURN_INTO_OBJREF(varset, iface::cellml_api::CellMLVariableSet,
                         comp->variables());
      RETURN_INTO_OBJREF(svar, iface::cellml_api::CellMLVariable,
                         varset->getVariable(ivs));
      if (svar == NULL)
      {
        std::wstring emsg = L"Variable ";
        RETURN_INTO_WSTRING(v, var->name());
        emsg += v;
        emsg += L" has initial_value ";
        emsg += ivs;
        emsg += L" which isn't a valid floating point number, nor a valid "
          L"variable name within that component.";
        throw CodeGenerationError(emsg);
      }
      VariableInformation* svi = aCGS->FindOrAddVariableInformation(svar);
      ProcessInitialValue(aCGS, aInitial, svi, svar,
                          svi->GetSourceVariable(), depth + 1);
      ia.source = svi;
      ia.factor = aCGS->GetConversion(svi->GetSourceVariable(), sv, ia.offset);
      aInitial.push_back(ia);
    }
  }
}

void
RecursivelyFlagVariables
(
 CodeGenerationState* aCGS,
 iface::cellml_api::CellMLComponent* aComponent,
 iface::mathml_dom::MathMLElement* aElement,
 std::set<VariableInformation*>& aLocallyBound,
 uint32_t contextFlags,
 uint32_t degree,
 std::list<InitialAssignment>& aInitial
)
{
  DECLARE_QUERY_INTERFACE_OBJREF(ci, aElement, mathml_dom::MathMLCiElement);
  if (ci != NULL)
  {
    RETURN_INTO_OBJREF(var, iface::cellml_api::CellMLVariable,
                       FindVariableForCI(aComponent, ci));
    VariableInformation* vi = aCGS->FindOrAddVariableInformation(var);
    ProcessInitialValue(aCGS, aInitial, vi, var,
                        vi->GetSourceVariable(), 0);
    if ((contextFlags & CONTEXT_DIFFBVAR) == CONTEXT_DIFF)
    {
      vi->SetFlag(VariableInformation::SUBJECT_OF_DIFF);
      vi->SetDegree(degree);
    }
    else if ((contextFlags & CONTEXT_DIFFBVAR) == CONTEXT_DIFFBVAR)
      vi->SetFlag(VariableInformation::BVAR_OF_DIFF);
    else if ((contextFlags & CONTEXT_INTBVAR) == CONTEXT_INTBVAR)
    {
      aLocallyBound.insert(vi);
      vi->SetFlag(VariableInformation::OTHER_BVAR);
    }
    if (aLocallyBound.count(vi) == 0)
      vi->SetFlag(VariableInformation::NONBVAR_USE);
    return;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(apply, aElement,
                                 mathml_dom::MathMLApplyElement);
  if (apply != NULL)
  {
    uint32_t n = apply->nArguments();
    if (n <= 1)
      throw CodeGenerationError(L"Don't know how to deal with apply with one "
                                L"or fewer arguments only.");
    RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                       apply->_cxx_operator());
    DECLARE_QUERY_INTERFACE_OBJREF(oppd, op,
                                   mathml_dom::MathMLPredefinedSymbol);
    if (oppd == NULL)
      throw CodeGenerationError(L"You can only apply a built-in function.");
    RETURN_INTO_WSTRING(sn, oppd->symbolName());
    // See if it is a diff or int...
    if (sn == L"diff")
    {
      contextFlags |= CONTEXT_DIFF;
      // Also find the degree. Can't use opDegree, because it creates if not
      // found.
      RETURN_INTO_OBJREF(nl, iface::dom::NodeList,
                         apply->getElementsByTagNameNS
                         (L"http://www.w3.org/1998/Math/MathML",
                          L"degree"));
      RETURN_INTO_OBJREF(deg, iface::dom::Node, nl->item(0));
      DECLARE_QUERY_INTERFACE_OBJREF(degc, deg,
                                     mathml_dom::MathMLContentContainer);
      if (degc == NULL)
        degree = 1;
      else
      {
        if (degc->nArguments() != 1)
          throw CodeGenerationError(L"Cannot deal with degree with != 1 "
                                    L"children.");
        RETURN_INTO_OBJREF(d, iface::mathml_dom::MathMLElement,
                           degc->getArgument(1));
        DECLARE_QUERY_INTERFACE_OBJREF(dcn, d, mathml_dom::MathMLCnElement);
        if (dcn == NULL)
          throw CodeGenerationError(L"Only cn inside degree is supported.");
        RETURN_INTO_OBJREF(dcnn, iface::dom::Node, dcn->getArgument(1));
        if (dcnn == NULL)
          throw CodeGenerationError(L"cn inside degree can't be empty.");
        RETURN_INTO_WSTRING(dcnval, dcnn->nodeValue());
        TrimString(dcnval);
        const wchar_t* tmp = dcnval.c_str();
        wchar_t* end;
        degree = wcstoul(tmp, &end, 10);
        if (*end != L'\0')
          throw CodeGenerationError(L"Degree of differential is malformed.");
      }
    }
    else if (sn == L"int")
      contextFlags |= CONTEXT_INT;
    std::set<VariableInformation*> mLocallyBound = aLocallyBound;
    uint32_t nb = apply->nBoundVariables(), i;
    for (i = 1; i <= nb; i++)
    {
      RETURN_INTO_OBJREF(bve, iface::mathml_dom::MathMLBvarElement,
                         apply->getBoundVariable(i));
      RecursivelyFlagVariables(aCGS, aComponent, bve, mLocallyBound,
                               contextFlags, degree, aInitial);
    }

    for (i = 1; i <= n; i++)
    {
      RETURN_INTO_OBJREF(a, iface::mathml_dom::MathMLElement,
                         apply->getArgument(i));
      RecursivelyFlagVariables(aCGS, aComponent, a, mLocallyBound,
                               contextFlags, degree, aInitial);
    }

    // Also, need to go into uplimit, lowlimit, degree, and logbase...
    RETURN_INTO_OBJREF(nl, iface::dom::NodeList, apply->childNodes());
    n = nl->length();
    for (i = 0; i < n; i++)
    {
      RETURN_INTO_OBJREF(it, iface::dom::Node, nl->item(i));
      if (it == NULL)
        break;
      RETURN_INTO_WSTRING(nsuri, it->namespaceURI());
      RETURN_INTO_WSTRING(ln, it->localName());
      if (nsuri != L"http://www.w3.org/1998/Math/MathML")
        continue;
      if (ln != L"uplimit" && ln != L"lowlimit" && ln != L"degree" &&
          ln != L"logbase")
        continue;
      RETURN_INTO_OBJREF(nl2, iface::dom::NodeList, it->childNodes());
      uint32_t j, m = nl2->length();
      for (j = 0; j < m; j++)
      {
        RETURN_INTO_OBJREF(it2, iface::dom::Node, nl2->item(j));
        if (it2 == NULL)
          break;
        DECLARE_QUERY_INTERFACE_OBJREF(it2el, it2, mathml_dom::MathMLElement);
        if (it2el == NULL)
          continue;
        RecursivelyFlagVariables(aCGS, aComponent, it2el, mLocallyBound,
                                 contextFlags, degree, aInitial);
      }
    }
    return;
  }

  // It could also be a bvar...
  DECLARE_QUERY_INTERFACE_OBJREF(bv, aElement,
                                 mathml_dom::MathMLBvarElement);
  if (bv != NULL)
  {
    uint32_t n = bv->nArguments();
    if (n != 1)
      throw CodeGenerationError(L"Don't know how to deal with bvar with "
                                L"anything other than one argument.");
    RETURN_INTO_OBJREF(bvc, iface::mathml_dom::MathMLElement,
                       bv->getArgument(1));
    DECLARE_QUERY_INTERFACE_OBJREF(bvci, bvc, mathml_dom::MathMLCiElement);
    if (bvci == NULL)
      throw CodeGenerationError(L"Cannot cope with bvar children which are "
                                L"not ci elements.");
    RecursivelyFlagVariables(aCGS, aComponent, bvci, aLocallyBound,
                             contextFlags | CONTEXT_BVAR, degree, aInitial);
    return;
  }

  DECLARE_QUERY_INTERFACE_OBJREF(pw, aElement,
                                 mathml_dom::MathMLPiecewiseElement);
  if (pw != NULL)
  {
    RETURN_INTO_OBJREF(pl, iface::mathml_dom::MathMLNodeList,
                       pw->pieces());
    uint32_t i, l = pl->length();
    for (i = 1; i <= l; i++)
    {
      RETURN_INTO_OBJREF(p, iface::mathml_dom::MathMLCaseElement,
                         pw->getCase(i));
      RETURN_INTO_OBJREF(cc, iface::mathml_dom::MathMLElement,
                         p->caseCondition());
      RETURN_INTO_OBJREF(cv, iface::mathml_dom::MathMLElement,
                         p->caseValue());
      RecursivelyFlagVariables(aCGS, aComponent, cc, aLocallyBound,
                               contextFlags, degree, aInitial);
      RecursivelyFlagVariables(aCGS, aComponent, cv, aLocallyBound,
                               contextFlags, degree, aInitial);
    }
    try
    {
      RETURN_INTO_OBJREF(ow, iface::mathml_dom::MathMLElement,
                         pw->otherwise());
      if (ow != NULL)
        RecursivelyFlagVariables(aCGS, aComponent, ow, aLocallyBound,
                                 contextFlags, degree, aInitial);
    }
    catch (iface::dom::DOMException& de) {}
  }
}

void
Equation::FlagVariables(CodeGenerationState* aCGS,
                        std::list<InitialAssignment>& aInitial)
{
  std::list<std::pair<iface::cellml_api::CellMLComponent*,
                      iface::mathml_dom::MathMLElement*> >::iterator i;
  std::set<VariableInformation*> emptySet;
  iface::cellml_api::CellMLComponent* comp = NULL;
  for (i = equal.begin(); i != equal.end(); i++)
  {
    RecursivelyFlagVariables(aCGS, (*i).first, (*i).second, emptySet, 0, 0,
                             aInitial);
    comp = (*i).first;
  }
  if (mDiff != NULL)
    RecursivelyFlagVariables(aCGS, comp, mDiff, emptySet, 0, 0,
                             aInitial);
}

bool
Equation::AttemptEvaluation
(
 CodeGenerationState* aCGS, std::set<VariableInformation*>& aAvailable,
 std::set<VariableInformation*>& aWanted,
 std::stringstream& aExpressions,
 std::stringstream& aSupplementary,
 bool aHaveBound
)
{
  std::list<std::pair<iface::cellml_api::CellMLComponent*,
                      iface::mathml_dom::MathMLElement*> >::iterator i, j, er;
  iface::cellml_api::CellMLComponent* comp = NULL;
  ObjRef<iface::mathml_dom::MathMLElement> completelyKnown, singleWanted;
  ObjRef<iface::mathml_dom::MathMLCiElement> wantedCI;
  bool eligibleForDirectAssignment = true;
  VariableInformation* wantedVI;
  uint32_t wantedCount = 0;

  if (equal.size() == 1)
    return false;

  for (i = equal.begin(); i != equal.end(); i++)
  {
    uint32_t xwantedCount = wantedCount;
    // If it returns false, it means that either the wanted count exceeded one,
    // or a variable neither known nor wanted was encountered.
    ObjRef<iface::mathml_dom::MathMLCiElement> xCI = wantedCI;
    VariableInformation* xVI = wantedVI;
    if (!RecursivelyCountUnknown(aCGS, (*i).first, (*i).second, aAvailable,
                                 aWanted, xwantedCount,
                                 xCI, xVI, 0))
      continue;
    if (singleWanted || xwantedCount == wantedCount)
    {
      if (completelyKnown != NULL)
      {
        // We already know two expressions in the equality. That means it is
        // overconstrained.
        RETURN_INTO_OBJREF(n, iface::dom::Node,
                           (*i).second->parentNode());
        DECLARE_QUERY_INTERFACE_OBJREF(el, n, dom::Element);
        OverconstrainedError oce(el);
        std::set<VariableInformation*>::iterator si;
        for (si = aAvailable.begin(); si != aAvailable.end(); si++)
          oce.addKnownVariable((*si)->GetSourceVariable());
        throw oce;
      }
      completelyKnown = (*i).second;
      comp = (*i).first;
      if (xwantedCount != wantedCount)
        eligibleForDirectAssignment = false;
    }
    else
    {
      singleWanted = (*i).second;
      wantedCI = xCI;
      wantedVI = xVI;
      wantedCount = xwantedCount;
      er = i;
    }

    if (completelyKnown && singleWanted)
      break;
  }

  // This equation is extraneous, so is no use. We will be giving an error
  // about this later.
  if (!singleWanted)
    return false;

  if (!completelyKnown)
    return false;

  RETURN_INTO_OBJREF(vloc, iface::cellml_api::CellMLVariable,
                     FindVariableForCI(comp, wantedCI));
  if (vloc == NULL)
  {
    std::wstring msg = L"Cannot find variable ";
    wchar_t* tmp = vloc->name();
    msg += tmp;
    free(tmp);
    msg += L"; referenced in ci but not in component.";
    throw CodeGenerationError(msg);
  }
  iface::cellml_api::CellMLVariable* vsource = 
    aCGS->FindOrAddVariableInformation(vloc)->GetSourceVariable();

  // See if singleWanted is a bare CI...
  DECLARE_QUERY_INTERFACE_OBJREF(swci, singleWanted,
                                 mathml_dom::MathMLCiElement);
  std::string evaluation;
  if (eligibleForDirectAssignment && swci != NULL)
  {
    // Next, find the overall conversion factor...
    double offset;
    double factor = aCGS->GetConversion(vloc, vsource, offset);
    aExpressions << aCGS->GetVariableText(vsource) << " = ";
    if (offset != 0.0)
      aExpressions << "(";
    if (factor != 1.0)
      aExpressions << "(";
    GenerateExpression(aCGS, comp, completelyKnown, aExpressions,
                       aSupplementary, aHaveBound);
    if (factor != 1.0)
      aExpressions << ") * " << std::setiosflags(std::ios_base::showpoint) << factor;
    if (offset != 0.0)
      aExpressions << ") + " << std::setiosflags(std::ios_base::showpoint) << offset;
    aExpressions << ";" << std::endl;
  }
  else
  {
    // We have a variable that needs computing, but it isn't in a ci by itself.
    std::stringstream func;
    uint32_t id;
    func << "double NR_minfunc_" << (id = aCGS->AssignFunctionId())
         << "(double* CONSTANTS, double* VARIABLES"
         << ", double* BOUND";
    func << ")" << std::endl
         << "{" << std::endl
         << "  return ((";
    GenerateExpression(aCGS, comp, completelyKnown, func,
                       aSupplementary, aHaveBound);
    func << ") - (";
    GenerateExpression(aCGS, comp, singleWanted, func,
                       aSupplementary, aHaveBound);
    func << "));" << std::endl
         << "}" << std::endl;
    aSupplementary << func.str();
    aExpressions << "NR_MINIMIZE(NR_minfunc_" << id
                 << ", CONSTANTS, VARIABLES, ";
    if (aHaveBound)
      aExpressions << "BOUND, ";
    else
      aExpressions << "NULL, ";
    aExpressions << aCGS->GetVariableIndex(vsource) << ");" << std::endl;
    mTriggersNewtonRaphson = true;
  }

  // We remove the newly known part, rather than the previous completely known
  // part, in order to allow the compiler to better optimise. Of course, we
  // could probably do some logic to figure out which one is simplest, and
  // throw the more complex one away(later optimisation).
  (*er).second->release_ref();
  equal.erase(er);

  // The variable is no longer wanted, instead its available...
  {
    RETURN_INTO_WSTRING(sn, wantedVI->GetSourceVariable()->name());
    RETURN_INTO_OBJREF(topapply, iface::dom::Node, singleWanted->parentNode());
    RETURN_INTO_OBJREF(math, iface::dom::Node, topapply->parentNode());
    DECLARE_QUERY_INTERFACE_OBJREF(mathel, math, dom::Element);
    RETURN_INTO_WSTRING(cm, mathel->getAttributeNS
                        (L"http://www.cellml.org/metadata/1.0#", L"id"));
  }

  aWanted.erase(wantedVI);
  aAvailable.insert(wantedVI);

  // If we only have one equal left, it isn't an equation any more, so we
  // should request that the CGS delete us...
  if (equal.size() == 1)
    aCGS->EquationFullyUsed(this);

  return true;
}

bool
Equation::AttemptRateEvaluation
(
 CodeGenerationState* aCGS,
 std::stringstream& aRateStream,
 std::stringstream& aSupplementary
)
{
  if (equal.size() == 0 || mDiffCI == NULL)
    return false;

  iface::cellml_api::CellMLComponent* comp = equal.front().first;
  iface::mathml_dom::MathMLElement* rhs = equal.front().second;

  RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable,
                     FindVariableForCI(comp, mDiffCI));
  VariableInformation* vi = aCGS->FindOrAddVariableInformation(cv);

  RETURN_INTO_OBJREF(bcv, iface::cellml_api::CellMLVariable,
                     FindVariableForCI(comp, mBoundCI));

  // Find out the conversion factors. We ignore the offsets, because it is a
  // rate, and only the factors matter.
  double unused;
  double boundfac = aCGS->GetConversion
    (bcv, aCGS->FindOrAddVariableInformation(bcv)->GetSourceVariable(),
     unused);
  double difffac = aCGS->GetConversion(cv, vi->GetSourceVariable(), unused);
  double fac = difffac / boundfac;

  uint32_t deg = vi->GetDegree(), idx = vi->GetIndex();
  uint32_t i;
  for (i = 1; i < deg; i++)
    aRateStream << "RATES[" << (idx + i - 1) << "] = VARIABLES[" << (idx + i)
                << "];" << std::endl;

  aRateStream << "RATES[" << (idx + deg - 1) << "] = ";
  if (fac != 1.0)
    aRateStream << "(";
  GenerateExpression(aCGS, comp, rhs, aRateStream, aSupplementary, true);
  if (fac != 1.0)
    aRateStream << ") * " << fac;
  aRateStream << ";" << std::endl;

  return true;
}

void
Equation::AddIfTriggersNR(std::vector<iface::dom::Element*>& aNRList)
{
  if (!mTriggersNewtonRaphson)
    return;

  if (equal.size() == 0)
    return;
  
  RETURN_INTO_OBJREF(n, iface::dom::Node,
                     equal.front().second->parentNode());
  DECLARE_QUERY_INTERFACE(el, n, dom::Element);
  aNRList.push_back(el);
}

void
Equation::release_state()
{
  std::pair<iface::cellml_api::CellMLComponent*,
    iface::mathml_dom::MathMLElement*> p = equal.front();
  equal.pop_front();
  p.second->release_ref();
  mDiffCI = NULL;
  mBoundCI = NULL;
  mDiff = NULL;
}
