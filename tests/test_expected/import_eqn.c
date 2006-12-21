/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The main variable array needs 1 entries.
 * The rate array needs 0 entries.
 * The constant array needs 1 entries.
 * The bound array needs 0 entries.
 * Variable storage is as follows:
 * * Variable sin in component actual_sin
 * * * Variable type: computed once
 * * * Variable index: 0
 * * * Has differential: false
 * * Variable x in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Has differential: false
 */
void SetupFixedConstants(double* CONSTANTS)
{
CONSTANTS[0] = 0;
}
void SetupComputedConstants(double* CONSTANTS, double* VARIABLES)
{
VARIABLES[0] = sin(CONSTANTS[0]);
}
void ComputeRates(double* BOUND, double* RATES, double* CONSTANTS, double* VARIABLES)
{
}
void ComputeVariables(double* BOUND, double* RATES, double* CONSTANTS, double* VARIABLES)
{
#ifndef VARIABLES_FOR_RATES_ONLY
#endif
}
