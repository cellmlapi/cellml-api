/* Model is correctly constrained.
 * The following equations needed Newton-Raphson evaluation:
 *   <equation with no cmeta ID>
 *   in math with cmeta:id eq2
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
double minfunc_0(double VOI, double* CONSTANTS, double* RATES, double* STATES, double * ALGEBRAIC)
{
return fabs((ALGEBRAIC[0] - CONSTANTS[0])-(pow(VOI, 2)));
}
void SetupFixedConstants(double* CONSTANTS, double* RATES)
{
CONSTANTS[0] = 3;
CONSTANTS[1] = CONSTANTS[0]>1&&CONSTANTS[0]<=3 ? ( sin(CONSTANTS[0])) : CONSTANTS[0]>3 ? 3 : 1;
STATES[0] = CONSTANTS[0];
}
void EvaluateVariables(double BOUND, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
NR_MINIMISE(minfunc_0, VOI, CONSTANTS, RATES, STATES, ALGEBRAIC, &ALGEBRAIC[0]);
}
void ComputeRates(double BOUND, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
RATES[0] =  2*VOI;
}
