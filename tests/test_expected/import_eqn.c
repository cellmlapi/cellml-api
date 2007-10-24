/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 0 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 2 entries.
 * Variable storage is as follows:
 * * Target sin in component actual_sin
 * * * Variable type: constant
 * * * Variable index: 1
 * * * Variable storage: CONSTANTS[1]
 * * Target x in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
CONSTANTS[0] = 0;
CONSTANTS[1] =  sin(CONSTANTS[0]);
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
}
