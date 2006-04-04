#include "CellMLImplementation.hpp"
#include "CellMLBootstrapImpl.hpp"
#include "CellMLBootstrap.hpp"

CDA_CellMLBootstrap::CDA_CellMLBootstrap()
  : _cda_refcount(1), domimpl(CreateDOMImplementation())
{
}

iface::cellml_api::DOMModelLoader*
CDA_CellMLBootstrap::modelLoader()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(dul, CDA_DOMURLLoader, new CDA_DOMURLLoader(domimpl));
  return new CDA_ModelLoader(dul);
}

iface::dom::DOMImplementation*
CDA_CellMLBootstrap::domImplementation()
  throw(std::exception&)
{
  domimpl->add_ref();
  return domimpl;
}

iface::cellml_api::DOMURLLoader*
CDA_CellMLBootstrap::localURLLoader()
  throw(std::exception&)
{
  return new CDA_DOMURLLoader(domimpl);
}

CDA_DOMURLLoader::CDA_DOMURLLoader(CellML_DOMImplementationBase* aDOMImpl)
  : _cda_refcount(1), mDOMImpl(aDOMImpl)
{
  mDOMImpl->add_ref();
}

iface::dom::Document*
CDA_DOMURLLoader::loadDocument(const wchar_t* URL)
  throw(std::exception&)
{
  iface::dom::Document* d = mDOMImpl->loadDocument(URL, mLastError);
  if (d == NULL)
  {
    throw iface::cellml_api::CellMLException();
  }
  return d;
}

void
CDA_DOMURLLoader::asyncLoadDocument
(
 const wchar_t* URL,
 iface::cellml_api::DocumentLoadedListener* listener
)
  throw(std::exception&)
{
  // Gdome can't do this(and I don't know if any API should try to, given that
  // we do not have a perform_work function and don't require that applications
  // be re-entrant/threadsafe).
  throw iface::cellml_api::CellMLException();
}

wchar_t*
CDA_DOMURLLoader::lastErrorMessage()
  throw(std::exception&)
{
  return CDA_wcsdup(mLastError.c_str());
}

CDA_ModelLoader::CDA_ModelLoader(iface::cellml_api::DOMURLLoader* aURLLoader)
  : _cda_refcount(1), mURLLoader(aURLLoader)
{
  mURLLoader->add_ref();
}

iface::cellml_api::Model*
CDA_ModelLoader::loadFromURL(const wchar_t* URL)
  throw(std::exception&)
{
  return createFromDOM(URL, mURLLoader);
}

void
CDA_ModelLoader::asyncLoadFromURL
(
 const wchar_t* URL,
 iface::cellml_api::ModelLoadedListener* listener
)
  throw(std::exception&)
{
  throw iface::cellml_api::CellMLException();
}

wchar_t*
CDA_ModelLoader::lastErrorMessage()
  throw(std::exception&)
{
  return CDA_wcsdup(mLastError.c_str());
}

#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"

iface::cellml_api::Model*
CDA_ModelLoader::createFromDOM(const wchar_t* URL,
                               iface::cellml_api::DOMURLLoader* loader)
  throw(std::exception&)
{
  ObjRef<iface::dom::Document> modelDoc;
  try
  {
    modelDoc = already_AddRefd<iface::dom::Document>
      (loader->loadDocument(URL));
  }
  catch (...)
  {
    wchar_t* str = loader->lastErrorMessage();
    mLastError = str;
    free(str);
    throw iface::cellml_api::CellMLException();
  }

  try
  {
    RETURN_INTO_OBJREF(modelEl, iface::dom::Element,
                       modelDoc->documentElement());
    if (modelEl == NULL)
    {
      mLastError = L"nodocumentelement";
      throw iface::cellml_api::CellMLException();
    }

    // Check it is a CellML model...
    RETURN_INTO_WSTRING(nsURI, modelEl->namespaceURI());
    if (nsURI != CELLML_1_0_NS &&
        nsURI != CELLML_1_1_NS)
    {
      mLastError = L"notcellml";
      throw iface::cellml_api::CellMLException();
    }

    RETURN_INTO_WSTRING(modName, modelEl->localName());
    if (modName != L"model")
    {
      mLastError = L"notcellml";
      throw iface::cellml_api::CellMLException();
    }

    return new CDA_Model(loader, modelDoc, modelEl);
  }
  catch (iface::dom::DOMException& de)
  {
    mLastError = L"badxml/0/0/Missing document element";
    throw iface::cellml_api::CellMLException();
  }
}

