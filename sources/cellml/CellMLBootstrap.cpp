#include "cda_compiler_support.h"
#define IN_CELLML_MODULE
// CellML and DOM are in the same module...
#define IN_DOM_MODULE
#define IN_DOMWRITER_MODULE
#define MODULE_CONTAINS_xpcom
#define MODULE_CONTAINS_RDFAPISPEC
#define MODULE_CONTAINS_CellMLAPISPEC
#define MODULE_CONTAINS_CellMLBootstrap
#define MODULE_CONTAINS_DOMAPISPEC
#define MODULE_CONTAINS_DOMevents

#include "CellMLImplementation.hpp"
#include "CellMLBootstrapImpl.hpp"
#include "CellMLBootstrap.hpp"
#include "DOMWriter.hxx"

#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"

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

iface::cellml_api::Model*
CDA_CellMLBootstrap::createModel(const wchar_t* version)
  throw(std::exception&)
{
  ObjRef<iface::cellml_api::DOMURLLoader> ul =
    already_AddRefd<iface::cellml_api::DOMURLLoader>(localURLLoader());
  const wchar_t* ns;
  if (!wcscmp(version, L"1.0"))
    ns = L"http://www.cellml.org/cellml/1.0#";
  else if (!wcscmp(version, L"1.1"))
    ns = L"http://www.cellml.org/cellml/1.1#";
  else // unrecognised CellML version requested.
    throw iface::cellml_api::CellMLException();
  RETURN_INTO_OBJREF(dt, iface::dom::DocumentType,
                     domimpl->createDocumentType(L"model", L"", L""));
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     domimpl->createDocument(ns, L"model", dt));
  RETURN_INTO_OBJREF(de, iface::dom::Element,
                     doc->documentElement());
  return new CDA_Model(ul, doc, de);
}

wchar_t*
CDA_CellMLBootstrap::serialiseNode(iface::dom::Node* aNode)
  throw(std::exception&)
{
  DOMWriter dw;
  std::wstring str;
  dw.writeNode(NULL, aNode, str);
  return CDA_wcsdup(str.c_str());
}

wchar_t*
CDA_CellMLBootstrap::makeURLAbsolute
(const wchar_t* aRelTo, const wchar_t* aRelURL) throw()
{
  std::wstring aURL(aRelURL);

  // It may already be an absolute URL...
  if (aURL.find(L"://") != std::wstring::npos)
    return CDA_wcsdup(aRelURL);

  if (aURL.find(L"://") != std::wstring::npos)
    return CDA_wcsdup(aRelURL);

  std::wstring base(aRelTo);

  // See if it is a '/' type URL...
  if (aURL[0] == L'/')
  {
    size_t pos = base.find(L"://");
    // Bail if we are trying to resolve relative to a relative URL...
    if (pos == std::wstring::npos)
      return CDA_wcsdup(aRelURL);

    // Assume protocol://host/path, where host may be zero length e.g. file:///
    pos = base.find(L"/", pos + 3);
    std::wstring absURL;
    if (pos == std::wstring::npos)
      absURL = base;
    else
      // Don't include the slash, only everything up to it...
      absURL = base.substr(0, pos);
    absURL += aURL;
    aURL.assign(absURL);
    return CDA_wcsdup(aURL.c_str());
  }

  // No point trying to deal with a zero-length base URI.
  if (base.length() == 0)
    return CDA_wcsdup(aRelURL);

  // It is a completely relative URL.
  // See if base ends in a /...
  size_t pos = base.find(L"://");
  if (base[base.length() - 1] != L'/')
  {
    // aURL last component needs to be removed...
    size_t tpos = base.rfind(L"/");
    if (tpos == std::wstring::npos || tpos < pos + 3)
      base += L"/";
    else
      base = base.substr(0, tpos + 1);
  }
  base += aURL;

  // Substitute [^/]*/../ => / and /./ => /
  size_t prepathlength = 3;
  size_t pos2 = base.find(L"/", pos + 3);

  if (pos2 != std::wstring::npos)
    prepathlength = pos2 - pos + 1;

  pos += prepathlength;

  aURL.assign(base.substr(0, pos));

  std::list<std::wstring> pathComponents;
  bool last = false;

  do
  {
    pos2 = base.find(L"/", pos);
    if (pos2 == std::wstring::npos)
    {
      last = true;
      pos2 = base.length();
    }

    // Don't bother putting an empty path component for //
    if (pos2 != pos)
    {
      std::wstring str = base.substr(pos, pos2 - pos);
      if (str == L"..")
      {
        if (!pathComponents.empty())
          pathComponents.pop_back();
      }
      else if (str == L".")
        ;
      else
        pathComponents.push_back(str);
    }
    pos = pos2 + 1;
  }
  while (!last);

  bool first = true;

  // Now go through the path components and make a path...
  std::list<std::wstring>::iterator i;
  for (i = pathComponents.begin(); i != pathComponents.end(); i++)
  {
    if (first)
      first = false;
    else
      aURL += L'/';

    aURL += *i;
  }
  if (base[base.length() - 1] == L'/')
    aURL += L'/';

  return CDA_wcsdup(aURL.c_str());
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

iface::dom::Document*
CDA_DOMURLLoader::loadDocumentFromText(const wchar_t* xmlText)
  throw(std::exception&)
{
  iface::dom::Document* d = mDOMImpl->loadDocumentFromText(xmlText, mLastError);
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

iface::cellml_api::Model*
CDA_ModelLoader::createFromText(const wchar_t* xmlText)
  throw (std::exception&)
{
  ObjRef<iface::dom::Document> modelDoc;
  try
  {
    modelDoc = already_AddRefd<iface::dom::Document>
      (mURLLoader->loadDocumentFromText(xmlText));
  }
  catch (...)
  {
    wchar_t* str = mURLLoader->lastErrorMessage();
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

    return new CDA_Model(mURLLoader, modelDoc, modelEl);
  }
  catch (iface::dom::DOMException& de)
  {
    mLastError = L"badxml/0/0/Missing document element";
    throw iface::cellml_api::CellMLException();
  }
}

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

    CDA_Model* model = new CDA_Model(loader, modelDoc, modelEl);

    RETURN_INTO_OBJREF(bu, iface::cellml_api::URI, model->xmlBase());
    RETURN_INTO_WSTRING(base, bu->asText());
    if (base == L"")
      bu->asText(URL);

    return model;
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
  CDA_IMPL_QI1(cellml_api::DocumentLoadedListener);
  CDA_IMPL_ID;

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

iface::xpath::XPathEvaluator*
CDA_CellMLBootstrap::xpathEvaluator()
  throw()
{
  return NULL;
}

int myfunc() { return 0; }

CDA_EXPORT_PRE CDA_EXPORT_POST iface::cellml_api::CellMLBootstrap*
CreateCellMLBootstrap()
{
  return new CDA_CellMLBootstrap();
} 

#ifdef DEBUG_PTR_TAG
CDA_EXPORT_PRE CDA_EXPORT_POST std::map<void*,uint32_t> currentlyActivePtrTags;
#endif
