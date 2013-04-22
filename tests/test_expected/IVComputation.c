/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 2 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 4 entries.
 * Variable storage is as follows:
 * * Target amplitude in component environment
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 * * Target d^1/dt^1 x in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 1
 * * * Variable storage: RATES[1]
 * * Target d^1/dt^1 y in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target initial_t in component environment
 * * * Variable type: constant
 * * * Variable index: 1
 * * * Variable storage: CONSTANTS[1]
 * * Target t in component environment
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 * * Target x in component main
 * * * Variable type: state variable
 * * * Variable index: 1
 * * * Variable storage: STATES[1]
 * * Target x_initial in component main
 * * * Variable type: constant
 * * * Variable index: 3
 * * * Variable storage: CONSTANTS[3]
 * * Target y in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target y_initial in component main
 * * * Variable type: constant
 * * * Variable index: 2
 * * * Variable storage: CONSTANTS[2]
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
/* amplitude */
CONSTANTS[0] = 5;
/* initial_t */
CONSTANTS[1] = 0.78;
/* 3.2.3 */
CONSTANTS[2] =  CONSTANTS[0]* sin(CONSTANTS[1]);
/* 3.2.4 */
CONSTANTS[3] =  CONSTANTS[0]*cos(CONSTANTS[1]);
/* y */
STATES[0] = CONSTANTS[2];
/* x */
STATES[1] = CONSTANTS[3];
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
/* 3.2.1 */
RATES[0] = STATES[1];
/* 3.2.2 */
RATES[1] = - STATES[0];
}
