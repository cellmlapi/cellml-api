#define MODULE_CONTAINS_CIS
#define GSL_DLL
#include <exception>
#include "cda_compiler_support.h"
#include <limits>
#include <sstream>
#include "Utilities.hxx"
#include "CISImplementation.hxx"
#ifdef ENABLE_GSL_INTEGRATORS
#include <gsl/gsl_odeiv.h>
#include <gsl/gsl_errno.h>
#endif
#include <math.h>
#include <stdarg.h>
#include <assert.h>
#ifdef ENABLE_GSL_INTEGRATORS
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>
#endif
#ifdef WIN32
#include <io.h>
#include <time.h>
#endif

#undef N
#undef M
#include <cvode/cvode.h>
#include <ida/ida.h>
// #include <ida/ida_spgmr.h>
#include <ida/ida_dense.h>
// #include <ida/ida_sptfqmr.h>

#include <nvector/nvector_serial.h>
#include <sundials/sundials_nvector.h>
#include <sundials/sundials_types.h>
#include <cvode/cvode_dense.h>
#include <cvode/cvode_dense.h>

#ifdef _MSC_VER
#include <float.h>
#define isfinite _finite
#define isnan _isnan
#define isinf(x) (_fpclass(x) & (_FPCLASS_PINF | _FPCLASS_NINF) != 0)
#define INFINITY (double)(0x7FF0000000000000L)
double asinh(double input)
{
  return log(input + sqrt(input*input + 1.0));
}
double acosh(double input)
{
  return log(input +
             sqrt((input + 1.0) * (input - 1.0)));
}
double atanh(double input)
{
  return 0.5 * log((1 + input) / (1 - input));
}
#endif

#include <kinsol/kinsol.h>
#include <kinsol/kinsol_spgmr.h>

#include "CISModelSupport.h"

// It would be good to one day make this configurable by the user.
#define SUBSOL_TOLERANCE 1E-6

void clearFailure(struct fail_info* aFail)
{
  aFail->failtype = 0;
  aFail->failmsg = "";
}

void setFailure(struct fail_info* aFail, const char* aMsg, int type)
{
  aFail->failtype = type;
  aFail->failmsg = aMsg;
}

void failAddCause(struct fail_info* aFail, const char* aCause)
{
  if (!aFail->failtype)
  {
    setFailure(aFail, aCause, -1);
    return;
  }
  aFail->failmsg = aCause + (", caused by " + aFail->failmsg);
}

struct EvaluationInformation
{
  double* constants, * rates, * algebraic, * states;
  uint32_t rateSizeBytes, rateSize;
  struct fail_info* failInfo;
  void (*ComputeRates)(double VOI, double* CONSTANTS, double* RATES,
                       double* STATES, double* ALGEBRAIC, struct fail_info*);
  void (*ComputeVariables)(double VOI, double* CONSTANTS, double* RATES,
                           double* STATES, double* ALGEBRAIC, struct fail_info*);
};

#ifdef ENABLE_GSL_INTEGRATORS
int
EvaluateRatesGSL(double voi, const double vars[],
                 double rates[], void* params)
{
  EvaluationInformation* ei = reinterpret_cast<EvaluationInformation*>(params);
  
  // Update variables that change based on bound/other vars...
  int ret = ei->ComputeRates(voi, ei->constants, ei->rates, const_cast<double*>(vars),
                             ei->algebraic);

  if (rates != ei->rates)
    memcpy(rates, ei->rates, ei->rateSizeBytes);

  if (ret == 0)
    return GSL_SUCCESS;
  else if (ret < 0)
    return GSL_FAILURE;
  else
    return GSL_CONTINUE;
}

inline int
EvaluateJacobianGSL
(
 double voi, const double vars[],
 double jac[], double rates[], void* params
)
{
  EvaluationInformation* ei = reinterpret_cast<EvaluationInformation*>(params);
  
  // Back up the states, so we can perturb them...
  double* states = new double[ei->rateSize];
  memcpy(states, vars, ei->rateSizeBytes);

  double* rate0 = new double[ei->rateSize];
  double* rate1 = new double[ei->rateSize];

  // Initial rates...
  ei->ComputeRates(voi, ei->constants, rate0, states,
                   ei->algebraic);

  uint32_t i, j;
  for (i = 0; i < ei->rateSize; i++)
  {
    double perturb = vars[i] * 1E-10;
    if (perturb == 0)
      perturb = 1E-90;

    states[i] = vars[i] + perturb;

    ei->ComputeRates(voi, ei->constants, rate1, states, ei->algebraic);

    for (j = 0; j < ei->rateSize; j++)
    {
      jac[i * ei->rateSize + j] = (rate1[j] - rate0[j]) / perturb;
    }

    states[i] = vars[i];
  }

  // Now perturb the VOI and see what happens...
  double perturb = voi * 1E-13;
  if (perturb == 0)
    perturb = 1E-90;
  double newvoi = voi + perturb;

  ei->ComputeRates(newvoi, ei->constants, rate1, states, ei->algebraic);

  for (i = 0; i < ei->rateSize; i++)
  {
    rates[i] = (rate1[i] - rate0[i]) / perturb;
  }

  delete [] rate0;
  delete [] rate1;
  delete [] states;

  return GSL_SUCCESS;
}
#endif

#define tabulationRelativeTolerance 1E-20

bool floatsEqual
(
 const double &candidate, const double &expected, const double &tolerance
)
{
#undef min
  return fabs(candidate - expected) / (fabs(expected) + std::numeric_limits<double>::min()) <= tolerance;
}

int
EvaluateRatesCVODE(double bound, N_Vector varsV, N_Vector ratesV, void* params)
{
  EvaluationInformation* ei = reinterpret_cast<EvaluationInformation*>(params);
  
  // Update variables that change based on bound/other vars...
  ei->ComputeRates(bound, ei->constants, ei->rates, N_VGetArrayPointer_Serial(varsV),
                   ei->algebraic, ei->failInfo);

  double* rates = N_VGetArrayPointer_Serial(ratesV);
  if (rates != ei->rates)
    memcpy(rates, ei->rates, ei->rateSizeBytes);

  for (int i = 0; i < NV_LENGTH_S(ratesV); i++)
  {
    if (!isfinite(NV_Ith_S(ratesV, i)))
    {
      if (!ei->failInfo->failtype)
        setFailure(ei->failInfo, "One of the rates cannot be represented as a valid finite number", -1);
    }
    if (ei->failInfo->failtype)
      break;
  }

  return ei->failInfo->failtype;
}

// Don't cache more than 2MB of variables (assuming 8 bytes per variable). This
// leaves a little bit of room in the 2MB for CORBA overhead.
#define VARIABLE_STORAGE_LIMIT 262016
// Don't cache for more than 1 second...
#define VARIABLE_TIME_LIMIT 1

bool
CDA_CellMLIntegrationRun::checkPauseOrCancellation()
{
#ifdef WIN32
  COMMTIMEOUTS timeouts;
#else
  fd_set pipeset;
  FD_ZERO(&pipeset);
  FD_SET(mThreadPipes[0], &pipeset);
  struct timeval to;
#endif
  while (true)
  {
#ifdef WIN32
    // Don't block...
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    SetCommTimeouts(mThreadPipes[0], &timeouts);
#else
    to.tv_sec = 0;
    to.tv_usec = 0;
    if (select(1, &pipeset, NULL, NULL, &to) == 0)
      return false;
#endif

    int command;
#ifdef WIN32
    ReadFile(mThreadPipes[0], &command, sizeof(command), NULL, NULL);
#else
    read(mThreadPipes[0], &command, sizeof(command));
#endif

    if (command == 1) // Cancel
      return true;
    else if (command == 2) // Pause
    {
      while (command == 2)
      {
#ifdef WIN32
        // Do block...
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        SetCommTimeouts(mThreadPipes[0], &timeouts);
        ReadFile(mThreadPipes[0], &command, sizeof(command),
                 NULL, NULL);
#else
        read(mThreadPipes[0], &command, sizeof(command));
#endif
        if (command == 1) // Cancel
          return true;
        else if (command == 3) // Resume
          break;
      }
    }
  }
}

#ifdef ENABLE_GSL_INTEGRATORS
void
CDA_ODESolverRun::SolveODEProblemGSL
(
 CompiledModelFunctions* f, uint32_t constSize,
 double* constants, uint32_t rateSize, double* rates,
 double* states, uint32_t algSize, double* algebraic
)
{
  gsl_odeiv_system sys;
  EvaluationInformation ei;

  sys.dimension = rateSize;
  sys.params = reinterpret_cast<void*>(&ei);

  sys.function = EvaluateRatesGSL;
  sys.jacobian = EvaluateJacobianGSL;
  ei.constants = constants;
  ei.states = states;
  ei.rates = rates;
  ei.algebraic = algebraic;

  ei.rateSize = rateSize;
  ei.rateSizeBytes = rateSize * sizeof(double);

  ei.ComputeRates = f->ComputeRates;
  ei.ComputeVariables = f->ComputeVariables;

  gsl_odeiv_step* s;
  gsl_odeiv_evolve* e = gsl_odeiv_evolve_alloc(rateSize);
  gsl_odeiv_control* c = gsl_odeiv_control_standard_new(mEpsAbs, mEpsRel,
                                                        mScalVar, mScalRate);

  // Allocate a control function...
  const gsl_odeiv_step_type* T = NULL;
  switch (mStepType)
  {
  case iface::cellml_services::RUNGE_KUTTA_2_3:
    T = gsl_odeiv_step_rk2;
    break;
  case iface::cellml_services::RUNGE_KUTTA_4:
    T = gsl_odeiv_step_rk4;
    break;
  case iface::cellml_services::RUNGE_KUTTA_FEHLBERG_4_5:
    T = gsl_odeiv_step_rkf45;
    break;
  case iface::cellml_services::RUNGE_KUTTA_CASH_KARP_4_5:
    T = gsl_odeiv_step_rkck;
    break;
  case iface::cellml_services::RUNGE_KUTTA_PRINCE_DORMAND_8_9:
    T = gsl_odeiv_step_rk8pd;
    break;
  case iface::cellml_services::RUNGE_KUTTA_IMPLICIT_2:
    T = gsl_odeiv_step_rk2imp;
    break;
  case iface::cellml_services::RUNGE_KUTTA_IMPLICIT_2_SOLVE:
    T = gsl_odeiv_step_rk2simp;
    break;
  case iface::cellml_services::RUNGE_KUTTA_IMPLICIT_4:
    T = gsl_odeiv_step_rk4imp;
    break;
  case iface::cellml_services::BULIRSCH_STOER_IMPLICIT_BD:
    T = gsl_odeiv_step_bsimp;
    break;
  case iface::cellml_services::GEAR_1:
    T = gsl_odeiv_step_gear1;
    break;
  case iface::cellml_services::GEAR_2:
  default:
    T = gsl_odeiv_step_gear2;
    break;
  }
  s = gsl_odeiv_step_alloc(T, rateSize);
  
  // Start the main loop...
  double voi = mStartBvar;
  double stepSize = 1E-6;

  uint32_t recsize = rateSize * 2 + algSize + 1;
  uint32_t storageCapacity = (VARIABLE_STORAGE_LIMIT / recsize) * recsize;
  double* storage = new double[storageCapacity];
  uint32_t storageExpiry = time(0) + VARIABLE_TIME_LIMIT;
  uint32_t storageSize = 0;

  double lastVOI = 0.0 /* initialised only to avoid extraneous warning. */;
  bool isFirst = true;

  double minReportForDensity = (mStopBvar - mStartBvar) / mMaxPointDensity;
  uint32_t tabStepNumber = 1;
  double nextStopPoint = mTabulationStepSize + voi;
  if (mTabulationStepSize == 0.0)
    nextStopPoint = mStopBvar;

  while (voi < mStopBvar)
  {
    double bhl = mStopBvar;
    if (bhl - voi > mStepSizeMax)
      bhl = voi + mStepSizeMax;
    if(bhl > nextStopPoint)
      bhl = nextStopPoint;
    if (floatsEqual(bhl, mStopBvar, tabulationRelativeTolerance))
    {
      nextStopPoint = mStopBvar;
      bhl = mStopBvar;
    }

    gsl_odeiv_evolve_apply(e, c, s, &sys, &voi, bhl,
                           &stepSize, states);
    if (checkPauseOrCancellation())
      break;

    if (isFirst)
      isFirst = false;
    else if (voi - lastVOI < minReportForDensity && !floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance))
      continue;

    if(mStrictTabulation && !floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance))
      continue;

    if (voi==nextStopPoint)
      nextStopPoint = (mTabulationStepSize * ++tabStepNumber) + mStartBvar;

    lastVOI = voi;

    // Compute the extra variables that are only computed for display
    // purposes...
    f->ComputeVariables(voi, constants, rates, states, algebraic);

    // Add to storage...
    storage[storageSize] = voi;
    memcpy(storage + storageSize + 1, states, rateSize * sizeof(double));
    memcpy(storage + storageSize + 1 + rateSize, rates,
           rateSize * sizeof(double));
    memcpy(storage + storageSize + 1 + rateSize * 2, algebraic,
           algSize * sizeof(double));

    storageSize += recsize;

    // Are we ready to send?
    uint32_t timeNow = time(0);
    if (timeNow >= storageExpiry || storageSize == storageCapacity)
    {
      if (mObserver != NULL)
        mObserver->results(storageSize, storage);
      storageExpiry = timeNow + VARIABLE_TIME_LIMIT;
      storageSize = 0;
    }
  }
  if (storageSize != 0 && mObserver != NULL)
  {
    mObserver->results(storageSize, storage);
  }
  if (mObserver != NULL)
    mObserver->done();

  delete [] storage;

  // Free gsl structures...
  gsl_odeiv_evolve_free(e);
  gsl_odeiv_control_free(c);
  gsl_odeiv_step_free(s);
}
#endif

void
cda_cvode_error_handler(int error_code, const char* module,
                        const char* function, char* msg, void* eh_data)
{
  struct fail_info* failInfo = reinterpret_cast<struct fail_info*>(eh_data);
  setFailure(failInfo, msg, -1);
}

void
cda_ida_error_handler(int error_code, const char* module,
                        const char* function, char* msg, void* eh_data)
{
  setFailure(reinterpret_cast<struct fail_info*>(eh_data), msg, -1);
}

void
CDA_ODESolverRun::SolveODEProblemCVODE
(
 CompiledModelFunctions* f, uint32_t constSize,
 double* constants, uint32_t rateSize, double* rates,
 double* states, uint32_t algSize, double* algebraic
)
{
  N_Vector y = NULL;
  if (rateSize != 0)
    y = N_VMake_Serial(rateSize, states);
  void* solver = NULL;
  struct fail_info failInfo;

  if (rateSize != 0)
  {
    switch (mStepType)
    {
    case iface::cellml_services::ADAMS_MOULTON_1_12:
      solver = CVodeCreate(CV_ADAMS, CV_FUNCTIONAL);
      break;
    case iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE:
    default:
      solver = CVodeCreate(CV_BDF, CV_NEWTON);
      break;
    }

    CVodeSetErrHandlerFn(solver, cda_cvode_error_handler, &failInfo);
  }

  EvaluationInformation ei;
  ei.failInfo = &failInfo;

  if (rateSize != 0)
  {
    CVodeInit(solver, EvaluateRatesCVODE, mStartBvar, y);
    CVodeSStolerances(solver, mEpsRel, mEpsAbs);
    if (mStepType == iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE)
      CVDense(solver, rateSize);
    CVodeSetUserData(solver, &ei);
  }

  ei.constants = constants;
  ei.states = states;
  ei.rates = rates;
  ei.algebraic = algebraic;
  ei.rateSize = rateSize;
  ei.rateSizeBytes = rateSize * sizeof(double);
  ei.ComputeRates = f->ComputeRates;
  ei.ComputeVariables = f->ComputeVariables;
  
  uint32_t recsize = rateSize * 2 + algSize + 1;
  uint32_t storageCapacity = (VARIABLE_STORAGE_LIMIT / recsize) * recsize;
  double* storage = new double[storageCapacity];
  uint32_t storageExpiry = time(0) + VARIABLE_TIME_LIMIT;
  uint32_t storageSize = 0;

  double voi = mStartBvar;
  double lastVOI = 0.0 /* initialised only to avoid extraneous warning. */;
  bool isFirst = true;

  double minReportForDensity = (mStopBvar - mStartBvar) / mMaxPointDensity;
  uint32_t tabStepNumber = 1;
  double nextStopPoint = mTabulationStepSize + voi;

  if (mTabulationStepSize == 0.0)
    nextStopPoint = mStopBvar;

  if (rateSize != 0)
  {
    while (voi < mStopBvar)
    {
      double bhl = mStopBvar;
      if (bhl - voi > mStepSizeMax)
        bhl = voi + mStepSizeMax;
      if(bhl > nextStopPoint)
        bhl = nextStopPoint;
      
      CVodeSetStopTime(solver, bhl);
      if (CVode(solver, bhl, y, &voi, CV_ONE_STEP) < 0)
      {
        if (!failInfo.failtype)
          setFailure(&failInfo, "CVODE failure", -1);
        break;
      }
      
      if (checkPauseOrCancellation())
        break;
      
      if (isFirst)
        isFirst = false;
      else if (voi - lastVOI < minReportForDensity && !floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance))
        continue;

      if(mStrictTabulation && !floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance))
        continue;

      if (voi==nextStopPoint)
        nextStopPoint = (mTabulationStepSize * ++tabStepNumber) + mStartBvar;

      lastVOI = voi;

      f->    ComputeRates(voi, constants, rates, states, algebraic, &failInfo);
      f->ComputeVariables(voi, constants, rates, states, algebraic, &failInfo);

      // Add to storage...
      storage[storageSize] = voi;
      memcpy(storage + storageSize + 1, states, rateSize * sizeof(double));
      memcpy(storage + storageSize + 1 + rateSize, rates,
             rateSize * sizeof(double));
      memcpy(storage + storageSize + 1 + rateSize * 2, algebraic,
             algSize * sizeof(double));

      storageSize += recsize;

      // Are we ready to send?
      uint32_t timeNow = time(0);
      if (timeNow >= storageExpiry || storageSize == storageCapacity)
      {
        if (mObserver != NULL)
        {
          std::vector<double> resultsVector(storage, storage + storageSize);
          mObserver->results(resultsVector);
        }
        storageExpiry = timeNow + VARIABLE_TIME_LIMIT;
        storageSize = 0;
      }
    }
  }
  if (storageSize != 0 && mObserver != NULL)
  {
    std::vector<double> resultsVector(storage, storage + storageSize);
    mObserver->results(resultsVector);
  }
  if (mObserver != NULL)
  {
    if (failInfo.failtype)
      mObserver->failed(failInfo.failmsg);
    else
      mObserver->done();
  }

  delete [] storage;

  if (rateSize != 0)
  {
    CVodeFree(&solver);
    N_VDestroy(y);
  }
}

