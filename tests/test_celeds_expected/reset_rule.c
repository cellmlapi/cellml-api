/*
   There are a total of 0 entries in the algebraic variable array.
   There are a total of 1 entries in each of the rate and state variable arrays.
   There are a total of 0 entries in the constant variable array.
 */
/*
 * VOI is time in component main (dimensionless).
 * STATES[0] is x in component main (dimensionless).
 * RATES[0] is d/dt x in component main (dimensionless).
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
}
void
computeRates(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
if (STATES[0]>10.0000)
{
  STATES[0] = 0.00000;
}
if (!(STATES[0]>10.0000))
{
  RATES[0] = 1.00000;
}
}
void
computeVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
