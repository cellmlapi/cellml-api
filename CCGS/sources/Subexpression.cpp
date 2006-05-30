/*
 * Note: This code is not currently used, and probably won't compile. It is
 * just dumped here because it will probably be needed again in the future
 * (and it hasn't been committed anywhere yet).
 */
#if 0
class MathMLEquation
{
public:
  MathMLSubexpression(iface::mathml_dom::MathMLElement* aEl)
    : mEl(aEl)
  {
    // See if this is an apply...
    DECLARE_QUERY_INTERFACE_OBJREF(apply, mEl, mathml_dom::MathMLApplyElement);
    if (apply == NULL)
    {
      // A ci?
      DECLARE_QUERY_INTERFACE_OBJREF(ci, mEl, mathml_dom::MathMLCiElement);
      if (ci == NULL)
      {
        // Well, maybe a cn?
        DECLARE_QUERY_INTERFACE_OBJREF(cn, mEl, mathml_dom::MathMLCnElement);
        if (cn == NULL)
        {
          // Perhaps a piecewise?
          DECLARE_QUERY_INTERFACE_OBJREF(pw, mEl, mathml_dom::MathMLPiecewiseElement);
          if (pw == NULL)
          {
            HandleMisc(aEl);
          }
          else
            HandlePiecewise(pw);
        }
        else
          HandleCn(cn);
      }
      else
        HandleCi(ci);
      return 0;
    }
    else
      HandleApply(apply);
  }

  ~MathMLSubExpression()
  {
    std::vector<MathMLSubExpression*>::iterator i;
    for (i = mSubExpressions.begin(); i != mSubExpressions.end(); i++)
      (*i)->release_ref();
  }

  uint32_t
  ComputeSubExpressions()
  {
    if (mComputedSubExpressions)
      return mSubExpressions.size();
    mComputedSubExpressions = true;

    uint32_t count = 0;

    // If we are not an apply, we have no subexpressions...
    DECLARE_QUERY_INTERFACE_OBJREF(apply, mEl, mathml_dom::MathMLApplyElement);
    if (apply == NULL)
      return 0;
    
    mOp = apply->_cxx_operator();
    // We don't hold it, because the DOM can hold us and we don't want to
    // create a loop.
    mOp->release_ref();

    // Next, determine if we should order the 

    return count;
  }

  MathMLSubexpression*
  GetSubexpression(uint32_t i)
  {
    return mSubExpressions[i];
  }

  // If we knew the value of this subexpression, would it be worth storing it,
  // or would it be faster just to recompute it when required?
  bool
  IsWorthStoring()
  {
    return true;
  }

private:
  enum
  {
    CI,
    CN,
    BUILTIN_CONSTANT,
    PIECEWISE,
    DEFINITE_INTEGRAL,
    RATE,
    OPERATION
  } Type;

  void HandleApply(iface::mathml_dom::MathMLApplyElement* aApply)
  {
  }

  void HandleCi(iface::mathml_dom::MathMLCiElement* aCi)
  {
  }

  void HandleCn(iface::mathml_dom::MathMLCnElement* aCn)
  {
  }

  void HandlePiecewise(iface::mathml_dom::MathMLPiecewiseElement* aPw)
  {
    Type = PIECEWISE;
    RETURN_INTO_OBJREF(ow, iface::mathml_dom::MathMLContentElement,
                       aPw->otherwise());
    if (ow != NULL)
      mSubExpressions.push_back(new MathMLSubExpression(ow));
  }

  void HandleMisc(iface::mathml_dom::MathMLElement* aEl)
  {
    // See if it is a builtin constant...
    RETURN_INTO_WSTRING(ln, aEl->localName());
    mWorthStoring = false;
    bool foundConstant = false;
#define FCONST(x,v) if (ln == x) \
    { \
      mConstantValue = v; \
      foundConstant = true; \
    }

    Type = BUILTIN_CONSTANT;

    FCONST(L"exponentiale", "2.71828182845905")
    else FCONST(L"notanumber", "nan()")
    else FCONST(L"true", "1.0")
    else FCONST(L"false", "1.0")
    else FCONST(L"pi", "3.14159265358979")
    else FCONST(L"eulergamma", "0.577215664901533")
    else FCONST(L"infinity", "infinity()")
    else
    {
      std::wstring emsg = L"Unrecognised MathML element: ";
      emsg += ln;
      throw CodeGenerationError(emsg);
    }
  }
  
  iface::mathml_dom::MathMLElement* mEl;
  iface::mathml_dom::MathMLElement* mOp;
  OpInfo* opInformation;
  std::string mConstantValue;
  bool mWorthStoring;
  std::vector<MathMLSubexpression*> mSubExpressions;
};
#endif
