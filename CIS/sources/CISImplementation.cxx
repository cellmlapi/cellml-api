#define IN_CIS_MODULE
#include <stdlib.h>
#include <string>
#include "Utilities.hxx"
#include <sys/stat.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/errno.h>
#include <dlfcn.h>
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
  cmf->SetupFixedConstants = (void (*)(double*))
    getsym(module, "SetupFixedConstants");
  cmf->SetupComputedConstants = (void (*)(double*,double*))
    getsym(module, "SetupComputedConstants");
  cmf->ComputeRates = (void (*)(double*,double*,double*,double*))
    getsym(module, "ComputeRates");
  cmf->ComputeVariables = (void (*)(double*,double*,double*,double*))
    getsym(module, "ComputeVariables");
  cmf->ComputeVariablesForRates = (void (*)(double*,double*,double*,double*))
    getsym(module, "ComputeVariablesForRates");
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
    "-nodefaultlibs -Llib -lcis "
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
  int ret = system(cmd.c_str());
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
    lastError = L"Cannot load the model code module.";
    throw iface::cellml_api::CellMLException();
  }

  return t;
}

CDA_CellMLCompiledModel::CDA_CellMLCompiledModel
(
 void* aModule, CompiledModelFunctions* aCMF,
 iface::cellml_api::Model* aModel,
 iface::cellml_services::CCodeInformation* aCCI,
 std::string& aDirname
)
  : _cda_refcount(1), mModule(aModule), mCMF(aCMF), mModel(aModel), mCCI(aCCI),
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
  delete mCMF;
  mModel->release_ref();
  mCCI->release_ref();
#ifdef WIN32
  struct _finddata_t d;
  intptr_t hd;
  std::string pat = mDirname;
  pat += "\*.*";
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
 iface::cellml_services::CellMLCompiledModel* aModel
)
  : _cda_refcount(1),
    mStepType(iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5),
    mEpsAbs(1E-6), mEpsRel(1E-6), mScalVar(1.0), mScalRate(0.0),
    mStepSizeMax(1.0), mStartBvar(0.0), mStopBvar(10.0), mMaxPointDensity(10000.0),
    mObserver(NULL), mCancelIntegration(false)
{
  mModel = dynamic_cast<CDA_CellMLCompiledModel*>(aModel);
  if (mModel == NULL)
    throw iface::cellml_api::CellMLException();
  mModel->add_ref();
}

CDA_CellMLIntegrationRun::~CDA_CellMLIntegrationRun()
{
  mModel->release_ref();
  if (mObserver != NULL)
    mObserver->release_ref();
}