#ifdef DEBUG_MODE
#include <fenv.h>
#endif

void
CDA_ODESolverRun::SolveODEProblem
(
 CompiledModelFunctions* f, uint32_t constSize,
 double* constants, uint32_t rateSize, double* rates,
 double* states, uint32_t algSize, double* algebraic
)
{
#ifdef DEBUG_MODE
  feenableexcept(FE_DIVBYZERO);
#endif

  if (mStepType == iface::cellml_services::ADAMS_MOULTON_1_12 ||
      mStepType == iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE)
    SolveODEProblemCVODE(f, constSize, constants, rateSize, rates, states,
                         algSize, algebraic);
  else
#ifdef ENABLE_GSL_INTEGRATORS
    SolveODEProblemGSL(f, constSize, constants, rateSize, rates, states,
                         algSize, algebraic);
#else
    mObserver->failed("GSL integrators are disabled.");
#endif
}

int getFailType(struct fail_info* aFail)
{
  return aFail->failtype;
}

struct PDFInformation
{
  double (*pdf)(double bvar, double* constants, double* algebraic,
                struct fail_info*);
  double* constants, * algebraic, uplimit;
  double target;
  int nroots;
  struct fail_info* failInfo;
  double (**rootFuncs)(double bvar, double* CONSTANTS, double* ALGEBRAIC,
                       struct fail_info*);
  double lowBoundary, highBoundary;
};

#define BUMPFLOATUP(x) (x > 0) ? x*(1+1E-15) : x*(1-1E-15)
#define MAX(x,y) (x>y)?x:y
#define MIN(x,y) (x<y)?x:y

static int integrandForPDF(double t, N_Vector varsV, N_Vector ratesV, void* data)
{
  struct PDFInformation* info = (struct PDFInformation*)data;

  // The first rate is the integrand...
  *N_VGetArrayPointer_Serial(ratesV) =
    info->pdf(t, info->constants, info->algebraic, info->failInfo);

#ifdef DEBUG_UNCERT
  printf("PDF' at %.100g = %.100g\n", t, *N_VGetArrayPointer_Serial(ratesV));
#endif

  return info->failInfo->failtype;
}

static int rootfuncForPDF(double t, N_Vector varsV, double *gout, void* data)
{
  struct PDFInformation* info = (struct PDFInformation*)data;
  double tp1 = 1.0 + t;
  // We transform the integral from a limit between -infinity and uplimit into
  // one between -1 and 0...
  double tprime = info->uplimit + t / tp1;

  for (int i = 0; i < info->nroots; i++)
  {
    gout[i] = info->rootFuncs[i](tprime, info->constants, info->algebraic, info->failInfo);

    if (info->failInfo->failtype)
      return info->failInfo->failtype;
#ifdef DEBUG_UNCERT
    printf("gout[%d] = %g at t = %g, t' = %g\n", i, gout[i], t, tprime);
#endif
  }

  return 0;
}

static void
recordKINSOLError(int code, const char *module,
                  const char *function, char *msg,
                  void *dat)
{
  setFailure(reinterpret_cast<struct fail_info*>(dat), msg, -1);
}

static int minfuncForPDF(N_Vector pVec, N_Vector hxVec, void *adata)
{
  int ret;
  struct PDFInformation* info = (struct PDFInformation*)adata;
  info->uplimit = NV_Ith_S(pVec, 0);
  NV_Ith_S(hxVec, 0) = 0;

  if (!isfinite(info->uplimit))
    return 1;

  void* cv = CVodeCreate(CV_BDF, CV_NEWTON);
  CVodeInit(cv, integrandForPDF, info->lowBoundary, hxVec);
  CVodeSetErrHandlerFn(cv, recordKINSOLError, info->failInfo);
  CVodeSetMaxStep(cv, (info->highBoundary - info->lowBoundary) / 1000);
  CVodeSetMaxNumSteps(cv, 10000);
  CVodeSStolerances(cv, 1E-10, 1E-10);
  CVodeSetUserData(cv, info);
  CVDense(cv, 1);

  if (info->nroots != 0)
    CVodeRootInit(cv, info->nroots, rootfuncForPDF);

  double tret;
#ifdef DEBUG_UNCERT
  printf("Running CVode to integrate between %f and %f\n", info->lowBoundary, MIN(info->highBoundary, info->uplimit));
#endif
  while (true)
  {
    ret = CVode(cv, MIN(info->highBoundary, info->uplimit), hxVec, &tret, CV_NORMAL);
    if (ret != CV_ROOT_RETURN)
      break;

    tret = BUMPFLOATUP(tret); // TODO adjust this based on error tolerance.
#ifdef DEBUG_UNCERT
    printf("Reinitialising at %g\n", tret);
#endif
    CVodeReInit(cv, tret, hxVec);
  }
#ifdef DEBUG_UNCERT
  printf("C.D.F.(%f) = %f, ret = %d\n", NV_Ith_S(pVec, 0), NV_Ith_S(hxVec, 0), ret);
#endif
  CVodeFree(&cv);

  NV_Ith_S(hxVec, 0) -= info->target;

  return (ret < 0) ? 1 : 0;
}

