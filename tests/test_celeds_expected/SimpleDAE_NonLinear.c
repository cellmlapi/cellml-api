/*
   There are a total of 2 entries in the algebraic variable array.
   There are a total of 1 entries in each of the rate and state variable arrays.
   There are a total of 0 entries in the constant variable array.
 */
/*
 * STATES[0] is a in component main (dimensionless).
 * ALGEBRAIC[0] is d in component main (dimensionless).
 * ALGEBRAIC[1] is c in component main (dimensionless).
 * VOI is t in component main (dimensionless).
 * RATES[0] is d/dt a in component main (dimensionless).
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
STATES[0] = 1;
}
void
computeRates(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
rootfind_0(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);
RATES[0] =  (4.00000/7.00000)*( 2.00000*(pow(ALGEBRAIC[0], 3.00000))+ALGEBRAIC[1]);
}
void
computeVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}

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
  hx[0] = fixnans(( 2.00000*( -1.00000*STATES[0]+(pow(ALGEBRAIC[0], 3.00000))+ALGEBRAIC[1])) - (0.00000));
  hx[1] = fixnans(( 3.00000*((pow(ALGEBRAIC[0], 3.00000))+ -1.00000*( 3.00000*ALGEBRAIC[1]))) - (0.00000));
#undef VOI
#undef CONSTANTS
#undef RATES
#undef STATES
#undef ALGEBRAIC
#undef pret
}
void rootfind_0(double VOI, double* CONSTANTS, double* RATES,
double* STATES, double* ALGEBRAIC, int* pret)
{
  static double p[2] = {0.1,0.1};
  double bp[2], work[LM_DIF_WORKSZ(2, 2)];
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
