#include "DOMBootstrap.hxx"

class CDA_ModelLoader
  : public iface::cellml_api::DOMModelLoader
{
public:
  CDA_ModelLoader(iface::cellml_api::DOMURLLoader* aURLLoader);
  virtual ~CDA_ModelLoader() { mURLLoader->release_ref(); }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::DOMModelLoader, cellml_api::ModelLoader);
  CDA_IMPL_ID;

  iface::cellml_api::Model* loadFromURL(const wchar_t* URL)
    throw(std::exception&);
  void asyncLoadFromURL(const wchar_t* URL,
                        iface::cellml_api::ModelLoadedListener* listener)
    throw(std::exception&);
  wchar_t* lastErrorMessage() throw(std::exception&);

  iface::cellml_api::Model* createFromText(const wchar_t* xmlText) throw(std::exception&);

  iface::cellml_api::Model*
  createFromDOM(const wchar_t* url,
                iface::cellml_api::DOMURLLoader* loader)
    throw(std::exception&);
  void asyncCreateFromDOM(const wchar_t* URL,
                          iface::cellml_api::DOMURLLoader* loader,
                          iface::cellml_api::ModelLoadedListener* listener)
    throw(std::exception&);

  std::wstring mLastError;
private:
  iface::cellml_api::DOMURLLoader* mURLLoader;
};

class CDA_DOMURLLoader
  : public iface::cellml_api::DOMURLLoader
{
public:
  CDA_DOMURLLoader(CellML_DOMImplementationBase* aDOMImpl);
  virtual ~CDA_DOMURLLoader() { mDOMImpl->release_ref(); }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::DOMURLLoader);
  CDA_IMPL_ID;

  iface::dom::Document* loadDocument(const wchar_t* URL)
    throw(std::exception&);
  iface::dom::Document* loadDocumentFromText(const wchar_t* text)
    throw(std::exception&);
  void asyncLoadDocument(const wchar_t* URL,
                         iface::cellml_api::DocumentLoadedListener* listener)
    throw(std::exception&);
  wchar_t* lastErrorMessage() throw(std::exception&);
private:
  CellML_DOMImplementationBase* mDOMImpl;
  std::wstring mLastError;
};

class CDA_CellMLBootstrap
  : public iface::cellml_api::CellMLBootstrap
{
public:
  CDA_CellMLBootstrap();
  virtual ~CDA_CellMLBootstrap() { domimpl->release_ref(); }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::CellMLBootstrap);
  CDA_IMPL_ID;

  iface::cellml_api::DOMModelLoader* modelLoader() throw(std::exception&);
  iface::dom::DOMImplementation* domImplementation() throw(std::exception&);
  iface::cellml_api::DOMURLLoader* localURLLoader() throw(std::exception&);
  iface::cellml_api::Model* createModel(const wchar_t* version)
    throw(std::exception&);
private:
  CellML_DOMImplementationBase* domimpl;
};
