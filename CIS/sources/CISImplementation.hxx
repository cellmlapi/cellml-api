#include "IfaceCCGS.hxx"
#include "IfaceCIS.hxx"
#include "ThreadWrapper.hxx"
#include <string>
#include "cda_config.h"

#ifdef ENABLE_CONTEXT
#include "IfaceCellML_Context.hxx"

extern void UnloadCIS(void);
#endif

struct CompiledModelFunctions
{
  void (*SetupConstants)(double* CONSTANTS, double* RATES, double* STATES);
  void (*ComputeRates)(double VOI, double* CONSTANTS, double* RATES,
                       double* STATES, double* ALGEBRAIC);
  void (*ComputeVariables)(double VOI, double* CONSTANTS, double* RATES,
                           double* STATES, double* ALGEBRAIC);
};

class CDA_CellMLCompiledModel
  : public iface::cellml_services::CellMLCompiledModel
{
public:
  CDA_CellMLCompiledModel(
                           void* aModule, CompiledModelFunctions* aCMF,
                           iface::cellml_api::Model* aModel,
                           iface::cellml_services::CodeInformation* aCCI,
                           std::string& aDirname
                          );
  ~CDA_CellMLCompiledModel();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::CellMLCompiledModel);

  iface::cellml_api::Model* model()
    throw(std::exception&)
  {
    mModel->add_ref();
    return mModel;
  }

  iface::cellml_services::CodeInformation* codeInformation()
    throw(std::exception&)
  {
    mCCI->add_ref();
    return mCCI;
  }

  // These are not available directly across CORBA, but read-only access is
  // allowed within the same module...
  void* mModule;
  CompiledModelFunctions* mCMF;
  iface::cellml_api::Model* mModel;
  iface::cellml_services::CodeInformation* mCCI;
  std::string mDirname;
};

class CDA_CellMLIntegrationRun
  : public iface::cellml_services::CellMLIntegrationRun,
    public CDAThread
{
public:
  CDA_CellMLIntegrationRun(
                           iface::cellml_services::CellMLCompiledModel* aModel
                          );
  ~CDA_CellMLIntegrationRun();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::CellMLIntegrationRun);

  iface::cellml_services::IntegrationStepType stepType()
    throw (std::exception&);

  void stepType(iface::cellml_services::IntegrationStepType ist)
    throw (std::exception&);

  void setStepSizeControl(double epsAbs, double epsRel, double scalVar,
                          double scalRate, double maxStep) throw (std::exception&);
  void setResultRange(double startBvar, double stopBvar, double incrementBvar)
    throw (std::exception&);
  void setProgressObserver(iface::cellml_services::IntegrationProgressObserver*
                           aIpo)
    throw (std::exception&);
  void setOverride(iface::cellml_services::VariableEvaluationType aType,
                   uint32_t variableIndex, double newValue)
    throw (std::exception&);
  void start() throw (std::exception&);
  void stop() throw (std::exception&);

protected:
  void runthread();

private:
  void SolveODEProblem(CompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       double* states, uint32_t algSize, double* algebraic);
  void SolveODEProblemGSL(CompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       double* states, uint32_t algSize, double* algebraic);
  void SolveODEProblemCVODE(CompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       double* states, uint32_t algSize, double* algebraic);

  CDA_CellMLCompiledModel* mModel;
  iface::cellml_services::IntegrationStepType mStepType;
  double mEpsAbs, mEpsRel, mScalVar, mScalRate, mStepSizeMax;
  double mStartBvar, mStopBvar, mMaxPointDensity;
  iface::cellml_services::IntegrationProgressObserver* mObserver;
  typedef std::list<std::pair<uint32_t,double> > OverrideList;
  OverrideList mConstantOverrides, mIVOverrides;
  bool mCancelIntegration;
};

class CDA_CellMLIntegrationService
  : public iface::cellml_services::CellMLIntegrationService
#ifdef ENABLE_CONTEXT
    , public iface::cellml_context::CellMLModule
#endif
{
public:
  CDA_CellMLIntegrationService()
    : _cda_refcount(1)
#ifdef ENABLE_CONTEXT
  , mUnload(NULL)
#endif
  {
  }

  ~CDA_CellMLIntegrationService()
  {
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

#ifdef ENABLE_CONTEXT
  CDA_IMPL_QI2(cellml_services::CellMLIntegrationService,
               cellml_context::CellMLModule);
#else
  CDA_IMPL_QI1(cellml_services::CellMLIntegrationService);
#endif

  iface::cellml_services::CellMLCompiledModel*
  compileModel(iface::cellml_api::Model* aModel)
    throw(std::exception&);

  iface::cellml_services::CellMLIntegrationRun*
  createIntegrationRun(iface::cellml_services::CellMLCompiledModel* aModel)
    throw(std::exception&);
  
  wchar_t* lastError() throw(std::exception&)
  {
    return CDA_wcsdup(mLastError.c_str());
  }

#ifdef ENABLE_CONTEXT
  iface::cellml_context::CellMLModule::ModuleTypes moduleType()
    throw(std::exception&)
  {
    return iface::cellml_context::CellMLModule::SERVICE;
  }

  wchar_t* moduleName() throw (std::exception&)
  {
    return CDA_wcsdup(L"CIS");
  }

  wchar_t* moduleDescription() throw (std::exception&)
  {
    return CDA_wcsdup(L"The CellML Integration Service");
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

  void SetUnloadCIS(void (*unload)(void))
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
