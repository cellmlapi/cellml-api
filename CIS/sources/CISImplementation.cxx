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
  cmf->SetupConstants = (int (*)(double*, double*, double*))
    module->getSymbol("SetupConstants");
  cmf->ComputeRates = (int (*)(double,double*,double*,double*,double*))
    module->getSymbol("ComputeRates");
  cmf->ComputeVariables = (int (*)(double,double*,double*,double*,double*))
    module->getSymbol("ComputeVariables");
  return cmf;
}

IDACompiledModelFunctions*
SetupIDACompiledModelFunctions(CompiledModule* module)
{
  IDACompiledModelFunctions* cmf = new IDACompiledModelFunctions;
  cmf->SetupFixedConstants = (int (*)(double*, double*, double*))
    module->getSymbol("SetupFixedConstants");
  cmf->EvaluateVariables = (int (*)(double, double*, double*, double*, double*, double*))
    module->getSymbol("EvaluateVariables");
  cmf->EvaluateEssentialVariables = (int (*)(double, double*, double*, double*, double*, double*, double*, double*))
    module->getSymbol("EvaluateEssentialVariables");
  cmf->ComputeResiduals = (int (*)(double, double*, double*, double*, double*, double*, double*, double*, double*))
    module->getSymbol("ComputeResiduals");
  cmf->ComputeRootInformation = (int (*)(double, double*, double*, double*, double*, double*, double*, double*))
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
  : mStepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5),
    mEpsAbs(1E-6), mEpsRel(1E-6), mScalVar(1.0), mScalRate(0.0),
    mStepSizeMax(1.0), mStartBvar(0.0), mStopBvar(10.0), mMaxPointDensity(10000.0),
    mTabulationStepSize(0.0), mObserver(NULL), mCancelIntegration(false), mStrictTabulation(false)
{
#ifndef WIN32
  struct timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
#endif
}

CDA_CellMLIntegrationRun::~CDA_CellMLIntegrationRun()
{
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
  // The new thread accesses this, so must add_ref. Thread will release itself
  // before returning.
  add_ref();
  startthread();
}

void
CDA_CellMLIntegrationRun::stop()
  throw (std::exception&)
{
  mCancelIntegration = true;
}

