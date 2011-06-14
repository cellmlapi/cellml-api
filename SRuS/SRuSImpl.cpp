#include "SRuSBootstrap.hpp"
#include "Utilities.hxx"
#include "CellMLBootstrap.hpp"
#include "XPathBootstrap.hpp"

class CDA_SRuSProcessor
  : public iface::SRuS::Processor
{
public:
  CDA_SRuSProcessor
    : _cda_refcount(1) {}

  CDA_IMPL_OBJID;
  CDA_IMPL_QI1(SRuS::SEDMLProcessor);
  CDA_IMPL_REFCOUNT;

  bool supportsModellingLanguage(const wchar_t* aLang) throw();
  iface::SRuS::TransformedModel* buildOneModel(iface::SProS::Model* aModel)
    throw();
  iface::SRuS::TransformedModelSet* buildAllModels
    (iface::SProS::SEDMLElement* aElement) throw();
  void generateData(iface::SRuS::TransformedModelSet* aSet,
                    iface::SProS::SEDMLElement* aElement,
                    iface::SRuS::GeneratedDataMonitor* aMonitor) throw();

private:
  iface::cellml_api::CellMLElement* xmlToCellML(iface::dom::Node* aNode)
    throw();
};

template<typename C> class XPCOMContainerRAII
{
public:
  XPCOMContainerRAII(C& aCont)
    : mCont(aCont)
  {
  }

  ~XPCOMContainerRAII()
  {
    for (typename C::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i)->release_ref();
    mCont.clear();
  }

private:
  C& mCont;
};

iface::cellml_api::CellMLElement*
CDA_SRuSProcessor::xmlToCellML(iface::dom::Node* aNode) throw()
{
  std::list<iface::dom::Node*> l;
  XPCOMContainerRAII<std::list<iface::dom::Node*> > lRAII(l);
  
  ObjRef<iface::dom::Node> n(aNode);
  while (n)
  {
    n.add_ref();
    l.push_back(n);

    n = already_AddRefd<iface::dom::Node*>(aNode->parentNode());
  }

  
}

bool
CDA_SRuSProcess::supportsModellingLanguage(const wchar_t* aLang)
  throw()
{
  if (!CDA_wcscmp(aLang, L"http://www.cellml.org/cellml/1.1#") ||
      !CDA_wcscmp(aLang, L"http://www.cellml.org/cellml/1.0#"))
    return true;

  // In future, make this extensible so we can support other modelling languages.
  return false;
}

class CDA_SRuSTransformedModel
  : public iface::SRuS::TransformedModel
{
public:
  CDA_SRuSTransformedModel(iface::dom::Document* aDoc)
    : _cda_refcount(1), mDocument(aDoc) {}

  CDA_IMPL_OBJID;
  CDA_IMPL_QI1(SRuS::TransformedModel);
  CDA_IMPL_REFCOUNT;

  iface::dom::Document* xmlDocument() throw()
  {
    mDocument->add_ref();
    return mDocument;
  }

  iface::XPCOM::IObject* modelDocument() throw(std::exception&)
  {
    ensureModelOrRaise();
    mModel->add_ref();
    return mModel;
  }

  iface::SProS::Model* sedmlModel() throw()
  {
    mSEDMLModel->add_ref();
    return mSEDMLModel;
  }

private:
  void ensureModelOrRaise()
  {
    if (mModel)
      return;

    RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader, cb->modelLoader());
    DECLARE_QUERY_INTERFACE_OBJREF(dml, ml, cellml_api::DOMModelLoader);
    try
    {
      mModel = already_AddRefd<iface::cellml_api::Model>(dml->createFromDOMDocument());
    }
    catch (...)
    {
    }

    if (mModel == NULL)
      throw iface::SRuS::SRuSException();
  }

  ObjRef<iface::dom::Document> mDocument;
  ObjRef<iface::cellml_api::Model> mModel;
  ObjRef<iface::SProS::Model> mSEDMLModel;
};

