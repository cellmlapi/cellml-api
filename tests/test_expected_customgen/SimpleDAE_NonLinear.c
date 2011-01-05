Constraint level = OVERCONSTRAINED
Index count: 3
* Computation target main/a:0:
  => Type = STATE_VARIABLE - was requested, and is available.
  => Name = VARS[0]
  => Index = 0
* Computation target main/d:0:
  => Type = ALGEBRAIC - is used as an intermediate.
  => Name = VARS[1]
  => Index = 1
* Computation target main/c:0:
  => Type = VARIABLE_OF_INTEGRATION - was marked as independent.
  => Name = VARS[2]
  => Index = 2
* Computation target main/t:0:
  => Type = FLOATING - unused and not requested.
  => Name = 
  => Index = 0
* Computation target main/a:1:
  => Type = FLOATING - unused and not requested.
  => Name = 
  => Index = 0
Functions: void objfunc_0(double *p, double *hx, int m, int n, void *adata)
{
  struct rootfind_info* rfi = (struct rootfind_info*)adata;
#define VOI rfi->aVOI
#define CONSTANTS rfi->aCONSTANTS
#define RATES rfi->aRATES
#define STATES rfi->aSTATES
#define ALGEBRAIC rfi->aALGEBRAIC
#define pret rfi->aPRET
  VARS[1] = *p;
  *hx = ( 3.00000*((pow(VARS[1], 3.00000))+ -1.00000*( 3.00000*VARS[2]))) - (0.00000);
#undef VOI
#undef CONSTANTS
#undef RATES
#undef STATES
#undef ALGEBRAIC
#undef pret
}
void rootfind_0(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, int* pret)
{
  static double val = 0.1;
  double bp, work[LM_DIF_WORKSZ(1, 1)];
  struct rootfind_info rfi;
  rfi.aVOI = VOI;
  rfi.aCONSTANTS = CONSTANTS;
  rfi.aRATES = RATES;
  rfi.aSTATES = STATES;
  rfi.aALGEBRAIC = ALGEBRAIC;
  rfi.aPRET = pret;
  do_levmar(objfunc_0, &val, &bp, work, pret, 1, &rfi);
  VARS[1] = val;
}
void objfunc_1(double *p, double *hx, int m, int n, void *adata)
{
  struct rootfind_info* rfi = (struct rootfind_info*)adata;
#define VOI rfi->aVOI
#define CONSTANTS rfi->aCONSTANTS
#define RATES rfi->aRATES
#define STATES rfi->aSTATES
#define ALGEBRAIC rfi->aALGEBRAIC
#define pret rfi->aPRET
  VARS[0] = *p;
  *hx = ( 2.00000*( -1.00000*VARS[0]+(pow(VARS[1], 3.00000))+VARS[2])) - (0.00000);
#undef VOI
#undef CONSTANTS
#undef RATES
#undef STATES
#undef ALGEBRAIC
#undef pret
}
void rootfind_1(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, int* pret)
{
  static double val = 0.1;
  double bp, work[LM_DIF_WORKSZ(1, 1)];
  struct rootfind_info rfi;
  rfi.aVOI = VOI;
  rfi.aCONSTANTS = CONSTANTS;
  rfi.aRATES = RATES;
  rfi.aSTATES = STATES;
  rfi.aALGEBRAIC = ALGEBRAIC;
  rfi.aPRET = pret;
  do_levmar(objfunc_1, &val, &bp, work, pret, 1, &rfi);
  VARS[0] = val;
}

Code: rootfind_0(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);
rootfind_1(VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, pret);

