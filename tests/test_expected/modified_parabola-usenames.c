/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 0 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 0 entries.
 * Variable storage is as follows:
 * * Target d^1/dt^1 y in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: rate_main_y
 * * Target offset in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: main_offset
 * * Target time in component main
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: main_time
 * * Target x in component main
 * * * Variable type: algebraic variable
 * * * Variable index: 0
 * * * Variable storage: main_x
 * * Target y in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: main_y
 * * Target z in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: main_z
 */
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
main_offset = 3;
main_z = (main_offset>1.00000&&main_offset<=3.00000 ?  sin(main_offset) : main_offset>3.00000 ? 3.00000 : 1.00000);
main_y = main_offset;
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
main_x = pow(main_time, 2.00000)+main_offset;
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
rate_main_y =  2.00000*main_time;
}
