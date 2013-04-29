#define IN_CIS_MODULE
#define MODULE_CONTAINS_CIS
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include "Utilities.hxx"
#include <stdlib.h>
#include <string>
#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <dlfcn.h>
#include <sys/utsname.h>
#include <unistd.h>
#else
#include <io.h>
#define pipe _pipe
#endif
#ifdef _MSC_VER
#include <errno.h>
#endif
#include "CISImplementation.hxx"
#include <fstream>
#include "CISBootstrap.hpp"
#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#endif
#include "CCGSBootstrap.hpp"
#include "MaLaESBootstrap.hpp"

// Ensure these symbols are available for llvm to use:
#undef min
#undef max
#ifdef ENABLE_CLANG
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Job.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/Util.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include <clang/CodeGen/CodeGenAction.h>
#include <llvm/Support/MemoryBuffer.h>
#include <sstream>
#endif

class CompiledModule {
public:
#ifdef ENABLE_CLANG
  llvm::OwningPtr<llvm::Module> mModule;
  llvm::OwningPtr<llvm::ExecutionEngine> mExecutionEngine;
  std::list<llvm::Function*> mFunctions;

  void*
  getSymbol(const char* aName)
  {
    llvm::Function* f = mModule->getFunction(aName);
    // mFunctions.push_back(f);
    return mExecutionEngine->getPointerToFunction(f);
  }

  ~CompiledModule()
  {
    for (std::list<llvm::Function*>::iterator i = mFunctions.begin();
         i != mFunctions.end(); i++)
      delete (*i);
  }
#else
  void* mModule;

  void*
  getSymbol(const char* aName)
  {
#ifdef WIN32
#define getsym(m,s) GetProcAddress((HMODULE)m, s)
#else
#define getsym(m,s) dlsym(m,s)
#endif
    return (void*)getsym(mModule, aName);
#undef getsym
  }

  ~CompiledModule() {
#ifdef WIN32
    FreeLibrary((HMODULE)mModule);
#else
    dlclose(mModule);
#endif
  }
#endif
};

char*
attempt_make_tempdir(const char* parentDir)
{
  std::string basename = parentDir;
  basename += "/";
  while (true)
  {
#define DICT "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_."
    std::string fn = basename;
    uint32_t i;
    for (i = 0; i < 5; i++)
    {
      uint32_t r = mersenne_genrand_int32();
      fn += DICT[r & 0x3F];
      fn += DICT[(r >> 6) & 0x3F];
      fn += DICT[(r >> 12) & 0x3F];
      fn += DICT[(r >> 18) & 0x3F];
      fn += DICT[(r >> 24) & 0x3F];
    }
    if (mkdir(fn.c_str()
#ifndef WIN32
        , 0700
#endif
       ) == 0)
      return strdup(fn.c_str());
    // If the error is EEXIST, we can try again with another name. If not, we
    // should fail.
    if (errno != EEXIST)
      return NULL;
  }
}

CompiledModelFunctions*
SetupCompiledModelFunctions(CompiledModule* module)
{
  CompiledModelFunctions* cmf = new CompiledModelFunctions;
  cmf->SetupConstants = (void (*)(double*, double*, double*, struct Override*, struct fail_info*))
    module->getSymbol("SetupConstants");
  cmf->ComputeRates = (void (*)(double,double*,double*,double*,double*, struct fail_info*))
    module->getSymbol("ComputeRates");
  cmf->ComputeVariables = (void (*)(double,double*,double*,double*,double*,struct fail_info*))
    module->getSymbol("ComputeVariables");
  return cmf;
}

IDACompiledModelFunctions*
SetupIDACompiledModelFunctions(CompiledModule* module)
{
  IDACompiledModelFunctions* cmf = new IDACompiledModelFunctions;
  cmf->SetupFixedConstants = (void (*)(double*, double*, double*, double*, struct Override*, struct fail_info*))
    module->getSymbol("SetupFixedConstants");
  cmf->EvaluateVariables = (void (*)(double, double*, double*, double*, double*, double*, struct fail_info*))
    module->getSymbol("EvaluateVariables");
  cmf->EvaluateEssentialVariables = (void (*)(double, double*, double*, double*, double*, double*, double*, double*, struct fail_info*))
    module->getSymbol("EvaluateEssentialVariables");
  cmf->ComputeResiduals = (void (*)(double, double*, double*, double*, double*, double*, double*, double*, double*, struct fail_info*))
    module->getSymbol("ComputeResiduals");
  cmf->ComputeRootInformation = (void (*)(double, double*, double*, double*, double*, double*, double*, double*, struct fail_info*))
    module->getSymbol("ComputeRootInformation");
  cmf->SetupStateInfo = (void (*)(double*))
    module->getSymbol("SetupStateInfo");
  return cmf;
}

