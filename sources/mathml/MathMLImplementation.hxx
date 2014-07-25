#include <exception>
#include "cda_compiler_support.h"
#include "IfaceMathML_content_APISPEC.hxx"
#include "DOMImplementation.hpp"
#include <map>

class CDA_MathMLDocument
  : public virtual iface::mathml_dom::MathMLDocument,
    public virtual CDA_Document
{
public:
  CDA_MathMLDocument(
#ifdef USE_GDOME
                     GdomeDocument* d
#endif
                    );
  virtual ~CDA_MathMLDocument() {};

  CDA_IMPL_QI5(events::EventTarget, events::DocumentEvent, dom::Node,
               dom::Document, mathml_dom::MathMLDocument)

  std::wstring referrer() throw(std::exception&);
  std::wstring domain() throw(std::exception&);
  std::wstring URI() throw(std::exception&);
};

#ifdef USE_GDOME
#define ELCONSARG GdomeElement* el
#define ELCONSCALL el
#else
#define ELCONSARG CDA_Document* doc
#define ELCONSCALL doc
#endif

class CDA_MathMLElement
  : public virtual iface::mathml_dom::MathMLElement,
    public virtual CDA_Element
{
public:
  CDA_MathMLElement(ELCONSARG);

  CDA_IMPL_QI4(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement)

  std::wstring className() throw(std::exception&);
  void className(const std::wstring& attr) throw(std::exception&);
  std::wstring mathElementStyle() throw(std::exception&);
  void mathElementStyle(const std::wstring& attr) throw(std::exception&);
  std::wstring id() throw(std::exception&);
  void id(const std::wstring& attr) throw(std::exception&);
  std::wstring xref() throw(std::exception&);
  void xref(const std::wstring& attr) throw(std::exception&);
  std::wstring href() throw(std::exception&);
  void href(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLMathElement> ownerMathElement() throw(std::exception&);
};

class CDA_MathMLContainer
  : public virtual iface::mathml_dom::MathMLContainer,
    // Derive from this now, since we need it.
    public virtual iface::mathml_dom::MathMLElement
{
public:
  CDA_MathMLContainer();

  uint32_t nArguments() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLNodeList> arguments() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLNodeList> declarations() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> getArgument(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> setArgument(iface::mathml_dom::MathMLElement* newArgument, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> insertArgument(iface::mathml_dom::MathMLElement* newArgument, uint32_t index) throw(std::exception&);
  void deleteArgument(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> removeArgument(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLDeclareElement> getDeclaration(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLDeclareElement> setDeclaration(iface::mathml_dom::MathMLDeclareElement* newDeclaration, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLDeclareElement> insertDeclaration(iface::mathml_dom::MathMLDeclareElement* newDeclaration, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLDeclareElement> removeDeclaration(uint32_t index) throw(std::exception&);
  void deleteDeclaration(uint32_t index) throw(std::exception&);
};

class CDA_MathMLMathElement
  : public virtual iface::mathml_dom::MathMLMathElement,
    public virtual CDA_MathMLElement,
    public virtual CDA_MathMLContainer
{
public:
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLMathElement)

  CDA_MathMLMathElement(ELCONSARG);

  std::wstring macros() throw(std::exception&);
  void macros(const std::wstring& attr) throw(std::exception&);
  std::wstring display() throw(std::exception&);
  void display(const std::wstring& attr) throw(std::exception&);
};

class CDA_MathMLContentElement
  : public virtual iface::mathml_dom::MathMLContentElement,
    public virtual CDA_MathMLElement
{
public:
  CDA_MathMLContentElement(ELCONSARG);
};

class CDA_MathMLContentToken
  : public virtual iface::mathml_dom::MathMLContentToken,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLContentToken(ELCONSARG);

  already_AddRefd<iface::mathml_dom::MathMLNodeList> arguments() throw(std::exception&);
  std::wstring definitionURL() throw(std::exception&);
  void definitionURL(const std::wstring& attr) throw(std::exception&);
  std::wstring encoding() throw(std::exception&);
  void encoding(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::dom::Node> getArgument(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::dom::Node> insertArgument(iface::dom::Node* newArgument, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::dom::Node> setArgument(iface::dom::Node* newArgument, uint32_t index) throw(std::exception&);
  void deleteArgument(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::dom::Node> removeArgument(uint32_t index) throw(std::exception&);
};

class CDA_MathMLCnElement
  : public virtual iface::mathml_dom::MathMLCnElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_MathMLCnElement(ELCONSARG);
  CDA_IMPL_QI7(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContentToken, mathml_dom::MathMLCnElement)

  std::wstring type() throw(std::exception&);
  void type(const std::wstring& attr) throw(std::exception&);
  std::wstring base() throw(std::exception&);
  void base(const std::wstring& attr) throw(std::exception&);
  uint32_t nargs() throw(std::exception&);
};

class CDA_MathMLCiElement
  : public virtual iface::mathml_dom::MathMLCiElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_IMPL_QI7(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContentToken, mathml_dom::MathMLCiElement)

  CDA_MathMLCiElement(ELCONSARG);

  std::wstring type() throw(std::exception&);
  void type(const std::wstring& attr) throw(std::exception&);
};

class CDA_MathMLCsymbolElement
  : public virtual iface::mathml_dom::MathMLCsymbolElement,
    public virtual CDA_MathMLContentToken
{
public:
  CDA_MathMLCsymbolElement(ELCONSARG);

  CDA_IMPL_QI7(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContentToken, mathml_dom::MathMLCsymbolElement)
};

class CDA_MathMLContentContainer
  : public virtual iface::mathml_dom::MathMLContentContainer,
    public virtual CDA_MathMLContentElement,
    public virtual CDA_MathMLContainer
{
public:
  CDA_MathMLContentContainer(ELCONSARG);

  CDA_IMPL_QI7(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer,
               mathml_dom::MathMLContentContainer)

  uint32_t nBoundVariables() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLConditionElement> condition() throw(std::exception&);
  void condition(iface::mathml_dom::MathMLConditionElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> opDegree() throw(std::exception&);
  void opDegree(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> domainOfApplication() throw(std::exception&);
  void domainOfApplication(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> momentAbout() throw(std::exception&);
  void momentAbout(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLBvarElement> getBoundVariable(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLBvarElement> insertBoundVariable(iface::mathml_dom::MathMLBvarElement* newBVar, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLBvarElement> setBoundVariable(iface::mathml_dom::MathMLBvarElement* newBVar, uint32_t index) throw(std::exception&);
  void deleteBoundVariable(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLBvarElement> removeBoundVariable(uint32_t index) throw(std::exception&);
};

class CDA_MathMLApplyElement
  : public virtual iface::mathml_dom::MathMLApplyElement,
    public virtual CDA_MathMLContentContainer,
    public virtual CDA_Element
{
public:
  CDA_IMPL_QI8(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement, mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLApplyElement)

  CDA_MathMLApplyElement(ELCONSARG);

  already_AddRefd<iface::mathml_dom::MathMLElement> _cxx_operator() throw(std::exception&);
  void _cxx_operator(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> lowLimit() throw(std::exception&);
  void lowLimit(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> upLimit() throw(std::exception&);
  void upLimit(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
};

class CDA_MathMLFnElement
  : public virtual iface::mathml_dom::MathMLFnElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLFnElement(ELCONSARG);
  CDA_IMPL_QI8(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer,
               mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLFnElement)

  std::wstring definitionURL() throw(std::exception&);
  void definitionURL(const std::wstring& attr) throw(std::exception&);
  std::wstring encoding() throw(std::exception&);
  void encoding(const std::wstring& attr) throw(std::exception&);
};

class CDA_MathMLLambdaElement
  : public virtual iface::mathml_dom::MathMLLambdaElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLLambdaElement(ELCONSARG);

  CDA_IMPL_QI8(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLLambdaElement)

  already_AddRefd<iface::mathml_dom::MathMLElement> expression() throw(std::exception&);
  void expression(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
};

class CDA_MathMLSetElement
  : public virtual iface::mathml_dom::MathMLSetElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLSetElement(ELCONSARG);
  CDA_IMPL_QI8(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLSetElement)

  bool isExplicit() throw(std::exception&);
  std::wstring type() throw(std::exception&);
  void type(const std::wstring& attr) throw(std::exception&);
};

class CDA_MathMLListElement
  : public virtual iface::mathml_dom::MathMLListElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLListElement(ELCONSARG);
  CDA_IMPL_QI8(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLListElement)

  bool isExplicit() throw(std::exception&);
  std::wstring ordering() throw(std::exception&);
  void ordering(const std::wstring& attr) throw(std::exception&);
};

class CDA_MathMLBvarElement
  : public virtual iface::mathml_dom::MathMLBvarElement,
    public virtual CDA_MathMLContentContainer
{
public:
  CDA_MathMLBvarElement(ELCONSARG);
  CDA_IMPL_QI8(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLContainer, mathml_dom::MathMLContentContainer,
               mathml_dom::MathMLBvarElement)
};

class CDA_MathMLPredefinedSymbol
  : public virtual iface::mathml_dom::MathMLPredefinedSymbol,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLPredefinedSymbol(ELCONSARG);

  std::wstring definitionURL() throw(std::exception&);
  void definitionURL(const std::wstring& attr) throw(std::exception&);
  std::wstring encoding() throw(std::exception&);
  void encoding(const std::wstring& attr) throw(std::exception&);
  std::wstring arity() throw(std::exception&);
  std::wstring symbolName() throw(std::exception&);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLPredefinedSymbol)
};

class CDA_MathMLTendsToElement
  : public virtual iface::mathml_dom::MathMLTendsToElement,
    public virtual CDA_MathMLPredefinedSymbol
{
public:
  CDA_MathMLTendsToElement(ELCONSARG);
  CDA_IMPL_QI7(events::EventTarget, dom::Node, dom::Element, mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLPredefinedSymbol,
               mathml_dom::MathMLTendsToElement)

  std::wstring type() throw(std::exception&);
  void type(const std::wstring& attr) throw(std::exception&);
};

class CDA_MathMLIntervalElement
  : public virtual iface::mathml_dom::MathMLIntervalElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLIntervalElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLIntervalElement)

  std::wstring closure() throw(std::exception&);
  void closure(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> start() throw(std::exception&);
  void start(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> end() throw(std::exception&);
  void end(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
};

class CDA_MathMLConditionElement
  : public virtual iface::mathml_dom::MathMLConditionElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLConditionElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLConditionElement)

  already_AddRefd<iface::mathml_dom::MathMLApplyElement> condition() throw(std::exception&);
  void condition(iface::mathml_dom::MathMLApplyElement* attr) throw(std::exception&);
};

class CDA_MathMLDeclareElement
  : public virtual iface::mathml_dom::MathMLDeclareElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLDeclareElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLDeclareElement)

  std::wstring type() throw(std::exception&);
  void type(const std::wstring& attr) throw(std::exception&);
  uint32_t nargs() throw(std::exception&);
  void nargs(uint32_t attr) throw(std::exception&);
  std::wstring occurrence() throw(std::exception&);
  void occurrence(const std::wstring& attr) throw(std::exception&);
  std::wstring definitionURL() throw(std::exception&);
  void definitionURL(const std::wstring& attr) throw(std::exception&);
  std::wstring encoding() throw(std::exception&);
  void encoding(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLCiElement> identifier() throw(std::exception&);
  void identifier(iface::mathml_dom::MathMLCiElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLElement> constructor() throw(std::exception&);
  void constructor(iface::mathml_dom::MathMLElement* attr) throw(std::exception&);
};

class CDA_MathMLVectorElement
  : public virtual iface::mathml_dom::MathMLVectorElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLVectorElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLVectorElement)

  uint32_t ncomponents() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> getComponent(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> insertComponent(iface::mathml_dom::MathMLContentElement* newComponent, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> setComponent(iface::mathml_dom::MathMLContentElement* newComponent, uint32_t index) throw(std::exception&);
  void deleteComponent(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> removeComponent(uint32_t index) throw(std::exception&);
};

class CDA_MathMLMatrixElement
  : public virtual iface::mathml_dom::MathMLMatrixElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLMatrixElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLMatrixElement)

  uint32_t nrows() throw(std::exception&);
  uint32_t ncols() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLNodeList> rows() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLMatrixrowElement> getRow(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLMatrixrowElement> insertRow(iface::mathml_dom::MathMLMatrixrowElement* newRow, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLMatrixrowElement> setRow(iface::mathml_dom::MathMLMatrixrowElement* newRow, uint32_t index) throw(std::exception&);
  void deleteRow(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLMatrixrowElement> removeRow(uint32_t index) throw(std::exception&);
};

class CDA_MathMLMatrixrowElement
  : public virtual iface::mathml_dom::MathMLMatrixrowElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLMatrixrowElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLMatrixrowElement)

  uint32_t nEntries() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> getEntry(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> insertEntry(iface::mathml_dom::MathMLContentElement* newEntry, uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> setEntry(iface::mathml_dom::MathMLContentElement* newEntry, uint32_t index) throw(std::exception&);
  void deleteEntry(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> removeEntry(uint32_t index) throw(std::exception&);
};

class CDA_MathMLPiecewiseElement
  : public virtual iface::mathml_dom::MathMLPiecewiseElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLPiecewiseElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLPiecewiseElement)

  already_AddRefd<iface::mathml_dom::MathMLNodeList> pieces() throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> otherwise() throw(std::exception&);
  void otherwise(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLCaseElement> getCase(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLCaseElement> setCase(uint32_t index, iface::mathml_dom::MathMLCaseElement* caseEl) throw(std::exception&);
  void deleteCase(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLCaseElement> removeCase(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLCaseElement> insertCase(uint32_t index, iface::mathml_dom::MathMLCaseElement* newCase) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> getCaseValue(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> setCaseValue(uint32_t index, iface::mathml_dom::MathMLContentElement* value) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> getCaseCondition(uint32_t index) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> setCaseCondition(uint32_t index, iface::mathml_dom::MathMLContentElement* condition) throw(std::exception&);
};

class CDA_MathMLCaseElement
  : public virtual iface::mathml_dom::MathMLCaseElement,
    public virtual CDA_MathMLContentElement
{
public:
  CDA_MathMLCaseElement(ELCONSARG);
  CDA_IMPL_QI6(events::EventTarget, dom::Node, dom::Element,
               mathml_dom::MathMLElement,
               mathml_dom::MathMLContentElement,
               mathml_dom::MathMLCaseElement)

  already_AddRefd<iface::mathml_dom::MathMLContentElement> caseCondition() throw(std::exception&);
  void caseCondition(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
  already_AddRefd<iface::mathml_dom::MathMLContentElement> caseValue() throw(std::exception&);
  void caseValue(iface::mathml_dom::MathMLContentElement* attr) throw(std::exception&);
};
