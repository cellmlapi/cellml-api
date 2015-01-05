%{
  #include "cda_compiler_support.h"
  #define YYSTYPE TeLICeMSLValue
  #define YYSTYPE_IS_TRIVIAL 0
%}
%name_prefix="telicem_"

%pure-parser
%error-verbose

%defines
%{
  #define YYDEBUG 1

  #include "TeLICeMSImpl.hxx"

  #include <stack>
  #include <cmath>
  #include <assert.h>


  #define MATHML_NS L"http://www.w3.org/1998/Math/MathML"

  #define MAKE_MATHML_OBJECT(store,storetype,eltype) \
  ObjRef<iface::mathml_dom::storetype> store; \
  { \
   RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document()); \
   RETURN_INTO_OBJREF(eltmp, iface::dom::Element, doc->createElementNS(MATHML_NS, eltype)); \
   QUERY_INTERFACE(store, eltmp, mathml_dom::storetype);\
  }

  struct PropertyApplicator
  {
    const char* propertyName;
    void (*applyProperty)(iface::mathml_dom::MathMLContentElement* aEl, const std::string& aTarget);
  };


  #define DEFINE_PROPERTYAPPLICATOR(castTo, objName) \
  void apply##objName(iface::mathml_dom::MathMLContentElement* aEl, const std::string& aTarget) \
  { \
    std::wstring str(convertStringToWString(aTarget)); \
    static_cast<iface::mathml_dom::MathML##castTo*>(aEl)->objName(str.c_str()); \
  }

  #define DEFINE_PROPERTYAPPLICATOR_AMBIG(castTo, objName) \
  void apply##objName##castTo(iface::mathml_dom::MathMLContentElement* aEl, const std::string& aTarget) \
  { \
    std::wstring str(convertStringToWString(aTarget)); \
    dynamic_cast<iface::mathml_dom::MathML##castTo*>(aEl)->objName(str.c_str()); \
  }

  DEFINE_PROPERTYAPPLICATOR(ContentElement, className);
  DEFINE_PROPERTYAPPLICATOR(ContentElement, mathElementStyle);
  DEFINE_PROPERTYAPPLICATOR(ContentElement, id);
  DEFINE_PROPERTYAPPLICATOR(ContentElement, xref);
  DEFINE_PROPERTYAPPLICATOR(ContentElement, href);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(ContentToken, definitionURL);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(ContentToken, encoding);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(CnElement, type);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(CnElement, base);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(CiElement, type);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(PredefinedSymbol, definitionURL);
  DEFINE_PROPERTYAPPLICATOR_AMBIG(PredefinedSymbol, encoding);

  void applyunits(iface::mathml_dom::MathMLContentElement* aEl, const std::string& aTarget)
  {
    std::wstring wstr(convertStringToWString(aTarget));
    RETURN_INTO_OBJREF(doc, iface::dom::Document, aEl->ownerDocument());
    RETURN_INTO_OBJREF(el, iface::dom::Element, doc->documentElement());
    RETURN_INTO_WSTRING(nsURI, el->namespaceURI());
    aEl->setAttributeNS(nsURI.c_str(), L"units", wstr.c_str());
  }

  static PropertyApplicator pat_element[] =
  {
    {"class", applyclassName},
    {"style", applymathElementStyle},
    {"id", applyid},
    {"Xref", applyxref},
    {"Href", applyhref},
    {NULL, NULL}
  };

  static PropertyApplicator pat_token[] =
  {
    {"definitionURL", applydefinitionURLContentToken},
    {"encoding", applyencodingContentToken},
    {NULL, NULL}
  };

  static PropertyApplicator pat_cn[] =
  {
    {"type", applytypeCnElement},
    {"base", applybaseCnElement},
    {"units", applyunits},
    {NULL, NULL}
  };

  static PropertyApplicator pat_ci[] =
  {
    {"type", applytypeCiElement},
    {NULL, NULL}
  };

  static PropertyApplicator pat_predef[] =
  {
    {"definitionURL", applydefinitionURLPredefinedSymbol},
    {"encoding", applyencodingPredefinedSymbol},
    {NULL, NULL}
  };

  /*
   * This is used for a range of content elements which don't have any further
   * properties...
   */

  static PropertyApplicator* patt_contentelement[] =
  {
    pat_element,
    NULL
  };

  static PropertyApplicator* patt_cn[] =
  {
    pat_element,
    pat_token,
    pat_cn,
    NULL
  };

  static PropertyApplicator* patt_ci[] =
  {
    pat_element,
    pat_token,
    pat_ci,
    NULL
  };

  static PropertyApplicator* patt_predef[] =
  {
    pat_element,
    pat_predef,
    NULL
  };

  int telicem_lex(TeLICeMSLValue* aLValue, TeLICeMStateScan* aLexer);

  static void ApplyPropertyList(iface::mathml_dom::MathMLContentElement* aEl,
                                PropertyApplicator** aPropTables,
                                const std::map<std::string, std::string>& aProps)
  {
    PropertyApplicator ** p = aPropTables, * q;
    for (q = *p; (q = *p); p++)
    {
      for (; q->propertyName != NULL; q++)
      {
        std::map<std::string, std::string>::const_iterator i(aProps.find(q->propertyName));
        if (i == aProps.end())
          continue;

        q->applyProperty(aEl, (*i).second);
      }
    }
  }

  static bool IsConstant(const std::string& aName)
  {
    const char* t = aName.c_str();
    if (t[0] <= 'i')
    {
      if (t[0] <= 'e')
      {
        if (t[0] == 'e')
        {
          if (t[1] == 'm')
            // emptyset
            return strcmp(t + 2, "ptyset") == 0;
          else if (t[1] == 'u')
            // eulergamma
            return strcmp(t + 2, "lergamma") == 0;
          else if (t[1] == 'x')
            // exponentiale
            return strcmp(t + 2, "ponentiale") == 0;
          else
            return false;
        }
        else if (t[0] == 'c')
        {
          // complexes
          return strcmp(t + 1, "omplexes") == 0;
        }
        else
          return false;
      }
      else if (t[0] == 'i')
      {
        if (t[1] == 'n')
        {
          if (t[2] == 'f')
            // infinity
            return strcmp(t + 3, "inity") == 0;
          else if (t[2] == 't')
            // integers
            return strcmp(t + 3, "egers") == 0;
          else
            return false;
        }
        else if (t[1] == 'm')
          // imaginaryi
          return strcmp(t + 2, "aginaryi") == 0;
        else
          return false;
      }
      else if (t[0] == 'f')
        // false
        return strcmp(t + 1, "alse") == 0;
      else
        return false;
    }
    else
    {
      if (t[0] <= 'p')
      {
        if (t[0] == 'n')
        {
          if (t[1] == 'a')
            // naturalnumbers
            return strcmp("turalnumbers", t + 2) == 0;
          else if (t[1] == 'o')
            // notanumber
            return strcmp("tanumber", t + 2) == 0;
          else
            return false;
        }
        else if (t[0] == 'p')
        {
          if (t[1] == 'i')
            // pi
            return t[2] == 0;
          else if (t[1] == 'r')
            // primes
            return strcmp("imes", t + 2) == 0;
          else
            return false;
        }
      }
      else
      {
        if (t[0] == 'r')
        {
          if (t[1] == 'a')
            return strcmp(t + 2, "tionals") == 0;
          else if (t[1] == 'e')
            return strcmp(t + 2, "als") == 0;
          else
            return false;
        }
        else if (t[0] == 't')
          // true
          return strcmp(t + 1, "rue") == 0;
        else
          return false;
      }
    }
    // Unreachable, but gcc complains if this isn't here...
    return false;
  }

  static iface::mathml_dom::MathMLContentElement*
  MakeCIOrConstant(const std::string& aName,
                   bool aOverrideConstant,
                   const std::map<std::string,std::string>& aPropList,
                   TeLICeMSParseTarget* aParseTarget)
  {
    if (aOverrideConstant || !IsConstant(aName))
    {
      MAKE_MATHML_OBJECT(ciEl, MathMLCiElement, L"ci");
      std::wstring name(convertStringToWString(aName));
      {
        RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
        RETURN_INTO_OBJREF(tEl, iface::dom::Text,
                           doc->createTextNode(name.c_str()));
        ciEl->insertArgument(tEl, 1)->release_ref();
      }

      ApplyPropertyList(ciEl, patt_ci, aPropList);

      ciEl->add_ref();
      return ciEl;
    }
    else
    {
      std::wstring name(convertStringToWString(aName));
      RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
      RETURN_INTO_OBJREF(tmpstore, iface::dom::Element,
                         doc->createElementNS(MATHML_NS, name.c_str()));
      DECLARE_QUERY_INTERFACE_OBJREF(store, tmpstore,
                                     mathml_dom::MathMLContentElement);

      ApplyPropertyList(store, patt_predef, aPropList);

      store->add_ref();
      return store;
    }
  }

  static iface::mathml_dom::MathMLContentElement*
  MakeCaseElement
  (
   iface::mathml_dom::MathMLContentElement* aCondition,
   iface::mathml_dom::MathMLContentElement* aValue,
   TeLICeMSParseTarget* aParseTarget,
   const std::map<std::string, std::string>& aPropList
  )
  {
    MAKE_MATHML_OBJECT(ceEl, MathMLCaseElement, L"piece");
    ceEl->caseValue(aValue);
    ceEl->caseCondition(aCondition);

    ApplyPropertyList(ceEl, patt_contentelement, aPropList);

    ceEl->add_ref();
    return ceEl;
  }

  static iface::mathml_dom::MathMLContentElement*
  DoInorderExpression
  (
   const std::string& aOp,
   TeLICeMSLValue& aLHS, TeLICeMSLValue& aRHS,
   TeLICeMSParseTarget* aParseTarget,
   const std::map<std::string, std::string>& aPropList,
   bool foldLeft
  )
  {
    // Firstly, we try to merge into the left-hand expression...
    if (foldLeft && !aLHS.mIndirect && aPropList.size() == 0)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(apply, aLHS.math(), mathml_dom::MathMLApplyElement);
      if (apply != NULL)
      {
        RETURN_INTO_OBJREF(op, iface::mathml_dom::MathMLElement,
                           apply->_cxx_operator());
        RETURN_INTO_WSTRING(nsURI, op->namespaceURI());
        RETURN_INTO_WSTRING(lname, op->localName());
        if (nsURI == MATHML_NS && lname == convertStringToWString(aOp))
        {
          apply->appendChild(aRHS.math())->release_ref();
          apply->add_ref();
          return apply;
        }
      }
    }

    // We need a new apply element and operator...
    MAKE_MATHML_OBJECT(apply, MathMLApplyElement, L"apply");
    std::wstring waOp(convertStringToWString(aOp));
    {
      RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
      RETURN_INTO_OBJREF(operatorDOMEl,
                         iface::dom::Element,
                         doc->createElementNS(MATHML_NS, waOp.c_str()));
      DECLARE_QUERY_INTERFACE_OBJREF(operatorEl, operatorDOMEl, mathml_dom::MathMLContentElement);

      apply->appendChild(operatorEl)->release_ref();
    }

    // We now have a new element, so add the left and right hand sides...
    apply->appendChild(aLHS.math())->release_ref();
    apply->appendChild(aRHS.math())->release_ref();

    // Put attributes onto the apply...
    ApplyPropertyList(apply, patt_predef, aPropList);

    apply->add_ref();
    return apply;
  }

  static iface::mathml_dom::MathMLContentElement*
  MakeOtherwiseElement(iface::mathml_dom::MathMLElement* aExpression,
                       const std::map<std::string, std::string>& aPropList,
                       TeLICeMSParseTarget* aParseTarget)
  {
    MAKE_MATHML_OBJECT(otherwise, MathMLContentElement, L"otherwise");
    otherwise->appendChild(aExpression)->release_ref();

    // Put attributes onto the apply...
    ApplyPropertyList(otherwise, patt_contentelement, aPropList);

    otherwise->add_ref();
    return otherwise;
  }

  static iface::mathml_dom::MathMLContentElement*
  MakePredefinedWrapElement(const std::string& aType,
                            iface::mathml_dom::MathMLElement* aExpression,
                            const std::map<std::string,std::string>& aPropList,
                            TeLICeMSParseTarget* aParseTarget)
  {
    MAKE_MATHML_OBJECT(apply, MathMLApplyElement, L"apply");
    std::wstring waType(convertStringToWString(aType));
    RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
    RETURN_INTO_OBJREF(wrapDOMEl, iface::dom::Element,
                       doc->createElementNS(MATHML_NS, waType.c_str()));
    DECLARE_QUERY_INTERFACE_OBJREF(wrapEl, wrapDOMEl, mathml_dom::MathMLPredefinedSymbol);

    apply->appendChild(wrapDOMEl)->release_ref();
    apply->appendChild(aExpression)->release_ref();

    // Put attributes onto the apply...
    ApplyPropertyList(wrapEl, patt_predef, aPropList);

    apply->add_ref();
    return apply;
  }

  static iface::mathml_dom::MathMLContentElement*
  MakePiecewiseElement(const std::list<iface::mathml_dom::MathMLContentElement*>& aCases,
                       iface::mathml_dom::MathMLContentElement* aOtherwise,
                       const std::map<std::string,std::string>& aPropList,
                       TeLICeMSParseTarget* aParseTarget)
  {
    MAKE_MATHML_OBJECT(piecewise, MathMLPiecewiseElement, L"piecewise");
    for (std::list<iface::mathml_dom::MathMLContentElement*>::const_iterator i = aCases.begin();
         i != aCases.end(); i++)
      piecewise->appendChild(*i)->release_ref();
    if (aOtherwise != NULL)
      piecewise->appendChild(aOtherwise)->release_ref();

    piecewise->add_ref();
    return piecewise;
  }

  #define CHILD_BVAR      0x0001
  #define CHILD_LOWLIMIT  0x0002
  #define CHILD_UPLIMIT   0x0004
  #define CHILD_DEGREE    0x0008
  #define CHILD_LOGBASE   0x0010

  static struct FuncInfo {
    const char* match;
    enum
    {
      TOPLEVEL,
      APPLY
    } InterpretAs;
    int SpecialQualifiers;
    const char* el;
  } FunctionInformation[] =
  {
    {"bvar", FuncInfo::TOPLEVEL, 0, "bvar"},
    {"conditon", FuncInfo::TOPLEVEL, 0, "condition"},
    {"definite_integral", FuncInfo::APPLY,
    CHILD_BVAR|CHILD_LOWLIMIT|CHILD_UPLIMIT, "int"},
    {"degree", FuncInfo::TOPLEVEL, 0, "degree"},
    {"domainofapplication", FuncInfo::TOPLEVEL, 0, "domainofapplication"},
    {"fn", FuncInfo::TOPLEVEL, 0, "fn"},
    {"interval", FuncInfo::TOPLEVEL, 0, "interval"},
    {"lambda", FuncInfo::TOPLEVEL, 0, "lambda"},
    {"list", FuncInfo::TOPLEVEL, 0, "list"},
    {"log", FuncInfo::APPLY, CHILD_LOGBASE, "log"},
    {"logbase", FuncInfo::TOPLEVEL, 0, "logbase"},
    {"lowlimit", FuncInfo::TOPLEVEL, 0, "lowlimit"},
    {"matrix", FuncInfo::TOPLEVEL, 0, "matrix"},
    {"momentabout", FuncInfo::TOPLEVEL, 0, "momentabout"},
    {"row", FuncInfo::TOPLEVEL, 0, "matrixrow"},
    {"set", FuncInfo::TOPLEVEL, 0, "set"},
    {"uplimit", FuncInfo::TOPLEVEL, 0, "uplimit"},
    {"vector", FuncInfo::TOPLEVEL, 0, "vector"}
  };

  #define FUNCTION_TABLE_SIZE (sizeof(FunctionInformation)/sizeof(FunctionInformation[0]))

  static FuncInfo* GetFunctionInformation(const char* name)
  {
    uint32_t upperp = FUNCTION_TABLE_SIZE - 1, lowerp = 0;
    FuncInfo* ret = NULL;
    do
    {
      uint32_t mid = (upperp + lowerp) / 2;
      int cmp = strcmp(FunctionInformation[mid].match, name);
      if (cmp == 0)
      {
        ret = FunctionInformation + mid;
        break;
      }
      else if (upperp <= lowerp)
        break;
      else if (cmp < 0)
        lowerp = mid + 1;
      else
      {
        if (mid == lowerp)
          break;
        upperp = mid - 1;
      }
    }
    while (true);
    return ret;
  }

  struct Qualifier {
    int flag;
    const char* localname;
  } Qualifiers[] =
  {
    {CHILD_BVAR, "bvar"},
    {CHILD_LOWLIMIT, "lowlimit"},
    {CHILD_UPLIMIT, "uplimit"},
    {CHILD_DEGREE, "degree"},
    {CHILD_LOGBASE, "logbase"}
  };

  #define QUALIFIER_TABLE_SIZE (sizeof(Qualifiers)/sizeof(Qualifiers[0]))

  template<typename C> class auto_release_container
  {
  public:
    auto_release_container(C& aC)
      : mC(aC)
    {
    }

    ~auto_release_container()
    {
      for (typename C::iterator i = mC.begin(); i != mC.end(); i++)
        (*i)->release_ref();
    }

  private:
    C& mC;
  };

  static iface::mathml_dom::MathMLContentElement*
  DoFunction(const std::string& aName,
             const std::list<iface::mathml_dom::MathMLContentElement*>& aMathList,
             TeLICeMSParseTarget* aParseTarget,
             const std::map<std::string,std::string>& aPropList)
  {
    FuncInfo* fi = GetFunctionInformation(aName.c_str());
    FuncInfo deffi;
    std::wstring el;
    if (fi == NULL)
      el = convertStringToWString(aName);
    else
      el = convertStringToWString(fi->el);

    RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
    ObjRef<iface::mathml_dom::MathMLContentElement> namedEl =
      QueryInterface(doc->createElementNS(MATHML_NS, el));
    if (fi == NULL)
    {
      if (namedEl == NULL)
      {
        namedEl = QueryInterface(doc->createElementNS(MATHML_NS, L"csymbol"));
        ObjRef<iface::dom::Text> tn(doc->createTextNode(el));
        namedEl->appendChild(tn)->release_ref();
      }

      fi = &deffi;
      deffi.InterpretAs = FuncInfo::APPLY;
      deffi.SpecialQualifiers = 0;
    }
    else
      namedEl = QueryInterface(doc->createElementNS(MATHML_NS, convertStringToWString(fi->el)));

    std::list<iface::mathml_dom::MathMLContentElement*> notConsumed;
    std::list<iface::mathml_dom::MathMLContentElement*> endQualifiers;
    auto_release_container<std::list<iface::mathml_dom::MathMLContentElement*> > eqar(endQualifiers);

    ObjRef<iface::mathml_dom::MathMLContentElement> math;
    if (fi->InterpretAs == FuncInfo::TOPLEVEL)
    {
      math = namedEl;
    }
    else
    {
      MAKE_MATHML_OBJECT(apply, MathMLContentElement, L"apply");
      apply->appendChild(namedEl)->release_ref();
      math = apply;
    }

    int sq = fi->SpecialQualifiers;
    for (std::list<iface::mathml_dom::MathMLContentElement*>::const_iterator j = aMathList.begin();
         j != aMathList.end(); j++)
    {
      bool foundOne = false;
      for (size_t i = 0; i < static_cast<size_t>(QUALIFIER_TABLE_SIZE); i++)
      {
        std::wstring ln16(convertStringToWString(Qualifiers[i].localname));
        if ((sq & Qualifiers[i].flag) == 0)
          continue;
        RETURN_INTO_WSTRING(localname, (*j)->localName());
        if (localname != ln16)
          continue;

        sq &= ~Qualifiers[i].flag;
        (*j)->add_ref();
        endQualifiers.push_back(*j);
        foundOne = true;
      }

      if (!foundOne)
        notConsumed.push_back(*j);
    }

    // Any remaining qualifiers get pushed onto the end of the list.
    for (int i = QUALIFIER_TABLE_SIZE - 1; i >= 0; i--)
    {
      if ((sq & Qualifiers[i].flag) == 0)
        continue;
      /* We never allow all the arguments to be used up by qualifiers,
      * because we automatically hold one back to use as the argument, so
      * e.g. log(5) is <apply><log/><cn>5</cn></apply> not
      * <apply><log/><logbase><cn>5</cn></logbase></apply>
      */
      if (notConsumed.size() <= 2)
        break;

      std::wstring ln16(convertStringToWString(Qualifiers[i].localname));

      RETURN_INTO_OBJREF(namedElN, iface::dom::Element,
                         doc->createElementNS(MATHML_NS, ln16.c_str()));
      DECLARE_QUERY_INTERFACE_OBJREF(namedEl, namedElN, mathml_dom::MathMLContentElement);

      namedEl->appendChild(notConsumed.back())->release_ref();
      notConsumed.pop_back();

      namedEl->add_ref();
      endQualifiers.push_back(namedEl);
    }

    ObjRef<iface::dom::Node> ignore;
    for (std::list<iface::mathml_dom::MathMLContentElement*>::iterator i = endQualifiers.begin();
         i != endQualifiers.end(); i++)
      math->appendChild(*i)->release_ref();
    for (std::list<iface::mathml_dom::MathMLContentElement*>::iterator i = notConsumed.begin();
         i != notConsumed.end(); i++)
      math->appendChild(*i)->release_ref();

    math->add_ref();

    return math;
  }

  static iface::mathml_dom::MathMLContentElement*
  MakeConstant(double aValue, TeLICeMSParseTarget* aParseTarget,
               const std::map<std::string,std::string>& aPropList)
  {
    MAKE_MATHML_OBJECT(cn, MathMLCnElement, L"cn");

    // Put attributes onto the apply...
    ApplyPropertyList(cn, patt_cn, aPropList);

    wchar_t buf[40];
    int expn;
    if (aValue == 0)
      expn = 0;
    else if (aValue < 0)
       expn = -static_cast<int>(floor(log10(-aValue)));
    else
       expn = static_cast<int>(floor(log10(aValue)));

    RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
    if (expn < -3 || expn > 3)
    {
      double mant = aValue / pow(10.0, expn);
      cn->type(L"e-notation");
      {
        any_swprintf(buf, 40, L"%.10g", mant);
        RETURN_INTO_OBJREF(tn, iface::dom::Text, doc->createTextNode(buf));
        cn->insertArgument(tn, 1)->release_ref();
      }

      any_swprintf(buf, 40, L"%d", expn);
      RETURN_INTO_OBJREF(tn, iface::dom::Text, doc->createTextNode(buf));
      cn->insertArgument(tn, 2)->release_ref();
    }
    else
    {
      any_swprintf(buf, 40, L"%.10g", aValue);
      RETURN_INTO_OBJREF(tn, iface::dom::Text, doc->createTextNode(buf));
      cn->insertArgument(tn, 1)->release_ref();
    }

    cn->add_ref();
    return cn;
  }

  static iface::mathml_dom::MathMLContentElement*
  DoDerivative
  (
   const wchar_t* aDifftype,
   iface::mathml_dom::MathMLContentElement* aExpr,
   iface::mathml_dom::MathMLContentElement* aBvar,
   iface::mathml_dom::MathMLContentElement* aDegree,
   const std::map<std::string,std::string>& aPropList,
   TeLICeMSParseTarget* aParseTarget
  )
  {
    RETURN_INTO_OBJREF(doc, iface::dom::Document, aParseTarget->document());
    RETURN_INTO_OBJREF(opN, iface::dom::Element,
                       doc->createElementNS(MATHML_NS, aDifftype));
    DECLARE_QUERY_INTERFACE_OBJREF(op, opN, mathml_dom::MathMLContentElement);

    MAKE_MATHML_OBJECT(apply, MathMLApplyElement, L"apply");
    apply->appendChild(op)->release_ref();

    DECLARE_QUERY_INTERFACE_OBJREF(bvEl, aBvar, mathml_dom::MathMLBvarElement);
    if (bvEl == NULL)
    {
      MAKE_MATHML_OBJECT(bvar, MathMLContainer, L"bvar");
      QUERY_INTERFACE(bvEl, bvar, mathml_dom::MathMLBvarElement);
      bvar->insertArgument(aBvar, 1)->release_ref();
    }

    if (aDegree != NULL)
    {
      MAKE_MATHML_OBJECT(degree, MathMLElement, L"degree");
      degree->appendChild(aDegree)->release_ref();
      bvEl->appendChild(degree)->release_ref();
    }

    apply->appendChild(bvEl)->release_ref();
    apply->appendChild(aExpr)->release_ref();

    apply->add_ref();
    return apply;
  }


  void telicem_error(TeLICeMStateScan *aLexer,
                     TeLICeMSParseTarget* aParseTarget,
                     const char *aMessage)
  {
    aLexer->lexerError(aMessage);
  }
