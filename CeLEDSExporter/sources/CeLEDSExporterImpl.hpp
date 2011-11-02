#ifndef _CeLEDSExporterImpl_hpp
#define _CeLEDSExporterImpl_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCeLEDSExporter.hxx"
#include <string>
#include "Utilities.hxx"

class CDA_CodeExporter
  : public iface::cellml_services::CodeExporter
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CodeExporter);

  CDA_CodeExporter(iface::cellml_services::DictionaryGenerator *langDictGen) throw();
  ~CDA_CodeExporter() {};

  std::wstring generateCode(iface::cellml_api::Model* model)
    throw(std::exception&);

private:
  // get CCGS code generator, setting patterns using the XML language definition file
  already_AddRefd<iface::cellml_services::CodeGenerator> getExplicitCodeGenerator() 
    throw(std::exception&);
  already_AddRefd<iface::cellml_services::IDACodeGenerator> getImplicitCodeGenerator() 
    throw(std::exception&);
  void transferCommonCodeAttributes(iface::cellml_services::CodeGenerator* aCG);
  int generateCodeCommonHeader(std::wstring&,
                               iface::cellml_services::CodeGenerator*,
                               iface::cellml_services::CodeInformation*);
  int generateCodeCommonBody(std::wstring&,
                             iface::cellml_services::CodeGenerator*,
                             iface::cellml_services::CodeInformation*);
  int generateCodeCommonFooter(std::wstring&,
                               iface::cellml_services::CodeInformation*);

  // get a section of code from dictionary, replacing 
  // occurances of solver parameters where required
  std::wstring getCodeSection(const std::wstring& name)
    throw(std::exception&);

  // Get algebraic variable calculations from rate calculation section
  // for calculating all algebraic variables at once
  std::wstring getAlgebraic(const std::wstring& ratesCalc, const std::wstring& algebraicNamePattern)
    throw(std::exception&);

  // replace a parameter in a string with it's value, eg replace <MAXSTEP> with 0.1
  void replaceParameter(std::wstring &code, const std::wstring& parameter, double value)
    throw(std::exception&);

  // return definitions of extra functions required
  std::wstring defineExtraFunctions(const std::wstring& output)
    throw(std::exception&);

  std::wstring listVariablesByState(iface::cellml_services::CodeInformation *cci, int definedFilter)
    throw(std::exception&);

  std::wstring listFlaggedEquations(iface::cellml_services::CodeInformation *cci)
    throw(std::exception&);

  std::wstring diffop(uint32_t deg) throw(std::exception&);

  double rangeStart() throw();
  void rangeStart(double aValue) throw();
  double rangeEnd() throw();
  void rangeEnd(double aValue) throw();
  double absTol() throw();
  void absTol(double aValue) throw();
  double relTol() throw();
  void relTol(double aValue) throw();
  double maxStep() throw();
  void maxStep(double aValue) throw();

  ObjRef<iface::cellml_services::DictionaryGenerator> mLangDictGen;
  ObjRef<iface::cellml_services::LanguageDictionary> mLangDict, mCCGSLangDict;
  double mRangeStart, mRangeEnd, mAbsTol, mRelTol, mMaxStep;
  std::wstring
  getTextContents(iface::dom::Node* inNode)
    throw(std::exception&);

  std::wstring generateCodeExplicit(iface::cellml_api::Model* aModel);
  std::wstring generateCodeImplicit(iface::cellml_api::Model* aModel);

  typedef enum { CODESTYLE_EXPLICIT, CODESTYLE_IMPLICIT } codestyle_t;
  codestyle_t mCodeStyle;
};

class CDA_CeLEDSExporterBootstrap
  : public iface::cellml_services::CeLEDSExporterBootstrap
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CeLEDSExporterBootstrap);

  CDA_CeLEDSExporterBootstrap() {};
  ~CDA_CeLEDSExporterBootstrap() {};

  already_AddRefd<iface::cellml_services::DictionaryGenerator> createDictGenerator(const std::wstring& URL)
    throw(std::exception&);

  already_AddRefd<iface::cellml_services::DictionaryGenerator> createDictGeneratorFromText(const std::wstring& XMLText)
    throw(std::exception&);

  already_AddRefd<iface::cellml_services::CodeExporter> createExporter(const std::wstring& URL)
    throw(std::exception&);

  already_AddRefd<iface::cellml_services::CodeExporter> createExporterFromText(const std::wstring& XMLText)
    throw(std::exception&);

  std::wstring loadError() throw();

private:
  std::wstring mLoadError;
};

#endif // _CeLEDSExporterImpl_hpp