iface::cellml_services::IntegrationStepType
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
 iface::cellml_services::IntegrationStepType aStepType
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
  else if (aType == iface::cellml_services::DIFFERENTIAL)
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
CDA_CellMLIntegrationRun::runthread()
{
  std::string emsg = "Unknown error";
  double* constants = NULL, * variables = NULL, * rates = NULL;
  try
  {
    CompiledModelFunctions* f = mModel->mCMF;
    uint32_t varSize = mModel->mCCI->variableCount();
    uint32_t constSize = mModel->mCCI->constantCount();
    uint32_t boundSize = mModel->mCCI->boundCount();
    uint32_t rateSize = mModel->mCCI->rateVariableCount();
    if (boundSize > 1)
    {
      emsg = "This integrator only supports systems with one bound variable "
        "shared across all ODEs.";
      throw std::exception();
    }
    constants = new double[constSize];
    variables = new double[varSize + boundSize];
    rates = new double[rateSize];
    f->SetupFixedConstants(constants);
    // Now apply constant overrides...
    OverrideList::iterator oli;
    for (oli = mConstantOverrides.begin(); oli != mConstantOverrides.end();
         oli++)
      if ((*oli).first < constSize)
        constants[(*oli).first] = (*oli).second;

    f->SetupComputedConstants(constants, variables);
    // Now apply IV overrides...
    for (oli = mIVOverrides.begin(); oli != mIVOverrides.end();
         oli++)
      if ((*oli).first < rateSize)
        variables[(*oli).first] = (*oli).second;

    if (boundSize == 1)
      variables[varSize] = mStartBvar;
    f->ComputeVariables(variables + varSize, rates, constants, variables);

    if (mObserver != NULL)
      mObserver->results(varSize + boundSize, variables);

    if (boundSize == 1)
      SolveODEProblem(f, constSize, constants, varSize, variables, rateSize,
                      rates);
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
  if (variables != NULL)
    delete [] variables;
  if (rates != NULL)
    delete [] rates;
  release_ref();
}

iface::cellml_services::CellMLCompiledModel*
CDA_CellMLIntegrationService::compileModel
(
 iface::cellml_services::CGenerator* aCG,
 iface::cellml_api::Model* aModel
)
  throw(std::exception&)
{
  // Generate code information...
  ObjRef<iface::cellml_services::CCodeInformation> cci;
  try
  {
    cci = already_AddRefd<iface::cellml_services::CCodeInformation>
      (aCG->generateCode(aModel));
  }
  catch (...)
  {
    wchar_t* le = aCG->lastError();
    mLastError = le;
    free(le);
    throw iface::cellml_api::CellMLException();
  }

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
  std::string dirname = fn;
  free(fn);

  // We now have a temporary directory. Make the source file...
  std::string sourcename = dirname + "/generated.c";
  std::ofstream ss(sourcename.c_str());
  ss << "/* This file is automatically generated and will be automatically"
     << std::endl
     << " * deleted. Don't edit it or changes will be lost. */" << std::endl;
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
     << "extern double arbitrary_log(double x, double base);" << std::endl
     << "extern double gcd_pair(double a, double b);" << std::endl
     << "extern double lcm_pair(double a, double b);" << std::endl
     << "extern double gcd_multi(unsigned int size, ...);" << std::endl
     << "extern double lcm_multi(unsigned int size, ...);" << std::endl
     << "extern double multi_min(unsigned int size, ...);" << std::endl
     << "extern double multi_max(unsigned int size, ...);" << std::endl;
  char* frag = cci->functionsFragment();
  ss << frag << std::endl;
  free(frag);
  ss << "void SetupFixedConstants(double* CONSTANTS)" << std::endl;
  frag = cci->fixedConstantFragment();
  ss << "{" << std::endl << frag << std::endl << "}" << std::endl;
  free(frag);
  ss << "void SetupComputedConstants(double* CONSTANTS, double* VARIABLES)"
     << std::endl;
  frag = cci->computedConstantFragment();
  ss << "{" << std::endl << frag << std::endl << "}" << std::endl;
  free(frag);
  ss << "void ComputeRates(double* BOUND, double* RATES, double* CONSTANTS, "
    "double* VARIABLES)" << std::endl;
  frag = cci->rateCodeFragment();
  ss << "{" << std::endl << frag << std::endl << "}" << std::endl;
  free(frag);
  ss << "void ComputeVariables(double* BOUND, double* RATES, double* CONSTANTS, "
    "double* VARIABLES)" << std::endl;
  frag = cci->variableCodeFragment();
  ss << "{" << std::endl << frag << std::endl << "}" << std::endl;
  ss << "void ComputeVariablesForRates(double* BOUND, double* RATES, double* CONSTANTS, "
    "double* VARIABLES)" << std::endl;
  ss << "#define VARIABLES_FOR_RATES_ONLY" << std::endl;
  ss << "{" << std::endl << frag << std::endl << "}" << std::endl;
  free(frag);
  ss.close();

  void* mod = CompileSource(dirname, sourcename, mLastError);
  CompiledModelFunctions* cmf = SetupCompiledModelFunctions(mod);
  
  return new CDA_CellMLCompiledModel(mod, cmf, aModel, cci, dirname);
}

iface::cellml_services::CellMLIntegrationRun*
CDA_CellMLIntegrationService::createIntegrationRun
(
 iface::cellml_services::CellMLCompiledModel* aModel
)
  throw (std::exception&)
{
  return new CDA_CellMLIntegrationRun(aModel);
}

iface::cellml_services::CellMLIntegrationService*
CreateIntegrationService()
{
  return new CDA_CellMLIntegrationService();
}