%}

%parse-param {TeLICeMStateScan *aLexer}
%parse-param {TeLICeMSParseTarget *aParseTarget}

%lex-param {TeLICeMStateScan *aLexer}

%token T_ROLETYPE "role type (reactant, product, rate, catalyst, activator, inhibitor, or modifier)"
%token T_DIRECTIONTYPE "reaction direction type (forward or reverse)"
%token T_SIPREFIX "standard SI prefix"
%token T_INTERFACETYPE "interface type (in or out)"
%token T_REVERSIBILITY "reversibility (rev or irrev)"

/* General syntax... */
%token T_AS "as"
%token T_BASE "base"
%token T_BETWEEN "between"
%token T_CASE "case"
%token T_COMP "comp"
%token T_DEF "def"
%token T_DIR "dir"
%token T_DVAR "dvar"
%token T_ELSE "else"
%token T_ENDCOMP "endcomp"
%token T_ENDDEF "enddef"
%token T_ENDROLE "endrole"
%token T_ENDVAR "endvar"
%token T_EXPO "expo"
%token T_NONNEGNUMBER "non-negative number"
%token T_FOR "for"
%token T_GROUP "group"
%token T_IDENTIFIER "any identifier"
%token T_IMPORT "import"
%token T_INCL "incl"
%token T_INIT "init"
%token T_MAP "map"
%token T_MATH "math"
%token T_MODEL "model"
%token T_MULT "mult"
%token T_NAME "name"
%token T_NAMESPACE "namespace"
%token T_OFF "off"
%token T_PIECEWISE "piecewise"
%token T_PREF "pref"
%token T_PRIV "priv"
%token T_PUB "pub"
%token T_QUOTED "a quoted string"
%token T_REACT "react"
%token T_ROLE "role"
%token T_STOICHIO "stoichio"
%token T_THEN "then"
%token T_TYPE "type"
%token T_UNIT "unit"
%token T_VAR "var"
%token T_VARS "vars"
%token T_WITH "with"
%token T_OR "or"
%token T_AND "and"
%token T_EQEQ "=="
%token T_NEQ "<>"
%token T_GE ">="
%token T_LE "<="
%token T_NOT "not"
%token T_DIFF "d("
%token T_PARTIALDIFF "del("

