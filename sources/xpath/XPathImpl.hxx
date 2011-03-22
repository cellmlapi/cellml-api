#include "Ifacexpath.hxx"

class CDA_XPathEvaluator
  : public iface::xpath::XPathEvaluator
{
public:
  CDA_XPathEvaluator() : _cda_refcount(1) {}
  ~CDA_XPathEvaluator() {}
  CDA_IMPL_OBJID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(xpath::XPathEvaluator);

  iface::xpath::XPathExpression* createExpression(const wchar_t* aExpression,
                                                  iface::xpath::XPathNSResolver* aResolver)
    throw(std::exception&);
  iface::xpath::XPathNSResolver* createNSResolver(iface::dom::Node* aNodeResolver)
    throw();
  iface::xpath::XPathResult* evaluate(const wchar_t* aExpression,
                                      iface::dom::Node* aContextNode,
                                      iface::xpath::XPathNSResolver* aResolver,
                                      uint16_t aType,
                                      iface::xpath::XPathResult* aResult)
    throw(std::exception&);
};

class CDA_XPathResult
  : public iface::xpath::XPathResult
{

};