CompiledModule*
CDA_CellMLIntegrationService::CompileSource
(
 std::string& destDir, std::string& sourceFile, std::wstring& lastError
)
{
#ifdef ENABLE_CLANG
  LLVMLinkInJIT();
  // This code is modified from the code in OpenCOR.

  // Firstly use the driver as if we were going to do a compilation, but don't
  // actually run the compilation (we just do this to get the arguments we need).
  clang::DiagnosticsEngine diagnosticsEngine
    (
     llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs>(new clang::DiagnosticIDs()),
     NULL
    );
  clang::driver::Driver driver(
                               "internal code compilation",
                               llvm::sys::getDefaultTargetTriple(),
                               "", true, diagnosticsEngine);

  const char* args[] = { "internal code compilation", "-fsyntax-only", "-Wno-implicit-function-declaration", "-O3", sourceFile.c_str()
                       };
  llvm::OwningPtr<clang::driver::Compilation> compilation
    (driver.BuildCompilation(llvm::ArrayRef<const char*>(args, sizeof(args)/sizeof(char*))));

  if (!compilation)
  {
    mLastError = L"Cannot create an LLVM compiler driver.";
    throw iface::cellml_api::CellMLException();
  }

  const clang::driver::JobList &jobList =
    compilation->getJobs();
  const clang::driver::Command* command =
    llvm::cast<clang::driver::Command>(*(compilation->getJobs().begin()));
  const clang::driver::ArgStringList &commandArguments = command->getArguments();
  llvm::OwningPtr<clang::CompilerInvocation> compilerInvocation(new clang::CompilerInvocation());

  clang::CompilerInvocation::CreateFromArgs(*compilerInvocation.get(),
                                            const_cast<const char **>(commandArguments.data()),
                                            const_cast<const char **>(commandArguments.data())+
                                            commandArguments.size(),
                                            diagnosticsEngine);

  // By default, Clang deliberately leaks memory so it is faster if it is
  // just going to exit anyway. Tell it not to do that.
  compilerInvocation->getFrontendOpts().DisableFree = 0;

    // Create a compiler instance to handle the actual work

    clang::CompilerInstance compilerInstance;

    compilerInstance.setInvocation(compilerInvocation.take());

    // Create the compiler instance's diagnostics engine

#ifdef DEBUG_LLVM
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagnosticOptions = new clang::DiagnosticOptions();
#endif
    compilerInstance.createDiagnostics(int(commandArguments.size()),
                                       const_cast<char **>(commandArguments.data()),
#ifdef DEBUG_LLVM
                                       new clang::TextDiagnosticPrinter(llvm::outs(), &*diagnosticOptions)
#else
                                       NULL
#endif
                                      );
    // Create an LLVM module

    llvm::OwningPtr<CompiledModule> clangData(new CompiledModule());

    clangData->mModule.reset
      (new llvm::Module(sourceFile, llvm::getGlobalContext()));

    // Initialise the native target, so not only can we then create a JIT
    // execution engine, but more importantly its data layout will match that of
    // our target platform...

    llvm::InitializeNativeTarget();

    // Create an execution engine

    std::string whyFail;
    clangData->mExecutionEngine.reset
      (llvm::ExecutionEngine::createJIT(clangData->mModule.get(), &whyFail));

    if (!clangData->mExecutionEngine)
    {
      wchar_t buffer[1024];
      mbstowcs(buffer, whyFail.c_str(), 1024);
      buffer[1023] = 0;
      mLastError = std::wstring(L"Cannot create LLVM execution engine: ") + buffer;
      throw iface::cellml_api::CellMLException();
    }

    // Create and execute the frontend to generate the LLVM assembly code,
    // making sure that all added functions end up in the same module

    llvm::OwningPtr<clang::CodeGenAction> codeGenerationAction
      (new clang::EmitLLVMOnlyAction(&clangData->mModule->getContext()));

    codeGenerationAction->setLinkModule(clangData->mModule.take());

    if (!compilerInstance.ExecuteAction(*codeGenerationAction))
    {
      mLastError = L"Error generating code with LLVM.";
      throw iface::cellml_api::CellMLException();
    }

    // Switch from the source module to the linked module.
    clangData->mModule.reset(codeGenerationAction->takeModule());

    return clangData.take();
#else // ENABLE_CLANG
  setvbuf(stdout, NULL, _IONBF, 0);
  std::string targ = destDir;
#ifdef WIN32
  targ += "/generated.dll";
#else
  targ += "/generated.so";
#endif
  std::string cmd = "gcc -ggdb "
#ifdef WIN32
    "-mthreads -Llib -L. -lcis "
#else
    "-nodefaultlibs "
#ifdef __MACH__
    "-arch "
#ifdef __amd64__
    "x86_64"
#else
#ifdef __i386__
    "i386"
#else
    "ppc"
#endif
#endif
    " -L. -L./../Resources/lib -L./../Resources/components -Llib -lcellml -lannotools -lcuses -lmalaes -lcevas -lcis -lccgs "
#else
    "-Llib -lcis "
#endif
#endif
    "-O3 "
#ifdef ENABLE_FAST_MATH
    "-ffast-math "
#endif
#ifdef __MACH__
    "-lm -dynamiclib -o ";
#else
    "-shared -o";
#endif

  cmd += targ;
  cmd += " ";
  cmd += sourceFile;
#ifdef WIN32
  // -1 means no, 1 yes, 0 means to be determined...
  static int need_no_cygwin = 0;
  if (need_no_cygwin == 0)
  {
    char* dumpstring = "gcc -dumpspecs";
    SECURITY_ATTRIBUTES sa; 
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Set up to inherit stdout, by allowing handle inheritance...
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    HANDLE stdoutForNewProc = NULL, stdoutFromNewProc = NULL;
    CreatePipe(&stdoutFromNewProc, &stdoutForNewProc, &sa, 0);
    // Only one end of the pipe needs to be inherited by the new process...
    SetHandleInformation(&stdoutFromNewProc, HANDLE_FLAG_INHERIT, 0);

    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = stdoutForNewProc;

    if (!CreateProcess(NULL, dumpstring, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
      CloseHandle(stdoutForNewProc);
      need_no_cygwin = -1;
    }
    else
    {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      CloseHandle(stdoutForNewProc);
      char buf[1024];
      DWORD readCount;
      std::string spec;
      while (ReadFile(stdoutFromNewProc, buf, sizeof(buf), &readCount, NULL) && readCount != 0)
        spec += std::string(buf, readCount);
      if (spec.find("mno-cygwin") != std::string::npos)
        need_no_cygwin = 1;
      else
        need_no_cygwin = -1;
    }
    CloseHandle(stdoutFromNewProc);
  }

  if (need_no_cygwin > 0)
    cmd += " -mno-cygwin";

  // Need this at the end for the import library to work in all cases.
  cmd += " -lcis";
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    char *commandstring = strdup(cmd.c_str());

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        commandstring,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,              // No visible window
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
      free(commandstring);
      printf( "CreateProcess failed (%d)\n", GetLastError() );
      throw iface::cellml_api::CellMLException();
    }
    free(commandstring);
    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );
    DWORD lpExitCode;
    GetExitCodeProcess( pi.hProcess, &lpExitCode );
    int ret = lpExitCode;

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else
  // Add the "-fPIC" flag in case we are running on a 64-bit machine
  // Note: we currently only check against "x86_64", but this doesn't
  //       mean that there aren't other machines that also need that
  //       flag...

  utsname u;
  uname(&u);

  if (!strcmp(u.machine, "x86_64"))
    cmd += " -fPIC";

  // Execute the command (i.e. compile the model)

  int ret = system(cmd.c_str());
#endif
  if (ret != 0)
  {
    lastError = L"Could not compile the model code.";
    throw iface::cellml_api::CellMLException();
  }

#ifdef WIN32
  void* t = LoadLibrary(targ.c_str());
#else
  void* t = dlopen(targ.c_str(), RTLD_NOW);
#endif
  if (t == NULL)
  {
    lastError = L"Cannot load the model code module";
#ifndef WIN32
    char* msg = dlerror();
    uint32_t l = strlen(msg) + 1;
    wchar_t buf[l];
    any_swprintf(buf, l, L"%s", msg);
    lastError += L" (";
    lastError += buf;
    lastError += L").";
#else
    lastError += L".";
#endif
    throw iface::cellml_api::CellMLException();
  }

  CompiledModule *mod = new CompiledModule();
  mod->mModule = t;
  return mod;
#endif // !ENABLE_CLANG
}

CDA_CellMLCompiledModel::CDA_CellMLCompiledModel
(
 CompiledModule* aModule,
 iface::cellml_api::Model* aModel,
 iface::cellml_services::CodeInformation* aCCI,
 std::string& aDirname
)
  : mModule(aModule), mModel(aModel), mCCI(aCCI),
    mDirname(aDirname)
{
}

CDA_CellMLCompiledModel::~CDA_CellMLCompiledModel()
{
  delete mModule;
#ifdef WIN32
  struct _finddata_t d;
  intptr_t hd;
  std::string pat = mDirname;
  pat += "*.*";
  hd = _findfirst(pat.c_str(), &d);
  if (hd != -1)
  {
    do
    {
      unlink(d.name);
    }
    while (_findnext(hd, &d) == 0);
    _findclose(hd);
  }
#else
  DIR* d = opendir(mDirname.c_str());
  struct dirent* de;
  while ((de = readdir(d)))
  {
    std::string n = mDirname;
    n += "/";
    n += de->d_name;
    unlink(n.c_str());
  }
  closedir(d);
#endif
  rmdir(mDirname.c_str());
}

CDA_CellMLIntegrationRun::CDA_CellMLIntegrationRun
(
)
  : 
  mIsStarted(false),
  mStepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5),
  mEpsAbs(1E-6), mEpsRel(1E-6), mScalVar(1.0), mScalRate(0.0),
  mStepSizeMax(1.0), mStartBvar(0.0), mStopBvar(10.0), mMaxPointDensity(10000.0),
  mTabulationStepSize(0.0), mObserver(NULL), mCancelIntegration(false),
  mPauseIntegration(false), mStrictTabulation(false)
{
#ifndef WIN32
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
#endif
}