class CDA_CreateFromDOMDocumentLoadedListener
  : public iface::cellml_api::DocumentLoadedListener
{
public:
  CDA_CreateFromDOMDocumentLoadedListener
  (
   iface::cellml_api::DOMURLLoader* aLoader,
   CDA_ModelLoader* aModelLoader,
   iface::cellml_api::ModelLoadedListener* aListener
  )
    : _cda_refcount(1), mLoader(aLoader), mModelLoader(aModelLoader),
      mListener(aListener)
  {
    mLoader->add_ref();
    mModelLoader->add_ref();
    mListener->add_ref();
  }

  virtual ~CDA_CreateFromDOMDocumentLoadedListener()
  {
    mLoader->release_ref();
    mModelLoader->release_ref();
    mListener->release_ref();
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(iface::cellml_api::DocumentLoadedListener);
  CDA_IMPL_COMPARE_NAIVE(CDA_CreateFromDOMDocumentLoadedListener);

  void loadCompleted(iface::dom::Document* doc)
    throw(std::exception&)
  {
    if (doc == NULL)
    {
      wchar_t* str = mLoader->lastErrorMessage();
      mModelLoader->mLastError = str;
      free(str);
      mListener->loadCompleted(NULL);
      return;
    }

    try
    {
      RETURN_INTO_OBJREF(modelEl, iface::dom::Element,
                         doc->documentElement());
      if (modelEl == NULL)
      {
        mModelLoader->mLastError = L"nodocumentelement";
        mListener->loadCompleted(NULL);
        return;
      }
      
      // Check it is a CellML model...
      RETURN_INTO_WSTRING(nsURI, modelEl->namespaceURI());
      if (nsURI != CELLML_1_0_NS &&
          nsURI != CELLML_1_1_NS)
      {
        mModelLoader->mLastError = L"notcellml";
        mListener->loadCompleted(NULL);
        return;
      }

      RETURN_INTO_WSTRING(modName, modelEl->localName());
      if (modName != L"model")
      {
        mModelLoader->mLastError = L"notcellml";
        mListener->loadCompleted(NULL);
        return;
      }

      RETURN_INTO_OBJREF(cm, iface::cellml_api::Model,
                         new CDA_Model(mLoader, doc, modelEl));
      mListener->loadCompleted(cm);
    }
    catch (iface::dom::DOMException& de)
    {
      mModelLoader->mLastError = L"badxml/0/0/Missing document element";
      mListener->loadCompleted(NULL);
      return;
    }
  }
  
private:
  iface::cellml_api::DOMURLLoader* mLoader;
  CDA_ModelLoader* mModelLoader;
  iface::cellml_api::ModelLoadedListener* mListener;
};

void
CDA_ModelLoader::asyncCreateFromDOM
(
 const wchar_t* URL,
 iface::cellml_api::DOMURLLoader* loader,
 iface::cellml_api::ModelLoadedListener* listener
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(cfddll, iface::cellml_api::DocumentLoadedListener,
                     new CDA_CreateFromDOMDocumentLoadedListener
                     (loader, this, listener)
                    );
  loader->asyncLoadDocument(URL, cfddll);
}

iface::cellml_api::CellMLBootstrap*
CreateCellMLBootstrap()
{
  return new CDA_CellMLBootstrap();
}
