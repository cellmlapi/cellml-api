/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The main variable array needs 3 entries.
 * The rate array needs 1 entries.
 * The constant array needs 1 entries.
 * The bound array needs 1 entries.
 * Variable storage is as follows:
 * * Variable offset in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Has differential: false
 * * Variable time in component main
 * * * Variable type: bound variable
 * * * Variable index: 0
 * * * Has differential: false
 * * Variable x in component main
 * * * Variable type: computed for every bound variable
 * * * Variable index: 1
 * * * Has differential: false
 * * Variable y in component main
 * * * Variable type: differential
 * * * Variable index: 0
 * * * Has differential: true
 * * * Highest derivative: 1
 * * Variable z in component main
 * * * Variable type: computed once
 * * * Variable index: 2
 * * * Has differential: false
 */
void SetupFixedConstants(double* CONSTANTS)
{
CONSTANTS[0] = 3;
}
void SetupComputedConstants(double* CONSTANTS, double* VARIABLES)
{
VARIABLES[2] = ((((CONSTANTS[0]>1.00000))&&((CONSTANTS[0]<=3.00000)))) ? (sin(CONSTANTS[0])) : (((CONSTANTS[0]>3.00000))) ? (3.00000) : (1.00000);
VARIABLES[0] = CONSTANTS[0];
}
void ComputeRates(double* BOUND, double* RATES, double* CONSTANTS, double* VARIABLES)
{
RATES[0] = (2.00000*BOUND[0]);
}
void ComputeVariables(double* BOUND, double* RATES, double* CONSTANTS, double* VARIABLES)
{
#ifndef VARIABLES_FOR_RATES_ONLY
VARIABLES[1] = (pow(BOUND[0],2.00000)+CONSTANTS[0]);
#endif
}
