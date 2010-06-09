/*
   There are a total of 0 entries in the algebraic variable array.
   There are a total of 3 entries in each of the rate and state variable arrays.
   There are a total of 0 entries in the constant variable array.
 */
/*
 * STATES[0] is a in component main (dimensionless).
 * STATES[1] is d in component main (dimensionless).
 * STATES[2] is c in component main (dimensionless).
 * VOI is t in component main (dimensionless).
 * RATES[0] is d/dt a in component main (dimensionless).
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
STATES[0] = 1;
STATES[1] = 0.1;
STATES[2] = 0.1;
RATES[0] = 0.1;
}
void
computeResiduals(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
resid[0] = RATES[0] -  (4.00000/7.00000)*( 2.00000*(pow(STATES[1], 3.00000))+STATES[2]);
resid[1] =  2.00000*( -1.00000*STATES[0]+(pow(STATES[1], 3.00000))+STATES[2]) - 0.00000;
resid[2] =  3.00000*((pow(STATES[1], 3.00000))+ -1.00000*( 3.00000*STATES[2])) - 0.00000;
}
void
computeVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void
computeEssentialVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void
getStateInformation(double* SI)
{
SI[0] = 1.0;
SI[1] = 0.0;
SI[2] = 0.0;
}
