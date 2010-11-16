#include "TeLICeMSImpl.hxx"
#include "CellMLBootstrap.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include <sstream>

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
  wchar_t* showMaths(iface::mathml_dom::MathMLElement* aMaths)
    throw();
};

int telicem_parse(
                  iface::dom::Document *aDocument,
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
  DECLARE_QUERY_INTERFACE_OBJREF(cde, m, cellml_api::CellMLDOMElement);
  RETURN_INTO_OBJREF(el, iface::dom::Element, cde->domElement());
  RETURN_INTO_OBJREF(doc, iface::dom::Document, el->ownerDocument());

  TeLICeMSParseCellML parseCellML(m);
  RETURN_INTO_OBJREF(res, CDA_TeLICeMModelResult, parseCellML.result());

  size_t l = wcslen(aModelText);
  char* s = new char[l + 1];
  if (wcstombs(s, aModelText, l) != l)
  {
    delete [] s;
    res->addMessage(L"The input model text could not be converted to UTF8.");
    res->add_ref();
    return res;
  }
  std::stringstream str(s);
  delete [] s;

  TeLICeMStateScan lex(&str, &parseCellML);
  
  int ret = telicem_parse(doc, &lex, &parseCellML);

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
  if (wcstombs(s, aMathText, l) != l)
  {
    delete [] s;
    res->addMessage(L"The input math text could not be converted to UTF8.");
    res->add_ref();
    return res;
  }
  std::stringstream str(s);
  delete [] s;

  TeLICeMStateScan lex(&str, &parseMath);
  
  int ret = telicem_parse(aDoc, &lex, &parseMath);

  if (ret == 2)
    res->addMessage(L"There was insufficient memory to complete the parse.");
  else if (ret == 1)
    res->addMessage(L"Parsing failed due to a syntax error (see above).");

  res->add_ref();
  return res;
}

wchar_t*
CDA_TeLICeMService::showModel(iface::cellml_api::Model* aModel)
  throw()
{
  return CDA_wcsdup(L"TODO");
}

wchar_t*
CDA_TeLICeMService::showMaths(iface::mathml_dom::MathMLElement* aMaths)
  throw()
{
  return CDA_wcsdup(L"TODO");
}

TeLICeMStateScan::TeLICeMStateScan(std::stringstream* aStr,
                                   TeLICeMSParseTarget* aTarg)
  : TeLICeMFlexLexer(aStr), mTarget(aTarg), mEOF(false)
{
}

void
TeLICeMStateScan::adjustColumn()
{
  if (mThisLexIsNewRow)
    mThisLexIsNewRow = false;
  else
    mTarget->mColumn += yyleng;
}

void
TeLICeMStateScan::newRow()
{
  mTarget->mColumn = 0;
  mThisLexIsNewRow = true;
  mTarget->mRow++;
}

void
TeLICeMStateScan::lexerError(const char* m)
{
  mTarget->mResult->addMessage(convertStringToWString(m));
}

int TeLICeMS_lex(TeLICeMSLValue* aLValue, TeLICeMStateScan* aLexer)
{
  aLexer->mLValue = aLValue;

  int val = aLexer->yylex();
  aLexer->adjustColumn();

  aLexer->mEOF = (val == 0);

  return val;
}

iface::cellml_services::TeLICeMService*
CreateTeLICeMService(void)
{
  return new CDA_TeLICeMService();
}
