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
/* val */
STATES[0] = 0;
/* x */
CONSTANTS[0] = 0;
switch (rand() % 1000)
{
case 0:
/* r2r3dist */
CONSTANTS[1] = -0.397304;
/* r2r3dist */
CONSTANTS[2] = 8.70924;
break;
case 1:
/* r2r3dist */
CONSTANTS[1] = -1.16749;
/* r2r3dist */
CONSTANTS[2] = 9.68624;
break;
case 2:
/* r2r3dist */
CONSTANTS[1] = -0.649844;
/* r2r3dist */
CONSTANTS[2] = 10.0365;
break;
case 3:
/* r2r3dist */
CONSTANTS[1] = 1.82449;
/* r2r3dist */
CONSTANTS[2] = 12.1333;
break;
case 4:
/* r2r3dist */
CONSTANTS[1] = -2.67560;
/* r2r3dist */
CONSTANTS[2] = 9.43155;
break;
case 5:
/* r2r3dist */
CONSTANTS[1] = 0.0547296;
/* r2r3dist */
CONSTANTS[2] = 10.0719;
break;
case 6:
/* r2r3dist */
CONSTANTS[1] = 1.07090;
/* r2r3dist */
CONSTANTS[2] = 10.0235;
break;
case 7:
/* r2r3dist */
CONSTANTS[1] = -0.554121;
/* r2r3dist */
CONSTANTS[2] = 10.4162;
break;
case 8:
/* r2r3dist */
CONSTANTS[1] = -0.330647;
/* r2r3dist */
CONSTANTS[2] = 9.37518;
break;
case 9:
/* r2r3dist */
CONSTANTS[1] = 0.438340;
/* r2r3dist */
CONSTANTS[2] = 11.1350;
break;
case 10:
/* r2r3dist */
CONSTANTS[1] = -1.00341;
/* r2r3dist */
CONSTANTS[2] = 8.96924;
break;
case 11:
/* r2r3dist */
CONSTANTS[1] = -0.280605;
/* r2r3dist */
CONSTANTS[2] = 10.5120;
break;
case 12:
/* r2r3dist */
CONSTANTS[1] = 0.169892;
/* r2r3dist */
CONSTANTS[2] = 10.0342;
break;
case 13:
/* r2r3dist */
CONSTANTS[1] = -3.24431;
/* r2r3dist */
CONSTANTS[2] = 8.59351;
break;
case 14:
/* r2r3dist */
CONSTANTS[1] = 0.537377;
/* r2r3dist */
CONSTANTS[2] = 10.0864;
break;
case 15:
/* r2r3dist */
CONSTANTS[1] = 0.259131;
/* r2r3dist */
CONSTANTS[2] = 9.33872;
break;
case 16:
/* r2r3dist */
CONSTANTS[1] = 0.196380;
/* r2r3dist */
CONSTANTS[2] = 9.43961;
break;
case 17:
/* r2r3dist */
CONSTANTS[1] = 0.772454;
/* r2r3dist */
CONSTANTS[2] = 9.98352;
break;
case 18:
/* r2r3dist */
CONSTANTS[1] = -0.442969;
/* r2r3dist */
CONSTANTS[2] = 10.5780;
break;
case 19:
/* r2r3dist */
CONSTANTS[1] = 0.529665;
/* r2r3dist */
CONSTANTS[2] = 9.43131;
break;
case 20:
/* r2r3dist */
CONSTANTS[1] = -0.175020;
/* r2r3dist */
CONSTANTS[2] = 9.53173;
break;
case 21:
/* r2r3dist */
CONSTANTS[1] = 1.32604;
/* r2r3dist */
CONSTANTS[2] = 11.1211;
break;
case 22:
/* r2r3dist */
CONSTANTS[1] = 0.464047;
/* r2r3dist */
CONSTANTS[2] = 9.57088;
break;
case 23:
/* r2r3dist */
CONSTANTS[1] = 0.0433567;
/* r2r3dist */
CONSTANTS[2] = 9.02465;
break;
case 24:
/* r2r3dist */
CONSTANTS[1] = -1.56877;
/* r2r3dist */
CONSTANTS[2] = 9.92779;
break;
case 25:
/* r2r3dist */
CONSTANTS[1] = -0.475073;
/* r2r3dist */
CONSTANTS[2] = 10.5689;
break;
case 26:
/* r2r3dist */
CONSTANTS[1] = -1.06808;
/* r2r3dist */
CONSTANTS[2] = 9.96579;
break;
case 27:
/* r2r3dist */
CONSTANTS[1] = -0.398461;
/* r2r3dist */
CONSTANTS[2] = 10.2092;
break;
case 28:
/* r2r3dist */
CONSTANTS[1] = -0.527764;
/* r2r3dist */
CONSTANTS[2] = 10.1104;
break;
case 29:
/* r2r3dist */
CONSTANTS[1] = 0.461493;
/* r2r3dist */
CONSTANTS[2] = 10.6514;
break;
case 30:
/* r2r3dist */
CONSTANTS[1] = 1.95319;
/* r2r3dist */
CONSTANTS[2] = 10.7851;
break;
case 31:
/* r2r3dist */
CONSTANTS[1] = -0.284033;
/* r2r3dist */
CONSTANTS[2] = 9.92798;
break;
case 32:
/* r2r3dist */
CONSTANTS[1] = 1.31951;
/* r2r3dist */
CONSTANTS[2] = 11.4740;
break;
case 33:
/* r2r3dist */
CONSTANTS[1] = -0.116536;
/* r2r3dist */
CONSTANTS[2] = 10.1436;
break;
case 34:
/* r2r3dist */
CONSTANTS[1] = -0.601142;
/* r2r3dist */
CONSTANTS[2] = 9.35790;
break;
case 35:
/* r2r3dist */
CONSTANTS[1] = 0.409480;
/* r2r3dist */
CONSTANTS[2] = 10.3878;
break;
case 36:
/* r2r3dist */
CONSTANTS[1] = -1.69898;
/* r2r3dist */
CONSTANTS[2] = 9.13079;
break;
case 37:
/* r2r3dist */
CONSTANTS[1] = 0.247768;
/* r2r3dist */
CONSTANTS[2] = 10.8265;
break;
case 38:
/* r2r3dist */
CONSTANTS[1] = -1.29528;
/* r2r3dist */
CONSTANTS[2] = 9.87246;
break;
case 39:
/* r2r3dist */
CONSTANTS[1] = 0.484394;
/* r2r3dist */
CONSTANTS[2] = 10.3463;
break;
case 40:
/* r2r3dist */
CONSTANTS[1] = -0.122231;
/* r2r3dist */
CONSTANTS[2] = 9.76447;
break;
case 41:
/* r2r3dist */
CONSTANTS[1] = 1.12046;
/* r2r3dist */
CONSTANTS[2] = 11.8710;
break;
case 42:
/* r2r3dist */
CONSTANTS[1] = -0.925777;
/* r2r3dist */
CONSTANTS[2] = 10.0598;
break;
case 43:
/* r2r3dist */
CONSTANTS[1] = -1.19540;
/* r2r3dist */
CONSTANTS[2] = 9.54287;
break;
case 44:
/* r2r3dist */
CONSTANTS[1] = 1.00306;
/* r2r3dist */
CONSTANTS[2] = 9.96969;
break;
case 45:
/* r2r3dist */
CONSTANTS[1] = -0.915627;
/* r2r3dist */
CONSTANTS[2] = 9.68636;
break;
case 46:
/* r2r3dist */
CONSTANTS[1] = 0.000429096;
/* r2r3dist */
CONSTANTS[2] = 9.47965;
break;
case 47:
/* r2r3dist */
CONSTANTS[1] = -0.356800;
/* r2r3dist */
CONSTANTS[2] = 9.02946;
break;
case 48:
/* r2r3dist */
CONSTANTS[1] = 0.481115;
/* r2r3dist */
CONSTANTS[2] = 10.1639;
break;
case 49:
/* r2r3dist */
CONSTANTS[1] = 0.984145;
/* r2r3dist */
CONSTANTS[2] = 10.9652;
break;
case 50:
/* r2r3dist */
CONSTANTS[1] = -0.937497;
/* r2r3dist */
CONSTANTS[2] = 9.78731;
break;
case 51:
/* r2r3dist */
CONSTANTS[1] = 2.02395;
/* r2r3dist */
CONSTANTS[2] = 11.3836;
break;
case 52:
/* r2r3dist */
CONSTANTS[1] = -0.857944;
/* r2r3dist */
CONSTANTS[2] = 9.73200;
break;
case 53:
/* r2r3dist */
CONSTANTS[1] = -0.492842;
/* r2r3dist */
CONSTANTS[2] = 9.14182;
break;
case 54:
/* r2r3dist */
CONSTANTS[1] = 1.97649;
/* r2r3dist */
CONSTANTS[2] = 10.9234;
break;
case 55:
/* r2r3dist */
CONSTANTS[1] = 3.18893;
/* r2r3dist */
CONSTANTS[2] = 11.5659;
break;
case 56:
/* r2r3dist */
CONSTANTS[1] = 0.850761;
/* r2r3dist */
CONSTANTS[2] = 10.3575;
break;
case 57:
/* r2r3dist */
CONSTANTS[1] = 1.16166;
/* r2r3dist */
CONSTANTS[2] = 10.3596;
break;
case 58:
/* r2r3dist */
CONSTANTS[1] = 0.855790;
/* r2r3dist */
CONSTANTS[2] = 10.2775;
break;
case 59:
/* r2r3dist */
CONSTANTS[1] = -1.90250;
/* r2r3dist */
CONSTANTS[2] = 9.10788;
break;
case 60:
/* r2r3dist */
CONSTANTS[1] = 1.34709;
/* r2r3dist */
CONSTANTS[2] = 10.9309;
break;
case 61:
/* r2r3dist */
CONSTANTS[1] = -0.542613;
/* r2r3dist */
CONSTANTS[2] = 9.56537;
break;
case 62:
/* r2r3dist */
CONSTANTS[1] = -0.612029;
/* r2r3dist */
CONSTANTS[2] = 9.84630;
break;
case 63:
/* r2r3dist */
CONSTANTS[1] = 0.662960;
/* r2r3dist */
CONSTANTS[2] = 10.2884;
break;
case 64:
/* r2r3dist */
CONSTANTS[1] = -0.178510;
/* r2r3dist */
CONSTANTS[2] = 10.8677;
break;
case 65:
/* r2r3dist */
CONSTANTS[1] = -0.790018;
/* r2r3dist */
CONSTANTS[2] = 9.58712;
break;
case 66:
/* r2r3dist */
CONSTANTS[1] = 0.0497061;
/* r2r3dist */
CONSTANTS[2] = 9.62169;
break;
case 67:
/* r2r3dist */
CONSTANTS[1] = -0.109377;
/* r2r3dist */
CONSTANTS[2] = 11.1402;
break;
case 68:
/* r2r3dist */
CONSTANTS[1] = 0.973509;
/* r2r3dist */
CONSTANTS[2] = 11.4186;
break;
case 69:
/* r2r3dist */
CONSTANTS[1] = 0.424305;
/* r2r3dist */
CONSTANTS[2] = 11.1673;
break;
case 70:
/* r2r3dist */
CONSTANTS[1] = -0.862519;
/* r2r3dist */
CONSTANTS[2] = 9.24313;
break;
case 71:
/* r2r3dist */
CONSTANTS[1] = 0.873642;
/* r2r3dist */
CONSTANTS[2] = 9.90775;
break;
case 72:
/* r2r3dist */
CONSTANTS[1] = 0.627842;
/* r2r3dist */
CONSTANTS[2] = 11.0840;
break;
case 73:
/* r2r3dist */
CONSTANTS[1] = -0.982380;
/* r2r3dist */
CONSTANTS[2] = 9.31079;
break;
case 74:
/* r2r3dist */
CONSTANTS[1] = -0.753886;
/* r2r3dist */
CONSTANTS[2] = 9.67372;
break;
case 75:
/* r2r3dist */
CONSTANTS[1] = 0.305687;
/* r2r3dist */
CONSTANTS[2] = 9.95795;
break;
case 76:
/* r2r3dist */
CONSTANTS[1] = -0.448600;
/* r2r3dist */
CONSTANTS[2] = 9.98373;
break;
case 77:
/* r2r3dist */
CONSTANTS[1] = 1.96389;
/* r2r3dist */
CONSTANTS[2] = 10.2289;
break;
case 78:
/* r2r3dist */
CONSTANTS[1] = 0.103843;
/* r2r3dist */
CONSTANTS[2] = 9.91418;
break;
case 79:
/* r2r3dist */
CONSTANTS[1] = -0.294529;
/* r2r3dist */
CONSTANTS[2] = 9.92863;
break;
case 80:
/* r2r3dist */
CONSTANTS[1] = -1.09942;
/* r2r3dist */
CONSTANTS[2] = 9.73231;
break;
case 81:
/* r2r3dist */
CONSTANTS[1] = -1.21572;
/* r2r3dist */
CONSTANTS[2] = 10.1152;
break;
case 82:
/* r2r3dist */
CONSTANTS[1] = -1.97628;
/* r2r3dist */
CONSTANTS[2] = 8.52038;
break;
case 83:
/* r2r3dist */
CONSTANTS[1] = -1.08008;
/* r2r3dist */
CONSTANTS[2] = 10.0401;
break;
case 84:
/* r2r3dist */
CONSTANTS[1] = -0.131598;
/* r2r3dist */
CONSTANTS[2] = 9.98556;
break;
case 85:
/* r2r3dist */
CONSTANTS[1] = 1.48665;
/* r2r3dist */
CONSTANTS[2] = 10.9267;
break;
case 86:
/* r2r3dist */
CONSTANTS[1] = 0.385169;
/* r2r3dist */
CONSTANTS[2] = 9.95288;
break;
case 87:
/* r2r3dist */
CONSTANTS[1] = -0.314853;
/* r2r3dist */
CONSTANTS[2] = 9.64242;
break;
case 88:
/* r2r3dist */
CONSTANTS[1] = -1.46278;
/* r2r3dist */
CONSTANTS[2] = 10.1506;
break;
case 89:
/* r2r3dist */
CONSTANTS[1] = -0.345246;
/* r2r3dist */
CONSTANTS[2] = 10.5831;
break;
case 90:
/* r2r3dist */
CONSTANTS[1] = -1.33460;
/* r2r3dist */
CONSTANTS[2] = 9.31070;
break;
case 91:
/* r2r3dist */
CONSTANTS[1] = 0.973103;
/* r2r3dist */
CONSTANTS[2] = 11.1597;
break;
case 92:
/* r2r3dist */
CONSTANTS[1] = 0.134743;
/* r2r3dist */
CONSTANTS[2] = 9.92023;
break;
case 93:
/* r2r3dist */
CONSTANTS[1] = -1.24889;
/* r2r3dist */
CONSTANTS[2] = 10.2489;
break;
case 94:
/* r2r3dist */
CONSTANTS[1] = -0.412555;
/* r2r3dist */
CONSTANTS[2] = 9.91934;
break;
case 95:
/* r2r3dist */
CONSTANTS[1] = 1.29382;
/* r2r3dist */
CONSTANTS[2] = 10.2714;
break;
case 96:
/* r2r3dist */
CONSTANTS[1] = -0.726661;
/* r2r3dist */
CONSTANTS[2] = 9.75208;
break;
case 97:
/* r2r3dist */
CONSTANTS[1] = 1.05886;
/* r2r3dist */
CONSTANTS[2] = 11.1631;
break;
case 98:
/* r2r3dist */
CONSTANTS[1] = -0.522445;
/* r2r3dist */
CONSTANTS[2] = 9.16190;
break;
case 99:
/* r2r3dist */
CONSTANTS[1] = 0.248655;
/* r2r3dist */
CONSTANTS[2] = 10.3259;
break;
case 100:
/* r2r3dist */
CONSTANTS[1] = -0.489311;
/* r2r3dist */
CONSTANTS[2] = 9.80166;
break;
case 101:
/* r2r3dist */
CONSTANTS[1] = 0.0771580;
/* r2r3dist */
CONSTANTS[2] = 9.10271;
break;
case 102:
/* r2r3dist */
CONSTANTS[1] = 0.844544;
/* r2r3dist */
CONSTANTS[2] = 9.63365;
break;
case 103:
/* r2r3dist */
CONSTANTS[1] = 1.34215;
/* r2r3dist */
CONSTANTS[2] = 10.0078;
break;
case 104:
/* r2r3dist */
CONSTANTS[1] = 1.11532;
/* r2r3dist */
CONSTANTS[2] = 10.2361;
break;
case 105:
/* r2r3dist */
CONSTANTS[1] = 2.07959;
/* r2r3dist */
CONSTANTS[2] = 11.0199;
break;
case 106:
/* r2r3dist */
CONSTANTS[1] = -0.0884372;
/* r2r3dist */
CONSTANTS[2] = 9.34866;
break;
case 107:
/* r2r3dist */
CONSTANTS[1] = -0.759000;
/* r2r3dist */
CONSTANTS[2] = 9.26611;
break;
case 108:
/* r2r3dist */
CONSTANTS[1] = 0.227441;
/* r2r3dist */
CONSTANTS[2] = 9.62424;
break;
case 109:
/* r2r3dist */
CONSTANTS[1] = 0.0782673;
/* r2r3dist */
CONSTANTS[2] = 10.9109;
break;
case 110:
/* r2r3dist */
CONSTANTS[1] = -0.856699;
/* r2r3dist */
CONSTANTS[2] = 9.54973;
break;
case 111:
/* r2r3dist */
CONSTANTS[1] = -0.420034;
/* r2r3dist */
CONSTANTS[2] = 9.77135;
break;
case 112:
/* r2r3dist */
CONSTANTS[1] = -1.53474;
/* r2r3dist */
CONSTANTS[2] = 9.15775;
break;
case 113:
/* r2r3dist */
CONSTANTS[1] = -1.39898;
/* r2r3dist */
CONSTANTS[2] = 9.18780;
break;
case 114:
/* r2r3dist */
CONSTANTS[1] = -0.695713;
/* r2r3dist */
CONSTANTS[2] = 9.34482;
break;
case 115:
/* r2r3dist */
CONSTANTS[1] = 1.56354;
/* r2r3dist */
CONSTANTS[2] = 10.4519;
break;
case 116:
/* r2r3dist */
CONSTANTS[1] = 0.521136;
/* r2r3dist */
CONSTANTS[2] = 9.81228;
break;
case 117:
/* r2r3dist */
CONSTANTS[1] = -1.44351;
/* r2r3dist */
CONSTANTS[2] = 9.93480;
break;
case 118:
/* r2r3dist */
CONSTANTS[1] = 1.21405;
/* r2r3dist */
CONSTANTS[2] = 10.9245;
break;
case 119:
/* r2r3dist */
CONSTANTS[1] = -0.233544;
/* r2r3dist */
CONSTANTS[2] = 9.75290;
break;
case 120:
/* r2r3dist */
CONSTANTS[1] = -0.810330;
/* r2r3dist */
CONSTANTS[2] = 9.82137;
break;
case 121:
/* r2r3dist */
CONSTANTS[1] = -2.34638;
/* r2r3dist */
CONSTANTS[2] = 8.15355;
break;
case 122:
/* r2r3dist */
CONSTANTS[1] = -1.02356;
/* r2r3dist */
CONSTANTS[2] = 9.95785;
break;
case 123:
/* r2r3dist */
CONSTANTS[1] = -2.01851;
/* r2r3dist */
CONSTANTS[2] = 8.67204;
break;
case 124:
/* r2r3dist */
CONSTANTS[1] = 0.442318;
/* r2r3dist */
CONSTANTS[2] = 10.7635;
break;
case 125:
/* r2r3dist */
CONSTANTS[1] = -0.354193;
/* r2r3dist */
CONSTANTS[2] = 10.4928;
break;
case 126:
/* r2r3dist */
CONSTANTS[1] = -0.919096;
/* r2r3dist */
CONSTANTS[2] = 9.54161;
break;
case 127:
/* r2r3dist */
CONSTANTS[1] = 0.532995;
/* r2r3dist */
CONSTANTS[2] = 9.90541;
break;
case 128:
/* r2r3dist */
CONSTANTS[1] = -1.04530;
/* r2r3dist */
CONSTANTS[2] = 9.44963;
break;
case 129:
/* r2r3dist */
CONSTANTS[1] = -0.409808;
/* r2r3dist */
CONSTANTS[2] = 9.75445;
break;
case 130:
/* r2r3dist */
CONSTANTS[1] = -0.604661;
/* r2r3dist */
CONSTANTS[2] = 9.66697;
break;
case 131:
/* r2r3dist */
CONSTANTS[1] = -0.0262765;
/* r2r3dist */
CONSTANTS[2] = 10.3651;
break;
case 132:
/* r2r3dist */
CONSTANTS[1] = 0.608419;
/* r2r3dist */
CONSTANTS[2] = 9.99240;
break;
case 133:
/* r2r3dist */
CONSTANTS[1] = 1.20405;
/* r2r3dist */
CONSTANTS[2] = 10.0850;
break;
case 134:
/* r2r3dist */
CONSTANTS[1] = -0.399546;
/* r2r3dist */
CONSTANTS[2] = 9.43459;
break;
case 135:
/* r2r3dist */
CONSTANTS[1] = 0.507774;
/* r2r3dist */
CONSTANTS[2] = 9.81893;
break;
case 136:
/* r2r3dist */
CONSTANTS[1] = -1.39854;
/* r2r3dist */
CONSTANTS[2] = 9.07308;
break;
case 137:
/* r2r3dist */
CONSTANTS[1] = 0.156823;
/* r2r3dist */
CONSTANTS[2] = 10.8610;
break;
case 138:
/* r2r3dist */
CONSTANTS[1] = 0.274586;
/* r2r3dist */
CONSTANTS[2] = 9.59653;
break;
case 139:
/* r2r3dist */
CONSTANTS[1] = -0.302830;
/* r2r3dist */
CONSTANTS[2] = 9.21990;
break;
case 140:
/* r2r3dist */
CONSTANTS[1] = -1.65531;
/* r2r3dist */
CONSTANTS[2] = 10.2439;
break;
case 141:
/* r2r3dist */
CONSTANTS[1] = 0.340942;
/* r2r3dist */
CONSTANTS[2] = 9.14995;
break;
case 142:
/* r2r3dist */
CONSTANTS[1] = -1.13199;
/* r2r3dist */
CONSTANTS[2] = 9.48058;
break;
case 143:
/* r2r3dist */
CONSTANTS[1] = 0.562440;
/* r2r3dist */
CONSTANTS[2] = 10.3752;
break;
case 144:
/* r2r3dist */
CONSTANTS[1] = 1.41276;
/* r2r3dist */
CONSTANTS[2] = 11.2429;
break;
case 145:
/* r2r3dist */
CONSTANTS[1] = 1.21882;
/* r2r3dist */
CONSTANTS[2] = 11.6472;
break;
case 146:
/* r2r3dist */
CONSTANTS[1] = -0.739580;
/* r2r3dist */
CONSTANTS[2] = 9.27707;
break;
case 147:
/* r2r3dist */
CONSTANTS[1] = -1.11941;
/* r2r3dist */
CONSTANTS[2] = 9.27118;
break;
case 148:
/* r2r3dist */
CONSTANTS[1] = 1.56583;
/* r2r3dist */
CONSTANTS[2] = 10.7700;
break;
case 149:
/* r2r3dist */
CONSTANTS[1] = 0.0612810;
/* r2r3dist */
CONSTANTS[2] = 9.68441;
break;
case 150:
/* r2r3dist */
CONSTANTS[1] = 0.352526;
/* r2r3dist */
CONSTANTS[2] = 10.1031;
break;
case 151:
/* r2r3dist */
CONSTANTS[1] = 0.807038;
/* r2r3dist */
CONSTANTS[2] = 10.6675;
break;
case 152:
/* r2r3dist */
CONSTANTS[1] = 0.856719;
/* r2r3dist */
CONSTANTS[2] = 10.4921;
break;
case 153:
/* r2r3dist */
CONSTANTS[1] = -1.23230;
/* r2r3dist */
CONSTANTS[2] = 9.53305;
break;
case 154:
/* r2r3dist */
CONSTANTS[1] = -0.750940;
/* r2r3dist */
CONSTANTS[2] = 9.58842;
break;
case 155:
/* r2r3dist */
CONSTANTS[1] = 0.911477;
/* r2r3dist */
CONSTANTS[2] = 9.81170;
break;
case 156:
/* r2r3dist */
CONSTANTS[1] = -0.855538;
/* r2r3dist */
CONSTANTS[2] = 9.26032;
break;
case 157:
/* r2r3dist */
CONSTANTS[1] = 0.407426;
/* r2r3dist */
CONSTANTS[2] = 10.4182;
break;
case 158:
/* r2r3dist */
CONSTANTS[1] = -0.744870;
/* r2r3dist */
CONSTANTS[2] = 10.6346;
break;
case 159:
/* r2r3dist */
CONSTANTS[1] = 1.53028;
/* r2r3dist */
CONSTANTS[2] = 11.0655;
break;
case 160:
/* r2r3dist */
CONSTANTS[1] = 0.528775;
/* r2r3dist */
CONSTANTS[2] = 9.55617;
break;
case 161:
/* r2r3dist */
CONSTANTS[1] = -0.262340;
/* r2r3dist */
CONSTANTS[2] = 9.50097;
break;
case 162:
/* r2r3dist */
CONSTANTS[1] = -0.939561;
/* r2r3dist */
CONSTANTS[2] = 9.96487;
break;
case 163:
/* r2r3dist */
CONSTANTS[1] = 0.572930;
/* r2r3dist */
CONSTANTS[2] = 9.85895;
break;
case 164:
/* r2r3dist */
CONSTANTS[1] = 0.787766;
/* r2r3dist */
CONSTANTS[2] = 10.3281;
break;
case 165:
/* r2r3dist */
CONSTANTS[1] = -0.788928;
/* r2r3dist */
CONSTANTS[2] = 10.0178;
break;
case 166:
/* r2r3dist */
CONSTANTS[1] = 1.20733;
/* r2r3dist */
CONSTANTS[2] = 10.5219;
break;
case 167:
/* r2r3dist */
CONSTANTS[1] = -1.26415;
/* r2r3dist */
CONSTANTS[2] = 8.58893;
break;
case 168:
/* r2r3dist */
CONSTANTS[1] = 0.423778;
/* r2r3dist */
CONSTANTS[2] = 11.3135;
break;
case 169:
/* r2r3dist */
CONSTANTS[1] = 1.05752;
/* r2r3dist */
CONSTANTS[2] = 10.6387;
break;
case 170:
/* r2r3dist */
CONSTANTS[1] = 1.18197;
/* r2r3dist */
CONSTANTS[2] = 10.6165;
break;
case 171:
/* r2r3dist */
CONSTANTS[1] = 0.404062;
/* r2r3dist */
CONSTANTS[2] = 9.90065;
break;
case 172:
/* r2r3dist */
CONSTANTS[1] = -0.0720385;
/* r2r3dist */
CONSTANTS[2] = 10.0298;
break;
case 173:
/* r2r3dist */
CONSTANTS[1] = 0.610309;
/* r2r3dist */
CONSTANTS[2] = 10.2138;
break;
case 174:
/* r2r3dist */
CONSTANTS[1] = 0.579604;
/* r2r3dist */
CONSTANTS[2] = 10.8719;
break;
case 175:
/* r2r3dist */
CONSTANTS[1] = -0.562992;
/* r2r3dist */
CONSTANTS[2] = 9.98981;
break;
case 176:
/* r2r3dist */
CONSTANTS[1] = -0.516771;
/* r2r3dist */
CONSTANTS[2] = 8.44386;
break;
case 177:
/* r2r3dist */
CONSTANTS[1] = -0.576560;
/* r2r3dist */
CONSTANTS[2] = 9.50838;
break;
case 178:
/* r2r3dist */
CONSTANTS[1] = -1.67534;
/* r2r3dist */
CONSTANTS[2] = 9.34918;
break;
case 179:
/* r2r3dist */
CONSTANTS[1] = -0.845814;
/* r2r3dist */
CONSTANTS[2] = 9.31171;
break;
case 180:
/* r2r3dist */
CONSTANTS[1] = 2.00670;
/* r2r3dist */
CONSTANTS[2] = 11.9930;
break;
case 181:
/* r2r3dist */
CONSTANTS[1] = 0.410152;
/* r2r3dist */
CONSTANTS[2] = 10.0540;
break;
case 182:
/* r2r3dist */
CONSTANTS[1] = -1.34287;
/* r2r3dist */
CONSTANTS[2] = 8.49243;
break;
case 183:
/* r2r3dist */
CONSTANTS[1] = 2.33060;
/* r2r3dist */
CONSTANTS[2] = 11.9768;
break;
case 184:
/* r2r3dist */
CONSTANTS[1] = 0.0960324;
/* r2r3dist */
CONSTANTS[2] = 10.0347;
break;
case 185:
/* r2r3dist */
CONSTANTS[1] = -0.519482;
/* r2r3dist */
CONSTANTS[2] = 10.0400;
break;
case 186:
/* r2r3dist */
CONSTANTS[1] = 0.267195;
/* r2r3dist */
CONSTANTS[2] = 9.65796;
break;
case 187:
/* r2r3dist */
CONSTANTS[1] = 0.350331;
/* r2r3dist */
CONSTANTS[2] = 10.2407;
break;
case 188:
/* r2r3dist */
CONSTANTS[1] = -0.345025;
/* r2r3dist */
CONSTANTS[2] = 9.81514;
break;
case 189:
/* r2r3dist */
CONSTANTS[1] = -0.658426;
/* r2r3dist */
CONSTANTS[2] = 10.2496;
break;
case 190:
/* r2r3dist */
CONSTANTS[1] = 0.560210;
/* r2r3dist */
CONSTANTS[2] = 9.82967;
break;
case 191:
/* r2r3dist */
CONSTANTS[1] = -0.478582;
/* r2r3dist */
CONSTANTS[2] = 9.35953;
break;
case 192:
/* r2r3dist */
CONSTANTS[1] = 0.0428047;
/* r2r3dist */
CONSTANTS[2] = 10.7671;
break;
case 193:
/* r2r3dist */
CONSTANTS[1] = -0.707723;
/* r2r3dist */
CONSTANTS[2] = 8.94048;
break;
case 194:
/* r2r3dist */
CONSTANTS[1] = 0.124503;
/* r2r3dist */
CONSTANTS[2] = 9.16413;
break;
case 195:
/* r2r3dist */
CONSTANTS[1] = -0.473808;
/* r2r3dist */
CONSTANTS[2] = 11.0087;
break;
case 196:
/* r2r3dist */
CONSTANTS[1] = 2.31470;
/* r2r3dist */
CONSTANTS[2] = 10.5245;
break;
case 197:
/* r2r3dist */
CONSTANTS[1] = -1.94560;
/* r2r3dist */
CONSTANTS[2] = 9.50325;
break;
case 198:
/* r2r3dist */
CONSTANTS[1] = -0.715881;
/* r2r3dist */
CONSTANTS[2] = 8.87352;
break;
case 199:
/* r2r3dist */
CONSTANTS[1] = -1.03730;
/* r2r3dist */
CONSTANTS[2] = 9.48532;
break;
case 200:
/* r2r3dist */
CONSTANTS[1] = -0.497264;
/* r2r3dist */
CONSTANTS[2] = 10.0143;
break;
case 201:
/* r2r3dist */
CONSTANTS[1] = -1.10151;
/* r2r3dist */
CONSTANTS[2] = 9.66712;
break;
case 202:
/* r2r3dist */
CONSTANTS[1] = 1.96568;
/* r2r3dist */
CONSTANTS[2] = 10.5761;
break;
case 203:
/* r2r3dist */
CONSTANTS[1] = 1.34832;
/* r2r3dist */
CONSTANTS[2] = 11.1216;
break;
case 204:
/* r2r3dist */
CONSTANTS[1] = -2.43467;
/* r2r3dist */
CONSTANTS[2] = 9.06855;
break;
case 205:
/* r2r3dist */
CONSTANTS[1] = -0.371626;
/* r2r3dist */
CONSTANTS[2] = 8.91190;
break;
case 206:
/* r2r3dist */
CONSTANTS[1] = -0.605104;
/* r2r3dist */
CONSTANTS[2] = 10.5041;
break;
case 207:
/* r2r3dist */
CONSTANTS[1] = 2.23979;
/* r2r3dist */
CONSTANTS[2] = 11.2441;
break;
case 208:
/* r2r3dist */
CONSTANTS[1] = 0.130584;
/* r2r3dist */
CONSTANTS[2] = 9.97309;
break;
case 209:
/* r2r3dist */
CONSTANTS[1] = -1.60687;
/* r2r3dist */
CONSTANTS[2] = 9.12180;
break;
case 210:
/* r2r3dist */
CONSTANTS[1] = -0.834117;
/* r2r3dist */
CONSTANTS[2] = 8.92628;
break;
case 211:
/* r2r3dist */
CONSTANTS[1] = -0.169646;
/* r2r3dist */
CONSTANTS[2] = 11.2638;
break;
case 212:
/* r2r3dist */
CONSTANTS[1] = -0.776054;
/* r2r3dist */
CONSTANTS[2] = 10.7188;
break;
case 213:
/* r2r3dist */
CONSTANTS[1] = -1.28258;
/* r2r3dist */
CONSTANTS[2] = 8.80018;
break;
case 214:
/* r2r3dist */
CONSTANTS[1] = 2.48610;
/* r2r3dist */
CONSTANTS[2] = 11.4168;
break;
case 215:
/* r2r3dist */
CONSTANTS[1] = -0.440228;
/* r2r3dist */
CONSTANTS[2] = 9.62551;
break;
case 216:
/* r2r3dist */
CONSTANTS[1] = -0.402632;
/* r2r3dist */
CONSTANTS[2] = 10.4416;
break;
case 217:
/* r2r3dist */
CONSTANTS[1] = -0.0529521;
/* r2r3dist */
CONSTANTS[2] = 10.6863;
break;
case 218:
/* r2r3dist */
CONSTANTS[1] = 0.618992;
/* r2r3dist */
CONSTANTS[2] = 10.7826;
break;
case 219:
/* r2r3dist */
CONSTANTS[1] = -0.300217;
/* r2r3dist */
CONSTANTS[2] = 10.4268;
break;
case 220:
/* r2r3dist */
CONSTANTS[1] = -0.552240;
/* r2r3dist */
CONSTANTS[2] = 10.0846;
break;
case 221:
/* r2r3dist */
CONSTANTS[1] = -0.983473;
/* r2r3dist */
CONSTANTS[2] = 9.54819;
break;
case 222:
/* r2r3dist */
CONSTANTS[1] = -0.720989;
/* r2r3dist */
CONSTANTS[2] = 9.76708;
break;
case 223:
/* r2r3dist */
CONSTANTS[1] = -0.313844;
/* r2r3dist */
CONSTANTS[2] = 9.40583;
break;
case 224:
/* r2r3dist */
CONSTANTS[1] = -0.114939;
/* r2r3dist */
CONSTANTS[2] = 10.0928;
break;
case 225:
/* r2r3dist */
CONSTANTS[1] = -0.0309663;
/* r2r3dist */
CONSTANTS[2] = 10.0549;
break;
case 226:
/* r2r3dist */
CONSTANTS[1] = 0.972894;
/* r2r3dist */
CONSTANTS[2] = 10.6256;
break;
case 227:
/* r2r3dist */
CONSTANTS[1] = -1.30449;
/* r2r3dist */
CONSTANTS[2] = 8.08119;
break;
case 228:
/* r2r3dist */
CONSTANTS[1] = -0.370204;
/* r2r3dist */
CONSTANTS[2] = 9.54435;
break;
case 229:
/* r2r3dist */
CONSTANTS[1] = -0.406109;
/* r2r3dist */
CONSTANTS[2] = 9.43918;
break;
case 230:
/* r2r3dist */
CONSTANTS[1] = -0.794491;
/* r2r3dist */
CONSTANTS[2] = 9.11377;
break;
case 231:
/* r2r3dist */
CONSTANTS[1] = 0.388608;
/* r2r3dist */
CONSTANTS[2] = 10.1221;
break;
case 232:
/* r2r3dist */
CONSTANTS[1] = 0.194060;
/* r2r3dist */
CONSTANTS[2] = 10.8202;
break;
case 233:
/* r2r3dist */
CONSTANTS[1] = -1.15400;
/* r2r3dist */
CONSTANTS[2] = 9.53673;
break;
case 234:
/* r2r3dist */
CONSTANTS[1] = 1.48574;
/* r2r3dist */
CONSTANTS[2] = 10.5935;
break;
case 235:
/* r2r3dist */
CONSTANTS[1] = 0.791562;
/* r2r3dist */
CONSTANTS[2] = 10.3585;
break;
case 236:
/* r2r3dist */
CONSTANTS[1] = 0.202493;
/* r2r3dist */
CONSTANTS[2] = 10.2991;
break;
case 237:
/* r2r3dist */
CONSTANTS[1] = 0.410946;
/* r2r3dist */
CONSTANTS[2] = 10.8463;
break;
case 238:
/* r2r3dist */
CONSTANTS[1] = 0.853777;
/* r2r3dist */
CONSTANTS[2] = 11.2058;
break;
case 239:
/* r2r3dist */
CONSTANTS[1] = -0.993165;
/* r2r3dist */
CONSTANTS[2] = 9.49733;
break;
case 240:
/* r2r3dist */
CONSTANTS[1] = -0.534665;
/* r2r3dist */
CONSTANTS[2] = 10.3356;
break;
case 241:
/* r2r3dist */
CONSTANTS[1] = -0.237033;
/* r2r3dist */
CONSTANTS[2] = 10.1159;
break;
case 242:
/* r2r3dist */
CONSTANTS[1] = -0.482345;
/* r2r3dist */
CONSTANTS[2] = 9.41361;
break;
case 243:
/* r2r3dist */
CONSTANTS[1] = -0.638438;
/* r2r3dist */
CONSTANTS[2] = 9.35939;
break;
case 244:
/* r2r3dist */
CONSTANTS[1] = -1.64067;
/* r2r3dist */
CONSTANTS[2] = 8.60358;
break;
case 245:
/* r2r3dist */
CONSTANTS[1] = 0.215740;
/* r2r3dist */
CONSTANTS[2] = 9.68979;
break;
case 246:
/* r2r3dist */
CONSTANTS[1] = 0.433980;
/* r2r3dist */
CONSTANTS[2] = 10.2781;
break;
case 247:
/* r2r3dist */
CONSTANTS[1] = 1.17371;
/* r2r3dist */
CONSTANTS[2] = 9.78035;
break;
case 248:
/* r2r3dist */
CONSTANTS[1] = -1.01766;
/* r2r3dist */
CONSTANTS[2] = 10.5270;
break;
case 249:
/* r2r3dist */
CONSTANTS[1] = 1.58844;
/* r2r3dist */
CONSTANTS[2] = 11.1309;
break;
case 250:
/* r2r3dist */
CONSTANTS[1] = 1.20276;
/* r2r3dist */
CONSTANTS[2] = 10.7994;
break;
case 251:
/* r2r3dist */
CONSTANTS[1] = 0.524435;
/* r2r3dist */
CONSTANTS[2] = 10.1868;
break;
case 252:
/* r2r3dist */
CONSTANTS[1] = -1.32934;
/* r2r3dist */
CONSTANTS[2] = 10.0750;
break;
case 253:
/* r2r3dist */
CONSTANTS[1] = -1.01268;
/* r2r3dist */
CONSTANTS[2] = 9.78126;
break;
case 254:
/* r2r3dist */
CONSTANTS[1] = 1.40147;
/* r2r3dist */
CONSTANTS[2] = 10.8712;
break;
case 255:
/* r2r3dist */
CONSTANTS[1] = 0.704961;
/* r2r3dist */
CONSTANTS[2] = 9.97849;
break;
case 256:
/* r2r3dist */
CONSTANTS[1] = 0.242236;
/* r2r3dist */
CONSTANTS[2] = 10.1719;
break;
case 257:
/* r2r3dist */
CONSTANTS[1] = -0.117170;
/* r2r3dist */
CONSTANTS[2] = 9.95008;
break;
case 258:
/* r2r3dist */
CONSTANTS[1] = 1.73312;
/* r2r3dist */
CONSTANTS[2] = 11.0988;
break;
case 259:
/* r2r3dist */
CONSTANTS[1] = 0.271606;
/* r2r3dist */
CONSTANTS[2] = 9.93099;
break;
case 260:
/* r2r3dist */
CONSTANTS[1] = -0.905859;
/* r2r3dist */
CONSTANTS[2] = 10.3144;
break;
case 261:
/* r2r3dist */
CONSTANTS[1] = 0.175520;
/* r2r3dist */
CONSTANTS[2] = 9.80816;
break;
case 262:
/* r2r3dist */
CONSTANTS[1] = 0.412280;
/* r2r3dist */
CONSTANTS[2] = 9.93709;
break;
case 263:
/* r2r3dist */
CONSTANTS[1] = 2.99631;
/* r2r3dist */
CONSTANTS[2] = 12.0821;
break;
case 264:
/* r2r3dist */
CONSTANTS[1] = -3.01763;
/* r2r3dist */
CONSTANTS[2] = 8.33325;
break;
case 265:
/* r2r3dist */
CONSTANTS[1] = -0.727385;
/* r2r3dist */
CONSTANTS[2] = 8.99378;
break;
case 266:
/* r2r3dist */
CONSTANTS[1] = 0.0157086;
/* r2r3dist */
CONSTANTS[2] = 10.5865;
break;
case 267:
/* r2r3dist */
CONSTANTS[1] = -0.286581;
/* r2r3dist */
CONSTANTS[2] = 10.7517;
break;
case 268:
/* r2r3dist */
CONSTANTS[1] = 0.938769;
/* r2r3dist */
CONSTANTS[2] = 10.3930;
break;
case 269:
/* r2r3dist */
CONSTANTS[1] = 0.723831;
/* r2r3dist */
CONSTANTS[2] = 9.78356;
break;
case 270:
/* r2r3dist */
CONSTANTS[1] = 0.830976;
/* r2r3dist */
CONSTANTS[2] = 10.8575;
break;
case 271:
/* r2r3dist */
CONSTANTS[1] = -0.704115;
/* r2r3dist */
CONSTANTS[2] = 9.45562;
break;
case 272:
/* r2r3dist */
CONSTANTS[1] = 0.408444;
/* r2r3dist */
CONSTANTS[2] = 10.4821;
break;
case 273:
/* r2r3dist */
CONSTANTS[1] = -0.486152;
/* r2r3dist */
CONSTANTS[2] = 9.38259;
break;
case 274:
/* r2r3dist */
CONSTANTS[1] = 1.39911;
/* r2r3dist */
CONSTANTS[2] = 10.6266;
break;
case 275:
/* r2r3dist */
CONSTANTS[1] = 0.732713;
/* r2r3dist */
CONSTANTS[2] = 10.2012;
break;
case 276:
/* r2r3dist */
CONSTANTS[1] = -0.00993686;
/* r2r3dist */
CONSTANTS[2] = 9.69335;
break;
case 277:
/* r2r3dist */
CONSTANTS[1] = -0.869886;
/* r2r3dist */
CONSTANTS[2] = 10.0473;
break;
case 278:
/* r2r3dist */
CONSTANTS[1] = 0.709321;
/* r2r3dist */
CONSTANTS[2] = 10.8147;
break;
case 279:
/* r2r3dist */
CONSTANTS[1] = -0.739368;
/* r2r3dist */
CONSTANTS[2] = 9.05034;
break;
case 280:
/* r2r3dist */
CONSTANTS[1] = 1.12651;
/* r2r3dist */
CONSTANTS[2] = 10.4266;
break;
case 281:
/* r2r3dist */
CONSTANTS[1] = -0.824310;
/* r2r3dist */
CONSTANTS[2] = 9.01544;
break;
case 282:
/* r2r3dist */
CONSTANTS[1] = -0.681181;
/* r2r3dist */
CONSTANTS[2] = 9.33498;
break;
case 283:
/* r2r3dist */
CONSTANTS[1] = -0.814640;
/* r2r3dist */
CONSTANTS[2] = 9.32083;
break;
case 284:
/* r2r3dist */
CONSTANTS[1] = 0.0828610;
/* r2r3dist */
CONSTANTS[2] = 10.1460;
break;
case 285:
/* r2r3dist */
CONSTANTS[1] = -0.635449;
/* r2r3dist */
CONSTANTS[2] = 10.3652;
break;
case 286:
/* r2r3dist */
CONSTANTS[1] = 1.06753;
/* r2r3dist */
CONSTANTS[2] = 10.9575;
break;
case 287:
/* r2r3dist */
CONSTANTS[1] = -0.724906;
/* r2r3dist */
CONSTANTS[2] = 9.25978;
break;
case 288:
/* r2r3dist */
CONSTANTS[1] = 1.25197;
/* r2r3dist */
CONSTANTS[2] = 10.0278;
break;
case 289:
/* r2r3dist */
CONSTANTS[1] = -0.364233;
/* r2r3dist */
CONSTANTS[2] = 10.5406;
break;
case 290:
/* r2r3dist */
CONSTANTS[1] = -0.990632;
/* r2r3dist */
CONSTANTS[2] = 9.18209;
break;
case 291:
/* r2r3dist */
CONSTANTS[1] = 0.741541;
/* r2r3dist */
CONSTANTS[2] = 10.6190;
break;
case 292:
/* r2r3dist */
CONSTANTS[1] = -0.354966;
/* r2r3dist */
CONSTANTS[2] = 9.84978;
break;
case 293:
/* r2r3dist */
CONSTANTS[1] = -1.14659;
/* r2r3dist */
CONSTANTS[2] = 10.4718;
break;
case 294:
/* r2r3dist */
CONSTANTS[1] = -0.236549;
/* r2r3dist */
CONSTANTS[2] = 10.2378;
break;
case 295:
/* r2r3dist */
CONSTANTS[1] = -0.582285;
/* r2r3dist */
CONSTANTS[2] = 9.52304;
break;
case 296:
/* r2r3dist */
CONSTANTS[1] = 1.44766;
/* r2r3dist */
CONSTANTS[2] = 10.3251;
break;
case 297:
/* r2r3dist */
CONSTANTS[1] = 0.728650;
/* r2r3dist */
CONSTANTS[2] = 9.74946;
break;
case 298:
/* r2r3dist */
CONSTANTS[1] = 0.914210;
/* r2r3dist */
CONSTANTS[2] = 10.6848;
break;
case 299:
/* r2r3dist */
CONSTANTS[1] = -0.356111;
/* r2r3dist */
CONSTANTS[2] = 9.20544;
break;
case 300:
/* r2r3dist */
CONSTANTS[1] = 1.01582;
/* r2r3dist */
CONSTANTS[2] = 10.6114;
break;
case 301:
/* r2r3dist */
CONSTANTS[1] = -0.361836;
/* r2r3dist */
CONSTANTS[2] = 9.57746;
break;
case 302:
/* r2r3dist */
CONSTANTS[1] = -0.0843881;
/* r2r3dist */
CONSTANTS[2] = 9.80090;
break;
case 303:
/* r2r3dist */
CONSTANTS[1] = -1.27522;
/* r2r3dist */
CONSTANTS[2] = 9.74307;
break;
case 304:
/* r2r3dist */
CONSTANTS[1] = 0.229748;
/* r2r3dist */
CONSTANTS[2] = 9.15077;
break;
case 305:
/* r2r3dist */
CONSTANTS[1] = 2.07026;
/* r2r3dist */
CONSTANTS[2] = 10.4920;
break;
case 306:
/* r2r3dist */
CONSTANTS[1] = 0.246223;
/* r2r3dist */
CONSTANTS[2] = 9.31843;
break;
case 307:
/* r2r3dist */
CONSTANTS[1] = -0.732375;
/* r2r3dist */
CONSTANTS[2] = 9.91271;
break;
case 308:
/* r2r3dist */
CONSTANTS[1] = -1.30478;
/* r2r3dist */
CONSTANTS[2] = 9.04656;
break;
case 309:
/* r2r3dist */
CONSTANTS[1] = -0.978047;
/* r2r3dist */
CONSTANTS[2] = 9.66303;
break;
case 310:
/* r2r3dist */
CONSTANTS[1] = -0.304513;
/* r2r3dist */
CONSTANTS[2] = 10.0827;
break;
case 311:
/* r2r3dist */
CONSTANTS[1] = -0.150403;
/* r2r3dist */
CONSTANTS[2] = 9.56690;
break;
case 312:
/* r2r3dist */
CONSTANTS[1] = -1.54318;
/* r2r3dist */
CONSTANTS[2] = 8.35636;
break;
case 313:
/* r2r3dist */
CONSTANTS[1] = -0.993495;
/* r2r3dist */
CONSTANTS[2] = 10.0807;
break;
case 314:
/* r2r3dist */
CONSTANTS[1] = -0.183647;
/* r2r3dist */
CONSTANTS[2] = 9.89752;
break;
case 315:
/* r2r3dist */
CONSTANTS[1] = 0.249767;
/* r2r3dist */
CONSTANTS[2] = 9.97534;
break;
case 316:
/* r2r3dist */
CONSTANTS[1] = 1.46905;
/* r2r3dist */
CONSTANTS[2] = 11.3046;
break;
case 317:
/* r2r3dist */
CONSTANTS[1] = 0.610601;
/* r2r3dist */
CONSTANTS[2] = 10.2472;
break;
case 318:
/* r2r3dist */
CONSTANTS[1] = -0.198613;
/* r2r3dist */
CONSTANTS[2] = 9.78133;
break;
case 319:
/* r2r3dist */
CONSTANTS[1] = 0.775906;
/* r2r3dist */
CONSTANTS[2] = 11.5253;
break;
case 320:
/* r2r3dist */
CONSTANTS[1] = -1.78648;
/* r2r3dist */
CONSTANTS[2] = 9.66713;
break;
case 321:
/* r2r3dist */
CONSTANTS[1] = -0.496963;
/* r2r3dist */
CONSTANTS[2] = 9.61829;
break;
case 322:
/* r2r3dist */
CONSTANTS[1] = -0.100650;
/* r2r3dist */
CONSTANTS[2] = 10.7001;
break;
case 323:
/* r2r3dist */
CONSTANTS[1] = -0.944006;
/* r2r3dist */
CONSTANTS[2] = 9.89879;
break;
case 324:
/* r2r3dist */
CONSTANTS[1] = 0.0404770;
/* r2r3dist */
CONSTANTS[2] = 9.41798;
break;
case 325:
/* r2r3dist */
CONSTANTS[1] = -0.0147684;
/* r2r3dist */
CONSTANTS[2] = 9.53036;
break;
case 326:
/* r2r3dist */
CONSTANTS[1] = 0.0887674;
/* r2r3dist */
CONSTANTS[2] = 10.1302;
break;
case 327:
/* r2r3dist */
CONSTANTS[1] = 0.782595;
/* r2r3dist */
CONSTANTS[2] = 9.88518;
break;
case 328:
/* r2r3dist */
CONSTANTS[1] = -0.489841;
/* r2r3dist */
CONSTANTS[2] = 9.65013;
break;
case 329:
/* r2r3dist */
CONSTANTS[1] = -0.322764;
/* r2r3dist */
CONSTANTS[2] = 9.05272;
break;
case 330:
/* r2r3dist */
CONSTANTS[1] = -0.354850;
/* r2r3dist */
CONSTANTS[2] = 10.4625;
break;
case 331:
/* r2r3dist */
CONSTANTS[1] = 1.08722;
/* r2r3dist */
CONSTANTS[2] = 10.6515;
break;
case 332:
/* r2r3dist */
CONSTANTS[1] = 0.257476;
/* r2r3dist */
CONSTANTS[2] = 9.85615;
break;
case 333:
/* r2r3dist */
CONSTANTS[1] = -0.730117;
/* r2r3dist */
CONSTANTS[2] = 9.69705;
break;
case 334:
/* r2r3dist */
CONSTANTS[1] = 0.315660;
/* r2r3dist */
CONSTANTS[2] = 10.5815;
break;
case 335:
/* r2r3dist */
CONSTANTS[1] = 0.0507933;
/* r2r3dist */
CONSTANTS[2] = 9.36234;
break;
case 336:
/* r2r3dist */
CONSTANTS[1] = 0.0895123;
/* r2r3dist */
CONSTANTS[2] = 10.4958;
break;
case 337:
/* r2r3dist */
CONSTANTS[1] = 0.507961;
/* r2r3dist */
CONSTANTS[2] = 9.42056;
break;
case 338:
/* r2r3dist */
CONSTANTS[1] = 2.32480;
/* r2r3dist */
CONSTANTS[2] = 11.2680;
break;
case 339:
/* r2r3dist */
CONSTANTS[1] = 0.701741;
/* r2r3dist */
CONSTANTS[2] = 9.86404;
break;
case 340:
/* r2r3dist */
CONSTANTS[1] = -0.830073;
/* r2r3dist */
CONSTANTS[2] = 10.1170;
break;
case 341:
/* r2r3dist */
CONSTANTS[1] = 0.806094;
/* r2r3dist */
CONSTANTS[2] = 9.95862;
break;
case 342:
/* r2r3dist */
CONSTANTS[1] = 0.399277;
/* r2r3dist */
CONSTANTS[2] = 10.0397;
break;
case 343:
/* r2r3dist */
CONSTANTS[1] = 1.33871;
/* r2r3dist */
CONSTANTS[2] = 10.4454;
break;
case 344:
/* r2r3dist */
CONSTANTS[1] = -0.257457;
/* r2r3dist */
CONSTANTS[2] = 9.75569;
break;
case 345:
/* r2r3dist */
CONSTANTS[1] = 0.152115;
/* r2r3dist */
CONSTANTS[2] = 9.71261;
break;
case 346:
/* r2r3dist */
CONSTANTS[1] = 0.151250;
/* r2r3dist */
CONSTANTS[2] = 10.1712;
break;
case 347:
/* r2r3dist */
CONSTANTS[1] = 0.591541;
/* r2r3dist */
CONSTANTS[2] = 11.1063;
break;
case 348:
/* r2r3dist */
CONSTANTS[1] = 1.37580;
/* r2r3dist */
CONSTANTS[2] = 10.2575;
break;
case 349:
/* r2r3dist */
CONSTANTS[1] = 0.0823245;
/* r2r3dist */
CONSTANTS[2] = 9.75813;
break;
case 350:
/* r2r3dist */
CONSTANTS[1] = 0.0215586;
/* r2r3dist */
CONSTANTS[2] = 10.0095;
break;
case 351:
/* r2r3dist */
CONSTANTS[1] = -0.208262;
/* r2r3dist */
CONSTANTS[2] = 9.70960;
break;
case 352:
/* r2r3dist */
CONSTANTS[1] = -0.0454611;
/* r2r3dist */
CONSTANTS[2] = 10.0737;
break;
case 353:
/* r2r3dist */
CONSTANTS[1] = -0.181892;
/* r2r3dist */
CONSTANTS[2] = 10.5023;
break;
case 354:
/* r2r3dist */
CONSTANTS[1] = 0.956651;
/* r2r3dist */
CONSTANTS[2] = 10.8997;
break;
case 355:
/* r2r3dist */
CONSTANTS[1] = -0.432014;
/* r2r3dist */
CONSTANTS[2] = 9.23958;
break;
case 356:
/* r2r3dist */
CONSTANTS[1] = 1.12978;
/* r2r3dist */
CONSTANTS[2] = 10.5368;
break;
case 357:
/* r2r3dist */
CONSTANTS[1] = -0.524785;
/* r2r3dist */
CONSTANTS[2] = 8.92056;
break;
case 358:
/* r2r3dist */
CONSTANTS[1] = -0.961467;
/* r2r3dist */
CONSTANTS[2] = 9.47764;
break;
case 359:
/* r2r3dist */
CONSTANTS[1] = -0.902691;
/* r2r3dist */
CONSTANTS[2] = 9.24721;
break;
case 360:
/* r2r3dist */
CONSTANTS[1] = 0.550728;
/* r2r3dist */
CONSTANTS[2] = 10.8911;
break;
case 361:
/* r2r3dist */
CONSTANTS[1] = -0.334659;
/* r2r3dist */
CONSTANTS[2] = 9.95086;
break;
case 362:
/* r2r3dist */
CONSTANTS[1] = 0.923508;
/* r2r3dist */
CONSTANTS[2] = 9.97101;
break;
case 363:
/* r2r3dist */
CONSTANTS[1] = 1.23269;
/* r2r3dist */
CONSTANTS[2] = 10.7724;
break;
case 364:
/* r2r3dist */
CONSTANTS[1] = 0.448378;
/* r2r3dist */
CONSTANTS[2] = 10.3599;
break;
case 365:
/* r2r3dist */
CONSTANTS[1] = 1.12161;
/* r2r3dist */
CONSTANTS[2] = 10.6718;
break;
case 366:
/* r2r3dist */
CONSTANTS[1] = -0.974888;
/* r2r3dist */
CONSTANTS[2] = 9.05492;
break;
case 367:
/* r2r3dist */
CONSTANTS[1] = 0.226185;
/* r2r3dist */
CONSTANTS[2] = 10.4324;
break;
case 368:
/* r2r3dist */
CONSTANTS[1] = 0.546751;
/* r2r3dist */
CONSTANTS[2] = 11.2115;
break;
case 369:
/* r2r3dist */
CONSTANTS[1] = -0.344231;
/* r2r3dist */
CONSTANTS[2] = 9.62057;
break;
case 370:
/* r2r3dist */
CONSTANTS[1] = -1.21701;
/* r2r3dist */
CONSTANTS[2] = 9.07301;
break;
case 371:
/* r2r3dist */
CONSTANTS[1] = 0.0575525;
/* r2r3dist */
CONSTANTS[2] = 10.1561;
break;
case 372:
/* r2r3dist */
CONSTANTS[1] = -0.104721;
/* r2r3dist */
CONSTANTS[2] = 10.3032;
break;
case 373:
/* r2r3dist */
CONSTANTS[1] = 0.636903;
/* r2r3dist */
CONSTANTS[2] = 10.3563;
break;
case 374:
/* r2r3dist */
CONSTANTS[1] = 0.664935;
/* r2r3dist */
CONSTANTS[2] = 10.9770;
break;
case 375:
/* r2r3dist */
CONSTANTS[1] = -0.492528;
/* r2r3dist */
CONSTANTS[2] = 9.35389;
break;
case 376:
/* r2r3dist */
CONSTANTS[1] = -1.11057;
/* r2r3dist */
CONSTANTS[2] = 9.66904;
break;
case 377:
/* r2r3dist */
CONSTANTS[1] = 0.965508;
/* r2r3dist */
CONSTANTS[2] = 11.0908;
break;
case 378:
/* r2r3dist */
CONSTANTS[1] = -1.72447;
/* r2r3dist */
CONSTANTS[2] = 8.16322;
break;
case 379:
/* r2r3dist */
CONSTANTS[1] = -0.147483;
/* r2r3dist */
CONSTANTS[2] = 9.74561;
break;
case 380:
/* r2r3dist */
CONSTANTS[1] = 1.12219;
/* r2r3dist */
CONSTANTS[2] = 10.3062;
break;
case 381:
/* r2r3dist */
CONSTANTS[1] = 1.93998;
/* r2r3dist */
CONSTANTS[2] = 11.0952;
break;
case 382:
/* r2r3dist */
CONSTANTS[1] = -1.29926;
/* r2r3dist */
CONSTANTS[2] = 8.77189;
break;
case 383:
/* r2r3dist */
CONSTANTS[1] = -0.199801;
/* r2r3dist */
CONSTANTS[2] = 9.37340;
break;
case 384:
/* r2r3dist */
CONSTANTS[1] = 0.585881;
/* r2r3dist */
CONSTANTS[2] = 10.1899;
break;
case 385:
/* r2r3dist */
CONSTANTS[1] = -0.850936;
/* r2r3dist */
CONSTANTS[2] = 9.99370;
break;
case 386:
/* r2r3dist */
CONSTANTS[1] = -0.407387;
/* r2r3dist */
CONSTANTS[2] = 9.94165;
break;
case 387:
/* r2r3dist */
CONSTANTS[1] = 0.715235;
/* r2r3dist */
CONSTANTS[2] = 10.5723;
break;
case 388:
/* r2r3dist */
CONSTANTS[1] = 0.546907;
/* r2r3dist */
CONSTANTS[2] = 10.8288;
break;
case 389:
/* r2r3dist */
CONSTANTS[1] = 0.683153;
/* r2r3dist */
CONSTANTS[2] = 10.4300;
break;
case 390:
/* r2r3dist */
CONSTANTS[1] = 1.17422;
/* r2r3dist */
CONSTANTS[2] = 10.4537;
break;
case 391:
/* r2r3dist */
CONSTANTS[1] = 1.61563;
/* r2r3dist */
CONSTANTS[2] = 10.5203;
break;
case 392:
/* r2r3dist */
CONSTANTS[1] = 0.935109;
/* r2r3dist */
CONSTANTS[2] = 10.5779;
break;
case 393:
/* r2r3dist */
CONSTANTS[1] = -0.376379;
/* r2r3dist */
CONSTANTS[2] = 9.67100;
break;
case 394:
/* r2r3dist */
CONSTANTS[1] = 0.412832;
/* r2r3dist */
CONSTANTS[2] = 10.6364;
break;
case 395:
/* r2r3dist */
CONSTANTS[1] = 1.18668;
/* r2r3dist */
CONSTANTS[2] = 9.58270;
break;
case 396:
/* r2r3dist */
CONSTANTS[1] = 0.888008;
/* r2r3dist */
CONSTANTS[2] = 9.31945;
break;
case 397:
/* r2r3dist */
CONSTANTS[1] = -1.21579;
/* r2r3dist */
CONSTANTS[2] = 9.32189;
break;
case 398:
/* r2r3dist */
CONSTANTS[1] = -0.965409;
/* r2r3dist */
CONSTANTS[2] = 9.34154;
break;
case 399:
/* r2r3dist */
CONSTANTS[1] = 0.351405;
/* r2r3dist */
CONSTANTS[2] = 10.5833;
break;
case 400:
/* r2r3dist */
CONSTANTS[1] = 1.27509;
/* r2r3dist */
CONSTANTS[2] = 10.1668;
break;
case 401:
/* r2r3dist */
CONSTANTS[1] = 2.27365;
/* r2r3dist */
CONSTANTS[2] = 12.2267;
break;
case 402:
/* r2r3dist */
CONSTANTS[1] = -0.346561;
/* r2r3dist */
CONSTANTS[2] = 9.38031;
break;
case 403:
/* r2r3dist */
CONSTANTS[1] = 0.539864;
/* r2r3dist */
CONSTANTS[2] = 10.3647;
break;
case 404:
/* r2r3dist */
CONSTANTS[1] = 3.27730;
/* r2r3dist */
CONSTANTS[2] = 11.6012;
break;
case 405:
/* r2r3dist */
CONSTANTS[1] = -0.817065;
/* r2r3dist */
CONSTANTS[2] = 9.77023;
break;
case 406:
/* r2r3dist */
CONSTANTS[1] = 1.65362;
/* r2r3dist */
CONSTANTS[2] = 10.2156;
break;
case 407:
/* r2r3dist */
CONSTANTS[1] = -1.75359;
/* r2r3dist */
CONSTANTS[2] = 8.66985;
break;
case 408:
/* r2r3dist */
CONSTANTS[1] = -1.49925;
/* r2r3dist */
CONSTANTS[2] = 9.47209;
break;
case 409:
/* r2r3dist */
CONSTANTS[1] = 0.111396;
/* r2r3dist */
CONSTANTS[2] = 10.3601;
break;
case 410:
/* r2r3dist */
CONSTANTS[1] = -0.0907533;
/* r2r3dist */
CONSTANTS[2] = 9.41955;
break;
case 411:
/* r2r3dist */
CONSTANTS[1] = -0.663116;
/* r2r3dist */
CONSTANTS[2] = 9.64686;
break;
case 412:
/* r2r3dist */
CONSTANTS[1] = -1.57689;
/* r2r3dist */
CONSTANTS[2] = 9.20958;
break;
case 413:
/* r2r3dist */
CONSTANTS[1] = 1.05311;
/* r2r3dist */
CONSTANTS[2] = 10.4465;
break;
case 414:
/* r2r3dist */
CONSTANTS[1] = -0.486301;
/* r2r3dist */
CONSTANTS[2] = 9.91196;
break;
case 415:
/* r2r3dist */
CONSTANTS[1] = -1.12938;
/* r2r3dist */
CONSTANTS[2] = 9.08112;
break;
case 416:
/* r2r3dist */
CONSTANTS[1] = 0.646075;
/* r2r3dist */
CONSTANTS[2] = 10.1921;
break;
case 417:
/* r2r3dist */
CONSTANTS[1] = -0.704567;
/* r2r3dist */
CONSTANTS[2] = 9.66739;
break;
case 418:
/* r2r3dist */
CONSTANTS[1] = -0.752939;
/* r2r3dist */
CONSTANTS[2] = 9.26568;
break;
case 419:
/* r2r3dist */
CONSTANTS[1] = 0.899789;
/* r2r3dist */
CONSTANTS[2] = 8.62899;
break;
case 420:
/* r2r3dist */
CONSTANTS[1] = 1.43674;
/* r2r3dist */
CONSTANTS[2] = 11.1109;
break;
case 421:
/* r2r3dist */
CONSTANTS[1] = 2.35949;
/* r2r3dist */
CONSTANTS[2] = 11.9758;
break;
case 422:
/* r2r3dist */
CONSTANTS[1] = 0.0929581;
/* r2r3dist */
CONSTANTS[2] = 9.42542;
break;
case 423:
/* r2r3dist */
CONSTANTS[1] = 1.03410;
/* r2r3dist */
CONSTANTS[2] = 10.1089;
break;
case 424:
/* r2r3dist */
CONSTANTS[1] = -1.21878;
/* r2r3dist */
CONSTANTS[2] = 9.16772;
break;
case 425:
/* r2r3dist */
CONSTANTS[1] = 0.351029;
/* r2r3dist */
CONSTANTS[2] = 9.65057;
break;
case 426:
/* r2r3dist */
CONSTANTS[1] = -1.43901;
/* r2r3dist */
CONSTANTS[2] = 9.17139;
break;
case 427:
/* r2r3dist */
CONSTANTS[1] = 0.586937;
/* r2r3dist */
CONSTANTS[2] = 10.7464;
break;
case 428:
/* r2r3dist */
CONSTANTS[1] = -1.61676;
/* r2r3dist */
CONSTANTS[2] = 9.55986;
break;
case 429:
/* r2r3dist */
CONSTANTS[1] = -1.21429;
/* r2r3dist */
CONSTANTS[2] = 9.33483;
break;
case 430:
/* r2r3dist */
CONSTANTS[1] = -0.908345;
/* r2r3dist */
CONSTANTS[2] = 9.42696;
break;
case 431:
/* r2r3dist */
CONSTANTS[1] = -0.280592;
/* r2r3dist */
CONSTANTS[2] = 10.0751;
break;
case 432:
/* r2r3dist */
CONSTANTS[1] = -0.653744;
/* r2r3dist */
CONSTANTS[2] = 11.1481;
break;
case 433:
/* r2r3dist */
CONSTANTS[1] = -0.769794;
/* r2r3dist */
CONSTANTS[2] = 10.1001;
break;
case 434:
/* r2r3dist */
CONSTANTS[1] = -0.856532;
/* r2r3dist */
CONSTANTS[2] = 9.29220;
break;
case 435:
/* r2r3dist */
CONSTANTS[1] = -0.280135;
/* r2r3dist */
CONSTANTS[2] = 10.8181;
break;
case 436:
/* r2r3dist */
CONSTANTS[1] = 2.00824;
/* r2r3dist */
CONSTANTS[2] = 11.1645;
break;
case 437:
/* r2r3dist */
CONSTANTS[1] = 0.786302;
/* r2r3dist */
CONSTANTS[2] = 10.7160;
break;
case 438:
/* r2r3dist */
CONSTANTS[1] = 0.875345;
/* r2r3dist */
CONSTANTS[2] = 10.1950;
break;
case 439:
/* r2r3dist */
CONSTANTS[1] = -0.0405551;
/* r2r3dist */
CONSTANTS[2] = 10.6112;
break;
case 440:
/* r2r3dist */
CONSTANTS[1] = 0.377342;
/* r2r3dist */
CONSTANTS[2] = 10.4677;
break;
case 441:
/* r2r3dist */
CONSTANTS[1] = 0.944166;
/* r2r3dist */
CONSTANTS[2] = 9.62152;
break;
case 442:
/* r2r3dist */
CONSTANTS[1] = 0.503640;
/* r2r3dist */
CONSTANTS[2] = 10.0450;
break;
case 443:
/* r2r3dist */
CONSTANTS[1] = -0.942394;
/* r2r3dist */
CONSTANTS[2] = 9.78721;
break;
case 444:
/* r2r3dist */
CONSTANTS[1] = 1.24247;
/* r2r3dist */
CONSTANTS[2] = 10.7395;
break;
case 445:
/* r2r3dist */
CONSTANTS[1] = -1.64484;
/* r2r3dist */
CONSTANTS[2] = 9.04132;
break;
case 446:
/* r2r3dist */
CONSTANTS[1] = 0.800602;
/* r2r3dist */
CONSTANTS[2] = 10.5586;
break;
case 447:
/* r2r3dist */
CONSTANTS[1] = 1.72646;
/* r2r3dist */
CONSTANTS[2] = 11.9048;
break;
case 448:
/* r2r3dist */
CONSTANTS[1] = 0.167087;
/* r2r3dist */
CONSTANTS[2] = 10.0911;
break;
case 449:
/* r2r3dist */
CONSTANTS[1] = 0.195967;
/* r2r3dist */
CONSTANTS[2] = 10.1663;
break;
case 450:
/* r2r3dist */
CONSTANTS[1] = 0.601147;
/* r2r3dist */
CONSTANTS[2] = 11.4331;
break;
case 451:
/* r2r3dist */
CONSTANTS[1] = -0.959949;
/* r2r3dist */
CONSTANTS[2] = 9.88850;
break;
case 452:
/* r2r3dist */
CONSTANTS[1] = 1.89912;
/* r2r3dist */
CONSTANTS[2] = 11.1025;
break;
case 453:
/* r2r3dist */
CONSTANTS[1] = 0.508026;
/* r2r3dist */
CONSTANTS[2] = 10.2076;
break;
case 454:
/* r2r3dist */
CONSTANTS[1] = -1.15255;
/* r2r3dist */
CONSTANTS[2] = 9.43635;
break;
case 455:
/* r2r3dist */
CONSTANTS[1] = 1.88267;
/* r2r3dist */
CONSTANTS[2] = 10.8683;
break;
case 456:
/* r2r3dist */
CONSTANTS[1] = -0.124029;
/* r2r3dist */
CONSTANTS[2] = 9.58333;
break;
case 457:
/* r2r3dist */
CONSTANTS[1] = -1.53235;
/* r2r3dist */
CONSTANTS[2] = 9.24947;
break;
case 458:
/* r2r3dist */
CONSTANTS[1] = 3.69761;
/* r2r3dist */
CONSTANTS[2] = 13.0441;
break;
case 459:
/* r2r3dist */
CONSTANTS[1] = -0.826165;
/* r2r3dist */
CONSTANTS[2] = 10.5962;
break;
case 460:
/* r2r3dist */
CONSTANTS[1] = 0.464457;
/* r2r3dist */
CONSTANTS[2] = 9.76980;
break;
case 461:
/* r2r3dist */
CONSTANTS[1] = -1.10147;
/* r2r3dist */
CONSTANTS[2] = 9.47615;
break;
case 462:
/* r2r3dist */
CONSTANTS[1] = 0.0354406;
/* r2r3dist */
CONSTANTS[2] = 9.90579;
break;
case 463:
/* r2r3dist */
CONSTANTS[1] = 0.671877;
/* r2r3dist */
CONSTANTS[2] = 11.2018;
break;
case 464:
/* r2r3dist */
CONSTANTS[1] = -0.507594;
/* r2r3dist */
CONSTANTS[2] = 10.6321;
break;
case 465:
/* r2r3dist */
CONSTANTS[1] = -1.06747;
/* r2r3dist */
CONSTANTS[2] = 10.1960;
break;
case 466:
/* r2r3dist */
CONSTANTS[1] = 0.363329;
/* r2r3dist */
CONSTANTS[2] = 10.4891;
break;
case 467:
/* r2r3dist */
CONSTANTS[1] = 1.62710;
/* r2r3dist */
CONSTANTS[2] = 11.0831;
break;
case 468:
/* r2r3dist */
CONSTANTS[1] = -0.440007;
/* r2r3dist */
CONSTANTS[2] = 9.48834;
break;
case 469:
/* r2r3dist */
CONSTANTS[1] = -1.05077;
/* r2r3dist */
CONSTANTS[2] = 8.95881;
break;
case 470:
/* r2r3dist */
CONSTANTS[1] = -0.727333;
/* r2r3dist */
CONSTANTS[2] = 9.52694;
break;
case 471:
/* r2r3dist */
CONSTANTS[1] = -0.661971;
/* r2r3dist */
CONSTANTS[2] = 9.16460;
break;
case 472:
/* r2r3dist */
CONSTANTS[1] = -0.657821;
/* r2r3dist */
CONSTANTS[2] = 9.39564;
break;
case 473:
/* r2r3dist */
CONSTANTS[1] = -1.21630;
/* r2r3dist */
CONSTANTS[2] = 9.64805;
break;
case 474:
/* r2r3dist */
CONSTANTS[1] = 0.453523;
/* r2r3dist */
CONSTANTS[2] = 9.85691;
break;
case 475:
/* r2r3dist */
CONSTANTS[1] = 0.665175;
/* r2r3dist */
CONSTANTS[2] = 10.4098;
break;
case 476:
/* r2r3dist */
CONSTANTS[1] = -0.690009;
/* r2r3dist */
CONSTANTS[2] = 10.3589;
break;
case 477:
/* r2r3dist */
CONSTANTS[1] = 0.373612;
/* r2r3dist */
CONSTANTS[2] = 10.3439;
break;
case 478:
/* r2r3dist */
CONSTANTS[1] = 0.132482;
/* r2r3dist */
CONSTANTS[2] = 10.1983;
break;
case 479:
/* r2r3dist */
CONSTANTS[1] = -1.96359;
/* r2r3dist */
CONSTANTS[2] = 9.21284;
break;
case 480:
/* r2r3dist */
CONSTANTS[1] = 0.165573;
/* r2r3dist */
CONSTANTS[2] = 10.1481;
break;
case 481:
/* r2r3dist */
CONSTANTS[1] = -0.945926;
/* r2r3dist */
CONSTANTS[2] = 9.28928;
break;
case 482:
/* r2r3dist */
CONSTANTS[1] = 0.783164;
/* r2r3dist */
CONSTANTS[2] = 9.99796;
break;
case 483:
/* r2r3dist */
CONSTANTS[1] = -0.0488695;
/* r2r3dist */
CONSTANTS[2] = 9.94085;
break;
case 484:
/* r2r3dist */
CONSTANTS[1] = 0.384383;
/* r2r3dist */
CONSTANTS[2] = 10.6421;
break;
case 485:
/* r2r3dist */
CONSTANTS[1] = 0.123760;
/* r2r3dist */
CONSTANTS[2] = 10.3180;
break;
case 486:
/* r2r3dist */
CONSTANTS[1] = 0.370242;
/* r2r3dist */
CONSTANTS[2] = 10.1938;
break;
case 487:
/* r2r3dist */
CONSTANTS[1] = 0.768281;
/* r2r3dist */
CONSTANTS[2] = 11.0901;
break;
case 488:
/* r2r3dist */
CONSTANTS[1] = 1.11593;
/* r2r3dist */
CONSTANTS[2] = 11.0530;
break;
case 489:
/* r2r3dist */
CONSTANTS[1] = -0.820506;
/* r2r3dist */
CONSTANTS[2] = 9.99933;
break;
case 490:
/* r2r3dist */
CONSTANTS[1] = -1.83524;
/* r2r3dist */
CONSTANTS[2] = 9.74197;
break;
case 491:
/* r2r3dist */
CONSTANTS[1] = 1.24009;
/* r2r3dist */
CONSTANTS[2] = 9.53807;
break;
case 492:
/* r2r3dist */
CONSTANTS[1] = -0.664850;
/* r2r3dist */
CONSTANTS[2] = 9.35963;
break;
case 493:
/* r2r3dist */
CONSTANTS[1] = 0.512790;
/* r2r3dist */
CONSTANTS[2] = 9.39523;
break;
case 494:
/* r2r3dist */
CONSTANTS[1] = 0.643407;
/* r2r3dist */
CONSTANTS[2] = 10.6516;
break;
case 495:
/* r2r3dist */
CONSTANTS[1] = -1.13597;
/* r2r3dist */
CONSTANTS[2] = 9.59286;
break;
case 496:
/* r2r3dist */
CONSTANTS[1] = -1.13684;
/* r2r3dist */
CONSTANTS[2] = 10.0510;
break;
case 497:
/* r2r3dist */
CONSTANTS[1] = -0.158594;
/* r2r3dist */
CONSTANTS[2] = 9.24374;
break;
case 498:
/* r2r3dist */
CONSTANTS[1] = 0.885642;
/* r2r3dist */
CONSTANTS[2] = 10.3171;
break;
case 499:
/* r2r3dist */
CONSTANTS[1] = 0.314444;
/* r2r3dist */
CONSTANTS[2] = 10.4585;
break;
case 500:
/* r2r3dist */
CONSTANTS[1] = -0.475038;
/* r2r3dist */
CONSTANTS[2] = 9.35973;
break;
case 501:
/* r2r3dist */
CONSTANTS[1] = -0.987897;
/* r2r3dist */
CONSTANTS[2] = 9.32515;
break;
case 502:
/* r2r3dist */
CONSTANTS[1] = 1.23845;
/* r2r3dist */
CONSTANTS[2] = 11.1708;
break;
case 503:
/* r2r3dist */
CONSTANTS[1] = 0.511501;
/* r2r3dist */
CONSTANTS[2] = 10.2405;
break;
case 504:
/* r2r3dist */
CONSTANTS[1] = 1.33582;
/* r2r3dist */
CONSTANTS[2] = 11.0772;
break;
case 505:
/* r2r3dist */
CONSTANTS[1] = -0.400080;
/* r2r3dist */
CONSTANTS[2] = 10.0432;
break;
case 506:
/* r2r3dist */
CONSTANTS[1] = -0.622970;
/* r2r3dist */
CONSTANTS[2] = 9.17166;
break;
case 507:
/* r2r3dist */
CONSTANTS[1] = -0.340851;
/* r2r3dist */
CONSTANTS[2] = 9.00939;
break;
case 508:
/* r2r3dist */
CONSTANTS[1] = 0.438369;
/* r2r3dist */
CONSTANTS[2] = 10.1818;
break;
case 509:
/* r2r3dist */
CONSTANTS[1] = -1.90100;
/* r2r3dist */
CONSTANTS[2] = 8.81728;
break;
case 510:
/* r2r3dist */
CONSTANTS[1] = 0.215920;
/* r2r3dist */
CONSTANTS[2] = 9.34201;
break;
case 511:
/* r2r3dist */
CONSTANTS[1] = -0.230859;
/* r2r3dist */
CONSTANTS[2] = 10.2719;
break;
case 512:
/* r2r3dist */
CONSTANTS[1] = 0.156791;
/* r2r3dist */
CONSTANTS[2] = 9.87068;
break;
case 513:
/* r2r3dist */
CONSTANTS[1] = 1.17138;
/* r2r3dist */
CONSTANTS[2] = 10.7298;
break;
case 514:
/* r2r3dist */
CONSTANTS[1] = -1.26034;
/* r2r3dist */
CONSTANTS[2] = 10.1679;
break;
case 515:
/* r2r3dist */
CONSTANTS[1] = -0.479767;
/* r2r3dist */
CONSTANTS[2] = 9.89906;
break;
case 516:
/* r2r3dist */
CONSTANTS[1] = 1.52862;
/* r2r3dist */
CONSTANTS[2] = 10.7851;
break;
case 517:
/* r2r3dist */
CONSTANTS[1] = 1.49318;
/* r2r3dist */
CONSTANTS[2] = 10.6488;
break;
case 518:
/* r2r3dist */
CONSTANTS[1] = 1.32177;
/* r2r3dist */
CONSTANTS[2] = 10.3616;
break;
case 519:
/* r2r3dist */
CONSTANTS[1] = -0.0423543;
/* r2r3dist */
CONSTANTS[2] = 9.80688;
break;
case 520:
/* r2r3dist */
CONSTANTS[1] = -1.24957;
/* r2r3dist */
CONSTANTS[2] = 9.20051;
break;
case 521:
/* r2r3dist */
CONSTANTS[1] = 2.14020;
/* r2r3dist */
CONSTANTS[2] = 11.0278;
break;
case 522:
/* r2r3dist */
CONSTANTS[1] = 1.02965;
/* r2r3dist */
CONSTANTS[2] = 10.1165;
break;
case 523:
/* r2r3dist */
CONSTANTS[1] = -0.210068;
/* r2r3dist */
CONSTANTS[2] = 9.39555;
break;
case 524:
/* r2r3dist */
CONSTANTS[1] = -0.423401;
/* r2r3dist */
CONSTANTS[2] = 9.97163;
break;
case 525:
/* r2r3dist */
CONSTANTS[1] = 1.33140;
/* r2r3dist */
CONSTANTS[2] = 10.3506;
break;
case 526:
/* r2r3dist */
CONSTANTS[1] = -1.99508;
/* r2r3dist */
CONSTANTS[2] = 9.16234;
break;
case 527:
/* r2r3dist */
CONSTANTS[1] = -0.969015;
/* r2r3dist */
CONSTANTS[2] = 9.15691;
break;
case 528:
/* r2r3dist */
CONSTANTS[1] = -1.46154;
/* r2r3dist */
CONSTANTS[2] = 9.19254;
break;
case 529:
/* r2r3dist */
CONSTANTS[1] = 1.49764;
/* r2r3dist */
CONSTANTS[2] = 9.97038;
break;
case 530:
/* r2r3dist */
CONSTANTS[1] = 0.113081;
/* r2r3dist */
CONSTANTS[2] = 9.83325;
break;
case 531:
/* r2r3dist */
CONSTANTS[1] = -1.42146;
/* r2r3dist */
CONSTANTS[2] = 9.06825;
break;
case 532:
/* r2r3dist */
CONSTANTS[1] = -1.58281;
/* r2r3dist */
CONSTANTS[2] = 9.49332;
break;
case 533:
/* r2r3dist */
CONSTANTS[1] = 1.26386;
/* r2r3dist */
CONSTANTS[2] = 10.3359;
break;
case 534:
/* r2r3dist */
CONSTANTS[1] = 1.34212;
/* r2r3dist */
CONSTANTS[2] = 10.3533;
break;
case 535:
/* r2r3dist */
CONSTANTS[1] = -1.32330;
/* r2r3dist */
CONSTANTS[2] = 8.96032;
break;
case 536:
/* r2r3dist */
CONSTANTS[1] = 0.329914;
/* r2r3dist */
CONSTANTS[2] = 9.78799;
break;
case 537:
/* r2r3dist */
CONSTANTS[1] = -1.21327;
/* r2r3dist */
CONSTANTS[2] = 9.55096;
break;
case 538:
/* r2r3dist */
CONSTANTS[1] = -0.0303879;
/* r2r3dist */
CONSTANTS[2] = 9.75971;
break;
case 539:
/* r2r3dist */
CONSTANTS[1] = 1.06210;
/* r2r3dist */
CONSTANTS[2] = 10.3017;
break;
case 540:
/* r2r3dist */
CONSTANTS[1] = -0.808944;
/* r2r3dist */
CONSTANTS[2] = 8.86237;
break;
case 541:
/* r2r3dist */
CONSTANTS[1] = 0.352956;
/* r2r3dist */
CONSTANTS[2] = 10.3568;
break;
case 542:
/* r2r3dist */
CONSTANTS[1] = -0.870592;
/* r2r3dist */
CONSTANTS[2] = 10.4506;
break;
case 543:
/* r2r3dist */
CONSTANTS[1] = 1.05224;
/* r2r3dist */
CONSTANTS[2] = 10.4802;
break;
case 544:
/* r2r3dist */
CONSTANTS[1] = -0.0797880;
/* r2r3dist */
CONSTANTS[2] = 10.7142;
break;
case 545:
/* r2r3dist */
CONSTANTS[1] = -0.946376;
/* r2r3dist */
CONSTANTS[2] = 10.4784;
break;
case 546:
/* r2r3dist */
CONSTANTS[1] = 1.99960;
/* r2r3dist */
CONSTANTS[2] = 10.1019;
break;
case 547:
/* r2r3dist */
CONSTANTS[1] = -0.196236;
/* r2r3dist */
CONSTANTS[2] = 10.5751;
break;
case 548:
/* r2r3dist */
CONSTANTS[1] = 1.28829;
/* r2r3dist */
CONSTANTS[2] = 10.9514;
break;
case 549:
/* r2r3dist */
CONSTANTS[1] = 0.0823696;
/* r2r3dist */
CONSTANTS[2] = 10.3527;
break;
case 550:
/* r2r3dist */
CONSTANTS[1] = -0.268091;
/* r2r3dist */
CONSTANTS[2] = 9.70354;
break;
case 551:
/* r2r3dist */
CONSTANTS[1] = 0.901731;
/* r2r3dist */
CONSTANTS[2] = 10.7756;
break;
case 552:
/* r2r3dist */
CONSTANTS[1] = 1.28032;
/* r2r3dist */
CONSTANTS[2] = 11.5576;
break;
case 553:
/* r2r3dist */
CONSTANTS[1] = -1.68936;
/* r2r3dist */
CONSTANTS[2] = 9.48078;
break;
case 554:
/* r2r3dist */
CONSTANTS[1] = -0.571127;
/* r2r3dist */
CONSTANTS[2] = 9.71225;
break;
case 555:
/* r2r3dist */
CONSTANTS[1] = 1.71868;
/* r2r3dist */
CONSTANTS[2] = 10.7382;
break;
case 556:
/* r2r3dist */
CONSTANTS[1] = 0.175661;
/* r2r3dist */
CONSTANTS[2] = 9.76734;
break;
case 557:
/* r2r3dist */
CONSTANTS[1] = -0.185540;
/* r2r3dist */
CONSTANTS[2] = 9.12749;
break;
case 558:
/* r2r3dist */
CONSTANTS[1] = -0.346409;
/* r2r3dist */
CONSTANTS[2] = 9.29452;
break;
case 559:
/* r2r3dist */
CONSTANTS[1] = -0.342999;
/* r2r3dist */
CONSTANTS[2] = 9.97823;
break;
case 560:
/* r2r3dist */
CONSTANTS[1] = 0.737461;
/* r2r3dist */
CONSTANTS[2] = 11.3272;
break;
case 561:
/* r2r3dist */
CONSTANTS[1] = -0.504493;
/* r2r3dist */
CONSTANTS[2] = 10.7028;
break;
case 562:
/* r2r3dist */
CONSTANTS[1] = 0.523573;
/* r2r3dist */
CONSTANTS[2] = 10.6462;
break;
case 563:
/* r2r3dist */
CONSTANTS[1] = -0.309610;
/* r2r3dist */
CONSTANTS[2] = 9.99799;
break;
case 564:
/* r2r3dist */
CONSTANTS[1] = -0.881047;
/* r2r3dist */
CONSTANTS[2] = 9.55033;
break;
case 565:
/* r2r3dist */
CONSTANTS[1] = -0.494034;
/* r2r3dist */
CONSTANTS[2] = 8.70862;
break;
case 566:
/* r2r3dist */
CONSTANTS[1] = -0.582777;
/* r2r3dist */
CONSTANTS[2] = 9.61434;
break;
case 567:
/* r2r3dist */
CONSTANTS[1] = 0.544494;
/* r2r3dist */
CONSTANTS[2] = 10.3399;
break;
case 568:
/* r2r3dist */
CONSTANTS[1] = -1.98849;
/* r2r3dist */
CONSTANTS[2] = 8.82237;
break;
case 569:
/* r2r3dist */
CONSTANTS[1] = 0.555495;
/* r2r3dist */
CONSTANTS[2] = 10.7356;
break;
case 570:
/* r2r3dist */
CONSTANTS[1] = 0.802086;
/* r2r3dist */
CONSTANTS[2] = 10.3418;
break;
case 571:
/* r2r3dist */
CONSTANTS[1] = 0.372985;
/* r2r3dist */
CONSTANTS[2] = 10.4661;
break;
case 572:
/* r2r3dist */
CONSTANTS[1] = -1.85823;
/* r2r3dist */
CONSTANTS[2] = 9.69814;
break;
case 573:
/* r2r3dist */
CONSTANTS[1] = 0.879953;
/* r2r3dist */
CONSTANTS[2] = 10.2006;
break;
case 574:
/* r2r3dist */
CONSTANTS[1] = 1.53822;
/* r2r3dist */
CONSTANTS[2] = 9.71154;
break;
case 575:
/* r2r3dist */
CONSTANTS[1] = -2.86551;
/* r2r3dist */
CONSTANTS[2] = 8.61682;
break;
case 576:
/* r2r3dist */
CONSTANTS[1] = 1.46682;
/* r2r3dist */
CONSTANTS[2] = 10.3537;
break;
case 577:
/* r2r3dist */
CONSTANTS[1] = -1.24871;
/* r2r3dist */
CONSTANTS[2] = 9.46812;
break;
case 578:
/* r2r3dist */
CONSTANTS[1] = -0.747512;
/* r2r3dist */
CONSTANTS[2] = 9.52576;
break;
case 579:
/* r2r3dist */
CONSTANTS[1] = 1.22458;
/* r2r3dist */
CONSTANTS[2] = 10.2689;
break;
case 580:
/* r2r3dist */
CONSTANTS[1] = 0.562402;
/* r2r3dist */
CONSTANTS[2] = 10.5759;
break;
case 581:
/* r2r3dist */
CONSTANTS[1] = 0.183738;
/* r2r3dist */
CONSTANTS[2] = 10.4753;
break;
case 582:
/* r2r3dist */
CONSTANTS[1] = 0.670118;
/* r2r3dist */
CONSTANTS[2] = 9.63987;
break;
case 583:
/* r2r3dist */
CONSTANTS[1] = 1.67535;
/* r2r3dist */
CONSTANTS[2] = 11.1555;
break;
case 584:
/* r2r3dist */
CONSTANTS[1] = 1.33373;
/* r2r3dist */
CONSTANTS[2] = 10.6024;
break;
case 585:
/* r2r3dist */
CONSTANTS[1] = -0.774551;
/* r2r3dist */
CONSTANTS[2] = 9.65820;
break;
case 586:
/* r2r3dist */
CONSTANTS[1] = 1.25987;
/* r2r3dist */
CONSTANTS[2] = 10.6762;
break;
case 587:
/* r2r3dist */
CONSTANTS[1] = 1.85013;
/* r2r3dist */
CONSTANTS[2] = 10.5999;
break;
case 588:
/* r2r3dist */
CONSTANTS[1] = 0.427977;
/* r2r3dist */
CONSTANTS[2] = 10.0741;
break;
case 589:
/* r2r3dist */
CONSTANTS[1] = -0.542151;
/* r2r3dist */
CONSTANTS[2] = 10.4372;
break;
case 590:
/* r2r3dist */
CONSTANTS[1] = -0.274625;
/* r2r3dist */
CONSTANTS[2] = 9.52362;
break;
case 591:
/* r2r3dist */
CONSTANTS[1] = -0.317415;
/* r2r3dist */
CONSTANTS[2] = 9.83726;
break;
case 592:
/* r2r3dist */
CONSTANTS[1] = 0.674831;
/* r2r3dist */
CONSTANTS[2] = 10.7653;
break;
case 593:
/* r2r3dist */
CONSTANTS[1] = 0.677258;
/* r2r3dist */
CONSTANTS[2] = 9.89435;
break;
case 594:
/* r2r3dist */
CONSTANTS[1] = 0.716131;
/* r2r3dist */
CONSTANTS[2] = 10.0076;
break;
case 595:
/* r2r3dist */
CONSTANTS[1] = -0.858801;
/* r2r3dist */
CONSTANTS[2] = 8.92050;
break;
case 596:
/* r2r3dist */
CONSTANTS[1] = -0.101121;
/* r2r3dist */
CONSTANTS[2] = 9.49111;
break;
case 597:
/* r2r3dist */
CONSTANTS[1] = -0.239913;
/* r2r3dist */
CONSTANTS[2] = 10.3514;
break;
case 598:
/* r2r3dist */
CONSTANTS[1] = -0.477360;
/* r2r3dist */
CONSTANTS[2] = 10.6174;
break;
case 599:
/* r2r3dist */
CONSTANTS[1] = -0.738714;
/* r2r3dist */
CONSTANTS[2] = 9.22485;
break;
case 600:
/* r2r3dist */
CONSTANTS[1] = 0.00411387;
/* r2r3dist */
CONSTANTS[2] = 10.5929;
break;
case 601:
/* r2r3dist */
CONSTANTS[1] = -1.69666;
/* r2r3dist */
CONSTANTS[2] = 9.39686;
break;
case 602:
/* r2r3dist */
CONSTANTS[1] = -0.620922;
/* r2r3dist */
CONSTANTS[2] = 10.1151;
break;
case 603:
/* r2r3dist */
CONSTANTS[1] = -1.05565;
/* r2r3dist */
CONSTANTS[2] = 9.45448;
break;
case 604:
/* r2r3dist */
CONSTANTS[1] = 1.57039;
/* r2r3dist */
CONSTANTS[2] = 9.81664;
break;
case 605:
/* r2r3dist */
CONSTANTS[1] = -0.104777;
/* r2r3dist */
CONSTANTS[2] = 9.69525;
break;
case 606:
/* r2r3dist */
CONSTANTS[1] = -0.272459;
/* r2r3dist */
CONSTANTS[2] = 9.44754;
break;
case 607:
/* r2r3dist */
CONSTANTS[1] = -0.0577842;
/* r2r3dist */
CONSTANTS[2] = 10.2906;
break;
case 608:
/* r2r3dist */
CONSTANTS[1] = 0.505851;
/* r2r3dist */
CONSTANTS[2] = 10.3218;
break;
case 609:
/* r2r3dist */
CONSTANTS[1] = -0.399555;
/* r2r3dist */
CONSTANTS[2] = 9.81272;
break;
case 610:
/* r2r3dist */
CONSTANTS[1] = 0.924814;
/* r2r3dist */
CONSTANTS[2] = 11.2289;
break;
case 611:
/* r2r3dist */
CONSTANTS[1] = 0.966235;
/* r2r3dist */
CONSTANTS[2] = 10.4605;
break;
case 612:
/* r2r3dist */
CONSTANTS[1] = -0.806886;
/* r2r3dist */
CONSTANTS[2] = 9.05107;
break;
case 613:
/* r2r3dist */
CONSTANTS[1] = -0.497842;
/* r2r3dist */
CONSTANTS[2] = 9.27487;
break;
case 614:
/* r2r3dist */
CONSTANTS[1] = -0.413479;
/* r2r3dist */
CONSTANTS[2] = 10.0058;
break;
case 615:
/* r2r3dist */
CONSTANTS[1] = -0.985805;
/* r2r3dist */
CONSTANTS[2] = 8.72723;
break;
case 616:
/* r2r3dist */
CONSTANTS[1] = -1.59998;
/* r2r3dist */
CONSTANTS[2] = 9.39491;
break;
case 617:
/* r2r3dist */
CONSTANTS[1] = -0.404125;
/* r2r3dist */
CONSTANTS[2] = 9.50980;
break;
case 618:
/* r2r3dist */
CONSTANTS[1] = 0.445495;
/* r2r3dist */
CONSTANTS[2] = 10.2844;
break;
case 619:
/* r2r3dist */
CONSTANTS[1] = -0.909585;
/* r2r3dist */
CONSTANTS[2] = 9.22876;
break;
case 620:
/* r2r3dist */
CONSTANTS[1] = 0.0976493;
/* r2r3dist */
CONSTANTS[2] = 10.0220;
break;
case 621:
/* r2r3dist */
CONSTANTS[1] = -1.25973;
/* r2r3dist */
CONSTANTS[2] = 9.36721;
break;
case 622:
/* r2r3dist */
CONSTANTS[1] = 1.61795;
/* r2r3dist */
CONSTANTS[2] = 10.4501;
break;
case 623:
/* r2r3dist */
CONSTANTS[1] = 1.23804;
/* r2r3dist */
CONSTANTS[2] = 10.5586;
break;
case 624:
/* r2r3dist */
CONSTANTS[1] = -0.718394;
/* r2r3dist */
CONSTANTS[2] = 9.78457;
break;
case 625:
/* r2r3dist */
CONSTANTS[1] = -1.38174;
/* r2r3dist */
CONSTANTS[2] = 8.61673;
break;
case 626:
/* r2r3dist */
CONSTANTS[1] = 0.146050;
/* r2r3dist */
CONSTANTS[2] = 10.5760;
break;
case 627:
/* r2r3dist */
CONSTANTS[1] = -0.301388;
/* r2r3dist */
CONSTANTS[2] = 10.2475;
break;
case 628:
/* r2r3dist */
CONSTANTS[1] = 0.329288;
/* r2r3dist */
CONSTANTS[2] = 10.6663;
break;
case 629:
/* r2r3dist */
CONSTANTS[1] = -0.367443;
/* r2r3dist */
CONSTANTS[2] = 9.89087;
break;
case 630:
/* r2r3dist */
CONSTANTS[1] = -2.12423;
/* r2r3dist */
CONSTANTS[2] = 8.93525;
break;
case 631:
/* r2r3dist */
CONSTANTS[1] = 0.0621354;
/* r2r3dist */
CONSTANTS[2] = 9.77677;
break;
case 632:
/* r2r3dist */
CONSTANTS[1] = 0.157480;
/* r2r3dist */
CONSTANTS[2] = 10.7725;
break;
case 633:
/* r2r3dist */
CONSTANTS[1] = -0.0994025;
/* r2r3dist */
CONSTANTS[2] = 9.06774;
break;
case 634:
/* r2r3dist */
CONSTANTS[1] = -1.05937;
/* r2r3dist */
CONSTANTS[2] = 8.97319;
break;
case 635:
/* r2r3dist */
CONSTANTS[1] = 0.00701648;
/* r2r3dist */
CONSTANTS[2] = 9.69851;
break;
case 636:
/* r2r3dist */
CONSTANTS[1] = 0.665293;
/* r2r3dist */
CONSTANTS[2] = 9.72380;
break;
case 637:
/* r2r3dist */
CONSTANTS[1] = -1.98755;
/* r2r3dist */
CONSTANTS[2] = 9.94040;
break;
case 638:
/* r2r3dist */
CONSTANTS[1] = 0.0113768;
/* r2r3dist */
CONSTANTS[2] = 10.8783;
break;
case 639:
/* r2r3dist */
CONSTANTS[1] = 0.0453297;
/* r2r3dist */
CONSTANTS[2] = 10.7279;
break;
case 640:
/* r2r3dist */
CONSTANTS[1] = -1.26847;
/* r2r3dist */
CONSTANTS[2] = 10.2385;
break;
case 641:
/* r2r3dist */
CONSTANTS[1] = -0.135900;
/* r2r3dist */
CONSTANTS[2] = 10.2942;
break;
case 642:
/* r2r3dist */
CONSTANTS[1] = -0.320005;
/* r2r3dist */
CONSTANTS[2] = 9.86866;
break;
case 643:
/* r2r3dist */
CONSTANTS[1] = -1.17535;
/* r2r3dist */
CONSTANTS[2] = 9.50894;
break;
case 644:
/* r2r3dist */
CONSTANTS[1] = 0.0596427;
/* r2r3dist */
CONSTANTS[2] = 9.41408;
break;
case 645:
/* r2r3dist */
CONSTANTS[1] = -0.575592;
/* r2r3dist */
CONSTANTS[2] = 9.03367;
break;
case 646:
/* r2r3dist */
CONSTANTS[1] = -1.54249;
/* r2r3dist */
CONSTANTS[2] = 9.59057;
break;
case 647:
/* r2r3dist */
CONSTANTS[1] = -0.467236;
/* r2r3dist */
CONSTANTS[2] = 10.4076;
break;
case 648:
/* r2r3dist */
CONSTANTS[1] = -0.476990;
/* r2r3dist */
CONSTANTS[2] = 9.00160;
break;
case 649:
/* r2r3dist */
CONSTANTS[1] = -1.37371;
/* r2r3dist */
CONSTANTS[2] = 10.3046;
break;
case 650:
/* r2r3dist */
CONSTANTS[1] = -0.154625;
/* r2r3dist */
CONSTANTS[2] = 10.9566;
break;
case 651:
/* r2r3dist */
CONSTANTS[1] = 1.44285;
/* r2r3dist */
CONSTANTS[2] = 10.7695;
break;
case 652:
/* r2r3dist */
CONSTANTS[1] = -0.332722;
/* r2r3dist */
CONSTANTS[2] = 9.07131;
break;
case 653:
/* r2r3dist */
CONSTANTS[1] = 0.709138;
/* r2r3dist */
CONSTANTS[2] = 10.7518;
break;
case 654:
/* r2r3dist */
CONSTANTS[1] = 0.746652;
/* r2r3dist */
CONSTANTS[2] = 9.86163;
break;
case 655:
/* r2r3dist */
CONSTANTS[1] = 0.680933;
/* r2r3dist */
CONSTANTS[2] = 9.90815;
break;
case 656:
/* r2r3dist */
CONSTANTS[1] = 0.117772;
/* r2r3dist */
CONSTANTS[2] = 8.75121;
break;
case 657:
/* r2r3dist */
CONSTANTS[1] = 1.32237;
/* r2r3dist */
CONSTANTS[2] = 10.8637;
break;
case 658:
/* r2r3dist */
CONSTANTS[1] = -0.738373;
/* r2r3dist */
CONSTANTS[2] = 9.45378;
break;
case 659:
/* r2r3dist */
CONSTANTS[1] = 0.151820;
/* r2r3dist */
CONSTANTS[2] = 10.1513;
break;
case 660:
/* r2r3dist */
CONSTANTS[1] = -0.319313;
/* r2r3dist */
CONSTANTS[2] = 9.49655;
break;
case 661:
/* r2r3dist */
CONSTANTS[1] = 0.179654;
/* r2r3dist */
CONSTANTS[2] = 9.19866;
break;
case 662:
/* r2r3dist */
CONSTANTS[1] = 0.174360;
/* r2r3dist */
CONSTANTS[2] = 9.07776;
break;
case 663:
/* r2r3dist */
CONSTANTS[1] = 1.07812;
/* r2r3dist */
CONSTANTS[2] = 10.1251;
break;
case 664:
/* r2r3dist */
CONSTANTS[1] = 0.562038;
/* r2r3dist */
CONSTANTS[2] = 11.1739;
break;
case 665:
/* r2r3dist */
CONSTANTS[1] = 0.869652;
/* r2r3dist */
CONSTANTS[2] = 9.92537;
break;
case 666:
/* r2r3dist */
CONSTANTS[1] = 0.465189;
/* r2r3dist */
CONSTANTS[2] = 11.1155;
break;
case 667:
/* r2r3dist */
CONSTANTS[1] = -1.51759;
/* r2r3dist */
CONSTANTS[2] = 9.63016;
break;
case 668:
/* r2r3dist */
CONSTANTS[1] = -0.966909;
/* r2r3dist */
CONSTANTS[2] = 8.58672;
break;
case 669:
/* r2r3dist */
CONSTANTS[1] = -0.698523;
/* r2r3dist */
CONSTANTS[2] = 9.73816;
break;
case 670:
/* r2r3dist */
CONSTANTS[1] = -1.10229;
/* r2r3dist */
CONSTANTS[2] = 9.56694;
break;
case 671:
/* r2r3dist */
CONSTANTS[1] = 0.433178;
/* r2r3dist */
CONSTANTS[2] = 10.0252;
break;
case 672:
/* r2r3dist */
CONSTANTS[1] = 1.11356;
/* r2r3dist */
CONSTANTS[2] = 10.7501;
break;
case 673:
/* r2r3dist */
CONSTANTS[1] = -0.165635;
/* r2r3dist */
CONSTANTS[2] = 9.91605;
break;
case 674:
/* r2r3dist */
CONSTANTS[1] = 0.784282;
/* r2r3dist */
CONSTANTS[2] = 10.7656;
break;
case 675:
/* r2r3dist */
CONSTANTS[1] = -1.45336;
/* r2r3dist */
CONSTANTS[2] = 8.99533;
break;
case 676:
/* r2r3dist */
CONSTANTS[1] = 1.56549;
/* r2r3dist */
CONSTANTS[2] = 12.2716;
break;
case 677:
/* r2r3dist */
CONSTANTS[1] = -0.0288238;
/* r2r3dist */
CONSTANTS[2] = 10.0791;
break;
case 678:
/* r2r3dist */
CONSTANTS[1] = -0.594152;
/* r2r3dist */
CONSTANTS[2] = 9.34606;
break;
case 679:
/* r2r3dist */
CONSTANTS[1] = 0.849283;
/* r2r3dist */
CONSTANTS[2] = 10.8055;
break;
case 680:
/* r2r3dist */
CONSTANTS[1] = 0.170170;
/* r2r3dist */
CONSTANTS[2] = 10.0751;
break;
case 681:
/* r2r3dist */
CONSTANTS[1] = -0.715273;
/* r2r3dist */
CONSTANTS[2] = 10.7120;
break;
case 682:
/* r2r3dist */
CONSTANTS[1] = -0.753155;
/* r2r3dist */
CONSTANTS[2] = 10.1609;
break;
case 683:
/* r2r3dist */
CONSTANTS[1] = -0.381936;
/* r2r3dist */
CONSTANTS[2] = 10.1288;
break;
case 684:
/* r2r3dist */
CONSTANTS[1] = 0.117365;
/* r2r3dist */
CONSTANTS[2] = 10.7385;
break;
case 685:
/* r2r3dist */
CONSTANTS[1] = -0.0499983;
/* r2r3dist */
CONSTANTS[2] = 9.90393;
break;
case 686:
/* r2r3dist */
CONSTANTS[1] = 1.46231;
/* r2r3dist */
CONSTANTS[2] = 10.4129;
break;
case 687:
/* r2r3dist */
CONSTANTS[1] = 0.693661;
/* r2r3dist */
CONSTANTS[2] = 10.6085;
break;
case 688:
/* r2r3dist */
CONSTANTS[1] = -0.810199;
/* r2r3dist */
CONSTANTS[2] = 8.30520;
break;
case 689:
/* r2r3dist */
CONSTANTS[1] = 0.180981;
/* r2r3dist */
CONSTANTS[2] = 9.60121;
break;
case 690:
/* r2r3dist */
CONSTANTS[1] = -1.07555;
/* r2r3dist */
CONSTANTS[2] = 9.29245;
break;
case 691:
/* r2r3dist */
CONSTANTS[1] = 0.250373;
/* r2r3dist */
CONSTANTS[2] = 9.63609;
break;
case 692:
/* r2r3dist */
CONSTANTS[1] = -1.37192;
/* r2r3dist */
CONSTANTS[2] = 9.82880;
break;
case 693:
/* r2r3dist */
CONSTANTS[1] = -1.17980;
/* r2r3dist */
CONSTANTS[2] = 9.80105;
break;
case 694:
/* r2r3dist */
CONSTANTS[1] = -0.739278;
/* r2r3dist */
CONSTANTS[2] = 9.92105;
break;
case 695:
/* r2r3dist */
CONSTANTS[1] = 1.30972;
/* r2r3dist */
CONSTANTS[2] = 11.2621;
break;
case 696:
/* r2r3dist */
CONSTANTS[1] = -0.401853;
/* r2r3dist */
CONSTANTS[2] = 9.18155;
break;
case 697:
/* r2r3dist */
CONSTANTS[1] = -0.584644;
/* r2r3dist */
CONSTANTS[2] = 9.84614;
break;
case 698:
/* r2r3dist */
CONSTANTS[1] = 0.269095;
/* r2r3dist */
CONSTANTS[2] = 10.8294;
break;
case 699:
/* r2r3dist */
CONSTANTS[1] = 0.295820;
/* r2r3dist */
CONSTANTS[2] = 10.5321;
break;
case 700:
/* r2r3dist */
CONSTANTS[1] = 0.0459941;
/* r2r3dist */
CONSTANTS[2] = 10.6502;
break;
case 701:
/* r2r3dist */
CONSTANTS[1] = 0.0465433;
/* r2r3dist */
CONSTANTS[2] = 9.22829;
break;
case 702:
/* r2r3dist */
CONSTANTS[1] = 1.95760;
/* r2r3dist */
CONSTANTS[2] = 11.0755;
break;
case 703:
/* r2r3dist */
CONSTANTS[1] = 0.0925624;
/* r2r3dist */
CONSTANTS[2] = 9.75964;
break;
case 704:
/* r2r3dist */
CONSTANTS[1] = 2.54523;
/* r2r3dist */
CONSTANTS[2] = 10.7614;
break;
case 705:
/* r2r3dist */
CONSTANTS[1] = -0.713451;
/* r2r3dist */
CONSTANTS[2] = 10.1387;
break;
case 706:
/* r2r3dist */
CONSTANTS[1] = -0.874198;
/* r2r3dist */
CONSTANTS[2] = 10.3242;
break;
case 707:
/* r2r3dist */
CONSTANTS[1] = 1.70470;
/* r2r3dist */
CONSTANTS[2] = 11.1288;
break;
case 708:
/* r2r3dist */
CONSTANTS[1] = 0.0643534;
/* r2r3dist */
CONSTANTS[2] = 10.1860;
break;
case 709:
/* r2r3dist */
CONSTANTS[1] = -1.43201;
/* r2r3dist */
CONSTANTS[2] = 8.80782;
break;
case 710:
/* r2r3dist */
CONSTANTS[1] = 0.519043;
/* r2r3dist */
CONSTANTS[2] = 11.3021;
break;
case 711:
/* r2r3dist */
CONSTANTS[1] = 0.991137;
/* r2r3dist */
CONSTANTS[2] = 11.5725;
break;
case 712:
/* r2r3dist */
CONSTANTS[1] = -0.756182;
/* r2r3dist */
CONSTANTS[2] = 9.51829;
break;
case 713:
/* r2r3dist */
CONSTANTS[1] = 0.125791;
/* r2r3dist */
CONSTANTS[2] = 10.5654;
break;
case 714:
/* r2r3dist */
CONSTANTS[1] = 0.797916;
/* r2r3dist */
CONSTANTS[2] = 10.1496;
break;
case 715:
/* r2r3dist */
CONSTANTS[1] = 0.213357;
/* r2r3dist */
CONSTANTS[2] = 9.84970;
break;
case 716:
/* r2r3dist */
CONSTANTS[1] = -1.11240;
/* r2r3dist */
CONSTANTS[2] = 9.70595;
break;
case 717:
/* r2r3dist */
CONSTANTS[1] = -0.938336;
/* r2r3dist */
CONSTANTS[2] = 9.83747;
break;
case 718:
/* r2r3dist */
CONSTANTS[1] = 0.423435;
/* r2r3dist */
CONSTANTS[2] = 10.1917;
break;
case 719:
/* r2r3dist */
CONSTANTS[1] = -0.221586;
/* r2r3dist */
CONSTANTS[2] = 9.46389;
break;
case 720:
/* r2r3dist */
CONSTANTS[1] = -0.243507;
/* r2r3dist */
CONSTANTS[2] = 10.5818;
break;
case 721:
/* r2r3dist */
CONSTANTS[1] = 1.72484;
/* r2r3dist */
CONSTANTS[2] = 10.5350;
break;
case 722:
/* r2r3dist */
CONSTANTS[1] = -1.92122;
/* r2r3dist */
CONSTANTS[2] = 9.33013;
break;
case 723:
/* r2r3dist */
CONSTANTS[1] = -1.35452;
/* r2r3dist */
CONSTANTS[2] = 9.53380;
break;
case 724:
/* r2r3dist */
CONSTANTS[1] = -1.58391;
/* r2r3dist */
CONSTANTS[2] = 9.33147;
break;
case 725:
/* r2r3dist */
CONSTANTS[1] = -0.480346;
/* r2r3dist */
CONSTANTS[2] = 9.40854;
break;
case 726:
/* r2r3dist */
CONSTANTS[1] = -0.432034;
/* r2r3dist */
CONSTANTS[2] = 9.75558;
break;
case 727:
/* r2r3dist */
CONSTANTS[1] = 0.00440832;
/* r2r3dist */
CONSTANTS[2] = 10.0524;
break;
case 728:
/* r2r3dist */
CONSTANTS[1] = -0.650565;
/* r2r3dist */
CONSTANTS[2] = 9.96439;
break;
case 729:
/* r2r3dist */
CONSTANTS[1] = -0.906049;
/* r2r3dist */
CONSTANTS[2] = 9.88386;
break;
case 730:
/* r2r3dist */
CONSTANTS[1] = 1.09229;
/* r2r3dist */
CONSTANTS[2] = 10.5758;
break;
case 731:
/* r2r3dist */
CONSTANTS[1] = -0.0536554;
/* r2r3dist */
CONSTANTS[2] = 9.82221;
break;
case 732:
/* r2r3dist */
CONSTANTS[1] = -0.742173;
/* r2r3dist */
CONSTANTS[2] = 8.87081;
break;
case 733:
/* r2r3dist */
CONSTANTS[1] = 0.534684;
/* r2r3dist */
CONSTANTS[2] = 10.0586;
break;
case 734:
/* r2r3dist */
CONSTANTS[1] = 1.71160;
/* r2r3dist */
CONSTANTS[2] = 11.7864;
break;
case 735:
/* r2r3dist */
CONSTANTS[1] = -1.00782;
/* r2r3dist */
CONSTANTS[2] = 9.44479;
break;
case 736:
/* r2r3dist */
CONSTANTS[1] = 1.77866;
/* r2r3dist */
CONSTANTS[2] = 10.8613;
break;
case 737:
/* r2r3dist */
CONSTANTS[1] = 0.0668529;
/* r2r3dist */
CONSTANTS[2] = 9.31710;
break;
case 738:
/* r2r3dist */
CONSTANTS[1] = -0.924675;
/* r2r3dist */
CONSTANTS[2] = 10.0205;
break;
case 739:
/* r2r3dist */
CONSTANTS[1] = -0.743831;
/* r2r3dist */
CONSTANTS[2] = 9.95471;
break;
case 740:
/* r2r3dist */
CONSTANTS[1] = 0.270911;
/* r2r3dist */
CONSTANTS[2] = 11.0003;
break;
case 741:
/* r2r3dist */
CONSTANTS[1] = 0.347752;
/* r2r3dist */
CONSTANTS[2] = 10.1545;
break;
case 742:
/* r2r3dist */
CONSTANTS[1] = -0.173728;
/* r2r3dist */
CONSTANTS[2] = 9.53353;
break;
case 743:
/* r2r3dist */
CONSTANTS[1] = -1.83276;
/* r2r3dist */
CONSTANTS[2] = 8.79439;
break;
case 744:
/* r2r3dist */
CONSTANTS[1] = -0.302903;
/* r2r3dist */
CONSTANTS[2] = 10.7371;
break;
case 745:
/* r2r3dist */
CONSTANTS[1] = 0.668477;
/* r2r3dist */
CONSTANTS[2] = 10.1338;
break;
case 746:
/* r2r3dist */
CONSTANTS[1] = -0.0690888;
/* r2r3dist */
CONSTANTS[2] = 9.61609;
break;
case 747:
/* r2r3dist */
CONSTANTS[1] = -1.06532;
/* r2r3dist */
CONSTANTS[2] = 10.0634;
break;
case 748:
/* r2r3dist */
CONSTANTS[1] = 1.72766;
/* r2r3dist */
CONSTANTS[2] = 11.1485;
break;
case 749:
/* r2r3dist */
CONSTANTS[1] = -0.360946;
/* r2r3dist */
CONSTANTS[2] = 9.73864;
break;
case 750:
/* r2r3dist */
CONSTANTS[1] = -1.12066;
/* r2r3dist */
CONSTANTS[2] = 9.11297;
break;
case 751:
/* r2r3dist */
CONSTANTS[1] = 0.447154;
/* r2r3dist */
CONSTANTS[2] = 9.57730;
break;
case 752:
/* r2r3dist */
CONSTANTS[1] = -0.0156557;
/* r2r3dist */
CONSTANTS[2] = 10.0695;
break;
case 753:
/* r2r3dist */
CONSTANTS[1] = -0.539593;
/* r2r3dist */
CONSTANTS[2] = 9.96815;
break;
case 754:
/* r2r3dist */
CONSTANTS[1] = 2.29818;
/* r2r3dist */
CONSTANTS[2] = 11.1277;
break;
case 755:
/* r2r3dist */
CONSTANTS[1] = 0.507553;
/* r2r3dist */
CONSTANTS[2] = 9.92079;
break;
case 756:
/* r2r3dist */
CONSTANTS[1] = -1.17862;
/* r2r3dist */
CONSTANTS[2] = 8.15489;
break;
case 757:
/* r2r3dist */
CONSTANTS[1] = 1.90703;
/* r2r3dist */
CONSTANTS[2] = 10.8078;
break;
case 758:
/* r2r3dist */
CONSTANTS[1] = -0.902915;
/* r2r3dist */
CONSTANTS[2] = 9.31834;
break;
case 759:
/* r2r3dist */
CONSTANTS[1] = -0.167218;
/* r2r3dist */
CONSTANTS[2] = 9.96754;
break;
case 760:
/* r2r3dist */
CONSTANTS[1] = 0.556958;
/* r2r3dist */
CONSTANTS[2] = 10.3550;
break;
case 761:
/* r2r3dist */
CONSTANTS[1] = -0.261562;
/* r2r3dist */
CONSTANTS[2] = 10.5152;
break;
case 762:
/* r2r3dist */
CONSTANTS[1] = -2.76881;
/* r2r3dist */
CONSTANTS[2] = 9.04895;
break;
case 763:
/* r2r3dist */
CONSTANTS[1] = -0.441135;
/* r2r3dist */
CONSTANTS[2] = 9.19270;
break;
case 764:
/* r2r3dist */
CONSTANTS[1] = 0.691029;
/* r2r3dist */
CONSTANTS[2] = 11.1500;
break;
case 765:
/* r2r3dist */
CONSTANTS[1] = -0.564776;
/* r2r3dist */
CONSTANTS[2] = 9.06734;
break;
case 766:
/* r2r3dist */
CONSTANTS[1] = 0.0606141;
/* r2r3dist */
CONSTANTS[2] = 10.8898;
break;
case 767:
/* r2r3dist */
CONSTANTS[1] = 1.18677;
/* r2r3dist */
CONSTANTS[2] = 10.4244;
break;
case 768:
/* r2r3dist */
CONSTANTS[1] = 1.00683;
/* r2r3dist */
CONSTANTS[2] = 10.3092;
break;
case 769:
/* r2r3dist */
CONSTANTS[1] = -0.199073;
/* r2r3dist */
CONSTANTS[2] = 9.51280;
break;
case 770:
/* r2r3dist */
CONSTANTS[1] = -0.0875493;
/* r2r3dist */
CONSTANTS[2] = 9.81344;
break;
case 771:
/* r2r3dist */
CONSTANTS[1] = -0.222262;
/* r2r3dist */
CONSTANTS[2] = 10.0699;
break;
case 772:
/* r2r3dist */
CONSTANTS[1] = -0.0650175;
/* r2r3dist */
CONSTANTS[2] = 9.58257;
break;
case 773:
/* r2r3dist */
CONSTANTS[1] = -0.281041;
/* r2r3dist */
CONSTANTS[2] = 9.52095;
break;
case 774:
/* r2r3dist */
CONSTANTS[1] = -0.751988;
/* r2r3dist */
CONSTANTS[2] = 9.39143;
break;
case 775:
/* r2r3dist */
CONSTANTS[1] = -0.779442;
/* r2r3dist */
CONSTANTS[2] = 9.93878;
break;
case 776:
/* r2r3dist */
CONSTANTS[1] = 2.06637;
/* r2r3dist */
CONSTANTS[2] = 11.3209;
break;
case 777:
/* r2r3dist */
CONSTANTS[1] = 0.944681;
/* r2r3dist */
CONSTANTS[2] = 10.0518;
break;
case 778:
/* r2r3dist */
CONSTANTS[1] = -0.539419;
/* r2r3dist */
CONSTANTS[2] = 9.99597;
break;
case 779:
/* r2r3dist */
CONSTANTS[1] = 1.45410;
/* r2r3dist */
CONSTANTS[2] = 10.9979;
break;
case 780:
/* r2r3dist */
CONSTANTS[1] = -0.873517;
/* r2r3dist */
CONSTANTS[2] = 9.46646;
break;
case 781:
/* r2r3dist */
CONSTANTS[1] = 0.366166;
/* r2r3dist */
CONSTANTS[2] = 10.0650;
break;
case 782:
/* r2r3dist */
CONSTANTS[1] = -0.694822;
/* r2r3dist */
CONSTANTS[2] = 10.2249;
break;
case 783:
/* r2r3dist */
CONSTANTS[1] = -0.770951;
/* r2r3dist */
CONSTANTS[2] = 9.57244;
break;
case 784:
/* r2r3dist */
CONSTANTS[1] = 0.180218;
/* r2r3dist */
CONSTANTS[2] = 10.0573;
break;
case 785:
/* r2r3dist */
CONSTANTS[1] = -1.07333;
/* r2r3dist */
CONSTANTS[2] = 8.97283;
break;
case 786:
/* r2r3dist */
CONSTANTS[1] = 0.943767;
/* r2r3dist */
CONSTANTS[2] = 10.5321;
break;
case 787:
/* r2r3dist */
CONSTANTS[1] = -0.941803;
/* r2r3dist */
CONSTANTS[2] = 9.51315;
break;
case 788:
/* r2r3dist */
CONSTANTS[1] = 1.28852;
/* r2r3dist */
CONSTANTS[2] = 10.6615;
break;
case 789:
/* r2r3dist */
CONSTANTS[1] = -0.525633;
/* r2r3dist */
CONSTANTS[2] = 9.71593;
break;
case 790:
/* r2r3dist */
CONSTANTS[1] = -0.356620;
/* r2r3dist */
CONSTANTS[2] = 10.3038;
break;
case 791:
/* r2r3dist */
CONSTANTS[1] = -0.0983978;
/* r2r3dist */
CONSTANTS[2] = 9.45802;
break;
case 792:
/* r2r3dist */
CONSTANTS[1] = 1.28864;
/* r2r3dist */
CONSTANTS[2] = 10.6820;
break;
case 793:
/* r2r3dist */
CONSTANTS[1] = -1.44419;
/* r2r3dist */
CONSTANTS[2] = 8.76478;
break;
case 794:
/* r2r3dist */
CONSTANTS[1] = -0.0645201;
/* r2r3dist */
CONSTANTS[2] = 9.37212;
break;
case 795:
/* r2r3dist */
CONSTANTS[1] = 1.60105;
/* r2r3dist */
CONSTANTS[2] = 10.7671;
break;
case 796:
/* r2r3dist */
CONSTANTS[1] = -0.959894;
/* r2r3dist */
CONSTANTS[2] = 9.25317;
break;
case 797:
/* r2r3dist */
CONSTANTS[1] = 1.38561;
/* r2r3dist */
CONSTANTS[2] = 10.1304;
break;
case 798:
/* r2r3dist */
CONSTANTS[1] = 2.20120;
/* r2r3dist */
CONSTANTS[2] = 11.0129;
break;
case 799:
/* r2r3dist */
CONSTANTS[1] = -2.48587;
/* r2r3dist */
CONSTANTS[2] = 8.84285;
break;
case 800:
/* r2r3dist */
CONSTANTS[1] = -0.368809;
/* r2r3dist */
CONSTANTS[2] = 9.93711;
break;
case 801:
/* r2r3dist */
CONSTANTS[1] = -1.25567;
/* r2r3dist */
CONSTANTS[2] = 9.66618;
break;
case 802:
/* r2r3dist */
CONSTANTS[1] = -0.622936;
/* r2r3dist */
CONSTANTS[2] = 9.68600;
break;
case 803:
/* r2r3dist */
CONSTANTS[1] = 0.230822;
/* r2r3dist */
CONSTANTS[2] = 9.33185;
break;
case 804:
/* r2r3dist */
CONSTANTS[1] = -0.325642;
/* r2r3dist */
CONSTANTS[2] = 10.2148;
break;
case 805:
/* r2r3dist */
CONSTANTS[1] = -1.81875;
/* r2r3dist */
CONSTANTS[2] = 8.70316;
break;
case 806:
/* r2r3dist */
CONSTANTS[1] = -0.630032;
/* r2r3dist */
CONSTANTS[2] = 9.43414;
break;
case 807:
/* r2r3dist */
CONSTANTS[1] = 0.632039;
/* r2r3dist */
CONSTANTS[2] = 10.1936;
break;
case 808:
/* r2r3dist */
CONSTANTS[1] = -0.457704;
/* r2r3dist */
CONSTANTS[2] = 10.1273;
break;
case 809:
/* r2r3dist */
CONSTANTS[1] = 0.0919556;
/* r2r3dist */
CONSTANTS[2] = 9.70082;
break;
case 810:
/* r2r3dist */
CONSTANTS[1] = -0.385448;
/* r2r3dist */
CONSTANTS[2] = 9.77498;
break;
case 811:
/* r2r3dist */
CONSTANTS[1] = -0.470480;
/* r2r3dist */
CONSTANTS[2] = 9.49388;
break;
case 812:
/* r2r3dist */
CONSTANTS[1] = -1.52280;
/* r2r3dist */
CONSTANTS[2] = 10.0155;
break;
case 813:
/* r2r3dist */
CONSTANTS[1] = 0.741745;
/* r2r3dist */
CONSTANTS[2] = 10.0430;
break;
case 814:
/* r2r3dist */
CONSTANTS[1] = 1.33674;
/* r2r3dist */
CONSTANTS[2] = 11.2950;
break;
case 815:
/* r2r3dist */
CONSTANTS[1] = -0.646287;
/* r2r3dist */
CONSTANTS[2] = 9.43464;
break;
case 816:
/* r2r3dist */
CONSTANTS[1] = -0.460706;
/* r2r3dist */
CONSTANTS[2] = 9.20949;
break;
case 817:
/* r2r3dist */
CONSTANTS[1] = -0.591060;
/* r2r3dist */
CONSTANTS[2] = 9.87348;
break;
case 818:
/* r2r3dist */
CONSTANTS[1] = 2.53276;
/* r2r3dist */
CONSTANTS[2] = 12.0561;
break;
case 819:
/* r2r3dist */
CONSTANTS[1] = 0.662793;
/* r2r3dist */
CONSTANTS[2] = 10.3984;
break;
case 820:
/* r2r3dist */
CONSTANTS[1] = 0.776526;
/* r2r3dist */
CONSTANTS[2] = 10.3776;
break;
case 821:
/* r2r3dist */
CONSTANTS[1] = -1.61852;
/* r2r3dist */
CONSTANTS[2] = 9.05938;
break;
case 822:
/* r2r3dist */
CONSTANTS[1] = -1.90429;
/* r2r3dist */
CONSTANTS[2] = 9.27718;
break;
case 823:
/* r2r3dist */
CONSTANTS[1] = 0.301283;
/* r2r3dist */
CONSTANTS[2] = 10.1125;
break;
case 824:
/* r2r3dist */
CONSTANTS[1] = -0.509330;
/* r2r3dist */
CONSTANTS[2] = 10.2612;
break;
case 825:
/* r2r3dist */
CONSTANTS[1] = 0.804883;
/* r2r3dist */
CONSTANTS[2] = 11.0501;
break;
case 826:
/* r2r3dist */
CONSTANTS[1] = 0.0540325;
/* r2r3dist */
CONSTANTS[2] = 10.6170;
break;
case 827:
/* r2r3dist */
CONSTANTS[1] = -0.923890;
/* r2r3dist */
CONSTANTS[2] = 8.59465;
break;
case 828:
/* r2r3dist */
CONSTANTS[1] = -0.230129;
/* r2r3dist */
CONSTANTS[2] = 10.1681;
break;
case 829:
/* r2r3dist */
CONSTANTS[1] = -0.0905123;
/* r2r3dist */
CONSTANTS[2] = 10.4879;
break;
case 830:
/* r2r3dist */
CONSTANTS[1] = 1.01425;
/* r2r3dist */
CONSTANTS[2] = 10.8978;
break;
case 831:
/* r2r3dist */
CONSTANTS[1] = -0.736676;
/* r2r3dist */
CONSTANTS[2] = 10.3459;
break;
case 832:
/* r2r3dist */
CONSTANTS[1] = 0.250745;
/* r2r3dist */
CONSTANTS[2] = 10.4242;
break;
case 833:
/* r2r3dist */
CONSTANTS[1] = 0.920250;
/* r2r3dist */
CONSTANTS[2] = 10.0086;
break;
case 834:
/* r2r3dist */
CONSTANTS[1] = -0.353749;
/* r2r3dist */
CONSTANTS[2] = 9.45337;
break;
case 835:
/* r2r3dist */
CONSTANTS[1] = 0.855165;
/* r2r3dist */
CONSTANTS[2] = 10.4502;
break;
case 836:
/* r2r3dist */
CONSTANTS[1] = -0.0274329;
/* r2r3dist */
CONSTANTS[2] = 9.71096;
break;
case 837:
/* r2r3dist */
CONSTANTS[1] = 0.409861;
/* r2r3dist */
CONSTANTS[2] = 9.68843;
break;
case 838:
/* r2r3dist */
CONSTANTS[1] = 1.12378;
/* r2r3dist */
CONSTANTS[2] = 10.3903;
break;
case 839:
/* r2r3dist */
CONSTANTS[1] = 1.82371;
/* r2r3dist */
CONSTANTS[2] = 10.9673;
break;
case 840:
/* r2r3dist */
CONSTANTS[1] = -0.949015;
/* r2r3dist */
CONSTANTS[2] = 10.4615;
break;
case 841:
/* r2r3dist */
CONSTANTS[1] = -0.248353;
/* r2r3dist */
CONSTANTS[2] = 9.56553;
break;
case 842:
/* r2r3dist */
CONSTANTS[1] = 0.115274;
/* r2r3dist */
CONSTANTS[2] = 10.2195;
break;
case 843:
/* r2r3dist */
CONSTANTS[1] = 0.859606;
/* r2r3dist */
CONSTANTS[2] = 10.5438;
break;
case 844:
/* r2r3dist */
CONSTANTS[1] = 0.00345507;
/* r2r3dist */
CONSTANTS[2] = 9.13125;
break;
case 845:
/* r2r3dist */
CONSTANTS[1] = 0.465153;
/* r2r3dist */
CONSTANTS[2] = 9.92882;
break;
case 846:
/* r2r3dist */
CONSTANTS[1] = 1.38424;
/* r2r3dist */
CONSTANTS[2] = 11.0707;
break;
case 847:
/* r2r3dist */
CONSTANTS[1] = 1.94105;
/* r2r3dist */
CONSTANTS[2] = 11.0993;
break;
case 848:
/* r2r3dist */
CONSTANTS[1] = 0.326845;
/* r2r3dist */
CONSTANTS[2] = 10.1832;
break;
case 849:
/* r2r3dist */
CONSTANTS[1] = 0.497948;
/* r2r3dist */
CONSTANTS[2] = 10.8151;
break;
case 850:
/* r2r3dist */
CONSTANTS[1] = 1.53639;
/* r2r3dist */
CONSTANTS[2] = 10.1781;
break;
case 851:
/* r2r3dist */
CONSTANTS[1] = -0.0563169;
/* r2r3dist */
CONSTANTS[2] = 9.43552;
break;
case 852:
/* r2r3dist */
CONSTANTS[1] = 1.24752;
/* r2r3dist */
CONSTANTS[2] = 10.6178;
break;
case 853:
/* r2r3dist */
CONSTANTS[1] = 0.689006;
/* r2r3dist */
CONSTANTS[2] = 10.3522;
break;
case 854:
/* r2r3dist */
CONSTANTS[1] = -0.0745950;
/* r2r3dist */
CONSTANTS[2] = 9.53297;
break;
case 855:
/* r2r3dist */
CONSTANTS[1] = 0.654379;
/* r2r3dist */
CONSTANTS[2] = 11.1969;
break;
case 856:
/* r2r3dist */
CONSTANTS[1] = 0.138394;
/* r2r3dist */
CONSTANTS[2] = 9.63237;
break;
case 857:
/* r2r3dist */
CONSTANTS[1] = 1.29041;
/* r2r3dist */
CONSTANTS[2] = 10.5151;
break;
case 858:
/* r2r3dist */
CONSTANTS[1] = -0.716263;
/* r2r3dist */
CONSTANTS[2] = 9.47040;
break;
case 859:
/* r2r3dist */
CONSTANTS[1] = 1.06463;
/* r2r3dist */
CONSTANTS[2] = 11.0940;
break;
case 860:
/* r2r3dist */
CONSTANTS[1] = 0.691537;
/* r2r3dist */
CONSTANTS[2] = 9.66581;
break;
case 861:
/* r2r3dist */
CONSTANTS[1] = -0.524528;
/* r2r3dist */
CONSTANTS[2] = 9.98193;
break;
case 862:
/* r2r3dist */
CONSTANTS[1] = 0.712051;
/* r2r3dist */
CONSTANTS[2] = 10.3048;
break;
case 863:
/* r2r3dist */
CONSTANTS[1] = -1.08764;
/* r2r3dist */
CONSTANTS[2] = 8.86982;
break;
case 864:
/* r2r3dist */
CONSTANTS[1] = -1.03854;
/* r2r3dist */
CONSTANTS[2] = 10.0426;
break;
case 865:
/* r2r3dist */
CONSTANTS[1] = -0.300397;
/* r2r3dist */
CONSTANTS[2] = 10.0395;
break;
case 866:
/* r2r3dist */
CONSTANTS[1] = 0.163687;
/* r2r3dist */
CONSTANTS[2] = 9.89039;
break;
case 867:
/* r2r3dist */
CONSTANTS[1] = -0.362930;
/* r2r3dist */
CONSTANTS[2] = 10.4433;
break;
case 868:
/* r2r3dist */
CONSTANTS[1] = -1.45712;
/* r2r3dist */
CONSTANTS[2] = 8.62815;
break;
case 869:
/* r2r3dist */
CONSTANTS[1] = 1.03559;
/* r2r3dist */
CONSTANTS[2] = 10.5646;
break;
case 870:
/* r2r3dist */
CONSTANTS[1] = 0.737062;
/* r2r3dist */
CONSTANTS[2] = 11.3079;
break;
case 871:
/* r2r3dist */
CONSTANTS[1] = -0.622119;
/* r2r3dist */
CONSTANTS[2] = 10.6272;
break;
case 872:
/* r2r3dist */
CONSTANTS[1] = 0.126348;
/* r2r3dist */
CONSTANTS[2] = 10.1047;
break;
case 873:
/* r2r3dist */
CONSTANTS[1] = 0.718506;
/* r2r3dist */
CONSTANTS[2] = 9.66798;
break;
case 874:
/* r2r3dist */
CONSTANTS[1] = -1.01678;
/* r2r3dist */
CONSTANTS[2] = 9.98269;
break;
case 875:
/* r2r3dist */
CONSTANTS[1] = -0.0207573;
/* r2r3dist */
CONSTANTS[2] = 10.3439;
break;
case 876:
/* r2r3dist */
CONSTANTS[1] = -1.42406;
/* r2r3dist */
CONSTANTS[2] = 8.68692;
break;
case 877:
/* r2r3dist */
CONSTANTS[1] = 0.299812;
/* r2r3dist */
CONSTANTS[2] = 9.50249;
break;
case 878:
/* r2r3dist */
CONSTANTS[1] = -0.0204763;
/* r2r3dist */
CONSTANTS[2] = 10.5821;
break;
case 879:
/* r2r3dist */
CONSTANTS[1] = 1.06733;
/* r2r3dist */
CONSTANTS[2] = 9.85556;
break;
case 880:
/* r2r3dist */
CONSTANTS[1] = 0.654187;
/* r2r3dist */
CONSTANTS[2] = 11.0155;
break;
case 881:
/* r2r3dist */
CONSTANTS[1] = -0.276507;
/* r2r3dist */
CONSTANTS[2] = 9.39672;
break;
case 882:
/* r2r3dist */
CONSTANTS[1] = 0.192439;
/* r2r3dist */
CONSTANTS[2] = 9.99971;
break;
case 883:
/* r2r3dist */
CONSTANTS[1] = 0.392675;
/* r2r3dist */
CONSTANTS[2] = 10.6096;
break;
case 884:
/* r2r3dist */
CONSTANTS[1] = -0.537059;
/* r2r3dist */
CONSTANTS[2] = 9.43838;
break;
case 885:
/* r2r3dist */
CONSTANTS[1] = -0.121059;
/* r2r3dist */
CONSTANTS[2] = 9.88089;
break;
case 886:
/* r2r3dist */
CONSTANTS[1] = 0.824322;
/* r2r3dist */
CONSTANTS[2] = 9.85888;
break;
case 887:
/* r2r3dist */
CONSTANTS[1] = 1.32241;
/* r2r3dist */
CONSTANTS[2] = 9.97195;
break;
case 888:
/* r2r3dist */
CONSTANTS[1] = 1.72189;
/* r2r3dist */
CONSTANTS[2] = 9.72212;
break;
case 889:
/* r2r3dist */
CONSTANTS[1] = -0.476691;
/* r2r3dist */
CONSTANTS[2] = 9.33982;
break;
case 890:
/* r2r3dist */
CONSTANTS[1] = -0.513109;
/* r2r3dist */
CONSTANTS[2] = 9.69062;
break;
case 891:
/* r2r3dist */
CONSTANTS[1] = 0.907776;
/* r2r3dist */
CONSTANTS[2] = 10.4962;
break;
case 892:
/* r2r3dist */
CONSTANTS[1] = 0.998408;
/* r2r3dist */
CONSTANTS[2] = 9.77578;
break;
case 893:
/* r2r3dist */
CONSTANTS[1] = -0.453564;
/* r2r3dist */
CONSTANTS[2] = 8.48969;
break;
case 894:
/* r2r3dist */
CONSTANTS[1] = -0.0336600;
/* r2r3dist */
CONSTANTS[2] = 10.3131;
break;
case 895:
/* r2r3dist */
CONSTANTS[1] = 0.665363;
/* r2r3dist */
CONSTANTS[2] = 10.2302;
break;
case 896:
/* r2r3dist */
CONSTANTS[1] = -2.09161;
/* r2r3dist */
CONSTANTS[2] = 9.27132;
break;
case 897:
/* r2r3dist */
CONSTANTS[1] = -0.564510;
/* r2r3dist */
CONSTANTS[2] = 10.2897;
break;
case 898:
/* r2r3dist */
CONSTANTS[1] = 0.221554;
/* r2r3dist */
CONSTANTS[2] = 9.36024;
break;
case 899:
/* r2r3dist */
CONSTANTS[1] = -0.919540;
/* r2r3dist */
CONSTANTS[2] = 8.71021;
break;
case 900:
/* r2r3dist */
CONSTANTS[1] = -2.05721;
/* r2r3dist */
CONSTANTS[2] = 8.62042;
break;
case 901:
/* r2r3dist */
CONSTANTS[1] = -1.73401;
/* r2r3dist */
CONSTANTS[2] = 9.31850;
break;
case 902:
/* r2r3dist */
CONSTANTS[1] = -0.255069;
/* r2r3dist */
CONSTANTS[2] = 10.9187;
break;
case 903:
/* r2r3dist */
CONSTANTS[1] = -0.693615;
/* r2r3dist */
CONSTANTS[2] = 10.6267;
break;
case 904:
/* r2r3dist */
CONSTANTS[1] = 0.996264;
/* r2r3dist */
CONSTANTS[2] = 10.2673;
break;
case 905:
/* r2r3dist */
CONSTANTS[1] = 0.0130013;
/* r2r3dist */
CONSTANTS[2] = 9.95764;
break;
case 906:
/* r2r3dist */
CONSTANTS[1] = 0.613593;
/* r2r3dist */
CONSTANTS[2] = 10.7239;
break;
case 907:
/* r2r3dist */
CONSTANTS[1] = 1.00697;
/* r2r3dist */
CONSTANTS[2] = 9.83879;
break;
case 908:
/* r2r3dist */
CONSTANTS[1] = 0.993183;
/* r2r3dist */
CONSTANTS[2] = 10.5225;
break;
case 909:
/* r2r3dist */
CONSTANTS[1] = -1.48908;
/* r2r3dist */
CONSTANTS[2] = 9.54085;
break;
case 910:
/* r2r3dist */
CONSTANTS[1] = 1.30673;
/* r2r3dist */
CONSTANTS[2] = 9.74829;
break;
case 911:
/* r2r3dist */
CONSTANTS[1] = -0.0716510;
/* r2r3dist */
CONSTANTS[2] = 9.94066;
break;
case 912:
/* r2r3dist */
CONSTANTS[1] = 0.546889;
/* r2r3dist */
CONSTANTS[2] = 10.8390;
break;
case 913:
/* r2r3dist */
CONSTANTS[1] = -1.79451;
/* r2r3dist */
CONSTANTS[2] = 9.29911;
break;
case 914:
/* r2r3dist */
CONSTANTS[1] = -0.0469439;
/* r2r3dist */
CONSTANTS[2] = 9.46157;
break;
case 915:
/* r2r3dist */
CONSTANTS[1] = 0.390029;
/* r2r3dist */
CONSTANTS[2] = 10.2650;
break;
case 916:
/* r2r3dist */
CONSTANTS[1] = -0.626906;
/* r2r3dist */
CONSTANTS[2] = 9.64548;
break;
case 917:
/* r2r3dist */
CONSTANTS[1] = 0.289368;
/* r2r3dist */
CONSTANTS[2] = 10.4897;
break;
case 918:
/* r2r3dist */
CONSTANTS[1] = -2.06796;
/* r2r3dist */
CONSTANTS[2] = 8.77572;
break;
case 919:
/* r2r3dist */
CONSTANTS[1] = -1.03680;
/* r2r3dist */
CONSTANTS[2] = 10.7073;
break;
case 920:
/* r2r3dist */
CONSTANTS[1] = -1.40329;
/* r2r3dist */
CONSTANTS[2] = 9.27088;
break;
case 921:
/* r2r3dist */
CONSTANTS[1] = 0.200492;
/* r2r3dist */
CONSTANTS[2] = 10.2361;
break;
case 922:
/* r2r3dist */
CONSTANTS[1] = -0.599786;
/* r2r3dist */
CONSTANTS[2] = 9.92477;
break;
case 923:
/* r2r3dist */
CONSTANTS[1] = 0.811769;
/* r2r3dist */
CONSTANTS[2] = 9.74241;
break;
case 924:
/* r2r3dist */
CONSTANTS[1] = -0.722313;
/* r2r3dist */
CONSTANTS[2] = 9.96940;
break;
case 925:
/* r2r3dist */
CONSTANTS[1] = -1.17244;
/* r2r3dist */
CONSTANTS[2] = 10.1378;
break;
case 926:
/* r2r3dist */
CONSTANTS[1] = -0.222185;
/* r2r3dist */
CONSTANTS[2] = 10.1155;
break;
case 927:
/* r2r3dist */
CONSTANTS[1] = -1.13232;
/* r2r3dist */
CONSTANTS[2] = 10.5031;
break;
case 928:
/* r2r3dist */
CONSTANTS[1] = -0.455183;
/* r2r3dist */
CONSTANTS[2] = 10.7724;
break;
case 929:
/* r2r3dist */
CONSTANTS[1] = 1.16838;
/* r2r3dist */
CONSTANTS[2] = 10.6923;
break;
case 930:
/* r2r3dist */
CONSTANTS[1] = -0.549595;
/* r2r3dist */
CONSTANTS[2] = 10.0214;
break;
case 931:
/* r2r3dist */
CONSTANTS[1] = -0.140955;
/* r2r3dist */
CONSTANTS[2] = 9.00435;
break;
case 932:
/* r2r3dist */
CONSTANTS[1] = -0.587395;
/* r2r3dist */
CONSTANTS[2] = 9.93871;
break;
case 933:
/* r2r3dist */
CONSTANTS[1] = -0.888449;
/* r2r3dist */
CONSTANTS[2] = 9.78375;
break;
case 934:
/* r2r3dist */
CONSTANTS[1] = 1.85345;
/* r2r3dist */
CONSTANTS[2] = 11.5064;
break;
case 935:
/* r2r3dist */
CONSTANTS[1] = -0.479088;
/* r2r3dist */
CONSTANTS[2] = 10.2979;
break;
case 936:
/* r2r3dist */
CONSTANTS[1] = -0.124059;
/* r2r3dist */
CONSTANTS[2] = 9.88491;
break;
case 937:
/* r2r3dist */
CONSTANTS[1] = -0.301915;
/* r2r3dist */
CONSTANTS[2] = 9.19361;
break;
case 938:
/* r2r3dist */
CONSTANTS[1] = -0.956338;
/* r2r3dist */
CONSTANTS[2] = 9.10211;
break;
case 939:
/* r2r3dist */
CONSTANTS[1] = 2.16400;
/* r2r3dist */
CONSTANTS[2] = 10.6818;
break;
case 940:
/* r2r3dist */
CONSTANTS[1] = -0.723715;
/* r2r3dist */
CONSTANTS[2] = 10.1692;
break;
case 941:
/* r2r3dist */
CONSTANTS[1] = -0.665370;
/* r2r3dist */
CONSTANTS[2] = 9.15916;
break;
case 942:
/* r2r3dist */
CONSTANTS[1] = -0.646668;
/* r2r3dist */
CONSTANTS[2] = 10.0831;
break;
case 943:
/* r2r3dist */
CONSTANTS[1] = -0.695649;
/* r2r3dist */
CONSTANTS[2] = 10.5380;
break;
case 944:
/* r2r3dist */
CONSTANTS[1] = 0.315151;
/* r2r3dist */
CONSTANTS[2] = 9.72909;
break;
case 945:
/* r2r3dist */
CONSTANTS[1] = 0.352090;
/* r2r3dist */
CONSTANTS[2] = 10.6583;
break;
case 946:
/* r2r3dist */
CONSTANTS[1] = 1.36952;
/* r2r3dist */
CONSTANTS[2] = 10.8534;
break;
case 947:
/* r2r3dist */
CONSTANTS[1] = -0.910147;
/* r2r3dist */
CONSTANTS[2] = 9.26979;
break;
case 948:
/* r2r3dist */
CONSTANTS[1] = 1.34310;
/* r2r3dist */
CONSTANTS[2] = 10.3238;
break;
case 949:
/* r2r3dist */
CONSTANTS[1] = 1.03072;
/* r2r3dist */
CONSTANTS[2] = 11.3989;
break;
case 950:
/* r2r3dist */
CONSTANTS[1] = -0.455554;
/* r2r3dist */
CONSTANTS[2] = 11.2028;
break;
case 951:
/* r2r3dist */
CONSTANTS[1] = 0.852450;
/* r2r3dist */
CONSTANTS[2] = 10.4955;
break;
case 952:
/* r2r3dist */
CONSTANTS[1] = -0.0807580;
/* r2r3dist */
CONSTANTS[2] = 9.23639;
break;
case 953:
/* r2r3dist */
CONSTANTS[1] = -0.565375;
/* r2r3dist */
CONSTANTS[2] = 10.2599;
break;
case 954:
/* r2r3dist */
CONSTANTS[1] = -0.447625;
/* r2r3dist */
CONSTANTS[2] = 9.82794;
break;
case 955:
/* r2r3dist */
CONSTANTS[1] = -0.997441;
/* r2r3dist */
CONSTANTS[2] = 9.48575;
break;
case 956:
/* r2r3dist */
CONSTANTS[1] = 1.32302;
/* r2r3dist */
CONSTANTS[2] = 11.2775;
break;
case 957:
/* r2r3dist */
CONSTANTS[1] = 0.220730;
/* r2r3dist */
CONSTANTS[2] = 10.7034;
break;
case 958:
/* r2r3dist */
CONSTANTS[1] = 1.07881;
/* r2r3dist */
CONSTANTS[2] = 10.8143;
break;
case 959:
/* r2r3dist */
CONSTANTS[1] = 1.13480;
/* r2r3dist */
CONSTANTS[2] = 10.2689;
break;
case 960:
/* r2r3dist */
CONSTANTS[1] = 1.78137;
/* r2r3dist */
CONSTANTS[2] = 10.4883;
break;
case 961:
/* r2r3dist */
CONSTANTS[1] = -0.434830;
/* r2r3dist */
CONSTANTS[2] = 9.36449;
break;
case 962:
/* r2r3dist */
CONSTANTS[1] = 0.115596;
/* r2r3dist */
CONSTANTS[2] = 10.0827;
break;
case 963:
/* r2r3dist */
CONSTANTS[1] = 1.41563;
/* r2r3dist */
CONSTANTS[2] = 10.6233;
break;
case 964:
/* r2r3dist */
CONSTANTS[1] = 0.259822;
/* r2r3dist */
CONSTANTS[2] = 11.8505;
break;
case 965:
/* r2r3dist */
CONSTANTS[1] = 0.187238;
/* r2r3dist */
CONSTANTS[2] = 10.5595;
break;
case 966:
/* r2r3dist */
CONSTANTS[1] = -1.19495;
/* r2r3dist */
CONSTANTS[2] = 9.27385;
break;
case 967:
/* r2r3dist */
CONSTANTS[1] = -0.0711781;
/* r2r3dist */
CONSTANTS[2] = 9.49080;
break;
case 968:
/* r2r3dist */
CONSTANTS[1] = -1.13640;
/* r2r3dist */
CONSTANTS[2] = 9.51427;
break;
case 969:
/* r2r3dist */
CONSTANTS[1] = -1.00529;
/* r2r3dist */
CONSTANTS[2] = 9.82120;
break;
case 970:
/* r2r3dist */
CONSTANTS[1] = -0.148196;
/* r2r3dist */
CONSTANTS[2] = 9.28400;
break;
case 971:
/* r2r3dist */
CONSTANTS[1] = 0.00359574;
/* r2r3dist */
CONSTANTS[2] = 10.3654;
break;
case 972:
/* r2r3dist */
CONSTANTS[1] = -2.15386;
/* r2r3dist */
CONSTANTS[2] = 9.42831;
break;
case 973:
/* r2r3dist */
CONSTANTS[1] = -0.316218;
/* r2r3dist */
CONSTANTS[2] = 9.61370;
break;
case 974:
/* r2r3dist */
CONSTANTS[1] = -0.273544;
/* r2r3dist */
CONSTANTS[2] = 9.45219;
break;
case 975:
/* r2r3dist */
CONSTANTS[1] = 0.309532;
/* r2r3dist */
CONSTANTS[2] = 10.1687;
break;
case 976:
/* r2r3dist */
CONSTANTS[1] = 0.703372;
/* r2r3dist */
CONSTANTS[2] = 11.1437;
break;
case 977:
/* r2r3dist */
CONSTANTS[1] = -0.412385;
/* r2r3dist */
CONSTANTS[2] = 9.73344;
break;
case 978:
/* r2r3dist */
CONSTANTS[1] = 0.470502;
/* r2r3dist */
CONSTANTS[2] = 10.6581;
break;
case 979:
/* r2r3dist */
CONSTANTS[1] = 1.67429;
/* r2r3dist */
CONSTANTS[2] = 9.58666;
break;
case 980:
/* r2r3dist */
CONSTANTS[1] = -1.26984;
/* r2r3dist */
CONSTANTS[2] = 9.36332;
break;
case 981:
/* r2r3dist */
CONSTANTS[1] = 0.838545;
/* r2r3dist */
CONSTANTS[2] = 10.5400;
break;
case 982:
/* r2r3dist */
CONSTANTS[1] = -0.675980;
/* r2r3dist */
CONSTANTS[2] = 9.50656;
break;
case 983:
/* r2r3dist */
CONSTANTS[1] = 0.801317;
/* r2r3dist */
CONSTANTS[2] = 10.4778;
break;
case 984:
/* r2r3dist */
CONSTANTS[1] = 1.18293;
/* r2r3dist */
CONSTANTS[2] = 10.5387;
break;
case 985:
/* r2r3dist */
CONSTANTS[1] = -0.889240;
/* r2r3dist */
CONSTANTS[2] = 9.94658;
break;
case 986:
/* r2r3dist */
CONSTANTS[1] = -0.784644;
/* r2r3dist */
CONSTANTS[2] = 8.30034;
break;
case 987:
/* r2r3dist */
CONSTANTS[1] = -0.515011;
/* r2r3dist */
CONSTANTS[2] = 9.43816;
break;
case 988:
/* r2r3dist */
CONSTANTS[1] = -1.56726;
/* r2r3dist */
CONSTANTS[2] = 8.83984;
break;
case 989:
/* r2r3dist */
CONSTANTS[1] = 0.352946;
/* r2r3dist */
CONSTANTS[2] = 9.81484;
break;
case 990:
/* r2r3dist */
CONSTANTS[1] = 0.798638;
/* r2r3dist */
CONSTANTS[2] = 10.4169;
break;
case 991:
/* r2r3dist */
CONSTANTS[1] = 1.87032;
/* r2r3dist */
CONSTANTS[2] = 9.92826;
break;
case 992:
/* r2r3dist */
CONSTANTS[1] = 0.518340;
/* r2r3dist */
CONSTANTS[2] = 9.43074;
break;
case 993:
/* r2r3dist */
CONSTANTS[1] = -0.914958;
/* r2r3dist */
CONSTANTS[2] = 9.07036;
break;
case 994:
/* r2r3dist */
CONSTANTS[1] = 3.18263;
/* r2r3dist */
CONSTANTS[2] = 11.6355;
break;
case 995:
/* r2r3dist */
CONSTANTS[1] = -2.11596;
/* r2r3dist */
CONSTANTS[2] = 8.82667;
break;
case 996:
/* r2r3dist */
CONSTANTS[1] = 1.07909;
/* r2r3dist */
CONSTANTS[2] = 9.45082;
break;
case 997:
/* r2r3dist */
CONSTANTS[1] = -1.36946;
/* r2r3dist */
CONSTANTS[2] = 9.18706;
break;
case 998:
/* r2r3dist */
CONSTANTS[1] = 1.20362;
/* r2r3dist */
CONSTANTS[2] = 11.0572;
break;
case 999:
/* r2r3dist */
CONSTANTS[1] = 0.133729;
/* r2r3dist */
CONSTANTS[2] = 10.4051;
break;
}
/* r1V */
CONSTANTS[3] = 1.00000;
/* r1M */
CONSTANTS[4] = 0.00000;
/* r1Dist */
CONSTANTS[5] = SampleUsingPDF(&pdf_0, 0, pdf_roots_0, CONSTANTS, ALGEBRAIC);
/* valrate */
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
