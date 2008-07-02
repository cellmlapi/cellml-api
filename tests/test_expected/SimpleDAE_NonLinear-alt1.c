/* Model is correctly constrained.
 * The following equations needed Newton-Raphson evaluation:
 *   <equation with no cmeta ID>
 *   in <math with no cmeta ID>
 *   <equation with no cmeta ID>
 *   in <math with no cmeta ID>
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 2 entries.
 * The constant array needs 0 entries.
 * Variable storage is as follows:
 * * Target a in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target c in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: ALGEBRAIC[0]
 * * Target d in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 1
 * * * Variable storage: ALGEBRAIC[1]
 * * Target d^1/dt^1 a in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target t in component main
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 */
void objfunc_0(double *p, double *hx, int m, int n, void *adata)
{
  struct rootfind_info* rfi = (struct rootfind_info*)adata;
#define VOI rfi->aVOI
#define CONSTANTS rfi->aCONSTANTS
#define RATES rfi->aRATES
#define STATES rfi->aSTATES
#define ALGEBRAIC rfi->aALGEBRAIC
#define pret rfi->aPRET
  ALGEBRAIC[0] = p[0];
  ALGEBRAIC[1] = p[1];
  hx[0] = fixnans(( 2.00000*( -1.00000*STATES[0]+(pow(ALGEBRAIC[1], 3.00000))+ALGEBRAIC[0])) - (0.00000));
  hx[1] = fixnans(( 3.00000*((pow(ALGEBRAIC[1], 3.00000))+ -1.00000*( 3.00000*ALGEBRAIC[0]))) - (0.00000));
#undef VOI
#undef CONSTANTS
#undef RATES
#undef STATES
#undef ALGEBRAIC
#undef pret
}
void rootfind_0(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
  static double p[2] = {0.1,0.1},
         bp[2], work[LM_DIF_WORKSZ(2, 2)];
  struct rootfind_info rfi;
  rfi.aVOI = VOI;
  rfi.aCONSTANTS = CONSTANTS;
  rfi.aRATES = RATES;
  rfi.aSTATES = STATES;
  rfi.aALGEBRAIC = ALGEBRAIC;
  rfi.aPRET = pret;
  do_levmar(objfunc_0, p, bp, work, pret, 2, &rfi);
  ALGEBRAIC[0] = p[0];
  ALGEBRAIC[1] = p[1];
}
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
STATES[0] = 1;
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
rootfind_0(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);
RATES[0] =  (4.00000/7.00000)*( 2.00000*(pow(ALGEBRAIC[1], 3.00000))+ALGEBRAIC[0]);
}
