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

  already_AddRefd<iface::cellml_api::Model> loadFromURL(const std::wstring& URL)
    throw(std::exception&);
  void asyncLoadFromURL(const std::wstring& URL,
                        iface::cellml_api::ModelLoadedListener* listener)
    throw(std::exception&);
  std::wstring lastErrorMessage() throw(std::exception&);

  already_AddRefd<iface::cellml_api::Model> createFromText(const std::wstring& xmlText) throw(std::exception&);

  already_AddRefd<iface::cellml_api::Model>
  createFromDOM(const std::wstring& url,
                iface::cellml_api::DOMURLLoader* loader)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::Model>
  createFromDOMDocument(iface::dom::Document* aModel)
    throw(std::exception&);
  void asyncCreateFromDOM(const std::wstring& URL,
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

  already_AddRefd<iface::dom::Document> loadDocument(const std::wstring& URL)
    throw(std::exception&);
  already_AddRefd<iface::dom::Document> loadDocumentFromText(const std::wstring& text)
    throw(std::exception&);
  void asyncLoadDocument(const std::wstring& URL,
                         iface::cellml_api::DocumentLoadedListener* listener)
    throw(std::exception&);
  std::wstring lastErrorMessage() throw(std::exception&);
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

  already_AddRefd<iface::cellml_api::DOMModelLoader> modelLoader() throw(std::exception&);
  already_AddRefd<iface::dom::DOMImplementation> domImplementation() throw(std::exception&);
  already_AddRefd<iface::cellml_api::DOMURLLoader> localURLLoader() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Model> createModel(const std::wstring& version)
    throw(std::exception&);
  std::wstring serialiseNode(iface::dom::Node* aNode) throw(std::exception&);
  std::wstring makeURLAbsolute(const std::wstring& aRelTo, const std::wstring& aRelURL) throw();
private:
  CellML_DOMImplementationBase* domimpl;
};