CDA_CellMLIntegrationRun::~CDA_CellMLIntegrationRun()
{
  if (mIsStarted)
  {
    close(mThreadPipes[0]);
    close(mThreadPipes[1]);
  }
  if (mObserver != NULL)
    mObserver->release_ref();
}

iface::cellml_services::ODEIntegrationStepType
CDA_CellMLIntegrationRun::stepType
(
)
  throw (std::exception&)
{
  return mStepType;
}

void
CDA_CellMLIntegrationRun::stepType
(
 iface::cellml_services::ODEIntegrationStepType aStepType
)
  throw(std::exception&)
{
  mStepType = aStepType;
}

void
CDA_CellMLIntegrationRun::setStepSizeControl
(
 double epsAbs, double epsRel, double scalVar,
 double scalRate, double maxStep
)
  throw (std::exception&)
{
  mEpsAbs = epsAbs;
  mEpsRel = epsRel;
  mScalVar = scalVar;
  mScalRate = scalRate;
  mStepSizeMax = maxStep;
}

void
CDA_CellMLIntegrationRun::setTabulationStepControl
(
  double tabulationStepSize, bool strictTabulation
)
  throw (std::exception&)
{
  mTabulationStepSize = tabulationStepSize;
  mStrictTabulation = strictTabulation;
}

void
CDA_CellMLIntegrationRun::setResultRange
(
 double startBvar, double stopBvar, double maxPointDensity
)
  throw (std::exception&)
{
  mStartBvar = startBvar;
  mStopBvar = stopBvar;
  mMaxPointDensity = maxPointDensity;
}

void
CDA_CellMLIntegrationRun::setProgressObserver
(
 iface::cellml_services::IntegrationProgressObserver* aIpo
)
  throw (std::exception&)
{
  if (mObserver != NULL)
    mObserver->release_ref();
  mObserver = aIpo;
  if (mObserver != NULL)
    mObserver->add_ref();
}

void
CDA_CellMLIntegrationRun::setOverride
(
 iface::cellml_services::VariableEvaluationType aType,
 uint32_t variableIndex, double newValue
)
  throw (std::exception&)
{
  if (aType == iface::cellml_services::CONSTANT)
  {
    mConstantOverrides.push_back(std::pair<uint32_t,double>
                                 (variableIndex, newValue));
  }
  else if (aType == iface::cellml_services::STATE_VARIABLE)
  {
    mIVOverrides.push_back(std::pair<uint32_t,double>
                           (variableIndex, newValue));
  }
  else
    throw iface::cellml_api::CellMLException();
}

void
CDA_CellMLIntegrationRun::start()
  throw (std::exception&)
{
  if (mIsStarted)
    throw iface::cellml_api::CellMLException();
  mIsStarted = true;

  pipe(mThreadPipes);

  // The new thread accesses this, so must add_ref. Thread will release itself
  // before returning.
  add_ref();
  startthread();
}

void
CDA_CellMLIntegrationRun::stop()
  throw (std::exception&)
{
  if (!mIsStarted || mCancelIntegration)
    return;
  mCancelIntegration = true;

  int stop = 1;
  write(mThreadPipes[1], &stop, sizeof(stop));
}

void
CDA_CellMLIntegrationRun::pause()
  throw (std::exception&)
{
  if (!mIsStarted || mCancelIntegration || mPauseIntegration)
    return;
  mPauseIntegration = true;

  int pause = 2;
  write(mThreadPipes[1], &pause, sizeof(pause));
}

void
CDA_CellMLIntegrationRun::resume()
  throw (std::exception&)
{
  if (!mIsStarted || mCancelIntegration || !mPauseIntegration)
    return;

  mPauseIntegration = false;
  int resume = 3;
  write(mThreadPipes[1], &resume, sizeof(resume));
}

void
CDA_ODESolverRun::runthread()
{
  struct fail_info failInfo;
  double* constants = NULL, * buffer = NULL, * algebraic, * rates, * states;

  try
  {
    CompiledModelFunctions* f = mModel->mCMF;
    uint32_t algSize = mModel->mCCI->algebraicIndexCount();
    uint32_t constSize = mModel->mCCI->constantIndexCount();
    uint32_t rateSize = mModel->mCCI->rateIndexCount();

    constants = new double[constSize];
    buffer = new double[2 * rateSize + algSize + 1];

    buffer[0] = mStartBvar;
    states = buffer + 1;
    rates = states + rateSize;
    algebraic = rates + rateSize;

    memset(rates, 0, rateSize * sizeof(double));

    struct Override overrides;
    overrides.isOverriden = new bool[constSize];
    overrides.constants = constants;
    overrides.nConstants = constSize;
    for (int i = 0; i < constSize; i++)
      overrides.isOverriden[i] = false;
    OverrideList::iterator oli;
    for (oli = mConstantOverrides.begin(); oli != mConstantOverrides.end();
         oli++)
      if ((*oli).first < constSize)
      {
        overrides.isOverriden[(*oli).first] = true;
        constants[(*oli).first] = (*oli).second;
      }

    struct fail_info failInfo;
    f->SetupConstants(constants, rates, states, &overrides, &failInfo);
    if (failInfo.failtype)
      throw iface::cellml_api::CellMLException(); // Caught below.

    delete [] overrides.isOverriden;

    // Now apply overrides...
    for (oli = mIVOverrides.begin(); oli != mIVOverrides.end();
         oli++)
      if ((*oli).first < rateSize)
        states[(*oli).first] = (*oli).second;

    if (mObserver != NULL)
    {
      std::vector<double> constantsVec(constants, constants + constSize);
      mObserver->computedConstants(constantsVec);
    }

    f->ComputeRates(mStartBvar, constants, rates, states, algebraic, &failInfo);
    f->ComputeVariables(mStartBvar, constants, rates, states, algebraic, &failInfo);
    if (failInfo.failtype)
      throw iface::cellml_api::CellMLException(); // Caught below.

    if (mObserver != NULL)
    {
      std::vector<double> resultsVec(buffer, buffer + 2 * rateSize + algSize + 1);
      mObserver->results(resultsVec);
    }

    SolveODEProblem(f, constSize, constants, rateSize, rates, states,
                    algSize, algebraic);
  }
  catch (...)
  {
    try
    {
      if (mObserver != NULL)
        mObserver->failed(failInfo.failmsg.c_str());
    }
    catch (...)
    {
    }
  }

  if (constants != NULL)
    delete [] constants;
  if (buffer != NULL)
    delete [] buffer;

  release_ref(); // Thread is finishing, cancel the add_ref call before startthread.
}

