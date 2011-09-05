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

  CDA_LanguageDictionary(const wchar_t* nameSpace, iface::dom::Element *DictionaryXML) throw();
  ~CDA_LanguageDictionary() {};

  wchar_t* getValue(const wchar_t* keyName) 
    throw(std::exception&);

  iface::dom::NodeList* getMappings()
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

  iface::cellml_services::LanguageDictionary* 
  getDictionary(const wchar_t* dictionaryNameSpace) 
    throw(std::exception&);

  iface::dom::Element* 
  getElementNS(const wchar_t* nameSpace, const wchar_t* elementName)
    throw(std::exception&);

  iface::cellml_services::MaLaESTransform* getMalTransform() 
    throw(std::exception&);

  wchar_t* padMalString(const wchar_t* inString) 
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

  iface::cellml_services::DictionaryGenerator* createDictGenerator(const wchar_t* URL)
    throw(std::exception&);

  iface::cellml_services::DictionaryGenerator* createDictGeneratorFromText(const wchar_t* XMLText)
    throw(std::exception&);

  wchar_t* loadError() throw();

private:
  std::wstring mLoadError;
};

#endif // _CeLEDSImpl_hpp

