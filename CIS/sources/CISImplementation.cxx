#define IN_CIS_MODULE
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
SetupCompiledModelFunctions(void* module)
{
  CompiledModelFunctions* cmf = new CompiledModelFunctions;
#ifdef WIN32
#define getsym(m,s) GetProcAddress((HMODULE)m, s)
#else
#define getsym(m,s) dlsym(m,s)
#endif
  cmf->SetupConstants = (int (*)(double*, double*, double*))
    getsym(module, "SetupConstants");
  cmf->ComputeRates = (int (*)(double,double*,double*,double*,double*))
    getsym(module, "ComputeRates");
  cmf->ComputeVariables = (int (*)(double,double*,double*,double*,double*))
    getsym(module, "ComputeVariables");
  return cmf;
}

IDACompiledModelFunctions*
SetupIDACompiledModelFunctions(void* module)
{
  IDACompiledModelFunctions* cmf = new IDACompiledModelFunctions;
  cmf->SetupFixedConstants = (int (*)(double*, double*, double*))
    getsym(module, "SetupFixedConstants");
  cmf->EvaluateVariables = (int (*)(double, double*, double*, double*, double*))
    getsym(module, "EvaluateVariables");
  cmf->EvaluateEssentialVariables = (int (*)(double, double*, double*, double*, double*))
    getsym(module, "EvaluateEssentialVariables");
  cmf->ComputeResiduals = (int (*)(double, double*, double*, double*, double*, double*))
    getsym(module, "ComputeResiduals");
  cmf->SetupStateInfo = (void (*)(double*))
    getsym(module, "SetupStateInfo");
  return cmf;
}

void*
CompileSource(std::string& destDir, std::string& sourceFile,
              std::wstring& lastError)
{
  setvbuf(stdout, NULL, _IONBF, 0);
  std::string targ = destDir;
#ifdef WIN32
  targ += "/generated.dll";
#else
  targ += "/generated.so";
#endif
  std::string cmd = "gcc -ggdb "
#ifdef WIN32
    "-mno-cygwin -mthreads -Llib -lcis-0 "
#else
    "-nodefaultlibs "
#ifdef __MACH__
    "-arch i386 -L. -L./.libs -L./../Resources/lib -L./../Resources/components -Llib -lcellml.0 -lannotools.0 -lcuses.0 -lmalaes.0 -lcevas.0 -lcis -lccgs.0 "
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
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    char *commandstring = (char*) malloc(strlen(cmd.c_str()));
    strcpy(commandstring, cmd.c_str());

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
    swprintf(buf, l, L"%s", msg);
    lastError += L" (";
    lastError += buf;
    lastError += L").";
#else
    lastError += L".";
#endif
    throw iface::cellml_api::CellMLException();
  }

  return t;
}

CDA_CellMLCompiledModel::CDA_CellMLCompiledModel
(
 void* aModule,
 iface::cellml_api::Model* aModel,
 iface::cellml_services::CodeInformation* aCCI,
 std::string& aDirname
)
  : _cda_refcount(1), mModule(aModule), mModel(aModel), mCCI(aCCI),
    mDirname(aDirname)
{
  mModel->add_ref();
  mCCI->add_ref();
}

CDA_CellMLCompiledModel::~CDA_CellMLCompiledModel()
{
#ifdef WIN32
  FreeLibrary((HMODULE)mModule);
#else
  dlclose(mModule);
#endif
  mModel->release_ref();
  mCCI->release_ref();
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
  : _cda_refcount(1),
    mStepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5),
    mEpsAbs(1E-6), mEpsRel(1E-6), mScalVar(1.0), mScalRate(0.0),
    mStepSizeMax(1.0), mStartBvar(0.0), mStopBvar(10.0), mMaxPointDensity(10000.0),
    mTabulationStepSize(0.0), mObserver(NULL), mCancelIntegration(false), mStrictTabulation(false)
{
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
      mObserver->computedConstants(constSize, constants);

    f->ComputeRates(mStartBvar, constants, rates, states, algebraic);
    f->ComputeVariables(mStartBvar, constants, rates, states, algebraic);

    if (mObserver != NULL)
      mObserver->results(2 * rateSize + algSize + 1, buffer);

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

  release_ref();
}

