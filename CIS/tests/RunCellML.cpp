#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCCGS.hxx"
#include "IfaceCIS.hxx"
#include "CISBootstrap.hpp"
#include "CCGSBootstrap.hpp"
#include "cda_compiler_support.h"
#include "CellMLBootstrap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#define strcasecmp _stricmp
#endif
#include "Utilities.hxx"

#ifdef ENABLE_FIND_NUMERIC_ERRORS
#include <fenv.h>
#endif

bool gFinished = false;
double gStart = 0.0, gStop = 10.0, gDensity = 1000.0;
double gTabStep = 0.0;
bool gTStrict = false;
uint32_t gSleepTime = 0;


#ifdef WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#endif

class TestProgressObserver
  : public iface::cellml_services::IntegrationProgressObserver
{
public:
  TestProgressObserver(iface::cellml_services::CellMLCompiledModel* aCCM)
    : mRefcount(1)
  {
    mCCM = aCCM;
    mCCM->add_ref();
    mCI = mCCM->codeInformation();

    iface::cellml_services::ComputationTargetIterator* cti =
      mCI->iterateTargets();
    bool first = true;
    while (true)
    {
      iface::cellml_services::ComputationTarget* ct = cti->nextComputationTarget();
      if (ct == NULL)
        break;
      if ((ct->type() == iface::cellml_services::STATE_VARIABLE ||
           ct->type() == iface::cellml_services::ALGEBRAIC ||
           ct->type() == iface::cellml_services::VARIABLE_OF_INTEGRATION) &&
          ct->degree() == 0)
      {
        iface::cellml_api::CellMLVariable* source = ct->variable();
        wchar_t* n = source->name();
        source->release_ref();
        printf(first ? "\"%S\"" : ",\"%S\"", n);
        first = false;
        free(n);
      }
      ct->release_ref();
    }
    printf("\n");
    cti->release_ref();
  }

  ~TestProgressObserver()
  {
    mCCM->release_ref();
    mCI->release_ref();
  }

  void add_ref()
    throw(std::exception&)
  {
    mRefcount++;
  }

  void release_ref()
    throw(std::exception&)
  {
    mRefcount--;
    if (mRefcount == 0)
      delete this;
  }

  char* objid()
    throw (std::exception&)
  {
    return strdup("singletonTestProgressObserver");
  }

  void* query_interface(const char* iface)
    throw (std::exception&)
  {
    if (!strcmp(iface, "xpcom::IObject"))
      return static_cast< ::iface::XPCOM::IObject* >(this);
    else if (!strcmp(iface, "cellml_services::IntegrationProgressObserver"))
      return
        static_cast< ::iface::cellml_services::IntegrationProgressObserver*>
        (this);
    return NULL;
  }

  void computedConstants(uint32_t length, double* values)
    throw (std::exception&)
  {
    iface::cellml_services::ComputationTargetIterator* cti =
      mCI->iterateTargets();
    while (true)
    {
      iface::cellml_services::ComputationTarget* ct = cti->nextComputationTarget();
      if (ct == NULL)
        break;
      if (ct->type() == iface::cellml_services::CONSTANT &&
          ct->degree() == 0)
      {
        iface::cellml_api::CellMLVariable* source = ct->variable();
        wchar_t* n = source->name();
        source->release_ref();
        printf("# Computed constant: %S = %e\n", n, values[ct->assignedIndex()]);
        free(n);
      }
      ct->release_ref();
    }
    cti->release_ref();
  }

  void results(uint32_t length, double* values)
    throw (std::exception&)
  {
    uint32_t aic = mCI->algebraicIndexCount();
    uint32_t ric = mCI->rateIndexCount();
    uint32_t recsize = 2 * ric + aic + 1;

    if (recsize == 0)
      return;

    uint32_t i;
    for (i = 0; i < length; i += recsize)
    {
      bool first = true;
      iface::cellml_services::ComputationTargetIterator* cti =
        mCI->iterateTargets();
      while (true)
      {
        iface::cellml_services::ComputationTarget* ct = cti->nextComputationTarget();
        if (ct == NULL)
          break;

        if (ct->degree() != 0)
        {
          ct->release_ref();
          continue;
        }

        iface::cellml_services::VariableEvaluationType et = ct->type();
        uint32_t varOff = 0;
        
        switch (et)
        {
        case iface::cellml_services::STATE_VARIABLE:
          varOff = 1 + ct->assignedIndex();
          break;
        case iface::cellml_services::VARIABLE_OF_INTEGRATION:
          varOff = 0;
          break;
        case iface::cellml_services::ALGEBRAIC:
          varOff = 1 + 2 * ric + ct->assignedIndex();
          break;
        default:
          ct->release_ref();
          continue;
        }

        printf(first ? "\"%g\"" : ",\"%g\"", values[i + varOff]);
        first = false;

        ct->release_ref();
      }
      printf("\n");
      cti->release_ref();
    }
  }

  void done()
    throw (std::exception&)
  {
    printf("# Run completed.\n");
    gFinished = true;
  }

  void failed(const char* errmsg)
    throw (std::exception&)
  {
    printf("# Integration failed (%s)\n", errmsg);
    gFinished = true;
  }
private:
  iface::cellml_services::CellMLCompiledModel* mCCM;
  iface::cellml_services::CodeInformation* mCI;
  uint32_t mRefcount;
};

