/*
   There are a total of 1 entries in the algebraic variable array.
   There are a total of 1 entries in each of the rate and state variable arrays.
   There are a total of 2 entries in the constant variable array.
 */
/*
 * VOI is time in component main (dimensionless).
 * CONSTANTS[0] is offset in component main (dimensionless).
 * ALGEBRAIC[0] is x in component main (dimensionless).
 * STATES[0] is y in component main (dimensionless).
 * CONSTANTS[1] is z in component main (dimensionless).
 * RATES[0] is d/dt y in component main (dimensionless).
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
CONSTANTS[0] = 3;
CONSTANTS[1] = (CONSTANTS[0]>1.00000&&CONSTANTS[0]<=3.00000 ?  sin(CONSTANTS[0]) : CONSTANTS[0]>3.00000 ? 3.00000 : 1.00000);
STATES[0] = CONSTANTS[0];
}
void
computeRates(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
RATES[0] =  2.00000*VOI;
}
void
computeVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
ALGEBRAIC[0] = pow(VOI, 2.00000)+CONSTANTS[0];
}
