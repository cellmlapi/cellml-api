/*
   There are a total of 0 entries in the algebraic variable array.
   There are a total of 0 entries in each of the rate and state variable arrays.
   There are a total of 2 entries in the constant variable array.
 */
/*
 * CONSTANTS[0] is x in component main (dimensionless).
 * CONSTANTS[1] is sin in component actual_sin (dimensionless).
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
CONSTANTS[0] = 0;
CONSTANTS[1] =  sin(CONSTANTS[0]);
}
void
computeRates(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void
computeVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
