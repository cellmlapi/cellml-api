#include <exception>
#include <inttypes.h>
#include "IfaceMathML-content-APISPEC.hxx"
#include "DOMImplementation.hxx"
#include <map>

class CDA_MathMLDocument
  : public virtual iface::mathml_dom::MathMLDocument,
    public virtual CDA_Document
{
public:
  CDA_MathMLDocument(GdomeDocument* d);
  virtual ~CDA_MathMLDocument() {};

  CDA_IMPL_QI3(dom::Node, dom::Document, mathml_dom::MathMLDocument)

  wchar_t* referrer() throw(std::exception&);
  wchar_t* domain() throw(std::exception&);
  wchar_t* URI() throw(std::exception&);
};

class CDA_MathMLElement
  : public virtual iface::mathml_dom::MathMLElement,
    public virtual CDA_Element
{
public:
  CDA_MathMLElement(GdomeElement* el);

  CDA_IMPL_QI3(dom::Node, dom::Element, mathml_dom::MathMLElement)

  wchar_t* className() throw(std::exception&);
  void className(const wchar_t* attr) throw(std::exception&);
  wchar_t* mathElementStyle() throw(std::exception&);
  void mathElementStyle(const wchar_t* attr) throw(std::exception&);
  wchar_t* id() throw(std::exception&);
  void id(const wchar_t* attr) throw(std::exception&);
  wchar_t* xref() throw(std::exception&);
  void xref(const wchar_t* attr) throw(std::exception&);
  wchar_t* href() throw(std::exception&);
  void href(const wchar_t* attr) throw(std::exception&);
  iface::mathml_dom::MathMLMathElement* ownerMathElement() throw(std::exception&);
};

