/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 0 entries.
 * Variable storage is as follows:
 * * Target d^1/dt^1 x in component mainComp
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target time in component mainComp
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 * * Target x in component mainComp
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
/* Constant x */
STATES[0] = 0;
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
if (STATES[0]<100.000)
{
  /* Element with no id */
RATES[0] = 1.00000;
}
else if (1.0)
{
  /* Element with no id */
RATES[0] = -1.00000;
}
}
