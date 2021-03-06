/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 1 entries.
 * The constant array needs 2 entries.
 * Variable storage is as follows:
 * * Target d^1/dt^1 y in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target offset in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 * * Target time in component main
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 * * Target x in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: ALGEBRAIC[0]
 * * Target y in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target z in component main
 * * * Variable type: constant
 * * * Variable index: 1
 * * * Variable storage: CONSTANTS[1]
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
/* Constant offset */
CONSTANTS[0] = 3;
/* Constant Element with no id */
CONSTANTS[1] = (CONSTANTS[0]>1.00000&&CONSTANTS[0]<=3.00000 ?  sin(CONSTANTS[0]) : CONSTANTS[0]>3.00000 ? 3.00000 : 1.00000);
/* Constant y */
STATES[0] = CONSTANTS[0];
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
/* Element with no id */
ALGEBRAIC[0] = pow(VOI, 2.00000)+CONSTANTS[0];
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
/* Element with no id */
RATES[0] =  2.00000*VOI;
}
