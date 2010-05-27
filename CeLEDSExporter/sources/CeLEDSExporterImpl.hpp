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

  // get CCGS code generator, setting patterns using the XML language definition file
  iface::cellml_services::CodeGenerator* getCodeGenerator() 
    throw(std::exception&);

  wchar_t* generateCode(iface::cellml_api::Model* model)
    throw(std::exception&);

  // get a section of code from dictionary, replacing 
  // occurances of solver parameters where required
  std::wstring getCodeSection(const wchar_t* name)
    throw(std::exception&);

  // Get algebraic variable calculations from rate calculation section
  // for calculating all algebraic variables at once
  std::wstring getAlgebraic(const wchar_t* ratesCalc, const wchar_t* algebraicNamePattern)
    throw(std::exception&);

  // replace a parameter in a string with it's value, eg replace <MAXSTEP> with 0.1
  void replaceParameter(std::wstring &code, const wchar_t* parameter, double value)
    throw(std::exception&);

  // return definitions of extra functions required
  std::wstring defineExtraFunctions(const std::wstring output)
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

private:
  ObjRef<iface::cellml_services::DictionaryGenerator> mLangDictGen;
  double mRangeStart, mRangeEnd, mAbsTol, mRelTol, mMaxStep;
  wchar_t*
  getTextContents(iface::dom::Node* inNode)
    throw(std::exception&);
};

class CDA_CeLEDSExporterBootstrap
  : public iface::cellml_services::CeLEDSExporterBootstrap
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CeLEDSExporterBootstrap);

  CDA_CeLEDSExporterBootstrap() : _cda_refcount(1) {};
  ~CDA_CeLEDSExporterBootstrap() {};

  iface::cellml_services::DictionaryGenerator* createDictGenerator(const wchar_t* URL)
    throw(std::exception&);

  iface::cellml_services::DictionaryGenerator* createDictGeneratorFromText(const wchar_t* XMLText)
    throw(std::exception&);

  iface::cellml_services::CodeExporter* createExporter(const wchar_t* URL)
    throw(std::exception&);

  iface::cellml_services::CodeExporter* createExporterFromText(const wchar_t* XMLText)
    throw(std::exception&);

  wchar_t* loadError() throw();

private:
  std::wstring mLoadError;
};

#endif // _CeLEDSExporterImpl_hpp