void
CDA_DAESolverRun::runthread()
{
  std::string emsg = "Unknown error";
  double* constants = NULL, * buffer = NULL, * algebraic, * rates, * states, * condvars;

  try
  {
    IDACompiledModelFunctions* f = mModel->mCMF;
    uint32_t algSize = mModel->mCCI->algebraicIndexCount();
    uint32_t constSize = mModel->mCCI->constantIndexCount();
    uint32_t rateSize = mModel->mCCI->rateIndexCount();
    DECLARE_QUERY_INTERFACE_OBJREF(cci, mModel->mCCI, cellml_services::IDACodeInformation);
    uint32_t condVarSize = cci->conditionVariableCount();

    constants = new double[constSize];
    buffer = new double[2 * rateSize + algSize + 1 + condVarSize];
    
    buffer[0] = mStartBvar;
    states = buffer + 1;
    rates = states + rateSize;
    algebraic = rates + rateSize;
    condvars = algebraic + algSize;

    memset(rates, 0, rateSize * sizeof(double));
    memset(condvars, 0, condVarSize * sizeof(double));

    struct Override overrides;
    overrides.isOverriden = new bool[constSize];
    overrides.constants = constants;
    overrides.nConstants = constSize;
    for (int i = 0; i < constSize; i++)
      overrides.isOverriden[i] = false;
    OverrideList::iterator oli;
    for (oli = mConstantOverrides.begin(); oli != mConstantOverrides.end();
         oli++)
      if ((*oli).first < constSize)
      {
        overrides.isOverriden[(*oli).first] = true;
        constants[(*oli).first] = (*oli).second;
      }

    struct fail_info failInfo;
    // Algebraic is needed for locally bound variables (e.g. for definite integrals).
    f->SetupFixedConstants(constants, rates, states, algebraic, &overrides, &failInfo);

    // Now apply overrides...
    for (oli = mIVOverrides.begin(); oli != mIVOverrides.end();
         oli++)
      if ((*oli).first < rateSize)
        states[(*oli).first] = (*oli).second;

    delete [] overrides.isOverriden;

    if (mObserver != NULL)
    {
      std::vector<double> constVector(constants, constants + constSize);
      mObserver->computedConstants(constVector);
    }

    SolveDAEProblem(f, constSize, constants, rateSize, rates, rateSize, states,
                    algSize, algebraic, condVarSize, condvars);
  }
  catch (...)
  {
    try
    {
      if (mObserver != NULL)
        mObserver->failed(emsg.c_str());
    }
    catch (...)
    {
    }
  }

  if (constants != NULL)
    delete [] constants;
  if (buffer != NULL)
    delete [] buffer;

  release_ref(); // Thread is finishing, cancel the add_ref call before startthread.
}

void
CDA_CellMLIntegrationService::setupCodeEnvironment
(
 iface::cellml_services::CodeInformation* cci,
 std::string& dirname,
 std::string& sourcename,
 std::ofstream& ss
)
{
  iface::cellml_services::ModelConstraintLevel mcl = cci->constraintLevel();
  if (mcl != iface::cellml_services::CORRECTLY_CONSTRAINED)
  {
    if (mcl == iface::cellml_services::OVERCONSTRAINED)
      mLastError = L"Model is overconstrained. Run CellML2C for more "
        L"information.";
    else
      mLastError = L"Model is underconstrained. Run CellML2C for more "
        L"information.";
    throw iface::cellml_api::CellMLException();
  }

  // Create a temporary directory...
  const char* tmpenvs[] = {"TMPDIR", "TEMP", "TMP", NULL};
  const char** p = tmpenvs;
  char* fn = NULL;
  while (!fn && *p)
  {
    char* env = getenv(*p);
    if (env != NULL)
      fn = attempt_make_tempdir(env);
    p++;
  }
  if (fn == NULL)
  {
#ifdef WIN32
    const char* tmpdirs[] = {"c:\\temp", "c:\\tmp", "\\temp", "\\tmp", NULL};
#else
    const char* tmpdirs[] = {"/tmp", "/var/tmp", "/usr/tmp", NULL};
#endif
    p = tmpdirs;
    while (!fn && *p)
    {
      fn = attempt_make_tempdir(*p);
      p++;
    }
  }
  if (fn == NULL)
    throw iface::cellml_api::CellMLException();
  dirname = fn;
  free(fn);

  // We now have a temporary directory. Make the source file...
  sourcename = dirname + "/generated.c";
  ss.open(sourcename.c_str());

  ss << "/* This file is automatically generated and will be automatically"
     << std::endl
     << " * deleted. Don't edit it or changes will be lost. */" << std::endl
     << "#define NULL ((void*)0)" << std::endl;
  ss << 
#include "CISModelSupportString.h"
     << std::endl
    // C library...
     << "extern double fabs(double x);" << std::endl
     << "extern double acos(double x);" << std::endl
     << "extern double acosh(double x);" << std::endl
     << "extern double atan(double x);" << std::endl
     << "extern double atanh(double x);" << std::endl
     << "extern double asin(double x);" << std::endl
     << "extern double asinh(double x);" << std::endl
     << "extern double acos(double x);" << std::endl
     << "extern double acosh(double x);" << std::endl
     << "extern double asin(double x);" << std::endl
     << "extern double asinh(double x);" << std::endl
     << "extern double atan(double x);" << std::endl
     << "extern double atanh(double x);" << std::endl
     << "extern double ceil(double x);" << std::endl
     << "extern double cos(double x);" << std::endl
     << "extern double cosh(double x);" << std::endl
     << "extern double tan(double x);" << std::endl
     << "extern double tanh(double x);" << std::endl
     << "extern double sin(double x);" << std::endl
     << "extern double sinh(double x);" << std::endl
     << "extern double exp(double x);" << std::endl
     << "extern double floor(double x);" << std::endl
     << "extern double pow(double x, double y);" << std::endl
     << "extern double log(double x);" << std::endl
     << "struct rootfind_info" << std::endl
     << "{" << std::endl
     << "  double aVOI, * aCONSTANTS, * aRATES, * aSTATES, * aALGEBRAIC;" << std::endl
     << "  struct fail_info* aFail;" << std::endl
     << "};" << std::endl
     << "#define LM_DIF_WORKSZ(npar, nmeas) (4*(nmeas) + 4*(npar) + "
    "(nmeas)*(npar) + (npar)*(npar))" << std::endl;

  std::wstring frag = cci->functionsString();
  size_t fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << frag8 << std::endl;
  delete [] frag8;
}

already_AddRefd<iface::cellml_services::ODESolverCompiledModel>
CDA_CellMLIntegrationService::compileModelODE
(
 iface::cellml_api::Model* aModel
)
  throw(std::exception&)
{
  return compileModelODEInternal(aModel, false);
}

already_AddRefd<iface::cellml_services::ODESolverCompiledModel>
CDA_CellMLIntegrationService::compileDebugModelODE
(
 iface::cellml_api::Model* aModel
)
  throw(std::exception&)
{
  return compileModelODEInternal(aModel, true);
}