class CDA_MathMLContainer
  : public virtual iface::mathml_dom::MathMLContainer,
    // Derive from this now, since we need it.
    public virtual iface::mathml_dom::MathMLElement
{
public:
  CDA_MathMLContainer();

  uint32_t nArguments() throw(std::exception&);
  iface::mathml_dom::MathMLNodeList* arguments() throw(std::exception&);
  iface::mathml_dom::MathMLNodeList* declarations() throw(std::exception&);
  iface::mathml_dom::MathMLElement* getArgument(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLElement* setArgument(iface::mathml_dom::MathMLElement* newArgument, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLElement* insertArgument(iface::mathml_dom::MathMLElement* newArgument, uint32_t index) throw(std::exception&);
  void deleteArgument(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLElement* removeArgument(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* getDeclaration(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* setDeclaration(iface::mathml_dom::MathMLDeclareElement* newDeclaration, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* insertDeclaration(iface::mathml_dom::MathMLDeclareElement* newDeclaration, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* removeDeclaration(uint32_t index) throw(std::exception&);
  void deleteDeclaration(uint32_t index) throw(std::exception&);
};

class CDA_MathMLMathElement
  : public virtual iface::mathml_dom::MathMLMathElement,
    public virtual CDA_MathMLElement,
    public virtual CDA_MathMLContainer
{
public:
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLMathElement)

  CDA_MathMLMathElement(GdomeElement* elem);

  wchar_t* macros() throw(std::exception&);
  void macros(const wchar_t* attr) throw(std::exception&);
  wchar_t* display() throw(std::exception&);
  void display(const wchar_t* attr) throw(std::exception&);
};

class CDA_MathMLContentElement
  : public virtual iface::mathml_dom::MathMLContentElement,
    public virtual CDA_MathMLElement
{
public:
  CDA_MathMLContentElement(GdomeElement* elem);
};

class CDA_MathMLContentToken
  : public virtual iface::mathml_dom::MathMLContentToken,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLContentToken(GdomeElement* elem);

  iface::mathml_dom::MathMLNodeList* arguments() throw(std::exception&);
  wchar_t* definitionURL() throw(std::exception&);
  void definitionURL(const wchar_t* attr) throw(std::exception&);
  wchar_t* encoding() throw(std::exception&);
  void encoding(const wchar_t* attr) throw(std::exception&);
  iface::dom::Node* getArgument(uint32_t index) throw(std::exception&);
  iface::dom::Node* insertArgument(iface::dom::Node* newArgument, uint32_t index) throw(std::exception&);
  iface::dom::Node* setArgument(iface::dom::Node* newArgument, uint32_t index) throw(std::exception&);
  void deleteArgument(uint32_t index) throw(std::exception&);
  iface::dom::Node* removeArgument(uint32_t index) throw(std::exception&);
};

class CDA_MathMLCnElement
  : public virtual iface::mathml_dom::MathMLCnElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_MathMLCnElement(GdomeElement* el);
  CDA_IMPL_QI6(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContentToken, mathml_dom::MathMLCnElement)

  wchar_t* type() throw(std::exception&);
  void type(const wchar_t* attr) throw(std::exception&);
  wchar_t* base() throw(std::exception&);
  void base(const wchar_t* attr) throw(std::exception&);
  uint32_t nargs() throw(std::exception&);
};

class CDA_MathMLCiElement
  : public virtual iface::mathml_dom::MathMLCiElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_IMPL_QI6(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContentToken, mathml_dom::MathMLCiElement)

  CDA_MathMLCiElement(GdomeElement* elem);

  wchar_t* type() throw(std::exception&);
  void type(const wchar_t* attr) throw(std::exception&);
};

class CDA_MathMLCsymbolElement
  : public virtual iface::mathml_dom::MathMLCsymbolElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_MathMLCsymbolElement(GdomeElement* elem);

  CDA_IMPL_QI6(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContentToken, mathml_dom::MathMLCsymbolElement)
};

class CDA_MathMLContentContainer
  : public virtual iface::mathml_dom::MathMLContentContainer,
    public virtual CDA_MathMLContentElement,
    public virtual CDA_MathMLContainer
{
public:
  CDA_MathMLContentContainer(GdomeElement* elem);

  CDA_IMPL_QI6(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer,
               mathml_dom::MathMLContentContainer)

  uint32_t nBoundVariables() throw(std::exception&);
  iface::mathml_dom::MathMLConditionElement* condition() throw(std::exception&);
  void condition(iface::mathml_dom::MathMLConditionElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* opDegree() throw(std::exception&);
  void opDegree(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* domainOfApplication() throw(std::exception&);
  void domainOfApplication(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* momentAbout() throw(std::exception&);
  void momentAbout(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* getBoundVariable(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* insertBoundVariable(iface::mathml_dom::MathMLBvarElement* newBVar, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* setBoundVariable(iface::mathml_dom::MathMLBvarElement* newBVar, uint32_t index) throw(std::exception&);
  void deleteBoundVariable(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* removeBoundVariable(uint32_t index) throw(std::exception&);
};

class CDA_MathMLApplyElement
  : public virtual iface::mathml_dom::MathMLApplyElement,
    public virtual CDA_MathMLContentContainer,
    public virtual CDA_Element
{
public:
  CDA_IMPL_QI7(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLApplyElement)

  CDA_MathMLApplyElement(GdomeElement* el);

  iface::mathml_dom::MathMLElement* _cxx_operator() throw(std::exception&);
  void _cxx_operator(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* lowLimit() throw(std::exception&);
  void lowLimit(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* upLimit() throw(std::exception&);
  void upLimit(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
};

class CDA_MathMLFnElement
  : public virtual iface::mathml_dom::MathMLFnElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLFnElement(GdomeElement* el);
  CDA_IMPL_QI7(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer,
               mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLFnElement)

  wchar_t* definitionURL() throw(std::exception&);
  void definitionURL(const wchar_t* attr) throw(std::exception&);
  wchar_t* encoding() throw(std::exception&);
  void encoding(const wchar_t* attr) throw(std::exception&);
};

class CDA_MathMLLambdaElement
  : public virtual iface::mathml_dom::MathMLLambdaElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLLambdaElement(GdomeElement* el);

  CDA_IMPL_QI7(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLLambdaElement)

  iface::mathml_dom::MathMLElement* expression() throw(std::exception&);
  void expression(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
};

class CDA_MathMLSetElement
  : public virtual iface::mathml_dom::MathMLSetElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLSetElement(GdomeElement* el);
  CDA_IMPL_QI7(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLSetElement)

  bool isExplicit() throw(std::exception&);
  wchar_t* type() throw(std::exception&);
  void type(const wchar_t* attr) throw(std::exception&);
};

class CDA_MathMLListElement
  : public virtual iface::mathml_dom::MathMLListElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLListElement(GdomeElement* el);
  CDA_IMPL_QI7(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLListElement)

  bool isExplicit() throw(std::exception&);
  wchar_t* ordering() throw(std::exception&);
  void ordering(const wchar_t* attr) throw(std::exception&);
};

class CDA_MathMLBvarElement
  : public virtual iface::mathml_dom::MathMLBvarElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLBvarElement(GdomeElement* el);
  CDA_IMPL_QI7(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLBvarElement)
};

class CDA_MathMLPredefinedSymbol
  : public virtual iface::mathml_dom::MathMLPredefinedSymbol,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLPredefinedSymbol(GdomeElement* el);

  wchar_t* definitionURL() throw(std::exception&);
  void definitionURL(const wchar_t* attr) throw(std::exception&);
  wchar_t* encoding() throw(std::exception&);
  void encoding(const wchar_t* attr) throw(std::exception&);
  wchar_t* arity() throw(std::exception&);
  wchar_t* symbolName() throw(std::exception&);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLPredefinedSymbol)
};

class CDA_MathMLTendsToElement
  : public virtual iface::mathml_dom::MathMLTendsToElement,
    public virtual CDA_MathMLPredefinedSymbol
{
public:
  CDA_MathMLTendsToElement(GdomeElement* el);
  CDA_IMPL_QI6(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLPredefinedSymbol,
               mathml_dom::MathMLTendsToElement)

  wchar_t* type() throw(std::exception&);
  void type(const wchar_t* attr) throw(std::exception&);
};

class CDA_MathMLIntervalElement
  : public virtual iface::mathml_dom::MathMLIntervalElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLIntervalElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLIntervalElement)

  wchar_t* closure() throw(std::exception&);
  void closure(const wchar_t* attr) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* start() throw(std::exception&);
  void start(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* end() throw(std::exception&);
  void end(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
};

class CDA_MathMLConditionElement
  : public virtual iface::mathml_dom::MathMLConditionElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLConditionElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLConditionElement)

  iface::mathml_dom::MathMLApplyElement* condition() throw(std::exception&);
  void condition(iface::mathml_dom::MathMLApplyElement* attr) throw(std::exception&);
};

class CDA_MathMLDeclareElement
  : public virtual iface::mathml_dom::MathMLDeclareElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLDeclareElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLDeclareElement)

  wchar_t* type() throw(std::exception&);
  void type(const wchar_t* attr) throw(std::exception&);
  uint32_t nargs() throw(std::exception&);
  void nargs(uint32_t attr) throw(std::exception&);
  wchar_t* occurrence() throw(std::exception&);
  void occurrence(const wchar_t* attr) throw(std::exception&);
  wchar_t* definitionURL() throw(std::exception&);
  void definitionURL(const wchar_t* attr) throw(std::exception&);
  wchar_t* encoding() throw(std::exception&);
  void encoding(const wchar_t* attr) throw(std::exception&);
  iface::mathml_dom::MathMLCiElement* identifier() throw(std::exception&);
  void identifier(iface::mathml_dom::MathMLCiElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* constructor() throw(std::exception&);
  void constructor(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
};

class CDA_MathMLVectorElement
  : public virtual iface::mathml_dom::MathMLVectorElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLVectorElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLVectorElement)

  uint32_t ncomponents() throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getComponent(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* insertComponent(iface::mathml_dom::MathMLContentElement* newComponent, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setComponent(iface::mathml_dom::MathMLContentElement* newComponent, uint32_t index) throw(std::exception&);
  void deleteComponent(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* removeComponent(uint32_t index) throw(std::exception&);
};

class CDA_MathMLMatrixElement
  : public virtual iface::mathml_dom::MathMLMatrixElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLMatrixElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLMatrixElement)

  uint32_t nrows() throw(std::exception&);
  uint32_t ncols() throw(std::exception&);
  iface::mathml_dom::MathMLNodeList* rows() throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* getRow(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* insertRow(iface::mathml_dom::MathMLMatrixrowElement* newRow, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* setRow(iface::mathml_dom::MathMLMatrixrowElement* newRow, uint32_t index) throw(std::exception&);
  void deleteRow(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* removeRow(uint32_t index) throw(std::exception&);
};

class CDA_MathMLMatrixrowElement
  : public virtual iface::mathml_dom::MathMLMatrixrowElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLMatrixrowElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLMatrixrowElement)

  uint32_t nEntries() throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getEntry(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* insertEntry(iface::mathml_dom::MathMLContentElement* newEntry, uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setEntry(iface::mathml_dom::MathMLContentElement* newEntry, uint32_t index) throw(std::exception&);
  void deleteEntry(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* removeEntry(uint32_t index) throw(std::exception&);
};

class CDA_MathMLPiecewiseElement
  : public virtual iface::mathml_dom::MathMLPiecewiseElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLPiecewiseElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLPiecewiseElement)

  iface::mathml_dom::MathMLNodeList* pieces() throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* otherwise() throw(std::exception&);
  void otherwise(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* getCase(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* setCase(uint32_t index, iface::mathml_dom::MathMLCaseElement* caseEl) throw(std::exception&);
  void deleteCase(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* removeCase(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* insertCase(uint32_t index, iface::mathml_dom::MathMLCaseElement* newCase) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getCaseValue(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setCaseValue(uint32_t index, iface::mathml_dom::MathMLContentElement* value) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getCaseCondition(uint32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setCaseCondition(uint32_t index, iface::mathml_dom::MathMLContentElement* condition) throw(std::exception&);
};

class CDA_MathMLCaseElement
  : public virtual iface::mathml_dom::MathMLCaseElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLCaseElement(GdomeElement* el);
  CDA_IMPL_QI5(dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLCaseElement)

  iface::mathml_dom::MathMLContentElement* caseCondition() throw(std::exception&);
  void caseCondition(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* caseValue() throw(std::exception&);
  void caseValue(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
};