void
CDA_DAESolverRun::runthread()
{
  std::string emsg = "Unknown error";
  double* constants = NULL, * buffer = NULL, * algebraic, * rates, * states;

  try
  {
    IDACompiledModelFunctions* f = mModel->mCMF;
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
      mObserver->computedConstants(constSize, constants);

    SolveDAEProblem(f, constSize, constants, rateSize, rates, rateSize, states,
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

  release_ref();
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
     << "#define LM_DIF_WORKSZ(npar, nmeas) (4*(nmeas) + 4*(npar) + "
    "(nmeas)*(npar) + (npar)*(npar))" << std::endl
     << "extern void do_levmar(void (*)(double *, double *, int, int, void*), "
    "double*, double*, double*, int*, unsigned long, void*);" << std::endl;

  wchar_t* frag = cci->functionsString();
  size_t fragLen = wcstombs(NULL, frag, 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << frag8 << std::endl;
  free(frag);
  delete [] frag8;
}

iface::cellml_services::ODESolverCompiledModel*
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
    wchar_t* msg = cci->errorMessage();
    if (!wcscmp(msg, L""))
      free(msg);
    else
    {
      mLastError = msg;
      free(msg);
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
  wchar_t* frag = cci->initConstsString();
  size_t fragLen = wcstombs(NULL, frag, 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << "#define VOI 0.0" << std::endl
     << "#define ALGEBRAIC NULL" << std::endl
     << frag8 << std::endl
     << "#undef VOI" << std::endl
     << "#undef ALGEBRAIC" << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;

  ss << "int ComputeRates(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC)" << std::endl;
  frag = cci->ratesString();
  fragLen = wcstombs(NULL, frag, 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;

  ss << "int ComputeVariables(double VOI, double* CONSTANTS, double* RATES, "
    "double* STATES, double* ALGEBRAIC)" << std::endl;
  frag = cci->variablesString();
  fragLen = wcstombs(NULL, frag, 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;
  ss.close();

  void* mod = CompileSource(dirname, sourcename, mLastError);
  CompiledModelFunctions* cmf = SetupCompiledModelFunctions(mod);
  
  return new CDA_ODESolverModel(mod, cmf, aModel, cci, dirname);
}

iface::cellml_services::DAESolverCompiledModel*
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
    wchar_t* msg = cci->errorMessage();
    if (!wcscmp(msg, L""))
      free(msg);
    else
    {
      mLastError = msg;
      free(msg);
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
  wchar_t* frag = cci->initConstsString();
  size_t fragLen = wcstombs(NULL, frag, 0) + 1;
  char* frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << "#define VOI 0.0" << std::endl
     << "#define ALGEBRAIC NULL" << std::endl
     << frag8 << std::endl
     << "#undef VOI" << std::endl
     << "#undef ALGEBRAIC" << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;

  ss << "int EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC)" << std::endl;
  frag = cci->variablesString();
  fragLen = wcstombs(NULL, frag, 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;

  ss << "int EvaluateEssentialVariables(double VOI, double* CONSTANTS, double* RATES, "
     << "double* STATES, double* ALGEBRAIC)" << std::endl;
  frag = cci->essentialVariablesString();
  fragLen = wcstombs(NULL, frag, 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;

  ss << "int ComputeResiduals(double VOI, double* CONSTANTS, double* RATES, "
    "double* STATES, double* ALGEBRAIC, double* resid)" << std::endl;
  frag = cci->ratesString();
  fragLen = wcstombs(NULL, frag, 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << "  int ret = 0, *pret = &ret;" << std::endl
     << frag8 << std::endl
     << "  return ret;" << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;

  ss << "void SetupStateInfo(double * SI)" << std::endl;
  frag = cci->stateInformationString();
  fragLen = wcstombs(NULL, frag, 0) + 1;
  frag8 = new char[fragLen];
  wcstombs(frag8, frag, fragLen);
  ss << "{" << std::endl
     << frag8 << std::endl
     << "}" << std::endl;
  free(frag);
  delete [] frag8;
  ss.close();

  void* mod = CompileSource(dirname, sourcename, mLastError);
  IDACompiledModelFunctions* cmf = SetupIDACompiledModelFunctions(mod);
  
  return new CDA_DAESolverModel(mod, cmf, aModel, cci, dirname);
}

iface::cellml_services::ODESolverRun*
CDA_CellMLIntegrationService::createODEIntegrationRun
(
 iface::cellml_services::ODESolverCompiledModel* aModel
)
  throw (std::exception&)
{
  return new CDA_ODESolverRun(unsafe_dynamic_cast<CDA_ODESolverModel*>(aModel));
}

iface::cellml_services::DAESolverRun*
CDA_CellMLIntegrationService::createDAEIntegrationRun
(
 iface::cellml_services::DAESolverCompiledModel* aModel
)
  throw (std::exception&)
{
  return new CDA_DAESolverRun(unsafe_dynamic_cast<CDA_DAESolverModel*>(aModel));
}

iface::cellml_services::CellMLIntegrationService*
CreateIntegrationService()
{
  return new CDA_CellMLIntegrationService();
}
