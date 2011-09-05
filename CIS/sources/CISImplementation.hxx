#include "IfaceCCGS.hxx"
#include "IfaceCIS.hxx"
#include <string>
#include "cda_compiler_support.h"

#undef ENABLE_CONTEXT
#ifdef ENABLE_CONTEXT
#include "IfaceCellML_Context.hxx"

extern void UnloadCIS(void);
#endif

struct CompiledModelFunctions
{
  int (*SetupConstants)(double* CONSTANTS, double* RATES, double* STATES);
  int (*ComputeRates)(double VOI, double* CONSTANTS, double* RATES,
                       double* STATES, double* ALGEBRAIC);
  int (*ComputeVariables)(double VOI, double* CONSTANTS, double* RATES,
                           double* STATES, double* ALGEBRAIC);

};

struct IDACompiledModelFunctions
{
  int (*SetupFixedConstants)(double* CONSTANTS, double* RATES, double *STATES);
  int (*EvaluateVariables)(double VOI, double* CONSTANTS, double* RATES,
                           double *STATES, double* ALGEBRAIC);
  int (*EvaluateEssentialVariables)(double VOI, double* CONSTANTS, double* RATES,
                                    double* OLDRATES, double* STATES,
                                    double* OLDSTATES, double* ALGEBRAIC,
                                    double* CONDVARS);

  int (*ComputeResiduals)(double VOI, double* CONSTANTS, double* RATES,
                          double* OLDRATES, double* STATES, double* OLDSTATES,
                          double* ALGEBRAIC, double* CONDVAR, double* resids);
  int (*ComputeRootInformation)(double VOI, double* CONSTANTS, double* RATES,
                                double* OLDRATES, double* STATES, double* OLDSTATES,
                                double* ALGEBRAIC, double* CONDVAR);
  void (*SetupStateInfo)(double * SI);
};

class CDA_CellMLCompiledModel
  : public iface::cellml_services::ODESolverCompiledModel,
    public iface::cellml_services::DAESolverCompiledModel
{
public:
  CDA_CellMLCompiledModel(
                           void* aModule,
                           iface::cellml_api::Model* aModel,
                           iface::cellml_services::CodeInformation* aCCI,
                           std::string& aDirname
                         );
  ~CDA_CellMLCompiledModel();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

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
  iface::cellml_api::Model* mModel;
  iface::cellml_services::CodeInformation* mCCI;
  std::string mDirname;
};

class CDA_ODESolverModel
  : public CDA_CellMLCompiledModel
{
public:
  CDA_ODESolverModel
  (
   void* aModule, CompiledModelFunctions* aCMF,
   iface::cellml_api::Model* aModel,
   iface::cellml_services::CodeInformation* aCCI,
   std::string& aDirname
  )
    : CDA_CellMLCompiledModel(aModule, aModel, aCCI, aDirname), mCMF(aCMF)
  {}

  ~CDA_ODESolverModel() { delete mCMF; }

  CDA_IMPL_QI2(cellml_services::CellMLCompiledModel, cellml_services::ODESolverCompiledModel);
  CompiledModelFunctions* mCMF;
};

class CDA_DAESolverModel
  : public CDA_CellMLCompiledModel
{
public:
  CDA_DAESolverModel
  (
   void* aModule, IDACompiledModelFunctions* aCMF,
   iface::cellml_api::Model* aModel,
   iface::cellml_services::CodeInformation* aCCI,
   std::string& aDirname
  )
    : CDA_CellMLCompiledModel(aModule, aModel, aCCI, aDirname),
      mCMF(aCMF)
  {}

  ~CDA_DAESolverModel() { delete mCMF; }

  CDA_IMPL_QI2(cellml_services::CellMLCompiledModel, cellml_services::DAESolverCompiledModel);
  IDACompiledModelFunctions* mCMF;
};