%left '=' T_EQEQ T_NEQ
%left T_AND
%left T_OR
%left T_GE T_LE '<' '>'
%left '+' '-'
%left '*' '/' T_DIFF T_PARIALDIFF
%left T_IDENTIFIER
%left T_NOT
%right VT_UNARY
%left '(' ')' '{' '}'
%%

valid: model {
  if (!aParseTarget->assertType("CellML"))
    return 1;
} | math_expr {
  if (!aParseTarget->assertType("MathML"))
    return 1;
  static_cast<TeLICeMSParseMathML*>(aParseTarget)->mElement->
    appendChild($1.math())->release_ref();
};

model: modelstart modellist T_ENDDEF ';' ;

modelstart: T_DEF T_MODEL T_IDENTIFIER T_AS {
  // Set the name of the model
  std::wstring modelName($3.widestring());
  static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->name(modelName.c_str());
};

modellist: modellist T_DEF modeldefsomething {
    static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
      ->addElement($3.element());
  } | /* empty */ {};
modeldefsomething: units | comp | group | map | import;

import: importstart importlist T_ENDDEF ';' { $$ = $2; };

importstart: T_IMPORT T_QUOTED T_AS {
  // We are importing something, which means the model should be a CellML 1.1
  // model (as opposed to a CellML 1.0 model, which is what it is by default)
  RETURN_INTO_WSTRING(v, static_cast<TeLICeMSParseCellML*>(aParseTarget)->
                      mModel->cellmlVersion());

  if (v == L"1.0")
  {
    RETURN_INTO_OBJREF(altVer, iface::cellml_api::Model,
                       static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                       getAlternateVersion(L"1.1"));

    static_cast<CDA_TeLICeMModelResult*>(
      static_cast<CDA_TeLICeMResultBase*>(
        static_cast<TeLICeMSParseCellML*>(aParseTarget)->mResult))->mModel = altVer;
    static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel = altVer;
  }

  // Create the import definition
  RETURN_INTO_OBJREF(imp,
                     iface::cellml_api::CellMLImport,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->createCellMLImport());

  // Check that there is actually a URI
  std::wstring uri($2.widestring());

  if (uri == L"")
    YYABORT;

  // Specify the URI for the import definition
  RETURN_INTO_OBJREF(xlinkHref, iface::cellml_api::URI, imp->xlinkHref());

  xlinkHref->asText(uri.c_str());
  $$.element(imp);
};