// A table of points that cover a wide range of double values at logarithmic
// spacing, to assist with finding where a p.d.f. is non-zero. For the algorithms
// here to work, the p.d.f. must not have any islands where it is non-zero that don't
// include one of these points.
static double samplePoints[2048] = {
  // sort $ concatMap (\i -> let i2 = i/2.0 in [2**i2, 2**(-i2), -(2**i2), -(2**(-i2))]) [0..511]
-8.187737150746413e76,-5.78960446186581e76,-4.0938685753732067e76,-2.894802230932905e76,-2.0469342876866033e76,-1.4474011154664524e76,-1.0234671438433017e76,-7.237005577332262e75,-5.117335719216508e75,-3.618502788666131e75,
-2.558667859608254e75,-1.8092513943330656e75,-1.279333929804127e75,-9.046256971665328e74,-6.396669649020635e74,-4.523128485832664e74,-3.1983348245103177e74,-2.261564242916332e74,-1.5991674122551589e74,-1.130782121458166e74,
-7.995837061275794e73,-5.65391060729083e73,-3.997918530637897e73,-2.826955303645415e73,-1.9989592653189486e73,-1.4134776518227075e73,-9.994796326594743e72,-7.067388259113537e72,-4.9973981632973714e72,-3.533694129556769e72,
-2.4986990816486857e72,-1.7668470647783843e72,-1.2493495408243429e72,-8.834235323891922e71,-6.246747704121714e71,-4.417117661945961e71,-3.123373852060857e71,-2.2085588309729804e71,-1.5616869260304286e71,-1.1042794154864902e71,
-7.808434630152143e70,-5.521397077432451e70,-3.9042173150760714e70,-2.7606985387162255e70,-1.9521086575380357e70,-1.3803492693581128e70,-9.760543287690179e69,-6.901746346790564e69,-4.880271643845089e69,-3.450873173395282e69,
-2.4401358219225446e69,-1.725436586697641e69,-1.2200679109612723e69,-8.627182933488205e68,-6.100339554806362e68,-4.3135914667441024e68,-3.050169777403181e68,-2.1567957333720512e68,-1.5250848887015904e68,-1.0783978666860256e68,
-7.625424443507952e67,-5.391989333430128e67,-3.812712221753976e67,-2.695994666715064e67,-1.906356110876988e67,-1.347997333357532e67,-9.53178055438494e66,-6.73998666678766e66,-4.76589027719247e66,-3.36999333339383e66,
-2.382945138596235e66,-1.684996666696915e66,-1.1914725692981175e66,-8.424983333484575e65,-5.9573628464905875e65,-4.2124916667422875e65,-2.9786814232452937e65,-2.1062458333711437e65,-1.4893407116226469e65,-1.0531229166855719e65,
-7.446703558113234e64,-5.2656145834278593e64,-3.723351779056617e64,-2.6328072917139297e64,-1.8616758895283086e64,-1.3164036458569648e64,-9.308379447641543e63,-6.582018229284824e63,-4.6541897238207715e63,-3.291009114642412e63,
-2.3270948619103857e63,-1.645504557321206e63,-1.1635474309551929e63,-8.22752278660603e62,-5.817737154775964e62,-4.113761393303015e62,-2.908868577387982e62,-2.0568806966515076e62,-1.454434288693991e62,-1.0284403483257538e62,
-7.272171443469955e61,-5.142201741628769e61,-3.6360857217349777e61,-2.5711008708143844e61,-1.8180428608674889e61,-1.2855504354071922e61,-9.090214304337444e60,-6.427752177035961e60,-4.545107152168722e60,-3.2138760885179806e60,
-2.272553576084361e60,-1.6069380442589903e60,-1.1362767880421805e60,-8.034690221294951e59,-5.681383940210903e59,-4.017345110647476e59,-2.8406919701054513e59,-2.008672555323738e59,-1.4203459850527257e59,-1.004336277661869e59,
-7.101729925263628e58,-5.021681388309345e58,-3.550864962631814e58,-2.5108406941546723e58,-1.775432481315907e58,-1.2554203470773362e58,-8.877162406579535e57,-6.277101735386681e57,-4.438581203289768e57,-3.1385508676933404e57,
-2.219290601644884e57,-1.5692754338466702e57,-1.109645300822442e57,-7.846377169233351e56,-5.54822650411221e56,-3.9231885846166755e56,-2.774113252056105e56,-1.9615942923083377e56,-1.3870566260280524e56,-9.807971461541689e55,
-6.935283130140262e55,-4.9039857307708443e55,-3.467641565070131e55,-2.4519928653854222e55,-1.7338207825350655e55,-1.2259964326927111e55,-8.669103912675328e54,-6.129982163463556e54,-4.334551956337664e54,-3.064991081731778e54,
-2.167275978168832e54,-1.532495540865889e54,-1.083637989084416e54,-7.662477704329444e53,-5.41818994542208e53,-3.831238852164722e53,-2.70909497271104e53,-1.915619426082361e53,-1.35454748635552e53,-9.578097130411805e52,
-6.7727374317776e52,-4.789048565205903e52,-3.3863687158888e52,-2.3945242826029513e52,-1.6931843579444e52,-1.1972621413014757e52,-8.465921789722e51,-5.986310706507379e51,-4.232960894861e51,-2.9931553532536892e51,
-2.1164804474305e51,-1.4965776766268446e51,-1.05824022371525e51,-7.482888383134223e50,-5.29120111857625e50,-3.7414441915671115e50,-2.645600559288125e50,-1.8707220957835557e50,-1.3228002796440624e50,-9.353610478917779e49,
-6.614001398220312e49,-4.6768052394588893e49,-3.307000699110156e49,-2.3384026197294447e49,-1.653500349555078e49,-1.1692013098647223e49,-8.26750174777539e48,-5.846006549323612e48,-4.133750873887695e48,-2.923003274661806e48,
-2.0668754369438476e48,-1.461501637330903e48,-1.0334377184719238e48,-7.307508186654515e47,-5.167188592359619e47,-3.6537540933272573e47,-2.5835942961798094e47,-1.8268770466636286e47,-1.2917971480899047e47,-9.134385233318143e46,
-6.458985740449524e46,-4.567192616659072e46,-3.229492870224762e46,-2.283596308329536e46,-1.614746435112381e46,-1.141798154164768e46,-8.073732175561905e45,-5.70899077082384e45,-4.036866087780952e45,-2.85449538541192e45,
-2.018433043890476e45,-1.42724769270596e45,-1.009216521945238e45,-7.1362384635298e44,-5.04608260972619e44,-3.5681192317649e44,-2.523041304863095e44,-1.78405961588245e44,-1.2615206524315476e44,-8.92029807941225e43,
-6.307603262157738e43,-4.460149039706125e43,-3.153801631078869e43,-2.2300745198530623e43,-1.5769008155394345e43,-1.1150372599265312e43,-7.884504077697172e42,-5.575186299632656e42,-3.942252038848586e42,-2.787593149816328e42,
-1.971126019424293e42,-1.393796574908164e42,-9.855630097121465e41,-6.96898287454082e41,-4.927815048560733e41,-3.48449143727041e41,-2.4639075242803664e41,-1.742245718635205e41,-1.2319537621401832e41,-8.711228593176025e40,
-6.159768810700916e40,-4.3556142965880123e40,-3.079884405350458e40,-2.1778071482940062e40,-1.539942202675229e40,-1.0889035741470031e40,-7.699711013376145e39,-5.444517870735016e39,-3.8498555066880724e39,-2.722258935367508e39,
-1.9249277533440362e39,-1.361129467683754e39,-9.624638766720181e38,-6.80564733841877e38,-4.8123193833600906e38,-3.402823669209385e38,-2.4061596916800453e38,-1.7014118346046923e38,-1.2030798458400226e38,-8.507059173023462e37,
-6.015399229200113e37,-4.253529586511731e37,-3.0076996146000566e37,-2.1267647932558654e37,-1.5038498073000283e37,-1.0633823966279327e37,-7.519249036500141e36,-5.316911983139664e36,-3.759624518250071e36,-2.658455991569832e36,
-1.8798122591250354e36,-1.329227995784916e36,-9.399061295625177e35,-6.64613997892458e35,-4.6995306478125884e35,-3.32306998946229e35,-2.3497653239062942e35,-1.661534994731145e35,-1.1748826619531471e35,-8.307674973655724e34,
-5.874413309765736e34,-4.153837486827862e34,-2.937206654882868e34,-2.076918743413931e34,-1.468603327441434e34,-1.0384593717069655e34,-7.34301663720717e33,-5.192296858534828e33,-3.671508318603585e33,-2.596148429267414e33,
-1.8357541593017924e33,-1.298074214633707e33,-9.178770796508962e32,-6.490371073168535e32,-4.589385398254481e32,-3.2451855365842673e32,-2.2946926991272404e32,-1.6225927682921336e32,-1.1473463495636202e32,-8.112963841460668e31,
-5.736731747818101e31,-4.056481920730334e31,-2.8683658739090506e31,-2.028240960365167e31,-1.4341829369545253e31,-1.0141204801825835e31,-7.170914684772626e30,-5.070602400912918e30,-3.585457342386313e30,-2.535301200456459e30,
-1.7927286711931566e30,-1.2676506002282294e30,-8.963643355965783e29,-6.338253001141147e29,-4.4818216779828915e29,-3.1691265005705735e29,-2.2409108389914457e29,-1.5845632502852868e29,-1.1204554194957229e29,-7.922816251426434e28,-5.602277097478614e28,-3.961408125713217e28,-2.801138548739307e28,-1.9807040628566084e28,-1.4005692743696536e28,-9.903520314283042e27,-7.002846371848268e27,-4.951760157141521e27,-3.501423185924134e27,-2.4758800785707605e27,-1.750711592962067e27,-1.2379400392853803e27,-8.753557964810335e26,-6.189700196426902e26,-4.3767789824051675e26,-3.094850098213451e26,-2.1883894912025837e26,-1.5474250491067253e26,-1.0941947456012919e26,-7.737125245533627e25,-5.470973728006459e25,-3.8685626227668134e25,-2.7354868640032297e25,-1.9342813113834067e25,-1.3677434320016148e25,-9.671406556917033e24,-6.838717160008074e24,-4.835703278458517e24,-3.419358580004037e24,-2.4178516392292583e24,-1.7096792900020185e24,-1.2089258196146292e24,-8.548396450010093e23,-6.044629098073146e23,-4.2741982250050464e23,-3.022314549036573e23,-2.1370991125025232e23,-1.5111572745182865e23,-1.0685495562512616e23,-7.555786372591432e22,-5.342747781256308e22,-3.777893186295716e22,-2.671373890628154e22,-1.888946593147858e22,-1.335686945314077e22,-9.44473296573929e21,-6.678434726570385e21,-4.722366482869645e21,-3.3392173632851925e21,-2.3611832414348226e21,-1.6696086816425962e21,-1.1805916207174113e21,-8.348043408212981e20,-5.902958103587057e20,-4.1740217041064906e20,-2.9514790517935283e20,-2.0870108520532453e20,-1.4757395258967641e20,-1.0435054260266226e20,-7.378697629483821e19,-5.217527130133113e19,-3.6893488147419103e19,-2.6087635650665566e19,-1.8446744073709552e19,-1.3043817825332783e19,-9.223372036854776e18,-6.521908912666392e18,-4.611686018427388e18,-3.260954456333196e18,-2.305843009213694e18,-1.630477228166598e18,-1.152921504606847e18,-8.15238614083299e17,-5.764607523034235e17,-4.076193070416495e17,-2.8823037615171174e17,-2.0380965352082474e17,-1.4411518807585587e17,-1.0190482676041237e17,-7.205759403792794e16,-5.0952413380206184e16,-3.602879701896397e16,-2.5476206690103092e16,-1.8014398509481984e16,-1.2738103345051546e16,-9.007199254740992e15,-6.369051672525773e15,-4.503599627370496e15,-3.1845258362628865e15,-2.251799813685248e15,-1.5922629181314433e15,-1.125899906842624e15,-7.961314590657216e14,-5.62949953421312e14,-3.980657295328608e14,-2.81474976710656e14,-1.990328647664304e14,-1.40737488355328e14,-9.95164323832152e13,-7.0368744177664e13,-4.97582161916076e13,-3.5184372088832e13,-2.48791080958038e13,-1.7592186044416e13,-1.24395540479019e13,-8.796093022208e12,-6.21977702395095e12,-4.398046511104e12,-3.109888511975475e12,-2.199023255552e12,-1.5549442559877375e12,-1.099511627776e12,-7.774721279938688e11,
-5.49755813888e11,-3.887360639969344e11,-2.74877906944e11,-1.943680319984672e11,-1.37438953472e11,-9.71840159992336e10,-6.8719476736e10,-4.85920079996168e10,-3.4359738368e10,-2.42960039998084e10,-1.7179869184e10,-1.21480019999042e10,-8.589934592e9,-6.0740009999521e9,-4.294967296e9,-3.03700049997605e9,-2.147483648e9,-1.518500249988025e9,-1.073741824e9,-7.592501249940125e8,-5.36870912e8,-3.7962506249700624e8,-2.68435456e8,-1.8981253124850312e8,-1.34217728e8,-9.490626562425156e7,-6.7108864e7,-4.745313281212578e7,-3.3554432e7,-2.372656640606289e7,-1.6777216e7,-1.1863283203031445e7,-8388608.0,-5931641.601515722,-4194304.0,-2965820.800757861,-2097152.0,-1482910.4003789306,-1048576.0,-741455.2001894653,-524288.0,
-370727.60009473265,-262144.0,-185363.80004736633,-131072.0,-92681.90002368316,-65536.0,-46340.95001184158,-32768.0,-23170.47500592079,-16384.0,-11585.237502960395,-8192.0,-5792.618751480198,-4096.0,-2896.309375740099,-2048.0,-1448.1546878700494,-1024.0,-724.0773439350247,-512.0,-362.03867196751236,-256.0,-181.01933598375618,-128.0,-90.50966799187809,-64.0,-45.254833995939045,-32.0,-22.627416997969522,-16.0,-11.313708498984761,-8.0,-5.656854249492381,-4.0,-2.8284271247461903,-2.0,-1.4142135623730951,-1.0,-1.0,-0.7071067811865476,-0.5,-0.3535533905932738,-0.25,-0.1767766952966369,-0.125,-8.838834764831845e-2,-6.25e-2,-4.419417382415922e-2,-3.125e-2,-2.209708691207961e-2,-1.5625e-2,-1.1048543456039806e-2,-7.8125e-3,-5.524271728019903e-3,-3.90625e-3,-2.7621358640099515e-3,-1.953125e-3,-1.3810679320049757e-3,-9.765625e-4,-6.905339660024879e-4,-4.8828125e-4,-3.4526698300124393e-4,-2.44140625e-4,-1.7263349150062197e-4,-1.220703125e-4,-8.631674575031098e-5,-6.103515625e-5,-4.315837287515549e-5,-3.0517578125e-5,-2.1579186437577746e-5,-1.52587890625e-5,-1.0789593218788873e-5,-7.62939453125e-6,-5.3947966093944364e-6,-3.814697265625e-6,-2.6973983046972182e-6,-1.9073486328125e-6,-1.3486991523486091e-6,-9.5367431640625e-7,-6.743495761743046e-7,-4.76837158203125e-7,-3.371747880871523e-7,-2.384185791015625e-7,-1.6858739404357614e-7,-1.1920928955078125e-7,-8.429369702178807e-8,-5.960464477539063e-8,-4.2146848510894035e-8,-2.9802322387695313e-8,-2.1073424255447017e-8,-1.4901161193847656e-8,-1.0536712127723509e-8,-7.450580596923828e-9,-5.268356063861754e-9,-3.725290298461914e-9,-2.634178031930877e-9,-1.862645149230957e-9,-1.3170890159654386e-9,-9.313225746154785e-10,-6.585445079827193e-10,-4.656612873077393e-10,-3.2927225399135965e-10,-2.3283064365386963e-10,-1.6463612699567982e-10,-1.1641532182693481e-10,
-8.231806349783991e-11,-5.820766091346741e-11,-4.1159031748919956e-11,-2.9103830456733704e-11,-2.0579515874459978e-11,-1.4551915228366852e-11,-1.0289757937229989e-11,-7.275957614183426e-12,-5.1448789686149945e-12,-3.637978807091713e-12,-2.5724394843074972e-12,-1.8189894035458565e-12,-1.2862197421537486e-12,-9.094947017729282e-13,-6.431098710768743e-13,-4.547473508864641e-13,-3.2155493553843715e-13,-2.2737367544323206e-13,-1.6077746776921858e-13,-1.1368683772161603e-13,
-8.038873388460929e-14,-5.684341886080802e-14,-4.0194366942304644e-14,-2.842170943040401e-14,-2.0097183471152322e-14,-1.4210854715202004e-14,-1.0048591735576161e-14,-7.105427357601002e-15,-5.0242958677880805e-15,-3.552713678800501e-15,-2.5121479338940403e-15,-1.7763568394002505e-15,-1.2560739669470201e-15,-8.881784197001252e-16,-6.280369834735101e-16,-4.440892098500626e-16,-3.1401849173675503e-16,-2.220446049250313e-16,-1.5700924586837752e-16,-1.1102230246251565e-16,-7.850462293418876e-17,-5.551115123125783e-17,-3.925231146709438e-17,
-2.7755575615628914e-17,-1.962615573354719e-17,-1.3877787807814457e-17,-9.813077866773595e-18,-6.938893903907228e-18,-4.9065389333867974e-18,-3.469446951953614e-18,-2.4532694666933987e-18,-1.734723475976807e-18,-1.2266347333466993e-18,-8.673617379884035e-19,-6.133173666733497e-19,-4.336808689942018e-19,-3.0665868333667484e-19,-2.168404344971009e-19,-1.5332934166833742e-19,-1.0842021724855044e-19,-7.666467083416871e-20,-5.421010862427522e-20,-3.8332335417084355e-20,-2.710505431213761e-20,-1.9166167708542177e-20,-1.3552527156068805e-20,-9.583083854271089e-21,-6.776263578034403e-21,
-4.791541927135544e-21,-3.3881317890172014e-21,-2.395770963567772e-21,-1.6940658945086007e-21,-1.197885481783886e-21,-8.470329472543003e-22,-5.98942740891943e-22,-4.235164736271502e-22,-2.994713704459715e-22,-2.117582368135751e-22,-1.4973568522298576e-22,-1.0587911840678754e-22,-7.486784261149288e-23,-5.293955920339377e-23,-3.743392130574644e-23,-2.6469779601696886e-23,-1.871696065287322e-23,-1.3234889800848443e-23,-9.35848032643661e-24,-6.617444900424222e-24,-4.679240163218305e-24,-3.308722450212111e-24,-2.3396200816091525e-24,-1.6543612251060553e-24,-1.1698100408045763e-24,-8.271806125530277e-25,
-5.849050204022881e-25,-4.1359030627651384e-25,-2.9245251020114406e-25,-2.0679515313825692e-25,-1.4622625510057203e-25,-1.0339757656912846e-25,-7.311312755028602e-26,-5.169878828456423e-26,-3.655656377514301e-26,-2.5849394142282115e-26,-1.8278281887571504e-26,-1.2924697071141057e-26,-9.139140943785752e-27,-6.462348535570529e-27,-4.569570471892876e-27,-3.2311742677852644e-27,-2.284785235946438e-27,-1.6155871338926322e-27,-1.142392617973219e-27,-8.077935669463161e-28,-5.711963089866095e-28,-4.0389678347315804e-28,-2.8559815449330475e-28,-2.0194839173657902e-28,-1.4279907724665237e-28,-1.0097419586828951e-28,-7.139953862332619e-29,
-5.048709793414476e-29,-3.5699769311663094e-29,-2.524354896707238e-29,-1.7849884655831547e-29,-1.262177448353619e-29,-8.924942327915773e-30,-6.310887241768095e-30,-4.462471163957887e-30,-3.1554436208840472e-30,-2.2312355819789434e-30,-1.5777218104420236e-30,-1.1156177909894717e-30,-7.888609052210118e-31,-5.578088954947358e-31,-3.944304526105059e-31,-2.789044477473679e-31,-1.9721522630525295e-31,-1.3945222387368396e-31,-9.860761315262648e-32,-6.972611193684198e-32,-4.930380657631324e-32,-3.486305596842099e-32,
-2.465190328815662e-32,-1.7431527984210495e-32,-1.232595164407831e-32,-8.715763992105247e-33,-6.162975822039155e-33,-4.357881996052624e-33,-3.0814879110195774e-33,-2.178940998026312e-33,-1.5407439555097887e-33,-1.089470499013156e-33,-7.703719777548943e-34,-5.44735249506578e-34,-3.851859888774472e-34,-2.72367624753289e-34,-1.925929944387236e-34,-1.361838123766445e-34,-9.62964972193618e-35,-6.809190618832225e-35,-4.81482486096809e-35,
-3.4045953094161123e-35,-2.407412430484045e-35,-1.7022976547080561e-35,-1.2037062152420224e-35,-8.511488273540281e-36,-6.018531076210112e-36,-4.2557441367701404e-36,-3.009265538105056e-36,-2.1278720683850702e-36,-1.504632769052528e-36,-1.0639360341925351e-36,-7.52316384526264e-37,-5.3196801709626755e-37,-3.76158192263132e-37,-2.6598400854813377e-37,-1.88079096131566e-37,-1.3299200427406689e-37,-9.4039548065783e-38,-6.649600213703344e-38,-4.70197740328915e-38,-3.324800106851672e-38,-2.350988701644575e-38,
-1.662400053425836e-38,-1.1754943508222875e-38,-8.31200026712918e-39,-5.877471754111438e-39,-4.15600013356459e-39,-2.938735877055719e-39,-2.078000066782295e-39,-1.4693679385278594e-39,-1.0390000333911476e-39,-7.346839692639297e-40,-5.195000166955738e-40,-3.6734198463196485e-40,-2.597500083477869e-40,-1.8367099231598242e-40,-1.2987500417389344e-40,-9.183549615799121e-41,-6.493750208694672e-41,-4.591774807899561e-41,-3.246875104347336e-41,-2.2958874039497803e-41,-1.623437552173668e-41,-1.1479437019748901e-41,
-8.11718776086834e-42,-5.739718509874451e-42,-4.05859388043417e-42,-2.8698592549372254e-42,-2.029296940217085e-42,-1.4349296274686127e-42,-1.0146484701085425e-42,-7.174648137343064e-43,-5.073242350542713e-43,
-3.587324068671532e-43,-2.5366211752713563e-43,-1.793662034335766e-43,-1.2683105876356782e-43,-8.96831017167883e-44,-6.341552938178391e-44,-4.484155085839415e-44,-3.1707764690891954e-44,-2.2420775429197073e-44,-1.5853882345445977e-44,-1.1210387714598537e-44,-7.926941172722989e-45,-5.605193857299268e-45,-3.963470586361494e-45,-2.802596928649634e-45,-1.981735293180747e-45,-1.401298464324817e-45,-9.908676465903736e-46,-7.006492321624085e-46,-4.954338232951868e-46,-3.503246160812043e-46,-2.477169116475934e-46,
-1.7516230804060213e-46,-1.238584558237967e-46,-8.758115402030107e-47,-6.192922791189835e-47,-4.3790577010150533e-47,-3.0964613955949174e-47,-2.1895288505075267e-47,-1.5482306977974587e-47,-1.0947644252537633e-47,-7.741153488987293e-48,-5.473822126268817e-48,-3.870576744493647e-48,-2.7369110631344083e-48,-1.9352883722468234e-48,-1.3684555315672042e-48,-9.676441861234117e-49,-6.842277657836021e-49,-4.8382209306170584e-49,-3.4211388289180104e-49,-2.4191104653085292e-49,-1.7105694144590052e-49,-1.2095552326542646e-49,
-8.552847072295026e-50,-6.047776163271323e-50,-4.276423536147513e-50,-3.0238880816356615e-50,-2.1382117680737565e-50,-1.5119440408178308e-50,-1.0691058840368783e-50,-7.559720204089154e-51,-5.345529420184391e-51,-3.779860102044577e-51,-2.6727647100921956e-51,-1.8899300510222884e-51,
-1.3363823550460978e-51,-9.449650255111442e-52,-6.681911775230489e-52,-4.724825127555721e-52,-3.3409558876152446e-52,-2.3624125637778606e-52,-1.6704779438076223e-52,-1.1812062818889303e-52,-8.352389719038111e-53,-5.906031409444651e-53,-4.176194859519056e-53,-2.9530157047223257e-53,-2.088097429759528e-53,-1.4765078523611628e-53,-1.044048714879764e-53,-7.382539261805814e-54,-5.22024357439882e-54,-3.691269630902907e-54,-2.61012178719941e-54,-1.8456348154514536e-54,-1.305060893599705e-54,-9.228174077257268e-55,
-6.525304467998525e-55,-4.614087038628634e-55,-3.2626522339992623e-55,-2.307043519314317e-55,-1.6313261169996311e-55,-1.1535217596571585e-55,-8.156630584998156e-56,-5.767608798285792e-56,-4.078315292499078e-56,-2.883804399142896e-56,-2.039157646249539e-56,-1.441902199571448e-56,-1.0195788231247695e-56,-7.20951099785724e-57,-5.0978941156238473e-57,-3.60475549892862e-57,-2.5489470578119236e-57,
-1.80237774946431e-57,-1.2744735289059618e-57,-9.01188874732155e-58,-6.372367644529809e-58,-4.505944373660775e-58,-3.1861838222649046e-58,-2.2529721868303876e-58,-1.5930919111324523e-58,-1.1264860934151938e-58,-7.965459555662261e-59,-5.632430467075969e-59,-3.982729777831131e-59,-2.8162152335379846e-59,-1.9913648889155653e-59,-1.4081076167689923e-59,-9.956824444577827e-60,-7.040538083844961e-60,-4.9784122222889134e-60,-3.5202690419224807e-60,-2.4892061111444567e-60,-1.7601345209612403e-60,-1.2446030555722283e-60,
-8.800672604806202e-61,-6.223015277861142e-61,-4.400336302403101e-61,-3.111507638930571e-61,-2.2001681512015504e-61,-1.5557538194652854e-61,-1.1000840756007752e-61,-7.778769097326427e-62,-5.500420378003876e-62,-3.8893845486632136e-62,-2.750210189001938e-62,-1.9446922743316068e-62,-1.375105094500969e-62,-9.723461371658034e-63,-6.875525472504845e-63,-4.861730685829017e-63,-3.4377627362524226e-63,-2.4308653429145085e-63,-1.7188813681262113e-63,-1.2154326714572542e-63,-8.594406840631056e-64,
-6.077163357286271e-64,-4.297203420315528e-64,-3.0385816786431356e-64,-2.148601710157764e-64,-1.5192908393215678e-64,-1.074300855078882e-64,-7.596454196607839e-65,-5.37150427539441e-65,-3.7982270983039195e-65,-2.685752137697205e-65,-1.8991135491519597e-65,-1.3428760688486026e-65,-9.495567745759799e-66,-6.714380344243013e-66,-4.7477838728798994e-66,-3.3571901721215064e-66,-2.3738919364399497e-66,-1.6785950860607532e-66,-1.1869459682199748e-66,-8.392975430303766e-67,-5.934729841099874e-67,-4.196487715151883e-67,
-2.967364920549937e-67,-2.0982438575759415e-67,-1.4836824602749686e-67,-1.0491219287879708e-67,-7.418412301374843e-68,-5.245609643939854e-68,-3.7092061506874214e-68,-2.622804821969927e-68,-1.8546030753437107e-68,-1.3114024109849634e-68,-9.273015376718553e-69,-6.557012054924817e-69,-4.636507688359277e-69,-3.2785060274624086e-69,-2.3182538441796384e-69,-1.6392530137312043e-69,-1.1591269220898192e-69,
-8.196265068656021e-70,-5.795634610449096e-70,-4.098132534328011e-70,-2.897817305224548e-70,-2.0490662671640054e-70,-1.448908652612274e-70,-1.0245331335820027e-70,-7.24454326306137e-71,-5.1226656679100134e-71,-3.622271631530685e-71,-2.5613328339550067e-71,-1.8111358157653425e-71,-1.2806664169775034e-71,-9.055679078826712e-72,-6.403332084887517e-72,-4.527839539413356e-72,-3.2016660424437584e-72,-2.263919769706678e-72,-1.6008330212218792e-72,-1.131959884853339e-72,-8.004165106109396e-73,-5.659799424266695e-73,
-4.002082553054698e-73,-2.8298997121333476e-73,-2.001041276527349e-73,-1.4149498560666738e-73,-1.0005206382636745e-73,-7.074749280333369e-74,-5.0026031913183725e-74,-3.5373746401666845e-74,-2.5013015956591862e-74,-1.7686873200833423e-74,-1.2506507978295931e-74,-8.843436600416711e-75,-6.253253989147966e-75,-4.421718300208356e-75,-3.126626994573983e-75,-2.210859150104178e-75,-1.5633134972869914e-75,-1.105429575052089e-75,-7.816567486434957e-76,-5.527147875260445e-76,-3.9082837432174785e-76,-2.7635739376302223e-76,
-1.9541418716087393e-76,-1.3817869688151111e-76,-9.770709358043696e-77,-6.908934844075556e-77,-4.885354679021848e-77,-3.454467422037778e-77,
-2.442677339510924e-77,-1.727233711018889e-77,-1.221338669755462e-77,1.221338669755462e-77,1.727233711018889e-77,2.442677339510924e-77,3.454467422037778e-77,4.885354679021848e-77,6.908934844075556e-77,9.770709358043696e-77,1.3817869688151111e-76,1.9541418716087393e-76,2.7635739376302223e-76,3.9082837432174785e-76,5.527147875260445e-76,7.816567486434957e-76,1.105429575052089e-75,1.5633134972869914e-75,2.210859150104178e-75,3.126626994573983e-75,4.421718300208356e-75,6.253253989147966e-75,8.843436600416711e-75,
1.2506507978295931e-74,1.7686873200833423e-74,2.5013015956591862e-74,3.5373746401666845e-74,5.0026031913183725e-74,7.074749280333369e-74,1.0005206382636745e-73,1.4149498560666738e-73,2.001041276527349e-73,2.8298997121333476e-73,4.002082553054698e-73,5.659799424266695e-73,8.004165106109396e-73,1.131959884853339e-72,1.6008330212218792e-72,2.263919769706678e-72,3.2016660424437584e-72,4.527839539413356e-72,6.403332084887517e-72,9.055679078826712e-72,1.2806664169775034e-71,1.8111358157653425e-71,2.5613328339550067e-71,
3.622271631530685e-71,5.1226656679100134e-71,7.24454326306137e-71,1.0245331335820027e-70,1.448908652612274e-70,2.0490662671640054e-70,2.897817305224548e-70,4.098132534328011e-70,5.795634610449096e-70,8.196265068656021e-70,1.1591269220898192e-69,1.6392530137312043e-69,2.3182538441796384e-69,3.2785060274624086e-69,4.636507688359277e-69,6.557012054924817e-69,9.273015376718553e-69,1.3114024109849634e-68,1.8546030753437107e-68,2.622804821969927e-68,3.7092061506874214e-68,5.245609643939854e-68,7.418412301374843e-68,
1.0491219287879708e-67,1.4836824602749686e-67,2.0982438575759415e-67,2.967364920549937e-67,4.196487715151883e-67,5.934729841099874e-67,8.392975430303766e-67,1.1869459682199748e-66,1.6785950860607532e-66,2.3738919364399497e-66,3.3571901721215064e-66,4.7477838728798994e-66,6.714380344243013e-66,9.495567745759799e-66,1.3428760688486026e-65,1.8991135491519597e-65,2.685752137697205e-65,3.7982270983039195e-65,5.37150427539441e-65,7.596454196607839e-65,1.074300855078882e-64,1.5192908393215678e-64,2.148601710157764e-64,
3.0385816786431356e-64,4.297203420315528e-64,6.077163357286271e-64,
8.594406840631056e-64,1.2154326714572542e-63,1.7188813681262113e-63,2.4308653429145085e-63,3.4377627362524226e-63,4.861730685829017e-63,6.875525472504845e-63,9.723461371658034e-63,1.375105094500969e-62,1.9446922743316068e-62,2.750210189001938e-62,3.8893845486632136e-62,5.500420378003876e-62,7.778769097326427e-62,1.1000840756007752e-61,1.5557538194652854e-61,2.2001681512015504e-61,3.111507638930571e-61,4.400336302403101e-61,6.223015277861142e-61,8.800672604806202e-61,1.2446030555722283e-60,1.7601345209612403e-60,
2.4892061111444567e-60,3.5202690419224807e-60,4.9784122222889134e-60,7.040538083844961e-60,9.956824444577827e-60,1.4081076167689923e-59,1.9913648889155653e-59,2.8162152335379846e-59,3.982729777831131e-59,5.632430467075969e-59,7.965459555662261e-59,1.1264860934151938e-58,1.5930919111324523e-58,2.2529721868303876e-58,3.1861838222649046e-58,4.505944373660775e-58,6.372367644529809e-58,9.01188874732155e-58,1.2744735289059618e-57,1.80237774946431e-57,2.5489470578119236e-57,3.60475549892862e-57,5.0978941156238473e-57,
7.20951099785724e-57,1.0195788231247695e-56,
1.441902199571448e-56,2.039157646249539e-56,2.883804399142896e-56,4.078315292499078e-56,5.767608798285792e-56,8.156630584998156e-56,1.1535217596571585e-55,1.6313261169996311e-55,2.307043519314317e-55,3.2626522339992623e-55,4.614087038628634e-55,6.525304467998525e-55,9.228174077257268e-55,1.305060893599705e-54,1.8456348154514536e-54,2.61012178719941e-54,3.691269630902907e-54,5.22024357439882e-54,7.382539261805814e-54,1.044048714879764e-53,1.4765078523611628e-53,2.088097429759528e-53,2.9530157047223257e-53,
4.176194859519056e-53,5.906031409444651e-53,8.352389719038111e-53,1.1812062818889303e-52,1.6704779438076223e-52,2.3624125637778606e-52,3.3409558876152446e-52,4.724825127555721e-52,6.681911775230489e-52,9.449650255111442e-52,1.3363823550460978e-51,1.8899300510222884e-51,2.6727647100921956e-51,3.779860102044577e-51,5.345529420184391e-51,7.559720204089154e-51,1.0691058840368783e-50,1.5119440408178308e-50,2.1382117680737565e-50,3.0238880816356615e-50,4.276423536147513e-50,6.047776163271323e-50,
8.552847072295026e-50,1.2095552326542646e-49,1.7105694144590052e-49,2.4191104653085292e-49,3.4211388289180104e-49,4.8382209306170584e-49,6.842277657836021e-49,9.676441861234117e-49,1.3684555315672042e-48,1.9352883722468234e-48,2.7369110631344083e-48,3.870576744493647e-48,5.473822126268817e-48,7.741153488987293e-48,1.0947644252537633e-47,1.5482306977974587e-47,2.1895288505075267e-47,3.0964613955949174e-47,4.3790577010150533e-47,6.192922791189835e-47,8.758115402030107e-47,1.238584558237967e-46,1.7516230804060213e-46,
2.477169116475934e-46,3.503246160812043e-46,4.954338232951868e-46,7.006492321624085e-46,9.908676465903736e-46,1.401298464324817e-45,1.981735293180747e-45,2.802596928649634e-45,3.963470586361494e-45,5.605193857299268e-45,7.926941172722989e-45,1.1210387714598537e-44,1.5853882345445977e-44,2.2420775429197073e-44,3.1707764690891954e-44,4.484155085839415e-44,6.341552938178391e-44,8.96831017167883e-44,1.2683105876356782e-43,1.793662034335766e-43,2.5366211752713563e-43,3.587324068671532e-43,
5.073242350542713e-43,7.174648137343064e-43,1.0146484701085425e-42,1.4349296274686127e-42,2.029296940217085e-42,2.8698592549372254e-42,4.05859388043417e-42,5.739718509874451e-42,8.11718776086834e-42,1.1479437019748901e-41,1.623437552173668e-41,2.2958874039497803e-41,3.246875104347336e-41,4.591774807899561e-41,6.493750208694672e-41,9.183549615799121e-41,1.2987500417389344e-40,1.8367099231598242e-40,2.597500083477869e-40,3.6734198463196485e-40,5.195000166955738e-40,7.346839692639297e-40,1.0390000333911476e-39,
1.4693679385278594e-39,2.078000066782295e-39,2.938735877055719e-39,4.15600013356459e-39,5.877471754111438e-39,8.31200026712918e-39,1.1754943508222875e-38,1.662400053425836e-38,2.350988701644575e-38,3.324800106851672e-38,4.70197740328915e-38,6.649600213703344e-38,9.4039548065783e-38,1.3299200427406689e-37,1.88079096131566e-37,2.6598400854813377e-37,3.76158192263132e-37,5.3196801709626755e-37,7.52316384526264e-37,1.0639360341925351e-36,1.504632769052528e-36,2.1278720683850702e-36,3.009265538105056e-36,
4.2557441367701404e-36,6.018531076210112e-36,8.511488273540281e-36,
1.2037062152420224e-35,1.7022976547080561e-35,2.407412430484045e-35,3.4045953094161123e-35,4.81482486096809e-35,6.809190618832225e-35,9.62964972193618e-35,1.361838123766445e-34,1.925929944387236e-34,2.72367624753289e-34,3.851859888774472e-34,5.44735249506578e-34,7.703719777548943e-34,1.089470499013156e-33,1.5407439555097887e-33,2.178940998026312e-33,3.0814879110195774e-33,4.357881996052624e-33,6.162975822039155e-33,8.715763992105247e-33,1.232595164407831e-32,1.7431527984210495e-32,2.465190328815662e-32,
3.486305596842099e-32,4.930380657631324e-32,6.972611193684198e-32,9.860761315262648e-32,1.3945222387368396e-31,1.9721522630525295e-31,2.789044477473679e-31,3.944304526105059e-31,5.578088954947358e-31,7.888609052210118e-31,1.1156177909894717e-30,1.5777218104420236e-30,2.2312355819789434e-30,3.1554436208840472e-30,4.462471163957887e-30,6.310887241768095e-30,8.924942327915773e-30,1.262177448353619e-29,1.7849884655831547e-29,2.524354896707238e-29,3.5699769311663094e-29,5.048709793414476e-29,
7.139953862332619e-29,1.0097419586828951e-28,1.4279907724665237e-28,2.0194839173657902e-28,2.8559815449330475e-28,4.0389678347315804e-28,5.711963089866095e-28,8.077935669463161e-28,1.142392617973219e-27,1.6155871338926322e-27,2.284785235946438e-27,3.2311742677852644e-27,4.569570471892876e-27,6.462348535570529e-27,9.139140943785752e-27,1.2924697071141057e-26,1.8278281887571504e-26,2.5849394142282115e-26,3.655656377514301e-26,5.169878828456423e-26,7.311312755028602e-26,1.0339757656912846e-25,1.4622625510057203e-25,
2.0679515313825692e-25,2.9245251020114406e-25,4.1359030627651384e-25,5.849050204022881e-25,8.271806125530277e-25,1.1698100408045763e-24,1.6543612251060553e-24,2.3396200816091525e-24,3.308722450212111e-24,4.679240163218305e-24,6.617444900424222e-24,9.35848032643661e-24,1.3234889800848443e-23,1.871696065287322e-23,2.6469779601696886e-23,3.743392130574644e-23,5.293955920339377e-23,7.486784261149288e-23,1.0587911840678754e-22,1.4973568522298576e-22,2.117582368135751e-22,2.994713704459715e-22,4.235164736271502e-22,
5.98942740891943e-22,8.470329472543003e-22,1.197885481783886e-21,
1.6940658945086007e-21,2.395770963567772e-21,3.3881317890172014e-21,4.791541927135544e-21,6.776263578034403e-21,9.583083854271089e-21,1.3552527156068805e-20,1.9166167708542177e-20,2.710505431213761e-20,3.8332335417084355e-20,5.421010862427522e-20,7.666467083416871e-20,1.0842021724855044e-19,1.5332934166833742e-19,2.168404344971009e-19,3.0665868333667484e-19,4.336808689942018e-19,6.133173666733497e-19,8.673617379884035e-19,1.2266347333466993e-18,1.734723475976807e-18,2.4532694666933987e-18,3.469446951953614e-18,
4.9065389333867974e-18,6.938893903907228e-18,9.813077866773595e-18,1.3877787807814457e-17,1.962615573354719e-17,2.7755575615628914e-17,3.925231146709438e-17,5.551115123125783e-17,7.850462293418876e-17,1.1102230246251565e-16,1.5700924586837752e-16,2.220446049250313e-16,3.1401849173675503e-16,4.440892098500626e-16,6.280369834735101e-16,8.881784197001252e-16,1.2560739669470201e-15,1.7763568394002505e-15,2.5121479338940403e-15,3.552713678800501e-15,5.0242958677880805e-15,7.105427357601002e-15,1.0048591735576161e-14,
1.4210854715202004e-14,2.0097183471152322e-14,2.842170943040401e-14,4.0194366942304644e-14,5.684341886080802e-14,8.038873388460929e-14,1.1368683772161603e-13,1.6077746776921858e-13,2.2737367544323206e-13,3.2155493553843715e-13,4.547473508864641e-13,6.431098710768743e-13,9.094947017729282e-13,1.2862197421537486e-12,1.8189894035458565e-12,2.5724394843074972e-12,3.637978807091713e-12,5.1448789686149945e-12,7.275957614183426e-12,1.0289757937229989e-11,1.4551915228366852e-11,2.0579515874459978e-11,2.9103830456733704e-11,
4.1159031748919956e-11,5.820766091346741e-11,8.231806349783991e-11,1.1641532182693481e-10,1.6463612699567982e-10,2.3283064365386963e-10,3.2927225399135965e-10,4.656612873077393e-10,6.585445079827193e-10,9.313225746154785e-10,1.3170890159654386e-9,1.862645149230957e-9,2.634178031930877e-9,3.725290298461914e-9,5.268356063861754e-9,7.450580596923828e-9,1.0536712127723509e-8,1.4901161193847656e-8,2.1073424255447017e-8,2.9802322387695313e-8,4.2146848510894035e-8,5.960464477539063e-8,8.429369702178807e-8,
1.1920928955078125e-7,1.6858739404357614e-7,
2.384185791015625e-7,3.371747880871523e-7,4.76837158203125e-7,6.743495761743046e-7,9.5367431640625e-7,1.3486991523486091e-6,1.9073486328125e-6,2.6973983046972182e-6,3.814697265625e-6,5.3947966093944364e-6,7.62939453125e-6,1.0789593218788873e-5,1.52587890625e-5,2.1579186437577746e-5,3.0517578125e-5,4.315837287515549e-5,6.103515625e-5,8.631674575031098e-5,1.220703125e-4,1.7263349150062197e-4,2.44140625e-4,3.4526698300124393e-4,4.8828125e-4,6.905339660024879e-4,9.765625e-4,1.3810679320049757e-3,1.953125e-3,
2.7621358640099515e-3,3.90625e-3,5.524271728019903e-3,7.8125e-3,1.1048543456039806e-2,1.5625e-2,2.209708691207961e-2,3.125e-2,4.419417382415922e-2,6.25e-2,8.838834764831845e-2,0.125,0.1767766952966369,0.25,0.3535533905932738,0.5,0.7071067811865476,1.0,1.0,1.4142135623730951,2.0,2.8284271247461903,4.0,5.656854249492381,8.0,11.313708498984761,16.0,22.627416997969522,32.0,45.254833995939045,64.0,90.50966799187809,128.0,181.01933598375618,256.0,362.03867196751236,512.0,724.0773439350247,
1024.0,1448.1546878700494,2048.0,2896.309375740099,4096.0,5792.618751480198,8192.0,11585.237502960395,16384.0,23170.47500592079,32768.0,46340.95001184158,65536.0,92681.90002368316,131072.0,185363.80004736633,262144.0,370727.60009473265,524288.0,741455.2001894653,1048576.0,1482910.4003789306,2097152.0,2965820.800757861,4194304.0,5931641.601515722,8388608.0,1.1863283203031445e7,1.6777216e7,2.372656640606289e7,3.3554432e7,4.745313281212578e7,6.7108864e7,9.490626562425156e7,1.34217728e8,1.8981253124850312e8,
2.68435456e8,3.7962506249700624e8,5.36870912e8,7.592501249940125e8,1.073741824e9,1.518500249988025e9,2.147483648e9,3.03700049997605e9,4.294967296e9,6.0740009999521e9,8.589934592e9,1.21480019999042e10,1.7179869184e10,2.42960039998084e10,3.4359738368e10,4.85920079996168e10,6.8719476736e10,9.71840159992336e10,1.37438953472e11,1.943680319984672e11,2.74877906944e11,3.887360639969344e11,5.49755813888e11,7.774721279938688e11,
1.099511627776e12,1.5549442559877375e12,2.199023255552e12,3.109888511975475e12,4.398046511104e12,6.21977702395095e12,8.796093022208e12,1.24395540479019e13,1.7592186044416e13,2.48791080958038e13,3.5184372088832e13,4.97582161916076e13,7.0368744177664e13,9.95164323832152e13,1.40737488355328e14,1.990328647664304e14,2.81474976710656e14,3.980657295328608e14,5.62949953421312e14,7.961314590657216e14,1.125899906842624e15,1.5922629181314433e15,2.251799813685248e15,3.1845258362628865e15,4.503599627370496e15,
6.369051672525773e15,9.007199254740992e15,1.2738103345051546e16,1.8014398509481984e16,2.5476206690103092e16,3.602879701896397e16,5.0952413380206184e16,7.205759403792794e16,1.0190482676041237e17,1.4411518807585587e17,2.0380965352082474e17,2.8823037615171174e17,4.076193070416495e17,5.764607523034235e17,8.15238614083299e17,1.152921504606847e18,1.630477228166598e18,2.305843009213694e18,3.260954456333196e18,4.611686018427388e18,6.521908912666392e18,9.223372036854776e18,1.3043817825332783e19,1.8446744073709552e19,
2.6087635650665566e19,3.6893488147419103e19,5.217527130133113e19,7.378697629483821e19,1.0435054260266226e20,1.4757395258967641e20,2.0870108520532453e20,2.9514790517935283e20,4.1740217041064906e20,5.902958103587057e20,8.348043408212981e20,1.1805916207174113e21,1.6696086816425962e21,2.3611832414348226e21,3.3392173632851925e21,4.722366482869645e21,6.678434726570385e21,9.44473296573929e21,1.335686945314077e22,1.888946593147858e22,2.671373890628154e22,3.777893186295716e22,5.342747781256308e22,7.555786372591432e22,
1.0685495562512616e23,1.5111572745182865e23,2.1370991125025232e23,3.022314549036573e23,4.2741982250050464e23,6.044629098073146e23,8.548396450010093e23,1.2089258196146292e24,1.7096792900020185e24,2.4178516392292583e24,3.419358580004037e24,4.835703278458517e24,6.838717160008074e24,9.671406556917033e24,1.3677434320016148e25,1.9342813113834067e25,2.7354868640032297e25,3.8685626227668134e25,5.470973728006459e25,7.737125245533627e25,1.0941947456012919e26,1.5474250491067253e26,
2.1883894912025837e26,3.094850098213451e26,4.3767789824051675e26,6.189700196426902e26,8.753557964810335e26,1.2379400392853803e27,1.750711592962067e27,2.4758800785707605e27,3.501423185924134e27,4.951760157141521e27,7.002846371848268e27,9.903520314283042e27,1.4005692743696536e28,1.9807040628566084e28,2.801138548739307e28,3.961408125713217e28,5.602277097478614e28,7.922816251426434e28,1.1204554194957229e29,1.5845632502852868e29,2.2409108389914457e29,3.1691265005705735e29,4.4818216779828915e29,6.338253001141147e29,
8.963643355965783e29,1.2676506002282294e30,1.7927286711931566e30,2.535301200456459e30,3.585457342386313e30,5.070602400912918e30,7.170914684772626e30,1.0141204801825835e31,1.4341829369545253e31,2.028240960365167e31,2.8683658739090506e31,4.056481920730334e31,5.736731747818101e31,8.112963841460668e31,1.1473463495636202e32,1.6225927682921336e32,2.2946926991272404e32,3.2451855365842673e32,4.589385398254481e32,6.490371073168535e32,9.178770796508962e32,1.298074214633707e33,1.8357541593017924e33,2.596148429267414e33,
3.671508318603585e33,5.192296858534828e33,7.34301663720717e33,1.0384593717069655e34,
1.468603327441434e34,2.076918743413931e34,2.937206654882868e34,4.153837486827862e34,5.874413309765736e34,8.307674973655724e34,1.1748826619531471e35,1.661534994731145e35,2.3497653239062942e35,3.32306998946229e35,4.6995306478125884e35,6.64613997892458e35,9.399061295625177e35,1.329227995784916e36,1.8798122591250354e36,2.658455991569832e36,3.759624518250071e36,5.316911983139664e36,7.519249036500141e36,1.0633823966279327e37,1.5038498073000283e37,2.1267647932558654e37,3.0076996146000566e37,4.253529586511731e37,
6.015399229200113e37,8.507059173023462e37,1.2030798458400226e38,1.7014118346046923e38,2.4061596916800453e38,3.402823669209385e38,4.8123193833600906e38,6.80564733841877e38,9.624638766720181e38,1.361129467683754e39,1.9249277533440362e39,2.722258935367508e39,3.8498555066880724e39,5.444517870735016e39,7.699711013376145e39,1.0889035741470031e40,1.539942202675229e40,2.1778071482940062e40,3.079884405350458e40,4.3556142965880123e40,6.159768810700916e40,8.711228593176025e40,1.2319537621401832e41,1.742245718635205e41,
2.4639075242803664e41,3.48449143727041e41,4.927815048560733e41,6.96898287454082e41,9.855630097121465e41,1.393796574908164e42,1.971126019424293e42,2.787593149816328e42,3.942252038848586e42,5.575186299632656e42,7.884504077697172e42,1.1150372599265312e43,1.5769008155394345e43,2.2300745198530623e43,3.153801631078869e43,4.460149039706125e43,6.307603262157738e43,8.92029807941225e43,1.2615206524315476e44,1.78405961588245e44,2.523041304863095e44,3.5681192317649e44,5.04608260972619e44,7.1362384635298e44,1.009216521945238e45,
1.42724769270596e45,2.018433043890476e45,2.85449538541192e45,4.036866087780952e45,5.70899077082384e45,8.073732175561905e45,1.141798154164768e46,
1.614746435112381e46,2.283596308329536e46,3.229492870224762e46,4.567192616659072e46,6.458985740449524e46,9.134385233318143e46,1.2917971480899047e47,1.8268770466636286e47,2.5835942961798094e47,3.6537540933272573e47,5.167188592359619e47,7.307508186654515e47,1.0334377184719238e48,1.461501637330903e48,2.0668754369438476e48,2.923003274661806e48,4.133750873887695e48,5.846006549323612e48,8.26750174777539e48,1.1692013098647223e49,1.653500349555078e49,2.3384026197294447e49,3.307000699110156e49,4.6768052394588893e49,
6.614001398220312e49,9.353610478917779e49,1.3228002796440624e50,1.8707220957835557e50,2.645600559288125e50,3.7414441915671115e50,5.29120111857625e50,7.482888383134223e50,1.05824022371525e51,1.4965776766268446e51,2.1164804474305e51,2.9931553532536892e51,
4.232960894861e51,5.986310706507379e51,8.465921789722e51,1.1972621413014757e52,1.6931843579444e52,2.3945242826029513e52,3.3863687158888e52,4.789048565205903e52,6.7727374317776e52,9.578097130411805e52,1.35454748635552e53,1.915619426082361e53,2.70909497271104e53,3.831238852164722e53,5.41818994542208e53,7.662477704329444e53,1.083637989084416e54,1.532495540865889e54,2.167275978168832e54,3.064991081731778e54,4.334551956337664e54,6.129982163463556e54,8.669103912675328e54,1.2259964326927111e55,1.7338207825350655e55,
2.4519928653854222e55,3.467641565070131e55,4.9039857307708443e55,6.935283130140262e55,9.807971461541689e55,1.3870566260280524e56,1.9615942923083377e56,
2.774113252056105e56,3.9231885846166755e56,5.54822650411221e56,7.846377169233351e56,1.109645300822442e57,1.5692754338466702e57,2.219290601644884e57,3.1385508676933404e57,4.438581203289768e57,6.277101735386681e57,8.877162406579535e57,1.2554203470773362e58,1.775432481315907e58,2.5108406941546723e58,3.550864962631814e58,5.021681388309345e58,7.101729925263628e58,1.004336277661869e59,1.4203459850527257e59,2.008672555323738e59,2.8406919701054513e59,4.017345110647476e59,5.681383940210903e59,8.034690221294951e59,
1.1362767880421805e60,1.6069380442589903e60,2.272553576084361e60,3.2138760885179806e60,4.545107152168722e60,6.427752177035961e60,9.090214304337444e60,1.2855504354071922e61,1.8180428608674889e61,2.5711008708143844e61,3.6360857217349777e61,5.142201741628769e61,7.272171443469955e61,1.0284403483257538e62,1.454434288693991e62,2.0568806966515076e62,2.908868577387982e62,4.113761393303015e62,5.817737154775964e62,8.22752278660603e62,1.1635474309551929e63,1.645504557321206e63,2.3270948619103857e63,3.291009114642412e63,
4.6541897238207715e63,6.582018229284824e63,9.308379447641543e63,1.3164036458569648e64,1.8616758895283086e64,2.6328072917139297e64,3.723351779056617e64,5.2656145834278593e64,7.446703558113234e64,1.0531229166855719e65,1.4893407116226469e65,2.1062458333711437e65,2.9786814232452937e65,4.2124916667422875e65,5.9573628464905875e65,8.424983333484575e65,1.1914725692981175e66,1.684996666696915e66,2.382945138596235e66,3.36999333339383e66,4.76589027719247e66,6.73998666678766e66,9.53178055438494e66,1.347997333357532e67,
1.906356110876988e67,2.695994666715064e67,3.812712221753976e67,5.391989333430128e67,7.625424443507952e67,1.0783978666860256e68,1.5250848887015904e68,2.1567957333720512e68,3.050169777403181e68,4.3135914667441024e68,6.100339554806362e68,8.627182933488205e68,1.2200679109612723e69,1.725436586697641e69,2.4401358219225446e69,3.450873173395282e69,4.880271643845089e69,6.901746346790564e69,9.760543287690179e69,1.3803492693581128e70,1.9521086575380357e70,2.7606985387162255e70,3.9042173150760714e70,5.521397077432451e70,
7.808434630152143e70,1.1042794154864902e71,1.5616869260304286e71,2.2085588309729804e71,3.123373852060857e71,4.417117661945961e71,6.246747704121714e71,8.834235323891922e71,1.2493495408243429e72,1.7668470647783843e72,2.4986990816486857e72,3.533694129556769e72,4.9973981632973714e72,7.067388259113537e72,9.994796326594743e72,1.4134776518227075e73,1.9989592653189486e73,2.826955303645415e73,3.997918530637897e73,5.65391060729083e73,7.995837061275794e73,1.130782121458166e74,1.5991674122551589e74,2.261564242916332e74,
3.1983348245103177e74,4.523128485832664e74,6.396669649020635e74,9.046256971665328e74,1.279333929804127e75,1.8092513943330656e75,2.558667859608254e75,3.618502788666131e75,5.117335719216508e75,7.237005577332262e75,1.0234671438433017e76,1.4474011154664524e76,2.0469342876866033e76,2.894802230932905e76,4.0938685753732067e76,5.78960446186581e76,8.187737150746413e76
};

