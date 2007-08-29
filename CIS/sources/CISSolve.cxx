#define GSL_DLL
#include <exception>
#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include "Utilities.hxx"
#include "CISImplementation.hxx"
#include <gsl/gsl_odeiv.h>
#include <gsl/gsl_errno.h>
#include <math.h>
#include <stdarg.h>
#include <assert.h>
#ifdef _MSC_VER
#include <time.h>
#endif

#undef N
#include <cvode.h>
#include <nvector_serial.h>
#include <sundials_types.h>
#include <cvode_dense.h>

#ifdef _MSC_VER
#include <float.h>
#define isfinite _finite
#define INFINITY (double)(0x7FF0000000000000L)
#endif

struct EvaluationInformation
{
  double* constants, * rates, * algebraic, * states;
  uint32_t rateSizeBytes, rateSize;
  void (*ComputeRates)(double VOI, double* CONSTANTS, double* RATES,
                       double* STATES, double* ALGEBRAIC);
  void (*ComputeVariables)(double VOI, double* CONSTANTS, double* RATES,
                           double* STATES, double* ALGEBRAIC);
};

int
EvaluateRatesGSL(double voi, const double vars[],
                 double rates[], void* params)
{
  EvaluationInformation* ei = reinterpret_cast<EvaluationInformation*>(params);
  
  // Update variables that change based on bound/other vars...
  ei->ComputeRates(voi, ei->constants, ei->rates, const_cast<double*>(vars),
                   ei->algebraic);

  if (rates != ei->rates)
    memcpy(rates, ei->rates, ei->rateSizeBytes);

  // printf("Compute r0=%g at bound %g\n", rates[0], bound);

  return GSL_SUCCESS;
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

int
EvaluateRatesCVODE(double bound, N_Vector varsV, N_Vector ratesV, void* params)
{
  EvaluateRatesGSL(bound, N_VGetArrayPointer_Serial(varsV),
                   N_VGetArrayPointer_Serial(ratesV), params);
  return 0;
}

// Don't cache more than 2MB of variables (assuming 8 bytes per variable). This
// leaves a little bit of room in the 2MB for CORBA overhead.
#define VARIABLE_STORAGE_LIMIT 262016
// Don't cache for more than 1 second...
#define VARIABLE_TIME_LIMIT 1

void
CDA_CellMLIntegrationRun::SolveODEProblemGSL
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

  while (voi < mStopBvar)
  {
    double bhl = mStopBvar;
    if (bhl - voi > mStepSizeMax)
      bhl = voi + mStepSizeMax;
    gsl_odeiv_evolve_apply(e, c, s, &sys, &voi, bhl,
                           &stepSize, states);

    if (mCancelIntegration)
      break;

    if (isFirst)
      isFirst = false;
    else if (voi - lastVOI < minReportForDensity)
      continue;

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

void
CDA_CellMLIntegrationRun::SolveODEProblemCVODE
(
 CompiledModelFunctions* f, uint32_t constSize,
 double* constants, uint32_t rateSize, double* rates,
 double* states, uint32_t algSize, double* algebraic
)
{
  N_Vector y = N_VMake_Serial(rateSize, states);
  void* solver;
  
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

  EvaluationInformation ei;

  CVodeMalloc(solver, EvaluateRatesCVODE, mStartBvar, y, CV_SS, mEpsRel,
              &mEpsAbs);
  if (mStepType == iface::cellml_services::BDF_IMPLICIT_1_5_SOLVE)
    CVDense(solver, rateSize);
  CVodeSetFdata(solver, &ei);

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

  while (voi < mStopBvar)
  {
    double bhl = mStopBvar;
    if (bhl - voi > mStepSizeMax)
      bhl = voi + mStepSizeMax;
    CVodeSetStopTime(solver, bhl);
    CVode(solver, bhl, y, &voi, CV_ONE_STEP_TSTOP);

    if (mCancelIntegration)
      break;

    if (isFirst)
      isFirst = false;
    else if (voi - lastVOI < minReportForDensity)
      continue;

    lastVOI = voi;

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

  CVodeFree(&solver);
  N_VDestroy_Serial(y);
}

#ifdef DEBUG_MODE
#include <fenv.h>
#endif

void
CDA_CellMLIntegrationRun::SolveODEProblem
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
    SolveODEProblemGSL(f, constSize, constants, rateSize, rates, states,
                         algSize, algebraic);
}

extern "C"
{
  CDA_EXPORT_PRE double factorial(double x) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double arbitrary_log(double value, double logbase) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double gcd_pair(double a, double b) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double lcm_pair(double a, double b) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double gcd_multi(uint32_t size, ...) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double lcm_multi(uint32_t size, ...) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double multi_min(uint32_t size, ...) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double multi_max(uint32_t size, ...) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double safe_quotient(double num, double den) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double safe_remainder(double num, double den) CDA_EXPORT_POST;
  CDA_EXPORT_PRE double safe_factorof(double num, double den) CDA_EXPORT_POST;
  CDA_EXPORT_PRE void NR_MINIMISE(double(*func)(double VOI, double *C, double *R, double *S, double *A),
                                  double VOI,double *C,double *R,double *S,double *A,double *V)
    CDA_EXPORT_POST;

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
  va_list val;
  double* storage1, *storage2, *t, ret;
  uint32_t i, j = 0;

  if (count == 0)
    return 1.0;

  storage1 = new double[count];
  assert(storage1 != NULL /* Out of memory? */);
  storage2 = new double[count >> 1];
  assert(storage2 != NULL /* Out of memory? */);

  va_start(val, count);
  for (i = 0; i < count - 1; i += 2)
  {
    double a1, a2;
    a1 = va_arg(val, double);
    a2 = va_arg(val, double);
    storage1[j++] = gcd_pair(a1, a2);
  }
  if (i < count)
    storage1[j++] = va_arg(val, double);
  va_end(val);

  while (j != 1)
  {
    count = j;
    j = 0;

    for (i = 0; i < j - 1; i += 2)
      storage2[j++] = gcd_pair(storage1[i], storage1[i + 1]);
    if (i < j)
      storage2[j++] = storage1[i];

    t = storage1;
    storage1 = storage2;
    storage2 = t;
  }

  ret = storage1[0];
  delete [] storage1;
  delete [] storage2;

  return ret;
}

double lcm_multi(uint32_t count, ...)
{
  va_list val;
  double* storage1, *storage2, *t, ret;
  uint32_t i, j = 0;

  if (count == 0)
    return 1.0;

  storage1 = new double[count];
  assert(storage1 != NULL /* Out of memory? */);
  storage2 = new double[count >> 1];
  assert(storage2 != NULL /* Out of memory? */);

  va_start(val, count);
  for (i = 0; i < count - 1; i += 2)
  {
    double a1, a2;
    a1 = va_arg(val, double);
    a2 = va_arg(val, double);
    storage1[j++] = lcm_pair(a1, a2);
  }
  if (i < count)
    storage1[j++] = va_arg(val, double);
  va_end(val);

  while (j != 1)
  {
    count = j;
    j = 0;

    for (i = 0; i < j - 1; i += 2)
      storage2[j++] = lcm_pair(storage1[i], storage1[i + 1]);
    if (i < j)
      storage2[j++] = storage1[i];

    t = storage1;
    storage1 = storage2;
    storage2 = t;
  }

  ret = storage1[0];
  delete [] storage1;
  delete [] storage2;

  return ret;
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

#define NR_RANDOM_STARTS 100
#define NR_MAX_STEPS 1000
#define NR_MAX_STEPS_INITIAL 10

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

static double
take_numeric_derivative
(
 double(*func)(double VOI, double *C, double *R, double *S, double *A),
 double VOI,
 double *C,
 double *R,
 double *S,
 double *A,
 double *x
)
{
  double saved_x, value0, value1, value2, delta, slope1, slope2, ratio;
  saved_x = *x;
  /*
   * Since we have only 52 bits of mantissa, we need delta to flip only the
   * last couple of bits.
   */
  delta = saved_x * 1E-10;
  value0 = func(VOI, C, R, S, A);
  *x -= delta;
  value1 = func(VOI, C, R, S, A);
  *x = saved_x + delta;
  value2 = func(VOI, C, R, S, A);
  *x = saved_x;
  /* We take two numeric derivatives, so we can compare them. This stops us
   * from getting trapped at around discontinuities(which otherwise produce
   * very steep trapping slopes).
   */
  slope1 = (value0 - value1) / delta;
  slope2 = (value2 - value0) / delta;

  /* If one slope is zero, return the other... */
  if (slope1 == 0.0 || !isfinite(slope1))
    return slope2;
  if (slope2 == 0.0 || !isfinite(slope2))
    return slope1;

  ratio = slope1 / slope2;
  /* If the slopes are similar, return the average... */
  if (ratio > 0.5 && ratio < 2)
    return (slope1 + slope2) / 2.0;
  /* Otherwise, return the least steep of the two... */
  if (fabs(slope1) < fabs(slope2))
    return slope1;
  return slope2;
}

void
NR_MINIMISE
(
 double(*func)(double VOI, double *C, double *R, double *S, double *A),
 double VOI,
 double *C,
 double *R,
 double *S,
 double *A,
 double *x
)
{
  double best_X = 0.0, best_fX = INFINITY;
  double current_X, current_fX, current_dfX_dX;
  uint32_t steps, maxsteps;
  uint32_t i;

  current_X = *x;

  /* We use a 100 random start Newton-Raphson algorithm... */
  for (i = 0; i < NR_RANDOM_STARTS; i++)
  {
    /* Choose a random X as a starting point, except for the first run. */
    if (i > 0 || !isfinite(current_X))
      current_X = random_double_logUniform();

    maxsteps = NR_MAX_STEPS_INITIAL;
    for (steps = 0; steps < maxsteps; steps++)
    {
      *x = current_X;
      current_fX = func(VOI, C, R, S, A);
      if (!isfinite(current_fX))
      {
        break;
      }

      if (best_fX > current_fX)
      {
        /* We can go past NR_MAX_STEPS_INITIAL steps up to NR_MAX_STEPS steps,
         * but only if we keep improving on our previous answer.
         */
        maxsteps += 2;
        if (maxsteps > NR_MAX_STEPS)
          maxsteps = NR_MAX_STEPS;
        best_fX = current_fX;
        best_X = current_X;
        /* This is quite far from 0(relatively speaking for a double) to avoid
         * numerical instability issues when the minimum doesn't exist but the
         * limit at a point from a particular direction would otherwise be the
         * minimum.
         */
        if (best_fX <= 1E-250)
	{
          break;
	}
      }

      current_dfX_dX = take_numeric_derivative(func, VOI, C, R, S, A, x);
      /* If it is completely flat, or infinite, we are done with this round. */
      if (!isfinite(current_dfX_dX) || current_dfX_dX == 0.0)
      {
        break;
      }

      current_X -= current_fX / current_dfX_dX;
      if (!isfinite(current_X))
      {
        break;
      }
    }
    if (best_fX <= 1E-250)
      break;
  }

  *x = best_X;
}