importlist: importlist importsomething {
  $$ = $1;
  $$.element()->addElement($2.element());
 } | /* empty */ { $$ = $0; };

importsomething: importunit | importcomp;

importunit: T_UNIT T_IDENTIFIER T_AS T_IDENTIFIER ';' {
  // Create the units import
  RETURN_INTO_OBJREF(importUnits, iface::cellml_api::ImportUnits,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                     ->createImportUnits());

  // Set the name of the reference and current units
  std::wstring unitsRef($2.widestring());
  importUnits->unitsRef(unitsRef.c_str());

  std::wstring unitsName($4.widestring());
  importUnits->name(unitsName.c_str());

  $$.element(importUnits);
};

importcomp: T_COMP T_IDENTIFIER T_AS T_IDENTIFIER ';' {
  // Create the component import
  RETURN_INTO_OBJREF(importComponent, iface::cellml_api::ImportComponent,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                       ->createImportComponent()
                    );

  // Set the name of the reference and current components
  std::wstring componentRef($2.widestring());
  importComponent->componentRef(componentRef.c_str());

  std::wstring componentName($4.widestring());
  importComponent->name(componentName.c_str());

  $$.element(importComponent);
};

units: unitsstart unitsbody ';';

unitsbody: T_BASE T_UNIT {
  // Make the units definition a base unit definition
  DECLARE_QUERY_INTERFACE_OBJREF(u, $0.element(), cellml_api::Units);
  u->isBaseUnits(true);
} | T_DEF unitlist T_ENDDEF ;