void
CDA_ODESolverRun::runthread()
{
  std::string emsg = "Unknown error";
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

    f->SetupConstants(constants, rates, states);

    // Now apply overrides...
    OverrideList::iterator oli;
    for (oli = mConstantOverrides.begin(); oli != mConstantOverrides.end();
         oli++)
      if ((*oli).first < constSize)
        constants[(*oli).first] = (*oli).second;
    for (oli = mIVOverrides.begin(); oli != mIVOverrides.end();
         oli++)
      if ((*oli).first < rateSize)
        states[(*oli).first] = (*oli).second;

    if (mObserver != NULL)
    {
      std::vector<double> constantsVec(constants, constants + constSize);
      mObserver->computedConstants(constantsVec);
    }

    f->ComputeRates(mStartBvar, constants, rates, states, algebraic);
    f->ComputeVariables(mStartBvar, constants, rates, states, algebraic);

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

    f->SetupFixedConstants(constants, rates, states);

    // Now apply overrides...
    OverrideList::iterator oli;
    for (oli = mConstantOverrides.begin(); oli != mConstantOverrides.end();
         oli++)
      if ((*oli).first < constSize)
        constants[(*oli).first] = (*oli).second;
    for (oli = mIVOverrides.begin(); oli != mIVOverrides.end();
         oli++)
      if ((*oli).first < rateSize)
        states[(*oli).first] = (*oli).second;

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
  ss << "extern double fabs(double x);" << std::endl
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
     << "extern double factorial(double x);" << std::endl
     << "extern double log(double x);" << std::endl
     << "extern double arbitrary_log(double x, double base);" << std::endl
     << "extern double gcd_pair(double a, double b);" << std::endl
     << "extern double lcm_pair(double a, double b);" << std::endl
     << "extern double gcd_multi(unsigned int size, ...);" << std::endl
     << "extern double lcm_multi(unsigned int size, ...);" << std::endl
     << "extern double multi_min(unsigned int size, ...);" << std::endl
     << "extern double multi_max(unsigned int size, ...);" << std::endl
     << "static double fixnans(double x) { return finite(x) ? x : 1E100; }" << std::endl
     << "struct rootfind_info" << std::endl
     << "{" << std::endl
     << "  double aVOI, * aCONSTANTS, * aRATES, * aSTATES, * aALGEBRAIC;" << std::endl
     << "  int* aPRET;" << std::endl
     << "};" << std::endl
     << "extern double defint(double (*f)(double VOI,double *C,double *R,double *S,"
     << "double *A, int* pret), double VOI,double *C,double *R,double *S,double *A,double *V,"
     << "double lowV, double highV, int* pret);" << std::endl
     << "extern double SampleUsingPDF(double (*pdf)(double bvar,"
     << "double* CONSTANTS, double* ALGEBRAIC), int, double (**pdf_roots)(double bvar, double*, double*),"
        "double* CONSTANTS, double* ALGEBRAIC);" << std::endl
     << "#define LM_DIF_WORKSZ(npar, nmeas) (4*(nmeas) + 4*(npar) + "
    "(nmeas)*(npar) + (npar)*(npar))" << std::endl
     << "extern void do_nonlinearsolve(void (*)(double *, double *, void*), "
    "double*, int*, unsigned long, void*);" << std::endl;

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
  RETURN_INTO_OBJREF(cgb, iface::cellml_services::CodeGeneratorBootstrap,
                     CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator,
                     cgb->createCodeGenerator());

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

  ss << "int SetupConstants(double* CONSTANTS, double* RATES, "
    "double *STATES)" << std::endl;
  std::wstring frag = cci->initConstsString();
  size_t fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << "  double ALGEBRAIC[" << cci->algebraicIndexCount() << "];" << std::endl
     << "#define VOI 0.0" << std::endl
     << frag8 << std::endl
     << "#undef VOI" << std::endl
     << "#undef ALGEBRAIC" << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "int ComputeRates(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC)" << std::endl;
  frag = cci->ratesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "int ComputeVariables(double VOI, double* CONSTANTS, double* RATES, "
    "double* STATES, double* ALGEBRAIC)" << std::endl;
  frag = cci->variablesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
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
  RETURN_INTO_OBJREF(cgb, iface::cellml_services::CodeGeneratorBootstrap,
                     CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg, iface::cellml_services::IDACodeGenerator,
                     cgb->createIDACodeGenerator());

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

  ss << "int SetupFixedConstants(double* CONSTANTS, double* RATES, "
    "double *STATES)" << std::endl;
  std::wstring frag = cci->initConstsString();
  size_t fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << "#define VOI 0.0" << std::endl
     << "#define ALGEBRAIC NULL" << std::endl
     << frag8 << std::endl
     << "#undef VOI" << std::endl
     << "#undef ALGEBRAIC" << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "int EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC, double* CONDVAR)" << std::endl;
  frag = cci->variablesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "int EvaluateEssentialVariables(double VOI, double* CONSTANTS, double* RATES, "
     << "double* OLDRATES, double* STATES, double* OLDSTATES, double* ALGEBRAIC, double* CONDVAR)" << std::endl;
  frag = cci->essentialVariablesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "int ComputeResiduals(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES, "
    "double* STATES, double* OLDSTATES, double* ALGEBRAIC, double* CONDVAR, double* resid)" << std::endl;
  frag = cci->ratesString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  delete [] frag8;

  ss << "int ComputeRootInformation(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES, "
    "double* STATES, double* OLDSTATES, double* ALGEBRAIC, double* CONDVAR)" << std::endl;
  frag = cci->rootInformationString();
  fragLen = wcstombs(NULL, frag.c_str(), 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag.c_str(), fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
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
