#ifndef _CCGSImplementation_hpp
#define _CCGSImplementation_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "IfaceCCGS.hxx"
#include "Utilities.hxx"
#include <sstream>
#include <vector>
#include <list>

#ifdef ENABLE_CONTEXT
#include "IfaceCellML_Context.hxx"

extern void UnloadCCGS(void);
#endif

class CDA_CGenerator
  : public iface::cellml_services::CGenerator
#ifdef ENABLE_CONTEXT
    , public iface::cellml_context::CellMLModule
#endif
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
#ifdef ENABLE_CONTEXT
  CDA_IMPL_QI2(cellml_services::CGenerator, cellml_context::CellMLModule);
#else
  CDA_IMPL_QI1(cellml_services::CGenerator);
#endif

  CDA_CGenerator() : _cda_refcount(1)
#ifdef ENABLE_CONTEXT
  , mUnload(NULL)
#endif
  {}
  ~CDA_CGenerator() {}

  iface::cellml_services::CCodeInformation*
  generateCode(iface::cellml_api::Model* aSourceModel)
    throw(std::exception&);

  wchar_t* lastError() throw (std::exception&)
  { return CDA_wcsdup(mLastError.c_str()); }

#ifdef ENABLE_CONTEXT
  iface::cellml_context::CellMLModule::ModuleTypes moduleType()
    throw(std::exception&)
  {
    return iface::cellml_context::CellMLModule::SERVICE;
  }

  wchar_t* moduleName() throw (std::exception&)
  {
    return CDA_wcsdup(L"CCCGS");
  }

  wchar_t* moduleDescription() throw (std::exception&)
  {
    return CDA_wcsdup(L"The CellML C Code Generation Service");
  }

  wchar_t* moduleVersion() throw (std::exception&)
  {
    return CDA_wcsdup(L"0.0");
  }

  wchar_t* moduleIconURL() throw (std::exception&)
  {
    return CDA_wcsdup(L"");
  }

  void unload() throw (std::exception&)
  {
    if (mUnload != NULL)
      mUnload();
  }

  void SetUnloadCCGS(void (*unload)(void))
  {
    mUnload = unload;
  }
#endif

private:
  std::wstring mLastError;
#ifdef ENABLE_CONTEXT
  void (*mUnload)();
#endif
};

class CDA_CCodeInformation
  : public iface::cellml_services::CCodeInformation
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CCodeInformation);

  CDA_CCodeInformation(iface::cellml_api::Model* aSourceModel);
  ~CDA_CCodeInformation();

  iface::cellml_services::ModelConstraintLevel constraintLevel()
    throw (std::exception&);
  uint32_t variableCount() throw (std::exception&);
  uint32_t constantCount() throw (std::exception&);
  uint32_t boundCount() throw (std::exception&);
  uint32_t rateVariableCount() throw (std::exception&);
  char* fixedConstantFragment() throw (std::exception&);
  char* computedConstantFragment() throw (std::exception&);
  char* rateCodeFragment() throw (std::exception&);
  char* variableCodeFragment() throw (std::exception&);
  char* functionsFragment() throw (std::exception&);
  iface::cellml_services::CCodeVariableIterator* iterateVariables()
    throw (std::exception&);
  iface::mathml_dom::MathMLNodeList* flaggedEquations()
    throw (std::exception&);
private:
  iface::cellml_services::ModelConstraintLevel mConstraintLevel;
  uint32_t mVariableCount, mConstantCount, mBoundCount, mRateVariableCount;
  std::stringstream mFixedConstantFragment, mComputedConstantFragment,
    mRateCodeFragment, mVariableCodeFragment, mFunctionsFragment;
  typedef std::list<iface::cellml_services::CCodeVariable*> CCVL_t;
  CCVL_t mVariables;
  typedef std::vector<iface::dom::Element*> MNL_t;
  MNL_t mFlaggedEquations;
};

class CDA_CCodeVariable
  : public iface::cellml_services::CCodeVariable
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CCodeVariable);

  CDA_CCodeVariable
  (
   iface::cellml_api::CellMLVariable* aVar, uint32_t aVariableIndex=0,
   bool aHasDifferential=false, uint32_t aDerivative=0,
   iface::cellml_services::VariableEvaluationType aType=
   iface::cellml_services::COMPUTED
  )
    : _cda_refcount(1), mVar(aVar), mVariableIndex(aVariableIndex),
      mHasDifferential(aHasDifferential), mDerivative(aDerivative),
      mType(aType)
  {
    mVar->add_ref();
  }

  ~CDA_CCodeVariable()
  {
    mVar->release_ref();
  }

  uint32_t variableIndex()
    throw (std::exception&)
  {
    return mVariableIndex;
  }

  bool hasDifferential()
    throw (std::exception&)
  {
    return mHasDifferential;
  }

  uint32_t derivative()
    throw (std::exception&)
  {
    return mDerivative;
  }

  iface::cellml_services::VariableEvaluationType type()
    throw (std::exception&)
  {
    return mType;
  }

  iface::cellml_api::CellMLVariable* source()
    throw (std::exception&)
  {
    mVar->add_ref();
    return mVar;
  }

private:
  iface::cellml_api::CellMLVariable* mVar;
  uint32_t mVariableIndex;
  bool mHasDifferential;
  uint32_t mDerivative;
  iface::cellml_services::VariableEvaluationType mType;
};

class CDA_CCodeVariableIterator
  : public iface::cellml_services::CCodeVariableIterator
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CCodeVariableIterator);

  CDA_CCodeVariableIterator
  (
   CDA_CCodeInformation* aInformation,
   const std::list<iface::cellml_services::CCodeVariable*>::iterator& aBegin,
   const std::list<iface::cellml_services::CCodeVariable*>::iterator& aEnd
  )
    : _cda_refcount(1), mInformation(aInformation), mCursor(aBegin), mEnd(aEnd)
  {
  }

  ~CDA_CCodeVariableIterator() {}

  iface::cellml_services::CCodeVariable* nextVariable()
    throw(std::exception&)
  {
    if (mCursor == mEnd)
      return NULL;
    iface::cellml_services::CCodeVariable* ccv = (*mCursor);
    mCursor++;
    ccv->add_ref();
    return ccv;
  }
private:
  ObjRef<CDA_CCodeInformation> mInformation;
  std::list<iface::cellml_services::CCodeVariable*>::iterator mCursor, mEnd;
};

class CDA_CCodeMathList
  : public iface::mathml_dom::MathMLNodeList
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(mathml_dom::MathMLNodeList,
               dom::NodeList);

  CDA_CCodeMathList(CDA_CCodeInformation* aInformation,
                    std::vector<iface::dom::Element*>& aNodes)
    : _cda_refcount(1), mInformation(aInformation), mNodes(aNodes)
  {
    std::vector<iface::dom::Element*>::iterator i;
    for (i = mNodes.begin(); i != mNodes.end(); i++)
      (*i)->add_ref();
  }

  ~CDA_CCodeMathList()
  {
    std::vector<iface::dom::Element*>::iterator i;
    for (i = mNodes.begin(); i != mNodes.end(); i++)
      (*i)->release_ref();
  }
  
  iface::dom::Node* item(uint32_t index)
    throw(std::exception&)
  {
    if (mNodes.size() <= index)
      return NULL;
    mNodes[index]->add_ref();
    return mNodes[index];
  }

  uint32_t length()
    throw(std::exception&)
  {
    return mNodes.size();
  }
private:
  ObjRef<CDA_CCodeInformation> mInformation;
  std::vector<iface::dom::Element*> mNodes;
};

#endif // _CCGSImplementation_hpp