void
ProcessKeywords(int argc, char** argv,
                iface::cellml_services::CellMLIntegrationRun* run)
{
  // Scoped locale change.
  CNumericLocale locobj;

  argc -= 2;
  argv += 2;
  while (argc >= 2)
  {
    const char* command = argv[0];
    char* value = argv[1];
    argc -= 2;
    argv += 2;
    
    if (!strcasecmp(command, "step_type"))
    {
      iface::cellml_services::IntegrationStepType ist;
      if (!strcasecmp(value, "RK2"))
        ist = iface::cellml_services::RUNGE_KUTTA_2_3;
      else if (!strcasecmp(value, "RK4"))
        ist = iface::cellml_services::RUNGE_KUTTA_4;
      else if (!strcasecmp(value, "RKF45"))
        ist = iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5;
      else if (!strcmp(value, "RKCK"))
        ist = iface::cellml_services::RUNGE_KUTTA_CASH_KARP_4_5;
      else if (!strcasecmp(value, "RKPD"))
        ist = iface::cellml_services::RUNGE_KUTTA_PRINCE_DORMAND_8_9;
      else if (!strcasecmp(value, "RK2IMP"))
        ist = iface::cellml_services::RUNGE_KUTTA_IMPLICIT_2;
      else if (!strcasecmp(value, "RK2SIMP"))
        ist = iface::cellml_services::RUNGE_KUTTA_IMPLICIT_2_SOLVE;
      else if (!strcasecmp(value, "RK4IMP"))
        ist = iface::cellml_services::RUNGE_KUTTA_IMPLICIT_4;
      else if (!strcasecmp(value, "BSIMP"))
        ist = iface::cellml_services::BULIRSCH_STOER_IMPLICIT_BD;
      else if (!strcasecmp(value, "GEAR1"))
        ist = iface::cellml_services::GEAR_1;
      else if (!strcasecmp(value, "GEAR2"))
        ist = iface::cellml_services::GEAR_2;
      else if (!strcasecmp(value, "AM_1_12"))
        ist = iface::cellml_services::ADAMS_MOULTON_1_12;
      else if (!strcasecmp(value, "BDF15SIMP"))
        ist = iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE;
      else
      {
        printf("# Warning: Unsupported step_type value %s (ignored)\n",
               value);
        continue;
      }
      run->stepType(ist);
    }
    else if (!strcasecmp(command, "step_size_control"))
    {
      double epsAbs, epsRel, scalVar, scalRate, maxStep;
      epsAbs = strtod(value, &value);
      if (*value != ',')
      {
        printf("# Warning: Expected ',' after absolute epsilon. "
               "step_size_control ignored.\n");
        continue;
      }
      value++;
      epsRel = strtod(value, &value);
      if (*value == ',')
      {
        value++;
        scalVar = strtod(value, &value);
        if (scalVar < 0.0 || scalVar > 1.0)
        {
          printf("# Variable scale should be in [0.0,1.0]. "
                 "step_size_control ignored.\n");
          continue;
        }
        scalRate = 1.0 - scalVar;
        
        if (*value == ',')
        {
          value++;
          maxStep = strtod(value, &value);
          if (maxStep <= 0.0)
          {
            printf("# Maximum step size should be in (0.0,infinity). "
                   "step_size_control ignored.\n");
            continue;
          }
        }
        else
        {
          maxStep = 1.0;
        }
      }
      else
      {
        scalVar = 1.0;
        scalRate = 0.0;
        maxStep = 1.0;
      }
      if (epsRel <= 0.0 || epsAbs <= 0.0)
      {
        printf("# Absolute and relative epsilon values must be positive. "
               "step_size_control ignored.\n");
        continue;
      }
      run->setStepSizeControl(epsAbs, epsRel, scalVar, scalRate, maxStep);
    }
    else if (!strcasecmp(command, "range"))
    {
      double start, stop, density;
      start = strtod(value, &value);
      if (*value != ',')
      {
        printf("# Warning: Expected ',' after starting point. "
               "range ignored.\n");
        continue;
      }
      value++;
      stop = strtod(value, &value);
      if (*value != ',')
      {
        printf("# Warning: Expected ',' after stopping point. "
               "range ignored.\n");
        continue;
      }
      value++;
      density = strtod(value, &value);
      run->setResultRange(start, stop, density);
      gStart = start;
      gStop = stop;
      gDensity = density;
    }
    else if (!strcasecmp(command, "tabulation"))
    {
      double tabstepsize;
      bool tstrict;
      tabstepsize = strtod(value, &value);
      if (*value != ',')
      {
        printf("# Warning: Expected ',' after starting point. "
               "tabulation ignored.\n");
        continue;
      }
      value++;
      tstrict = !strcasecmp(value, "true");
      run->setTabulationStepControl(tabstepsize, tstrict);
      gTabStep = tabstepsize;
      gTStrict = tstrict;
    }
    // A special undocumented debugging command...
    else if (!strcasecmp(command, "sleep_time"))
    {
      gSleepTime = strtoul(value, NULL, 10);
    }
    else
      printf("# Warning: Unrecognised command %s. Ignored.\n",
             command);
  }
}

