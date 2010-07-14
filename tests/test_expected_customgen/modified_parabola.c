Constraint level = CORRECTLY_CONSTRAINED
Index count: 5
* Computation target main/time:0:
  => Type = VARIABLE_OF_INTEGRATION - was marked as independent.
  => Name = VARS[0]
  => Index = 0
* Computation target main/offset:0:
  => Type = VARIABLE_OF_INTEGRATION - was marked as independent.
  => Name = VARS[1]
  => Index = 1
* Computation target main/x:0:
  => Type = STATE_VARIABLE - was requested, and is available.
  => Name = VARS[2]
  => Index = 2
* Computation target main/y:0:
  => Type = FLOATING - unused and not requested.
  => Name = 
  => Index = 0
* Computation target main/z:0:
  => Type = STATE_VARIABLE - was requested, and is available.
  => Name = VARS[3]
  => Index = 3
* Computation target main/y:1:
  => Type = STATE_VARIABLE - was requested, and is available.
  => Name = VARS[4]
  => Index = 4
Functions: 
Code: VARS[2] = (pow(VARS[0], 2.00000))+VARS[1];
VARS[3] = (VARS[1]>1.00000&&VARS[1]<=3.00000 ? ( sin(VARS[1])) : VARS[1]>3.00000 ? 3.00000 : 1.00000);
VARS[4] =  2.00000*VARS[0];

