/*
   There are a total of 0 entries in the algebraic variable array.
   There are a total of 1 entries in each of the rate and state variable arrays.
   There are a total of 0 entries in the constant variable array.
 */
/*
 * VOI is time in component main (dimensionless).
 * STATES[0] is x in component main (dimensionless).
 * RATES[0] is d/dt x in component main (dimensionless).
 * There are a total of 1 condition variables.
 */
void
initConsts(double* CONSTANTS, double* RATES, double *STATES)
{
STATES[0] = 0;
RATES[0] = 0.1;
}
void
computeResiduals(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES, double* STATES,
                 double* OLDSTATES, double* ALGEBRAIC, double* CONDVARS)
{
if (CONDVAR[0]>=0.00000)
{
  resid[0] = STATES[0] - 1.00000;
}
else if (1)
{
  resid[0] = RATES[0] - 1.00000;
}
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
}
void
computeRoots(double VOI, double* CONSTANTS, double* RATES, double* OLDRATES, double* STATES,
             double* OLDSTATES, double* ALGEBRAIC, double* CONDVARS)
{
CONDVAR[0] = STATES[0] - 10.0000;
}
