/* Model is correctly constrained.
 * No equations needed Newton-Raphson evaluation.
 * The rate and state arrays need 1 entries.
 * The algebraic variables array needs 0 entries.
 * The constant array needs 7 entries.
 * Variable storage is as follows:
 * * Target d^1/dt^1 val in component main
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: RATES[0]
 * * Target r1 in component main
 * * * Variable type: constant
 * * * Variable index: 5
 * * * Variable storage: CONSTANTS[5]
 * * Target r1Mean in component main
 * * * Variable type: constant
 * * * Variable index: 4
 * * * Variable storage: CONSTANTS[4]
 * * Target r1Variance in component main
 * * * Variable type: constant
 * * * Variable index: 3
 * * * Variable storage: CONSTANTS[3]
 * * Target r2 in component main
 * * * Variable type: constant
 * * * Variable index: 1
 * * * Variable storage: CONSTANTS[1]
 * * Target r3 in component main
 * * * Variable type: constant
 * * * Variable index: 2
 * * * Variable storage: CONSTANTS[2]
 * * Target time in component main
 * * * Variable type: variable of integration
 * * * Variable index: 0
 * * * Variable storage: VOI
 * * Target val in component main
 * * * Variable type: state variable
 * * * Variable index: 0
 * * * Variable storage: STATES[0]
 * * Target x in component main
 * * * Variable type: locally bound
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 */
double pdf_0(double bvar, double* CONSTANTS, double* ALGEBRAIC)
{
  return ( (1.00000/ pow(( 2.00000* 3.14159265358979*CONSTANTS[3]), 1.0 / 2))*exp(- (pow(bvar - CONSTANTS[4], 2.00000)/( 2.00000*CONSTANTS[3]))));
}
double (*pdf_roots_0[])(double bvar, double*, double*) = {};
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
/* Constant val */
STATES[0] = 0;
/* Constant x */
CONSTANTS[0] = 0;
switch (rand() % 1000)
{
case 0:
/* Constant r2r3dist */
CONSTANTS[1] = -0.397304;
/* Constant r2r3dist */
CONSTANTS[2] = 8.70924;
break;
case 1:
/* Constant r2r3dist */
CONSTANTS[1] = -1.16749;
/* Constant r2r3dist */
CONSTANTS[2] = 9.68624;
break;
case 2:
/* Constant r2r3dist */
CONSTANTS[1] = -0.649844;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0365;
break;
case 3:
/* Constant r2r3dist */
CONSTANTS[1] = 1.82449;
/* Constant r2r3dist */
CONSTANTS[2] = 12.1333;
break;
case 4:
/* Constant r2r3dist */
CONSTANTS[1] = -2.67560;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43155;
break;
case 5:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0547296;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0719;
break;
case 6:
/* Constant r2r3dist */
CONSTANTS[1] = 1.07090;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0235;
break;
case 7:
/* Constant r2r3dist */
CONSTANTS[1] = -0.554121;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4162;
break;
case 8:
/* Constant r2r3dist */
CONSTANTS[1] = -0.330647;
/* Constant r2r3dist */
CONSTANTS[2] = 9.37518;
break;
case 9:
/* Constant r2r3dist */
CONSTANTS[1] = 0.438340;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1350;
break;
case 10:
/* Constant r2r3dist */
CONSTANTS[1] = -1.00341;
/* Constant r2r3dist */
CONSTANTS[2] = 8.96924;
break;
case 11:
/* Constant r2r3dist */
CONSTANTS[1] = -0.280605;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5120;
break;
case 12:
/* Constant r2r3dist */
CONSTANTS[1] = 0.169892;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0342;
break;
case 13:
/* Constant r2r3dist */
CONSTANTS[1] = -3.24431;
/* Constant r2r3dist */
CONSTANTS[2] = 8.59351;
break;
case 14:
/* Constant r2r3dist */
CONSTANTS[1] = 0.537377;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0864;
break;
case 15:
/* Constant r2r3dist */
CONSTANTS[1] = 0.259131;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33872;
break;
case 16:
/* Constant r2r3dist */
CONSTANTS[1] = 0.196380;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43961;
break;
case 17:
/* Constant r2r3dist */
CONSTANTS[1] = 0.772454;
/* Constant r2r3dist */
CONSTANTS[2] = 9.98352;
break;
case 18:
/* Constant r2r3dist */
CONSTANTS[1] = -0.442969;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5780;
break;
case 19:
/* Constant r2r3dist */
CONSTANTS[1] = 0.529665;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43131;
break;
case 20:
/* Constant r2r3dist */
CONSTANTS[1] = -0.175020;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53173;
break;
case 21:
/* Constant r2r3dist */
CONSTANTS[1] = 1.32604;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1211;
break;
case 22:
/* Constant r2r3dist */
CONSTANTS[1] = 0.464047;
/* Constant r2r3dist */
CONSTANTS[2] = 9.57088;
break;
case 23:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0433567;
/* Constant r2r3dist */
CONSTANTS[2] = 9.02465;
break;
case 24:
/* Constant r2r3dist */
CONSTANTS[1] = -1.56877;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92779;
break;
case 25:
/* Constant r2r3dist */
CONSTANTS[1] = -0.475073;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5689;
break;
case 26:
/* Constant r2r3dist */
CONSTANTS[1] = -1.06808;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96579;
break;
case 27:
/* Constant r2r3dist */
CONSTANTS[1] = -0.398461;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2092;
break;
case 28:
/* Constant r2r3dist */
CONSTANTS[1] = -0.527764;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1104;
break;
case 29:
/* Constant r2r3dist */
CONSTANTS[1] = 0.461493;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6514;
break;
case 30:
/* Constant r2r3dist */
CONSTANTS[1] = 1.95319;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7851;
break;
case 31:
/* Constant r2r3dist */
CONSTANTS[1] = -0.284033;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92798;
break;
case 32:
/* Constant r2r3dist */
CONSTANTS[1] = 1.31951;
/* Constant r2r3dist */
CONSTANTS[2] = 11.4740;
break;
case 33:
/* Constant r2r3dist */
CONSTANTS[1] = -0.116536;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1436;
break;
case 34:
/* Constant r2r3dist */
CONSTANTS[1] = -0.601142;
/* Constant r2r3dist */
CONSTANTS[2] = 9.35790;
break;
case 35:
/* Constant r2r3dist */
CONSTANTS[1] = 0.409480;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3878;
break;
case 36:
/* Constant r2r3dist */
CONSTANTS[1] = -1.69898;
/* Constant r2r3dist */
CONSTANTS[2] = 9.13079;
break;
case 37:
/* Constant r2r3dist */
CONSTANTS[1] = 0.247768;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8265;
break;
case 38:
/* Constant r2r3dist */
CONSTANTS[1] = -1.29528;
/* Constant r2r3dist */
CONSTANTS[2] = 9.87246;
break;
case 39:
/* Constant r2r3dist */
CONSTANTS[1] = 0.484394;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3463;
break;
case 40:
/* Constant r2r3dist */
CONSTANTS[1] = -0.122231;
/* Constant r2r3dist */
CONSTANTS[2] = 9.76447;
break;
case 41:
/* Constant r2r3dist */
CONSTANTS[1] = 1.12046;
/* Constant r2r3dist */
CONSTANTS[2] = 11.8710;
break;
case 42:
/* Constant r2r3dist */
CONSTANTS[1] = -0.925777;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0598;
break;
case 43:
/* Constant r2r3dist */
CONSTANTS[1] = -1.19540;
/* Constant r2r3dist */
CONSTANTS[2] = 9.54287;
break;
case 44:
/* Constant r2r3dist */
CONSTANTS[1] = 1.00306;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96969;
break;
case 45:
/* Constant r2r3dist */
CONSTANTS[1] = -0.915627;
/* Constant r2r3dist */
CONSTANTS[2] = 9.68636;
break;
case 46:
/* Constant r2r3dist */
CONSTANTS[1] = 0.000429096;
/* Constant r2r3dist */
CONSTANTS[2] = 9.47965;
break;
case 47:
/* Constant r2r3dist */
CONSTANTS[1] = -0.356800;
/* Constant r2r3dist */
CONSTANTS[2] = 9.02946;
break;
case 48:
/* Constant r2r3dist */
CONSTANTS[1] = 0.481115;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1639;
break;
case 49:
/* Constant r2r3dist */
CONSTANTS[1] = 0.984145;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9652;
break;
case 50:
/* Constant r2r3dist */
CONSTANTS[1] = -0.937497;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78731;
break;
case 51:
/* Constant r2r3dist */
CONSTANTS[1] = 2.02395;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3836;
break;
case 52:
/* Constant r2r3dist */
CONSTANTS[1] = -0.857944;
/* Constant r2r3dist */
CONSTANTS[2] = 9.73200;
break;
case 53:
/* Constant r2r3dist */
CONSTANTS[1] = -0.492842;
/* Constant r2r3dist */
CONSTANTS[2] = 9.14182;
break;
case 54:
/* Constant r2r3dist */
CONSTANTS[1] = 1.97649;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9234;
break;
case 55:
/* Constant r2r3dist */
CONSTANTS[1] = 3.18893;
/* Constant r2r3dist */
CONSTANTS[2] = 11.5659;
break;
case 56:
/* Constant r2r3dist */
CONSTANTS[1] = 0.850761;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3575;
break;
case 57:
/* Constant r2r3dist */
CONSTANTS[1] = 1.16166;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3596;
break;
case 58:
/* Constant r2r3dist */
CONSTANTS[1] = 0.855790;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2775;
break;
case 59:
/* Constant r2r3dist */
CONSTANTS[1] = -1.90250;
/* Constant r2r3dist */
CONSTANTS[2] = 9.10788;
break;
case 60:
/* Constant r2r3dist */
CONSTANTS[1] = 1.34709;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9309;
break;
case 61:
/* Constant r2r3dist */
CONSTANTS[1] = -0.542613;
/* Constant r2r3dist */
CONSTANTS[2] = 9.56537;
break;
case 62:
/* Constant r2r3dist */
CONSTANTS[1] = -0.612029;
/* Constant r2r3dist */
CONSTANTS[2] = 9.84630;
break;
case 63:
/* Constant r2r3dist */
CONSTANTS[1] = 0.662960;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2884;
break;
case 64:
/* Constant r2r3dist */
CONSTANTS[1] = -0.178510;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8677;
break;
case 65:
/* Constant r2r3dist */
CONSTANTS[1] = -0.790018;
/* Constant r2r3dist */
CONSTANTS[2] = 9.58712;
break;
case 66:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0497061;
/* Constant r2r3dist */
CONSTANTS[2] = 9.62169;
break;
case 67:
/* Constant r2r3dist */
CONSTANTS[1] = -0.109377;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1402;
break;
case 68:
/* Constant r2r3dist */
CONSTANTS[1] = 0.973509;
/* Constant r2r3dist */
CONSTANTS[2] = 11.4186;
break;
case 69:
/* Constant r2r3dist */
CONSTANTS[1] = 0.424305;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1673;
break;
case 70:
/* Constant r2r3dist */
CONSTANTS[1] = -0.862519;
/* Constant r2r3dist */
CONSTANTS[2] = 9.24313;
break;
case 71:
/* Constant r2r3dist */
CONSTANTS[1] = 0.873642;
/* Constant r2r3dist */
CONSTANTS[2] = 9.90775;
break;
case 72:
/* Constant r2r3dist */
CONSTANTS[1] = 0.627842;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0840;
break;
case 73:
/* Constant r2r3dist */
CONSTANTS[1] = -0.982380;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31079;
break;
case 74:
/* Constant r2r3dist */
CONSTANTS[1] = -0.753886;
/* Constant r2r3dist */
CONSTANTS[2] = 9.67372;
break;
case 75:
/* Constant r2r3dist */
CONSTANTS[1] = 0.305687;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95795;
break;
case 76:
/* Constant r2r3dist */
CONSTANTS[1] = -0.448600;
/* Constant r2r3dist */
CONSTANTS[2] = 9.98373;
break;
case 77:
/* Constant r2r3dist */
CONSTANTS[1] = 1.96389;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2289;
break;
case 78:
/* Constant r2r3dist */
CONSTANTS[1] = 0.103843;
/* Constant r2r3dist */
CONSTANTS[2] = 9.91418;
break;
case 79:
/* Constant r2r3dist */
CONSTANTS[1] = -0.294529;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92863;
break;
case 80:
/* Constant r2r3dist */
CONSTANTS[1] = -1.09942;
/* Constant r2r3dist */
CONSTANTS[2] = 9.73231;
break;
case 81:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21572;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1152;
break;
case 82:
/* Constant r2r3dist */
CONSTANTS[1] = -1.97628;
/* Constant r2r3dist */
CONSTANTS[2] = 8.52038;
break;
case 83:
/* Constant r2r3dist */
CONSTANTS[1] = -1.08008;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0401;
break;
case 84:
/* Constant r2r3dist */
CONSTANTS[1] = -0.131598;
/* Constant r2r3dist */
CONSTANTS[2] = 9.98556;
break;
case 85:
/* Constant r2r3dist */
CONSTANTS[1] = 1.48665;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9267;
break;
case 86:
/* Constant r2r3dist */
CONSTANTS[1] = 0.385169;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95288;
break;
case 87:
/* Constant r2r3dist */
CONSTANTS[1] = -0.314853;
/* Constant r2r3dist */
CONSTANTS[2] = 9.64242;
break;
case 88:
/* Constant r2r3dist */
CONSTANTS[1] = -1.46278;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1506;
break;
case 89:
/* Constant r2r3dist */
CONSTANTS[1] = -0.345246;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5831;
break;
case 90:
/* Constant r2r3dist */
CONSTANTS[1] = -1.33460;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31070;
break;
case 91:
/* Constant r2r3dist */
CONSTANTS[1] = 0.973103;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1597;
break;
case 92:
/* Constant r2r3dist */
CONSTANTS[1] = 0.134743;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92023;
break;
case 93:
/* Constant r2r3dist */
CONSTANTS[1] = -1.24889;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2489;
break;
case 94:
/* Constant r2r3dist */
CONSTANTS[1] = -0.412555;
/* Constant r2r3dist */
CONSTANTS[2] = 9.91934;
break;
case 95:
/* Constant r2r3dist */
CONSTANTS[1] = 1.29382;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2714;
break;
case 96:
/* Constant r2r3dist */
CONSTANTS[1] = -0.726661;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75208;
break;
case 97:
/* Constant r2r3dist */
CONSTANTS[1] = 1.05886;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1631;
break;
case 98:
/* Constant r2r3dist */
CONSTANTS[1] = -0.522445;
/* Constant r2r3dist */
CONSTANTS[2] = 9.16190;
break;
case 99:
/* Constant r2r3dist */
CONSTANTS[1] = 0.248655;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3259;
break;
case 100:
/* Constant r2r3dist */
CONSTANTS[1] = -0.489311;
/* Constant r2r3dist */
CONSTANTS[2] = 9.80166;
break;
case 101:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0771580;
/* Constant r2r3dist */
CONSTANTS[2] = 9.10271;
break;
case 102:
/* Constant r2r3dist */
CONSTANTS[1] = 0.844544;
/* Constant r2r3dist */
CONSTANTS[2] = 9.63365;
break;
case 103:
/* Constant r2r3dist */
CONSTANTS[1] = 1.34215;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0078;
break;
case 104:
/* Constant r2r3dist */
CONSTANTS[1] = 1.11532;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2361;
break;
case 105:
/* Constant r2r3dist */
CONSTANTS[1] = 2.07959;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0199;
break;
case 106:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0884372;
/* Constant r2r3dist */
CONSTANTS[2] = 9.34866;
break;
case 107:
/* Constant r2r3dist */
CONSTANTS[1] = -0.759000;
/* Constant r2r3dist */
CONSTANTS[2] = 9.26611;
break;
case 108:
/* Constant r2r3dist */
CONSTANTS[1] = 0.227441;
/* Constant r2r3dist */
CONSTANTS[2] = 9.62424;
break;
case 109:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0782673;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9109;
break;
case 110:
/* Constant r2r3dist */
CONSTANTS[1] = -0.856699;
/* Constant r2r3dist */
CONSTANTS[2] = 9.54973;
break;
case 111:
/* Constant r2r3dist */
CONSTANTS[1] = -0.420034;
/* Constant r2r3dist */
CONSTANTS[2] = 9.77135;
break;
case 112:
/* Constant r2r3dist */
CONSTANTS[1] = -1.53474;
/* Constant r2r3dist */
CONSTANTS[2] = 9.15775;
break;
case 113:
/* Constant r2r3dist */
CONSTANTS[1] = -1.39898;
/* Constant r2r3dist */
CONSTANTS[2] = 9.18780;
break;
case 114:
/* Constant r2r3dist */
CONSTANTS[1] = -0.695713;
/* Constant r2r3dist */
CONSTANTS[2] = 9.34482;
break;
case 115:
/* Constant r2r3dist */
CONSTANTS[1] = 1.56354;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4519;
break;
case 116:
/* Constant r2r3dist */
CONSTANTS[1] = 0.521136;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81228;
break;
case 117:
/* Constant r2r3dist */
CONSTANTS[1] = -1.44351;
/* Constant r2r3dist */
CONSTANTS[2] = 9.93480;
break;
case 118:
/* Constant r2r3dist */
CONSTANTS[1] = 1.21405;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9245;
break;
case 119:
/* Constant r2r3dist */
CONSTANTS[1] = -0.233544;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75290;
break;
case 120:
/* Constant r2r3dist */
CONSTANTS[1] = -0.810330;
/* Constant r2r3dist */
CONSTANTS[2] = 9.82137;
break;
case 121:
/* Constant r2r3dist */
CONSTANTS[1] = -2.34638;
/* Constant r2r3dist */
CONSTANTS[2] = 8.15355;
break;
case 122:
/* Constant r2r3dist */
CONSTANTS[1] = -1.02356;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95785;
break;
case 123:
/* Constant r2r3dist */
CONSTANTS[1] = -2.01851;
/* Constant r2r3dist */
CONSTANTS[2] = 8.67204;
break;
case 124:
/* Constant r2r3dist */
CONSTANTS[1] = 0.442318;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7635;
break;
case 125:
/* Constant r2r3dist */
CONSTANTS[1] = -0.354193;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4928;
break;
case 126:
/* Constant r2r3dist */
CONSTANTS[1] = -0.919096;
/* Constant r2r3dist */
CONSTANTS[2] = 9.54161;
break;
case 127:
/* Constant r2r3dist */
CONSTANTS[1] = 0.532995;
/* Constant r2r3dist */
CONSTANTS[2] = 9.90541;
break;
case 128:
/* Constant r2r3dist */
CONSTANTS[1] = -1.04530;
/* Constant r2r3dist */
CONSTANTS[2] = 9.44963;
break;
case 129:
/* Constant r2r3dist */
CONSTANTS[1] = -0.409808;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75445;
break;
case 130:
/* Constant r2r3dist */
CONSTANTS[1] = -0.604661;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66697;
break;
case 131:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0262765;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3651;
break;
case 132:
/* Constant r2r3dist */
CONSTANTS[1] = 0.608419;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99240;
break;
case 133:
/* Constant r2r3dist */
CONSTANTS[1] = 1.20405;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0850;
break;
case 134:
/* Constant r2r3dist */
CONSTANTS[1] = -0.399546;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43459;
break;
case 135:
/* Constant r2r3dist */
CONSTANTS[1] = 0.507774;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81893;
break;
case 136:
/* Constant r2r3dist */
CONSTANTS[1] = -1.39854;
/* Constant r2r3dist */
CONSTANTS[2] = 9.07308;
break;
case 137:
/* Constant r2r3dist */
CONSTANTS[1] = 0.156823;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8610;
break;
case 138:
/* Constant r2r3dist */
CONSTANTS[1] = 0.274586;
/* Constant r2r3dist */
CONSTANTS[2] = 9.59653;
break;
case 139:
/* Constant r2r3dist */
CONSTANTS[1] = -0.302830;
/* Constant r2r3dist */
CONSTANTS[2] = 9.21990;
break;
case 140:
/* Constant r2r3dist */
CONSTANTS[1] = -1.65531;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2439;
break;
case 141:
/* Constant r2r3dist */
CONSTANTS[1] = 0.340942;
/* Constant r2r3dist */
CONSTANTS[2] = 9.14995;
break;
case 142:
/* Constant r2r3dist */
CONSTANTS[1] = -1.13199;
/* Constant r2r3dist */
CONSTANTS[2] = 9.48058;
break;
case 143:
/* Constant r2r3dist */
CONSTANTS[1] = 0.562440;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3752;
break;
case 144:
/* Constant r2r3dist */
CONSTANTS[1] = 1.41276;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2429;
break;
case 145:
/* Constant r2r3dist */
CONSTANTS[1] = 1.21882;
/* Constant r2r3dist */
CONSTANTS[2] = 11.6472;
break;
case 146:
/* Constant r2r3dist */
CONSTANTS[1] = -0.739580;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27707;
break;
case 147:
/* Constant r2r3dist */
CONSTANTS[1] = -1.11941;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27118;
break;
case 148:
/* Constant r2r3dist */
CONSTANTS[1] = 1.56583;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7700;
break;
case 149:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0612810;
/* Constant r2r3dist */
CONSTANTS[2] = 9.68441;
break;
case 150:
/* Constant r2r3dist */
CONSTANTS[1] = 0.352526;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1031;
break;
case 151:
/* Constant r2r3dist */
CONSTANTS[1] = 0.807038;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6675;
break;
case 152:
/* Constant r2r3dist */
CONSTANTS[1] = 0.856719;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4921;
break;
case 153:
/* Constant r2r3dist */
CONSTANTS[1] = -1.23230;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53305;
break;
case 154:
/* Constant r2r3dist */
CONSTANTS[1] = -0.750940;
/* Constant r2r3dist */
CONSTANTS[2] = 9.58842;
break;
case 155:
/* Constant r2r3dist */
CONSTANTS[1] = 0.911477;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81170;
break;
case 156:
/* Constant r2r3dist */
CONSTANTS[1] = -0.855538;
/* Constant r2r3dist */
CONSTANTS[2] = 9.26032;
break;
case 157:
/* Constant r2r3dist */
CONSTANTS[1] = 0.407426;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4182;
break;
case 158:
/* Constant r2r3dist */
CONSTANTS[1] = -0.744870;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6346;
break;
case 159:
/* Constant r2r3dist */
CONSTANTS[1] = 1.53028;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0655;
break;
case 160:
/* Constant r2r3dist */
CONSTANTS[1] = 0.528775;
/* Constant r2r3dist */
CONSTANTS[2] = 9.55617;
break;
case 161:
/* Constant r2r3dist */
CONSTANTS[1] = -0.262340;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50097;
break;
case 162:
/* Constant r2r3dist */
CONSTANTS[1] = -0.939561;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96487;
break;
case 163:
/* Constant r2r3dist */
CONSTANTS[1] = 0.572930;
/* Constant r2r3dist */
CONSTANTS[2] = 9.85895;
break;
case 164:
/* Constant r2r3dist */
CONSTANTS[1] = 0.787766;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3281;
break;
case 165:
/* Constant r2r3dist */
CONSTANTS[1] = -0.788928;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0178;
break;
case 166:
/* Constant r2r3dist */
CONSTANTS[1] = 1.20733;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5219;
break;
case 167:
/* Constant r2r3dist */
CONSTANTS[1] = -1.26415;
/* Constant r2r3dist */
CONSTANTS[2] = 8.58893;
break;
case 168:
/* Constant r2r3dist */
CONSTANTS[1] = 0.423778;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3135;
break;
case 169:
/* Constant r2r3dist */
CONSTANTS[1] = 1.05752;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6387;
break;
case 170:
/* Constant r2r3dist */
CONSTANTS[1] = 1.18197;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6165;
break;
case 171:
/* Constant r2r3dist */
CONSTANTS[1] = 0.404062;
/* Constant r2r3dist */
CONSTANTS[2] = 9.90065;
break;
case 172:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0720385;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0298;
break;
case 173:
/* Constant r2r3dist */
CONSTANTS[1] = 0.610309;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2138;
break;
case 174:
/* Constant r2r3dist */
CONSTANTS[1] = 0.579604;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8719;
break;
case 175:
/* Constant r2r3dist */
CONSTANTS[1] = -0.562992;
/* Constant r2r3dist */
CONSTANTS[2] = 9.98981;
break;
case 176:
/* Constant r2r3dist */
CONSTANTS[1] = -0.516771;
/* Constant r2r3dist */
CONSTANTS[2] = 8.44386;
break;
case 177:
/* Constant r2r3dist */
CONSTANTS[1] = -0.576560;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50838;
break;
case 178:
/* Constant r2r3dist */
CONSTANTS[1] = -1.67534;
/* Constant r2r3dist */
CONSTANTS[2] = 9.34918;
break;
case 179:
/* Constant r2r3dist */
CONSTANTS[1] = -0.845814;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31171;
break;
case 180:
/* Constant r2r3dist */
CONSTANTS[1] = 2.00670;
/* Constant r2r3dist */
CONSTANTS[2] = 11.9930;
break;
case 181:
/* Constant r2r3dist */
CONSTANTS[1] = 0.410152;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0540;
break;
case 182:
/* Constant r2r3dist */
CONSTANTS[1] = -1.34287;
/* Constant r2r3dist */
CONSTANTS[2] = 8.49243;
break;
case 183:
/* Constant r2r3dist */
CONSTANTS[1] = 2.33060;
/* Constant r2r3dist */
CONSTANTS[2] = 11.9768;
break;
case 184:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0960324;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0347;
break;
case 185:
/* Constant r2r3dist */
CONSTANTS[1] = -0.519482;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0400;
break;
case 186:
/* Constant r2r3dist */
CONSTANTS[1] = 0.267195;
/* Constant r2r3dist */
CONSTANTS[2] = 9.65796;
break;
case 187:
/* Constant r2r3dist */
CONSTANTS[1] = 0.350331;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2407;
break;
case 188:
/* Constant r2r3dist */
CONSTANTS[1] = -0.345025;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81514;
break;
case 189:
/* Constant r2r3dist */
CONSTANTS[1] = -0.658426;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2496;
break;
case 190:
/* Constant r2r3dist */
CONSTANTS[1] = 0.560210;
/* Constant r2r3dist */
CONSTANTS[2] = 9.82967;
break;
case 191:
/* Constant r2r3dist */
CONSTANTS[1] = -0.478582;
/* Constant r2r3dist */
CONSTANTS[2] = 9.35953;
break;
case 192:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0428047;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7671;
break;
case 193:
/* Constant r2r3dist */
CONSTANTS[1] = -0.707723;
/* Constant r2r3dist */
CONSTANTS[2] = 8.94048;
break;
case 194:
/* Constant r2r3dist */
CONSTANTS[1] = 0.124503;
/* Constant r2r3dist */
CONSTANTS[2] = 9.16413;
break;
case 195:
/* Constant r2r3dist */
CONSTANTS[1] = -0.473808;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0087;
break;
case 196:
/* Constant r2r3dist */
CONSTANTS[1] = 2.31470;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5245;
break;
case 197:
/* Constant r2r3dist */
CONSTANTS[1] = -1.94560;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50325;
break;
case 198:
/* Constant r2r3dist */
CONSTANTS[1] = -0.715881;
/* Constant r2r3dist */
CONSTANTS[2] = 8.87352;
break;
case 199:
/* Constant r2r3dist */
CONSTANTS[1] = -1.03730;
/* Constant r2r3dist */
CONSTANTS[2] = 9.48532;
break;
case 200:
/* Constant r2r3dist */
CONSTANTS[1] = -0.497264;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0143;
break;
case 201:
/* Constant r2r3dist */
CONSTANTS[1] = -1.10151;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66712;
break;
case 202:
/* Constant r2r3dist */
CONSTANTS[1] = 1.96568;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5761;
break;
case 203:
/* Constant r2r3dist */
CONSTANTS[1] = 1.34832;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1216;
break;
case 204:
/* Constant r2r3dist */
CONSTANTS[1] = -2.43467;
/* Constant r2r3dist */
CONSTANTS[2] = 9.06855;
break;
case 205:
/* Constant r2r3dist */
CONSTANTS[1] = -0.371626;
/* Constant r2r3dist */
CONSTANTS[2] = 8.91190;
break;
case 206:
/* Constant r2r3dist */
CONSTANTS[1] = -0.605104;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5041;
break;
case 207:
/* Constant r2r3dist */
CONSTANTS[1] = 2.23979;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2441;
break;
case 208:
/* Constant r2r3dist */
CONSTANTS[1] = 0.130584;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97309;
break;
case 209:
/* Constant r2r3dist */
CONSTANTS[1] = -1.60687;
/* Constant r2r3dist */
CONSTANTS[2] = 9.12180;
break;
case 210:
/* Constant r2r3dist */
CONSTANTS[1] = -0.834117;
/* Constant r2r3dist */
CONSTANTS[2] = 8.92628;
break;
case 211:
/* Constant r2r3dist */
CONSTANTS[1] = -0.169646;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2638;
break;
case 212:
/* Constant r2r3dist */
CONSTANTS[1] = -0.776054;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7188;
break;
case 213:
/* Constant r2r3dist */
CONSTANTS[1] = -1.28258;
/* Constant r2r3dist */
CONSTANTS[2] = 8.80018;
break;
case 214:
/* Constant r2r3dist */
CONSTANTS[1] = 2.48610;
/* Constant r2r3dist */
CONSTANTS[2] = 11.4168;
break;
case 215:
/* Constant r2r3dist */
CONSTANTS[1] = -0.440228;
/* Constant r2r3dist */
CONSTANTS[2] = 9.62551;
break;
case 216:
/* Constant r2r3dist */
CONSTANTS[1] = -0.402632;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4416;
break;
case 217:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0529521;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6863;
break;
case 218:
/* Constant r2r3dist */
CONSTANTS[1] = 0.618992;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7826;
break;
case 219:
/* Constant r2r3dist */
CONSTANTS[1] = -0.300217;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4268;
break;
case 220:
/* Constant r2r3dist */
CONSTANTS[1] = -0.552240;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0846;
break;
case 221:
/* Constant r2r3dist */
CONSTANTS[1] = -0.983473;
/* Constant r2r3dist */
CONSTANTS[2] = 9.54819;
break;
case 222:
/* Constant r2r3dist */
CONSTANTS[1] = -0.720989;
/* Constant r2r3dist */
CONSTANTS[2] = 9.76708;
break;
case 223:
/* Constant r2r3dist */
CONSTANTS[1] = -0.313844;
/* Constant r2r3dist */
CONSTANTS[2] = 9.40583;
break;
case 224:
/* Constant r2r3dist */
CONSTANTS[1] = -0.114939;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0928;
break;
case 225:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0309663;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0549;
break;
case 226:
/* Constant r2r3dist */
CONSTANTS[1] = 0.972894;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6256;
break;
case 227:
/* Constant r2r3dist */
CONSTANTS[1] = -1.30449;
/* Constant r2r3dist */
CONSTANTS[2] = 8.08119;
break;
case 228:
/* Constant r2r3dist */
CONSTANTS[1] = -0.370204;
/* Constant r2r3dist */
CONSTANTS[2] = 9.54435;
break;
case 229:
/* Constant r2r3dist */
CONSTANTS[1] = -0.406109;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43918;
break;
case 230:
/* Constant r2r3dist */
CONSTANTS[1] = -0.794491;
/* Constant r2r3dist */
CONSTANTS[2] = 9.11377;
break;
case 231:
/* Constant r2r3dist */
CONSTANTS[1] = 0.388608;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1221;
break;
case 232:
/* Constant r2r3dist */
CONSTANTS[1] = 0.194060;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8202;
break;
case 233:
/* Constant r2r3dist */
CONSTANTS[1] = -1.15400;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53673;
break;
case 234:
/* Constant r2r3dist */
CONSTANTS[1] = 1.48574;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5935;
break;
case 235:
/* Constant r2r3dist */
CONSTANTS[1] = 0.791562;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3585;
break;
case 236:
/* Constant r2r3dist */
CONSTANTS[1] = 0.202493;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2991;
break;
case 237:
/* Constant r2r3dist */
CONSTANTS[1] = 0.410946;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8463;
break;
case 238:
/* Constant r2r3dist */
CONSTANTS[1] = 0.853777;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2058;
break;
case 239:
/* Constant r2r3dist */
CONSTANTS[1] = -0.993165;
/* Constant r2r3dist */
CONSTANTS[2] = 9.49733;
break;
case 240:
/* Constant r2r3dist */
CONSTANTS[1] = -0.534665;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3356;
break;
case 241:
/* Constant r2r3dist */
CONSTANTS[1] = -0.237033;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1159;
break;
case 242:
/* Constant r2r3dist */
CONSTANTS[1] = -0.482345;
/* Constant r2r3dist */
CONSTANTS[2] = 9.41361;
break;
case 243:
/* Constant r2r3dist */
CONSTANTS[1] = -0.638438;
/* Constant r2r3dist */
CONSTANTS[2] = 9.35939;
break;
case 244:
/* Constant r2r3dist */
CONSTANTS[1] = -1.64067;
/* Constant r2r3dist */
CONSTANTS[2] = 8.60358;
break;
case 245:
/* Constant r2r3dist */
CONSTANTS[1] = 0.215740;
/* Constant r2r3dist */
CONSTANTS[2] = 9.68979;
break;
case 246:
/* Constant r2r3dist */
CONSTANTS[1] = 0.433980;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2781;
break;
case 247:
/* Constant r2r3dist */
CONSTANTS[1] = 1.17371;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78035;
break;
case 248:
/* Constant r2r3dist */
CONSTANTS[1] = -1.01766;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5270;
break;
case 249:
/* Constant r2r3dist */
CONSTANTS[1] = 1.58844;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1309;
break;
case 250:
/* Constant r2r3dist */
CONSTANTS[1] = 1.20276;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7994;
break;
case 251:
/* Constant r2r3dist */
CONSTANTS[1] = 0.524435;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1868;
break;
case 252:
/* Constant r2r3dist */
CONSTANTS[1] = -1.32934;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0750;
break;
case 253:
/* Constant r2r3dist */
CONSTANTS[1] = -1.01268;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78126;
break;
case 254:
/* Constant r2r3dist */
CONSTANTS[1] = 1.40147;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8712;
break;
case 255:
/* Constant r2r3dist */
CONSTANTS[1] = 0.704961;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97849;
break;
case 256:
/* Constant r2r3dist */
CONSTANTS[1] = 0.242236;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1719;
break;
case 257:
/* Constant r2r3dist */
CONSTANTS[1] = -0.117170;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95008;
break;
case 258:
/* Constant r2r3dist */
CONSTANTS[1] = 1.73312;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0988;
break;
case 259:
/* Constant r2r3dist */
CONSTANTS[1] = 0.271606;
/* Constant r2r3dist */
CONSTANTS[2] = 9.93099;
break;
case 260:
/* Constant r2r3dist */
CONSTANTS[1] = -0.905859;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3144;
break;
case 261:
/* Constant r2r3dist */
CONSTANTS[1] = 0.175520;
/* Constant r2r3dist */
CONSTANTS[2] = 9.80816;
break;
case 262:
/* Constant r2r3dist */
CONSTANTS[1] = 0.412280;
/* Constant r2r3dist */
CONSTANTS[2] = 9.93709;
break;
case 263:
/* Constant r2r3dist */
CONSTANTS[1] = 2.99631;
/* Constant r2r3dist */
CONSTANTS[2] = 12.0821;
break;
case 264:
/* Constant r2r3dist */
CONSTANTS[1] = -3.01763;
/* Constant r2r3dist */
CONSTANTS[2] = 8.33325;
break;
case 265:
/* Constant r2r3dist */
CONSTANTS[1] = -0.727385;
/* Constant r2r3dist */
CONSTANTS[2] = 8.99378;
break;
case 266:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0157086;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5865;
break;
case 267:
/* Constant r2r3dist */
CONSTANTS[1] = -0.286581;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7517;
break;
case 268:
/* Constant r2r3dist */
CONSTANTS[1] = 0.938769;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3930;
break;
case 269:
/* Constant r2r3dist */
CONSTANTS[1] = 0.723831;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78356;
break;
case 270:
/* Constant r2r3dist */
CONSTANTS[1] = 0.830976;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8575;
break;
case 271:
/* Constant r2r3dist */
CONSTANTS[1] = -0.704115;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45562;
break;
case 272:
/* Constant r2r3dist */
CONSTANTS[1] = 0.408444;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4821;
break;
case 273:
/* Constant r2r3dist */
CONSTANTS[1] = -0.486152;
/* Constant r2r3dist */
CONSTANTS[2] = 9.38259;
break;
case 274:
/* Constant r2r3dist */
CONSTANTS[1] = 1.39911;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6266;
break;
case 275:
/* Constant r2r3dist */
CONSTANTS[1] = 0.732713;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2012;
break;
case 276:
/* Constant r2r3dist */
CONSTANTS[1] = -0.00993686;
/* Constant r2r3dist */
CONSTANTS[2] = 9.69335;
break;
case 277:
/* Constant r2r3dist */
CONSTANTS[1] = -0.869886;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0473;
break;
case 278:
/* Constant r2r3dist */
CONSTANTS[1] = 0.709321;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8147;
break;
case 279:
/* Constant r2r3dist */
CONSTANTS[1] = -0.739368;
/* Constant r2r3dist */
CONSTANTS[2] = 9.05034;
break;
case 280:
/* Constant r2r3dist */
CONSTANTS[1] = 1.12651;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4266;
break;
case 281:
/* Constant r2r3dist */
CONSTANTS[1] = -0.824310;
/* Constant r2r3dist */
CONSTANTS[2] = 9.01544;
break;
case 282:
/* Constant r2r3dist */
CONSTANTS[1] = -0.681181;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33498;
break;
case 283:
/* Constant r2r3dist */
CONSTANTS[1] = -0.814640;
/* Constant r2r3dist */
CONSTANTS[2] = 9.32083;
break;
case 284:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0828610;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1460;
break;
case 285:
/* Constant r2r3dist */
CONSTANTS[1] = -0.635449;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3652;
break;
case 286:
/* Constant r2r3dist */
CONSTANTS[1] = 1.06753;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9575;
break;
case 287:
/* Constant r2r3dist */
CONSTANTS[1] = -0.724906;
/* Constant r2r3dist */
CONSTANTS[2] = 9.25978;
break;
case 288:
/* Constant r2r3dist */
CONSTANTS[1] = 1.25197;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0278;
break;
case 289:
/* Constant r2r3dist */
CONSTANTS[1] = -0.364233;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5406;
break;
case 290:
/* Constant r2r3dist */
CONSTANTS[1] = -0.990632;
/* Constant r2r3dist */
CONSTANTS[2] = 9.18209;
break;
case 291:
/* Constant r2r3dist */
CONSTANTS[1] = 0.741541;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6190;
break;
case 292:
/* Constant r2r3dist */
CONSTANTS[1] = -0.354966;
/* Constant r2r3dist */
CONSTANTS[2] = 9.84978;
break;
case 293:
/* Constant r2r3dist */
CONSTANTS[1] = -1.14659;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4718;
break;
case 294:
/* Constant r2r3dist */
CONSTANTS[1] = -0.236549;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2378;
break;
case 295:
/* Constant r2r3dist */
CONSTANTS[1] = -0.582285;
/* Constant r2r3dist */
CONSTANTS[2] = 9.52304;
break;
case 296:
/* Constant r2r3dist */
CONSTANTS[1] = 1.44766;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3251;
break;
case 297:
/* Constant r2r3dist */
CONSTANTS[1] = 0.728650;
/* Constant r2r3dist */
CONSTANTS[2] = 9.74946;
break;
case 298:
/* Constant r2r3dist */
CONSTANTS[1] = 0.914210;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6848;
break;
case 299:
/* Constant r2r3dist */
CONSTANTS[1] = -0.356111;
/* Constant r2r3dist */
CONSTANTS[2] = 9.20544;
break;
case 300:
/* Constant r2r3dist */
CONSTANTS[1] = 1.01582;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6114;
break;
case 301:
/* Constant r2r3dist */
CONSTANTS[1] = -0.361836;
/* Constant r2r3dist */
CONSTANTS[2] = 9.57746;
break;
case 302:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0843881;
/* Constant r2r3dist */
CONSTANTS[2] = 9.80090;
break;
case 303:
/* Constant r2r3dist */
CONSTANTS[1] = -1.27522;
/* Constant r2r3dist */
CONSTANTS[2] = 9.74307;
break;
case 304:
/* Constant r2r3dist */
CONSTANTS[1] = 0.229748;
/* Constant r2r3dist */
CONSTANTS[2] = 9.15077;
break;
case 305:
/* Constant r2r3dist */
CONSTANTS[1] = 2.07026;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4920;
break;
case 306:
/* Constant r2r3dist */
CONSTANTS[1] = 0.246223;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31843;
break;
case 307:
/* Constant r2r3dist */
CONSTANTS[1] = -0.732375;
/* Constant r2r3dist */
CONSTANTS[2] = 9.91271;
break;
case 308:
/* Constant r2r3dist */
CONSTANTS[1] = -1.30478;
/* Constant r2r3dist */
CONSTANTS[2] = 9.04656;
break;
case 309:
/* Constant r2r3dist */
CONSTANTS[1] = -0.978047;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66303;
break;
case 310:
/* Constant r2r3dist */
CONSTANTS[1] = -0.304513;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0827;
break;
case 311:
/* Constant r2r3dist */
CONSTANTS[1] = -0.150403;
/* Constant r2r3dist */
CONSTANTS[2] = 9.56690;
break;
case 312:
/* Constant r2r3dist */
CONSTANTS[1] = -1.54318;
/* Constant r2r3dist */
CONSTANTS[2] = 8.35636;
break;
case 313:
/* Constant r2r3dist */
CONSTANTS[1] = -0.993495;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0807;
break;
case 314:
/* Constant r2r3dist */
CONSTANTS[1] = -0.183647;
/* Constant r2r3dist */
CONSTANTS[2] = 9.89752;
break;
case 315:
/* Constant r2r3dist */
CONSTANTS[1] = 0.249767;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97534;
break;
case 316:
/* Constant r2r3dist */
CONSTANTS[1] = 1.46905;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3046;
break;
case 317:
/* Constant r2r3dist */
CONSTANTS[1] = 0.610601;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2472;
break;
case 318:
/* Constant r2r3dist */
CONSTANTS[1] = -0.198613;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78133;
break;
case 319:
/* Constant r2r3dist */
CONSTANTS[1] = 0.775906;
/* Constant r2r3dist */
CONSTANTS[2] = 11.5253;
break;
case 320:
/* Constant r2r3dist */
CONSTANTS[1] = -1.78648;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66713;
break;
case 321:
/* Constant r2r3dist */
CONSTANTS[1] = -0.496963;
/* Constant r2r3dist */
CONSTANTS[2] = 9.61829;
break;
case 322:
/* Constant r2r3dist */
CONSTANTS[1] = -0.100650;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7001;
break;
case 323:
/* Constant r2r3dist */
CONSTANTS[1] = -0.944006;
/* Constant r2r3dist */
CONSTANTS[2] = 9.89879;
break;
case 324:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0404770;
/* Constant r2r3dist */
CONSTANTS[2] = 9.41798;
break;
case 325:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0147684;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53036;
break;
case 326:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0887674;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1302;
break;
case 327:
/* Constant r2r3dist */
CONSTANTS[1] = 0.782595;
/* Constant r2r3dist */
CONSTANTS[2] = 9.88518;
break;
case 328:
/* Constant r2r3dist */
CONSTANTS[1] = -0.489841;
/* Constant r2r3dist */
CONSTANTS[2] = 9.65013;
break;
case 329:
/* Constant r2r3dist */
CONSTANTS[1] = -0.322764;
/* Constant r2r3dist */
CONSTANTS[2] = 9.05272;
break;
case 330:
/* Constant r2r3dist */
CONSTANTS[1] = -0.354850;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4625;
break;
case 331:
/* Constant r2r3dist */
CONSTANTS[1] = 1.08722;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6515;
break;
case 332:
/* Constant r2r3dist */
CONSTANTS[1] = 0.257476;
/* Constant r2r3dist */
CONSTANTS[2] = 9.85615;
break;
case 333:
/* Constant r2r3dist */
CONSTANTS[1] = -0.730117;
/* Constant r2r3dist */
CONSTANTS[2] = 9.69705;
break;
case 334:
/* Constant r2r3dist */
CONSTANTS[1] = 0.315660;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5815;
break;
case 335:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0507933;
/* Constant r2r3dist */
CONSTANTS[2] = 9.36234;
break;
case 336:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0895123;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4958;
break;
case 337:
/* Constant r2r3dist */
CONSTANTS[1] = 0.507961;
/* Constant r2r3dist */
CONSTANTS[2] = 9.42056;
break;
case 338:
/* Constant r2r3dist */
CONSTANTS[1] = 2.32480;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2680;
break;
case 339:
/* Constant r2r3dist */
CONSTANTS[1] = 0.701741;
/* Constant r2r3dist */
CONSTANTS[2] = 9.86404;
break;
case 340:
/* Constant r2r3dist */
CONSTANTS[1] = -0.830073;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1170;
break;
case 341:
/* Constant r2r3dist */
CONSTANTS[1] = 0.806094;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95862;
break;
case 342:
/* Constant r2r3dist */
CONSTANTS[1] = 0.399277;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0397;
break;
case 343:
/* Constant r2r3dist */
CONSTANTS[1] = 1.33871;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4454;
break;
case 344:
/* Constant r2r3dist */
CONSTANTS[1] = -0.257457;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75569;
break;
case 345:
/* Constant r2r3dist */
CONSTANTS[1] = 0.152115;
/* Constant r2r3dist */
CONSTANTS[2] = 9.71261;
break;
case 346:
/* Constant r2r3dist */
CONSTANTS[1] = 0.151250;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1712;
break;
case 347:
/* Constant r2r3dist */
CONSTANTS[1] = 0.591541;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1063;
break;
case 348:
/* Constant r2r3dist */
CONSTANTS[1] = 1.37580;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2575;
break;
case 349:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0823245;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75813;
break;
case 350:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0215586;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0095;
break;
case 351:
/* Constant r2r3dist */
CONSTANTS[1] = -0.208262;
/* Constant r2r3dist */
CONSTANTS[2] = 9.70960;
break;
case 352:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0454611;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0737;
break;
case 353:
/* Constant r2r3dist */
CONSTANTS[1] = -0.181892;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5023;
break;
case 354:
/* Constant r2r3dist */
CONSTANTS[1] = 0.956651;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8997;
break;
case 355:
/* Constant r2r3dist */
CONSTANTS[1] = -0.432014;
/* Constant r2r3dist */
CONSTANTS[2] = 9.23958;
break;
case 356:
/* Constant r2r3dist */
CONSTANTS[1] = 1.12978;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5368;
break;
case 357:
/* Constant r2r3dist */
CONSTANTS[1] = -0.524785;
/* Constant r2r3dist */
CONSTANTS[2] = 8.92056;
break;
case 358:
/* Constant r2r3dist */
CONSTANTS[1] = -0.961467;
/* Constant r2r3dist */
CONSTANTS[2] = 9.47764;
break;
case 359:
/* Constant r2r3dist */
CONSTANTS[1] = -0.902691;
/* Constant r2r3dist */
CONSTANTS[2] = 9.24721;
break;
case 360:
/* Constant r2r3dist */
CONSTANTS[1] = 0.550728;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8911;
break;
case 361:
/* Constant r2r3dist */
CONSTANTS[1] = -0.334659;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95086;
break;
case 362:
/* Constant r2r3dist */
CONSTANTS[1] = 0.923508;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97101;
break;
case 363:
/* Constant r2r3dist */
CONSTANTS[1] = 1.23269;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7724;
break;
case 364:
/* Constant r2r3dist */
CONSTANTS[1] = 0.448378;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3599;
break;
case 365:
/* Constant r2r3dist */
CONSTANTS[1] = 1.12161;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6718;
break;
case 366:
/* Constant r2r3dist */
CONSTANTS[1] = -0.974888;
/* Constant r2r3dist */
CONSTANTS[2] = 9.05492;
break;
case 367:
/* Constant r2r3dist */
CONSTANTS[1] = 0.226185;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4324;
break;
case 368:
/* Constant r2r3dist */
CONSTANTS[1] = 0.546751;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2115;
break;
case 369:
/* Constant r2r3dist */
CONSTANTS[1] = -0.344231;
/* Constant r2r3dist */
CONSTANTS[2] = 9.62057;
break;
case 370:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21701;
/* Constant r2r3dist */
CONSTANTS[2] = 9.07301;
break;
case 371:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0575525;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1561;
break;
case 372:
/* Constant r2r3dist */
CONSTANTS[1] = -0.104721;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3032;
break;
case 373:
/* Constant r2r3dist */
CONSTANTS[1] = 0.636903;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3563;
break;
case 374:
/* Constant r2r3dist */
CONSTANTS[1] = 0.664935;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9770;
break;
case 375:
/* Constant r2r3dist */
CONSTANTS[1] = -0.492528;
/* Constant r2r3dist */
CONSTANTS[2] = 9.35389;
break;
case 376:
/* Constant r2r3dist */
CONSTANTS[1] = -1.11057;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66904;
break;
case 377:
/* Constant r2r3dist */
CONSTANTS[1] = 0.965508;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0908;
break;
case 378:
/* Constant r2r3dist */
CONSTANTS[1] = -1.72447;
/* Constant r2r3dist */
CONSTANTS[2] = 8.16322;
break;
case 379:
/* Constant r2r3dist */
CONSTANTS[1] = -0.147483;
/* Constant r2r3dist */
CONSTANTS[2] = 9.74561;
break;
case 380:
/* Constant r2r3dist */
CONSTANTS[1] = 1.12219;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3062;
break;
case 381:
/* Constant r2r3dist */
CONSTANTS[1] = 1.93998;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0952;
break;
case 382:
/* Constant r2r3dist */
CONSTANTS[1] = -1.29926;
/* Constant r2r3dist */
CONSTANTS[2] = 8.77189;
break;
case 383:
/* Constant r2r3dist */
CONSTANTS[1] = -0.199801;
/* Constant r2r3dist */
CONSTANTS[2] = 9.37340;
break;
case 384:
/* Constant r2r3dist */
CONSTANTS[1] = 0.585881;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1899;
break;
case 385:
/* Constant r2r3dist */
CONSTANTS[1] = -0.850936;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99370;
break;
case 386:
/* Constant r2r3dist */
CONSTANTS[1] = -0.407387;
/* Constant r2r3dist */
CONSTANTS[2] = 9.94165;
break;
case 387:
/* Constant r2r3dist */
CONSTANTS[1] = 0.715235;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5723;
break;
case 388:
/* Constant r2r3dist */
CONSTANTS[1] = 0.546907;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8288;
break;
case 389:
/* Constant r2r3dist */
CONSTANTS[1] = 0.683153;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4300;
break;
case 390:
/* Constant r2r3dist */
CONSTANTS[1] = 1.17422;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4537;
break;
case 391:
/* Constant r2r3dist */
CONSTANTS[1] = 1.61563;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5203;
break;
case 392:
/* Constant r2r3dist */
CONSTANTS[1] = 0.935109;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5779;
break;
case 393:
/* Constant r2r3dist */
CONSTANTS[1] = -0.376379;
/* Constant r2r3dist */
CONSTANTS[2] = 9.67100;
break;
case 394:
/* Constant r2r3dist */
CONSTANTS[1] = 0.412832;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6364;
break;
case 395:
/* Constant r2r3dist */
CONSTANTS[1] = 1.18668;
/* Constant r2r3dist */
CONSTANTS[2] = 9.58270;
break;
case 396:
/* Constant r2r3dist */
CONSTANTS[1] = 0.888008;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31945;
break;
case 397:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21579;
/* Constant r2r3dist */
CONSTANTS[2] = 9.32189;
break;
case 398:
/* Constant r2r3dist */
CONSTANTS[1] = -0.965409;
/* Constant r2r3dist */
CONSTANTS[2] = 9.34154;
break;
case 399:
/* Constant r2r3dist */
CONSTANTS[1] = 0.351405;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5833;
break;
case 400:
/* Constant r2r3dist */
CONSTANTS[1] = 1.27509;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1668;
break;
case 401:
/* Constant r2r3dist */
CONSTANTS[1] = 2.27365;
/* Constant r2r3dist */
CONSTANTS[2] = 12.2267;
break;
case 402:
/* Constant r2r3dist */
CONSTANTS[1] = -0.346561;
/* Constant r2r3dist */
CONSTANTS[2] = 9.38031;
break;
case 403:
/* Constant r2r3dist */
CONSTANTS[1] = 0.539864;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3647;
break;
case 404:
/* Constant r2r3dist */
CONSTANTS[1] = 3.27730;
/* Constant r2r3dist */
CONSTANTS[2] = 11.6012;
break;
case 405:
/* Constant r2r3dist */
CONSTANTS[1] = -0.817065;
/* Constant r2r3dist */
CONSTANTS[2] = 9.77023;
break;
case 406:
/* Constant r2r3dist */
CONSTANTS[1] = 1.65362;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2156;
break;
case 407:
/* Constant r2r3dist */
CONSTANTS[1] = -1.75359;
/* Constant r2r3dist */
CONSTANTS[2] = 8.66985;
break;
case 408:
/* Constant r2r3dist */
CONSTANTS[1] = -1.49925;
/* Constant r2r3dist */
CONSTANTS[2] = 9.47209;
break;
case 409:
/* Constant r2r3dist */
CONSTANTS[1] = 0.111396;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3601;
break;
case 410:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0907533;
/* Constant r2r3dist */
CONSTANTS[2] = 9.41955;
break;
case 411:
/* Constant r2r3dist */
CONSTANTS[1] = -0.663116;
/* Constant r2r3dist */
CONSTANTS[2] = 9.64686;
break;
case 412:
/* Constant r2r3dist */
CONSTANTS[1] = -1.57689;
/* Constant r2r3dist */
CONSTANTS[2] = 9.20958;
break;
case 413:
/* Constant r2r3dist */
CONSTANTS[1] = 1.05311;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4465;
break;
case 414:
/* Constant r2r3dist */
CONSTANTS[1] = -0.486301;
/* Constant r2r3dist */
CONSTANTS[2] = 9.91196;
break;
case 415:
/* Constant r2r3dist */
CONSTANTS[1] = -1.12938;
/* Constant r2r3dist */
CONSTANTS[2] = 9.08112;
break;
case 416:
/* Constant r2r3dist */
CONSTANTS[1] = 0.646075;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1921;
break;
case 417:
/* Constant r2r3dist */
CONSTANTS[1] = -0.704567;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66739;
break;
case 418:
/* Constant r2r3dist */
CONSTANTS[1] = -0.752939;
/* Constant r2r3dist */
CONSTANTS[2] = 9.26568;
break;
case 419:
/* Constant r2r3dist */
CONSTANTS[1] = 0.899789;
/* Constant r2r3dist */
CONSTANTS[2] = 8.62899;
break;
case 420:
/* Constant r2r3dist */
CONSTANTS[1] = 1.43674;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1109;
break;
case 421:
/* Constant r2r3dist */
CONSTANTS[1] = 2.35949;
/* Constant r2r3dist */
CONSTANTS[2] = 11.9758;
break;
case 422:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0929581;
/* Constant r2r3dist */
CONSTANTS[2] = 9.42542;
break;
case 423:
/* Constant r2r3dist */
CONSTANTS[1] = 1.03410;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1089;
break;
case 424:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21878;
/* Constant r2r3dist */
CONSTANTS[2] = 9.16772;
break;
case 425:
/* Constant r2r3dist */
CONSTANTS[1] = 0.351029;
/* Constant r2r3dist */
CONSTANTS[2] = 9.65057;
break;
case 426:
/* Constant r2r3dist */
CONSTANTS[1] = -1.43901;
/* Constant r2r3dist */
CONSTANTS[2] = 9.17139;
break;
case 427:
/* Constant r2r3dist */
CONSTANTS[1] = 0.586937;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7464;
break;
case 428:
/* Constant r2r3dist */
CONSTANTS[1] = -1.61676;
/* Constant r2r3dist */
CONSTANTS[2] = 9.55986;
break;
case 429:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21429;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33483;
break;
case 430:
/* Constant r2r3dist */
CONSTANTS[1] = -0.908345;
/* Constant r2r3dist */
CONSTANTS[2] = 9.42696;
break;
case 431:
/* Constant r2r3dist */
CONSTANTS[1] = -0.280592;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0751;
break;
case 432:
/* Constant r2r3dist */
CONSTANTS[1] = -0.653744;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1481;
break;
case 433:
/* Constant r2r3dist */
CONSTANTS[1] = -0.769794;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1001;
break;
case 434:
/* Constant r2r3dist */
CONSTANTS[1] = -0.856532;
/* Constant r2r3dist */
CONSTANTS[2] = 9.29220;
break;
case 435:
/* Constant r2r3dist */
CONSTANTS[1] = -0.280135;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8181;
break;
case 436:
/* Constant r2r3dist */
CONSTANTS[1] = 2.00824;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1645;
break;
case 437:
/* Constant r2r3dist */
CONSTANTS[1] = 0.786302;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7160;
break;
case 438:
/* Constant r2r3dist */
CONSTANTS[1] = 0.875345;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1950;
break;
case 439:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0405551;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6112;
break;
case 440:
/* Constant r2r3dist */
CONSTANTS[1] = 0.377342;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4677;
break;
case 441:
/* Constant r2r3dist */
CONSTANTS[1] = 0.944166;
/* Constant r2r3dist */
CONSTANTS[2] = 9.62152;
break;
case 442:
/* Constant r2r3dist */
CONSTANTS[1] = 0.503640;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0450;
break;
case 443:
/* Constant r2r3dist */
CONSTANTS[1] = -0.942394;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78721;
break;
case 444:
/* Constant r2r3dist */
CONSTANTS[1] = 1.24247;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7395;
break;
case 445:
/* Constant r2r3dist */
CONSTANTS[1] = -1.64484;
/* Constant r2r3dist */
CONSTANTS[2] = 9.04132;
break;
case 446:
/* Constant r2r3dist */
CONSTANTS[1] = 0.800602;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5586;
break;
case 447:
/* Constant r2r3dist */
CONSTANTS[1] = 1.72646;
/* Constant r2r3dist */
CONSTANTS[2] = 11.9048;
break;
case 448:
/* Constant r2r3dist */
CONSTANTS[1] = 0.167087;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0911;
break;
case 449:
/* Constant r2r3dist */
CONSTANTS[1] = 0.195967;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1663;
break;
case 450:
/* Constant r2r3dist */
CONSTANTS[1] = 0.601147;
/* Constant r2r3dist */
CONSTANTS[2] = 11.4331;
break;
case 451:
/* Constant r2r3dist */
CONSTANTS[1] = -0.959949;
/* Constant r2r3dist */
CONSTANTS[2] = 9.88850;
break;
case 452:
/* Constant r2r3dist */
CONSTANTS[1] = 1.89912;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1025;
break;
case 453:
/* Constant r2r3dist */
CONSTANTS[1] = 0.508026;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2076;
break;
case 454:
/* Constant r2r3dist */
CONSTANTS[1] = -1.15255;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43635;
break;
case 455:
/* Constant r2r3dist */
CONSTANTS[1] = 1.88267;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8683;
break;
case 456:
/* Constant r2r3dist */
CONSTANTS[1] = -0.124029;
/* Constant r2r3dist */
CONSTANTS[2] = 9.58333;
break;
case 457:
/* Constant r2r3dist */
CONSTANTS[1] = -1.53235;
/* Constant r2r3dist */
CONSTANTS[2] = 9.24947;
break;
case 458:
/* Constant r2r3dist */
CONSTANTS[1] = 3.69761;
/* Constant r2r3dist */
CONSTANTS[2] = 13.0441;
break;
case 459:
/* Constant r2r3dist */
CONSTANTS[1] = -0.826165;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5962;
break;
case 460:
/* Constant r2r3dist */
CONSTANTS[1] = 0.464457;
/* Constant r2r3dist */
CONSTANTS[2] = 9.76980;
break;
case 461:
/* Constant r2r3dist */
CONSTANTS[1] = -1.10147;
/* Constant r2r3dist */
CONSTANTS[2] = 9.47615;
break;
case 462:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0354406;
/* Constant r2r3dist */
CONSTANTS[2] = 9.90579;
break;
case 463:
/* Constant r2r3dist */
CONSTANTS[1] = 0.671877;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2018;
break;
case 464:
/* Constant r2r3dist */
CONSTANTS[1] = -0.507594;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6321;
break;
case 465:
/* Constant r2r3dist */
CONSTANTS[1] = -1.06747;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1960;
break;
case 466:
/* Constant r2r3dist */
CONSTANTS[1] = 0.363329;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4891;
break;
case 467:
/* Constant r2r3dist */
CONSTANTS[1] = 1.62710;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0831;
break;
case 468:
/* Constant r2r3dist */
CONSTANTS[1] = -0.440007;
/* Constant r2r3dist */
CONSTANTS[2] = 9.48834;
break;
case 469:
/* Constant r2r3dist */
CONSTANTS[1] = -1.05077;
/* Constant r2r3dist */
CONSTANTS[2] = 8.95881;
break;
case 470:
/* Constant r2r3dist */
CONSTANTS[1] = -0.727333;
/* Constant r2r3dist */
CONSTANTS[2] = 9.52694;
break;
case 471:
/* Constant r2r3dist */
CONSTANTS[1] = -0.661971;
/* Constant r2r3dist */
CONSTANTS[2] = 9.16460;
break;
case 472:
/* Constant r2r3dist */
CONSTANTS[1] = -0.657821;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39564;
break;
case 473:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21630;
/* Constant r2r3dist */
CONSTANTS[2] = 9.64805;
break;
case 474:
/* Constant r2r3dist */
CONSTANTS[1] = 0.453523;
/* Constant r2r3dist */
CONSTANTS[2] = 9.85691;
break;
case 475:
/* Constant r2r3dist */
CONSTANTS[1] = 0.665175;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4098;
break;
case 476:
/* Constant r2r3dist */
CONSTANTS[1] = -0.690009;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3589;
break;
case 477:
/* Constant r2r3dist */
CONSTANTS[1] = 0.373612;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3439;
break;
case 478:
/* Constant r2r3dist */
CONSTANTS[1] = 0.132482;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1983;
break;
case 479:
/* Constant r2r3dist */
CONSTANTS[1] = -1.96359;
/* Constant r2r3dist */
CONSTANTS[2] = 9.21284;
break;
case 480:
/* Constant r2r3dist */
CONSTANTS[1] = 0.165573;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1481;
break;
case 481:
/* Constant r2r3dist */
CONSTANTS[1] = -0.945926;
/* Constant r2r3dist */
CONSTANTS[2] = 9.28928;
break;
case 482:
/* Constant r2r3dist */
CONSTANTS[1] = 0.783164;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99796;
break;
case 483:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0488695;
/* Constant r2r3dist */
CONSTANTS[2] = 9.94085;
break;
case 484:
/* Constant r2r3dist */
CONSTANTS[1] = 0.384383;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6421;
break;
case 485:
/* Constant r2r3dist */
CONSTANTS[1] = 0.123760;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3180;
break;
case 486:
/* Constant r2r3dist */
CONSTANTS[1] = 0.370242;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1938;
break;
case 487:
/* Constant r2r3dist */
CONSTANTS[1] = 0.768281;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0901;
break;
case 488:
/* Constant r2r3dist */
CONSTANTS[1] = 1.11593;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0530;
break;
case 489:
/* Constant r2r3dist */
CONSTANTS[1] = -0.820506;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99933;
break;
case 490:
/* Constant r2r3dist */
CONSTANTS[1] = -1.83524;
/* Constant r2r3dist */
CONSTANTS[2] = 9.74197;
break;
case 491:
/* Constant r2r3dist */
CONSTANTS[1] = 1.24009;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53807;
break;
case 492:
/* Constant r2r3dist */
CONSTANTS[1] = -0.664850;
/* Constant r2r3dist */
CONSTANTS[2] = 9.35963;
break;
case 493:
/* Constant r2r3dist */
CONSTANTS[1] = 0.512790;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39523;
break;
case 494:
/* Constant r2r3dist */
CONSTANTS[1] = 0.643407;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6516;
break;
case 495:
/* Constant r2r3dist */
CONSTANTS[1] = -1.13597;
/* Constant r2r3dist */
CONSTANTS[2] = 9.59286;
break;
case 496:
/* Constant r2r3dist */
CONSTANTS[1] = -1.13684;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0510;
break;
case 497:
/* Constant r2r3dist */
CONSTANTS[1] = -0.158594;
/* Constant r2r3dist */
CONSTANTS[2] = 9.24374;
break;
case 498:
/* Constant r2r3dist */
CONSTANTS[1] = 0.885642;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3171;
break;
case 499:
/* Constant r2r3dist */
CONSTANTS[1] = 0.314444;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4585;
break;
case 500:
/* Constant r2r3dist */
CONSTANTS[1] = -0.475038;
/* Constant r2r3dist */
CONSTANTS[2] = 9.35973;
break;
case 501:
/* Constant r2r3dist */
CONSTANTS[1] = -0.987897;
/* Constant r2r3dist */
CONSTANTS[2] = 9.32515;
break;
case 502:
/* Constant r2r3dist */
CONSTANTS[1] = 1.23845;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1708;
break;
case 503:
/* Constant r2r3dist */
CONSTANTS[1] = 0.511501;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2405;
break;
case 504:
/* Constant r2r3dist */
CONSTANTS[1] = 1.33582;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0772;
break;
case 505:
/* Constant r2r3dist */
CONSTANTS[1] = -0.400080;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0432;
break;
case 506:
/* Constant r2r3dist */
CONSTANTS[1] = -0.622970;
/* Constant r2r3dist */
CONSTANTS[2] = 9.17166;
break;
case 507:
/* Constant r2r3dist */
CONSTANTS[1] = -0.340851;
/* Constant r2r3dist */
CONSTANTS[2] = 9.00939;
break;
case 508:
/* Constant r2r3dist */
CONSTANTS[1] = 0.438369;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1818;
break;
case 509:
/* Constant r2r3dist */
CONSTANTS[1] = -1.90100;
/* Constant r2r3dist */
CONSTANTS[2] = 8.81728;
break;
case 510:
/* Constant r2r3dist */
CONSTANTS[1] = 0.215920;
/* Constant r2r3dist */
CONSTANTS[2] = 9.34201;
break;
case 511:
/* Constant r2r3dist */
CONSTANTS[1] = -0.230859;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2719;
break;
case 512:
/* Constant r2r3dist */
CONSTANTS[1] = 0.156791;
/* Constant r2r3dist */
CONSTANTS[2] = 9.87068;
break;
case 513:
/* Constant r2r3dist */
CONSTANTS[1] = 1.17138;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7298;
break;
case 514:
/* Constant r2r3dist */
CONSTANTS[1] = -1.26034;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1679;
break;
case 515:
/* Constant r2r3dist */
CONSTANTS[1] = -0.479767;
/* Constant r2r3dist */
CONSTANTS[2] = 9.89906;
break;
case 516:
/* Constant r2r3dist */
CONSTANTS[1] = 1.52862;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7851;
break;
case 517:
/* Constant r2r3dist */
CONSTANTS[1] = 1.49318;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6488;
break;
case 518:
/* Constant r2r3dist */
CONSTANTS[1] = 1.32177;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3616;
break;
case 519:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0423543;
/* Constant r2r3dist */
CONSTANTS[2] = 9.80688;
break;
case 520:
/* Constant r2r3dist */
CONSTANTS[1] = -1.24957;
/* Constant r2r3dist */
CONSTANTS[2] = 9.20051;
break;
case 521:
/* Constant r2r3dist */
CONSTANTS[1] = 2.14020;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0278;
break;
case 522:
/* Constant r2r3dist */
CONSTANTS[1] = 1.02965;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1165;
break;
case 523:
/* Constant r2r3dist */
CONSTANTS[1] = -0.210068;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39555;
break;
case 524:
/* Constant r2r3dist */
CONSTANTS[1] = -0.423401;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97163;
break;
case 525:
/* Constant r2r3dist */
CONSTANTS[1] = 1.33140;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3506;
break;
case 526:
/* Constant r2r3dist */
CONSTANTS[1] = -1.99508;
/* Constant r2r3dist */
CONSTANTS[2] = 9.16234;
break;
case 527:
/* Constant r2r3dist */
CONSTANTS[1] = -0.969015;
/* Constant r2r3dist */
CONSTANTS[2] = 9.15691;
break;
case 528:
/* Constant r2r3dist */
CONSTANTS[1] = -1.46154;
/* Constant r2r3dist */
CONSTANTS[2] = 9.19254;
break;
case 529:
/* Constant r2r3dist */
CONSTANTS[1] = 1.49764;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97038;
break;
case 530:
/* Constant r2r3dist */
CONSTANTS[1] = 0.113081;
/* Constant r2r3dist */
CONSTANTS[2] = 9.83325;
break;
case 531:
/* Constant r2r3dist */
CONSTANTS[1] = -1.42146;
/* Constant r2r3dist */
CONSTANTS[2] = 9.06825;
break;
case 532:
/* Constant r2r3dist */
CONSTANTS[1] = -1.58281;
/* Constant r2r3dist */
CONSTANTS[2] = 9.49332;
break;
case 533:
/* Constant r2r3dist */
CONSTANTS[1] = 1.26386;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3359;
break;
case 534:
/* Constant r2r3dist */
CONSTANTS[1] = 1.34212;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3533;
break;
case 535:
/* Constant r2r3dist */
CONSTANTS[1] = -1.32330;
/* Constant r2r3dist */
CONSTANTS[2] = 8.96032;
break;
case 536:
/* Constant r2r3dist */
CONSTANTS[1] = 0.329914;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78799;
break;
case 537:
/* Constant r2r3dist */
CONSTANTS[1] = -1.21327;
/* Constant r2r3dist */
CONSTANTS[2] = 9.55096;
break;
case 538:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0303879;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75971;
break;
case 539:
/* Constant r2r3dist */
CONSTANTS[1] = 1.06210;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3017;
break;
case 540:
/* Constant r2r3dist */
CONSTANTS[1] = -0.808944;
/* Constant r2r3dist */
CONSTANTS[2] = 8.86237;
break;
case 541:
/* Constant r2r3dist */
CONSTANTS[1] = 0.352956;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3568;
break;
case 542:
/* Constant r2r3dist */
CONSTANTS[1] = -0.870592;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4506;
break;
case 543:
/* Constant r2r3dist */
CONSTANTS[1] = 1.05224;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4802;
break;
case 544:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0797880;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7142;
break;
case 545:
/* Constant r2r3dist */
CONSTANTS[1] = -0.946376;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4784;
break;
case 546:
/* Constant r2r3dist */
CONSTANTS[1] = 1.99960;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1019;
break;
case 547:
/* Constant r2r3dist */
CONSTANTS[1] = -0.196236;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5751;
break;
case 548:
/* Constant r2r3dist */
CONSTANTS[1] = 1.28829;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9514;
break;
case 549:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0823696;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3527;
break;
case 550:
/* Constant r2r3dist */
CONSTANTS[1] = -0.268091;
/* Constant r2r3dist */
CONSTANTS[2] = 9.70354;
break;
case 551:
/* Constant r2r3dist */
CONSTANTS[1] = 0.901731;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7756;
break;
case 552:
/* Constant r2r3dist */
CONSTANTS[1] = 1.28032;
/* Constant r2r3dist */
CONSTANTS[2] = 11.5576;
break;
case 553:
/* Constant r2r3dist */
CONSTANTS[1] = -1.68936;
/* Constant r2r3dist */
CONSTANTS[2] = 9.48078;
break;
case 554:
/* Constant r2r3dist */
CONSTANTS[1] = -0.571127;
/* Constant r2r3dist */
CONSTANTS[2] = 9.71225;
break;
case 555:
/* Constant r2r3dist */
CONSTANTS[1] = 1.71868;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7382;
break;
case 556:
/* Constant r2r3dist */
CONSTANTS[1] = 0.175661;
/* Constant r2r3dist */
CONSTANTS[2] = 9.76734;
break;
case 557:
/* Constant r2r3dist */
CONSTANTS[1] = -0.185540;
/* Constant r2r3dist */
CONSTANTS[2] = 9.12749;
break;
case 558:
/* Constant r2r3dist */
CONSTANTS[1] = -0.346409;
/* Constant r2r3dist */
CONSTANTS[2] = 9.29452;
break;
case 559:
/* Constant r2r3dist */
CONSTANTS[1] = -0.342999;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97823;
break;
case 560:
/* Constant r2r3dist */
CONSTANTS[1] = 0.737461;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3272;
break;
case 561:
/* Constant r2r3dist */
CONSTANTS[1] = -0.504493;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7028;
break;
case 562:
/* Constant r2r3dist */
CONSTANTS[1] = 0.523573;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6462;
break;
case 563:
/* Constant r2r3dist */
CONSTANTS[1] = -0.309610;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99799;
break;
case 564:
/* Constant r2r3dist */
CONSTANTS[1] = -0.881047;
/* Constant r2r3dist */
CONSTANTS[2] = 9.55033;
break;
case 565:
/* Constant r2r3dist */
CONSTANTS[1] = -0.494034;
/* Constant r2r3dist */
CONSTANTS[2] = 8.70862;
break;
case 566:
/* Constant r2r3dist */
CONSTANTS[1] = -0.582777;
/* Constant r2r3dist */
CONSTANTS[2] = 9.61434;
break;
case 567:
/* Constant r2r3dist */
CONSTANTS[1] = 0.544494;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3399;
break;
case 568:
/* Constant r2r3dist */
CONSTANTS[1] = -1.98849;
/* Constant r2r3dist */
CONSTANTS[2] = 8.82237;
break;
case 569:
/* Constant r2r3dist */
CONSTANTS[1] = 0.555495;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7356;
break;
case 570:
/* Constant r2r3dist */
CONSTANTS[1] = 0.802086;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3418;
break;
case 571:
/* Constant r2r3dist */
CONSTANTS[1] = 0.372985;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4661;
break;
case 572:
/* Constant r2r3dist */
CONSTANTS[1] = -1.85823;
/* Constant r2r3dist */
CONSTANTS[2] = 9.69814;
break;
case 573:
/* Constant r2r3dist */
CONSTANTS[1] = 0.879953;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2006;
break;
case 574:
/* Constant r2r3dist */
CONSTANTS[1] = 1.53822;
/* Constant r2r3dist */
CONSTANTS[2] = 9.71154;
break;
case 575:
/* Constant r2r3dist */
CONSTANTS[1] = -2.86551;
/* Constant r2r3dist */
CONSTANTS[2] = 8.61682;
break;
case 576:
/* Constant r2r3dist */
CONSTANTS[1] = 1.46682;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3537;
break;
case 577:
/* Constant r2r3dist */
CONSTANTS[1] = -1.24871;
/* Constant r2r3dist */
CONSTANTS[2] = 9.46812;
break;
case 578:
/* Constant r2r3dist */
CONSTANTS[1] = -0.747512;
/* Constant r2r3dist */
CONSTANTS[2] = 9.52576;
break;
case 579:
/* Constant r2r3dist */
CONSTANTS[1] = 1.22458;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2689;
break;
case 580:
/* Constant r2r3dist */
CONSTANTS[1] = 0.562402;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5759;
break;
case 581:
/* Constant r2r3dist */
CONSTANTS[1] = 0.183738;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4753;
break;
case 582:
/* Constant r2r3dist */
CONSTANTS[1] = 0.670118;
/* Constant r2r3dist */
CONSTANTS[2] = 9.63987;
break;
case 583:
/* Constant r2r3dist */
CONSTANTS[1] = 1.67535;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1555;
break;
case 584:
/* Constant r2r3dist */
CONSTANTS[1] = 1.33373;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6024;
break;
case 585:
/* Constant r2r3dist */
CONSTANTS[1] = -0.774551;
/* Constant r2r3dist */
CONSTANTS[2] = 9.65820;
break;
case 586:
/* Constant r2r3dist */
CONSTANTS[1] = 1.25987;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6762;
break;
case 587:
/* Constant r2r3dist */
CONSTANTS[1] = 1.85013;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5999;
break;
case 588:
/* Constant r2r3dist */
CONSTANTS[1] = 0.427977;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0741;
break;
case 589:
/* Constant r2r3dist */
CONSTANTS[1] = -0.542151;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4372;
break;
case 590:
/* Constant r2r3dist */
CONSTANTS[1] = -0.274625;
/* Constant r2r3dist */
CONSTANTS[2] = 9.52362;
break;
case 591:
/* Constant r2r3dist */
CONSTANTS[1] = -0.317415;
/* Constant r2r3dist */
CONSTANTS[2] = 9.83726;
break;
case 592:
/* Constant r2r3dist */
CONSTANTS[1] = 0.674831;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7653;
break;
case 593:
/* Constant r2r3dist */
CONSTANTS[1] = 0.677258;
/* Constant r2r3dist */
CONSTANTS[2] = 9.89435;
break;
case 594:
/* Constant r2r3dist */
CONSTANTS[1] = 0.716131;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0076;
break;
case 595:
/* Constant r2r3dist */
CONSTANTS[1] = -0.858801;
/* Constant r2r3dist */
CONSTANTS[2] = 8.92050;
break;
case 596:
/* Constant r2r3dist */
CONSTANTS[1] = -0.101121;
/* Constant r2r3dist */
CONSTANTS[2] = 9.49111;
break;
case 597:
/* Constant r2r3dist */
CONSTANTS[1] = -0.239913;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3514;
break;
case 598:
/* Constant r2r3dist */
CONSTANTS[1] = -0.477360;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6174;
break;
case 599:
/* Constant r2r3dist */
CONSTANTS[1] = -0.738714;
/* Constant r2r3dist */
CONSTANTS[2] = 9.22485;
break;
case 600:
/* Constant r2r3dist */
CONSTANTS[1] = 0.00411387;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5929;
break;
case 601:
/* Constant r2r3dist */
CONSTANTS[1] = -1.69666;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39686;
break;
case 602:
/* Constant r2r3dist */
CONSTANTS[1] = -0.620922;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1151;
break;
case 603:
/* Constant r2r3dist */
CONSTANTS[1] = -1.05565;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45448;
break;
case 604:
/* Constant r2r3dist */
CONSTANTS[1] = 1.57039;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81664;
break;
case 605:
/* Constant r2r3dist */
CONSTANTS[1] = -0.104777;
/* Constant r2r3dist */
CONSTANTS[2] = 9.69525;
break;
case 606:
/* Constant r2r3dist */
CONSTANTS[1] = -0.272459;
/* Constant r2r3dist */
CONSTANTS[2] = 9.44754;
break;
case 607:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0577842;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2906;
break;
case 608:
/* Constant r2r3dist */
CONSTANTS[1] = 0.505851;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3218;
break;
case 609:
/* Constant r2r3dist */
CONSTANTS[1] = -0.399555;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81272;
break;
case 610:
/* Constant r2r3dist */
CONSTANTS[1] = 0.924814;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2289;
break;
case 611:
/* Constant r2r3dist */
CONSTANTS[1] = 0.966235;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4605;
break;
case 612:
/* Constant r2r3dist */
CONSTANTS[1] = -0.806886;
/* Constant r2r3dist */
CONSTANTS[2] = 9.05107;
break;
case 613:
/* Constant r2r3dist */
CONSTANTS[1] = -0.497842;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27487;
break;
case 614:
/* Constant r2r3dist */
CONSTANTS[1] = -0.413479;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0058;
break;
case 615:
/* Constant r2r3dist */
CONSTANTS[1] = -0.985805;
/* Constant r2r3dist */
CONSTANTS[2] = 8.72723;
break;
case 616:
/* Constant r2r3dist */
CONSTANTS[1] = -1.59998;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39491;
break;
case 617:
/* Constant r2r3dist */
CONSTANTS[1] = -0.404125;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50980;
break;
case 618:
/* Constant r2r3dist */
CONSTANTS[1] = 0.445495;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2844;
break;
case 619:
/* Constant r2r3dist */
CONSTANTS[1] = -0.909585;
/* Constant r2r3dist */
CONSTANTS[2] = 9.22876;
break;
case 620:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0976493;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0220;
break;
case 621:
/* Constant r2r3dist */
CONSTANTS[1] = -1.25973;
/* Constant r2r3dist */
CONSTANTS[2] = 9.36721;
break;
case 622:
/* Constant r2r3dist */
CONSTANTS[1] = 1.61795;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4501;
break;
case 623:
/* Constant r2r3dist */
CONSTANTS[1] = 1.23804;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5586;
break;
case 624:
/* Constant r2r3dist */
CONSTANTS[1] = -0.718394;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78457;
break;
case 625:
/* Constant r2r3dist */
CONSTANTS[1] = -1.38174;
/* Constant r2r3dist */
CONSTANTS[2] = 8.61673;
break;
case 626:
/* Constant r2r3dist */
CONSTANTS[1] = 0.146050;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5760;
break;
case 627:
/* Constant r2r3dist */
CONSTANTS[1] = -0.301388;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2475;
break;
case 628:
/* Constant r2r3dist */
CONSTANTS[1] = 0.329288;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6663;
break;
case 629:
/* Constant r2r3dist */
CONSTANTS[1] = -0.367443;
/* Constant r2r3dist */
CONSTANTS[2] = 9.89087;
break;
case 630:
/* Constant r2r3dist */
CONSTANTS[1] = -2.12423;
/* Constant r2r3dist */
CONSTANTS[2] = 8.93525;
break;
case 631:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0621354;
/* Constant r2r3dist */
CONSTANTS[2] = 9.77677;
break;
case 632:
/* Constant r2r3dist */
CONSTANTS[1] = 0.157480;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7725;
break;
case 633:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0994025;
/* Constant r2r3dist */
CONSTANTS[2] = 9.06774;
break;
case 634:
/* Constant r2r3dist */
CONSTANTS[1] = -1.05937;
/* Constant r2r3dist */
CONSTANTS[2] = 8.97319;
break;
case 635:
/* Constant r2r3dist */
CONSTANTS[1] = 0.00701648;
/* Constant r2r3dist */
CONSTANTS[2] = 9.69851;
break;
case 636:
/* Constant r2r3dist */
CONSTANTS[1] = 0.665293;
/* Constant r2r3dist */
CONSTANTS[2] = 9.72380;
break;
case 637:
/* Constant r2r3dist */
CONSTANTS[1] = -1.98755;
/* Constant r2r3dist */
CONSTANTS[2] = 9.94040;
break;
case 638:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0113768;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8783;
break;
case 639:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0453297;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7279;
break;
case 640:
/* Constant r2r3dist */
CONSTANTS[1] = -1.26847;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2385;
break;
case 641:
/* Constant r2r3dist */
CONSTANTS[1] = -0.135900;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2942;
break;
case 642:
/* Constant r2r3dist */
CONSTANTS[1] = -0.320005;
/* Constant r2r3dist */
CONSTANTS[2] = 9.86866;
break;
case 643:
/* Constant r2r3dist */
CONSTANTS[1] = -1.17535;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50894;
break;
case 644:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0596427;
/* Constant r2r3dist */
CONSTANTS[2] = 9.41408;
break;
case 645:
/* Constant r2r3dist */
CONSTANTS[1] = -0.575592;
/* Constant r2r3dist */
CONSTANTS[2] = 9.03367;
break;
case 646:
/* Constant r2r3dist */
CONSTANTS[1] = -1.54249;
/* Constant r2r3dist */
CONSTANTS[2] = 9.59057;
break;
case 647:
/* Constant r2r3dist */
CONSTANTS[1] = -0.467236;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4076;
break;
case 648:
/* Constant r2r3dist */
CONSTANTS[1] = -0.476990;
/* Constant r2r3dist */
CONSTANTS[2] = 9.00160;
break;
case 649:
/* Constant r2r3dist */
CONSTANTS[1] = -1.37371;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3046;
break;
case 650:
/* Constant r2r3dist */
CONSTANTS[1] = -0.154625;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9566;
break;
case 651:
/* Constant r2r3dist */
CONSTANTS[1] = 1.44285;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7695;
break;
case 652:
/* Constant r2r3dist */
CONSTANTS[1] = -0.332722;
/* Constant r2r3dist */
CONSTANTS[2] = 9.07131;
break;
case 653:
/* Constant r2r3dist */
CONSTANTS[1] = 0.709138;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7518;
break;
case 654:
/* Constant r2r3dist */
CONSTANTS[1] = 0.746652;
/* Constant r2r3dist */
CONSTANTS[2] = 9.86163;
break;
case 655:
/* Constant r2r3dist */
CONSTANTS[1] = 0.680933;
/* Constant r2r3dist */
CONSTANTS[2] = 9.90815;
break;
case 656:
/* Constant r2r3dist */
CONSTANTS[1] = 0.117772;
/* Constant r2r3dist */
CONSTANTS[2] = 8.75121;
break;
case 657:
/* Constant r2r3dist */
CONSTANTS[1] = 1.32237;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8637;
break;
case 658:
/* Constant r2r3dist */
CONSTANTS[1] = -0.738373;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45378;
break;
case 659:
/* Constant r2r3dist */
CONSTANTS[1] = 0.151820;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1513;
break;
case 660:
/* Constant r2r3dist */
CONSTANTS[1] = -0.319313;
/* Constant r2r3dist */
CONSTANTS[2] = 9.49655;
break;
case 661:
/* Constant r2r3dist */
CONSTANTS[1] = 0.179654;
/* Constant r2r3dist */
CONSTANTS[2] = 9.19866;
break;
case 662:
/* Constant r2r3dist */
CONSTANTS[1] = 0.174360;
/* Constant r2r3dist */
CONSTANTS[2] = 9.07776;
break;
case 663:
/* Constant r2r3dist */
CONSTANTS[1] = 1.07812;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1251;
break;
case 664:
/* Constant r2r3dist */
CONSTANTS[1] = 0.562038;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1739;
break;
case 665:
/* Constant r2r3dist */
CONSTANTS[1] = 0.869652;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92537;
break;
case 666:
/* Constant r2r3dist */
CONSTANTS[1] = 0.465189;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1155;
break;
case 667:
/* Constant r2r3dist */
CONSTANTS[1] = -1.51759;
/* Constant r2r3dist */
CONSTANTS[2] = 9.63016;
break;
case 668:
/* Constant r2r3dist */
CONSTANTS[1] = -0.966909;
/* Constant r2r3dist */
CONSTANTS[2] = 8.58672;
break;
case 669:
/* Constant r2r3dist */
CONSTANTS[1] = -0.698523;
/* Constant r2r3dist */
CONSTANTS[2] = 9.73816;
break;
case 670:
/* Constant r2r3dist */
CONSTANTS[1] = -1.10229;
/* Constant r2r3dist */
CONSTANTS[2] = 9.56694;
break;
case 671:
/* Constant r2r3dist */
CONSTANTS[1] = 0.433178;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0252;
break;
case 672:
/* Constant r2r3dist */
CONSTANTS[1] = 1.11356;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7501;
break;
case 673:
/* Constant r2r3dist */
CONSTANTS[1] = -0.165635;
/* Constant r2r3dist */
CONSTANTS[2] = 9.91605;
break;
case 674:
/* Constant r2r3dist */
CONSTANTS[1] = 0.784282;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7656;
break;
case 675:
/* Constant r2r3dist */
CONSTANTS[1] = -1.45336;
/* Constant r2r3dist */
CONSTANTS[2] = 8.99533;
break;
case 676:
/* Constant r2r3dist */
CONSTANTS[1] = 1.56549;
/* Constant r2r3dist */
CONSTANTS[2] = 12.2716;
break;
case 677:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0288238;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0791;
break;
case 678:
/* Constant r2r3dist */
CONSTANTS[1] = -0.594152;
/* Constant r2r3dist */
CONSTANTS[2] = 9.34606;
break;
case 679:
/* Constant r2r3dist */
CONSTANTS[1] = 0.849283;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8055;
break;
case 680:
/* Constant r2r3dist */
CONSTANTS[1] = 0.170170;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0751;
break;
case 681:
/* Constant r2r3dist */
CONSTANTS[1] = -0.715273;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7120;
break;
case 682:
/* Constant r2r3dist */
CONSTANTS[1] = -0.753155;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1609;
break;
case 683:
/* Constant r2r3dist */
CONSTANTS[1] = -0.381936;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1288;
break;
case 684:
/* Constant r2r3dist */
CONSTANTS[1] = 0.117365;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7385;
break;
case 685:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0499983;
/* Constant r2r3dist */
CONSTANTS[2] = 9.90393;
break;
case 686:
/* Constant r2r3dist */
CONSTANTS[1] = 1.46231;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4129;
break;
case 687:
/* Constant r2r3dist */
CONSTANTS[1] = 0.693661;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6085;
break;
case 688:
/* Constant r2r3dist */
CONSTANTS[1] = -0.810199;
/* Constant r2r3dist */
CONSTANTS[2] = 8.30520;
break;
case 689:
/* Constant r2r3dist */
CONSTANTS[1] = 0.180981;
/* Constant r2r3dist */
CONSTANTS[2] = 9.60121;
break;
case 690:
/* Constant r2r3dist */
CONSTANTS[1] = -1.07555;
/* Constant r2r3dist */
CONSTANTS[2] = 9.29245;
break;
case 691:
/* Constant r2r3dist */
CONSTANTS[1] = 0.250373;
/* Constant r2r3dist */
CONSTANTS[2] = 9.63609;
break;
case 692:
/* Constant r2r3dist */
CONSTANTS[1] = -1.37192;
/* Constant r2r3dist */
CONSTANTS[2] = 9.82880;
break;
case 693:
/* Constant r2r3dist */
CONSTANTS[1] = -1.17980;
/* Constant r2r3dist */
CONSTANTS[2] = 9.80105;
break;
case 694:
/* Constant r2r3dist */
CONSTANTS[1] = -0.739278;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92105;
break;
case 695:
/* Constant r2r3dist */
CONSTANTS[1] = 1.30972;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2621;
break;
case 696:
/* Constant r2r3dist */
CONSTANTS[1] = -0.401853;
/* Constant r2r3dist */
CONSTANTS[2] = 9.18155;
break;
case 697:
/* Constant r2r3dist */
CONSTANTS[1] = -0.584644;
/* Constant r2r3dist */
CONSTANTS[2] = 9.84614;
break;
case 698:
/* Constant r2r3dist */
CONSTANTS[1] = 0.269095;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8294;
break;
case 699:
/* Constant r2r3dist */
CONSTANTS[1] = 0.295820;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5321;
break;
case 700:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0459941;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6502;
break;
case 701:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0465433;
/* Constant r2r3dist */
CONSTANTS[2] = 9.22829;
break;
case 702:
/* Constant r2r3dist */
CONSTANTS[1] = 1.95760;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0755;
break;
case 703:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0925624;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75964;
break;
case 704:
/* Constant r2r3dist */
CONSTANTS[1] = 2.54523;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7614;
break;
case 705:
/* Constant r2r3dist */
CONSTANTS[1] = -0.713451;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1387;
break;
case 706:
/* Constant r2r3dist */
CONSTANTS[1] = -0.874198;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3242;
break;
case 707:
/* Constant r2r3dist */
CONSTANTS[1] = 1.70470;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1288;
break;
case 708:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0643534;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1860;
break;
case 709:
/* Constant r2r3dist */
CONSTANTS[1] = -1.43201;
/* Constant r2r3dist */
CONSTANTS[2] = 8.80782;
break;
case 710:
/* Constant r2r3dist */
CONSTANTS[1] = 0.519043;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3021;
break;
case 711:
/* Constant r2r3dist */
CONSTANTS[1] = 0.991137;
/* Constant r2r3dist */
CONSTANTS[2] = 11.5725;
break;
case 712:
/* Constant r2r3dist */
CONSTANTS[1] = -0.756182;
/* Constant r2r3dist */
CONSTANTS[2] = 9.51829;
break;
case 713:
/* Constant r2r3dist */
CONSTANTS[1] = 0.125791;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5654;
break;
case 714:
/* Constant r2r3dist */
CONSTANTS[1] = 0.797916;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1496;
break;
case 715:
/* Constant r2r3dist */
CONSTANTS[1] = 0.213357;
/* Constant r2r3dist */
CONSTANTS[2] = 9.84970;
break;
case 716:
/* Constant r2r3dist */
CONSTANTS[1] = -1.11240;
/* Constant r2r3dist */
CONSTANTS[2] = 9.70595;
break;
case 717:
/* Constant r2r3dist */
CONSTANTS[1] = -0.938336;
/* Constant r2r3dist */
CONSTANTS[2] = 9.83747;
break;
case 718:
/* Constant r2r3dist */
CONSTANTS[1] = 0.423435;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1917;
break;
case 719:
/* Constant r2r3dist */
CONSTANTS[1] = -0.221586;
/* Constant r2r3dist */
CONSTANTS[2] = 9.46389;
break;
case 720:
/* Constant r2r3dist */
CONSTANTS[1] = -0.243507;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5818;
break;
case 721:
/* Constant r2r3dist */
CONSTANTS[1] = 1.72484;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5350;
break;
case 722:
/* Constant r2r3dist */
CONSTANTS[1] = -1.92122;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33013;
break;
case 723:
/* Constant r2r3dist */
CONSTANTS[1] = -1.35452;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53380;
break;
case 724:
/* Constant r2r3dist */
CONSTANTS[1] = -1.58391;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33147;
break;
case 725:
/* Constant r2r3dist */
CONSTANTS[1] = -0.480346;
/* Constant r2r3dist */
CONSTANTS[2] = 9.40854;
break;
case 726:
/* Constant r2r3dist */
CONSTANTS[1] = -0.432034;
/* Constant r2r3dist */
CONSTANTS[2] = 9.75558;
break;
case 727:
/* Constant r2r3dist */
CONSTANTS[1] = 0.00440832;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0524;
break;
case 728:
/* Constant r2r3dist */
CONSTANTS[1] = -0.650565;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96439;
break;
case 729:
/* Constant r2r3dist */
CONSTANTS[1] = -0.906049;
/* Constant r2r3dist */
CONSTANTS[2] = 9.88386;
break;
case 730:
/* Constant r2r3dist */
CONSTANTS[1] = 1.09229;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5758;
break;
case 731:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0536554;
/* Constant r2r3dist */
CONSTANTS[2] = 9.82221;
break;
case 732:
/* Constant r2r3dist */
CONSTANTS[1] = -0.742173;
/* Constant r2r3dist */
CONSTANTS[2] = 8.87081;
break;
case 733:
/* Constant r2r3dist */
CONSTANTS[1] = 0.534684;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0586;
break;
case 734:
/* Constant r2r3dist */
CONSTANTS[1] = 1.71160;
/* Constant r2r3dist */
CONSTANTS[2] = 11.7864;
break;
case 735:
/* Constant r2r3dist */
CONSTANTS[1] = -1.00782;
/* Constant r2r3dist */
CONSTANTS[2] = 9.44479;
break;
case 736:
/* Constant r2r3dist */
CONSTANTS[1] = 1.77866;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8613;
break;
case 737:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0668529;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31710;
break;
case 738:
/* Constant r2r3dist */
CONSTANTS[1] = -0.924675;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0205;
break;
case 739:
/* Constant r2r3dist */
CONSTANTS[1] = -0.743831;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95471;
break;
case 740:
/* Constant r2r3dist */
CONSTANTS[1] = 0.270911;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0003;
break;
case 741:
/* Constant r2r3dist */
CONSTANTS[1] = 0.347752;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1545;
break;
case 742:
/* Constant r2r3dist */
CONSTANTS[1] = -0.173728;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53353;
break;
case 743:
/* Constant r2r3dist */
CONSTANTS[1] = -1.83276;
/* Constant r2r3dist */
CONSTANTS[2] = 8.79439;
break;
case 744:
/* Constant r2r3dist */
CONSTANTS[1] = -0.302903;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7371;
break;
case 745:
/* Constant r2r3dist */
CONSTANTS[1] = 0.668477;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1338;
break;
case 746:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0690888;
/* Constant r2r3dist */
CONSTANTS[2] = 9.61609;
break;
case 747:
/* Constant r2r3dist */
CONSTANTS[1] = -1.06532;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0634;
break;
case 748:
/* Constant r2r3dist */
CONSTANTS[1] = 1.72766;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1485;
break;
case 749:
/* Constant r2r3dist */
CONSTANTS[1] = -0.360946;
/* Constant r2r3dist */
CONSTANTS[2] = 9.73864;
break;
case 750:
/* Constant r2r3dist */
CONSTANTS[1] = -1.12066;
/* Constant r2r3dist */
CONSTANTS[2] = 9.11297;
break;
case 751:
/* Constant r2r3dist */
CONSTANTS[1] = 0.447154;
/* Constant r2r3dist */
CONSTANTS[2] = 9.57730;
break;
case 752:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0156557;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0695;
break;
case 753:
/* Constant r2r3dist */
CONSTANTS[1] = -0.539593;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96815;
break;
case 754:
/* Constant r2r3dist */
CONSTANTS[1] = 2.29818;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1277;
break;
case 755:
/* Constant r2r3dist */
CONSTANTS[1] = 0.507553;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92079;
break;
case 756:
/* Constant r2r3dist */
CONSTANTS[1] = -1.17862;
/* Constant r2r3dist */
CONSTANTS[2] = 8.15489;
break;
case 757:
/* Constant r2r3dist */
CONSTANTS[1] = 1.90703;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8078;
break;
case 758:
/* Constant r2r3dist */
CONSTANTS[1] = -0.902915;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31834;
break;
case 759:
/* Constant r2r3dist */
CONSTANTS[1] = -0.167218;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96754;
break;
case 760:
/* Constant r2r3dist */
CONSTANTS[1] = 0.556958;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3550;
break;
case 761:
/* Constant r2r3dist */
CONSTANTS[1] = -0.261562;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5152;
break;
case 762:
/* Constant r2r3dist */
CONSTANTS[1] = -2.76881;
/* Constant r2r3dist */
CONSTANTS[2] = 9.04895;
break;
case 763:
/* Constant r2r3dist */
CONSTANTS[1] = -0.441135;
/* Constant r2r3dist */
CONSTANTS[2] = 9.19270;
break;
case 764:
/* Constant r2r3dist */
CONSTANTS[1] = 0.691029;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1500;
break;
case 765:
/* Constant r2r3dist */
CONSTANTS[1] = -0.564776;
/* Constant r2r3dist */
CONSTANTS[2] = 9.06734;
break;
case 766:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0606141;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8898;
break;
case 767:
/* Constant r2r3dist */
CONSTANTS[1] = 1.18677;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4244;
break;
case 768:
/* Constant r2r3dist */
CONSTANTS[1] = 1.00683;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3092;
break;
case 769:
/* Constant r2r3dist */
CONSTANTS[1] = -0.199073;
/* Constant r2r3dist */
CONSTANTS[2] = 9.51280;
break;
case 770:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0875493;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81344;
break;
case 771:
/* Constant r2r3dist */
CONSTANTS[1] = -0.222262;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0699;
break;
case 772:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0650175;
/* Constant r2r3dist */
CONSTANTS[2] = 9.58257;
break;
case 773:
/* Constant r2r3dist */
CONSTANTS[1] = -0.281041;
/* Constant r2r3dist */
CONSTANTS[2] = 9.52095;
break;
case 774:
/* Constant r2r3dist */
CONSTANTS[1] = -0.751988;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39143;
break;
case 775:
/* Constant r2r3dist */
CONSTANTS[1] = -0.779442;
/* Constant r2r3dist */
CONSTANTS[2] = 9.93878;
break;
case 776:
/* Constant r2r3dist */
CONSTANTS[1] = 2.06637;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3209;
break;
case 777:
/* Constant r2r3dist */
CONSTANTS[1] = 0.944681;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0518;
break;
case 778:
/* Constant r2r3dist */
CONSTANTS[1] = -0.539419;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99597;
break;
case 779:
/* Constant r2r3dist */
CONSTANTS[1] = 1.45410;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9979;
break;
case 780:
/* Constant r2r3dist */
CONSTANTS[1] = -0.873517;
/* Constant r2r3dist */
CONSTANTS[2] = 9.46646;
break;
case 781:
/* Constant r2r3dist */
CONSTANTS[1] = 0.366166;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0650;
break;
case 782:
/* Constant r2r3dist */
CONSTANTS[1] = -0.694822;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2249;
break;
case 783:
/* Constant r2r3dist */
CONSTANTS[1] = -0.770951;
/* Constant r2r3dist */
CONSTANTS[2] = 9.57244;
break;
case 784:
/* Constant r2r3dist */
CONSTANTS[1] = 0.180218;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0573;
break;
case 785:
/* Constant r2r3dist */
CONSTANTS[1] = -1.07333;
/* Constant r2r3dist */
CONSTANTS[2] = 8.97283;
break;
case 786:
/* Constant r2r3dist */
CONSTANTS[1] = 0.943767;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5321;
break;
case 787:
/* Constant r2r3dist */
CONSTANTS[1] = -0.941803;
/* Constant r2r3dist */
CONSTANTS[2] = 9.51315;
break;
case 788:
/* Constant r2r3dist */
CONSTANTS[1] = 1.28852;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6615;
break;
case 789:
/* Constant r2r3dist */
CONSTANTS[1] = -0.525633;
/* Constant r2r3dist */
CONSTANTS[2] = 9.71593;
break;
case 790:
/* Constant r2r3dist */
CONSTANTS[1] = -0.356620;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3038;
break;
case 791:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0983978;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45802;
break;
case 792:
/* Constant r2r3dist */
CONSTANTS[1] = 1.28864;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6820;
break;
case 793:
/* Constant r2r3dist */
CONSTANTS[1] = -1.44419;
/* Constant r2r3dist */
CONSTANTS[2] = 8.76478;
break;
case 794:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0645201;
/* Constant r2r3dist */
CONSTANTS[2] = 9.37212;
break;
case 795:
/* Constant r2r3dist */
CONSTANTS[1] = 1.60105;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7671;
break;
case 796:
/* Constant r2r3dist */
CONSTANTS[1] = -0.959894;
/* Constant r2r3dist */
CONSTANTS[2] = 9.25317;
break;
case 797:
/* Constant r2r3dist */
CONSTANTS[1] = 1.38561;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1304;
break;
case 798:
/* Constant r2r3dist */
CONSTANTS[1] = 2.20120;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0129;
break;
case 799:
/* Constant r2r3dist */
CONSTANTS[1] = -2.48587;
/* Constant r2r3dist */
CONSTANTS[2] = 8.84285;
break;
case 800:
/* Constant r2r3dist */
CONSTANTS[1] = -0.368809;
/* Constant r2r3dist */
CONSTANTS[2] = 9.93711;
break;
case 801:
/* Constant r2r3dist */
CONSTANTS[1] = -1.25567;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66618;
break;
case 802:
/* Constant r2r3dist */
CONSTANTS[1] = -0.622936;
/* Constant r2r3dist */
CONSTANTS[2] = 9.68600;
break;
case 803:
/* Constant r2r3dist */
CONSTANTS[1] = 0.230822;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33185;
break;
case 804:
/* Constant r2r3dist */
CONSTANTS[1] = -0.325642;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2148;
break;
case 805:
/* Constant r2r3dist */
CONSTANTS[1] = -1.81875;
/* Constant r2r3dist */
CONSTANTS[2] = 8.70316;
break;
case 806:
/* Constant r2r3dist */
CONSTANTS[1] = -0.630032;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43414;
break;
case 807:
/* Constant r2r3dist */
CONSTANTS[1] = 0.632039;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1936;
break;
case 808:
/* Constant r2r3dist */
CONSTANTS[1] = -0.457704;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1273;
break;
case 809:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0919556;
/* Constant r2r3dist */
CONSTANTS[2] = 9.70082;
break;
case 810:
/* Constant r2r3dist */
CONSTANTS[1] = -0.385448;
/* Constant r2r3dist */
CONSTANTS[2] = 9.77498;
break;
case 811:
/* Constant r2r3dist */
CONSTANTS[1] = -0.470480;
/* Constant r2r3dist */
CONSTANTS[2] = 9.49388;
break;
case 812:
/* Constant r2r3dist */
CONSTANTS[1] = -1.52280;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0155;
break;
case 813:
/* Constant r2r3dist */
CONSTANTS[1] = 0.741745;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0430;
break;
case 814:
/* Constant r2r3dist */
CONSTANTS[1] = 1.33674;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2950;
break;
case 815:
/* Constant r2r3dist */
CONSTANTS[1] = -0.646287;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43464;
break;
case 816:
/* Constant r2r3dist */
CONSTANTS[1] = -0.460706;
/* Constant r2r3dist */
CONSTANTS[2] = 9.20949;
break;
case 817:
/* Constant r2r3dist */
CONSTANTS[1] = -0.591060;
/* Constant r2r3dist */
CONSTANTS[2] = 9.87348;
break;
case 818:
/* Constant r2r3dist */
CONSTANTS[1] = 2.53276;
/* Constant r2r3dist */
CONSTANTS[2] = 12.0561;
break;
case 819:
/* Constant r2r3dist */
CONSTANTS[1] = 0.662793;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3984;
break;
case 820:
/* Constant r2r3dist */
CONSTANTS[1] = 0.776526;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3776;
break;
case 821:
/* Constant r2r3dist */
CONSTANTS[1] = -1.61852;
/* Constant r2r3dist */
CONSTANTS[2] = 9.05938;
break;
case 822:
/* Constant r2r3dist */
CONSTANTS[1] = -1.90429;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27718;
break;
case 823:
/* Constant r2r3dist */
CONSTANTS[1] = 0.301283;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1125;
break;
case 824:
/* Constant r2r3dist */
CONSTANTS[1] = -0.509330;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2612;
break;
case 825:
/* Constant r2r3dist */
CONSTANTS[1] = 0.804883;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0501;
break;
case 826:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0540325;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6170;
break;
case 827:
/* Constant r2r3dist */
CONSTANTS[1] = -0.923890;
/* Constant r2r3dist */
CONSTANTS[2] = 8.59465;
break;
case 828:
/* Constant r2r3dist */
CONSTANTS[1] = -0.230129;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1681;
break;
case 829:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0905123;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4879;
break;
case 830:
/* Constant r2r3dist */
CONSTANTS[1] = 1.01425;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8978;
break;
case 831:
/* Constant r2r3dist */
CONSTANTS[1] = -0.736676;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3459;
break;
case 832:
/* Constant r2r3dist */
CONSTANTS[1] = 0.250745;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4242;
break;
case 833:
/* Constant r2r3dist */
CONSTANTS[1] = 0.920250;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0086;
break;
case 834:
/* Constant r2r3dist */
CONSTANTS[1] = -0.353749;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45337;
break;
case 835:
/* Constant r2r3dist */
CONSTANTS[1] = 0.855165;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4502;
break;
case 836:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0274329;
/* Constant r2r3dist */
CONSTANTS[2] = 9.71096;
break;
case 837:
/* Constant r2r3dist */
CONSTANTS[1] = 0.409861;
/* Constant r2r3dist */
CONSTANTS[2] = 9.68843;
break;
case 838:
/* Constant r2r3dist */
CONSTANTS[1] = 1.12378;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3903;
break;
case 839:
/* Constant r2r3dist */
CONSTANTS[1] = 1.82371;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9673;
break;
case 840:
/* Constant r2r3dist */
CONSTANTS[1] = -0.949015;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4615;
break;
case 841:
/* Constant r2r3dist */
CONSTANTS[1] = -0.248353;
/* Constant r2r3dist */
CONSTANTS[2] = 9.56553;
break;
case 842:
/* Constant r2r3dist */
CONSTANTS[1] = 0.115274;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2195;
break;
case 843:
/* Constant r2r3dist */
CONSTANTS[1] = 0.859606;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5438;
break;
case 844:
/* Constant r2r3dist */
CONSTANTS[1] = 0.00345507;
/* Constant r2r3dist */
CONSTANTS[2] = 9.13125;
break;
case 845:
/* Constant r2r3dist */
CONSTANTS[1] = 0.465153;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92882;
break;
case 846:
/* Constant r2r3dist */
CONSTANTS[1] = 1.38424;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0707;
break;
case 847:
/* Constant r2r3dist */
CONSTANTS[1] = 1.94105;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0993;
break;
case 848:
/* Constant r2r3dist */
CONSTANTS[1] = 0.326845;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1832;
break;
case 849:
/* Constant r2r3dist */
CONSTANTS[1] = 0.497948;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8151;
break;
case 850:
/* Constant r2r3dist */
CONSTANTS[1] = 1.53639;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1781;
break;
case 851:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0563169;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43552;
break;
case 852:
/* Constant r2r3dist */
CONSTANTS[1] = 1.24752;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6178;
break;
case 853:
/* Constant r2r3dist */
CONSTANTS[1] = 0.689006;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3522;
break;
case 854:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0745950;
/* Constant r2r3dist */
CONSTANTS[2] = 9.53297;
break;
case 855:
/* Constant r2r3dist */
CONSTANTS[1] = 0.654379;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1969;
break;
case 856:
/* Constant r2r3dist */
CONSTANTS[1] = 0.138394;
/* Constant r2r3dist */
CONSTANTS[2] = 9.63237;
break;
case 857:
/* Constant r2r3dist */
CONSTANTS[1] = 1.29041;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5151;
break;
case 858:
/* Constant r2r3dist */
CONSTANTS[1] = -0.716263;
/* Constant r2r3dist */
CONSTANTS[2] = 9.47040;
break;
case 859:
/* Constant r2r3dist */
CONSTANTS[1] = 1.06463;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0940;
break;
case 860:
/* Constant r2r3dist */
CONSTANTS[1] = 0.691537;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66581;
break;
case 861:
/* Constant r2r3dist */
CONSTANTS[1] = -0.524528;
/* Constant r2r3dist */
CONSTANTS[2] = 9.98193;
break;
case 862:
/* Constant r2r3dist */
CONSTANTS[1] = 0.712051;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3048;
break;
case 863:
/* Constant r2r3dist */
CONSTANTS[1] = -1.08764;
/* Constant r2r3dist */
CONSTANTS[2] = 8.86982;
break;
case 864:
/* Constant r2r3dist */
CONSTANTS[1] = -1.03854;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0426;
break;
case 865:
/* Constant r2r3dist */
CONSTANTS[1] = -0.300397;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0395;
break;
case 866:
/* Constant r2r3dist */
CONSTANTS[1] = 0.163687;
/* Constant r2r3dist */
CONSTANTS[2] = 9.89039;
break;
case 867:
/* Constant r2r3dist */
CONSTANTS[1] = -0.362930;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4433;
break;
case 868:
/* Constant r2r3dist */
CONSTANTS[1] = -1.45712;
/* Constant r2r3dist */
CONSTANTS[2] = 8.62815;
break;
case 869:
/* Constant r2r3dist */
CONSTANTS[1] = 1.03559;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5646;
break;
case 870:
/* Constant r2r3dist */
CONSTANTS[1] = 0.737062;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3079;
break;
case 871:
/* Constant r2r3dist */
CONSTANTS[1] = -0.622119;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6272;
break;
case 872:
/* Constant r2r3dist */
CONSTANTS[1] = 0.126348;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1047;
break;
case 873:
/* Constant r2r3dist */
CONSTANTS[1] = 0.718506;
/* Constant r2r3dist */
CONSTANTS[2] = 9.66798;
break;
case 874:
/* Constant r2r3dist */
CONSTANTS[1] = -1.01678;
/* Constant r2r3dist */
CONSTANTS[2] = 9.98269;
break;
case 875:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0207573;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3439;
break;
case 876:
/* Constant r2r3dist */
CONSTANTS[1] = -1.42406;
/* Constant r2r3dist */
CONSTANTS[2] = 8.68692;
break;
case 877:
/* Constant r2r3dist */
CONSTANTS[1] = 0.299812;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50249;
break;
case 878:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0204763;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5821;
break;
case 879:
/* Constant r2r3dist */
CONSTANTS[1] = 1.06733;
/* Constant r2r3dist */
CONSTANTS[2] = 9.85556;
break;
case 880:
/* Constant r2r3dist */
CONSTANTS[1] = 0.654187;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0155;
break;
case 881:
/* Constant r2r3dist */
CONSTANTS[1] = -0.276507;
/* Constant r2r3dist */
CONSTANTS[2] = 9.39672;
break;
case 882:
/* Constant r2r3dist */
CONSTANTS[1] = 0.192439;
/* Constant r2r3dist */
CONSTANTS[2] = 9.99971;
break;
case 883:
/* Constant r2r3dist */
CONSTANTS[1] = 0.392675;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6096;
break;
case 884:
/* Constant r2r3dist */
CONSTANTS[1] = -0.537059;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43838;
break;
case 885:
/* Constant r2r3dist */
CONSTANTS[1] = -0.121059;
/* Constant r2r3dist */
CONSTANTS[2] = 9.88089;
break;
case 886:
/* Constant r2r3dist */
CONSTANTS[1] = 0.824322;
/* Constant r2r3dist */
CONSTANTS[2] = 9.85888;
break;
case 887:
/* Constant r2r3dist */
CONSTANTS[1] = 1.32241;
/* Constant r2r3dist */
CONSTANTS[2] = 9.97195;
break;
case 888:
/* Constant r2r3dist */
CONSTANTS[1] = 1.72189;
/* Constant r2r3dist */
CONSTANTS[2] = 9.72212;
break;
case 889:
/* Constant r2r3dist */
CONSTANTS[1] = -0.476691;
/* Constant r2r3dist */
CONSTANTS[2] = 9.33982;
break;
case 890:
/* Constant r2r3dist */
CONSTANTS[1] = -0.513109;
/* Constant r2r3dist */
CONSTANTS[2] = 9.69062;
break;
case 891:
/* Constant r2r3dist */
CONSTANTS[1] = 0.907776;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4962;
break;
case 892:
/* Constant r2r3dist */
CONSTANTS[1] = 0.998408;
/* Constant r2r3dist */
CONSTANTS[2] = 9.77578;
break;
case 893:
/* Constant r2r3dist */
CONSTANTS[1] = -0.453564;
/* Constant r2r3dist */
CONSTANTS[2] = 8.48969;
break;
case 894:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0336600;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3131;
break;
case 895:
/* Constant r2r3dist */
CONSTANTS[1] = 0.665363;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2302;
break;
case 896:
/* Constant r2r3dist */
CONSTANTS[1] = -2.09161;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27132;
break;
case 897:
/* Constant r2r3dist */
CONSTANTS[1] = -0.564510;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2897;
break;
case 898:
/* Constant r2r3dist */
CONSTANTS[1] = 0.221554;
/* Constant r2r3dist */
CONSTANTS[2] = 9.36024;
break;
case 899:
/* Constant r2r3dist */
CONSTANTS[1] = -0.919540;
/* Constant r2r3dist */
CONSTANTS[2] = 8.71021;
break;
case 900:
/* Constant r2r3dist */
CONSTANTS[1] = -2.05721;
/* Constant r2r3dist */
CONSTANTS[2] = 8.62042;
break;
case 901:
/* Constant r2r3dist */
CONSTANTS[1] = -1.73401;
/* Constant r2r3dist */
CONSTANTS[2] = 9.31850;
break;
case 902:
/* Constant r2r3dist */
CONSTANTS[1] = -0.255069;
/* Constant r2r3dist */
CONSTANTS[2] = 10.9187;
break;
case 903:
/* Constant r2r3dist */
CONSTANTS[1] = -0.693615;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6267;
break;
case 904:
/* Constant r2r3dist */
CONSTANTS[1] = 0.996264;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2673;
break;
case 905:
/* Constant r2r3dist */
CONSTANTS[1] = 0.0130013;
/* Constant r2r3dist */
CONSTANTS[2] = 9.95764;
break;
case 906:
/* Constant r2r3dist */
CONSTANTS[1] = 0.613593;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7239;
break;
case 907:
/* Constant r2r3dist */
CONSTANTS[1] = 1.00697;
/* Constant r2r3dist */
CONSTANTS[2] = 9.83879;
break;
case 908:
/* Constant r2r3dist */
CONSTANTS[1] = 0.993183;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5225;
break;
case 909:
/* Constant r2r3dist */
CONSTANTS[1] = -1.48908;
/* Constant r2r3dist */
CONSTANTS[2] = 9.54085;
break;
case 910:
/* Constant r2r3dist */
CONSTANTS[1] = 1.30673;
/* Constant r2r3dist */
CONSTANTS[2] = 9.74829;
break;
case 911:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0716510;
/* Constant r2r3dist */
CONSTANTS[2] = 9.94066;
break;
case 912:
/* Constant r2r3dist */
CONSTANTS[1] = 0.546889;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8390;
break;
case 913:
/* Constant r2r3dist */
CONSTANTS[1] = -1.79451;
/* Constant r2r3dist */
CONSTANTS[2] = 9.29911;
break;
case 914:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0469439;
/* Constant r2r3dist */
CONSTANTS[2] = 9.46157;
break;
case 915:
/* Constant r2r3dist */
CONSTANTS[1] = 0.390029;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2650;
break;
case 916:
/* Constant r2r3dist */
CONSTANTS[1] = -0.626906;
/* Constant r2r3dist */
CONSTANTS[2] = 9.64548;
break;
case 917:
/* Constant r2r3dist */
CONSTANTS[1] = 0.289368;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4897;
break;
case 918:
/* Constant r2r3dist */
CONSTANTS[1] = -2.06796;
/* Constant r2r3dist */
CONSTANTS[2] = 8.77572;
break;
case 919:
/* Constant r2r3dist */
CONSTANTS[1] = -1.03680;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7073;
break;
case 920:
/* Constant r2r3dist */
CONSTANTS[1] = -1.40329;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27088;
break;
case 921:
/* Constant r2r3dist */
CONSTANTS[1] = 0.200492;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2361;
break;
case 922:
/* Constant r2r3dist */
CONSTANTS[1] = -0.599786;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92477;
break;
case 923:
/* Constant r2r3dist */
CONSTANTS[1] = 0.811769;
/* Constant r2r3dist */
CONSTANTS[2] = 9.74241;
break;
case 924:
/* Constant r2r3dist */
CONSTANTS[1] = -0.722313;
/* Constant r2r3dist */
CONSTANTS[2] = 9.96940;
break;
case 925:
/* Constant r2r3dist */
CONSTANTS[1] = -1.17244;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1378;
break;
case 926:
/* Constant r2r3dist */
CONSTANTS[1] = -0.222185;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1155;
break;
case 927:
/* Constant r2r3dist */
CONSTANTS[1] = -1.13232;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5031;
break;
case 928:
/* Constant r2r3dist */
CONSTANTS[1] = -0.455183;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7724;
break;
case 929:
/* Constant r2r3dist */
CONSTANTS[1] = 1.16838;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6923;
break;
case 930:
/* Constant r2r3dist */
CONSTANTS[1] = -0.549595;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0214;
break;
case 931:
/* Constant r2r3dist */
CONSTANTS[1] = -0.140955;
/* Constant r2r3dist */
CONSTANTS[2] = 9.00435;
break;
case 932:
/* Constant r2r3dist */
CONSTANTS[1] = -0.587395;
/* Constant r2r3dist */
CONSTANTS[2] = 9.93871;
break;
case 933:
/* Constant r2r3dist */
CONSTANTS[1] = -0.888449;
/* Constant r2r3dist */
CONSTANTS[2] = 9.78375;
break;
case 934:
/* Constant r2r3dist */
CONSTANTS[1] = 1.85345;
/* Constant r2r3dist */
CONSTANTS[2] = 11.5064;
break;
case 935:
/* Constant r2r3dist */
CONSTANTS[1] = -0.479088;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2979;
break;
case 936:
/* Constant r2r3dist */
CONSTANTS[1] = -0.124059;
/* Constant r2r3dist */
CONSTANTS[2] = 9.88491;
break;
case 937:
/* Constant r2r3dist */
CONSTANTS[1] = -0.301915;
/* Constant r2r3dist */
CONSTANTS[2] = 9.19361;
break;
case 938:
/* Constant r2r3dist */
CONSTANTS[1] = -0.956338;
/* Constant r2r3dist */
CONSTANTS[2] = 9.10211;
break;
case 939:
/* Constant r2r3dist */
CONSTANTS[1] = 2.16400;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6818;
break;
case 940:
/* Constant r2r3dist */
CONSTANTS[1] = -0.723715;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1692;
break;
case 941:
/* Constant r2r3dist */
CONSTANTS[1] = -0.665370;
/* Constant r2r3dist */
CONSTANTS[2] = 9.15916;
break;
case 942:
/* Constant r2r3dist */
CONSTANTS[1] = -0.646668;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0831;
break;
case 943:
/* Constant r2r3dist */
CONSTANTS[1] = -0.695649;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5380;
break;
case 944:
/* Constant r2r3dist */
CONSTANTS[1] = 0.315151;
/* Constant r2r3dist */
CONSTANTS[2] = 9.72909;
break;
case 945:
/* Constant r2r3dist */
CONSTANTS[1] = 0.352090;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6583;
break;
case 946:
/* Constant r2r3dist */
CONSTANTS[1] = 1.36952;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8534;
break;
case 947:
/* Constant r2r3dist */
CONSTANTS[1] = -0.910147;
/* Constant r2r3dist */
CONSTANTS[2] = 9.26979;
break;
case 948:
/* Constant r2r3dist */
CONSTANTS[1] = 1.34310;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3238;
break;
case 949:
/* Constant r2r3dist */
CONSTANTS[1] = 1.03072;
/* Constant r2r3dist */
CONSTANTS[2] = 11.3989;
break;
case 950:
/* Constant r2r3dist */
CONSTANTS[1] = -0.455554;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2028;
break;
case 951:
/* Constant r2r3dist */
CONSTANTS[1] = 0.852450;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4955;
break;
case 952:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0807580;
/* Constant r2r3dist */
CONSTANTS[2] = 9.23639;
break;
case 953:
/* Constant r2r3dist */
CONSTANTS[1] = -0.565375;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2599;
break;
case 954:
/* Constant r2r3dist */
CONSTANTS[1] = -0.447625;
/* Constant r2r3dist */
CONSTANTS[2] = 9.82794;
break;
case 955:
/* Constant r2r3dist */
CONSTANTS[1] = -0.997441;
/* Constant r2r3dist */
CONSTANTS[2] = 9.48575;
break;
case 956:
/* Constant r2r3dist */
CONSTANTS[1] = 1.32302;
/* Constant r2r3dist */
CONSTANTS[2] = 11.2775;
break;
case 957:
/* Constant r2r3dist */
CONSTANTS[1] = 0.220730;
/* Constant r2r3dist */
CONSTANTS[2] = 10.7034;
break;
case 958:
/* Constant r2r3dist */
CONSTANTS[1] = 1.07881;
/* Constant r2r3dist */
CONSTANTS[2] = 10.8143;
break;
case 959:
/* Constant r2r3dist */
CONSTANTS[1] = 1.13480;
/* Constant r2r3dist */
CONSTANTS[2] = 10.2689;
break;
case 960:
/* Constant r2r3dist */
CONSTANTS[1] = 1.78137;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4883;
break;
case 961:
/* Constant r2r3dist */
CONSTANTS[1] = -0.434830;
/* Constant r2r3dist */
CONSTANTS[2] = 9.36449;
break;
case 962:
/* Constant r2r3dist */
CONSTANTS[1] = 0.115596;
/* Constant r2r3dist */
CONSTANTS[2] = 10.0827;
break;
case 963:
/* Constant r2r3dist */
CONSTANTS[1] = 1.41563;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6233;
break;
case 964:
/* Constant r2r3dist */
CONSTANTS[1] = 0.259822;
/* Constant r2r3dist */
CONSTANTS[2] = 11.8505;
break;
case 965:
/* Constant r2r3dist */
CONSTANTS[1] = 0.187238;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5595;
break;
case 966:
/* Constant r2r3dist */
CONSTANTS[1] = -1.19495;
/* Constant r2r3dist */
CONSTANTS[2] = 9.27385;
break;
case 967:
/* Constant r2r3dist */
CONSTANTS[1] = -0.0711781;
/* Constant r2r3dist */
CONSTANTS[2] = 9.49080;
break;
case 968:
/* Constant r2r3dist */
CONSTANTS[1] = -1.13640;
/* Constant r2r3dist */
CONSTANTS[2] = 9.51427;
break;
case 969:
/* Constant r2r3dist */
CONSTANTS[1] = -1.00529;
/* Constant r2r3dist */
CONSTANTS[2] = 9.82120;
break;
case 970:
/* Constant r2r3dist */
CONSTANTS[1] = -0.148196;
/* Constant r2r3dist */
CONSTANTS[2] = 9.28400;
break;
case 971:
/* Constant r2r3dist */
CONSTANTS[1] = 0.00359574;
/* Constant r2r3dist */
CONSTANTS[2] = 10.3654;
break;
case 972:
/* Constant r2r3dist */
CONSTANTS[1] = -2.15386;
/* Constant r2r3dist */
CONSTANTS[2] = 9.42831;
break;
case 973:
/* Constant r2r3dist */
CONSTANTS[1] = -0.316218;
/* Constant r2r3dist */
CONSTANTS[2] = 9.61370;
break;
case 974:
/* Constant r2r3dist */
CONSTANTS[1] = -0.273544;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45219;
break;
case 975:
/* Constant r2r3dist */
CONSTANTS[1] = 0.309532;
/* Constant r2r3dist */
CONSTANTS[2] = 10.1687;
break;
case 976:
/* Constant r2r3dist */
CONSTANTS[1] = 0.703372;
/* Constant r2r3dist */
CONSTANTS[2] = 11.1437;
break;
case 977:
/* Constant r2r3dist */
CONSTANTS[1] = -0.412385;
/* Constant r2r3dist */
CONSTANTS[2] = 9.73344;
break;
case 978:
/* Constant r2r3dist */
CONSTANTS[1] = 0.470502;
/* Constant r2r3dist */
CONSTANTS[2] = 10.6581;
break;
case 979:
/* Constant r2r3dist */
CONSTANTS[1] = 1.67429;
/* Constant r2r3dist */
CONSTANTS[2] = 9.58666;
break;
case 980:
/* Constant r2r3dist */
CONSTANTS[1] = -1.26984;
/* Constant r2r3dist */
CONSTANTS[2] = 9.36332;
break;
case 981:
/* Constant r2r3dist */
CONSTANTS[1] = 0.838545;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5400;
break;
case 982:
/* Constant r2r3dist */
CONSTANTS[1] = -0.675980;
/* Constant r2r3dist */
CONSTANTS[2] = 9.50656;
break;
case 983:
/* Constant r2r3dist */
CONSTANTS[1] = 0.801317;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4778;
break;
case 984:
/* Constant r2r3dist */
CONSTANTS[1] = 1.18293;
/* Constant r2r3dist */
CONSTANTS[2] = 10.5387;
break;
case 985:
/* Constant r2r3dist */
CONSTANTS[1] = -0.889240;
/* Constant r2r3dist */
CONSTANTS[2] = 9.94658;
break;
case 986:
/* Constant r2r3dist */
CONSTANTS[1] = -0.784644;
/* Constant r2r3dist */
CONSTANTS[2] = 8.30034;
break;
case 987:
/* Constant r2r3dist */
CONSTANTS[1] = -0.515011;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43816;
break;
case 988:
/* Constant r2r3dist */
CONSTANTS[1] = -1.56726;
/* Constant r2r3dist */
CONSTANTS[2] = 8.83984;
break;
case 989:
/* Constant r2r3dist */
CONSTANTS[1] = 0.352946;
/* Constant r2r3dist */
CONSTANTS[2] = 9.81484;
break;
case 990:
/* Constant r2r3dist */
CONSTANTS[1] = 0.798638;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4169;
break;
case 991:
/* Constant r2r3dist */
CONSTANTS[1] = 1.87032;
/* Constant r2r3dist */
CONSTANTS[2] = 9.92826;
break;
case 992:
/* Constant r2r3dist */
CONSTANTS[1] = 0.518340;
/* Constant r2r3dist */
CONSTANTS[2] = 9.43074;
break;
case 993:
/* Constant r2r3dist */
CONSTANTS[1] = -0.914958;
/* Constant r2r3dist */
CONSTANTS[2] = 9.07036;
break;
case 994:
/* Constant r2r3dist */
CONSTANTS[1] = 3.18263;
/* Constant r2r3dist */
CONSTANTS[2] = 11.6355;
break;
case 995:
/* Constant r2r3dist */
CONSTANTS[1] = -2.11596;
/* Constant r2r3dist */
CONSTANTS[2] = 8.82667;
break;
case 996:
/* Constant r2r3dist */
CONSTANTS[1] = 1.07909;
/* Constant r2r3dist */
CONSTANTS[2] = 9.45082;
break;
case 997:
/* Constant r2r3dist */
CONSTANTS[1] = -1.36946;
/* Constant r2r3dist */
CONSTANTS[2] = 9.18706;
break;
case 998:
/* Constant r2r3dist */
CONSTANTS[1] = 1.20362;
/* Constant r2r3dist */
CONSTANTS[2] = 11.0572;
break;
case 999:
/* Constant r2r3dist */
CONSTANTS[1] = 0.133729;
/* Constant r2r3dist */
CONSTANTS[2] = 10.4051;
break;
}
/* Constant r1V */
CONSTANTS[3] = 1.00000;
/* Constant r1M */
CONSTANTS[4] = 0.00000;
/* Constant r1Dist */
CONSTANTS[5] = SampleUsingPDF(&pdf_0, 0, pdf_roots_0, CONSTANTS, ALGEBRAIC);
/* Constant valrate */
CONSTANTS[6] = CONSTANTS[5];
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
/* Rate Restore */
RATES[0] = CONSTANTS[6];
}
