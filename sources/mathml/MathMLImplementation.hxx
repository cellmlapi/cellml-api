#include <exception>
#include <sys/types.h>
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

  iface::dom::DOMString referrer() throw(std::exception&);
  iface::dom::DOMString domain() throw(std::exception&);
  iface::dom::DOMString URI() throw(std::exception&);
};

class CDA_MathMLElement
  : public virtual iface::mathml_dom::MathMLElement,
    public virtual CDA_Element
{
public:
  CDA_MathMLElement(GdomeElement* el);

  iface::dom::DOMString className() throw(std::exception&);
  void className(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString mathElementStyle() throw(std::exception&);
  void mathElementStyle(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString id() throw(std::exception&);
  void id(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString xref() throw(std::exception&);
  void xref(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString href() throw(std::exception&);
  void href(iface::dom::DOMString attr) throw(std::exception&);
  iface::mathml_dom::MathMLMathElement* ownerMathElement() throw(std::exception&);
};

class CDA_MathMLContainer
  : public virtual iface::mathml_dom::MathMLContainer,
    // Derive from this now, since we need it.
    public virtual iface::mathml_dom::MathMLElement
{
public:
  CDA_MathMLContainer();

  u_int32_t nArguments() throw(std::exception&);
  iface::mathml_dom::MathMLNodeList* arguments() throw(std::exception&);
  iface::mathml_dom::MathMLNodeList* declarations() throw(std::exception&);
  iface::mathml_dom::MathMLElement* getArgument(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLElement* setArgument(iface::mathml_dom::MathMLElement* newArgument, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLElement* insertArgument(iface::mathml_dom::MathMLElement* newArgument, u_int32_t index) throw(std::exception&);
  void deleteArgument(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLElement* removeArgument(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* getDeclaration(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* setDeclaration(iface::mathml_dom::MathMLDeclareElement* newDeclaration, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* insertDeclaration(iface::mathml_dom::MathMLDeclareElement* newDeclaration, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLDeclareElement* removeDeclaration(u_int32_t index) throw(std::exception&);
  void deleteDeclaration(u_int32_t index) throw(std::exception&);
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

  iface::dom::DOMString macros() throw(std::exception&);
  void macros(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString display() throw(std::exception&);
  void display(iface::dom::DOMString attr) throw(std::exception&);
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
  iface::dom::DOMString definitionURL() throw(std::exception&);
  void definitionURL(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString encoding() throw(std::exception&);
  void encoding(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::Node* getArgument(u_int32_t index) throw(std::exception&);
  iface::dom::Node* insertArgument(iface::dom::Node* newArgument, u_int32_t index) throw(std::exception&);
  iface::dom::Node* setArgument(iface::dom::Node* newArgument, u_int32_t index) throw(std::exception&);
  void deleteArgument(u_int32_t index) throw(std::exception&);
  iface::dom::Node* removeArgument(u_int32_t index) throw(std::exception&);
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

  iface::dom::DOMString type() throw(std::exception&);
  void type(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString base() throw(std::exception&);
  void base(iface::dom::DOMString attr) throw(std::exception&);
  u_int32_t nargs() throw(std::exception&);
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

  iface::dom::DOMString type() throw(std::exception&);
  void type(iface::dom::DOMString attr) throw(std::exception&);
};

class CDA_MathMLCsymbolElement
  : public virtual iface::mathml_dom::MathMLCsymbolElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_MathMLCsymbolElement(GdomeElement* elem);
};

class CDA_MathMLContentContainer
  : public virtual iface::mathml_dom::MathMLContentContainer,
    public virtual CDA_MathMLContentElement,
    public virtual CDA_MathMLContainer
{
public:
  CDA_MathMLContentContainer(GdomeElement* elem);

  u_int32_t nBoundVariables() throw(std::exception&);
  iface::mathml_dom::MathMLConditionElement* condition() throw(std::exception&);
  void condition(iface::mathml_dom::MathMLConditionElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* opDegree() throw(std::exception&);
  void opDegree(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* domainOfApplication() throw(std::exception&);
  void domainOfApplication(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLElement* momentAbout() throw(std::exception&);
  void momentAbout(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* getBoundVariable(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* insertBoundVariable(iface::mathml_dom::MathMLBvarElement* newBVar, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* setBoundVariable(iface::mathml_dom::MathMLBvarElement* newBVar, u_int32_t index) throw(std::exception&);
  void deleteBoundVariable(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLBvarElement* removeBoundVariable(u_int32_t index) throw(std::exception&);
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

  iface::dom::DOMString definitionURL() throw(std::exception&);
  void definitionURL(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString encoding() throw(std::exception&);
  void encoding(iface::dom::DOMString attr) throw(std::exception&);
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
  iface::dom::DOMString type() throw(std::exception&);
  void type(iface::dom::DOMString attr) throw(std::exception&);
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
  iface::dom::DOMString ordering() throw(std::exception&);
  void ordering(iface::dom::DOMString attr) throw(std::exception&);
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

  iface::dom::DOMString definitionURL() throw(std::exception&);
  void definitionURL(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString encoding() throw(std::exception&);
  void encoding(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString arity() throw(std::exception&);
  iface::dom::DOMString symbolName() throw(std::exception&);
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

  iface::dom::DOMString type() throw(std::exception&);
  void type(iface::dom::DOMString attr) throw(std::exception&);
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

  iface::dom::DOMString closure() throw(std::exception&);
  void closure(iface::dom::DOMString attr) throw(std::exception&);
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

  iface::dom::DOMString type() throw(std::exception&);
  void type(iface::dom::DOMString attr) throw(std::exception&);
  u_int32_t nargs() throw(std::exception&);
  void nargs(u_int32_t attr) throw(std::exception&);
  iface::dom::DOMString occurrence() throw(std::exception&);
  void occurrence(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString definitionURL() throw(std::exception&);
  void definitionURL(iface::dom::DOMString attr) throw(std::exception&);
  iface::dom::DOMString encoding() throw(std::exception&);
  void encoding(iface::dom::DOMString attr) throw(std::exception&);
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

  u_int32_t ncomponents() throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getComponent(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* insertComponent(iface::mathml_dom::MathMLContentElement* newComponent, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setComponent(iface::mathml_dom::MathMLContentElement* newComponent, u_int32_t index) throw(std::exception&);
  void deleteComponent(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* removeComponent(u_int32_t index) throw(std::exception&);
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

  u_int32_t nrows() throw(std::exception&);
  u_int32_t ncols() throw(std::exception&);
  iface::mathml_dom::MathMLNodeList* rows() throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* getRow(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* insertRow(iface::mathml_dom::MathMLMatrixrowElement* newRow, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* setRow(iface::mathml_dom::MathMLMatrixrowElement* newRow, u_int32_t index) throw(std::exception&);
  void deleteRow(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLMatrixrowElement* removeRow(u_int32_t index) throw(std::exception&);
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

  u_int32_t nEntries() throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getEntry(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* insertEntry(iface::mathml_dom::MathMLContentElement* newEntry, u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setEntry(iface::mathml_dom::MathMLContentElement* newEntry, u_int32_t index) throw(std::exception&);
  void deleteEntry(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* removeEntry(u_int32_t index) throw(std::exception&);
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
  iface::mathml_dom::MathMLCaseElement* getCase(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* setCase(u_int32_t index, iface::mathml_dom::MathMLCaseElement* caseEl) throw(std::exception&);
  void deleteCase(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* removeCase(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLCaseElement* insertCase(u_int32_t index, iface::mathml_dom::MathMLCaseElement* newCase) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getCaseValue(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setCaseValue(u_int32_t index, iface::mathml_dom::MathMLContentElement* value) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* getCaseCondition(u_int32_t index) throw(std::exception&);
  iface::mathml_dom::MathMLContentElement* setCaseCondition(u_int32_t index, iface::mathml_dom::MathMLContentElement* condition) throw(std::exception&);
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