double
SampleUsingPDF(double (*pdf)(double bvar, double* CONSTANTS, double* ALGEBRAIC, struct fail_info*),
               int nroots,
               double (**rootFuncs)(double bvar, double* CONSTANTS, double* ALGEBRAIC, struct fail_info*),
               double* CONSTANTS, double* ALGEBRAIC,
               struct fail_info* failInfo)
{
  struct PDFInformation pdfi;
  pdfi.constants = CONSTANTS;
  pdfi.algebraic = ALGEBRAIC;
  pdfi.pdf = pdf;
  pdfi.target = (rand() + 0.0) / RAND_MAX;
  pdfi.nroots = nroots;
  pdfi.rootFuncs = rootFuncs;
  double p;

  // We assume that the p.d.f. is zero except in a finite range. Find that range:
  int lowestNonZero = 2048, highestNonZero = -1;
  for (int attempt = 0; attempt < 2048; attempt++)
  {
    clearFailure(failInfo);
    if (pdfi.pdf(samplePoints[attempt], CONSTANTS, ALGEBRAIC, failInfo) >= 1E-100)
    {
      lowestNonZero = attempt;
      break;
    }
  }
  if (failInfo->failtype)
    return strtod("NAN", NULL);

  if (lowestNonZero == 2048)
  {
    setFailure(failInfo, "Could not find any point where p.d.f. is non-zero.", -1);
    return strtod("NAN", NULL); // Zero everywhere...
  }
  if (lowestNonZero == 0)
  {
    setFailure(failInfo, "The p.d.f. value is non-negligible at a range too low for the solver to handle.", -1);
    return strtod("NAN", NULL); // We didn't find a number below which it is zero.
  }

