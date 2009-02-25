/*
   There are a total of 0 entries in the algebraic variable array.
   There are a total of 1 entries in each of the rate and state variable arrays.
   There are a total of 0 entries in the constant variable array.
 */
/*
 * VOI is time in component main (second).
 * STATES[0] is distance in component main (metre).
 * RATES[0] is d/dt distance in component main (metre).
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
STATES[0] = 5;
}
void
computeRates(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
RATES[0] = -1.00000 -  - -1.00000*VOI;
}
void
computeVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
