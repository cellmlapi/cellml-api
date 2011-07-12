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
 * * * Variable type: constant
 * * * Variable index: 0
 * * * Variable storage: CONSTANTS[0]
 */
double pdf_0(double bvar, double* CONSTANTS, double* ALGEBRAIC)
{
  return ( (1.00000/ pow(( 2.00000* 3.14159265358979*CONSTANTS[3]), 1.0 / 2))*(exp((- ((pow((bvar - CONSTANTS[4]), 2.00000))/( 2.00000*CONSTANTS[3]))))));
}
void SetupFixedConstants(double* CONSTANTS, double* RATES, double* STATES)
{
STATES[0] = 0;
CONSTANTS[0] = 0;
switch (rand() % 1000)
{
case 0:
CONSTANTS[1] = -0.397304;
CONSTANTS[2] = 8.70924;
break;
case 1:
CONSTANTS[1] = -1.16749;
CONSTANTS[2] = 9.68624;
break;
case 2:
CONSTANTS[1] = -0.649844;
CONSTANTS[2] = 10.0365;
break;
case 3:
CONSTANTS[1] = 1.82449;
CONSTANTS[2] = 12.1333;
break;
case 4:
CONSTANTS[1] = -2.67560;
CONSTANTS[2] = 9.43155;
break;
case 5:
CONSTANTS[1] = 0.0547296;
CONSTANTS[2] = 10.0719;
break;
case 6:
CONSTANTS[1] = 1.07090;
CONSTANTS[2] = 10.0235;
break;
case 7:
CONSTANTS[1] = -0.554121;
CONSTANTS[2] = 10.4162;
break;
case 8:
CONSTANTS[1] = -0.330647;
CONSTANTS[2] = 9.37518;
break;
case 9:
CONSTANTS[1] = 0.438340;
CONSTANTS[2] = 11.1350;
break;
case 10:
CONSTANTS[1] = -1.00341;
CONSTANTS[2] = 8.96924;
break;
case 11:
CONSTANTS[1] = -0.280605;
CONSTANTS[2] = 10.5120;
break;
case 12:
CONSTANTS[1] = 0.169892;
CONSTANTS[2] = 10.0342;
break;
case 13:
CONSTANTS[1] = -3.24431;
CONSTANTS[2] = 8.59351;
break;
case 14:
CONSTANTS[1] = 0.537377;
CONSTANTS[2] = 10.0864;
break;
case 15:
CONSTANTS[1] = 0.259131;
CONSTANTS[2] = 9.33872;
break;
case 16:
CONSTANTS[1] = 0.196380;
CONSTANTS[2] = 9.43961;
break;
case 17:
CONSTANTS[1] = 0.772454;
CONSTANTS[2] = 9.98352;
break;
case 18:
CONSTANTS[1] = -0.442969;
CONSTANTS[2] = 10.5780;
break;
case 19:
CONSTANTS[1] = 0.529665;
CONSTANTS[2] = 9.43131;
break;
case 20:
CONSTANTS[1] = -0.175020;
CONSTANTS[2] = 9.53173;
break;
case 21:
CONSTANTS[1] = 1.32604;
CONSTANTS[2] = 11.1211;
break;
case 22:
CONSTANTS[1] = 0.464047;
CONSTANTS[2] = 9.57088;
break;
case 23:
CONSTANTS[1] = 0.0433567;
CONSTANTS[2] = 9.02465;
break;
case 24:
CONSTANTS[1] = -1.56877;
CONSTANTS[2] = 9.92779;
break;
case 25:
CONSTANTS[1] = -0.475073;
CONSTANTS[2] = 10.5689;
break;
case 26:
CONSTANTS[1] = -1.06808;
CONSTANTS[2] = 9.96579;
break;
case 27:
CONSTANTS[1] = -0.398461;
CONSTANTS[2] = 10.2092;
break;
case 28:
CONSTANTS[1] = -0.527764;
CONSTANTS[2] = 10.1104;
break;
case 29:
CONSTANTS[1] = 0.461493;
CONSTANTS[2] = 10.6514;
break;
case 30:
CONSTANTS[1] = 1.95319;
CONSTANTS[2] = 10.7851;
break;
case 31:
CONSTANTS[1] = -0.284033;
CONSTANTS[2] = 9.92798;
break;
case 32:
CONSTANTS[1] = 1.31951;
CONSTANTS[2] = 11.4740;
break;
case 33:
CONSTANTS[1] = -0.116536;
CONSTANTS[2] = 10.1436;
break;
case 34:
CONSTANTS[1] = -0.601142;
CONSTANTS[2] = 9.35790;
break;
case 35:
CONSTANTS[1] = 0.409480;
CONSTANTS[2] = 10.3878;
break;
case 36:
CONSTANTS[1] = -1.69898;
CONSTANTS[2] = 9.13079;
break;
case 37:
CONSTANTS[1] = 0.247768;
CONSTANTS[2] = 10.8265;
break;
case 38:
CONSTANTS[1] = -1.29528;
CONSTANTS[2] = 9.87246;
break;
case 39:
CONSTANTS[1] = 0.484394;
CONSTANTS[2] = 10.3463;
break;
case 40:
CONSTANTS[1] = -0.122231;
CONSTANTS[2] = 9.76447;
break;
case 41:
CONSTANTS[1] = 1.12046;
CONSTANTS[2] = 11.8710;
break;
case 42:
CONSTANTS[1] = -0.925777;
CONSTANTS[2] = 10.0598;
break;
case 43:
CONSTANTS[1] = -1.19540;
CONSTANTS[2] = 9.54287;
break;
case 44:
CONSTANTS[1] = 1.00306;
CONSTANTS[2] = 9.96969;
break;
case 45:
CONSTANTS[1] = -0.915627;
CONSTANTS[2] = 9.68636;
break;
case 46:
CONSTANTS[1] = 0.000429096;
CONSTANTS[2] = 9.47965;
break;
case 47:
CONSTANTS[1] = -0.356800;
CONSTANTS[2] = 9.02946;
break;
case 48:
CONSTANTS[1] = 0.481115;
CONSTANTS[2] = 10.1639;
break;
case 49:
CONSTANTS[1] = 0.984145;
CONSTANTS[2] = 10.9652;
break;
case 50:
CONSTANTS[1] = -0.937497;
CONSTANTS[2] = 9.78731;
break;
case 51:
CONSTANTS[1] = 2.02395;
CONSTANTS[2] = 11.3836;
break;
case 52:
CONSTANTS[1] = -0.857944;
CONSTANTS[2] = 9.73200;
break;
case 53:
CONSTANTS[1] = -0.492842;
CONSTANTS[2] = 9.14182;
break;
case 54:
CONSTANTS[1] = 1.97649;
CONSTANTS[2] = 10.9234;
break;
case 55:
CONSTANTS[1] = 3.18893;
CONSTANTS[2] = 11.5659;
break;
case 56:
CONSTANTS[1] = 0.850761;
CONSTANTS[2] = 10.3575;
break;
case 57:
CONSTANTS[1] = 1.16166;
CONSTANTS[2] = 10.3596;
break;
case 58:
CONSTANTS[1] = 0.855790;
CONSTANTS[2] = 10.2775;
break;
case 59:
CONSTANTS[1] = -1.90250;
CONSTANTS[2] = 9.10788;
break;
case 60:
CONSTANTS[1] = 1.34709;
CONSTANTS[2] = 10.9309;
break;
case 61:
CONSTANTS[1] = -0.542613;
CONSTANTS[2] = 9.56537;
break;
case 62:
CONSTANTS[1] = -0.612029;
CONSTANTS[2] = 9.84630;
break;
case 63:
CONSTANTS[1] = 0.662960;
CONSTANTS[2] = 10.2884;
break;
case 64:
CONSTANTS[1] = -0.178510;
CONSTANTS[2] = 10.8677;
break;
case 65:
CONSTANTS[1] = -0.790018;
CONSTANTS[2] = 9.58712;
break;
case 66:
CONSTANTS[1] = 0.0497061;
CONSTANTS[2] = 9.62169;
break;
case 67:
CONSTANTS[1] = -0.109377;
CONSTANTS[2] = 11.1402;
break;
case 68:
CONSTANTS[1] = 0.973509;
CONSTANTS[2] = 11.4186;
break;
case 69:
CONSTANTS[1] = 0.424305;
CONSTANTS[2] = 11.1673;
break;
case 70:
CONSTANTS[1] = -0.862519;
CONSTANTS[2] = 9.24313;
break;
case 71:
CONSTANTS[1] = 0.873642;
CONSTANTS[2] = 9.90775;
break;
case 72:
CONSTANTS[1] = 0.627842;
CONSTANTS[2] = 11.0840;
break;
case 73:
CONSTANTS[1] = -0.982380;
CONSTANTS[2] = 9.31079;
break;
case 74:
CONSTANTS[1] = -0.753886;
CONSTANTS[2] = 9.67372;
break;
case 75:
CONSTANTS[1] = 0.305687;
CONSTANTS[2] = 9.95795;
break;
case 76:
CONSTANTS[1] = -0.448600;
CONSTANTS[2] = 9.98373;
break;
case 77:
CONSTANTS[1] = 1.96389;
CONSTANTS[2] = 10.2289;
break;
case 78:
CONSTANTS[1] = 0.103843;
CONSTANTS[2] = 9.91418;
break;
case 79:
CONSTANTS[1] = -0.294529;
CONSTANTS[2] = 9.92863;
break;
case 80:
CONSTANTS[1] = -1.09942;
CONSTANTS[2] = 9.73231;
break;
case 81:
CONSTANTS[1] = -1.21572;
CONSTANTS[2] = 10.1152;
break;
case 82:
CONSTANTS[1] = -1.97628;
CONSTANTS[2] = 8.52038;
break;
case 83:
CONSTANTS[1] = -1.08008;
CONSTANTS[2] = 10.0401;
break;
case 84:
CONSTANTS[1] = -0.131598;
CONSTANTS[2] = 9.98556;
break;
case 85:
CONSTANTS[1] = 1.48665;
CONSTANTS[2] = 10.9267;
break;
case 86:
CONSTANTS[1] = 0.385169;
CONSTANTS[2] = 9.95288;
break;
case 87:
CONSTANTS[1] = -0.314853;
CONSTANTS[2] = 9.64242;
break;
case 88:
CONSTANTS[1] = -1.46278;
CONSTANTS[2] = 10.1506;
break;
case 89:
CONSTANTS[1] = -0.345246;
CONSTANTS[2] = 10.5831;
break;
case 90:
CONSTANTS[1] = -1.33460;
CONSTANTS[2] = 9.31070;
break;
case 91:
CONSTANTS[1] = 0.973103;
CONSTANTS[2] = 11.1597;
break;
case 92:
CONSTANTS[1] = 0.134743;
CONSTANTS[2] = 9.92023;
break;
case 93:
CONSTANTS[1] = -1.24889;
CONSTANTS[2] = 10.2489;
break;
case 94:
CONSTANTS[1] = -0.412555;
CONSTANTS[2] = 9.91934;
break;
case 95:
CONSTANTS[1] = 1.29382;
CONSTANTS[2] = 10.2714;
break;
case 96:
CONSTANTS[1] = -0.726661;
CONSTANTS[2] = 9.75208;
break;
case 97:
CONSTANTS[1] = 1.05886;
CONSTANTS[2] = 11.1631;
break;
case 98:
CONSTANTS[1] = -0.522445;
CONSTANTS[2] = 9.16190;
break;
case 99:
CONSTANTS[1] = 0.248655;
CONSTANTS[2] = 10.3259;
break;
case 100:
CONSTANTS[1] = -0.489311;
CONSTANTS[2] = 9.80166;
break;
case 101:
CONSTANTS[1] = 0.0771580;
CONSTANTS[2] = 9.10271;
break;
case 102:
CONSTANTS[1] = 0.844544;
CONSTANTS[2] = 9.63365;
break;
case 103:
CONSTANTS[1] = 1.34215;
CONSTANTS[2] = 10.0078;
break;
case 104:
CONSTANTS[1] = 1.11532;
CONSTANTS[2] = 10.2361;
break;
case 105:
CONSTANTS[1] = 2.07959;
CONSTANTS[2] = 11.0199;
break;
case 106:
CONSTANTS[1] = -0.0884372;
CONSTANTS[2] = 9.34866;
break;
case 107:
CONSTANTS[1] = -0.759000;
CONSTANTS[2] = 9.26611;
break;
case 108:
CONSTANTS[1] = 0.227441;
CONSTANTS[2] = 9.62424;
break;
case 109:
CONSTANTS[1] = 0.0782673;
CONSTANTS[2] = 10.9109;
break;
case 110:
CONSTANTS[1] = -0.856699;
CONSTANTS[2] = 9.54973;
break;
case 111:
CONSTANTS[1] = -0.420034;
CONSTANTS[2] = 9.77135;
break;
case 112:
CONSTANTS[1] = -1.53474;
CONSTANTS[2] = 9.15775;
break;
case 113:
CONSTANTS[1] = -1.39898;
CONSTANTS[2] = 9.18780;
break;
case 114:
CONSTANTS[1] = -0.695713;
CONSTANTS[2] = 9.34482;
break;
case 115:
CONSTANTS[1] = 1.56354;
CONSTANTS[2] = 10.4519;
break;
case 116:
CONSTANTS[1] = 0.521136;
CONSTANTS[2] = 9.81228;
break;
case 117:
CONSTANTS[1] = -1.44351;
CONSTANTS[2] = 9.93480;
break;
case 118:
CONSTANTS[1] = 1.21405;
CONSTANTS[2] = 10.9245;
break;
case 119:
CONSTANTS[1] = -0.233544;
CONSTANTS[2] = 9.75290;
break;
case 120:
CONSTANTS[1] = -0.810330;
CONSTANTS[2] = 9.82137;
break;
case 121:
CONSTANTS[1] = -2.34638;
CONSTANTS[2] = 8.15355;
break;
case 122:
CONSTANTS[1] = -1.02356;
CONSTANTS[2] = 9.95785;
break;
case 123:
CONSTANTS[1] = -2.01851;
CONSTANTS[2] = 8.67204;
break;
case 124:
CONSTANTS[1] = 0.442318;
CONSTANTS[2] = 10.7635;
break;
case 125:
CONSTANTS[1] = -0.354193;
CONSTANTS[2] = 10.4928;
break;
case 126:
CONSTANTS[1] = -0.919096;
CONSTANTS[2] = 9.54161;
break;
case 127:
CONSTANTS[1] = 0.532995;
CONSTANTS[2] = 9.90541;
break;
case 128:
CONSTANTS[1] = -1.04530;
CONSTANTS[2] = 9.44963;
break;
case 129:
CONSTANTS[1] = -0.409808;
CONSTANTS[2] = 9.75445;
break;
case 130:
CONSTANTS[1] = -0.604661;
CONSTANTS[2] = 9.66697;
break;
case 131:
CONSTANTS[1] = -0.0262765;
CONSTANTS[2] = 10.3651;
break;
case 132:
CONSTANTS[1] = 0.608419;
CONSTANTS[2] = 9.99240;
break;
case 133:
CONSTANTS[1] = 1.20405;
CONSTANTS[2] = 10.0850;
break;
case 134:
CONSTANTS[1] = -0.399546;
CONSTANTS[2] = 9.43459;
break;
case 135:
CONSTANTS[1] = 0.507774;
CONSTANTS[2] = 9.81893;
break;
case 136:
CONSTANTS[1] = -1.39854;
CONSTANTS[2] = 9.07308;
break;
case 137:
CONSTANTS[1] = 0.156823;
CONSTANTS[2] = 10.8610;
break;
case 138:
CONSTANTS[1] = 0.274586;
CONSTANTS[2] = 9.59653;
break;
case 139:
CONSTANTS[1] = -0.302830;
CONSTANTS[2] = 9.21990;
break;
case 140:
CONSTANTS[1] = -1.65531;
CONSTANTS[2] = 10.2439;
break;
case 141:
CONSTANTS[1] = 0.340942;
CONSTANTS[2] = 9.14995;
break;
case 142:
CONSTANTS[1] = -1.13199;
CONSTANTS[2] = 9.48058;
break;
case 143:
CONSTANTS[1] = 0.562440;
CONSTANTS[2] = 10.3752;
break;
case 144:
CONSTANTS[1] = 1.41276;
CONSTANTS[2] = 11.2429;
break;
case 145:
CONSTANTS[1] = 1.21882;
CONSTANTS[2] = 11.6472;
break;
case 146:
CONSTANTS[1] = -0.739580;
CONSTANTS[2] = 9.27707;
break;
case 147:
CONSTANTS[1] = -1.11941;
CONSTANTS[2] = 9.27118;
break;
case 148:
CONSTANTS[1] = 1.56583;
CONSTANTS[2] = 10.7700;
break;
case 149:
CONSTANTS[1] = 0.0612810;
CONSTANTS[2] = 9.68441;
break;
case 150:
CONSTANTS[1] = 0.352526;
CONSTANTS[2] = 10.1031;
break;
case 151:
CONSTANTS[1] = 0.807038;
CONSTANTS[2] = 10.6675;
break;
case 152:
CONSTANTS[1] = 0.856719;
CONSTANTS[2] = 10.4921;
break;
case 153:
CONSTANTS[1] = -1.23230;
CONSTANTS[2] = 9.53305;
break;
case 154:
CONSTANTS[1] = -0.750940;
CONSTANTS[2] = 9.58842;
break;
case 155:
CONSTANTS[1] = 0.911477;
CONSTANTS[2] = 9.81170;
break;
case 156:
CONSTANTS[1] = -0.855538;
CONSTANTS[2] = 9.26032;
break;
case 157:
CONSTANTS[1] = 0.407426;
CONSTANTS[2] = 10.4182;
break;
case 158:
CONSTANTS[1] = -0.744870;
CONSTANTS[2] = 10.6346;
break;
case 159:
CONSTANTS[1] = 1.53028;
CONSTANTS[2] = 11.0655;
break;
case 160:
CONSTANTS[1] = 0.528775;
CONSTANTS[2] = 9.55617;
break;
case 161:
CONSTANTS[1] = -0.262340;
CONSTANTS[2] = 9.50097;
break;
case 162:
CONSTANTS[1] = -0.939561;
CONSTANTS[2] = 9.96487;
break;
case 163:
CONSTANTS[1] = 0.572930;
CONSTANTS[2] = 9.85895;
break;
case 164:
CONSTANTS[1] = 0.787766;
CONSTANTS[2] = 10.3281;
break;
case 165:
CONSTANTS[1] = -0.788928;
CONSTANTS[2] = 10.0178;
break;
case 166:
CONSTANTS[1] = 1.20733;
CONSTANTS[2] = 10.5219;
break;
case 167:
CONSTANTS[1] = -1.26415;
CONSTANTS[2] = 8.58893;
break;
case 168:
CONSTANTS[1] = 0.423778;
CONSTANTS[2] = 11.3135;
break;
case 169:
CONSTANTS[1] = 1.05752;
CONSTANTS[2] = 10.6387;
break;
case 170:
CONSTANTS[1] = 1.18197;
CONSTANTS[2] = 10.6165;
break;
case 171:
CONSTANTS[1] = 0.404062;
CONSTANTS[2] = 9.90065;
break;
case 172:
CONSTANTS[1] = -0.0720385;
CONSTANTS[2] = 10.0298;
break;
case 173:
CONSTANTS[1] = 0.610309;
CONSTANTS[2] = 10.2138;
break;
case 174:
CONSTANTS[1] = 0.579604;
CONSTANTS[2] = 10.8719;
break;
case 175:
CONSTANTS[1] = -0.562992;
CONSTANTS[2] = 9.98981;
break;
case 176:
CONSTANTS[1] = -0.516771;
CONSTANTS[2] = 8.44386;
break;
case 177:
CONSTANTS[1] = -0.576560;
CONSTANTS[2] = 9.50838;
break;
case 178:
CONSTANTS[1] = -1.67534;
CONSTANTS[2] = 9.34918;
break;
case 179:
CONSTANTS[1] = -0.845814;
CONSTANTS[2] = 9.31171;
break;
case 180:
CONSTANTS[1] = 2.00670;
CONSTANTS[2] = 11.9930;
break;
case 181:
CONSTANTS[1] = 0.410152;
CONSTANTS[2] = 10.0540;
break;
case 182:
CONSTANTS[1] = -1.34287;
CONSTANTS[2] = 8.49243;
break;
case 183:
CONSTANTS[1] = 2.33060;
CONSTANTS[2] = 11.9768;
break;
case 184:
CONSTANTS[1] = 0.0960324;
CONSTANTS[2] = 10.0347;
break;
case 185:
CONSTANTS[1] = -0.519482;
CONSTANTS[2] = 10.0400;
break;
case 186:
CONSTANTS[1] = 0.267195;
CONSTANTS[2] = 9.65796;
break;
case 187:
CONSTANTS[1] = 0.350331;
CONSTANTS[2] = 10.2407;
break;
case 188:
CONSTANTS[1] = -0.345025;
CONSTANTS[2] = 9.81514;
break;
case 189:
CONSTANTS[1] = -0.658426;
CONSTANTS[2] = 10.2496;
break;
case 190:
CONSTANTS[1] = 0.560210;
CONSTANTS[2] = 9.82967;
break;
case 191:
CONSTANTS[1] = -0.478582;
CONSTANTS[2] = 9.35953;
break;
case 192:
CONSTANTS[1] = 0.0428047;
CONSTANTS[2] = 10.7671;
break;
case 193:
CONSTANTS[1] = -0.707723;
CONSTANTS[2] = 8.94048;
break;
case 194:
CONSTANTS[1] = 0.124503;
CONSTANTS[2] = 9.16413;
break;
case 195:
CONSTANTS[1] = -0.473808;
CONSTANTS[2] = 11.0087;
break;
case 196:
CONSTANTS[1] = 2.31470;
CONSTANTS[2] = 10.5245;
break;
case 197:
CONSTANTS[1] = -1.94560;
CONSTANTS[2] = 9.50325;
break;
case 198:
CONSTANTS[1] = -0.715881;
CONSTANTS[2] = 8.87352;
break;
case 199:
CONSTANTS[1] = -1.03730;
CONSTANTS[2] = 9.48532;
break;
case 200:
CONSTANTS[1] = -0.497264;
CONSTANTS[2] = 10.0143;
break;
case 201:
CONSTANTS[1] = -1.10151;
CONSTANTS[2] = 9.66712;
break;
case 202:
CONSTANTS[1] = 1.96568;
CONSTANTS[2] = 10.5761;
break;
case 203:
CONSTANTS[1] = 1.34832;
CONSTANTS[2] = 11.1216;
break;
case 204:
CONSTANTS[1] = -2.43467;
CONSTANTS[2] = 9.06855;
break;
case 205:
CONSTANTS[1] = -0.371626;
CONSTANTS[2] = 8.91190;
break;
case 206:
CONSTANTS[1] = -0.605104;
CONSTANTS[2] = 10.5041;
break;
case 207:
CONSTANTS[1] = 2.23979;
CONSTANTS[2] = 11.2441;
break;
case 208:
CONSTANTS[1] = 0.130584;
CONSTANTS[2] = 9.97309;
break;
case 209:
CONSTANTS[1] = -1.60687;
CONSTANTS[2] = 9.12180;
break;
case 210:
CONSTANTS[1] = -0.834117;
CONSTANTS[2] = 8.92628;
break;
case 211:
CONSTANTS[1] = -0.169646;
CONSTANTS[2] = 11.2638;
break;
case 212:
CONSTANTS[1] = -0.776054;
CONSTANTS[2] = 10.7188;
break;
case 213:
CONSTANTS[1] = -1.28258;
CONSTANTS[2] = 8.80018;
break;
case 214:
CONSTANTS[1] = 2.48610;
CONSTANTS[2] = 11.4168;
break;
case 215:
CONSTANTS[1] = -0.440228;
CONSTANTS[2] = 9.62551;
break;
case 216:
CONSTANTS[1] = -0.402632;
CONSTANTS[2] = 10.4416;
break;
case 217:
CONSTANTS[1] = -0.0529521;
CONSTANTS[2] = 10.6863;
break;
case 218:
CONSTANTS[1] = 0.618992;
CONSTANTS[2] = 10.7826;
break;
case 219:
CONSTANTS[1] = -0.300217;
CONSTANTS[2] = 10.4268;
break;
case 220:
CONSTANTS[1] = -0.552240;
CONSTANTS[2] = 10.0846;
break;
case 221:
CONSTANTS[1] = -0.983473;
CONSTANTS[2] = 9.54819;
break;
case 222:
CONSTANTS[1] = -0.720989;
CONSTANTS[2] = 9.76708;
break;
case 223:
CONSTANTS[1] = -0.313844;
CONSTANTS[2] = 9.40583;
break;
case 224:
CONSTANTS[1] = -0.114939;
CONSTANTS[2] = 10.0928;
break;
case 225:
CONSTANTS[1] = -0.0309663;
CONSTANTS[2] = 10.0549;
break;
case 226:
CONSTANTS[1] = 0.972894;
CONSTANTS[2] = 10.6256;
break;
case 227:
CONSTANTS[1] = -1.30449;
CONSTANTS[2] = 8.08119;
break;
case 228:
CONSTANTS[1] = -0.370204;
CONSTANTS[2] = 9.54435;
break;
case 229:
CONSTANTS[1] = -0.406109;
CONSTANTS[2] = 9.43918;
break;
case 230:
CONSTANTS[1] = -0.794491;
CONSTANTS[2] = 9.11377;
break;
case 231:
CONSTANTS[1] = 0.388608;
CONSTANTS[2] = 10.1221;
break;
case 232:
CONSTANTS[1] = 0.194060;
CONSTANTS[2] = 10.8202;
break;
case 233:
CONSTANTS[1] = -1.15400;
CONSTANTS[2] = 9.53673;
break;
case 234:
CONSTANTS[1] = 1.48574;
CONSTANTS[2] = 10.5935;
break;
case 235:
CONSTANTS[1] = 0.791562;
CONSTANTS[2] = 10.3585;
break;
case 236:
CONSTANTS[1] = 0.202493;
CONSTANTS[2] = 10.2991;
break;
case 237:
CONSTANTS[1] = 0.410946;
CONSTANTS[2] = 10.8463;
break;
case 238:
CONSTANTS[1] = 0.853777;
CONSTANTS[2] = 11.2058;
break;
case 239:
CONSTANTS[1] = -0.993165;
CONSTANTS[2] = 9.49733;
break;
case 240:
CONSTANTS[1] = -0.534665;
CONSTANTS[2] = 10.3356;
break;
case 241:
CONSTANTS[1] = -0.237033;
CONSTANTS[2] = 10.1159;
break;
case 242:
CONSTANTS[1] = -0.482345;
CONSTANTS[2] = 9.41361;
break;
case 243:
CONSTANTS[1] = -0.638438;
CONSTANTS[2] = 9.35939;
break;
case 244:
CONSTANTS[1] = -1.64067;
CONSTANTS[2] = 8.60358;
break;
case 245:
CONSTANTS[1] = 0.215740;
CONSTANTS[2] = 9.68979;
break;
case 246:
CONSTANTS[1] = 0.433980;
CONSTANTS[2] = 10.2781;
break;
case 247:
CONSTANTS[1] = 1.17371;
CONSTANTS[2] = 9.78035;
break;
case 248:
CONSTANTS[1] = -1.01766;
CONSTANTS[2] = 10.5270;
break;
case 249:
CONSTANTS[1] = 1.58844;
CONSTANTS[2] = 11.1309;
break;
case 250:
CONSTANTS[1] = 1.20276;
CONSTANTS[2] = 10.7994;
break;
case 251:
CONSTANTS[1] = 0.524435;
CONSTANTS[2] = 10.1868;
break;
case 252:
CONSTANTS[1] = -1.32934;
CONSTANTS[2] = 10.0750;
break;
case 253:
CONSTANTS[1] = -1.01268;
CONSTANTS[2] = 9.78126;
break;
case 254:
CONSTANTS[1] = 1.40147;
CONSTANTS[2] = 10.8712;
break;
case 255:
CONSTANTS[1] = 0.704961;
CONSTANTS[2] = 9.97849;
break;
case 256:
CONSTANTS[1] = 0.242236;
CONSTANTS[2] = 10.1719;
break;
case 257:
CONSTANTS[1] = -0.117170;
CONSTANTS[2] = 9.95008;
break;
case 258:
CONSTANTS[1] = 1.73312;
CONSTANTS[2] = 11.0988;
break;
case 259:
CONSTANTS[1] = 0.271606;
CONSTANTS[2] = 9.93099;
break;
case 260:
CONSTANTS[1] = -0.905859;
CONSTANTS[2] = 10.3144;
break;
case 261:
CONSTANTS[1] = 0.175520;
CONSTANTS[2] = 9.80816;
break;
case 262:
CONSTANTS[1] = 0.412280;
CONSTANTS[2] = 9.93709;
break;
case 263:
CONSTANTS[1] = 2.99631;
CONSTANTS[2] = 12.0821;
break;
case 264:
CONSTANTS[1] = -3.01763;
CONSTANTS[2] = 8.33325;
break;
case 265:
CONSTANTS[1] = -0.727385;
CONSTANTS[2] = 8.99378;
break;
case 266:
CONSTANTS[1] = 0.0157086;
CONSTANTS[2] = 10.5865;
break;
case 267:
CONSTANTS[1] = -0.286581;
CONSTANTS[2] = 10.7517;
break;
case 268:
CONSTANTS[1] = 0.938769;
CONSTANTS[2] = 10.3930;
break;
case 269:
CONSTANTS[1] = 0.723831;
CONSTANTS[2] = 9.78356;
break;
case 270:
CONSTANTS[1] = 0.830976;
CONSTANTS[2] = 10.8575;
break;
case 271:
CONSTANTS[1] = -0.704115;
CONSTANTS[2] = 9.45562;
break;
case 272:
CONSTANTS[1] = 0.408444;
CONSTANTS[2] = 10.4821;
break;
case 273:
CONSTANTS[1] = -0.486152;
CONSTANTS[2] = 9.38259;
break;
case 274:
CONSTANTS[1] = 1.39911;
CONSTANTS[2] = 10.6266;
break;
case 275:
CONSTANTS[1] = 0.732713;
CONSTANTS[2] = 10.2012;
break;
case 276:
CONSTANTS[1] = -0.00993686;
CONSTANTS[2] = 9.69335;
break;
case 277:
CONSTANTS[1] = -0.869886;
CONSTANTS[2] = 10.0473;
break;
case 278:
CONSTANTS[1] = 0.709321;
CONSTANTS[2] = 10.8147;
break;
case 279:
CONSTANTS[1] = -0.739368;
CONSTANTS[2] = 9.05034;
break;
case 280:
CONSTANTS[1] = 1.12651;
CONSTANTS[2] = 10.4266;
break;
case 281:
CONSTANTS[1] = -0.824310;
CONSTANTS[2] = 9.01544;
break;
case 282:
CONSTANTS[1] = -0.681181;
CONSTANTS[2] = 9.33498;
break;
case 283:
CONSTANTS[1] = -0.814640;
CONSTANTS[2] = 9.32083;
break;
case 284:
CONSTANTS[1] = 0.0828610;
CONSTANTS[2] = 10.1460;
break;
case 285:
CONSTANTS[1] = -0.635449;
CONSTANTS[2] = 10.3652;
break;
case 286:
CONSTANTS[1] = 1.06753;
CONSTANTS[2] = 10.9575;
break;
case 287:
CONSTANTS[1] = -0.724906;
CONSTANTS[2] = 9.25978;
break;
case 288:
CONSTANTS[1] = 1.25197;
CONSTANTS[2] = 10.0278;
break;
case 289:
CONSTANTS[1] = -0.364233;
CONSTANTS[2] = 10.5406;
break;
case 290:
CONSTANTS[1] = -0.990632;
CONSTANTS[2] = 9.18209;
break;
case 291:
CONSTANTS[1] = 0.741541;
CONSTANTS[2] = 10.6190;
break;
case 292:
CONSTANTS[1] = -0.354966;
CONSTANTS[2] = 9.84978;
break;
case 293:
CONSTANTS[1] = -1.14659;
CONSTANTS[2] = 10.4718;
break;
case 294:
CONSTANTS[1] = -0.236549;
CONSTANTS[2] = 10.2378;
break;
case 295:
CONSTANTS[1] = -0.582285;
CONSTANTS[2] = 9.52304;
break;
case 296:
CONSTANTS[1] = 1.44766;
CONSTANTS[2] = 10.3251;
break;
case 297:
CONSTANTS[1] = 0.728650;
CONSTANTS[2] = 9.74946;
break;
case 298:
CONSTANTS[1] = 0.914210;
CONSTANTS[2] = 10.6848;
break;
case 299:
CONSTANTS[1] = -0.356111;
CONSTANTS[2] = 9.20544;
break;
case 300:
CONSTANTS[1] = 1.01582;
CONSTANTS[2] = 10.6114;
break;
case 301:
CONSTANTS[1] = -0.361836;
CONSTANTS[2] = 9.57746;
break;
case 302:
CONSTANTS[1] = -0.0843881;
CONSTANTS[2] = 9.80090;
break;
case 303:
CONSTANTS[1] = -1.27522;
CONSTANTS[2] = 9.74307;
break;
case 304:
CONSTANTS[1] = 0.229748;
CONSTANTS[2] = 9.15077;
break;
case 305:
CONSTANTS[1] = 2.07026;
CONSTANTS[2] = 10.4920;
break;
case 306:
CONSTANTS[1] = 0.246223;
CONSTANTS[2] = 9.31843;
break;
case 307:
CONSTANTS[1] = -0.732375;
CONSTANTS[2] = 9.91271;
break;
case 308:
CONSTANTS[1] = -1.30478;
CONSTANTS[2] = 9.04656;
break;
case 309:
CONSTANTS[1] = -0.978047;
CONSTANTS[2] = 9.66303;
break;
case 310:
CONSTANTS[1] = -0.304513;
CONSTANTS[2] = 10.0827;
break;
case 311:
CONSTANTS[1] = -0.150403;
CONSTANTS[2] = 9.56690;
break;
case 312:
CONSTANTS[1] = -1.54318;
CONSTANTS[2] = 8.35636;
break;
case 313:
CONSTANTS[1] = -0.993495;
CONSTANTS[2] = 10.0807;
break;
case 314:
CONSTANTS[1] = -0.183647;
CONSTANTS[2] = 9.89752;
break;
case 315:
CONSTANTS[1] = 0.249767;
CONSTANTS[2] = 9.97534;
break;
case 316:
CONSTANTS[1] = 1.46905;
CONSTANTS[2] = 11.3046;
break;
case 317:
CONSTANTS[1] = 0.610601;
CONSTANTS[2] = 10.2472;
break;
case 318:
CONSTANTS[1] = -0.198613;
CONSTANTS[2] = 9.78133;
break;
case 319:
CONSTANTS[1] = 0.775906;
CONSTANTS[2] = 11.5253;
break;
case 320:
CONSTANTS[1] = -1.78648;
CONSTANTS[2] = 9.66713;
break;
case 321:
CONSTANTS[1] = -0.496963;
CONSTANTS[2] = 9.61829;
break;
case 322:
CONSTANTS[1] = -0.100650;
CONSTANTS[2] = 10.7001;
break;
case 323:
CONSTANTS[1] = -0.944006;
CONSTANTS[2] = 9.89879;
break;
case 324:
CONSTANTS[1] = 0.0404770;
CONSTANTS[2] = 9.41798;
break;
case 325:
CONSTANTS[1] = -0.0147684;
CONSTANTS[2] = 9.53036;
break;
case 326:
CONSTANTS[1] = 0.0887674;
CONSTANTS[2] = 10.1302;
break;
case 327:
CONSTANTS[1] = 0.782595;
CONSTANTS[2] = 9.88518;
break;
case 328:
CONSTANTS[1] = -0.489841;
CONSTANTS[2] = 9.65013;
break;
case 329:
CONSTANTS[1] = -0.322764;
CONSTANTS[2] = 9.05272;
break;
case 330:
CONSTANTS[1] = -0.354850;
CONSTANTS[2] = 10.4625;
break;
case 331:
CONSTANTS[1] = 1.08722;
CONSTANTS[2] = 10.6515;
break;
case 332:
CONSTANTS[1] = 0.257476;
CONSTANTS[2] = 9.85615;
break;
case 333:
CONSTANTS[1] = -0.730117;
CONSTANTS[2] = 9.69705;
break;
case 334:
CONSTANTS[1] = 0.315660;
CONSTANTS[2] = 10.5815;
break;
case 335:
CONSTANTS[1] = 0.0507933;
CONSTANTS[2] = 9.36234;
break;
case 336:
CONSTANTS[1] = 0.0895123;
CONSTANTS[2] = 10.4958;
break;
case 337:
CONSTANTS[1] = 0.507961;
CONSTANTS[2] = 9.42056;
break;
case 338:
CONSTANTS[1] = 2.32480;
CONSTANTS[2] = 11.2680;
break;
case 339:
CONSTANTS[1] = 0.701741;
CONSTANTS[2] = 9.86404;
break;
case 340:
CONSTANTS[1] = -0.830073;
CONSTANTS[2] = 10.1170;
break;
case 341:
CONSTANTS[1] = 0.806094;
CONSTANTS[2] = 9.95862;
break;
case 342:
CONSTANTS[1] = 0.399277;
CONSTANTS[2] = 10.0397;
break;
case 343:
CONSTANTS[1] = 1.33871;
CONSTANTS[2] = 10.4454;
break;
case 344:
CONSTANTS[1] = -0.257457;
CONSTANTS[2] = 9.75569;
break;
case 345:
CONSTANTS[1] = 0.152115;
CONSTANTS[2] = 9.71261;
break;
case 346:
CONSTANTS[1] = 0.151250;
CONSTANTS[2] = 10.1712;
break;
case 347:
CONSTANTS[1] = 0.591541;
CONSTANTS[2] = 11.1063;
break;
case 348:
CONSTANTS[1] = 1.37580;
CONSTANTS[2] = 10.2575;
break;
case 349:
CONSTANTS[1] = 0.0823245;
CONSTANTS[2] = 9.75813;
break;
case 350:
CONSTANTS[1] = 0.0215586;
CONSTANTS[2] = 10.0095;
break;
case 351:
CONSTANTS[1] = -0.208262;
CONSTANTS[2] = 9.70960;
break;
case 352:
CONSTANTS[1] = -0.0454611;
CONSTANTS[2] = 10.0737;
break;
case 353:
CONSTANTS[1] = -0.181892;
CONSTANTS[2] = 10.5023;
break;
case 354:
CONSTANTS[1] = 0.956651;
CONSTANTS[2] = 10.8997;
break;
case 355:
CONSTANTS[1] = -0.432014;
CONSTANTS[2] = 9.23958;
break;
case 356:
CONSTANTS[1] = 1.12978;
CONSTANTS[2] = 10.5368;
break;
case 357:
CONSTANTS[1] = -0.524785;
CONSTANTS[2] = 8.92056;
break;
case 358:
CONSTANTS[1] = -0.961467;
CONSTANTS[2] = 9.47764;
break;
case 359:
CONSTANTS[1] = -0.902691;
CONSTANTS[2] = 9.24721;
break;
case 360:
CONSTANTS[1] = 0.550728;
CONSTANTS[2] = 10.8911;
break;
case 361:
CONSTANTS[1] = -0.334659;
CONSTANTS[2] = 9.95086;
break;
case 362:
CONSTANTS[1] = 0.923508;
CONSTANTS[2] = 9.97101;
break;
case 363:
CONSTANTS[1] = 1.23269;
CONSTANTS[2] = 10.7724;
break;
case 364:
CONSTANTS[1] = 0.448378;
CONSTANTS[2] = 10.3599;
break;
case 365:
CONSTANTS[1] = 1.12161;
CONSTANTS[2] = 10.6718;
break;
case 366:
CONSTANTS[1] = -0.974888;
CONSTANTS[2] = 9.05492;
break;
case 367:
CONSTANTS[1] = 0.226185;
CONSTANTS[2] = 10.4324;
break;
case 368:
CONSTANTS[1] = 0.546751;
CONSTANTS[2] = 11.2115;
break;
case 369:
CONSTANTS[1] = -0.344231;
CONSTANTS[2] = 9.62057;
break;
case 370:
CONSTANTS[1] = -1.21701;
CONSTANTS[2] = 9.07301;
break;
case 371:
CONSTANTS[1] = 0.0575525;
CONSTANTS[2] = 10.1561;
break;
case 372:
CONSTANTS[1] = -0.104721;
CONSTANTS[2] = 10.3032;
break;
case 373:
CONSTANTS[1] = 0.636903;
CONSTANTS[2] = 10.3563;
break;
case 374:
CONSTANTS[1] = 0.664935;
CONSTANTS[2] = 10.9770;
break;
case 375:
CONSTANTS[1] = -0.492528;
CONSTANTS[2] = 9.35389;
break;
case 376:
CONSTANTS[1] = -1.11057;
CONSTANTS[2] = 9.66904;
break;
case 377:
CONSTANTS[1] = 0.965508;
CONSTANTS[2] = 11.0908;
break;
case 378:
CONSTANTS[1] = -1.72447;
CONSTANTS[2] = 8.16322;
break;
case 379:
CONSTANTS[1] = -0.147483;
CONSTANTS[2] = 9.74561;
break;
case 380:
CONSTANTS[1] = 1.12219;
CONSTANTS[2] = 10.3062;
break;
case 381:
CONSTANTS[1] = 1.93998;
CONSTANTS[2] = 11.0952;
break;
case 382:
CONSTANTS[1] = -1.29926;
CONSTANTS[2] = 8.77189;
break;
case 383:
CONSTANTS[1] = -0.199801;
CONSTANTS[2] = 9.37340;
break;
case 384:
CONSTANTS[1] = 0.585881;
CONSTANTS[2] = 10.1899;
break;
case 385:
CONSTANTS[1] = -0.850936;
CONSTANTS[2] = 9.99370;
break;
case 386:
CONSTANTS[1] = -0.407387;
CONSTANTS[2] = 9.94165;
break;
case 387:
CONSTANTS[1] = 0.715235;
CONSTANTS[2] = 10.5723;
break;
case 388:
CONSTANTS[1] = 0.546907;
CONSTANTS[2] = 10.8288;
break;
case 389:
CONSTANTS[1] = 0.683153;
CONSTANTS[2] = 10.4300;
break;
case 390:
CONSTANTS[1] = 1.17422;
CONSTANTS[2] = 10.4537;
break;
case 391:
CONSTANTS[1] = 1.61563;
CONSTANTS[2] = 10.5203;
break;
case 392:
CONSTANTS[1] = 0.935109;
CONSTANTS[2] = 10.5779;
break;
case 393:
CONSTANTS[1] = -0.376379;
CONSTANTS[2] = 9.67100;
break;
case 394:
CONSTANTS[1] = 0.412832;
CONSTANTS[2] = 10.6364;
break;
case 395:
CONSTANTS[1] = 1.18668;
CONSTANTS[2] = 9.58270;
break;
case 396:
CONSTANTS[1] = 0.888008;
CONSTANTS[2] = 9.31945;
break;
case 397:
CONSTANTS[1] = -1.21579;
CONSTANTS[2] = 9.32189;
break;
case 398:
CONSTANTS[1] = -0.965409;
CONSTANTS[2] = 9.34154;
break;
case 399:
CONSTANTS[1] = 0.351405;
CONSTANTS[2] = 10.5833;
break;
case 400:
CONSTANTS[1] = 1.27509;
CONSTANTS[2] = 10.1668;
break;
case 401:
CONSTANTS[1] = 2.27365;
CONSTANTS[2] = 12.2267;
break;
case 402:
CONSTANTS[1] = -0.346561;
CONSTANTS[2] = 9.38031;
break;
case 403:
CONSTANTS[1] = 0.539864;
CONSTANTS[2] = 10.3647;
break;
case 404:
CONSTANTS[1] = 3.27730;
CONSTANTS[2] = 11.6012;
break;
case 405:
CONSTANTS[1] = -0.817065;
CONSTANTS[2] = 9.77023;
break;
case 406:
CONSTANTS[1] = 1.65362;
CONSTANTS[2] = 10.2156;
break;
case 407:
CONSTANTS[1] = -1.75359;
CONSTANTS[2] = 8.66985;
break;
case 408:
CONSTANTS[1] = -1.49925;
CONSTANTS[2] = 9.47209;
break;
case 409:
CONSTANTS[1] = 0.111396;
CONSTANTS[2] = 10.3601;
break;
case 410:
CONSTANTS[1] = -0.0907533;
CONSTANTS[2] = 9.41955;
break;
case 411:
CONSTANTS[1] = -0.663116;
CONSTANTS[2] = 9.64686;
break;
case 412:
CONSTANTS[1] = -1.57689;
CONSTANTS[2] = 9.20958;
break;
case 413:
CONSTANTS[1] = 1.05311;
CONSTANTS[2] = 10.4465;
break;
case 414:
CONSTANTS[1] = -0.486301;
CONSTANTS[2] = 9.91196;
break;
case 415:
CONSTANTS[1] = -1.12938;
CONSTANTS[2] = 9.08112;
break;
case 416:
CONSTANTS[1] = 0.646075;
CONSTANTS[2] = 10.1921;
break;
case 417:
CONSTANTS[1] = -0.704567;
CONSTANTS[2] = 9.66739;
break;
case 418:
CONSTANTS[1] = -0.752939;
CONSTANTS[2] = 9.26568;
break;
case 419:
CONSTANTS[1] = 0.899789;
CONSTANTS[2] = 8.62899;
break;
case 420:
CONSTANTS[1] = 1.43674;
CONSTANTS[2] = 11.1109;
break;
case 421:
CONSTANTS[1] = 2.35949;
CONSTANTS[2] = 11.9758;
break;
case 422:
CONSTANTS[1] = 0.0929581;
CONSTANTS[2] = 9.42542;
break;
case 423:
CONSTANTS[1] = 1.03410;
CONSTANTS[2] = 10.1089;
break;
case 424:
CONSTANTS[1] = -1.21878;
CONSTANTS[2] = 9.16772;
break;
case 425:
CONSTANTS[1] = 0.351029;
CONSTANTS[2] = 9.65057;
break;
case 426:
CONSTANTS[1] = -1.43901;
CONSTANTS[2] = 9.17139;
break;
case 427:
CONSTANTS[1] = 0.586937;
CONSTANTS[2] = 10.7464;
break;
case 428:
CONSTANTS[1] = -1.61676;
CONSTANTS[2] = 9.55986;
break;
case 429:
CONSTANTS[1] = -1.21429;
CONSTANTS[2] = 9.33483;
break;
case 430:
CONSTANTS[1] = -0.908345;
CONSTANTS[2] = 9.42696;
break;
case 431:
CONSTANTS[1] = -0.280592;
CONSTANTS[2] = 10.0751;
break;
case 432:
CONSTANTS[1] = -0.653744;
CONSTANTS[2] = 11.1481;
break;
case 433:
CONSTANTS[1] = -0.769794;
CONSTANTS[2] = 10.1001;
break;
case 434:
CONSTANTS[1] = -0.856532;
CONSTANTS[2] = 9.29220;
break;
case 435:
CONSTANTS[1] = -0.280135;
CONSTANTS[2] = 10.8181;
break;
case 436:
CONSTANTS[1] = 2.00824;
CONSTANTS[2] = 11.1645;
break;
case 437:
CONSTANTS[1] = 0.786302;
CONSTANTS[2] = 10.7160;
break;
case 438:
CONSTANTS[1] = 0.875345;
CONSTANTS[2] = 10.1950;
break;
case 439:
CONSTANTS[1] = -0.0405551;
CONSTANTS[2] = 10.6112;
break;
case 440:
CONSTANTS[1] = 0.377342;
CONSTANTS[2] = 10.4677;
break;
case 441:
CONSTANTS[1] = 0.944166;
CONSTANTS[2] = 9.62152;
break;
case 442:
CONSTANTS[1] = 0.503640;
CONSTANTS[2] = 10.0450;
break;
case 443:
CONSTANTS[1] = -0.942394;
CONSTANTS[2] = 9.78721;
break;
case 444:
CONSTANTS[1] = 1.24247;
CONSTANTS[2] = 10.7395;
break;
case 445:
CONSTANTS[1] = -1.64484;
CONSTANTS[2] = 9.04132;
break;
case 446:
CONSTANTS[1] = 0.800602;
CONSTANTS[2] = 10.5586;
break;
case 447:
CONSTANTS[1] = 1.72646;
CONSTANTS[2] = 11.9048;
break;
case 448:
CONSTANTS[1] = 0.167087;
CONSTANTS[2] = 10.0911;
break;
case 449:
CONSTANTS[1] = 0.195967;
CONSTANTS[2] = 10.1663;
break;
case 450:
CONSTANTS[1] = 0.601147;
CONSTANTS[2] = 11.4331;
break;
case 451:
CONSTANTS[1] = -0.959949;
CONSTANTS[2] = 9.88850;
break;
case 452:
CONSTANTS[1] = 1.89912;
CONSTANTS[2] = 11.1025;
break;
case 453:
CONSTANTS[1] = 0.508026;
CONSTANTS[2] = 10.2076;
break;
case 454:
CONSTANTS[1] = -1.15255;
CONSTANTS[2] = 9.43635;
break;
case 455:
CONSTANTS[1] = 1.88267;
CONSTANTS[2] = 10.8683;
break;
case 456:
CONSTANTS[1] = -0.124029;
CONSTANTS[2] = 9.58333;
break;
case 457:
CONSTANTS[1] = -1.53235;
CONSTANTS[2] = 9.24947;
break;
case 458:
CONSTANTS[1] = 3.69761;
CONSTANTS[2] = 13.0441;
break;
case 459:
CONSTANTS[1] = -0.826165;
CONSTANTS[2] = 10.5962;
break;
case 460:
CONSTANTS[1] = 0.464457;
CONSTANTS[2] = 9.76980;
break;
case 461:
CONSTANTS[1] = -1.10147;
CONSTANTS[2] = 9.47615;
break;
case 462:
CONSTANTS[1] = 0.0354406;
CONSTANTS[2] = 9.90579;
break;
case 463:
CONSTANTS[1] = 0.671877;
CONSTANTS[2] = 11.2018;
break;
case 464:
CONSTANTS[1] = -0.507594;
CONSTANTS[2] = 10.6321;
break;
case 465:
CONSTANTS[1] = -1.06747;
CONSTANTS[2] = 10.1960;
break;
case 466:
CONSTANTS[1] = 0.363329;
CONSTANTS[2] = 10.4891;
break;
case 467:
CONSTANTS[1] = 1.62710;
CONSTANTS[2] = 11.0831;
break;
case 468:
CONSTANTS[1] = -0.440007;
CONSTANTS[2] = 9.48834;
break;
case 469:
CONSTANTS[1] = -1.05077;
CONSTANTS[2] = 8.95881;
break;
case 470:
CONSTANTS[1] = -0.727333;
CONSTANTS[2] = 9.52694;
break;
case 471:
CONSTANTS[1] = -0.661971;
CONSTANTS[2] = 9.16460;
break;
case 472:
CONSTANTS[1] = -0.657821;
CONSTANTS[2] = 9.39564;
break;
case 473:
CONSTANTS[1] = -1.21630;
CONSTANTS[2] = 9.64805;
break;
case 474:
CONSTANTS[1] = 0.453523;
CONSTANTS[2] = 9.85691;
break;
case 475:
CONSTANTS[1] = 0.665175;
CONSTANTS[2] = 10.4098;
break;
case 476:
CONSTANTS[1] = -0.690009;
CONSTANTS[2] = 10.3589;
break;
case 477:
CONSTANTS[1] = 0.373612;
CONSTANTS[2] = 10.3439;
break;
case 478:
CONSTANTS[1] = 0.132482;
CONSTANTS[2] = 10.1983;
break;
case 479:
CONSTANTS[1] = -1.96359;
CONSTANTS[2] = 9.21284;
break;
case 480:
CONSTANTS[1] = 0.165573;
CONSTANTS[2] = 10.1481;
break;
case 481:
CONSTANTS[1] = -0.945926;
CONSTANTS[2] = 9.28928;
break;
case 482:
CONSTANTS[1] = 0.783164;
CONSTANTS[2] = 9.99796;
break;
case 483:
CONSTANTS[1] = -0.0488695;
CONSTANTS[2] = 9.94085;
break;
case 484:
CONSTANTS[1] = 0.384383;
CONSTANTS[2] = 10.6421;
break;
case 485:
CONSTANTS[1] = 0.123760;
CONSTANTS[2] = 10.3180;
break;
case 486:
CONSTANTS[1] = 0.370242;
CONSTANTS[2] = 10.1938;
break;
case 487:
CONSTANTS[1] = 0.768281;
CONSTANTS[2] = 11.0901;
break;
case 488:
CONSTANTS[1] = 1.11593;
CONSTANTS[2] = 11.0530;
break;
case 489:
CONSTANTS[1] = -0.820506;
CONSTANTS[2] = 9.99933;
break;
case 490:
CONSTANTS[1] = -1.83524;
CONSTANTS[2] = 9.74197;
break;
case 491:
CONSTANTS[1] = 1.24009;
CONSTANTS[2] = 9.53807;
break;
case 492:
CONSTANTS[1] = -0.664850;
CONSTANTS[2] = 9.35963;
break;
case 493:
CONSTANTS[1] = 0.512790;
CONSTANTS[2] = 9.39523;
break;
case 494:
CONSTANTS[1] = 0.643407;
CONSTANTS[2] = 10.6516;
break;
case 495:
CONSTANTS[1] = -1.13597;
CONSTANTS[2] = 9.59286;
break;
case 496:
CONSTANTS[1] = -1.13684;
CONSTANTS[2] = 10.0510;
break;
case 497:
CONSTANTS[1] = -0.158594;
CONSTANTS[2] = 9.24374;
break;
case 498:
CONSTANTS[1] = 0.885642;
CONSTANTS[2] = 10.3171;
break;
case 499:
CONSTANTS[1] = 0.314444;
CONSTANTS[2] = 10.4585;
break;
case 500:
CONSTANTS[1] = -0.475038;
CONSTANTS[2] = 9.35973;
break;
case 501:
CONSTANTS[1] = -0.987897;
CONSTANTS[2] = 9.32515;
break;
case 502:
CONSTANTS[1] = 1.23845;
CONSTANTS[2] = 11.1708;
break;
case 503:
CONSTANTS[1] = 0.511501;
CONSTANTS[2] = 10.2405;
break;
case 504:
CONSTANTS[1] = 1.33582;
CONSTANTS[2] = 11.0772;
break;
case 505:
CONSTANTS[1] = -0.400080;
CONSTANTS[2] = 10.0432;
break;
case 506:
CONSTANTS[1] = -0.622970;
CONSTANTS[2] = 9.17166;
break;
case 507:
CONSTANTS[1] = -0.340851;
CONSTANTS[2] = 9.00939;
break;
case 508:
CONSTANTS[1] = 0.438369;
CONSTANTS[2] = 10.1818;
break;
case 509:
CONSTANTS[1] = -1.90100;
CONSTANTS[2] = 8.81728;
break;
case 510:
CONSTANTS[1] = 0.215920;
CONSTANTS[2] = 9.34201;
break;
case 511:
CONSTANTS[1] = -0.230859;
CONSTANTS[2] = 10.2719;
break;
case 512:
CONSTANTS[1] = 0.156791;
CONSTANTS[2] = 9.87068;
break;
case 513:
CONSTANTS[1] = 1.17138;
CONSTANTS[2] = 10.7298;
break;
case 514:
CONSTANTS[1] = -1.26034;
CONSTANTS[2] = 10.1679;
break;
case 515:
CONSTANTS[1] = -0.479767;
CONSTANTS[2] = 9.89906;
break;
case 516:
CONSTANTS[1] = 1.52862;
CONSTANTS[2] = 10.7851;
break;
case 517:
CONSTANTS[1] = 1.49318;
CONSTANTS[2] = 10.6488;
break;
case 518:
CONSTANTS[1] = 1.32177;
CONSTANTS[2] = 10.3616;
break;
case 519:
CONSTANTS[1] = -0.0423543;
CONSTANTS[2] = 9.80688;
break;
case 520:
CONSTANTS[1] = -1.24957;
CONSTANTS[2] = 9.20051;
break;
case 521:
CONSTANTS[1] = 2.14020;
CONSTANTS[2] = 11.0278;
break;
case 522:
CONSTANTS[1] = 1.02965;
CONSTANTS[2] = 10.1165;
break;
case 523:
CONSTANTS[1] = -0.210068;
CONSTANTS[2] = 9.39555;
break;
case 524:
CONSTANTS[1] = -0.423401;
CONSTANTS[2] = 9.97163;
break;
case 525:
CONSTANTS[1] = 1.33140;
CONSTANTS[2] = 10.3506;
break;
case 526:
CONSTANTS[1] = -1.99508;
CONSTANTS[2] = 9.16234;
break;
case 527:
CONSTANTS[1] = -0.969015;
CONSTANTS[2] = 9.15691;
break;
case 528:
CONSTANTS[1] = -1.46154;
CONSTANTS[2] = 9.19254;
break;
case 529:
CONSTANTS[1] = 1.49764;
CONSTANTS[2] = 9.97038;
break;
case 530:
CONSTANTS[1] = 0.113081;
CONSTANTS[2] = 9.83325;
break;
case 531:
CONSTANTS[1] = -1.42146;
CONSTANTS[2] = 9.06825;
break;
case 532:
CONSTANTS[1] = -1.58281;
CONSTANTS[2] = 9.49332;
break;
case 533:
CONSTANTS[1] = 1.26386;
CONSTANTS[2] = 10.3359;
break;
case 534:
CONSTANTS[1] = 1.34212;
CONSTANTS[2] = 10.3533;
break;
case 535:
CONSTANTS[1] = -1.32330;
CONSTANTS[2] = 8.96032;
break;
case 536:
CONSTANTS[1] = 0.329914;
CONSTANTS[2] = 9.78799;
break;
case 537:
CONSTANTS[1] = -1.21327;
CONSTANTS[2] = 9.55096;
break;
case 538:
CONSTANTS[1] = -0.0303879;
CONSTANTS[2] = 9.75971;
break;
case 539:
CONSTANTS[1] = 1.06210;
CONSTANTS[2] = 10.3017;
break;
case 540:
CONSTANTS[1] = -0.808944;
CONSTANTS[2] = 8.86237;
break;
case 541:
CONSTANTS[1] = 0.352956;
CONSTANTS[2] = 10.3568;
break;
case 542:
CONSTANTS[1] = -0.870592;
CONSTANTS[2] = 10.4506;
break;
case 543:
CONSTANTS[1] = 1.05224;
CONSTANTS[2] = 10.4802;
break;
case 544:
CONSTANTS[1] = -0.0797880;
CONSTANTS[2] = 10.7142;
break;
case 545:
CONSTANTS[1] = -0.946376;
CONSTANTS[2] = 10.4784;
break;
case 546:
CONSTANTS[1] = 1.99960;
CONSTANTS[2] = 10.1019;
break;
case 547:
CONSTANTS[1] = -0.196236;
CONSTANTS[2] = 10.5751;
break;
case 548:
CONSTANTS[1] = 1.28829;
CONSTANTS[2] = 10.9514;
break;
case 549:
CONSTANTS[1] = 0.0823696;
CONSTANTS[2] = 10.3527;
break;
case 550:
CONSTANTS[1] = -0.268091;
CONSTANTS[2] = 9.70354;
break;
case 551:
CONSTANTS[1] = 0.901731;
CONSTANTS[2] = 10.7756;
break;
case 552:
CONSTANTS[1] = 1.28032;
CONSTANTS[2] = 11.5576;
break;
case 553:
CONSTANTS[1] = -1.68936;
CONSTANTS[2] = 9.48078;
break;
case 554:
CONSTANTS[1] = -0.571127;
CONSTANTS[2] = 9.71225;
break;
case 555:
CONSTANTS[1] = 1.71868;
CONSTANTS[2] = 10.7382;
break;
case 556:
CONSTANTS[1] = 0.175661;
CONSTANTS[2] = 9.76734;
break;
case 557:
CONSTANTS[1] = -0.185540;
CONSTANTS[2] = 9.12749;
break;
case 558:
CONSTANTS[1] = -0.346409;
CONSTANTS[2] = 9.29452;
break;
case 559:
CONSTANTS[1] = -0.342999;
CONSTANTS[2] = 9.97823;
break;
case 560:
CONSTANTS[1] = 0.737461;
CONSTANTS[2] = 11.3272;
break;
case 561:
CONSTANTS[1] = -0.504493;
CONSTANTS[2] = 10.7028;
break;
case 562:
CONSTANTS[1] = 0.523573;
CONSTANTS[2] = 10.6462;
break;
case 563:
CONSTANTS[1] = -0.309610;
CONSTANTS[2] = 9.99799;
break;
case 564:
CONSTANTS[1] = -0.881047;
CONSTANTS[2] = 9.55033;
break;
case 565:
CONSTANTS[1] = -0.494034;
CONSTANTS[2] = 8.70862;
break;
case 566:
CONSTANTS[1] = -0.582777;
CONSTANTS[2] = 9.61434;
break;
case 567:
CONSTANTS[1] = 0.544494;
CONSTANTS[2] = 10.3399;
break;
case 568:
CONSTANTS[1] = -1.98849;
CONSTANTS[2] = 8.82237;
break;
case 569:
CONSTANTS[1] = 0.555495;
CONSTANTS[2] = 10.7356;
break;
case 570:
CONSTANTS[1] = 0.802086;
CONSTANTS[2] = 10.3418;
break;
case 571:
CONSTANTS[1] = 0.372985;
CONSTANTS[2] = 10.4661;
break;
case 572:
CONSTANTS[1] = -1.85823;
CONSTANTS[2] = 9.69814;
break;
case 573:
CONSTANTS[1] = 0.879953;
CONSTANTS[2] = 10.2006;
break;
case 574:
CONSTANTS[1] = 1.53822;
CONSTANTS[2] = 9.71154;
break;
case 575:
CONSTANTS[1] = -2.86551;
CONSTANTS[2] = 8.61682;
break;
case 576:
CONSTANTS[1] = 1.46682;
CONSTANTS[2] = 10.3537;
break;
case 577:
CONSTANTS[1] = -1.24871;
CONSTANTS[2] = 9.46812;
break;
case 578:
CONSTANTS[1] = -0.747512;
CONSTANTS[2] = 9.52576;
break;
case 579:
CONSTANTS[1] = 1.22458;
CONSTANTS[2] = 10.2689;
break;
case 580:
CONSTANTS[1] = 0.562402;
CONSTANTS[2] = 10.5759;
break;
case 581:
CONSTANTS[1] = 0.183738;
CONSTANTS[2] = 10.4753;
break;
case 582:
CONSTANTS[1] = 0.670118;
CONSTANTS[2] = 9.63987;
break;
case 583:
CONSTANTS[1] = 1.67535;
CONSTANTS[2] = 11.1555;
break;
case 584:
CONSTANTS[1] = 1.33373;
CONSTANTS[2] = 10.6024;
break;
case 585:
CONSTANTS[1] = -0.774551;
CONSTANTS[2] = 9.65820;
break;
case 586:
CONSTANTS[1] = 1.25987;
CONSTANTS[2] = 10.6762;
break;
case 587:
CONSTANTS[1] = 1.85013;
CONSTANTS[2] = 10.5999;
break;
case 588:
CONSTANTS[1] = 0.427977;
CONSTANTS[2] = 10.0741;
break;
case 589:
CONSTANTS[1] = -0.542151;
CONSTANTS[2] = 10.4372;
break;
case 590:
CONSTANTS[1] = -0.274625;
CONSTANTS[2] = 9.52362;
break;
case 591:
CONSTANTS[1] = -0.317415;
CONSTANTS[2] = 9.83726;
break;
case 592:
CONSTANTS[1] = 0.674831;
CONSTANTS[2] = 10.7653;
break;
case 593:
CONSTANTS[1] = 0.677258;
CONSTANTS[2] = 9.89435;
break;
case 594:
CONSTANTS[1] = 0.716131;
CONSTANTS[2] = 10.0076;
break;
case 595:
CONSTANTS[1] = -0.858801;
CONSTANTS[2] = 8.92050;
break;
case 596:
CONSTANTS[1] = -0.101121;
CONSTANTS[2] = 9.49111;
break;
case 597:
CONSTANTS[1] = -0.239913;
CONSTANTS[2] = 10.3514;
break;
case 598:
CONSTANTS[1] = -0.477360;
CONSTANTS[2] = 10.6174;
break;
case 599:
CONSTANTS[1] = -0.738714;
CONSTANTS[2] = 9.22485;
break;
case 600:
CONSTANTS[1] = 0.00411387;
CONSTANTS[2] = 10.5929;
break;
case 601:
CONSTANTS[1] = -1.69666;
CONSTANTS[2] = 9.39686;
break;
case 602:
CONSTANTS[1] = -0.620922;
CONSTANTS[2] = 10.1151;
break;
case 603:
CONSTANTS[1] = -1.05565;
CONSTANTS[2] = 9.45448;
break;
case 604:
CONSTANTS[1] = 1.57039;
CONSTANTS[2] = 9.81664;
break;
case 605:
CONSTANTS[1] = -0.104777;
CONSTANTS[2] = 9.69525;
break;
case 606:
CONSTANTS[1] = -0.272459;
CONSTANTS[2] = 9.44754;
break;
case 607:
CONSTANTS[1] = -0.0577842;
CONSTANTS[2] = 10.2906;
break;
case 608:
CONSTANTS[1] = 0.505851;
CONSTANTS[2] = 10.3218;
break;
case 609:
CONSTANTS[1] = -0.399555;
CONSTANTS[2] = 9.81272;
break;
case 610:
CONSTANTS[1] = 0.924814;
CONSTANTS[2] = 11.2289;
break;
case 611:
CONSTANTS[1] = 0.966235;
CONSTANTS[2] = 10.4605;
break;
case 612:
CONSTANTS[1] = -0.806886;
CONSTANTS[2] = 9.05107;
break;
case 613:
CONSTANTS[1] = -0.497842;
CONSTANTS[2] = 9.27487;
break;
case 614:
CONSTANTS[1] = -0.413479;
CONSTANTS[2] = 10.0058;
break;
case 615:
CONSTANTS[1] = -0.985805;
CONSTANTS[2] = 8.72723;
break;
case 616:
CONSTANTS[1] = -1.59998;
CONSTANTS[2] = 9.39491;
break;
case 617:
CONSTANTS[1] = -0.404125;
CONSTANTS[2] = 9.50980;
break;
case 618:
CONSTANTS[1] = 0.445495;
CONSTANTS[2] = 10.2844;
break;
case 619:
CONSTANTS[1] = -0.909585;
CONSTANTS[2] = 9.22876;
break;
case 620:
CONSTANTS[1] = 0.0976493;
CONSTANTS[2] = 10.0220;
break;
case 621:
CONSTANTS[1] = -1.25973;
CONSTANTS[2] = 9.36721;
break;
case 622:
CONSTANTS[1] = 1.61795;
CONSTANTS[2] = 10.4501;
break;
case 623:
CONSTANTS[1] = 1.23804;
CONSTANTS[2] = 10.5586;
break;
case 624:
CONSTANTS[1] = -0.718394;
CONSTANTS[2] = 9.78457;
break;
case 625:
CONSTANTS[1] = -1.38174;
CONSTANTS[2] = 8.61673;
break;
case 626:
CONSTANTS[1] = 0.146050;
CONSTANTS[2] = 10.5760;
break;
case 627:
CONSTANTS[1] = -0.301388;
CONSTANTS[2] = 10.2475;
break;
case 628:
CONSTANTS[1] = 0.329288;
CONSTANTS[2] = 10.6663;
break;
case 629:
CONSTANTS[1] = -0.367443;
CONSTANTS[2] = 9.89087;
break;
case 630:
CONSTANTS[1] = -2.12423;
CONSTANTS[2] = 8.93525;
break;
case 631:
CONSTANTS[1] = 0.0621354;
CONSTANTS[2] = 9.77677;
break;
case 632:
CONSTANTS[1] = 0.157480;
CONSTANTS[2] = 10.7725;
break;
case 633:
CONSTANTS[1] = -0.0994025;
CONSTANTS[2] = 9.06774;
break;
case 634:
CONSTANTS[1] = -1.05937;
CONSTANTS[2] = 8.97319;
break;
case 635:
CONSTANTS[1] = 0.00701648;
CONSTANTS[2] = 9.69851;
break;
case 636:
CONSTANTS[1] = 0.665293;
CONSTANTS[2] = 9.72380;
break;
case 637:
CONSTANTS[1] = -1.98755;
CONSTANTS[2] = 9.94040;
break;
case 638:
CONSTANTS[1] = 0.0113768;
CONSTANTS[2] = 10.8783;
break;
case 639:
CONSTANTS[1] = 0.0453297;
CONSTANTS[2] = 10.7279;
break;
case 640:
CONSTANTS[1] = -1.26847;
CONSTANTS[2] = 10.2385;
break;
case 641:
CONSTANTS[1] = -0.135900;
CONSTANTS[2] = 10.2942;
break;
case 642:
CONSTANTS[1] = -0.320005;
CONSTANTS[2] = 9.86866;
break;
case 643:
CONSTANTS[1] = -1.17535;
CONSTANTS[2] = 9.50894;
break;
case 644:
CONSTANTS[1] = 0.0596427;
CONSTANTS[2] = 9.41408;
break;
case 645:
CONSTANTS[1] = -0.575592;
CONSTANTS[2] = 9.03367;
break;
case 646:
CONSTANTS[1] = -1.54249;
CONSTANTS[2] = 9.59057;
break;
case 647:
CONSTANTS[1] = -0.467236;
CONSTANTS[2] = 10.4076;
break;
case 648:
CONSTANTS[1] = -0.476990;
CONSTANTS[2] = 9.00160;
break;
case 649:
CONSTANTS[1] = -1.37371;
CONSTANTS[2] = 10.3046;
break;
case 650:
CONSTANTS[1] = -0.154625;
CONSTANTS[2] = 10.9566;
break;
case 651:
CONSTANTS[1] = 1.44285;
CONSTANTS[2] = 10.7695;
break;
case 652:
CONSTANTS[1] = -0.332722;
CONSTANTS[2] = 9.07131;
break;
case 653:
CONSTANTS[1] = 0.709138;
CONSTANTS[2] = 10.7518;
break;
case 654:
CONSTANTS[1] = 0.746652;
CONSTANTS[2] = 9.86163;
break;
case 655:
CONSTANTS[1] = 0.680933;
CONSTANTS[2] = 9.90815;
break;
case 656:
CONSTANTS[1] = 0.117772;
CONSTANTS[2] = 8.75121;
break;
case 657:
CONSTANTS[1] = 1.32237;
CONSTANTS[2] = 10.8637;
break;
case 658:
CONSTANTS[1] = -0.738373;
CONSTANTS[2] = 9.45378;
break;
case 659:
CONSTANTS[1] = 0.151820;
CONSTANTS[2] = 10.1513;
break;
case 660:
CONSTANTS[1] = -0.319313;
CONSTANTS[2] = 9.49655;
break;
case 661:
CONSTANTS[1] = 0.179654;
CONSTANTS[2] = 9.19866;
break;
case 662:
CONSTANTS[1] = 0.174360;
CONSTANTS[2] = 9.07776;
break;
case 663:
CONSTANTS[1] = 1.07812;
CONSTANTS[2] = 10.1251;
break;
case 664:
CONSTANTS[1] = 0.562038;
CONSTANTS[2] = 11.1739;
break;
case 665:
CONSTANTS[1] = 0.869652;
CONSTANTS[2] = 9.92537;
break;
case 666:
CONSTANTS[1] = 0.465189;
CONSTANTS[2] = 11.1155;
break;
case 667:
CONSTANTS[1] = -1.51759;
CONSTANTS[2] = 9.63016;
break;
case 668:
CONSTANTS[1] = -0.966909;
CONSTANTS[2] = 8.58672;
break;
case 669:
CONSTANTS[1] = -0.698523;
CONSTANTS[2] = 9.73816;
break;
case 670:
CONSTANTS[1] = -1.10229;
CONSTANTS[2] = 9.56694;
break;
case 671:
CONSTANTS[1] = 0.433178;
CONSTANTS[2] = 10.0252;
break;
case 672:
CONSTANTS[1] = 1.11356;
CONSTANTS[2] = 10.7501;
break;
case 673:
CONSTANTS[1] = -0.165635;
CONSTANTS[2] = 9.91605;
break;
case 674:
CONSTANTS[1] = 0.784282;
CONSTANTS[2] = 10.7656;
break;
case 675:
CONSTANTS[1] = -1.45336;
CONSTANTS[2] = 8.99533;
break;
case 676:
CONSTANTS[1] = 1.56549;
CONSTANTS[2] = 12.2716;
break;
case 677:
CONSTANTS[1] = -0.0288238;
CONSTANTS[2] = 10.0791;
break;
case 678:
CONSTANTS[1] = -0.594152;
CONSTANTS[2] = 9.34606;
break;
case 679:
CONSTANTS[1] = 0.849283;
CONSTANTS[2] = 10.8055;
break;
case 680:
CONSTANTS[1] = 0.170170;
CONSTANTS[2] = 10.0751;
break;
case 681:
CONSTANTS[1] = -0.715273;
CONSTANTS[2] = 10.7120;
break;
case 682:
CONSTANTS[1] = -0.753155;
CONSTANTS[2] = 10.1609;
break;
case 683:
CONSTANTS[1] = -0.381936;
CONSTANTS[2] = 10.1288;
break;
case 684:
CONSTANTS[1] = 0.117365;
CONSTANTS[2] = 10.7385;
break;
case 685:
CONSTANTS[1] = -0.0499983;
CONSTANTS[2] = 9.90393;
break;
case 686:
CONSTANTS[1] = 1.46231;
CONSTANTS[2] = 10.4129;
break;
case 687:
CONSTANTS[1] = 0.693661;
CONSTANTS[2] = 10.6085;
break;
case 688:
CONSTANTS[1] = -0.810199;
CONSTANTS[2] = 8.30520;
break;
case 689:
CONSTANTS[1] = 0.180981;
CONSTANTS[2] = 9.60121;
break;
case 690:
CONSTANTS[1] = -1.07555;
CONSTANTS[2] = 9.29245;
break;
case 691:
CONSTANTS[1] = 0.250373;
CONSTANTS[2] = 9.63609;
break;
case 692:
CONSTANTS[1] = -1.37192;
CONSTANTS[2] = 9.82880;
break;
case 693:
CONSTANTS[1] = -1.17980;
CONSTANTS[2] = 9.80105;
break;
case 694:
CONSTANTS[1] = -0.739278;
CONSTANTS[2] = 9.92105;
break;
case 695:
CONSTANTS[1] = 1.30972;
CONSTANTS[2] = 11.2621;
break;
case 696:
CONSTANTS[1] = -0.401853;
CONSTANTS[2] = 9.18155;
break;
case 697:
CONSTANTS[1] = -0.584644;
CONSTANTS[2] = 9.84614;
break;
case 698:
CONSTANTS[1] = 0.269095;
CONSTANTS[2] = 10.8294;
break;
case 699:
CONSTANTS[1] = 0.295820;
CONSTANTS[2] = 10.5321;
break;
case 700:
CONSTANTS[1] = 0.0459941;
CONSTANTS[2] = 10.6502;
break;
case 701:
CONSTANTS[1] = 0.0465433;
CONSTANTS[2] = 9.22829;
break;
case 702:
CONSTANTS[1] = 1.95760;
CONSTANTS[2] = 11.0755;
break;
case 703:
CONSTANTS[1] = 0.0925624;
CONSTANTS[2] = 9.75964;
break;
case 704:
CONSTANTS[1] = 2.54523;
CONSTANTS[2] = 10.7614;
break;
case 705:
CONSTANTS[1] = -0.713451;
CONSTANTS[2] = 10.1387;
break;
case 706:
CONSTANTS[1] = -0.874198;
CONSTANTS[2] = 10.3242;
break;
case 707:
CONSTANTS[1] = 1.70470;
CONSTANTS[2] = 11.1288;
break;
case 708:
CONSTANTS[1] = 0.0643534;
CONSTANTS[2] = 10.1860;
break;
case 709:
CONSTANTS[1] = -1.43201;
CONSTANTS[2] = 8.80782;
break;
case 710:
CONSTANTS[1] = 0.519043;
CONSTANTS[2] = 11.3021;
break;
case 711:
CONSTANTS[1] = 0.991137;
CONSTANTS[2] = 11.5725;
break;
case 712:
CONSTANTS[1] = -0.756182;
CONSTANTS[2] = 9.51829;
break;
case 713:
CONSTANTS[1] = 0.125791;
CONSTANTS[2] = 10.5654;
break;
case 714:
CONSTANTS[1] = 0.797916;
CONSTANTS[2] = 10.1496;
break;
case 715:
CONSTANTS[1] = 0.213357;
CONSTANTS[2] = 9.84970;
break;
case 716:
CONSTANTS[1] = -1.11240;
CONSTANTS[2] = 9.70595;
break;
case 717:
CONSTANTS[1] = -0.938336;
CONSTANTS[2] = 9.83747;
break;
case 718:
CONSTANTS[1] = 0.423435;
CONSTANTS[2] = 10.1917;
break;
case 719:
CONSTANTS[1] = -0.221586;
CONSTANTS[2] = 9.46389;
break;
case 720:
CONSTANTS[1] = -0.243507;
CONSTANTS[2] = 10.5818;
break;
case 721:
CONSTANTS[1] = 1.72484;
CONSTANTS[2] = 10.5350;
break;
case 722:
CONSTANTS[1] = -1.92122;
CONSTANTS[2] = 9.33013;
break;
case 723:
CONSTANTS[1] = -1.35452;
CONSTANTS[2] = 9.53380;
break;
case 724:
CONSTANTS[1] = -1.58391;
CONSTANTS[2] = 9.33147;
break;
case 725:
CONSTANTS[1] = -0.480346;
CONSTANTS[2] = 9.40854;
break;
case 726:
CONSTANTS[1] = -0.432034;
CONSTANTS[2] = 9.75558;
break;
case 727:
CONSTANTS[1] = 0.00440832;
CONSTANTS[2] = 10.0524;
break;
case 728:
CONSTANTS[1] = -0.650565;
CONSTANTS[2] = 9.96439;
break;
case 729:
CONSTANTS[1] = -0.906049;
CONSTANTS[2] = 9.88386;
break;
case 730:
CONSTANTS[1] = 1.09229;
CONSTANTS[2] = 10.5758;
break;
case 731:
CONSTANTS[1] = -0.0536554;
CONSTANTS[2] = 9.82221;
break;
case 732:
CONSTANTS[1] = -0.742173;
CONSTANTS[2] = 8.87081;
break;
case 733:
CONSTANTS[1] = 0.534684;
CONSTANTS[2] = 10.0586;
break;
case 734:
CONSTANTS[1] = 1.71160;
CONSTANTS[2] = 11.7864;
break;
case 735:
CONSTANTS[1] = -1.00782;
CONSTANTS[2] = 9.44479;
break;
case 736:
CONSTANTS[1] = 1.77866;
CONSTANTS[2] = 10.8613;
break;
case 737:
CONSTANTS[1] = 0.0668529;
CONSTANTS[2] = 9.31710;
break;
case 738:
CONSTANTS[1] = -0.924675;
CONSTANTS[2] = 10.0205;
break;
case 739:
CONSTANTS[1] = -0.743831;
CONSTANTS[2] = 9.95471;
break;
case 740:
CONSTANTS[1] = 0.270911;
CONSTANTS[2] = 11.0003;
break;
case 741:
CONSTANTS[1] = 0.347752;
CONSTANTS[2] = 10.1545;
break;
case 742:
CONSTANTS[1] = -0.173728;
CONSTANTS[2] = 9.53353;
break;
case 743:
CONSTANTS[1] = -1.83276;
CONSTANTS[2] = 8.79439;
break;
case 744:
CONSTANTS[1] = -0.302903;
CONSTANTS[2] = 10.7371;
break;
case 745:
CONSTANTS[1] = 0.668477;
CONSTANTS[2] = 10.1338;
break;
case 746:
CONSTANTS[1] = -0.0690888;
CONSTANTS[2] = 9.61609;
break;
case 747:
CONSTANTS[1] = -1.06532;
CONSTANTS[2] = 10.0634;
break;
case 748:
CONSTANTS[1] = 1.72766;
CONSTANTS[2] = 11.1485;
break;
case 749:
CONSTANTS[1] = -0.360946;
CONSTANTS[2] = 9.73864;
break;
case 750:
CONSTANTS[1] = -1.12066;
CONSTANTS[2] = 9.11297;
break;
case 751:
CONSTANTS[1] = 0.447154;
CONSTANTS[2] = 9.57730;
break;
case 752:
CONSTANTS[1] = -0.0156557;
CONSTANTS[2] = 10.0695;
break;
case 753:
CONSTANTS[1] = -0.539593;
CONSTANTS[2] = 9.96815;
break;
case 754:
CONSTANTS[1] = 2.29818;
CONSTANTS[2] = 11.1277;
break;
case 755:
CONSTANTS[1] = 0.507553;
CONSTANTS[2] = 9.92079;
break;
case 756:
CONSTANTS[1] = -1.17862;
CONSTANTS[2] = 8.15489;
break;
case 757:
CONSTANTS[1] = 1.90703;
CONSTANTS[2] = 10.8078;
break;
case 758:
CONSTANTS[1] = -0.902915;
CONSTANTS[2] = 9.31834;
break;
case 759:
CONSTANTS[1] = -0.167218;
CONSTANTS[2] = 9.96754;
break;
case 760:
CONSTANTS[1] = 0.556958;
CONSTANTS[2] = 10.3550;
break;
case 761:
CONSTANTS[1] = -0.261562;
CONSTANTS[2] = 10.5152;
break;
case 762:
CONSTANTS[1] = -2.76881;
CONSTANTS[2] = 9.04895;
break;
case 763:
CONSTANTS[1] = -0.441135;
CONSTANTS[2] = 9.19270;
break;
case 764:
CONSTANTS[1] = 0.691029;
CONSTANTS[2] = 11.1500;
break;
case 765:
CONSTANTS[1] = -0.564776;
CONSTANTS[2] = 9.06734;
break;
case 766:
CONSTANTS[1] = 0.0606141;
CONSTANTS[2] = 10.8898;
break;
case 767:
CONSTANTS[1] = 1.18677;
CONSTANTS[2] = 10.4244;
break;
case 768:
CONSTANTS[1] = 1.00683;
CONSTANTS[2] = 10.3092;
break;
case 769:
CONSTANTS[1] = -0.199073;
CONSTANTS[2] = 9.51280;
break;
case 770:
CONSTANTS[1] = -0.0875493;
CONSTANTS[2] = 9.81344;
break;
case 771:
CONSTANTS[1] = -0.222262;
CONSTANTS[2] = 10.0699;
break;
case 772:
CONSTANTS[1] = -0.0650175;
CONSTANTS[2] = 9.58257;
break;
case 773:
CONSTANTS[1] = -0.281041;
CONSTANTS[2] = 9.52095;
break;
case 774:
CONSTANTS[1] = -0.751988;
CONSTANTS[2] = 9.39143;
break;
case 775:
CONSTANTS[1] = -0.779442;
CONSTANTS[2] = 9.93878;
break;
case 776:
CONSTANTS[1] = 2.06637;
CONSTANTS[2] = 11.3209;
break;
case 777:
CONSTANTS[1] = 0.944681;
CONSTANTS[2] = 10.0518;
break;
case 778:
CONSTANTS[1] = -0.539419;
CONSTANTS[2] = 9.99597;
break;
case 779:
CONSTANTS[1] = 1.45410;
CONSTANTS[2] = 10.9979;
break;
case 780:
CONSTANTS[1] = -0.873517;
CONSTANTS[2] = 9.46646;
break;
case 781:
CONSTANTS[1] = 0.366166;
CONSTANTS[2] = 10.0650;
break;
case 782:
CONSTANTS[1] = -0.694822;
CONSTANTS[2] = 10.2249;
break;
case 783:
CONSTANTS[1] = -0.770951;
CONSTANTS[2] = 9.57244;
break;
case 784:
CONSTANTS[1] = 0.180218;
CONSTANTS[2] = 10.0573;
break;
case 785:
CONSTANTS[1] = -1.07333;
CONSTANTS[2] = 8.97283;
break;
case 786:
CONSTANTS[1] = 0.943767;
CONSTANTS[2] = 10.5321;
break;
case 787:
CONSTANTS[1] = -0.941803;
CONSTANTS[2] = 9.51315;
break;
case 788:
CONSTANTS[1] = 1.28852;
CONSTANTS[2] = 10.6615;
break;
case 789:
CONSTANTS[1] = -0.525633;
CONSTANTS[2] = 9.71593;
break;
case 790:
CONSTANTS[1] = -0.356620;
CONSTANTS[2] = 10.3038;
break;
case 791:
CONSTANTS[1] = -0.0983978;
CONSTANTS[2] = 9.45802;
break;
case 792:
CONSTANTS[1] = 1.28864;
CONSTANTS[2] = 10.6820;
break;
case 793:
CONSTANTS[1] = -1.44419;
CONSTANTS[2] = 8.76478;
break;
case 794:
CONSTANTS[1] = -0.0645201;
CONSTANTS[2] = 9.37212;
break;
case 795:
CONSTANTS[1] = 1.60105;
CONSTANTS[2] = 10.7671;
break;
case 796:
CONSTANTS[1] = -0.959894;
CONSTANTS[2] = 9.25317;
break;
case 797:
CONSTANTS[1] = 1.38561;
CONSTANTS[2] = 10.1304;
break;
case 798:
CONSTANTS[1] = 2.20120;
CONSTANTS[2] = 11.0129;
break;
case 799:
CONSTANTS[1] = -2.48587;
CONSTANTS[2] = 8.84285;
break;
case 800:
CONSTANTS[1] = -0.368809;
CONSTANTS[2] = 9.93711;
break;
case 801:
CONSTANTS[1] = -1.25567;
CONSTANTS[2] = 9.66618;
break;
case 802:
CONSTANTS[1] = -0.622936;
CONSTANTS[2] = 9.68600;
break;
case 803:
CONSTANTS[1] = 0.230822;
CONSTANTS[2] = 9.33185;
break;
case 804:
CONSTANTS[1] = -0.325642;
CONSTANTS[2] = 10.2148;
break;
case 805:
CONSTANTS[1] = -1.81875;
CONSTANTS[2] = 8.70316;
break;
case 806:
CONSTANTS[1] = -0.630032;
CONSTANTS[2] = 9.43414;
break;
case 807:
CONSTANTS[1] = 0.632039;
CONSTANTS[2] = 10.1936;
break;
case 808:
CONSTANTS[1] = -0.457704;
CONSTANTS[2] = 10.1273;
break;
case 809:
CONSTANTS[1] = 0.0919556;
CONSTANTS[2] = 9.70082;
break;
case 810:
CONSTANTS[1] = -0.385448;
CONSTANTS[2] = 9.77498;
break;
case 811:
CONSTANTS[1] = -0.470480;
CONSTANTS[2] = 9.49388;
break;
case 812:
CONSTANTS[1] = -1.52280;
CONSTANTS[2] = 10.0155;
break;
case 813:
CONSTANTS[1] = 0.741745;
CONSTANTS[2] = 10.0430;
break;
case 814:
CONSTANTS[1] = 1.33674;
CONSTANTS[2] = 11.2950;
break;
case 815:
CONSTANTS[1] = -0.646287;
CONSTANTS[2] = 9.43464;
break;
case 816:
CONSTANTS[1] = -0.460706;
CONSTANTS[2] = 9.20949;
break;
case 817:
CONSTANTS[1] = -0.591060;
CONSTANTS[2] = 9.87348;
break;
case 818:
CONSTANTS[1] = 2.53276;
CONSTANTS[2] = 12.0561;
break;
case 819:
CONSTANTS[1] = 0.662793;
CONSTANTS[2] = 10.3984;
break;
case 820:
CONSTANTS[1] = 0.776526;
CONSTANTS[2] = 10.3776;
break;
case 821:
CONSTANTS[1] = -1.61852;
CONSTANTS[2] = 9.05938;
break;
case 822:
CONSTANTS[1] = -1.90429;
CONSTANTS[2] = 9.27718;
break;
case 823:
CONSTANTS[1] = 0.301283;
CONSTANTS[2] = 10.1125;
break;
case 824:
CONSTANTS[1] = -0.509330;
CONSTANTS[2] = 10.2612;
break;
case 825:
CONSTANTS[1] = 0.804883;
CONSTANTS[2] = 11.0501;
break;
case 826:
CONSTANTS[1] = 0.0540325;
CONSTANTS[2] = 10.6170;
break;
case 827:
CONSTANTS[1] = -0.923890;
CONSTANTS[2] = 8.59465;
break;
case 828:
CONSTANTS[1] = -0.230129;
CONSTANTS[2] = 10.1681;
break;
case 829:
CONSTANTS[1] = -0.0905123;
CONSTANTS[2] = 10.4879;
break;
case 830:
CONSTANTS[1] = 1.01425;
CONSTANTS[2] = 10.8978;
break;
case 831:
CONSTANTS[1] = -0.736676;
CONSTANTS[2] = 10.3459;
break;
case 832:
CONSTANTS[1] = 0.250745;
CONSTANTS[2] = 10.4242;
break;
case 833:
CONSTANTS[1] = 0.920250;
CONSTANTS[2] = 10.0086;
break;
case 834:
CONSTANTS[1] = -0.353749;
CONSTANTS[2] = 9.45337;
break;
case 835:
CONSTANTS[1] = 0.855165;
CONSTANTS[2] = 10.4502;
break;
case 836:
CONSTANTS[1] = -0.0274329;
CONSTANTS[2] = 9.71096;
break;
case 837:
CONSTANTS[1] = 0.409861;
CONSTANTS[2] = 9.68843;
break;
case 838:
CONSTANTS[1] = 1.12378;
CONSTANTS[2] = 10.3903;
break;
case 839:
CONSTANTS[1] = 1.82371;
CONSTANTS[2] = 10.9673;
break;
case 840:
CONSTANTS[1] = -0.949015;
CONSTANTS[2] = 10.4615;
break;
case 841:
CONSTANTS[1] = -0.248353;
CONSTANTS[2] = 9.56553;
break;
case 842:
CONSTANTS[1] = 0.115274;
CONSTANTS[2] = 10.2195;
break;
case 843:
CONSTANTS[1] = 0.859606;
CONSTANTS[2] = 10.5438;
break;
case 844:
CONSTANTS[1] = 0.00345507;
CONSTANTS[2] = 9.13125;
break;
case 845:
CONSTANTS[1] = 0.465153;
CONSTANTS[2] = 9.92882;
break;
case 846:
CONSTANTS[1] = 1.38424;
CONSTANTS[2] = 11.0707;
break;
case 847:
CONSTANTS[1] = 1.94105;
CONSTANTS[2] = 11.0993;
break;
case 848:
CONSTANTS[1] = 0.326845;
CONSTANTS[2] = 10.1832;
break;
case 849:
CONSTANTS[1] = 0.497948;
CONSTANTS[2] = 10.8151;
break;
case 850:
CONSTANTS[1] = 1.53639;
CONSTANTS[2] = 10.1781;
break;
case 851:
CONSTANTS[1] = -0.0563169;
CONSTANTS[2] = 9.43552;
break;
case 852:
CONSTANTS[1] = 1.24752;
CONSTANTS[2] = 10.6178;
break;
case 853:
CONSTANTS[1] = 0.689006;
CONSTANTS[2] = 10.3522;
break;
case 854:
CONSTANTS[1] = -0.0745950;
CONSTANTS[2] = 9.53297;
break;
case 855:
CONSTANTS[1] = 0.654379;
CONSTANTS[2] = 11.1969;
break;
case 856:
CONSTANTS[1] = 0.138394;
CONSTANTS[2] = 9.63237;
break;
case 857:
CONSTANTS[1] = 1.29041;
CONSTANTS[2] = 10.5151;
break;
case 858:
CONSTANTS[1] = -0.716263;
CONSTANTS[2] = 9.47040;
break;
case 859:
CONSTANTS[1] = 1.06463;
CONSTANTS[2] = 11.0940;
break;
case 860:
CONSTANTS[1] = 0.691537;
CONSTANTS[2] = 9.66581;
break;
case 861:
CONSTANTS[1] = -0.524528;
CONSTANTS[2] = 9.98193;
break;
case 862:
CONSTANTS[1] = 0.712051;
CONSTANTS[2] = 10.3048;
break;
case 863:
CONSTANTS[1] = -1.08764;
CONSTANTS[2] = 8.86982;
break;
case 864:
CONSTANTS[1] = -1.03854;
CONSTANTS[2] = 10.0426;
break;
case 865:
CONSTANTS[1] = -0.300397;
CONSTANTS[2] = 10.0395;
break;
case 866:
CONSTANTS[1] = 0.163687;
CONSTANTS[2] = 9.89039;
break;
case 867:
CONSTANTS[1] = -0.362930;
CONSTANTS[2] = 10.4433;
break;
case 868:
CONSTANTS[1] = -1.45712;
CONSTANTS[2] = 8.62815;
break;
case 869:
CONSTANTS[1] = 1.03559;
CONSTANTS[2] = 10.5646;
break;
case 870:
CONSTANTS[1] = 0.737062;
CONSTANTS[2] = 11.3079;
break;
case 871:
CONSTANTS[1] = -0.622119;
CONSTANTS[2] = 10.6272;
break;
case 872:
CONSTANTS[1] = 0.126348;
CONSTANTS[2] = 10.1047;
break;
case 873:
CONSTANTS[1] = 0.718506;
CONSTANTS[2] = 9.66798;
break;
case 874:
CONSTANTS[1] = -1.01678;
CONSTANTS[2] = 9.98269;
break;
case 875:
CONSTANTS[1] = -0.0207573;
CONSTANTS[2] = 10.3439;
break;
case 876:
CONSTANTS[1] = -1.42406;
CONSTANTS[2] = 8.68692;
break;
case 877:
CONSTANTS[1] = 0.299812;
CONSTANTS[2] = 9.50249;
break;
case 878:
CONSTANTS[1] = -0.0204763;
CONSTANTS[2] = 10.5821;
break;
case 879:
CONSTANTS[1] = 1.06733;
CONSTANTS[2] = 9.85556;
break;
case 880:
CONSTANTS[1] = 0.654187;
CONSTANTS[2] = 11.0155;
break;
case 881:
CONSTANTS[1] = -0.276507;
CONSTANTS[2] = 9.39672;
break;
case 882:
CONSTANTS[1] = 0.192439;
CONSTANTS[2] = 9.99971;
break;
case 883:
CONSTANTS[1] = 0.392675;
CONSTANTS[2] = 10.6096;
break;
case 884:
CONSTANTS[1] = -0.537059;
CONSTANTS[2] = 9.43838;
break;
case 885:
CONSTANTS[1] = -0.121059;
CONSTANTS[2] = 9.88089;
break;
case 886:
CONSTANTS[1] = 0.824322;
CONSTANTS[2] = 9.85888;
break;
case 887:
CONSTANTS[1] = 1.32241;
CONSTANTS[2] = 9.97195;
break;
case 888:
CONSTANTS[1] = 1.72189;
CONSTANTS[2] = 9.72212;
break;
case 889:
CONSTANTS[1] = -0.476691;
CONSTANTS[2] = 9.33982;
break;
case 890:
CONSTANTS[1] = -0.513109;
CONSTANTS[2] = 9.69062;
break;
case 891:
CONSTANTS[1] = 0.907776;
CONSTANTS[2] = 10.4962;
break;
case 892:
CONSTANTS[1] = 0.998408;
CONSTANTS[2] = 9.77578;
break;
case 893:
CONSTANTS[1] = -0.453564;
CONSTANTS[2] = 8.48969;
break;
case 894:
CONSTANTS[1] = -0.0336600;
CONSTANTS[2] = 10.3131;
break;
case 895:
CONSTANTS[1] = 0.665363;
CONSTANTS[2] = 10.2302;
break;
case 896:
CONSTANTS[1] = -2.09161;
CONSTANTS[2] = 9.27132;
break;
case 897:
CONSTANTS[1] = -0.564510;
CONSTANTS[2] = 10.2897;
break;
case 898:
CONSTANTS[1] = 0.221554;
CONSTANTS[2] = 9.36024;
break;
case 899:
CONSTANTS[1] = -0.919540;
CONSTANTS[2] = 8.71021;
break;
case 900:
CONSTANTS[1] = -2.05721;
CONSTANTS[2] = 8.62042;
break;
case 901:
CONSTANTS[1] = -1.73401;
CONSTANTS[2] = 9.31850;
break;
case 902:
CONSTANTS[1] = -0.255069;
CONSTANTS[2] = 10.9187;
break;
case 903:
CONSTANTS[1] = -0.693615;
CONSTANTS[2] = 10.6267;
break;
case 904:
CONSTANTS[1] = 0.996264;
CONSTANTS[2] = 10.2673;
break;
case 905:
CONSTANTS[1] = 0.0130013;
CONSTANTS[2] = 9.95764;
break;
case 906:
CONSTANTS[1] = 0.613593;
CONSTANTS[2] = 10.7239;
break;
case 907:
CONSTANTS[1] = 1.00697;
CONSTANTS[2] = 9.83879;
break;
case 908:
CONSTANTS[1] = 0.993183;
CONSTANTS[2] = 10.5225;
break;
case 909:
CONSTANTS[1] = -1.48908;
CONSTANTS[2] = 9.54085;
break;
case 910:
CONSTANTS[1] = 1.30673;
CONSTANTS[2] = 9.74829;
break;
case 911:
CONSTANTS[1] = -0.0716510;
CONSTANTS[2] = 9.94066;
break;
case 912:
CONSTANTS[1] = 0.546889;
CONSTANTS[2] = 10.8390;
break;
case 913:
CONSTANTS[1] = -1.79451;
CONSTANTS[2] = 9.29911;
break;
case 914:
CONSTANTS[1] = -0.0469439;
CONSTANTS[2] = 9.46157;
break;
case 915:
CONSTANTS[1] = 0.390029;
CONSTANTS[2] = 10.2650;
break;
case 916:
CONSTANTS[1] = -0.626906;
CONSTANTS[2] = 9.64548;
break;
case 917:
CONSTANTS[1] = 0.289368;
CONSTANTS[2] = 10.4897;
break;
case 918:
CONSTANTS[1] = -2.06796;
CONSTANTS[2] = 8.77572;
break;
case 919:
CONSTANTS[1] = -1.03680;
CONSTANTS[2] = 10.7073;
break;
case 920:
CONSTANTS[1] = -1.40329;
CONSTANTS[2] = 9.27088;
break;
case 921:
CONSTANTS[1] = 0.200492;
CONSTANTS[2] = 10.2361;
break;
case 922:
CONSTANTS[1] = -0.599786;
CONSTANTS[2] = 9.92477;
break;
case 923:
CONSTANTS[1] = 0.811769;
CONSTANTS[2] = 9.74241;
break;
case 924:
CONSTANTS[1] = -0.722313;
CONSTANTS[2] = 9.96940;
break;
case 925:
CONSTANTS[1] = -1.17244;
CONSTANTS[2] = 10.1378;
break;
case 926:
CONSTANTS[1] = -0.222185;
CONSTANTS[2] = 10.1155;
break;
case 927:
CONSTANTS[1] = -1.13232;
CONSTANTS[2] = 10.5031;
break;
case 928:
CONSTANTS[1] = -0.455183;
CONSTANTS[2] = 10.7724;
break;
case 929:
CONSTANTS[1] = 1.16838;
CONSTANTS[2] = 10.6923;
break;
case 930:
CONSTANTS[1] = -0.549595;
CONSTANTS[2] = 10.0214;
break;
case 931:
CONSTANTS[1] = -0.140955;
CONSTANTS[2] = 9.00435;
break;
case 932:
CONSTANTS[1] = -0.587395;
CONSTANTS[2] = 9.93871;
break;
case 933:
CONSTANTS[1] = -0.888449;
CONSTANTS[2] = 9.78375;
break;
case 934:
CONSTANTS[1] = 1.85345;
CONSTANTS[2] = 11.5064;
break;
case 935:
CONSTANTS[1] = -0.479088;
CONSTANTS[2] = 10.2979;
break;
case 936:
CONSTANTS[1] = -0.124059;
CONSTANTS[2] = 9.88491;
break;
case 937:
CONSTANTS[1] = -0.301915;
CONSTANTS[2] = 9.19361;
break;
case 938:
CONSTANTS[1] = -0.956338;
CONSTANTS[2] = 9.10211;
break;
case 939:
CONSTANTS[1] = 2.16400;
CONSTANTS[2] = 10.6818;
break;
case 940:
CONSTANTS[1] = -0.723715;
CONSTANTS[2] = 10.1692;
break;
case 941:
CONSTANTS[1] = -0.665370;
CONSTANTS[2] = 9.15916;
break;
case 942:
CONSTANTS[1] = -0.646668;
CONSTANTS[2] = 10.0831;
break;
case 943:
CONSTANTS[1] = -0.695649;
CONSTANTS[2] = 10.5380;
break;
case 944:
CONSTANTS[1] = 0.315151;
CONSTANTS[2] = 9.72909;
break;
case 945:
CONSTANTS[1] = 0.352090;
CONSTANTS[2] = 10.6583;
break;
case 946:
CONSTANTS[1] = 1.36952;
CONSTANTS[2] = 10.8534;
break;
case 947:
CONSTANTS[1] = -0.910147;
CONSTANTS[2] = 9.26979;
break;
case 948:
CONSTANTS[1] = 1.34310;
CONSTANTS[2] = 10.3238;
break;
case 949:
CONSTANTS[1] = 1.03072;
CONSTANTS[2] = 11.3989;
break;
case 950:
CONSTANTS[1] = -0.455554;
CONSTANTS[2] = 11.2028;
break;
case 951:
CONSTANTS[1] = 0.852450;
CONSTANTS[2] = 10.4955;
break;
case 952:
CONSTANTS[1] = -0.0807580;
CONSTANTS[2] = 9.23639;
break;
case 953:
CONSTANTS[1] = -0.565375;
CONSTANTS[2] = 10.2599;
break;
case 954:
CONSTANTS[1] = -0.447625;
CONSTANTS[2] = 9.82794;
break;
case 955:
CONSTANTS[1] = -0.997441;
CONSTANTS[2] = 9.48575;
break;
case 956:
CONSTANTS[1] = 1.32302;
CONSTANTS[2] = 11.2775;
break;
case 957:
CONSTANTS[1] = 0.220730;
CONSTANTS[2] = 10.7034;
break;
case 958:
CONSTANTS[1] = 1.07881;
CONSTANTS[2] = 10.8143;
break;
case 959:
CONSTANTS[1] = 1.13480;
CONSTANTS[2] = 10.2689;
break;
case 960:
CONSTANTS[1] = 1.78137;
CONSTANTS[2] = 10.4883;
break;
case 961:
CONSTANTS[1] = -0.434830;
CONSTANTS[2] = 9.36449;
break;
case 962:
CONSTANTS[1] = 0.115596;
CONSTANTS[2] = 10.0827;
break;
case 963:
CONSTANTS[1] = 1.41563;
CONSTANTS[2] = 10.6233;
break;
case 964:
CONSTANTS[1] = 0.259822;
CONSTANTS[2] = 11.8505;
break;
case 965:
CONSTANTS[1] = 0.187238;
CONSTANTS[2] = 10.5595;
break;
case 966:
CONSTANTS[1] = -1.19495;
CONSTANTS[2] = 9.27385;
break;
case 967:
CONSTANTS[1] = -0.0711781;
CONSTANTS[2] = 9.49080;
break;
case 968:
CONSTANTS[1] = -1.13640;
CONSTANTS[2] = 9.51427;
break;
case 969:
CONSTANTS[1] = -1.00529;
CONSTANTS[2] = 9.82120;
break;
case 970:
CONSTANTS[1] = -0.148196;
CONSTANTS[2] = 9.28400;
break;
case 971:
CONSTANTS[1] = 0.00359574;
CONSTANTS[2] = 10.3654;
break;
case 972:
CONSTANTS[1] = -2.15386;
CONSTANTS[2] = 9.42831;
break;
case 973:
CONSTANTS[1] = -0.316218;
CONSTANTS[2] = 9.61370;
break;
case 974:
CONSTANTS[1] = -0.273544;
CONSTANTS[2] = 9.45219;
break;
case 975:
CONSTANTS[1] = 0.309532;
CONSTANTS[2] = 10.1687;
break;
case 976:
CONSTANTS[1] = 0.703372;
CONSTANTS[2] = 11.1437;
break;
case 977:
CONSTANTS[1] = -0.412385;
CONSTANTS[2] = 9.73344;
break;
case 978:
CONSTANTS[1] = 0.470502;
CONSTANTS[2] = 10.6581;
break;
case 979:
CONSTANTS[1] = 1.67429;
CONSTANTS[2] = 9.58666;
break;
case 980:
CONSTANTS[1] = -1.26984;
CONSTANTS[2] = 9.36332;
break;
case 981:
CONSTANTS[1] = 0.838545;
CONSTANTS[2] = 10.5400;
break;
case 982:
CONSTANTS[1] = -0.675980;
CONSTANTS[2] = 9.50656;
break;
case 983:
CONSTANTS[1] = 0.801317;
CONSTANTS[2] = 10.4778;
break;
case 984:
CONSTANTS[1] = 1.18293;
CONSTANTS[2] = 10.5387;
break;
case 985:
CONSTANTS[1] = -0.889240;
CONSTANTS[2] = 9.94658;
break;
case 986:
CONSTANTS[1] = -0.784644;
CONSTANTS[2] = 8.30034;
break;
case 987:
CONSTANTS[1] = -0.515011;
CONSTANTS[2] = 9.43816;
break;
case 988:
CONSTANTS[1] = -1.56726;
CONSTANTS[2] = 8.83984;
break;
case 989:
CONSTANTS[1] = 0.352946;
CONSTANTS[2] = 9.81484;
break;
case 990:
CONSTANTS[1] = 0.798638;
CONSTANTS[2] = 10.4169;
break;
case 991:
CONSTANTS[1] = 1.87032;
CONSTANTS[2] = 9.92826;
break;
case 992:
CONSTANTS[1] = 0.518340;
CONSTANTS[2] = 9.43074;
break;
case 993:
CONSTANTS[1] = -0.914958;
CONSTANTS[2] = 9.07036;
break;
case 994:
CONSTANTS[1] = 3.18263;
CONSTANTS[2] = 11.6355;
break;
case 995:
CONSTANTS[1] = -2.11596;
CONSTANTS[2] = 8.82667;
break;
case 996:
CONSTANTS[1] = 1.07909;
CONSTANTS[2] = 9.45082;
break;
case 997:
CONSTANTS[1] = -1.36946;
CONSTANTS[2] = 9.18706;
break;
case 998:
CONSTANTS[1] = 1.20362;
CONSTANTS[2] = 11.0572;
break;
case 999:
CONSTANTS[1] = 0.133729;
CONSTANTS[2] = 10.4051;
break;
}
CONSTANTS[3] = 1.00000;
CONSTANTS[4] = 0.00000;
CONSTANTS[5] = SampleUsingPDF(&pdf_0, CONSTANTS, ALGEBRAIC);
CONSTANTS[6] = CONSTANTS[5];
}
void EvaluateVariables(double VOI, double* CONSTANTS, double* RATES, double* STATES, double* ALGEBRAIC)
{
}
void ComputeRates(double VOI, double* STATES, double* RATES, double* CONSTANTS, double* ALGEBRAIC)
{
RATES[0] = CONSTANTS[6];
}