int
main(int argc, char** argv)
{
  // Get the URL from which to load the model...
  if (argc < 2 || !strcasecmp(argv[1], "--help"))
  {
    printf("Usage: RunCellML modelURL (options)*\n"
           "Available options:\n"
           "  step_type RK2|RK4|RKF45|RKCK|RKPD|"
           "RK2IMP|RK2SIMP|RK4IMP|BSIMP|GEAR1|GEAR2|AM_1_12|BDF15SIMP\n"
           "    => Sets the stepping algorithm to use:\n"
           "      RK2     = 2nd order Runge-Kutta.\n"
           "      RK4     = 4th order Runge-Kutta.\n"
           "      RKF45   = Embedded Runge-Kutta-Fehlberg (4,5).\n"
           "      RKCK    = Embedded Runge-Kutta Cash-Karp (4, 5).\n"
           "      RKPD    = Embedded Runge-Kutta Prince-Dormand (8,9).\n"
           "      RK2IMP  = Implicit 2nd order Runge-Kutta.\n"
           "      RK2SIMP = Implicit 2nd order Runge-Kutta with non-linear solve.\n"
           "      RK4IMP  = Implicit 4th order Runge-Kutta.\n"
           "      BSIMP   = Implicit Bulirsch-Stoer method of Bader and Deuflhard.\n"
           "      GEAR1   = Implict Gear method (M=1).\n"
           "      GEAR2   = Implict Gear method (M=2).\n"
           "    AM_1_12   = Adams-Moulton (1-12)\n"
           "  BDF15SIMP   = BDF(1-5) with non-linear solve.\n"
           "  step_size_control absolute_epsilon,relative_epsilon[,variable_weight[,max_step]]\n"
           "    => Sets the step-size control parameters.\n"
           "      absolute_epsilon: A floating point absolute error tolerance value.\n"
           "      relative_epsilon: A floating point relative error tolerance value.\n"
           "      variable_weight: The weighting (default 1.0) for variables (vs rates).\n"
           "      max_step: The maximum step size to ever take.\n"
           "  range start,stop,density\n"
           "    => Sets the range to solve over.\n"
           "       start: A floating point start value.\n"
           "       stop: A floating point stop value.\n"
           "       density: A floating point value specifying the maximum "
           "density of points (as a number of points for the whole run).\n"
           "  tabulation step_size,true|false\n"
           "    => Sets the interval in the bound variable for guaranteed values in other variables,\n"
           "       and whether to only tabulate values at points that are thus guaranteed.\n"
           "       step_size: A floating point tabulation step size.\n"
          );
    return -1;
  }

  wchar_t* URL;
  size_t l = strlen(argv[1]);
  URL = new wchar_t[l + 1];
  memset(URL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(URL, &mbrurl, l, NULL);

  iface::cellml_api::CellMLBootstrap* cb =
    CreateCellMLBootstrap();

  iface::cellml_api::ModelLoader* ml =
    cb->modelLoader();
  cb->release_ref();

  iface::cellml_api::Model* mod;
  printf("# Loading model...\n");
  try
  {
    mod = ml->loadFromURL(URL);
  }
  catch (...)
  {
    printf("Error loading model URL.\n");
    // Well, a leak on exit wouldn't be so bad, but someone might reuse this
    // code, so...
    delete [] URL;
    ml->release_ref();
    return -1;
  }

  ml->release_ref();
  delete [] URL;

  printf("# Creating integration service...\n");
  iface::cellml_services::CellMLIntegrationService* cis =
    CreateIntegrationService();

  iface::cellml_services::CellMLCompiledModel* ccm = NULL;
  try
  {
    printf("# Compiling model...\n");
    ccm = cis->compileModel(mod);
  }
  catch (iface::cellml_api::CellMLException& ce)
  {
    wchar_t* err = cis->lastError();
    printf("Caught a CellMLException while compiling model: %S\n", err);
    free(err);
    mod->release_ref();
    cis->release_ref();
    return -1;
  }
  catch (...)
  {
    printf("Unexpected exception calling compileModel!\n");
    // this is a leak, but it should also never happen :)
    return -1;
  }
  mod->release_ref();

  printf("# Creating run...\n");
  iface::cellml_services::CellMLIntegrationRun* cir =
    cis->createIntegrationRun(ccm);
  cis->release_ref();

  TestProgressObserver* tpo = new TestProgressObserver(ccm);
  cir->setProgressObserver(tpo);
  tpo->release_ref();

  ProcessKeywords(argc, argv, cir);

#ifdef ENABLE_FIND_NUMERIC_ERRORS
  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
#endif

  cir->start();
  cir->release_ref();
  ccm->release_ref();

  while (!gFinished)
    sleep(1);

  if (gSleepTime)
    sleep(gSleepTime);

  return 0;
}
