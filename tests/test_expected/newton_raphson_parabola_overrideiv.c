/* Model is correctly constrained.
 * The following equations needed Newton-Raphson evaluation:
 *   <equation with no cmeta ID>
 *   in math with cmeta:id eq2
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 1 entries.
 * The constant array needs 2 entries.
 * Variable storage is as follows:
 * * Target d^1/dt^1 y in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target offset in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 * * Target time in component main
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 * * Target x in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: ALGEBRAIC[0]
 * * Target y in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target z in component main
 * * * Variable type: constant
 * * * Variable index: 1
 * * * Variable storage: CONSTANTS[1]
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
  ALGEBRAIC[0] = *p;
  *hx = fixnans((ALGEBRAIC[0] - CONSTANTS[0]) - (pow(VOI, 2.00000)));
#undef VOI
#undef CONSTANTS
#undef RATES
#undef STATES
#undef ALGEBRAIC
#undef pret
}
void rootfind_0(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, int* pret)
{
  static double bp, work[LM_DIF_WORKSZ(1, 1)];
  struct rootfind_info rfi;
  rfi.aVOI = VOI;
  rfi.aCONSTANTS = CONSTANTS;
  rfi.aRATES = RATES;
  rfi.aSTATES = STATES;
  rfi.aALGEBRAIC = ALGEBRAIC;
  rfi.aPRET = pret;
  do_levmar(objfunc_0, &ALGEBRAIC[0], &bp, work, pret, 1, &rfi);
}
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
CONSTANTS[0] = 3;
CONSTANTS[1] = (CONSTANTS[0]>1.00000&&CONSTANTS[0]<=3.00000 ? ( sin(CONSTANTS[0])) : CONSTANTS[0]>3.00000 ? 3.00000 : 1.00000);
STATES[0] = CONSTANTS[0];
ALGEBRAIC[0] = 1.234;
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
rootfind_0(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
RATES[0] =  2.00000*VOI;
}
