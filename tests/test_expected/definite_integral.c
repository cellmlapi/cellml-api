/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 2 entries.
 * The constant array needs 4 entries.
 * Variable storage is as follows:
 * * Target boundheight in component Height
 * * * Variable type: locally bound
 * * * Variable index: 0
 * * * Variable storage: ALGEBRAIC[0]
 * * Target d^1/dt^1 height in component Height
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target growthRate in component Height
 * * * Variable type: constant
 * * * Variable index: 2
 * * * Variable storage: CONSTANTS[2]
 * * Target height in component Height
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target heightStdDev in component Height
 * * * Variable type: constant
 * * * Variable index: 1
 * * * Variable storage: CONSTANTS[1]
 * * Target meanHeight in component Height
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 * * Target pvalue in component Height
 * * * Variable type: algebraic variable
 * * * Variable index: 1
 * * * Variable storage: ALGEBRAIC[1]
 * * Target time in component Height
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 */
 double func1(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC, int* pret) { return  (1.00000/ pow(( 2.00000* 3.14159265358979*pow(CONSTANTS[1], 2.00000)), 1.0 / 2))*exp( (-0.500000/pow(CONSTANTS[1], 2.00000))*pow(ALGEBRAIC[0] - CONSTANTS[0], 2.00000)); }
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
/* meanHeight */
CONSTANTS[0] = 1.7;
/* heightStdDev */
CONSTANTS[1] = 0.1;
/* height */
STATES[0] = 1.7;
/* growthRate */
CONSTANTS[2] = 0.01;
/* Element with no id */
CONSTANTS[3] = CONSTANTS[2];
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
/* Element with no id */
ALGEBRAIC[1] = 1.00000 - defint(func1, VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, &ALGEBRAIC[0],  2.00000*CONSTANTS[0] - STATES[0], STATES[0], pret);
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
/* Rate Restore */
RATES[0] = CONSTANTS[3];
}