  for (int attempt = 2047; attempt >= 0; attempt--)
  {
    if (pdfi.pdf(samplePoints[attempt], CONSTANTS, ALGEBRAIC, failInfo) >= 1E-100)
    {
      clearFailure(failInfo);
      highestNonZero = attempt;
      break;
    }
  }
  if (failInfo->failtype)
    return strtod("NAN", NULL);
  if (highestNonZero == 2047)
  {
    setFailure(failInfo, "The p.d.f. value is non-negligible at a range too high for the solver to handle.", -1);
    return strtod("NAN", NULL); // We didn't find a number above which it is zero.
  }

  double lowlim = samplePoints[lowestNonZero - 1], uplim = samplePoints[lowestNonZero];
  for (p = (lowlim + uplim) / 2.0; (uplim - lowlim) / (MAX(1E-6, MAX(fabs(uplim), fabs(lowlim)))) > 1E-6;
       p = (lowlim + uplim) / 2.0)
  {
    if (pdfi.pdf(p, CONSTANTS, ALGEBRAIC, failInfo) >= 1E-100)
      uplim = p;
    else
      lowlim = p;
  }
  pdfi.lowBoundary = p;
#ifdef DEBUG_UNCERT
  printf("p.d.f low zero boundary in (%g,%g): %g\n", lowlim, uplim, p);
#endif

  lowlim = samplePoints[highestNonZero], uplim = samplePoints[highestNonZero + 1];
  for (p = (lowlim + uplim) / 2.0; (uplim - lowlim) / (MAX(1E-6, MAX(fabs(uplim), fabs(lowlim)))) > 1E-6;
       p = (lowlim + uplim) / 2.0)
  {
    if (pdfi.pdf(p, CONSTANTS, ALGEBRAIC, failInfo) >= 1E-100)
      lowlim = p;
    else
      uplim = p;
  }
  pdfi.highBoundary = p;
#ifdef DEBUG_UNCERT
  printf("p.d.f high zero boundary in (%g,%g): %g\n", lowlim, uplim, p);
#endif

  lowlim = pdfi.lowBoundary;
  uplim = pdfi.highBoundary;

  double hx;
  N_Vector hxVec = N_VMake_Serial(1, &hx);
  N_Vector pVec = N_VMake_Serial(1, &p);

  for (p = (lowlim + uplim) / 2.0; (uplim - lowlim) / (MAX(1E-6, MAX(fabs(uplim), fabs(lowlim)))) > 1E-6;
       p = (lowlim + uplim) / 2.0)
  {
#ifdef DEBUG_UNCERT
    printf("Trying p = %g (in (%g, %g)): ", p, lowlim, uplim);
#endif
    if (minfuncForPDF(pVec, hxVec, (void*)&pdfi))
    {
#ifdef DEBUG_UNCERT
      printf("minfunc failed!\n");
#endif
      p = strtod("NAN", NULL);
      failAddCause(failInfo, "Error finding valid range for p.d.f.");
      break;
    }

#ifdef DEBUG_UNCERT
    printf("hx = %g\n", hx);
#endif

    if (fabs(hx) < 1E-6)
      break;

    if (hx < 0)
      lowlim = p;
    else
      uplim = p;
  }

  N_VDestroy(hxVec);
  N_VDestroy(pVec);

  return p;
}

struct DefintInformation
{
  double voi;
  double* constants, * rates, * algebraic, * states, * var;
  double (*f)(double VOI,double *C,double *R,double *S,double *A, struct fail_info*);
  struct fail_info* failInfo;
};

struct DefintDebugInformation
{
  double voi;
  double* constants, * rates, * algebraic, * states, * var;
  EDouble (*f)(double VOI,double *C,double *R,double *S,double *A, struct fail_info*);
  struct fail_info* failInfo;
};

struct DAEEvaluationInformation
{
  double* constants, * rates, * oldrates, * algebraic, * states, * oldstates, * condvars;
  uint32_t condVarSize;
  void (*ComputeResiduals)(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES,
                          double* STATES, double* OLDSTATES, double* ALGEBRAIC,
                           double* CONDVAR, double* resids, struct fail_info* failInfo);
  void (*ComputeRootInformation)(double VOI, double* CONSTANTS, double* RATES,
                                double* OLDRATES, double* STATES,
                                double* OLDSTATES, double* ALGEBRAIC,
                                double* CONDVAR, struct fail_info* failInfo);
  void (*EvaluateEssentialVariables)(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES,
                                     double* STATES, double* OLDSTATES, double* ALGEBRAIC, double* CONDVAR,
                                     struct fail_info* failInfo);
  void (*EvaluateVariables)(double VOI, double* CONSTANTS, double* RATES,
                            double* STATES, double* ALGEBRAIC, double* CONDVAR,
                            struct fail_info* failInfo);
  struct fail_info* failInfo;

  ~DAEEvaluationInformation()
  {
  }
};

static int
ida_resfn(double t, N_Vector yy, N_Vector yp, N_Vector resval, void* userdata)
{
  DAEEvaluationInformation * d = reinterpret_cast<DAEEvaluationInformation*>(userdata);
  double *states = N_VGetArrayPointer(yy);
  double * rates = N_VGetArrayPointer(yp);
  double * resids = N_VGetArrayPointer(resval);
  d->EvaluateEssentialVariables(t, d->constants, rates, d->oldrates, states,
                                d->oldstates, d->algebraic, d->condvars, d->failInfo);
  if (d->failInfo->failtype != 0)
    return d->failInfo->failtype;

  d->ComputeResiduals(t, d->constants, rates, d->oldrates,
                      states, d->oldstates,
                      d->algebraic, d->condvars, resids, d->failInfo);
  if (d->failInfo->failtype == 0)
  {
    for (unsigned int i = 0; i < NV_LENGTH_S(resval); i++)
      if (!(resids[i] < 1E200)) // Clip the data to deal with NaN / inf / -inf.
        resids[i] = 1E200;
      else if (!(resids[i] > -1E200))
        resids[i] = -1E200;
  }
  return d->failInfo->failtype;
}

static int
ida_rootfn(double t, N_Vector y, N_Vector yp, double *gout, void *userdata)
{
  DAEEvaluationInformation * d = reinterpret_cast<DAEEvaluationInformation*>(userdata);
  double *states = N_VGetArrayPointer(y);
  double *rates = N_VGetArrayPointer(y);
  d->ComputeRootInformation(t, d->constants, rates, d->oldrates, states, d->oldstates,
                            d->algebraic, gout, d->failInfo);

  return d->failInfo->failtype;
}

static void
RatesStatesICInfoToParameters
(
 size_t sz, double * rates, double * states, double * icinfo, double * params
)
{
  for (size_t i = 0; i < sz; i++)
    params[i] = (icinfo[i] == 0.0) ? states[i] : rates[i];
}

static void
MergeParametersICInfoIntoRatesStates
(
 size_t sz, double * rates, double * states, double * icinfo, double * params
)
{
  for (size_t i = 0; i < sz; i++)
    if (icinfo[i] == 0.0)
      states[i] = params[i];
    else
      rates[i] = params[i];
}

struct DAEIVFindingInformation
{
  size_t n;
  double voi0;
  double* constants, * rates, * oldrates, * algebraic, * states, * oldstates, * condvars, * hxtmp, * icinfo;
  void (*ComputeResiduals)(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES,
                          double* STATES, double* OLDSTATES,
                          double* ALGEBRAIC, double* CONDOUT, double* resids, struct fail_info*);
  void (*ComputeRootInformation)(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES,
                                double* STATES, double* OLDSTATES,
                                double* ALGEBRAIC, double* CONDOUT, struct fail_info*);
  void (*EvaluateEssentialVariables)(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES,
                                    double* STATES, double* OLDSTATES, double* ALGEBRAIC,
                                    double* CONDVAR, struct fail_info*);
  struct fail_info* failInfo;
};

static void DetermineRateOrStateSensitivity(double * hx, int n, DAEIVFindingInformation* info)
{
  // printf("condvars[0] = %g\n", info->condvars[0]);
  for (int i = 0; i < n; i++)
  {
    double oldrate = info->rates[i];
    info->EvaluateEssentialVariables(info->voi0, info->constants, info->rates, info->oldrates, info->states,
                                     info->oldstates, info->algebraic, info->condvars, info->failInfo);
    info->ComputeResiduals(info->voi0, info->constants, info->rates, info->oldrates,
                           info->states, info->oldstates,
                           info->algebraic, info->condvars, hx, info->failInfo);
    info->rates[i] = oldrate * 1000 + 1;
    info->EvaluateEssentialVariables(info->voi0, info->constants, info->rates, info->oldrates, info->states,
                                     info->oldstates, info->algebraic, info->condvars,
                                     info->failInfo);
    info->ComputeRootInformation(info->voi0, info->constants, info->rates, info->oldrates,
                                 info->states, info->oldstates,
                                 info->algebraic, info->condvars, info->failInfo);
    info->ComputeResiduals(info->voi0, info->constants, info->rates, info->oldrates,
                           info->states, info->oldstates,
                           info->algebraic, info->condvars, info->hxtmp,
                           info->failInfo);
    double gap1 = 0;
    if (info->failInfo->failtype != 0)
    {
      gap1 = 1.0;
      clearFailure(info->failInfo);
    }
    else
    {
      for (int j = 0; j < n; j++)
      {
      // printf("Rate %u: hx[%u] = %g, hxtmp[%u] = %g, diff = %g\n", i, j, hx[j], j, info->hxtmp[j], hx[j] - info->hxtmp[j]);
        gap1 += fabs(hx[j] - info->hxtmp[j]);
      }
    }
    info->icinfo[i] = gap1 > 0;
    info->rates[i] = oldrate;

#ifdef CONSIDER_BOTH_SENSITIVITIES

    double oldstate = info->states[i];
    info->EvaluateEssentialVariables(info->voi0, info->constants, info->rates,
                                     info->oldrates, info->states, info->oldstates,
                                     info->algebraic);
    info->ComputeResiduals(info->voi0, info->constants, info->rates, info->oldrates,
                           info->states, info->oldstates,
                           info->algebraic, info->condvars, hx);
    info->states[i] = oldstate * (1 + 1E-10);
    info->EvaluateEssentialVariables(info->voi0, info->constants, info->rates, info->states,
                                     info->algebraic);
    info->ComputeResiduals(info->voi0, info->constants, info->rates, info->oldrates,
                           info->states, info->oldstates,
                           info->algebraic, info->condvars, info->hxtmp);

    double gap2 = 0;
    for (int j = 0; j < n; j++)
    {
      // printf("State: hx[%u] = %g, hxtmp[%u] = %g\n", j, hx[j], j, info->hxtmp[j]);
      gap2 += fabs(hx[j] - info->hxtmp[j]);
    }

    // printf("gap1 = %g, gap2 = %g\n", gap1, gap2);

    info->icinfo[i] = (gap1 > 0 || gap2 <= 0.0);
    info->states[i] = oldstate;
#endif
  }
}

