#include "CellMLImplementation.hpp"
#include "../dom/DOMImplementation.hxx"
#include "CellMLBootstrapImpl.hpp"
#include "CellMLBootstrap.hpp"

CDA_CellMLBootstrap::CDA_CellMLBootstrap()
  : domimpl(new CDA_DOMImplementation())
{
}

iface::cellml_api::DOMModelLoader*
CDA_CellMLBootstrap::modelLoader()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(dul, CDA_DOMURLLoader, new CDA_DOMURLLoader(domimpl));
  return new CDA_ModelLoader(&dul);
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

CDA_DOMURLLoader::CDA_DOMURLLoader(CDA_DOMImplementation* aDOMImpl)
  : mDOMImpl(aDOMImpl)
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

wchar_t*
CDA_DOMURLLoader::lastErrorMessage()
  throw(std::exception&)
{
  return wcsdup(mLastError.c_str());
}

CDA_ModelLoader::CDA_ModelLoader(iface::cellml_api::DOMURLLoader* aURLLoader)
  : mURLLoader(aURLLoader)
{
  mURLLoader->add_ref();
}

iface::cellml_api::Model*
CDA_ModelLoader::loadFromURL(const wchar_t* URL)
  throw(std::exception&)
{
  return createFromDOM(URL, mURLLoader);
}

wchar_t*
CDA_ModelLoader::lastErrorMessage()
  throw(std::exception&)
{
  return wcsdup(mLastError.c_str());
}

#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"

iface::cellml_api::Model*
CDA_ModelLoader::createFromDOM(const wchar_t* URL,
                               iface::cellml_api::DOMURLLoader* loader)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(modelDoc, iface::dom::Document,
                     loader->loadDocument(URL));

  try
  {
    RETURN_INTO_OBJREF(modelEl, iface::dom::Element,
                       modelDoc->documentElement());
    if (&modelEl == NULL)
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

    return new CDA_Model(loader, &modelDoc, &modelEl);
  }
  catch (iface::dom::DOMException& de)
  {
    mLastError = L"badxml/0/0/Missing document element";
    throw iface::cellml_api::CellMLException();
  }
}