void
CDA_CellMLIntegrationService::SetupCodeGenStrings(iface::cellml_services::CodeGenerator* aCGS, bool aIsDebug)
{
  ObjRef<iface::cellml_services::MaLaESBootstrap> mb(CreateMaLaESBootstrap());

  if (aIsDebug)
  {
    aCGS->assignPattern(L"TryAssign(&(<LHS>), <RHS>, \"<XMLID>\", failInfo);\r\nif (getFailType(failInfo)) return FAIL_RETURN;\r\n");
    aCGS->assignConstantPattern(L"TryOverrideAssign(&(<LHS>), <RHS>, \"<XMLID>\", OVERRIDES, failInfo);\r\nif (getFailType(failInfo)) return FAIL_RETURN;\r\n");
    aCGS->sampleDensityFunctionPattern
    (
     L"SampleUsingPDF(&pdf_<ID>, <ROOTCOUNT>, pdf_roots_<ID>, CONSTANTS, ALGEBRAIC, failInfo)"
     L"<SUP>double pdf_<ID>(double bvar, double* CONSTANTS, double* ALGEBRAIC, struct fail_info* failInfo)\r\n"
     L"{\r\ndouble value;\r\n"
     L"TryAssign(&value, <EXPR>, \"probability density function\", failInfo);\r\n"
     L"if (getFailType(failInfo)) return FAIL_RETURN;\r\n"
     L"return value;\r\n}\r\n"
     L"double (*pdf_roots_<ID>[])(double bvar, double*, double*, struct fail_info* failInfo) = "
     L"{<FOREACH_ROOT>pdf_<ID>_root_<ROOTID>,<ROOTSUP>double pdf_<ID>_root_<ROOTID>"
     L"(double bvar, double* CONSTANTS, double* ALGEBRAIC)\r\n"
     L"{\r\ndouble value; TryAssign(&value, <EXPR>, \"roots of probability density function\", failInfo);\r\n"
     L"if (getFailType(failInfo)) return FAIL_RETURN;\r\n"
     L"return (<EXPR>);\r\n}\r\n</FOREACH_ROOT>};\r\n");
  aCGS->solvePattern
    (
     L"rootfind_<ID>(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, failInfo);\r\n"
     L"<SUP>"
     L"void objfunc_<ID>(double* p, double* hx, void *adata)\r\n"
     L"{\r\n"
     L"  /* Solver for equation: <XMLID> */\r\n"
     L"  struct rootfind_info* rfi = (struct rootfind_info*)adata;\r\n"
     L"#define VOI rfi->aVOI\r\n"
     L"#define CONSTANTS rfi->aCONSTANTS\r\n"
     L"#define RATES rfi->aRATES\r\n"
     L"#define STATES rfi->aSTATES\r\n"
     L"#define ALGEBRAIC rfi->aALGEBRAIC\r\n"
     L"  <VAR> = *p;\r\n"
     L"  TryAssign(hx, TryMinus(<LHS>, <RHS>), \"<XMLID>\", rfi->aFail);\r\n"
     L"  if (getFailType(rfi->aFail)) return;\r\n"
     L"#undef VOI\r\n"
     L"#undef CONSTANTS\r\n"
     L"#undef RATES\r\n"
     L"#undef STATES\r\n"
     L"#undef ALGEBRAIC\r\n"
     L"}\r\n"
     L"void rootfind_<ID>(double VOI, double* CONSTANTS, double* RATES, "
     L"double* STATES, double* ALGEBRAIC, struct fail_info* failInfo)\r\n"
     L"{\r\n"
     L"  static double val = <IV>;\r\n"
     L"  struct rootfind_info rfi;\r\n"
     L"  rfi.aVOI = VOI;\r\n"
     L"  rfi.aCONSTANTS = CONSTANTS;\r\n"
     L"  rfi.aRATES = RATES;\r\n"
     L"  rfi.aSTATES = STATES;\r\n"
     L"  rfi.aALGEBRAIC = ALGEBRAIC;\r\n"
     L"  rfi.aFail = failInfo;\r\n"
     L"  do_nonlinearsolve(objfunc_<ID>, &val, failInfo, 1, &rfi);\r\n"
     L"  <VAR> = val;\r\n"
     L"}\r\n"
     );
  aCGS->solveNLSystemPattern(
    L"rootfind_<ID>(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, failInfo);\r\n"
    L"<SUP>"
    L"void objfunc_<ID>(double* p, double* hx, void *adata)\r\n"
    L"{\r\n"
    L"  struct rootfind_info* rfi = (struct rootfind_info*)adata;\r\n"
    L"#define VOI rfi->aVOI\r\n"
    L"#define CONSTANTS rfi->aCONSTANTS\r\n"
    L"#define RATES rfi->aRATES\r\n"
    L"#define STATES rfi->aSTATES\r\n"
    L"#define ALGEBRAIC rfi->aALGEBRAIC\r\n"
    L"#define failInfo rfi->aFail\r\n"
    L"  <EQUATIONS><VAR> = p[<INDEX>];<JOIN>\r\n"
    L"  </EQUATIONS>\r\n"
    L"  <EQUATIONS>TryAssign(hx + <INDEX>, <EXPR>, \"<XMLID>\", rfi->aFail);<JOIN>\r\n"
    L"  </EQUATIONS>\r\n"
    L"#undef VOI\r\n"
    L"#undef CONSTANTS\r\n"
    L"#undef RATES\r\n"
    L"#undef STATES\r\n"
    L"#undef ALGEBRAIC\r\n"
    L"#undef failInfo\r\n"
    L"}\r\n"
    L"void rootfind_<ID>(double VOI, double* CONSTANTS, double* RATES, "
    L"double* STATES, double* ALGEBRAIC, struct fail_info* failInfo)\r\n"
    L"{\r\n"
    L"  /* Solver for equations: <EQUATIONS><XMLID><JOIN>, </EQUATIONS> */\r\n"
    L"  static double p[<COUNT>] = {<EQUATIONS><IV><JOIN>,</EQUATIONS>};\r\n"
    L"  struct rootfind_info rfi;\r\n"
    L"  rfi.aVOI = VOI;\r\n"
    L"  rfi.aCONSTANTS = CONSTANTS;\r\n"
    L"  rfi.aRATES = RATES;\r\n"
    L"  rfi.aSTATES = STATES;\r\n"
    L"  rfi.aALGEBRAIC = ALGEBRAIC;\r\n"
    L"  rfi.aFail = failInfo;\r\n"
    L"  do_nonlinearsolve(objfunc_<ID>, p, failInfo, <COUNT>, &rfi);\r\n"
    L"  <EQUATIONS><VAR> = p[<INDEX>];<JOIN>\r\n"
    L"  </EQUATIONS>\r\n"
    L"}\r\n"
    );
  aCGS->conditionalAssignmentPattern
    (
     L"if (UseEDouble(<CONDITION>, failInfo, \"top-level piecewise condition\") != 0)\r\n"
     L"{\r\n"
     L"  <STATEMENT>\r\n"
     L"}\r\n"
     L"<CASES>else if (UseEDouble(<CONDITION>, failInfo, \"top-level piecewise condition\") != 0.0)\r\n"
     L"{\r\n"
     L"  <STATEMENT>\r\n"
     L"}\r\n"
     L"</CASES>");
    ObjRef<iface::cellml_services::IDACodeGenerator> idaCG(QueryInterface(aCGS));
    if (idaCG)
    {
      idaCG->residualPattern(L"TryAssign(resid+<RNO>, TryMinus(<LHS>, <RHS>), \"<XMLID>\", failInfo);\r\nif (getFailType(failInfo)) return FAIL_RETURN;\r\n");
    }

  ObjRef<iface::cellml_services::MaLaESTransform> transform
    (
     mb->compileTransformer(
L"opengroup: (\r\n"
L"closegroup: )\r\n"
L"wrapvalue: CreateEDouble(#expr)\r\n"
L"abs: #prec[H]TryAbs(#expr1)\r\n"
L"and: #prec[H]TryAnd(#count, #exprs[,])\r\n"
L"arccos: #prec[H]TryACos(#expr1)\r\n"
L"arccosh: #prec[H]TryACosh(#expr1)\r\n"
L"arccot: #prec[H]TryATan(#expr1)\r\n"
L"arccoth: #prec[H]TryATanh(#expr1)\r\n"
L"arccsc: #prec[H]TryASin(#expr1)\r\n"
L"arccsch: #prec[H]TryASinh(#expr1)\r\n"
L"arcsec: #prec[H]TryACos(#expr1)\r\n"
L"arcsech: #prec[H]TryACosh(#expr1)\r\n"
L"arcsin: #prec[H]TryASin(#expr1)\r\n"
L"arcsinh: #prec[H]TryASinh(#expr1)\r\n"
L"arctan: #prec[H]TryATan(#expr1)\r\n"
L"arctanh: #prec[H]TryATanh(#expr1)\r\n"
L"ceiling: #prec[H]TryCeil(#expr1)\r\n"
L"cos: #prec[H]TryCos(#expr1)\r\n"
L"cosh: #prec[H]TryCosh(#expr1)\r\n"
L"cot: #prec[H]TryCot(#expr1)\r\n"
L"coth: #prec[H]TryCoth(#expr1)\r\n"
L"csc: #prec[H]TryCosec(#expr1)\r\n"
L"csch: #prec[H]TrySinh(#expr1)\r\n"
L"diff: #lookupDiffVariable\r\n"
L"divide: #prec[H]TryDivide(#expr1, #expr2)\r\n"
L"eq: #prec[H]TryEq(#count, #exprs[,])\r\n"
L"exp: #prec[H]TryExp(#expr1)\r\n"
L"factorial: #prec[H]TryFactorial(#expr1)\r\n"
L"factorof: #prec[H]TryFactorOf(#expr1, #expr2)\r\n"
L"floor: #prec[H]TryFloor(#expr1)\r\n"
L"gcd: #prec[H]TryGCD(#count, #exprs[, ])\r\n"
L"geq: #prec[H]TryGeq(#count, #exprs[, ])\r\n"
L"gt: #prec[H]TryGt(#count, #exprs[, ])\r\n"
L"implies: #prec[H]TryImplies(#expr1, #expr2)\r\n"
L"int: #prec[H]TryDefint(func#unique1, VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, &#bvarIndex, #lowlimit, #uplimit"
L", failInfo)#supplement EDouble func#unique1(double VOI, "
L"double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, struct fail_info* failInfo) { return #expr1; }\r\n"
L"lcm: #prec[H]TryLCM(#count, #exprs[, ])\r\n"
L"leq: #prec[H]TryLeq(#count, #exprs[, ])\r\n"
L"ln: #prec[H]TryLn(#expr1)\r\n"
L"log: #prec[H]TryLogBase(#expr1, #logbase)\r\n"
L"lt: #prec[H]TryLt(#count, #exprs[, ])\r\n"
L"max: #prec[H]TryMax(#count, #exprs[, ])\r\n"
L"min: #prec[H]TryMin(#count, #exprs[, ])\r\n"
L"minus: #prec[H]TryMinus(#expr1, #expr2)\r\n"
L"neq: #prec[H]TryNeq(#expr1, #expr2)\r\n"
L"not: #prec[H]TryNot(#expr1)\r\n"
L"or: #prec[H]TryOr(#count, #exprs[,])\r\n"
L"plus: #prec[H]TryPlus(#count, #exprs[,])\r\n"
L"power: #prec[H]TryPower(#expr1, #expr2)\r\n"
L"quotient: #prec[H]TryQuotient(#expr1, #expr2)\r\n"
L"rem: #prec[H]TryRem(#expr1, #expr2)\r\n"
L"root: #prec[H]TryRoot(#expr1, #degree)\r\n"
L"sec: #prec[H]TrySec(#expr1)\r\n"
L"sech: #prec[900(0)]TrySech(#expr1)\r\n"
L"sin: #prec[H]TrySin(#expr1)\r\n"
L"sinh: #prec[H]TrySinh(#expr1)\r\n"
L"tan: #prec[H]TryTan(#expr1)\r\n"
L"tanh: #prec[H]TryTanh(#expr1)\r\n"
L"times: #prec[H]TryTimes(#count, #exprs[,])\r\n"
L"unary_minus: #prec[H]TryUnaryMinus(#expr1)\r\n"
L"units_conversion: #prec[H]TryUnitsConversion(#expr1, #expr2, #expr3)\r\n"
L"units_conversion_factor: #prec[H]TryUnitsConversion(#expr1, #expr2, 0.0)\r\n"
L"units_conversion_offset: #prec[H]TryUnitsConversion(#expr1, 1.0, #expr2)\r\n"
L"xor: #prec[H]TryXor(#expr1, #expr2)\r\n"
L"piecewise_first_case: #prec[H]TryPiecewise(#expr1, #expr2, \r\n"
L"piecewise_extra_case: #prec[H]1, #expr1, #expr2, \r\n"
L"piecewise_otherwise: #prec[H]2, #expr1)\r\n"
L"piecewise_no_otherwise: #prec[H]0)\r\n"
L"eulergamma: #prec[H]CreateEDouble(0.577215664901533)\r\n"
L"exponentiale: #prec[H]CreateEDouble(2.71828182845905)\r\n"
L"false: #prec[H]CreateEDouble(0.0)\r\n"
L"infinity: #prec[H]TryInfinity()\r\n"
L"notanumber: #prec[H]TryNaN()\r\n"
L"pi: #prec[H]CreateEDouble(3.14159265358979)\r\n"
L"true: #prec[H]CreateEDouble(1.0)\r\n"
                            )
    );
    aCGS->transform(transform);
  }
  else
  {
    aCGS->sampleDensityFunctionPattern
    (
     L"SampleUsingPDF(&pdf_<ID>, <ROOTCOUNT>, pdf_roots_<ID>, CONSTANTS, ALGEBRAIC, failInfo)"
     L"<SUP>double pdf_<ID>(double bvar, double* CONSTANTS, double* ALGEBRAIC, struct fail_info* failInfo)\r\n"
     L"{\r\nreturn (<EXPR>);\r\n}\r\n"
     L"double (*pdf_roots_<ID>[])(double bvar, double*, double*, struct fail_info* failInfo) = "
     L"{<FOREACH_ROOT>pdf_<ID>_root_<ROOTID>,<ROOTSUP>double pdf_<ID>_root_<ROOTID>"
     L"(double bvar, double* CONSTANTS, double* ALGEBRAIC)\r\n"
     L"{\r\nreturn (<EXPR>);\r\n}\r\n</FOREACH_ROOT>};\r\n");
    aCGS->solvePattern
    (
     L"rootfind_<ID>(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, failInfo);\r\n"
     L"<SUP>"
     L"void objfunc_<ID>(double* p, double* hx, void *adata)\r\n"
     L"{\r\n"
     L"  /* Solver for equation: <XMLID> */\r\n"
     L"  struct rootfind_info* rfi = (struct rootfind_info*)adata;\r\n"
     L"#define VOI rfi->aVOI\r\n"
     L"#define CONSTANTS rfi->aCONSTANTS\r\n"
     L"#define RATES rfi->aRATES\r\n"
     L"#define STATES rfi->aSTATES\r\n"
     L"#define ALGEBRAIC rfi->aALGEBRAIC\r\n"
     L"  <VAR> = *p;\r\n"
     L"  *hx = (<LHS>) - (<RHS>);\r\n"
     L"#undef VOI\r\n"
     L"#undef CONSTANTS\r\n"
     L"#undef RATES\r\n"
     L"#undef STATES\r\n"
     L"#undef ALGEBRAIC\r\n"
     L"}\r\n"
     L"void rootfind_<ID>(double VOI, double* CONSTANTS, double* RATES, "
     L"double* STATES, double* ALGEBRAIC, struct fail_info* failInfo)\r\n"
     L"{\r\n"
     L"  static double val = <IV>;\r\n"
     L"  struct rootfind_info rfi;\r\n"
     L"  rfi.aVOI = VOI;\r\n"
     L"  rfi.aCONSTANTS = CONSTANTS;\r\n"
     L"  rfi.aRATES = RATES;\r\n"
     L"  rfi.aSTATES = STATES;\r\n"
     L"  rfi.aALGEBRAIC = ALGEBRAIC;\r\n"
     L"  rfi.aFail = failInfo;\r\n"
     L"  do_nonlinearsolve(objfunc_<ID>, &val, failInfo, 1, &rfi);\r\n"
     L"  <VAR> = val;\r\n"
     L"}\r\n"
     );
    aCGS->solveNLSystemPattern
      (
       L"rootfind_<ID>(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, failInfo);\r\n"
       L"<SUP>"
       L"void objfunc_<ID>(double* p, double* hx, void *adata)\r\n"
       L"{\r\n"
       L"  struct rootfind_info* rfi = (struct rootfind_info*)adata;\r\n"
       L"#define VOI rfi->aVOI\r\n"
       L"#define CONSTANTS rfi->aCONSTANTS\r\n"
       L"#define RATES rfi->aRATES\r\n"
       L"#define STATES rfi->aSTATES\r\n"
       L"#define ALGEBRAIC rfi->aALGEBRAIC\r\n"
       L"#define failInfo rfi->aFail\r\n"
       L"  <EQUATIONS><VAR> = p[<INDEX>];<JOIN>\r\n"
       L"  </EQUATIONS>\r\n"
       L"  <EQUATIONS>hx[<INDEX>] = <EXPR>;<JOIN>\r\n"
       L"  </EQUATIONS>\r\n"
       L"#undef VOI\r\n"
       L"#undef CONSTANTS\r\n"
       L"#undef RATES\r\n"
       L"#undef STATES\r\n"
       L"#undef ALGEBRAIC\r\n"
       L"#undef failInfo\r\n"
       L"}\r\n"
       L"void rootfind_<ID>(double VOI, double* CONSTANTS, double* RATES, "
       L"double* STATES, double* ALGEBRAIC, struct fail_info* failInfo)\r\n"
       L"{\r\n"
       L"  /* Solver for equations: <EQUATIONS><XMLID><JOIN>, </EQUATIONS> */\r\n"
       L"  static double p[<COUNT>] = {<EQUATIONS><IV><JOIN>,</EQUATIONS>};\r\n"
       L"  struct rootfind_info rfi;\r\n"
       L"  rfi.aVOI = VOI;\r\n"
       L"  rfi.aCONSTANTS = CONSTANTS;\r\n"
       L"  rfi.aRATES = RATES;\r\n"
       L"  rfi.aSTATES = STATES;\r\n"
       L"  rfi.aALGEBRAIC = ALGEBRAIC;\r\n"
       L"  rfi.aFail = failInfo;\r\n"
       L"  do_nonlinearsolve(objfunc_<ID>, p, failInfo, <COUNT>, &rfi);\r\n"
       L"  <EQUATIONS><VAR> = p[<INDEX>];<JOIN>\r\n"
       L"  </EQUATIONS>\r\n"
       L"}\r\n"
       );
    aCGS->assignConstantPattern(L"OverrideAssign(&(<LHS>), <RHS>, OVERRIDES);\r\n");
    aCGS->assignPattern(L"<LHS>= <RHS>;\r\n");
    ObjRef<iface::cellml_services::MaLaESTransform> transform
      (
       mb->compileTransformer(
L"opengroup: (\r\n"
L"closegroup: )\r\n"
L"abs: #prec[H]fabs(#expr1)\r\n"
L"and: #prec[20]#exprs[&&]\r\n"
L"arccos: #prec[H]acos(#expr1)\r\n"
L"arccosh: #prec[H]acosh(#expr1)\r\n"
L"arccot: #prec[1000(900)]atan(1.0/#expr1)\r\n"
L"arccoth: #prec[1000(900)]atanh(1.0/#expr1)\r\n"
L"arccsc: #prec[1000(900)]asin(1/#expr1)\r\n"
L"arccsch: #prec[1000(900)]asinh(1/#expr1)\r\n"
L"arcsec: #prec[1000(900)]acos(1/#expr1)\r\n"
L"arcsech: #prec[1000(900)]acosh(1/#expr1)\r\n"
L"arcsin: #prec[H]asin(#expr1)\r\n"
L"arcsinh: #prec[H]asinh(#expr1)\r\n"
L"arctan: #prec[H]atan(#expr1)\r\n"
L"arctanh: #prec[H]atanh(#expr1)\r\n"
L"ceiling: #prec[H]ceil(#expr1)\r\n"
L"cos: #prec[H]cos(#expr1)\r\n"
L"cosh: #prec[H]cosh(#expr1)\r\n"
L"cot: #prec[900(0)]1.0/tan(#expr1)\r\n"
L"coth: #prec[900(0)]1.0/tanh(#expr1)\r\n"
L"csc: #prec[900(0)]1.0/sin(#expr1)\r\n"
L"csch: #prec[900(0)]1.0/sinh(#expr1)\r\n"
L"diff: #lookupDiffVariable\r\n"
L"divide: #prec[900]#expr1/#expr2\r\n"
L"eq: #prec[30]#exprs[==]\r\n"
L"exp: #prec[H]exp(#expr1)\r\n"
L"factorial: #prec[H]factorial(#expr1)\r\n"
L"factorof: #prec[30(900)]#expr1 % #expr2 == 0\r\n"
L"floor: #prec[H]floor(#expr1)\r\n"
L"gcd: #prec[H]gcd_multi(#count, #exprs[, ])\r\n"
L"geq: #prec[30]#exprs[>=]\r\n"
L"gt: #prec[30]#exprs[>]\r\n"
L"implies: #prec[10(950)] !#expr1 || #expr2\r\n"
L"int: #prec[H]defint(func#unique1, VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, &#bvarIndex, #lowlimit, #uplimit, "
L"failInfo)#supplement double func#unique1(double VOI, "
L"double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, struct fail_info* failInfo) { return #expr1; }\r\n"
L"lcm: #prec[H]lcm_multi(#count, #exprs[, ])\r\n"
L"leq: #prec[30]#exprs[<=]\r\n"
L"ln: #prec[H]log(#expr1)\r\n"
L"log: #prec[H]arbitrary_log(#expr1, #logbase)\r\n"
L"lt: #prec[30]#exprs[<]\r\n"
L"max: #prec[H]multi_max(#count, #exprs[, ])\r\n"
L"min: #prec[H]multi_min(#count, #exprs[, ])\r\n"
L"minus: #prec[500]#expr1 - #expr2\r\n"
L"neq: #prec[30]#expr1 != #expr2\r\n"
L"not: #prec[950]!#expr1\r\n"
L"or: #prec[10]#exprs[||]\r\n"
L"plus: #prec[500]#exprs[+]\r\n"
L"power: #prec[H]pow(#expr1, #expr2)\r\n"
L"quotient: #prec[1000(0)] (double)(((int)#expr2) == 0 ? #expr1 / 0.0 : (int)(#expr1) / (int)(#expr2))\r\n"
L"rem: #prec[1000(0)] (double)(((int)#expr2) == 0 ? (#expr1) / 0.0 : (int)(#expr1) % (int)(#expr2))\r\n"
L"root: #prec[1000(900)] pow(#expr1, 1.0 / #degree)\r\n"
L"sec: #prec[900(0)]1.0 / cos(#expr1)\r\n"
L"sech: #prec[900(0)]1.0 / cosh(#expr1)\r\n"
L"sin: #prec[H] sin(#expr1)\r\n"
L"sinh: #prec[H] sinh(#expr1)\r\n"
L"tan: #prec[H] tan(#expr1)\r\n"
L"tanh: #prec[H] tanh(#expr1)\r\n"
L"times: #prec[900] #exprs[*]\r\n"
L"unary_minus: #prec[950]- #expr1\r\n"
L"units_conversion: #prec[500(900)]#expr1*#expr2 + #expr3\r\n"
L"units_conversion_factor: #prec[900]#expr1*#expr2\r\n"
L"units_conversion_offset: #prec[500]#expr1+#expr2\r\n"
L"xor: #prec[25(30)] (#expr1 != 0) ^ (#expr2 != 0)\r\n"
L"piecewise_first_case: #prec[1000(5)](#expr1 ? #expr2 : \r\n"
L"piecewise_extra_case: #prec[1000(5)]#expr1 ? #expr2 : \r\n"
L"piecewise_otherwise: #prec[1000(5)]#expr1)\r\n"
L"piecewise_no_otherwise: #prec[1000(5)]0.0/0.0)\r\n"
L"eulergamma: #prec[999]0.577215664901533\r\n"
L"exponentiale: #prec[999]2.71828182845905\r\n"
L"false: #prec[999]0.0\r\n"
L"infinity: #prec[900]1.0/0.0\r\n"
L"notanumber: #prec[999]0.0/0.0\r\n"
L"pi: #prec[999] 3.14159265358979\r\n"
L"true: #prec[999]1.0\r\n"
                            )
    );
    aCGS->transform(transform);
  }
}

