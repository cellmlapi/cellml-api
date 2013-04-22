/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 0 entries.
 * Variable storage is as follows:
 * * Target d^1/dt^1 distance in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target distance in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target time in component main
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
/* distance */
STATES[0] = 5;
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
/* Element with no id */
RATES[0] = -1.00000 -  - -1.00000*VOI;
}
