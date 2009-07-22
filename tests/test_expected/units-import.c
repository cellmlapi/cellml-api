/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 0 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 1 entries.
 * Variable storage is as follows:
 * * Target alpha in component component
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
CONSTANTS[0] = 0.100000;
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
}