unitsstart: T_UNIT T_IDENTIFIER T_AS {
  // Create the units definition
  RETURN_INTO_OBJREF(cellmlUnits, iface::cellml_api::Units,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                       ->createUnits());

  // Set the name of the units definition
  std::wstring unitsName($2.widestring());
  cellmlUnits->name(unitsName.c_str());

  $$.element(cellmlUnits);
};

unitlist: unitlist unititem { $1.element()->addElement($2.element()); $$ = $1; }
  | /* empty */ { $$ = $-1; };

unititem: unititemstart unititemattributes ';' ;

unititemstart: T_UNIT T_IDENTIFIER {
  // Create the unit item
  RETURN_INTO_OBJREF(cellmlUnit, iface::cellml_api::Unit,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                     ->createUnit());

  // Set the units that the unit item refers to
  std::wstring unitItemUnits($2.widestring());
  cellmlUnit->units(unitItemUnits.c_str());
  $$.element(cellmlUnit);
};

unititemattributes: '{'  unitattributelist '}' |
  /* empty */;

unitattributelist: unitattributelist ',' unitattributeitem |
                   unitattributeitem { $$ = $-1; };

anynumber: T_NONNEGNUMBER | '-' T_NONNEGNUMBER { $$.number(-$2.number()); };

siPrefixOrNumber: T_SIPREFIX { $$.number($1.siPrefix()); } | anynumber;