static int dae_iv_paramfinder(N_Vector p, N_Vector hx, void *adata)
{
  DAEIVFindingInformation * info = reinterpret_cast<DAEIVFindingInformation*>(adata);

  MergeParametersICInfoIntoRatesStates(info->n, info->rates, info->states,
                                       info->icinfo, NV_DATA_S(p));

  info->EvaluateEssentialVariables(info->voi0, info->constants, info->rates, info->oldrates,
                                   info->states, info->oldstates, info->algebraic,
                                   info->condvars, info->failInfo);
  info->ComputeResiduals(info->voi0, info->constants, info->rates, info->oldrates,
                         info->states, info->oldstates,
                         info->algebraic, info->condvars, NV_DATA_S(hx),
                         info->failInfo);
  for (unsigned int i = 0; i < info->n; i++)
  {
    for (unsigned int i = 0; i < NV_LENGTH_S(hx); i++)
      if (!(NV_Ith_S(hx, i) < 1E200)) // Clip the data to deal with NaN / inf / -inf.
        NV_Ith_S(hx, i) = 1E200;
      else if (!(NV_Ith_S(hx, i) > -1E200))
        NV_Ith_S(hx, i) = -1E200;
  }
  return 0;
}

void
CDA_DAESolverRun::SolveDAEProblem
(
 IDACompiledModelFunctions* f, uint32_t constSize,
 double* constants, uint32_t rateSize, double* rates,
 uint32_t stateSize, double* states, uint32_t algSize,
 double* algebraic, uint32_t condVarSize, double* condvars
)
{
  struct fail_info failInfo;
  double* icinfo = new double[stateSize];
  N_Vector params = N_VNew_Serial(stateSize);
  N_Vector ones = N_VNew_Serial(stateSize);
  N_VConst(1.0, ones);
  bool isFirst = true;

  void* idamem = IDACreate();
  double voi = mStartBvar;

  uint32_t recsize = rateSize * 2 + algSize + 1;
  uint32_t storageCapacity = (VARIABLE_STORAGE_LIMIT / recsize) * recsize;
  double* storage = new double[storageCapacity];
  uint32_t storageExpiry = time(0) + VARIABLE_TIME_LIMIT;
  uint32_t storageSize = 0;
  N_Vector y0 = NULL, dy0 = NULL;

  if (rateSize != 0)
  {
    y0 = N_VMake_Serial(stateSize, states);
    dy0 = N_VMake_Serial(rateSize, rates);
  }

  DAEEvaluationInformation ei;

  ei.failInfo = &failInfo;
  ei.constants = constants;
  ei.states = states;
  ei.rates = rates;
  ei.algebraic = algebraic;
  ei.condvars = condvars;
  ei.ComputeResiduals = f->ComputeResiduals;
  ei.ComputeRootInformation = f->ComputeRootInformation;
  ei.EvaluateEssentialVariables = f->EvaluateEssentialVariables;
  ei.EvaluateVariables = f->EvaluateVariables;
  ei.oldrates = new double[rateSize];
  ei.oldstates = new double[stateSize];
  ei.condVarSize = condVarSize;
  memcpy(ei.oldrates, rates, rateSize * sizeof(double));
  memcpy(ei.oldstates, states, stateSize * sizeof(double));

  DAEIVFindingInformation ivf;
  ivf.hxtmp = new double[stateSize];
  ivf.n = stateSize;
  ivf.voi0 = voi;
  ivf.constants = constants;
  ivf.rates = rates;
  ivf.algebraic = algebraic;
  ivf.states = states;
  ivf.condvars = condvars;
  ivf.icinfo = icinfo;
  ivf.failInfo = &failInfo;
  ivf.ComputeResiduals = f->ComputeResiduals;
  ivf.EvaluateEssentialVariables = f->EvaluateEssentialVariables;
  ivf.ComputeRootInformation = f->ComputeRootInformation;
  ivf.oldrates = ei.oldrates;
  ivf.oldstates = ei.oldstates;
  double * hx = new double[stateSize];
  int * roots = new int[condVarSize];
  memset(roots, 0, sizeof(int) * condVarSize);

  void* kin_mem = KINCreate();
  KINInit(kin_mem, dae_iv_paramfinder, params);
  KINSpgmr(kin_mem, 0);
  KINSetErrHandlerFn(kin_mem, cda_ida_error_handler, &failInfo);
  KINSetUserData(kin_mem, &ivf);
  
  f->SetupStateInfo(icinfo);

  double lastVOI = 0.0 /* initialised only to avoid extraneous warning. */;
  
  double minReportForDensity = (mStopBvar - mStartBvar) / mMaxPointDensity;
  uint32_t tabStepNumber = 0;
  double nextStopPoint = mTabulationStepSize == 0.0 ? mStopBvar : voi;

  if (rateSize > 0)
  {
    bool restart = true;
    while (restart)
    {
      restart = false;
      
      f->ComputeRootInformation(voi, constants, rates, ei.oldrates, states, ei.oldstates,
                                algebraic, condvars, &failInfo);
      for (uint32_t i = 0; i < condVarSize; i++)
      {
        if (condvars[i] == 0 && roots[i] != 0)
          condvars[i] = roots[i] * 1E-100;
      }
      DetermineRateOrStateSensitivity(hx, stateSize, &ivf);
      // printf("Just determined sensitivity array:\n");
      // for (uint32_t i = 0; i < stateSize; i++)
      //   printf("  sens[%u] = %g\n", i, icinfo[i]);
      RatesStatesICInfoToParameters(stateSize, rates, states, icinfo, NV_DATA_S(params));

      int ret = KINSol(kin_mem, params, KIN_NONE, ones, ones);
      if (ret < 0)
      {
        failAddCause(&failInfo, "Failure solving for initial values");
        // Apparently IDAFree after IDACreate but before IDAInit fails, so lets IDAInit...
        IDAInit(idamem, ida_resfn, /* t0 = */voi, y0, dy0);
        break;
      }
        
      MergeParametersICInfoIntoRatesStates(stateSize, rates, states,
                                           icinfo, NV_DATA_S(params));
      
      f->ComputeRootInformation(voi, constants, rates, ei.oldrates, states, ei.oldstates,
                                algebraic, condvars, &failInfo);
      for (uint32_t i = 0; i < condVarSize; i++)
      {
        if (condvars[i] == 0 && roots[i] != 0)
          condvars[i] = roots[i] * 1E-100;
      }
      DetermineRateOrStateSensitivity(hx, stateSize, &ivf);

      IDAInit(idamem, ida_resfn, /* t0 = */voi, y0, dy0);
      IDARootInit(idamem, condVarSize, ida_rootfn);
      IDASStolerances(idamem, mEpsRel, mEpsAbs);
      // IDASpgmr(idamem, 0);
      // IDASptfqmr(idamem, 0);
      IDADense(idamem, stateSize);
      IDASetErrHandlerFn(idamem, cda_ida_error_handler, &failInfo);
      IDASetUserData(idamem, &ei);

      bool firstAfterRestart = true;

      while (1)
      {
        if (firstAfterRestart &&
            (isFirst || mTabulationStepSize == 0.0 ||
             floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance) ||
             voi >= nextStopPoint || voi >= mStopBvar))
        {
          f->EvaluateVariables(voi, constants, rates, states, algebraic, condvars, &failInfo);
          storage[storageSize] = voi;
          memcpy(storage + storageSize + 1, states, rateSize * sizeof(double));
          memcpy(storage + storageSize + 1 + rateSize, rates,
                 rateSize * sizeof(double));
          memcpy(storage + storageSize + 1 + rateSize * 2, algebraic,
                 algSize * sizeof(double));
          storageSize += recsize;
          
          nextStopPoint = mTabulationStepSize == 0.0 ? mStopBvar :
            (mTabulationStepSize * ++tabStepNumber) + mStartBvar;
        }
        firstAfterRestart = false;

        if (restart || voi >= mStopBvar)
          break;
        
        double bhl = mStopBvar;
        if (bhl - voi > mStepSizeMax)
          bhl = voi + mStepSizeMax;
        if(bhl > nextStopPoint)
          bhl = nextStopPoint;
        
        IDASetStopTime(idamem, bhl);

        int ret = IDASolve(idamem, bhl, &voi, y0, dy0, IDA_ONE_STEP);

        if (ret < 0)
        {
          failAddCause(&failInfo, "Failure solving model with IDA");
          break;
        }

        if (ret == IDA_ROOT_RETURN)
        {
          // printf("Root hit at %g... restarting...\n",  voi);
          restart = true;
          ivf.voi0 = voi;
          IDAGetRootInfo(idamem, roots);
        }

        memcpy(ei.oldrates, rates, rateSize * sizeof(double));
        memcpy(ei.oldstates, states, stateSize * sizeof(double));
      
        if (checkPauseOrCancellation())
          break;
      
        if (isFirst)
          isFirst = false;
        else if (voi - lastVOI < minReportForDensity && !floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance))
          continue;
      
        if(mStrictTabulation && !floatsEqual(voi, nextStopPoint, tabulationRelativeTolerance))
          continue;
      
        if (voi==nextStopPoint)
          nextStopPoint = (mTabulationStepSize * ++tabStepNumber) + mStartBvar;
      
        lastVOI = voi;
      
        f->EvaluateVariables(voi, constants, rates, states, algebraic, condvars, &failInfo);
      
        if (!restart)
        {
          // Add to storage...
          storage[storageSize] = voi;
          memcpy(storage + storageSize + 1, states, rateSize * sizeof(double));
          memcpy(storage + storageSize + 1 + rateSize, rates,
                 rateSize * sizeof(double));
          memcpy(storage + storageSize + 1 + rateSize * 2, algebraic,
                 algSize * sizeof(double));
          storageSize += recsize;
        }
      
        // Are we ready to send?
        uint32_t timeNow = time(0);
        if (timeNow >= storageExpiry || storageSize == storageCapacity)
        {
          if (mObserver != NULL)
          {
            std::vector<double> resultsVector(storage, storage + storageSize);
            mObserver->results(resultsVector);
          }
          storageExpiry = timeNow + VARIABLE_TIME_LIMIT;
          storageSize = 0;
        }
      }
    }
  }

  delete [] ei.oldstates;
  delete [] ei.oldrates;
  delete [] hx;
  delete [] icinfo;
  delete [] ivf.hxtmp;
  delete [] roots;
  KINFree(&kin_mem);
  N_VDestroy(ones);
  N_VDestroy(params);

  if (storageSize != 0 && mObserver != NULL)
  {
    std::vector<double> resultsVector(storage, storage + storageSize);
    mObserver->results(resultsVector);
  }

  delete [] storage;

  if (rateSize != 0)
  {
    IDAFree(&idamem);
    N_VDestroy(y0);
    N_VDestroy(dy0);
  }

  if (mObserver != NULL)
  {
    if (failInfo.failtype)
      mObserver->failed(failInfo.failmsg.c_str());
    else
      mObserver->done();
  }
}

int
EvaluateDefintCVODE(double x, N_Vector varsV, N_Vector ratesV, void* params)
{
  DefintInformation* ei = reinterpret_cast<DefintInformation*>(params);
  *(ei->var) = x;
  *N_VGetArrayPointer_Serial(ratesV) = ei->f(ei->voi, ei->constants, ei->rates, ei->states,
                                             ei->algebraic, ei->failInfo);
  return ei->failInfo->failtype;
}

struct EDoubleStruct {
  EDoubleStruct(double aValue) : mValue(aValue) {}

  EDouble addCause(const std::string& aCause) {
    if (mWhyError == "")
      mWhyError = aCause;
    else
      mWhyError = aCause + ", caused by " + mWhyError;
    return this;
  }

  EDouble noError() {
    mWhyError = "";
    return this;
  }

  double mValue;
  std::string mWhyError;
};

int
EvaluateDefintDebugCVODE(double x, N_Vector varsV, N_Vector ratesV, void* params)
{
  DefintDebugInformation* ei = reinterpret_cast<DefintDebugInformation*>(params);
  *(ei->var) = x;
  EDouble v = ei->f(ei->voi, ei->constants, ei->rates, ei->states,
                    ei->algebraic, ei->failInfo);
  *N_VGetArrayPointer_Serial(ratesV) = v->mValue;
  if (!isfinite(v->mValue))
  {
    v->addCause("evaluating definite integral integrand");
    setFailure(ei->failInfo, v->mWhyError.c_str(), 1);
  }
  delete v;
  return ei->failInfo->failtype;
}

EDouble
CreateEDouble(double aValue)
{
  return new EDoubleStruct(aValue);
}

double
UseEDouble(EDouble aValue, struct fail_info* aFail, const char* aContext)
{
  if (!isfinite(aValue->mValue))
  {
    aValue->addCause(aContext);
    setFailure(aFail, aValue->mWhyError.c_str(), -1);
  }

  double result = aValue->mValue;

  delete aValue;  
  return result;
}

void TryAssign(double* aDest, EDouble aValue, const char* aContext, struct fail_info* aFail)
{
  if (!isfinite(aValue->mValue))
  {
    aValue->addCause(std::string("Computed value for ") + aContext + " is not finite");
    setFailure(aFail, aValue->mWhyError.c_str(), -1);
  }
  
  *aDest = aValue->mValue;
  delete aValue;
}

void TryOverrideAssign(double* aDest, EDouble aValue, const char* aContext,
                       struct Override* aOverride, struct fail_info* aFail)
{
  if (aDest >= aOverride->constants)
  {
    size_t idx = aDest - aOverride->constants;
    if (idx < aOverride->nConstants)
    {
      if (aOverride->isOverriden[idx])
        return;
    }
  }

  if (!isfinite(aValue->mValue))
  {
    aValue->addCause(std::string("Computed value for ") + aContext + " is not finite");
    setFailure(aFail, aValue->mWhyError.c_str(), -1);
  }
  
  *aDest = aValue->mValue;
  delete aValue;
}

void OverrideAssign(double* aDest, double aValue, struct Override* aOverride)
{
  if (aDest >= aOverride->constants)
  {
    size_t idx = aDest - aOverride->constants;
    if (idx < aOverride->nConstants)
    {
      if (aOverride->isOverriden[idx])
        return;
    }
  }
  *aDest = aValue;
}

void putIth(int i, std::ostream& aStr)
{
  aStr << i;
  switch (i % 100)
  {
  case 1:
    aStr << "th";
    break;
  default:
    switch (i % 10)
    {
    case 1:
      aStr << "st";
      break;
    case 2:
      aStr << "nd";
      break;
    case 3:
      aStr << "rd";
      break;
    default:
      aStr << "th";
      break;
    }
  }
}

EDouble TryAbs(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("taking absolute of a non-finite number");
  else
    aInput->noError();

  aInput->mValue = fabs(aInput->mValue);
  return aInput;
}

EDouble TryAnd(int count, ...)
{
  va_list val;
  int badidx = -1;
  EDouble badInput;
  EDouble result = new EDoubleStruct(true);

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble v = va_arg(val, EDouble);
    if (!isfinite(v->mValue))
    {
      if (badidx == -1)
      {
        badidx = i + 1;
        badInput = v;
      }
      else
        delete v;
    }
    else
    {
      result->mValue = ((result->mValue!=0.0) && (v->mValue != 0.0)) ? 1.0 : 0.0;
      delete v;
    }
  }
  va_end(val);

  if (badidx != -1)
  {
    std::stringstream ssError;
    ssError << "the ";
    putIth(badidx, ssError);
    ssError << " argument to an 'and' operation is not finite";
    badInput->addCause(ssError.str());
    result->mWhyError = badInput->mWhyError;
    delete badInput;
  }

  return result;
}

EDouble TryACos(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("argument to arccos is not finite");
  else if (aInput->mValue > 1.0)
    aInput->noError()->addCause("argument to arccos is >1");
  else if (aInput->mValue < -1.0)
    aInput->noError()->addCause("argument to arccos is < -1");

  aInput->mValue = acos(aInput->mValue);
  return aInput;
}

EDouble TryACosh(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("argument to arccosh is not finite");
  else if (aInput->mValue < 1.0)
    aInput->noError()->addCause("argument to arccosh is <1");
  else
    aInput->noError();

  aInput->mValue = acosh(aInput->mValue);
  return aInput;
}

EDouble TryACot(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("argument to arccot is NaN");
  else
    aInput->noError();

  // Note: acot(0) = pi/2, it isn't an error.
  aInput->mValue = atan(1.0/aInput->mValue);
  return aInput;
}

EDouble TryACoth(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to arccoth is not finite");
  else if (aInput->mValue == 0.0)
    aInput->noError()->addCause("input to arccoth is zero");
  else
    aInput->noError();

  aInput->mValue = atanh(1.0/aInput->mValue);
  return aInput;
}

EDouble TryACsc(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to arccosec is NaN");
  else if (aInput->mValue > -1.0 && aInput->mValue < 1.0)
    aInput->noError()->addCause("input to arccosec is in (-1.0, 1.0)");

  aInput->mValue = asin(1.0/aInput->mValue);
  return aInput;
}

EDouble TryACsch(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to arccosech is NaN");
  else if (aInput->mValue > -1.488E-8 && aInput->mValue < 7.459E-155)
    aInput->noError()->addCause("input to arccosech is too close to zero - would overflow");
  else
    aInput->noError();

  aInput->mValue = asinh(1.0/aInput->mValue);
  return aInput;
}