iface::SRuS::TransformedModel*
CDA_SRuSProcess::buildOneModel(iface::SProS::Model* aModel)
  throw()
{
  try
  {
    RETURN_INTO_WSTRING(lang, aModel->language());
    if (lang != L"urn:sedml:language:cellML" &&
        lang != L"urn:sedml:language:cellml.1_0" &&
        lang != L"urn:sedml:language:cellml.1_1")
    {
      // To do: Support other languages here...
      throw iface::SRuS::SRuSException();
    }

    // Get the URI...
    RETURN_INTO_WSTRING(uri, aModel->source());
    
    // Load it...
    RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
    RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader, cb->modelLoader());
    DECLARE_QUERY_INTERFACE_OBJREF(dml, ml, dom::Document);
    RETURN_INTO_OBJREF(doc, iface::dom::Document, dml->loadDocument(uri.c_str()));
    
    RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
    RETURN_INTO_OBJREF(de, iface::dom::Element, doc->documentElement());
    RETURN_INTO_OBJREF(resolver, iface::xpath::XPathNSResolver,
                       xe->createNSResolver(de));
    
    // Apply the transformations to the document...
    RETURN_INTO_OBJREF(cs, iface::SProS::ChangeSet, aModel->changes());
    RETURN_INTO_OBJREF(ci, iface::SProS::ChangeIterator, cs->iterateChanges());
    while (true)
    {
      RETURN_INTO_OBJREF(c, iface::SProS::Change, ci->nextChange());
      if (c == NULL)
        break;
      
      RETURN_INTO_WSTRING(targ, c->target());
      RETURN_INTO_OBJREF(xr, iface::xpath::XPathResult,
                         xe->evaluate(targ.c_str(), doc, resolver,
                                      iface::xpath::XPathResult::UNORDERED_NODE_ITERATOR_TYPE,
                                      NULL));
      
      RETURN_INTO_OBJREF(n, iface::dom::Node, rx->iterateNext());
      if (n == NULL)
        throw iface::SRuS::SRuSException();
      
      RETURN_INTO_OBJREF(n2, iface::dom::Node, rx->iterateNext());
      if (n2)
        throw iface::SRuS::SRuSException();
      

      DECLARE_QUERY_INTERFACE_OBJREF(ca, c, SProS::ChangeAttribute);
      if (ca != NULL)
      {
        DECLARE_QUERY_INTERFACE_OBJREF(at, n, dom::Attr);
        if (at == NULL) // XPath target is not an attribute?
          throw iface::SRuS::SRuSException();
        RETURN_INTO_WSTRING(v, ca->newValue());
        at->value(v.c_str());
        continue;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(ax, c, SProS::AddXML);
      if (ax != NULL)
      {
        RETURN_INTO_OBJREF(nl, iface::dom::NodeList, ax->anyXML());
        RETURN_INTO_OBJREF(np, iface::dom::Node, n->parentNode());

        RETURN_INTO_OBJREF(axnl, iface::dom::NodeList, ax->anyXML());
        for (uint32_t i = 0, l = axnl->length(); i < l; i++)
        {
          RETURN_INTO_OBJREF(nn, iface::dom::Node, axnl->item(i));
          RETURN_INTO_OBJREF(tmp, np->insertBefore(nn, n));
        }

        // Deliberate fall-through, ChangeXML is a specific type of AddXML.
      }

      DECLARE_QUERY_INTERFACE_OBJREF(cx, c, SProS::ChangeXML);
      DECLARE_QUERY_INTERFACE_OBJREF(rx, c, SProS::RemoveXML);
      if (cx != NULL || rx != NULL)
      {
        RETURN_INTO_OBJREF(np, iface::dom::Node, n->parentNode());
        RETURN_INTO_OBJREF(tmp, iface::dom::Node, np->removeChild(n));
        continue;
      }

      DECLARE_QUERY_INTERFACE_OBJREF(cc, c, SProS::ComputeChange);
      if (cc != NULL)
      {
        throw iface::SRuS::SRuSException();
      }

      if (ax != NULL)
        continue;
    }

    return new CDA_TransformedModel(doc);
  }
  catch (...)
  {
    throw iface::SRuS::SRuSException();
  }
}

class CDA_SRuSTransformedModelSet
  : public iface::SRuS::TransformedModelSet
{
public:
  CDA_SRuSTransformedModelSet
    : _cda_refcount(1) {}

  CDA_IMPL_OBJID;
  CDA_IMPL_QI1(SRuS::TransformedModelSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length() throw();
  iface::SRuS::TransformedModel* item(uint32_t aIdx)
    throw(std::exception&);
  iface::SRuS::TransformedModel* getItemByID(const std::wstring& aMatchID)
    throw(std::exception&);
};

iface::SRuS::TransformedModelSet*
CDA_SRuSProcess::buildAllModels(iface::SProS::SEDMLElement* aElement)
  throw()
{
#error TODO
}

class CDA_SRuSGeneratedData
  : public iface::SRuS::GeneratedData
{
public:
  CDA_SRuSGeneratedData
    : _cda_refcount(1) {}
  
  CDA_IMPL_OBJID;
  CDA_IMPL_QI1(SRuS::GeneratedData);
  CDA_IMPL_REFCOUNT;

  iface::SProS::DataGenerator* sedmlDataGenerator() throw();
  uint32_t length() throw();
  double dataPoint(uint32_t idex) throw(iface::SRuS::SRuSException&);
};

class CDA_SRuSGeneratedDataSet
  : public iface::SRuS::GeneratedDataSet
{
public:
  CDA_SRuSGeneratedDataSet
    : _cda_refcount(1) {}
  
  CDA_IMPL_OBJID;
  CDA_IMPL_QI1(SRuS::GeneratedDataSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length() throw();
  iface::SRuS::GeneratedData* item(uint32_t aIdx) throw(iface::SRuS::SRuSException&);
};

void
CDA_SRuSProcessor::generateData
(
 iface::SRuS::TransformedModelSet* aSet,
 iface::SProS::SEDMLElement* aElement,
 iface::SRuS::GeneratedDataMonitor* aMonitor
)
  throw()
{
#error TODO
}

class CDA_SRuSBootstrap
  : public iface::SRuS::Bootstrap
{
public:
  CDA_SRuSBootstrap
    : _cda_refcount(1) {}

  CDA_IMPL_OBJID;
  CDA_IMPL_QI1(SRuS::Bootstrap);
  CDA_IMPL_REFCOUNT;

  iface::SRuS::SEDMLProcessor* makeDefaultProcessor()
    throw()
  {
    return new CDA_SRuSProcessor();
  }
};

iface::SProS::Bootstrap*
CreateSRuSBootstrap() throw()
{
  return new CDA_SRuSBootstrap();
}