unitattributeitem: T_PREF ':' siPrefixOrNumber {
  // Set the prefix for the unit item
  DECLARE_QUERY_INTERFACE_OBJREF(unit, $-1.element(), cellml_api::Unit);
  unit->prefix(static_cast<int32_t>($3.number()));
} | T_EXPO ':' anynumber {
  // Set the exponent for the unit item
  DECLARE_QUERY_INTERFACE_OBJREF(unit, $-1.element(), cellml_api::Unit);
  unit->exponent($3.number());
} | T_MULT ':' anynumber {
  // Set the multiplier for the unit item
  DECLARE_QUERY_INTERFACE_OBJREF(unit, $-1.element(), cellml_api::Unit);
  unit->multiplier($3.number());
} | T_OFF ':' anynumber {
  // Set the offset for the unit item
  DECLARE_QUERY_INTERFACE_OBJREF(unit, $-1.element(), cellml_api::Unit);
  unit->offset($3.number());
};


comp: compstart complist T_ENDDEF ';';

compstart: T_COMP T_IDENTIFIER T_AS {
  // Create the component definition
  RETURN_INTO_OBJREF(component, iface::cellml_api::CellMLComponent,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                       ->createComponent());

  // Set the name of the component definition
  std::wstring componentName($2.widestring());
  component->name(componentName.c_str());

  $$.element(component);
};

complistdefsomething: units { $-1.element()->addElement($1.element()); } | math {
    DECLARE_QUERY_INTERFACE_OBJREF(mathCont, $-1.element(), cellml_api::MathContainer);
    mathCont->addMath($1.mathMath());
  } | reaction  { $-1.element()->addElement($1.element()); };
complistsomething: T_DEF complistdefsomething { $$ = $2; }
  | var { $0.element()->addElement($1.element()); };

complist: complist complistsomething {
  $$ = $1;
 } | /* empty */ { $$ = $0; };

var: varstart varparams ';' ;

varstart: T_VAR T_IDENTIFIER ':' T_IDENTIFIER {
  // Create the variable definition
  RETURN_INTO_OBJREF(cellmlVariable,
                     iface::cellml_api::CellMLVariable,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                     createCellMLVariable());

  // Set the variable and units name of the variable definition
  std::wstring variableName($2.widestring());
  cellmlVariable->name(variableName.c_str());

  std::wstring unitsName($4.widestring());
  cellmlVariable->unitsName(unitsName.c_str());

  $$.element(cellmlVariable);
};

varparams: '{' varparamlist '}'
         | /* empty */ ;

varparamlist: varparamlist ',' varparamitem
              | varparamitem { $$ = $-1; };

varparamitem: T_INIT ':' varparamiteminitval {
  // Set the initial value for the variable definition
  std::wstring ivText($3.widestring());
  DECLARE_QUERY_INTERFACE_OBJREF(cv, $-1.element(), cellml_api::CellMLVariable);
  cv->initialValue(ivText.c_str());
} | T_PUB ':' T_INTERFACETYPE {
  // Set the public interface for the variable definition
  DECLARE_QUERY_INTERFACE_OBJREF(cv, $-1.element(), cellml_api::CellMLVariable);
  cv->publicInterface($3.variableInterfaceType());
} | T_PRIV ':' T_INTERFACETYPE {
  // Set the public interface for the variable definition
  DECLARE_QUERY_INTERFACE_OBJREF(cv, $-1.element(), cellml_api::CellMLVariable);
  cv->privateInterface($3.variableInterfaceType());
};

varparamiteminitval: anynumber {
  char buf[30];
  any_snprintf(buf, sizeof(buf), "%g", $1.number());
  $$.string(buf);
} | T_IDENTIFIER;

math: mathstart math_list T_ENDDEF ';' {
  $$ = $1;
  for (std::list<iface::mathml_dom::MathMLContentElement*>::const_iterator i =
         $2.mathList().begin(); i != $2.mathList().end(); i++)
    $$.mathMath()->appendChild(*i)->release_ref();
 };

mathstart: T_MATH T_AS {
  // Create a math container...
  RETURN_INTO_OBJREF(mathel, iface::dom::Element,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->createMathElement());
  $$.mathMathFromDOM(mathel);
};

math_list: math_list math_expr ';' {
  $$ = $1;
  $$.addToMathList($2.math());
} | /* empty */ { $$.makeMathList(); };

math_attrs: '{' math_attr_list '}' {
  $$ = $2;
} | /* empty */ {
  std::map<std::string, std::string> empty;
  $$.propertyMap(empty);
};

math_attr_list: math_attr_list2 math_attr { $$ = $2; } | /* empty */ {
  std::map<std::string, std::string> empty;
  $$.propertyMap(empty);
};