EDouble TryASec(EDouble aInput)
{
  if (!isnan(aInput->mValue))
    aInput->addCause("input to arcsec is NaN");
  else if (aInput->mValue > -1.0 && aInput->mValue < 1.0)
    aInput->noError()->addCause("input to arcsec is in (-1.0, 1.0)");
  else
    aInput->noError();

  aInput->mValue = acos(1.0/aInput->mValue);
  return aInput;
}

EDouble TryASech(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to arcsech is not finite");
  else if (aInput->mValue > 1)
    aInput->noError()->addCause("input to arcsech is >1");
  else if (aInput->mValue <= 0)
    aInput->noError()->addCause("input to arcsech is <= 0");
  else if (aInput->mValue < 1.113E-308)
    aInput->noError()->addCause("input to arcsech too close to zero so function will overflow");
  else
    aInput->noError();

  aInput->mValue = acosh(1.0 / aInput->mValue);
  return aInput;
}

EDouble TryASin(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to asin is not finite.");
  else if (aInput->mValue > 1.0 || aInput->mValue < -1.0)
    aInput->noError()->addCause("input to asin lies out [-1.0, 1.0]");
  else
    aInput->noError();

  aInput->mValue = asin(aInput->mValue);
  return aInput;
}

EDouble TryASinh(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to asinh is not finite");
  else if (aInput->mValue < -6.710E7)
    aInput->noError()->addCause("input to asinh is too small, causing output to overflow");
  else if (aInput->mValue > 1.340E154)
    aInput->noError()->addCause("input to asinh is too large, causing output to overflow");
  else
    aInput->noError();

  aInput->mValue = asinh(aInput->mValue);
  return aInput;
}

EDouble TryATan(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to atan is notanumber");
  else
    aInput->noError();
  
  aInput->mValue = atan(aInput->mValue);
  return aInput;
}

EDouble TryATanh(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to atanh is not finite");
  else if (aInput->mValue <= -1.0)
    aInput->noError()->addCause("input to atanh is <= -1.0");
  else if (aInput->mValue >= 1.0)
    aInput->noError()->addCause("input to atanh is >= 1.0");
  else
    aInput->noError();

  aInput->mValue = atan(aInput->mValue);
  return aInput;
}

EDouble TryCeil(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to ceil is not finite.");
  else
    aInput->noError();

  aInput->mValue = ceil(aInput->mValue);
  return aInput;
}

EDouble TryCos(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to cos is not finite.");
  else
    aInput->noError();

  aInput->mValue = cos(aInput->mValue);
  return aInput;
}

EDouble TryCosh(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to cosh is not finite.");
  else if (aInput->mValue > 710.0)
    aInput->noError()->addCause("input to cosh is too large, output would overflow.");
  else if (aInput->mValue < -710.0)
    aInput->noError()->addCause("input to cosh is too small, output would overflow.");
  else
    aInput->noError();

  aInput->mValue = cosh(aInput->mValue);
  return aInput;
}

#define PI 3.141592653589793

EDouble TryCot(EDouble aInput)
{
  double normInput = aInput->mValue - floor(aInput->mValue / PI) * PI;
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to cot is not finite.");
  else if (normInput == 0)
    aInput->noError()->addCause("input to cot is a multiple of pi");
  else
    aInput->noError();

  aInput->mValue = 1.0/tan(aInput->mValue);
  return aInput;
}

EDouble TryCoth(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to coth is not a number");
  else if (aInput->mValue > -5.563E-309 && aInput->mValue < 5.563E-309)
    aInput->noError()->
      addCause("input to coth is too close to zero so output would overflow");
  else
    aInput->noError();

  aInput->mValue = 1.0/tanh(aInput->mValue);
  return aInput;
}

EDouble TryCsc(EDouble aInput)
{
  double normInput = aInput->mValue - floor(aInput->mValue / PI) * PI;
  if (isnan(aInput->mValue))
    aInput->addCause("input to cosec is not a number");
  else if (normInput == 0)
    aInput->noError()->addCause("input to cosec is multiple of pi");
  else
    aInput->noError();

  aInput->mValue = 1.0 / sin(aInput->mValue);
  return aInput;
}

EDouble TryCsch(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to cosech is not a number");
  else if (aInput->mValue > -5.563E-309 &&
           aInput->mValue < 5.563E-309)
    aInput->noError()->addCause("input to cosech is too close to zero, would overflow");
  else
    aInput->noError();

  aInput->mValue = 1.0 / asinh(aInput->mValue);
  return aInput;
}

EDouble TryDivide(EDouble aInput1, EDouble aInput2)
{
  double rawresult = aInput1->mValue / aInput2->mValue;
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("numerator to divide is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput2->addCause("denominator to divide is not finite");
    aInput1->mWhyError = aInput2->mWhyError;
  }
  else if (aInput2->mValue == 0.0)
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("division by zero");
  }
  else if (rawresult == 0.0 && aInput1->mValue != 0.0)
    aInput1->addCause("result of divide was underflow");
  else if (!isfinite(rawresult))
    aInput1->addCause("result of divide was overflow");
  else
    aInput1->noError();

  delete aInput2;

  aInput1->mValue = rawresult;
  return aInput1;
}

EDouble TryEq(int count, ...)
{
  EDouble result = new EDoubleStruct(1.0);
  double allEqualTo;

  va_list val;

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble eval = va_arg(val, EDouble);
    if (i != 0 && eval->mValue != allEqualTo)
      result->mValue = 0.0;
    else if (i == 0)
      allEqualTo = eval->mValue;
    if (isnan(eval->mValue))
    {
      result->mWhyError = eval->mWhyError;
      std::stringstream ssCause;
      ssCause << "the ";
      putIth(i + 1, ssCause);
      ssCause << " argument to equals is not a number";
      result->addCause(ssCause.str());
      result->mValue = eval->mValue;
    }
    delete eval;
  }

  va_end(val);

  return result;
}

EDouble TryExp(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to exp is not finite");
  else if (aInput->mValue > 7.097E2)
    aInput->noError()->addCause("input to exp is too big (overflow)");
  else
    aInput->noError();

  aInput->mValue = exp(aInput->mValue);
  return aInput;
}

EDouble TryFactorial(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to factorial is not finite");
  else if (aInput->mValue > 170.6)
    aInput->noError()->addCause("input to factorial is too big (overflow)");
  else
    aInput->noError();

  aInput->mValue = factorial(aInput->mValue);
  return aInput;
}

EDouble TryFactorOf(EDouble aInput1, EDouble aInput2)
{
  if (!isfinite(aInput2->mValue))
    aInput2->addCause("numerator to factorof is not finite");
  else if (!isfinite(aInput1->mValue))
  {
    aInput2->mWhyError = aInput1->mWhyError;
    aInput2->addCause("denominator to factorof is not finite");
  }
  else if (aInput1->mValue == 0.0)
  {
    aInput2->mWhyError = aInput1->mWhyError;
    aInput2->addCause("attempt to use factorof with a denominator of zero");
  }

  double divResult = aInput2->mValue / aInput1->mValue;
  aInput2->mValue = divResult == floor(divResult);
  delete aInput1;

  return aInput2;
}

EDouble TryFloor(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to floor is not finite");
  else
    aInput->noError();

  aInput->mValue = floor(aInput->mValue);
  return aInput;
}

EDouble TryGCD(int count, ...)
{
  if (count == 0)
    return new EDoubleStruct(1.0);

  va_list parameters;

  va_start(parameters, count);
  EDouble res = va_arg(parameters, EDouble);
  if (!isfinite(res->mValue))
    res->addCause("input to GCD is not finite");
  else
    res->noError();

  while (--count)
  {
    EDouble newRes = va_arg(parameters, EDouble);
    if (!isfinite(newRes->mValue) && res->mWhyError == "")
    {
      res->mWhyError = newRes->mWhyError;
      res->addCause("input to GCD is not finite");
    }
    res->mValue = gcd_pair(res->mValue, newRes->mValue);
    delete newRes;
  }

  va_end(parameters);
  return res;
}

EDouble TryGeq(int count, ...)
{
  EDouble result = new EDoubleStruct(1.0);
  double allGt;

  va_list val;

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble eval = va_arg(val, EDouble);
    if (i != 0 && allGt < eval->mValue)
      result->mValue = 0.0;
    allGt = eval->mValue;
    if (isnan(eval->mValue))
    {
      result->mWhyError = eval->mWhyError;
      std::stringstream ssCause;
      ssCause << "the ";
      putIth(i + 1, ssCause);
      ssCause << " argument to geq is not a number";
      result->addCause(ssCause.str());
      result->mValue = eval->mValue;
    }
    delete eval;
  }

  va_end(val);

  return result;
}

EDouble TryGt(int count, ...)
{
  EDouble result = new EDoubleStruct(1.0);
  double allGt;

  va_list val;

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble eval = va_arg(val, EDouble);
    if (i != 0 && allGt <= eval->mValue)
      result->mValue = 0.0;
    allGt = eval->mValue;
    if (isnan(eval->mValue))
    {
      result->mWhyError = eval->mWhyError;
      std::stringstream ssCause;
      ssCause << "the ";
      putIth(i + 1, ssCause);
      ssCause << " argument to gt is not a number";
      result->addCause(ssCause.str());
      result->mValue = eval->mValue;
    }
    delete eval;
  }

  va_end(val);

  return result;
}

EDouble TryImplies(EDouble aInput1, EDouble aInput2)
{
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("first input to implies is not finite");
  else if (!isfinite(aInput2->mValue))
    aInput1->addCause("second input to implies is not finite");
    
  aInput1->mValue =
    ((aInput1->mValue == 0.0) || (aInput2->mValue != 0.0)) ? 1.0 : 0.0;

  delete aInput2;
  return aInput1;
}

EDouble TryLCM(int count, ...)
{
  if (count == 0)
    return new EDoubleStruct(1.0);

  va_list parameters;

  va_start(parameters, count);
  EDouble res = va_arg(parameters, EDouble);
  if (!isfinite(res->mValue))
    res->addCause("input to LCM is not finite");
  else
    res->noError();

  while (--count)
  {
    EDouble newRes = va_arg(parameters, EDouble);
    if (!isfinite(newRes->mValue) && res->mWhyError == "")
    {
      res->mWhyError = newRes->mWhyError;
      res->addCause("input to LCM is not finite");
    }
    res->mValue = lcm_pair(res->mValue, newRes->mValue);
    delete newRes;
  }

  va_end(parameters);
  return res;
}

EDouble TryLeq(int count, ...)
{
  EDouble result = new EDoubleStruct(1.0);
  double allGt;

  va_list val;

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble eval = va_arg(val, EDouble);
    if (i != 0 && allGt > eval->mValue)
      result->mValue = 0.0;
    allGt = eval->mValue;
    if (isnan(eval->mValue))
    {
      result->mWhyError = eval->mWhyError;
      std::stringstream ssCause;
      ssCause << "the ";
      putIth(i + 1, ssCause);
      ssCause << " argument to leq is not a number";
      result->addCause(ssCause.str());
      result->mValue = eval->mValue;
    }
    delete eval;
  }

  va_end(val);

  return result;
}

EDouble TryLn(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to natural log is not finite");
  else if (aInput->mValue <= 0)
    aInput->addCause("input to natural log is not positive");
  aInput->mValue = log(aInput->mValue);
  return aInput;
}

EDouble TryLog(EDouble aInput1, EDouble aInput2)
{
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("input to log is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("base to log is not finite");
  }
  else if (aInput1->mValue <= 0)
    aInput1->noError()->addCause("input to natural log is not positive");
  else if (aInput2->mValue <= 0)
    aInput1->noError()->addCause("base to natural log is not positive");
  
  aInput1->mValue = log(aInput1->mValue) / log(aInput2->mValue);
  delete aInput2;
  return aInput1;
}

EDouble TryLt(int count, ...)
{
  EDouble result = new EDoubleStruct(1.0);
  double allGt;

  va_list val;

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble eval = va_arg(val, EDouble);
    if (i != 0 && allGt >= eval->mValue)
      result->mValue = 0.0;
    allGt = eval->mValue;
    if (isnan(eval->mValue))
    {
      result->mWhyError = eval->mWhyError;
      std::stringstream ssCause;
      ssCause << "the ";
      putIth(i + 1, ssCause);
      ssCause << " argument to lt is not a number";
      result->addCause(ssCause.str());
      result->mValue = eval->mValue;
    }
    delete eval;
  }

  va_end(val);

  return result;
}

EDouble TryMax(int count, ...)
{
  if (count == 0)
  {
    EDouble ret = new EDoubleStruct(strtod("NAN", NULL));
    ret->addCause("maximum of zero expressions");
    return ret;
  }
  bool hadErr = false;

  va_list val;
  va_start(val, count);
  EDouble value = va_arg(val, EDouble);
  if (isnan(value->mValue))
  {
    value->addCause("first argument to max is not a number");
    hadErr = true;
  }
  else if (isinf(value->mValue) && value->mValue > 0)
  {
    value->addCause("first argument to max is +infinity");
    hadErr = true;
  }
  
  for (int i = 1; i < count; i++)
  {
    EDouble nextVal = va_arg(val, EDouble);
    if (isnan(nextVal->mValue) || (isinf(nextVal->mValue) && nextVal->mValue > 0))
    {
      value->mValue = nextVal->mValue;
      if (!hadErr)
      {
        value->mWhyError = nextVal->mWhyError;
        std::stringstream ssError;
        ssError << "the ";
        putIth(i + 1, ssError);
        ssError << " argument to max is ";
        if (isnan(nextVal->mValue))
          ssError << "not a number";
        else
          ssError << "+infinity";
        value->addCause(ssError.str());
      }
    }
    else
      value->mValue = (nextVal->mValue > value->mValue) ? nextVal->mValue : value->mValue;
    delete nextVal;
  }

  if (!hadErr && isinf(value->mValue))
    value->addCause("all arguments to max are negative infinity");
  else
    value->noError();

  va_end(val);

  return value;
}

EDouble TryMin(int count, ...)
{
  if (count == 0)
  {
    EDouble ret = new EDoubleStruct(strtod("NAN", NULL));
    ret->addCause("maximum of zero expressions");
    return ret;
  }
  bool hadErr = false;

  va_list val;
  va_start(val, count);
  EDouble value = va_arg(val, EDouble);
  if (isnan(value->mValue))
  {
    value->addCause("first argument to min is not a number");
    hadErr = true;
  }
  else if (isinf(value->mValue) && value->mValue < 0)
  {
    value->addCause("first argument to min is negative infinity");
    hadErr = true;
  }
  
  for (int i = 1; i < count; i++)
  {
    EDouble nextVal = va_arg(val, EDouble);
    if (isnan(nextVal->mValue) || (isinf(nextVal->mValue) && nextVal->mValue < 0))
    {
      value->mValue = nextVal->mValue;
      if (!hadErr)
      {
        value->mWhyError = nextVal->mWhyError;
        std::stringstream ssError;
        ssError << "the ";
        putIth(i + 1, ssError);
        ssError << " argument to min is ";
        if (isnan(nextVal->mValue))
          ssError << "not a number";
        else
          ssError << "+infinity";
        value->addCause(ssError.str());
      }
    }
    else
      value->mValue = (nextVal->mValue < value->mValue) ? nextVal->mValue : value->mValue;
    delete nextVal;
  }

  if (!hadErr && isinf(value->mValue))
    value->addCause("all arguments to min are +infinity");
  else
    value->noError();

  va_end(val);

  return value;
}

EDouble TryMinus(EDouble aInput1, EDouble aInput2)
{
  double result = aInput1->mValue - aInput2->mValue;
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("the first operand to minus is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("the second operand to minus is not finite");
  }
  else if (!isfinite(result))
    aInput1->noError()->addCause("an overflow in the result of minus");

  aInput1->mValue = result;
  delete aInput2;
  return aInput1;
}

EDouble TryNeq(EDouble aInput1, EDouble aInput2)
{
  double result = (aInput1->mValue != aInput2->mValue) ? 1.0 : 0.0;

  if (isnan(aInput1->mValue))
  {
    aInput1->addCause("the first operand to neq is not a number");
    result = aInput1->mValue;
  }
  else if (!isnan(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("the second operand to neq is not a number");
    result = aInput2->mValue;
  }

  aInput1->mValue = result;
  delete aInput2;
  return aInput1;
}

EDouble TryNot(EDouble aInput)
{
  double result = (aInput->mValue == 0.0) ? 1.0 : 0.0;

  if (!isfinite(aInput->mValue))
  {
    aInput->addCause("the operand to not is not finite");
    result = aInput->mValue;
  }

  aInput->mValue = result;
  return aInput;
}

EDouble TryOr(int count, ...)
{
  va_list val;
  int badidx = -1;
  EDouble badInput;
  EDouble result = new EDoubleStruct(0.0);

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble v = va_arg(val, EDouble);
    if (!isfinite(v->mValue))
    {
      if (badidx == -1)
      {
        badidx = i + 1;
        badInput = v;
      }
      else
        delete v;
    }
    else
    {
      result->mValue = ((result->mValue!=0.0) || (v->mValue != 0.0)) ? 1.0 : 0.0;
      delete v;
    }
  }
  va_end(val);

  if (badidx != -1)
  {
    std::stringstream ssError;
    ssError << "the ";
    putIth(badidx, ssError);
    ssError << " argument to an 'or' operation is not finite";
    badInput->addCause(ssError.str());
    result->mWhyError = badInput->mWhyError;
    delete badInput;
  }

  return result;
}

EDouble TryPlus(int count, ...)
{
  va_list val;
  int badidx = -1;
  EDouble badInput;
  EDouble result = new EDoubleStruct(0.0);

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble v = va_arg(val, EDouble);
    if (!isfinite(v->mValue))
    {
      if (badidx == -1)
      {
        badidx = i + 1;
        badInput = v;
      }
      else
        delete v;
    }
    else
    {
      result->mValue = (result->mValue + v->mValue);
      delete v;
    }
  }
  va_end(val);

  if (badidx != -1)
  {
    std::stringstream ssError;
    ssError << "the ";
    putIth(badidx, ssError);
    ssError << " argument to a 'plus' operation is not finite";
    badInput->addCause(ssError.str());
    result->mWhyError = badInput->mWhyError;
    delete badInput;
  }
  else if (!isfinite(result->mValue))
    result->noError()->addCause("plus operation overflowed");

  return result;
}