already_AddRefd<iface::cellml_services::ODESolverCompiledModel>
CDA_CellMLIntegrationService::compileModelODEInternal
(
 iface::cellml_api::Model* aModel,
 bool aIsDebug
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(cgb, iface::cellml_services::CodeGeneratorBootstrap,
                     CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator,
                     cgb->createCodeGenerator());

  SetupCodeGenStrings(cg, aIsDebug);

  // Generate code information...
  ObjRef<iface::cellml_services::CodeInformation> cci;
  try
  {
    cci = already_AddRefd<iface::cellml_services::CodeInformation>
      (cg->generateCode(aModel));

    std::wstring msg = cci->errorMessage();
    if (msg != L"")
    {
      mLastError = msg;
      throw iface::cellml_api::CellMLException();
    }
  }
  catch (...)
  {
    mLastError = L"Unexpected exception generating code";
    throw iface::cellml_api::CellMLException();
  }

  std::ofstream ss;

  std::string dirname, sourcename;
  setupCodeEnvironment(cci, dirname, sourcename, ss);

  ss << "void SetupConstants(double* CONSTANTS, double* RATES, "
    "double *STATES, struct Override* OVERRIDES, struct fail_info* failInfo)" << std::endl;
  std::wstring frag = cci->initConstsString();
  size_t fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  double ALGEBRAIC[" << cci->algebraicIndexCount() << "];" << std::endl
     << "#define VOI 0.0" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "#undef VOI" << std::endl
     << "#undef ALGEBRAIC" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void ComputeRates(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC, struct fail_info* failInfo)" << std::endl;
  frag = cci->ratesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void ComputeVariables(double VOI, double* CONSTANTS, double* RATES, "
    "double* STATES, double* ALGEBRAIC, struct fail_info* failInfo)" << std::endl;
  frag = cci->variablesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss.close();

  CompiledModule* mod = CompileSource(dirname, sourcename, mLastError);
  CompiledModelFunctions* cmf = SetupCompiledModelFunctions(mod);
  
  return new CDA_ODESolverModel(mod, cmf, aModel, cci, dirname);
}

already_AddRefd<iface::cellml_services::DAESolverCompiledModel>
CDA_CellMLIntegrationService::compileModelDAE
(
 iface::cellml_api::Model* aModel
)
  throw(std::exception&)
{
  return compileModelDAEInternal(aModel, false);
}

already_AddRefd<iface::cellml_services::DAESolverCompiledModel>
CDA_CellMLIntegrationService::compileDebugModelDAE
(
 iface::cellml_api::Model* aModel
)
  throw(std::exception&)
{
  return compileModelDAEInternal(aModel, true);
}

already_AddRefd<iface::cellml_services::DAESolverCompiledModel>
CDA_CellMLIntegrationService::compileModelDAEInternal
(
 iface::cellml_api::Model* aModel,
 bool aIsDebug
)
  throw(std::exception&)
{
  RETURN_INTO_OBJREF(cgb, iface::cellml_services::CodeGeneratorBootstrap,
                     CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::IDACodeGenerator,
                     cgb->createIDACodeGenerator());

  SetupCodeGenStrings(cg, aIsDebug);

  // Generate code information...
  ObjRef<iface::cellml_services::IDACodeInformation> cci;
  try
  {
    cci = already_AddRefd<iface::cellml_services::IDACodeInformation>
      (cg->generateIDACode(aModel));
    std::wstring msg = cci->errorMessage();
    if (msg != L"")
    {
      mLastError = msg;
      throw iface::cellml_api::CellMLException();
    }
  }
  catch (...)
  {
    mLastError = L"Unexpected exception generating code";
    throw iface::cellml_api::CellMLException();
  }

  std::ofstream ss;

  std::string dirname, sourcename;
  setupCodeEnvironment(cci, dirname, sourcename, ss);

  ss << "void SetupFixedConstants(double* CONSTANTS, double* RATES, "
    "double *STATES, double *ALGEBRAIC, struct Override* OVERRIDES, "
    "struct fail_info* failInfo)" << std::endl;
  std::wstring frag = cci->initConstsString();
  size_t fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define VOI 0.0" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "#undef VOI" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC, double* CONDVAR, struct fail_info* failInfo)" << std::endl;
  frag = cci->variablesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void EvaluateEssentialVariables(double VOI, double* CONSTANTS, double* RATES, "
     << "double* OLDRATES, double* STATES, double* OLDSTATES, double* ALGEBRAIC, "
     << "double* CONDVAR, struct fail_info* failInfo)" << std::endl;
  frag = cci->essentialVariablesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void ComputeResiduals(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES, "
    "double* STATES, double* OLDSTATES, double* ALGEBRAIC, double* CONDVAR, double* resid,"
    "struct fail_info* failInfo)" << std::endl;
  frag = cci->ratesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void ComputeRootInformation(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES, "
    "double* STATES, double* OLDSTATES, double* ALGEBRAIC, double* CONDVAR, "
    "struct fail_info* failInfo)" << std::endl;
  frag = cci->rootInformationString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "#define FAIL_RETURN" << std::endl
     << frag8 << std::endl
     << "#undef FAIL_RETURN" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "void SetupStateInfo(double * SI)" << std::endl;
  frag = cci->stateInformationString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << frag8 << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss.close();

  CompiledModule* mod = CompileSource(dirname, sourcename, mLastError);
  IDACompiledModelFunctions* cmf = SetupIDACompiledModelFunctions(mod);
  
  return new CDA_DAESolverModel(mod, cmf, aModel, cci, dirname);
}

already_AddRefd<iface::cellml_services::ODESolverRun>
CDA_CellMLIntegrationService::createODEIntegrationRun
(
 iface::cellml_services::ODESolverCompiledModel* aModel
)
  throw (std::exception&)
{
  return new CDA_ODESolverRun(unsafe_dynamic_cast<CDA_ODESolverModel*>(aModel));
}

already_AddRefd<iface::cellml_services::DAESolverRun>
CDA_CellMLIntegrationService::createDAEIntegrationRun
(
 iface::cellml_services::DAESolverCompiledModel* aModel
)
  throw (std::exception&)
{
  return new CDA_DAESolverRun(unsafe_dynamic_cast<CDA_DAESolverModel*>(aModel));
}

already_AddRefd<iface::cellml_services::CellMLIntegrationService>
CreateIntegrationService()
{
  return new CDA_CellMLIntegrationService();
}
