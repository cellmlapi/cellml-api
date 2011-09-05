#ifndef _INCLUDED_TELICEMS_IMPL_HXX
#define _INCLUDED_TELICEMS_IMPL_HXX

#define IN_TeLICeMS_MODULE
#include "TeLICeMService.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include <string>
#define yyFlexLexer TeLICeMFlexLexer
#include "FlexLexer.h"
#include "Utilities.hxx"

#define MAGIC_NEWLINE 9999
#define MAGIC_WHITESPACE 9998

class CDA_TeLICeMResultBase
  : public virtual iface::cellml_services::TeLICeMResult
{
public:
  CDA_TeLICeMResultBase() {}
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  void
  expect(const std::wstring& aExpect)
  {
    mExpecting = aExpect;
  }

  void
  addMessage(const std::wstring& aMsg)
  {
    mMessage += aMsg;
    mMessage += L"\n";
  }

  wchar_t*
  errorMessage() throw()
  {
    return CDA_wcsdup(mMessage.c_str());
  }

private:
  std::wstring mMessage, mExpecting;
};

class CDA_TeLICeMModelResult
  : public CDA_TeLICeMResultBase, public iface::cellml_services::TeLICeMModelResult
{
public:
  CDA_TeLICeMModelResult(iface::cellml_api::Model* aModel)
    : mModel(aModel)
  {
  }

  CDA_IMPL_QI2(cellml_services::TeLICeMResult, cellml_services::TeLICeMModelResult);

  iface::cellml_api::Model*
  modelResult() throw()
  {
    mModel->add_ref();
    return mModel;
  }

  ObjRef<iface::cellml_api::Model> mModel;
};

class TeLICeMSParseTarget
{
public:
  typedef enum {
    TargetMathML,
    TargetCellML
  } Target;


  TeLICeMSParseTarget()
    : mRow(1), mColumn(0), mGroupingLevel(0)
  {
  }
  ~TeLICeMSParseTarget() {}

  virtual Target target() const = 0;
  virtual const char* type() = 0;

  bool assertType(const char* match)
  {
    if (!strcmp(match, type()))
      return true;

    wchar_t msg[80];
    any_swprintf(msg, 80, L"Was trying to parse TeLICEMS input for %8s, but got syntax for %8s.",
             type(), match);
    mResult->addMessage(msg);
    return false;
  }

  virtual iface::dom::Document* document() = 0;

  ObjRef<CDA_TeLICeMResultBase> mResult;
  int mRow, mColumn;
  int mGroupingLevel;
};

class TeLICeMSParseCellML
  : public TeLICeMSParseTarget
{
public:
  TeLICeMSParseCellML(iface::cellml_api::Model* aModel) : mModel(aModel)
  {
    mResult = already_AddRefd<CDA_TeLICeMModelResult>
      (new CDA_TeLICeMModelResult(aModel));
  }

  Target target() const { return TeLICeMSParseTarget::TargetCellML; }
  const char* type() { return "CellML"; }

  // Not AddRefd - AddRef required if this is to leave the parser.
  iface::cellml_api::Model* model() const { return mModel; }

  iface::cellml_services::TeLICeMModelResult* result() const
  {
    DECLARE_QUERY_INTERFACE(ret, mResult, cellml_services::TeLICeMModelResult);
    return ret;
  }

  iface::dom::Document* document()
  {
    DECLARE_QUERY_INTERFACE_OBJREF(de, mModel, cellml_api::CellMLDOMElement);
    RETURN_INTO_OBJREF(el, iface::dom::Element, de->domElement());
    return el->ownerDocument();
  }

  // Precondition: mModel guaranteed to outlive this object.
  iface::cellml_api::Model* mModel;
};

class CDA_TeLICeMMathResult
  : public CDA_TeLICeMResultBase, public iface::cellml_services::TeLICeMMathResult
{
public:
  CDA_TeLICeMMathResult(iface::mathml_dom::MathMLMathElement* aElement)
    : mElement(aElement)
  {
  }

  CDA_IMPL_QI2(cellml_services::TeLICeMResult, cellml_services::TeLICeMMathResult);

  iface::mathml_dom::MathMLMathElement*
  mathResult() throw()
  {
    mElement->add_ref();
    return mElement;
  }
private:
  ObjRef<iface::mathml_dom::MathMLMathElement> mElement;
};

class TeLICeMSParseMathML
  : public TeLICeMSParseTarget
{
public:
  TeLICeMSParseMathML(iface::mathml_dom::MathMLMathElement* aElement) : mElement(aElement)
  {
    mResult = already_AddRefd<CDA_TeLICeMMathResult>
      (new CDA_TeLICeMMathResult(aElement));
  }
  Target target() const { return TeLICeMSParseTarget::TargetMathML; }
  const char* type() { return "MathML"; }

  // Not AddRefd - AddRef required if this is to leave the parser.
  iface::mathml_dom::MathMLMathElement* mathElement() const { return mElement; }

  iface::cellml_services::TeLICeMMathResult* result() const
  {
    DECLARE_QUERY_INTERFACE(ret, mResult, cellml_services::TeLICeMMathResult);
    return ret;
  }

  iface::dom::Document* document()
  {
    return mElement->ownerDocument();
  }

  // Precondition: mElement guaranteed to outlive this object.
  iface::mathml_dom::MathMLMathElement* mElement;
};

#include "TeLICeMStateScan.hxx"
#endif // _INCLUDED_TELICEMS_IMPL_HXX