math_attr_list2: math_attr_list2 math_attr ',' { $$ = $2; } | /* empty */ {
  std::map<std::string, std::string> empty;
  $$.propertyMap(empty);
};

math_attr: math_attr_id ':' math_attr_value {
  $$ = $0;
  $$.propertyMap().insert(std::pair<std::string, std::string>($1.string(), $3.string()));
};

math_attr_id: T_IDENTIFIER | T_QUOTED | T_BASE { $$.string("base"); } |
              T_TYPE { $$.string("type"); } | T_UNIT { $$.string("units"); };

math_attr_value: T_QUOTED | T_IDENTIFIER | anynumber {
    char buf[30];
    any_snprintf(buf, sizeof(buf), "%g", $1.number());
    $$.string(buf);
  };

math_expr: T_IDENTIFIER math_attrs math_maybefunction_args {
  if ($3.mOverrideBuiltin)
  {
    RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                       DoFunction($1.string(), $3.mathList(), aParseTarget,
                                  $2.propertyMap()));
    $$.math(m);
  }
  else
  {
    RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                       MakeCIOrConstant($1.string(), $1.mOverrideBuiltin,
                                        $2.propertyMap(), aParseTarget));
    $$.math(m);
  }
} | '(' math_expr ')' {
  $$ = $2;
  $$.mIndirect = true;
} | math_expr additive_op math_attrs math_expr %prec '+' {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoInorderExpression($2.string(), $1, $4, aParseTarget,
                                         $3.propertyMap(), $2.string() == "plus"));
  $$.math(m);
} | math_expr multiplicative_op math_attrs math_expr %prec '*' {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoInorderExpression($2.string(), $1, $4, aParseTarget,
                                         $3.propertyMap(), $2.string() == "times"));
  $$.math(m);
} | math_expr comparative_op_lowerprec math_attrs math_expr %prec T_EQEQ {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoInorderExpression($2.string(), $1, $4, aParseTarget,
                                         $3.propertyMap(), true));
  $$.math(m);
} | math_expr comparative_op_higherprec math_attrs math_expr %prec T_GE {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoInorderExpression($2.string(), $1, $4, aParseTarget,
                                         $3.propertyMap(), true));
  $$.math(m);
} | math_expr T_AND math_attrs math_expr {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoInorderExpression("and", $1, $4, aParseTarget,
                                         $3.propertyMap(), true));
  $$.math(m);
} | math_expr T_OR math_attrs math_expr {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoInorderExpression("or", $1, $4, aParseTarget,
                                         $3.propertyMap(), true));
  $$.math(m);
} | T_NONNEGNUMBER math_attrs {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     MakeConstant($1.number(), aParseTarget, $2.propertyMap()));
  $$.math(m);
} | T_PIECEWISE math_attrs '(' piecewise_case_list piecewise_maybe_else ')' {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     MakePiecewiseElement($4.mathList(), $5.math(),
                                          $2.propertyMap(), aParseTarget));
  $$.math(m);
} | unary_op math_attrs math_expr %prec VT_UNARY {
  bool success = false;
  if (!$3.mIndirect)
  {
    bool isMinus = $1.string() == "minus", isPlus = $1.string() == "plus";
    if (isMinus || isPlus)
    {
      DECLARE_QUERY_INTERFACE_OBJREF(cn, $3.math(), mathml_dom::MathMLCnElement);
      if (cn != NULL)
      {
        RETURN_INTO_OBJREF(ma1, iface::dom::Node, cn->getArgument(1));
        DECLARE_QUERY_INTERFACE_OBJREF(t, ma1, dom::Text);
        RETURN_INTO_WSTRING(td, t->data());
        if (isMinus)
          td = L"-" + td;
        t->data(td.c_str());
        $$ = $3;
        success = true;
      }
    }
  }
  if (!success)
  {
    RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                       MakePredefinedWrapElement($1.string(), $3.math(), $2.propertyMap(),
                                                 aParseTarget));
    $$.math(m);
  }
} | T_DIFF math_expr ')' '/' T_DIFF math_expr math_possible_degree ')' math_attrs {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoDerivative(L"diff", $2.math(), $6.math(), $7.math(),
                                  $9.propertyMap(), aParseTarget));
  $$.math(m);
} | T_PARTIALDIFF math_expr ')' '/' T_PARTIALDIFF math_expr math_possible_degree ')' math_attrs {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     DoDerivative(L"partialdiff", $2.math(), $6.math(), $7.math(),
                                  $9.propertyMap(), aParseTarget));
  $$.math(m);
};

additive_op: '+' { $$.string("plus"); } | '-' { $$.string("minus"); };
multiplicative_op: '*' { $$.string("times"); } | '/' { $$.string("divide"); };
comparative_op_lowerprec: '=' { $$.string("eq"); } |
            T_EQEQ { $$.string("eq"); } | T_NEQ { $$.string("neq"); }
comparative_op_higherprec: '<' { $$.string("lt"); } | '>' { $$.string("gt"); } |
                          T_GE { $$.string("geq");} | T_LE { $$.string("leq"); };

unary_op: '-' { $$.string("minus"); } | T_NOT { $$.string("not"); } | '+' { $$.string("plus"); };

math_maybefunction_args: '(' math_function_arg_list ')' {
  $$ = $2;
  $$.mOverrideBuiltin = true;
} | /* empty */ {
  $$.mOverrideBuiltin = false;
};

math_function_arg_list: math_function_arg_list ',' math_expr {
  $$ = $1;
  $$.addToMathList($3.math());
} | math_expr {
  $$.makeMathList();
  $$.addToMathList($1.math());
};

piecewise_case_list: piecewise_case_list piecewise_case_pair {
  $$ = $1;
  $$.addToMathList($2.math());
} | /* empty */ {
  $$.makeMathList();
};

piecewise_case_pair: T_CASE math_attrs math_expr T_THEN math_expr {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     MakeCaseElement($3.math(), $5.math(), aParseTarget,
                                     $2.propertyMap()));
  $$.math(m);
};

piecewise_maybe_else: T_ELSE math_attrs math_expr {
  RETURN_INTO_OBJREF(m, iface::mathml_dom::MathMLContentElement,
                     MakeOtherwiseElement($3.math(), $2.propertyMap(),
                                          aParseTarget));
  $$.math(m);
} | /* empty */ {
  $$.math(NULL);
};