EDouble TryPower(EDouble aInput1, EDouble aInput2)
{
  double result = pow(aInput1->mValue, aInput2->mValue);
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("the first operand to power is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("the second operand to power is not finite");
  }
  else if (!isfinite(result))
    aInput1->noError()->addCause("an overflow in the result of power");

  aInput1->mValue = result;
  delete aInput2;
  return aInput1;
}

EDouble TryQuotient(EDouble aInput1, EDouble aInput2)
{
  double result = floor(aInput1->mValue / aInput2->mValue);
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("the first operand to quotient is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("the second operand to quotient is not finite");
  }
  else if (aInput2->mValue == 0.0)
    aInput1->noError()->addCause("the second operand to quotient is zero");
  else if (!isfinite(result))
    aInput1->noError()->addCause("an overflow in the result of quotient");

  aInput1->mValue = result;
  delete aInput2;
  return aInput1;
}

EDouble TryRem(EDouble aInput1, EDouble aInput2)
{
  double quo = floor(aInput1->mValue / aInput2->mValue);
  double result = aInput1->mValue - aInput2->mValue * quo;
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("the first operand to quotient is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("the second operand to quotient is not finite");
  }
  else if (aInput2->mValue == 0.0)
    aInput1->noError()->addCause("the second operand to quotient is zero");
  else if (!isfinite(result))
    aInput1->noError()->addCause("an overflow in the result of quotient");

  aInput1->mValue = result;
  delete aInput2;
  return aInput1;
}

EDouble TryRoot(EDouble aInput1, EDouble aInput2)
{
  double result = pow(aInput1->mValue, 1.0 / aInput2->mValue);
  if (!isfinite(aInput1->mValue))
    aInput1->addCause("the first operand to root is not finite");
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("the first operand to root is not finite");
  }
  else if (aInput1->mValue < 0.0)
    aInput1->noError()->addCause("the first operand to root is negative");
  else if (aInput2->mValue == 0.0)
    aInput1->noError()->addCause("the second operand to root is zero");
  else if (!isfinite(result))
    aInput1->noError()->addCause("the second operand to root is zero");
  
  delete aInput2;
  aInput1->mValue = result;
  return aInput1;
}

EDouble TrySec(EDouble aInput)
{
  double normInput = aInput->mValue - floor(aInput->mValue / PI) * PI;
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to sec is not finite");
  else if (normInput == PI/2)
    aInput->noError()->addCause("input to sec is equal to pi/2 + n*pi for some n");
  else
    aInput->noError();

  aInput->mValue = 1.0 / cos(aInput->mValue);
  return aInput;
}

EDouble TrySech(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to sech is not a number");
  aInput->mValue = 1.0 / cosh(aInput->mValue);
  return aInput;
}

EDouble TrySin(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to sin is not finite");
  else
    aInput->noError();

  aInput->mValue = sin(aInput->mValue);
  return aInput;
}

EDouble TrySinh(EDouble aInput)
{
  double result = sinh(aInput->mValue);

  if (!isfinite(aInput->mValue))
    aInput->addCause("input to sinh is not finite");
  else if (!isfinite(result))
    aInput->noError()->addCause("result of sinh overflows");
  else
    aInput->noError();

  aInput->mValue = sin(aInput->mValue);
  return aInput;
}

EDouble TryTan(EDouble aInput)
{
  double normInput = aInput->mValue - floor(aInput->mValue / PI) * PI;
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to tan is not finite");
  else if (normInput == PI/2)
    aInput->noError()->addCause("input to tan is equal to pi/2 + n*pi for some n");
  else
    aInput->noError();

  aInput->mValue = tan(aInput->mValue);
  return aInput;
}

EDouble TryTanh(EDouble aInput)
{
  if (isnan(aInput->mValue))
    aInput->addCause("input to tanh is not a number");
  else
    aInput->noError();

  aInput->mValue = tanh(aInput->mValue);
  return aInput;

}

EDouble TryTimes(int count, ...)
{
  va_list val;
  int badidx = -1;
  EDouble badInput;
  EDouble result = new EDoubleStruct(1.0);

  va_start(val, count);
  for (int i = 0; i < count; i++)
  {
    EDouble v = va_arg(val, EDouble);
    if (!isfinite(v->mValue))
    {
      if (badidx == -1)
      {
        badidx = i + 1;
        badInput = v;
      }
      else
        delete v;
    }
    else
    {
      result->mValue = result->mValue * v->mValue;
      delete v;
    }
  }
  va_end(val);

  if (badidx != -1)
  {
    std::stringstream ssError;
    ssError << "the ";
    putIth(badidx, ssError);
    ssError << " argument to a 'times' operation is not finite";
    badInput->addCause(ssError.str());
    result->mWhyError = badInput->mWhyError;
    delete badInput;
  }
  else if (!isfinite(result->mValue))
    result->noError()->addCause("times operation overflowed");

  return result;
}

EDouble TryUnaryMinus(EDouble aInput)
{
  if (!isfinite(aInput->mValue))
    aInput->addCause("input to unary minus operator is not finite");

  aInput->mValue = -aInput->mValue;
  return aInput;
}

EDouble TryUnitsConversion(EDouble value, EDouble mup, EDouble offset)
{
  double result = value->mValue * mup->mValue + offset->mValue;

  if (!isfinite(value->mValue))
    ;
  else if (!isfinite(value->mValue))
    value->addCause("invalid conversion multiplier");
  else if (!isfinite(offset->mValue))
    value->addCause("invalid conversion offset");
  else if (!isfinite(result))
    value->noError()->addCause("overflow during units conversion");
  else
    value->noError();

  value->mValue = result;
  delete mup;
  delete offset;
  return value;
}

EDouble TryXor(EDouble aInput1, EDouble aInput2)
{
  double result = ((aInput1->mValue != 0) ^ (aInput2->mValue != 0)) ? 1.0 : 0.0;
  if (!isfinite(aInput1->mValue))
  {
    aInput1->addCause("first input to xor is not finite");
    result = aInput1->mValue;
  }
  else if (!isfinite(aInput2->mValue))
  {
    aInput1->mWhyError = aInput2->mWhyError;
    aInput1->addCause("second input to xor is not finite");
    result = aInput2->mValue;
  }

  delete aInput2;
  aInput1->mValue = result;
  return aInput1;
}

EDouble TryPiecewise(EDouble firstCond, EDouble firstValue, ...)
{
  va_list val;
  va_start(val, firstValue);
  for (int i = 0;; i++)
  {
    EDouble cond, value;
    if (i == 0)
    {
      cond = firstCond;
      value = firstValue;
    }
    else
    {
      int command = va_arg(val, int);
      if (command == 0)
      {
        EDouble ret = new EDoubleStruct(strtod("NAN", NULL));
        ret->addCause("no conditions matched on piecewise with no otherwise");
        return ret;
      }
      else if (command == 1)
      {
        cond = va_arg(val, EDouble);
        value = va_arg(val, EDouble);
      }
      else if (command == 2)
      {
        EDouble otherwise = va_arg(val, EDouble);
        if (!isfinite(otherwise->mValue))
          otherwise->addCause("otherwise case on piecewise is not finite");
        else
          otherwise->noError();
        return otherwise;
      }
    }
    bool returnThis = cond->mValue != 0.0;
    if (!isfinite(cond->mValue))
    {
      value->mWhyError = cond->mWhyError;
      std::stringstream ssErr;
      ssErr << "the ";
      putIth(i, ssErr);
      ssErr << " piecewise condition is not finite";
      value->addCause(ssErr.str());
      returnThis = true;
    }
    else if (returnThis && !isfinite(value->mValue))
    {
      std::stringstream ssErr;
      ssErr << "the ";
      putIth(i, ssErr);
      ssErr << " piecewise value is not finite";
      value->addCause(ssErr.str());
    }

    delete cond;

    if (returnThis)
      return value;
    else
      delete value;
  }

  va_end(val);
}

EDouble TryInfinity()
{
  EDouble ret = new EDoubleStruct(INFINITY);
  ret->mWhyError = "MathML predefined symbol infinity used";
  return ret;
}

EDouble TryNaN()
{
  EDouble ret = new EDoubleStruct(strtod("NAN", NULL));
  ret->mWhyError = "MathML predefined symbol notanumber used";
  return ret;
}

EDouble
TryDefint(
          EDouble (*f)(double VOI,double *C,double *R,double *S,double *A, struct fail_info*),
          double VOI,double *C,double *R,double *S,double *A,double *V,
          EDouble lowEV, EDouble highEV,
          struct fail_info* failInfo
         )
{
  if (!isfinite(lowEV->mValue))
  {
    delete highEV;
    lowEV->addCause("evaluating lower limit for definite integral");
    return lowEV;
  }
  if (!isfinite(highEV->mValue))
  {
    delete lowEV;
    highEV->addCause("evaluating upper limit for definite integral");
    return highEV;
  }
  double lowV = lowEV->mValue, highV = highEV->mValue;
  delete lowEV;
  delete highEV;

  if (lowV == highV)
    return CreateEDouble(0.0);

  double zero = 0;
  N_Vector y = N_VMake_Serial(1, &zero);
  void* subsolver = CVodeCreate(CV_ADAMS, CV_FUNCTIONAL);
  double epsAbs = SUBSOL_TOLERANCE;
  CVodeInit(subsolver, EvaluateDefintDebugCVODE, lowV, y);
  CVodeSStolerances(subsolver, SUBSOL_TOLERANCE, epsAbs);

  DefintDebugInformation ei;
  ei.failInfo = failInfo;
  ei.voi = VOI;
  ei.constants = C;
  ei.rates = R;
  ei.states = S;
  ei.algebraic = A;
  ei.f = f;
  ei.var = V;
  CVodeSetUserData(subsolver, &ei);

  *V = lowV;
  double ret, tret;
  if (CVode(subsolver, highV, y, &tret, CV_NORMAL) < 0)
  {
    failAddCause(failInfo, "CVODE solver failure");
    ret = 0.0;
  }
  else
  {
    ret = N_VGetArrayPointer_Serial(y)[0];
  }

  CVodeFree(&subsolver);
  N_VDestroy(y);

  EDouble retE(CreateEDouble(ret));
  retE->mWhyError = failInfo->failmsg;
  clearFailure(failInfo);

  return retE;
}

double
defint(
       double (*f)(double VOI,double *C,double *R,double *S,double *A, struct fail_info*),
       double VOI,double *C,double *R,double *S,double *A,double *V,
       double lowV, double highV,
       struct fail_info* failInfo
      )
{
  if (lowV == highV)
    return 0.0;

  double zero = 0;
  N_Vector y = N_VMake_Serial(1, &zero);
  void* subsolver = CVodeCreate(CV_ADAMS, CV_FUNCTIONAL);
  double epsAbs = SUBSOL_TOLERANCE;
  CVodeInit(subsolver, EvaluateDefintCVODE, lowV, y);
  CVodeSStolerances(subsolver, SUBSOL_TOLERANCE, epsAbs);

  DefintInformation ei;
  ei.failInfo = failInfo;
  ei.voi = VOI;
  ei.constants = C;
  ei.rates = R;
  ei.states = S;
  ei.algebraic = A;
  ei.f = f;
  ei.var = V;
  CVodeSetUserData(subsolver, &ei);

  *V = lowV;
  double ret, tret;
  if (CVode(subsolver, highV, y, &tret, CV_NORMAL) < 0)
  {
    failAddCause(failInfo, "CVODE solver failure");
    ret = 0.0;
  }
  else
  {
    ret = N_VGetArrayPointer_Serial(y)[0];
  }

  CVodeFree(&subsolver);
  N_VDestroy(y);

  return ret;
}

double
factorial(double x)
{
  double ret = 1.0;
  while (x > 0.0)
  {
    ret *= x;
    x -= 1.0;
  }
  return ret;
}

double arbitrary_log(double value, double logbase)
{
  return log(value) / log(logbase);
}

double gcd_pair(double a, double b)
{
  uint32_t ai = (uint32_t)fabs(a), bi = (uint32_t)fabs(b);
  unsigned int shift = 0;
  if (ai == 0)
    return bi;
  if (bi == 0)
    return ai;
#define EVEN(x) ((x&1)==0)
  while (EVEN(ai) && EVEN(bi))
  {
    shift++;
    ai >>= 1;
    bi >>= 1;
  }
  do
  {
    if (EVEN(ai))
      ai >>= 1;
    else if (EVEN(bi))
      bi >>= 1;
    else if (ai >= bi)
      ai = (ai - bi) >> 1;
    else
      bi = (bi - ai) >> 1;
  }
  while (ai > 0);

  return (bi << shift);
}

double lcm_pair(double a, double b)
{
  return (a * b) / gcd_pair(a, b);
}

double gcd_multi(uint32_t count, ...)
{
  if (!count)
    return 1.0;

  va_list parameters;

  va_start(parameters, count);
  double res = va_arg(parameters, double);

  while (--count)
    res = gcd_pair(res, va_arg(parameters, double));

  va_end(parameters);
  return res;
}

double lcm_multi(uint32_t count, ...)
{
  if (!count)
    return 1.0;

  va_list parameters;

  va_start(parameters, count);
  double res = va_arg(parameters, double);

  while (--count)
    res = lcm_pair(res, va_arg(parameters, double));

  va_end(parameters);
  return res;
}

double multi_min(uint32_t count, ...)
{
  va_list val;
  double best, attempt;
  if (count == 0)
    return strtod("NAN", NULL);
  va_start(val, count);
  best = va_arg(val, double);
  while (--count)
  {
    attempt = va_arg(val, double);
    if (attempt < best)
      best = attempt;
  }
  va_end(val);

  return best;
}

double multi_max(uint32_t count, ...)
{
  va_list val;
  double best, attempt;
  if (count == 0)
    return strtod("NAN", NULL);
  va_start(val, count);
  best = va_arg(val, double);
  while (--count)
  {
    attempt = va_arg(val, double);
    if (attempt > best)
      best = attempt;
  }
  va_end(val);

  return best;
}

double
safe_quotient(double num, double den)
{
  if (!isfinite(num) || !isfinite(den))
    return strtod("NAN", NULL);
  int inum = (int)num, iden = (int)den;
  if (iden == 0)
    return strtod("NAN", NULL);

  return inum / iden;
}

double
safe_remainder(double num, double den)
{
  if (!isfinite(num) || !isfinite(den))
    return strtod("NAN", NULL);
  int inum = (int)num, iden = (int)den;
  if (iden == 0)
    return strtod("NAN", NULL);

  return inum % iden;
}

double
safe_factorof(double num, double den)
{
  if (!isfinite(num) || !isfinite(den))
    return strtod("NAN", NULL);
  int inum = (int)num, iden = (int)den;
  if (iden == 0)
    return strtod("NAN", NULL);

  return ((inum % iden) == 0) ? 1.0 : 0.0;
}

#define NR_RANDOM_STARTS_MIN 100
#define NR_RANDOM_STARTS_MAX 10000000
#define NR_MAX_STEPS 1000
#define NR_MAX_STEPS_INITIAL 10
#define RANDOM_SEED 0x7ce4176c

static double
random_double_logUniform()
{
  union
  {
    double asDouble;
#ifdef WIN32
    uint16_t asIntegers[4];
#else
    uint32_t asIntegers[2];
#endif
  } X;

#ifdef WIN32
  uint16_t spareRand;
#else
  uint32_t spareRand;
#endif

  do
  {
    spareRand = rand();
#ifdef WIN32
    X.asIntegers[0] = rand() | ((spareRand & 0x1) << 15);
    X.asIntegers[1] = rand() | ((spareRand & 0x2) << 14);
    X.asIntegers[2] = rand() | ((spareRand & 0x4) << 13);
    X.asIntegers[3] = rand() | ((spareRand & 0x8) << 12);
#else
    X.asIntegers[0] = rand() | ((spareRand & 0x1) << 31);
    X.asIntegers[1] = rand() | ((spareRand & 0x2) << 30);
#endif
  }
  while (!isfinite(X.asDouble));

  return X.asDouble;
}

struct Adapt_NLS_Data {
  void * adata;
  int n;
  void (*f)(double* p, double* hx, void *adata);
  struct fail_info* failInfo;
};

static int
adaptNonlinearsolve(N_Vector p, N_Vector hx, void* adata)
{
  struct Adapt_NLS_Data* adapt = reinterpret_cast<struct Adapt_NLS_Data*>(adata);
  adapt->f(NV_DATA_S(p), NV_DATA_S(hx), adapt->adata);
  for (int i = 0; i < adapt->n; i++)
    if (!isfinite(NV_Ith_S(hx, i)))
      return 1;
  return 0;
}

void
do_nonlinearsolve
(
 void (*f)(double* p, double* hx, void *adata),
 double* ioParams,
 struct fail_info* failInfo,
 uint32_t size,
 void* adata
)
{
  uint32_t i = 0, k, noSuccess = 1;
  struct Adapt_NLS_Data adapt;

  adapt.adata = adata;
  adapt.f = f;
  adapt.n = size;
  adapt.failInfo = failInfo;

  N_Vector params = N_VMake_Serial(size, ioParams);
  N_Vector ones = N_VNew_Serial(size);
  N_VConst(1.0, ones);

  srand(RANDOM_SEED);
  void* kin_mem = KINCreate();
  KINInit(kin_mem, adaptNonlinearsolve, params);
  KINSpgmr(kin_mem, 0);
  KINSetErrHandlerFn(kin_mem, recordKINSOLError, failInfo);
  KINSetUserData(kin_mem, &adapt);

  do
  {
    const int returnCode = KINSol(kin_mem, params, KIN_NONE, ones, ones);
    if (returnCode == KIN_SUCCESS)
    {
      noSuccess = 0;
      clearFailure(failInfo);
      break;
    }

    for (k = 0; k < size; k++)
      NV_Ith_S(params, k) = random_double_logUniform();
  }
  while (++i <= NR_RANDOM_STARTS_MAX);

  KINFree(&kin_mem);
  N_VDestroy(ones);
  N_VDestroy(params);

  /* XXX we shouldn't hard-code the tolerance... */
  if (noSuccess)
    failAddCause(failInfo, "Problem solving non-linear system");
}
