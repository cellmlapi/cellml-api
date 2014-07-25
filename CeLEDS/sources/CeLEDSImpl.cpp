#define IN_CELEDS_MODULE
#define MODULE_CONTAINS_CeLEDS
#include "CeLEDSImpl.hpp"
#include "CeLEDSBootstrap.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include "MaLaESBootstrap.hpp"
#include <sstream>
#include <iostream>

static std::wstring
getTextContents(iface::dom::Node* inNode)
  throw(std::exception&);

CDA_LanguageDictionary::CDA_LanguageDictionary(const std::wstring& nameSpace, iface::dom::Element *DictionaryXML)
  throw ()
  : mNameSpace(nameSpace), mDictionaryXML(DictionaryXML)
{
}

already_AddRefd<iface::dom::NodeList>
CDA_LanguageDictionary::getMappings()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(entries, iface::dom::NodeList,
      mDictionaryXML->getElementsByTagNameNS(mNameSpace.c_str(), L"mapping"));
  entries->add_ref();
  return entries.getPointer();
}

std::wstring
CDA_LanguageDictionary::getValue(const std::wstring& keyName)
  throw(std::exception&)
{
  // Return a value from the dictionary
  uint32_t i;
  RETURN_INTO_OBJREF(entries, iface::dom::NodeList,
                     getMappings());

  for (i=0; i < entries->length(); i++)
  {
    RETURN_INTO_OBJREF(currentNode, iface::dom::Node, entries->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(currentEl, currentNode, dom::Element);
    if (currentEl == NULL)
      continue;

    // If keyname matches then return node
    RETURN_INTO_WSTRING(mapKeyName, currentEl->getAttribute(L"keyname"));
    if (mapKeyName == keyName)
      return getTextContents(currentNode);
  }

  return L"";
}

static std::wstring
getTextContents(iface::dom::Node* inNode)
  throw(std::exception&)
{
  uint32_t i;
  RETURN_INTO_OBJREF(children, iface::dom::NodeList, inNode->childNodes());
  for (i = 0; i < children->length(); i++)
  {
    RETURN_INTO_OBJREF(n, iface::dom::Node, children->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(tn, n, dom::Text);

    if (tn != NULL)
      return tn->data();
  }

  return L"";
}

CDA_DictionaryGenerator::CDA_DictionaryGenerator(iface::dom::Document* LangXML)
  throw()
  : mLangXML(LangXML)
{
}

already_AddRefd<iface::cellml_services::LanguageDictionary>
CDA_DictionaryGenerator::getDictionary(const std::wstring& dictionaryNameSpace)
  throw(std::exception&)
{
  // Use XML element dictionary corresponding to
  // supplied namespace to create new language definition component
  RETURN_INTO_OBJREF(dictionary, iface::dom::Element,
                     getElementNS(dictionaryNameSpace, L"dictionary"));

  if (dictionary != NULL)
    return new CDA_LanguageDictionary(dictionaryNameSpace, dictionary);
  else
    return NULL;
}

already_AddRefd<iface::dom::Element>
CDA_DictionaryGenerator::getElementNS(const std::wstring& nameSpace, const std::wstring& elementName)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(elements, iface::dom::NodeList,
                     mLangXML->getElementsByTagNameNS(nameSpace, elementName));
  if (elements->length() > 0)
  {
    RETURN_INTO_OBJREF(element, iface::dom::Node, elements->item(0));
    DECLARE_QUERY_INTERFACE_OBJREF(returnElement, element, dom::Element);

    returnElement->add_ref();
    return returnElement.getPointer();
  }
  else
    return NULL;
}

already_AddRefd<iface::cellml_services::MaLaESTransform>
CDA_DictionaryGenerator::getMalTransform()
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(malDict, iface::cellml_services::LanguageDictionary,
                     getDictionary(L"http://www.cellml.org/CeLEDS/MaLaES/1.0#"));
  if (malDict == NULL)
    return NULL;

  uint32_t i;
  std::wstring MalString(L"");

  RETURN_INTO_OBJREF(entries, iface::dom::NodeList,
                     malDict->getMappings());

  for (i=0; i < entries->length(); i++)
  {
    RETURN_INTO_OBJREF(currentNode, iface::dom::Node, entries->item(i));
    DECLARE_QUERY_INTERFACE_OBJREF(currentEl, currentNode, dom::Element);
    if (currentEl == NULL)
      continue;

    // Get attributes
    RETURN_INTO_WSTRING(keyName,
                       currentEl->getAttribute(L"keyname"));
    RETURN_INTO_WSTRING(precedence,
                       currentEl->getAttribute(L"precedence"));

    // Create Mal string
    MalString.append(keyName);
    MalString.append(L": ");
    if (precedence != L"")
    {
      MalString.append(L"#prec[");
      MalString.append(precedence);
      MalString.append(L"]");
    }

    RETURN_INTO_WSTRING(tc, getTextContents(currentNode));
    RETURN_INTO_WSTRING(ptc, padMalString(tc.c_str()));
    MalString.append(ptc);
    MalString.append(L"\r\n");
  }

  // create transformer
  RETURN_INTO_OBJREF(mb, iface::cellml_services::MaLaESBootstrap,
                     CreateMaLaESBootstrap());

  try
  {
    return mb->compileTransformer(MalString.c_str());
  }
  catch (...)
  {
    return NULL;
  }
}

std::wstring
CDA_DictionaryGenerator::padMalString(const std::wstring& inString)
  throw(std::exception&)
{
  // Insert two spaces at beginning of new lines
  std::wstring valueString(inString);
  size_t pos = 0;
  while(pos != std::string::npos)
  {
    pos = valueString.find(L"\n", pos);
    if (pos != std::string::npos)
    {
      valueString.insert(pos+1, L"  ");
      pos += 2;
    }
  }

  return valueString;
}

already_AddRefd<iface::cellml_services::DictionaryGenerator>
CDA_CeLEDSBootstrap::createDictGenerator(const std::wstring& URL)
  throw(std::exception&)
{
  mLoadError = L"";
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(URLLoader, iface::cellml_api::DOMURLLoader,
                     cb->localURLLoader());

  try
  {
    RETURN_INTO_OBJREF(langDoc, iface::dom::Document,
                       URLLoader->loadDocument(URL));
    return new CDA_DictionaryGenerator(langDoc);
  }
  catch (...)
  {
    RETURN_INTO_WSTRING(lem, URLLoader->lastErrorMessage());
    mLoadError = lem;
    return NULL;
  }
}

already_AddRefd<iface::cellml_services::DictionaryGenerator>
CDA_CeLEDSBootstrap::createDictGeneratorFromText(const std::wstring& XMLText)
  throw(std::exception&)
{
  mLoadError = L"";
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(URLLoader, iface::cellml_api::DOMURLLoader,
                     cb->localURLLoader());

  try
  {
    RETURN_INTO_OBJREF(langDoc, iface::dom::Document,
                       URLLoader->loadDocumentFromText(XMLText));
    return new CDA_DictionaryGenerator(langDoc);
  }
  catch (...)
  {
    RETURN_INTO_WSTRING(lem, URLLoader->lastErrorMessage());
    mLoadError = lem;
    return NULL;
  }
}

std::wstring
CDA_CeLEDSBootstrap::loadError() throw()
{
  return mLoadError;
}

already_AddRefd<iface::cellml_services::CeLEDSBootstrap>
CreateCeLEDSBootstrap()
{
  return new CDA_CeLEDSBootstrap();
}