math_possible_degree: ',' math_expr {
  $$ = $2;
}| /* empty */ {
  $$.math(NULL);
};

reaction: reaction_start reaction_list T_ENDDEF ';';

reaction_start: T_REVERSIBILITY T_REACT T_FOR {
  // Create the reaction definition

  RETURN_INTO_OBJREF(reaction, iface::cellml_api::Reaction,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->createReaction());

  // Set the type of the reaction
  reaction->reversible($1.boolean());

  $$.element(reaction);
};

reaction_list: reaction_list reaction_statement {
  $$ = $1;
  $$.element()->addElement($2.element());
} | /* empty */ { $$ = $0; };

reaction_statement: reaction_statement_start role_list T_ENDVAR ';';
reaction_statement_start: T_VAR T_IDENTIFIER T_WITH {
  // Create the variable reference
  RETURN_INTO_OBJREF(varRef, iface::cellml_api::VariableRef,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                     createVariableRef());

  // Set the name of the variable reference
  std::wstring varName($2.widestring());
  varRef->variableName(varName.c_str());

  $$.element(varRef);
};

role_list: role_list role {
  $$ = $1;
  $$.element()->addElement($2.element());
} | /* empty */ { $$ = $0; };

role: role_start role_attributes role_maybe_with ';';

role_start: T_ROLE T_ROLETYPE {
  // Create the role
  RETURN_INTO_OBJREF(role, iface::cellml_api::Role,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                     createRole());

  // Set the type of the role
  role->variableRole($2.reactionRoleType());

  $$.element(role);
};

role_maybe_with: T_WITH role_math_statements T_ENDROLE | /* empty */;

role_attributes: '{' role_attribute_list '}' { $$ = $0; }
 | /* empty */ { $$ = $0; };

role_attribute_list: role_attribute_list ',' role_attribute_item
  | role_attribute_item { $$ = $-1; };

role_attribute_item: T_DIR ':' T_DIRECTIONTYPE {
  // Set the direction of the role
  DECLARE_QUERY_INTERFACE_OBJREF(role, $-1.element(), cellml_api::Role);
  role->direction($3.reactionDirectionType());
} | T_DVAR ':' T_IDENTIFIER {
  // Set the delta variable of the role
  std::wstring deltaVariableName($3.widestring());
  DECLARE_QUERY_INTERFACE_OBJREF(role, $-1.element(), cellml_api::Role);
  role->deltaVariableName(deltaVariableName.c_str());
} | T_STOICHIO ':' anynumber {
  DECLARE_QUERY_INTERFACE_OBJREF(role, $-1.element(), cellml_api::Role);
  // Set the sotichiometry of the role
  role->stoichiometry($3.number());
};

role_math_statements: role_math_statements T_DEF math {
  $$ = $1;
  $$.addToMathList($3.math());
} | /* empty */ { $$ = $-1; };

group: group_start group_types group_list T_ENDDEF ';' ;

group_start: T_GROUP T_AS {
  // Create the group definition
  RETURN_INTO_OBJREF(g, iface::cellml_api::Group,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                     createGroup());
  $$.element(g);
};

group_types: group_types group_type {
  $$ = $1;
  $$.element()->addElement($2.element());
} | /* empty */ { $$ = $0; };

group_type: group_type_start group_type_attributes ';';

group_type_start: T_TYPE T_IDENTIFIER group_maybenamespace {
  // Create the relationship reference for the group
  RETURN_INTO_OBJREF(rr, iface::cellml_api::RelationshipRef,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                     ->createRelationshipRef());

  std::wstring name($2.widestring()), ns($3.widestring());
  rr->setRelationshipName(ns.c_str(), name.c_str());
  $$.element(rr);
};

group_maybenamespace: T_NAMESPACE T_QUOTED { $$ = $2; } |
/* empty */ { $$.string(""); } ;

group_type_attributes: T_WITH T_NAME T_IDENTIFIER {
  DECLARE_QUERY_INTERFACE_OBJREF(rr, $0.element(), cellml_api::RelationshipRef);
  std::wstring name($3.widestring());
  rr->name(name.c_str());
} | /* empty */ ;

group_list: group_list group_item {
  $$ = $1;
  $$.element()->addElement($2.element());
} | /* empty */ { $$ = $0; };

group_item: group_item_start group_maybe_incl ';' {
  $$ = $1;
};

group_maybe_incl: group_list_incl group_list T_ENDCOMP | /* empty */;
group_list_incl: T_INCL { $$ = $0; };

group_item_start: T_COMP T_IDENTIFIER {
  // Create the component reference for the group
  RETURN_INTO_OBJREF(cr, iface::cellml_api::ComponentRef,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                     createComponentRef());

  // Set the name of the component reference
  std::wstring componentRefName($2.widestring());
  cr->componentName(componentRefName.c_str());
  $$.element(cr);
};

map: map_start map_list T_ENDDEF ';';

map_start: T_MAP T_BETWEEN T_IDENTIFIER T_AND T_IDENTIFIER T_FOR {
  // Create the connection definition
  RETURN_INTO_OBJREF(conn, iface::cellml_api::Connection,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel->
                     createConnection());

  RETURN_INTO_OBJREF(cm, iface::cellml_api::MapComponents,
                     conn->componentMapping());
  std::wstring cname1($3.widestring()), cname2($5.widestring());
  cm->firstComponentName(cname1.c_str());
  cm->secondComponentName(cname2.c_str());

  $$.element(conn);
};

map_list: map_list map_item {
  $$ = $1;
  $$.element()->addElement($2.element());
} | /* empty */ { $$ = $0; };

map_item: T_VARS T_IDENTIFIER T_AND T_IDENTIFIER ';' {
  // Create for the map variables for the connection
  RETURN_INTO_OBJREF(mapvar, iface::cellml_api::MapVariables,
                     static_cast<TeLICeMSParseCellML*>(aParseTarget)->mModel
                     ->createMapVariables());

  // Set the name of the first and second variables for the map variables
  std::wstring vname1($2.widestring()), vname2($4.widestring());
  mapvar->firstVariableName(vname1.c_str());
  mapvar->secondVariableName(vname2.c_str());

  $$.element(mapvar);
};
