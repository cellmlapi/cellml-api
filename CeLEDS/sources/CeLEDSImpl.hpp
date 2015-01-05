#ifndef _CeLEDSImpl_hpp
#define _CeLEDSImpl_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCeLEDS.hxx"
#include <string>
#include "Utilities.hxx"

class CDA_LanguageDictionary
  : public iface::cellml_services::LanguageDictionary
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::LanguageDictionary);

  CDA_LanguageDictionary(const std::wstring& nameSpace, iface::dom::Element *DictionaryXML) throw();
  ~CDA_LanguageDictionary() {};

  std::wstring getValue(const std::wstring& keyName)
    throw(std::exception&);

  already_AddRefd<iface::dom::NodeList> getMappings()
    throw(std::exception&);

private:
  std::wstring mNameSpace;
  ObjRef<iface::dom::Element> mDictionaryXML;
};

class CDA_DictionaryGenerator
  : public iface::cellml_services::DictionaryGenerator
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::DictionaryGenerator);

  CDA_DictionaryGenerator(iface::dom::Document *LangXML) throw();
  ~CDA_DictionaryGenerator() {};

  already_AddRefd<iface::cellml_services::LanguageDictionary>
  getDictionary(const std::wstring& dictionaryNameSpace)
    throw(std::exception&);

  already_AddRefd<iface::dom::Element>
  getElementNS(const std::wstring& nameSpace, const std::wstring& elementName)
    throw(std::exception&);

  already_AddRefd<iface::cellml_services::MaLaESTransform> getMalTransform()
    throw(std::exception&);

  std::wstring padMalString(const std::wstring& inString)
    throw(std::exception&);

private:
  ObjRef<iface::dom::Document> mLangXML;
};

class CDA_CeLEDSBootstrap
  : public iface::cellml_services::CeLEDSBootstrap
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CeLEDSBootstrap);

  CDA_CeLEDSBootstrap() : mLoadError(L"") {};
  ~CDA_CeLEDSBootstrap() {};

  already_AddRefd<iface::cellml_services::DictionaryGenerator> createDictGenerator(const std::wstring& URL)
    throw(std::exception&);

  already_AddRefd<iface::cellml_services::DictionaryGenerator> createDictGeneratorFromText(const std::wstring& XMLText)
    throw(std::exception&);

  std::wstring loadError() throw();

private:
  std::wstring mLoadError;
};

#endif // _CeLEDSImpl_hpp