class CDA_CellMLIntegrationRun
  : public iface::cellml_services::ODESolverRun,
    public iface::cellml_services::DAESolverRun,
    public CDAThread
{
public:
  CDA_CellMLIntegrationRun();
  ~CDA_CellMLIntegrationRun();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  iface::cellml_services::ODEIntegrationStepType stepType()
    throw (std::exception&);

  void stepType(iface::cellml_services::ODEIntegrationStepType ist)
    throw (std::exception&);

  void setStepSizeControl(double epsAbs, double epsRel, double scalVar,
                          double scalRate, double maxStep) throw (std::exception&);
  void setTabulationStepControl(double tabulationStepSize, bool strictTabulation)
    throw (std::exception&);
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

  void CVODEError(int error_code, const char* module, const char* function,
                  const char* msg);

protected:
  virtual void runthread() = 0;

protected:
  iface::cellml_services::ODEIntegrationStepType mStepType;
  double mEpsAbs, mEpsRel, mScalVar, mScalRate, mStepSizeMax;
  double mStartBvar, mStopBvar, mMaxPointDensity, mTabulationStepSize;
  iface::cellml_services::IntegrationProgressObserver* mObserver;
  typedef std::list<std::pair<uint32_t,double> > OverrideList;
  OverrideList mConstantOverrides, mIVOverrides;
  volatile bool mCancelIntegration;
  bool mStrictTabulation;
  std::string mWhyFailure;
};

class CDA_ODESolverRun
  : public CDA_CellMLIntegrationRun
{
public:
  CDA_ODESolverRun(CDA_ODESolverModel* m) :
    CDA_CellMLIntegrationRun(), mModel(m) {}
  CDA_IMPL_QI2(cellml_services::CellMLIntegrationRun, cellml_services::ODESolverRun);
protected:
  ObjRef<CDA_ODESolverModel> mModel;
  void SolveODEProblem(CompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       double* states, uint32_t algSize, double* algebraic);
  void SolveODEProblemGSL(CompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       double* states, uint32_t algSize, double* algebraic);
  void SolveODEProblemCVODE(CompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       double* states, uint32_t algSize, double* algebraic);
  void runthread();
};

class CDA_DAESolverRun
  : public CDA_CellMLIntegrationRun
{
public:
  CDA_DAESolverRun(CDA_DAESolverModel* m) : CDA_CellMLIntegrationRun(),
                                            mModel(m) {}
  CDA_IMPL_QI2(cellml_services::CellMLIntegrationRun, cellml_services::DAESolverRun);
protected:
  ObjRef<CDA_DAESolverModel> mModel;
  void runthread();

  void SolveDAEProblem(IDACompiledModelFunctions* f, uint32_t constSize,
                       double* constants, uint32_t rateSize, double* rates,
                       uint32_t stateSize, double* states, uint32_t algSize, double* algebraic,
                       uint32_t condVarSize, double* condvars);
};

class CDA_CellMLIntegrationService
  : public iface::cellml_services::CellMLIntegrationService
#ifdef ENABLE_CONTEXT
    , public iface::cellml_context::CellMLModule
#endif
{
public:
  CDA_CellMLIntegrationService()
#ifdef ENABLE_CONTEXT
    : mUnload(NULL)
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

  void setupCodeEnvironment(iface::cellml_services::CodeInformation* cci,
                            std::string& dirname,
                            std::string& sourcename,
                            std::ofstream& ss);

  iface::cellml_services::ODESolverCompiledModel*
  compileModelODE(iface::cellml_api::Model* aModel)
    throw(std::exception&);
  iface::cellml_services::DAESolverCompiledModel*
  compileModelDAE(iface::cellml_api::Model* aModel)
    throw(std::exception&);

  iface::cellml_services::ODESolverRun*
  createODEIntegrationRun(iface::cellml_services::ODESolverCompiledModel* aModel)
    throw(std::exception&);
  iface::cellml_services::DAESolverRun*
  createDAEIntegrationRun(iface::cellml_services::DAESolverCompiledModel* aModel)
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
