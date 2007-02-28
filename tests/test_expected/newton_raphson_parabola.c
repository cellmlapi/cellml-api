/* Model is correctly constrained.
 * The following equations needed Newton-Raphson evaluation:
 *   <equation with no cmeta ID>
 *   in math with cmeta:id eq2
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
double NR_minfunc_1(double* CONSTANTS, double* VARIABLES, double* BOUND)
{
  return (((VARIABLES[1]-CONSTANTS[0])) - (pow(BOUND[0],2.00000)));
}
void SetupFixedConstants(double* CONSTANTS)
{
CONSTANTS[0] = 3;
}
void SetupComputedConstants(double* CONSTANTS, double* VARIABLES)
{
VARIABLES[0] = CONSTANTS[0];
VARIABLES[2] = ((((CONSTANTS[0]>1.00000))&&((CONSTANTS[0]<=3.00000)))) ? (sin(CONSTANTS[0])) : (((CONSTANTS[0]>3.00000))) ? (3.00000) : (1.00000);
}
void ComputeRates(double* BOUND, double* RATES, double* CONSTANTS, double* VARIABLES)
{
RATES[0] = (2.00000*BOUND[0]);
}
void ComputeVariables(double* BOUND, double* RATES, double* CONSTANTS, double* VARIABLES)
{
#ifndef VARIABLES_FOR_RATES_ONLY
NR_MINIMISE(NR_minfunc_1, CONSTANTS, VARIABLES, BOUND, 1);
#endif
}
