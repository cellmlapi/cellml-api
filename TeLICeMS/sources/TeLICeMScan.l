%top{
#define YY_NO_UNISTD_H
#include <iostream>
#define IN_LEXER
#include "TeLICeMParse.gen.hpp"
}

%option c++
%option noyywrap
%option prefix="TeLICeM"
%option yyclass="TeLICeMStateScan"
%option noyylineno
%option outfile="TeLICeMScanner.cpp"
%x stringsc

%{
#include "TeLICeMStateScan.hxx"
#define MAGIC_NEWLINE 9999
#define MAGIC_WHITESPACE 9998
%}

/*
 * Note that a URI must conform to RFC2396 (see http://www.ietf.org/rfc/rfc2396.txt),
 * of which a regular expression is (see Appendix B):
 *
 *   ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?
 *
 * Its Flex equivalent is:
 *
 *   ([^:/?# \t\n]+:)?(\/\/[^/?# \t\n]*)?[^?# \t\n]*(\?[^# \t\n]*)?(#[^ \t\n]*)?
 *
 * Note, however, that the above will catch pretty much anything, so it cannot
 * be used directly. See the T_URI rule for more information...
 */

%%

[ \t\r\f]+ return MAGIC_WHITESPACE;
\n return MAGIC_NEWLINE;

  /* Reaction role types */
reactant {
  mLValue->reactionRoleType(iface::cellml_api::Role::REACTANT);
  return T_ROLETYPE;
}
product {
  mLValue->reactionRoleType(iface::cellml_api::Role::PRODUCT);
  return T_ROLETYPE;
}
rate {
  mLValue->reactionRoleType(iface::cellml_api::Role::RATE);
  return T_ROLETYPE;
}
catalyst {
  mLValue->reactionRoleType(iface::cellml_api::Role::CATALYST);
  return T_ROLETYPE;
}
activator {
  mLValue->reactionRoleType(iface::cellml_api::Role::ACTIVATOR);
  return T_ROLETYPE;
}
inhibitor {
  mLValue->reactionRoleType(iface::cellml_api::Role::INHIBITOR);
  return T_ROLETYPE;
}
modifier {
  mLValue->reactionRoleType(iface::cellml_api::Role::MODIFIER);
  return T_ROLETYPE;
}

 /* Reacton directions... */
forward {
  mLValue->reactionDirectionType(iface::cellml_api::Role::FORWARD);
  return T_DIRECTIONTYPE;
}
reverse {
  mLValue->reactionDirectionType(iface::cellml_api::Role::REVERSE);
  return T_DIRECTIONTYPE;
}
both {
  mLValue->reactionDirectionType(iface::cellml_api::Role::BOTH);
  return T_DIRECTIONTYPE;
}

 /* SI Prefixes... */
yotta {
  mLValue->siPrefix(24);
  return T_SIPREFIX;
}
zetta {
  mLValue->siPrefix(21);
  return T_SIPREFIX;
}
exa {
  mLValue->siPrefix(18);
  return T_SIPREFIX;
}
peta {
  mLValue->siPrefix(15);
  return T_SIPREFIX;
}
tera {
  mLValue->siPrefix(12);
  return T_SIPREFIX;
}
giga {
  mLValue->siPrefix(9);
  return T_SIPREFIX;
}
mega {
  mLValue->siPrefix(6);
  return T_SIPREFIX;
}
kilo {
  mLValue->siPrefix(3);
  return T_SIPREFIX;
}
hecto {
  mLValue->siPrefix(2);
  return T_SIPREFIX;
}
de[kc]a {
  mLValue->siPrefix(1);
  return T_SIPREFIX;
}
deci {
  mLValue->siPrefix(-1);
  return T_SIPREFIX;
}
centi {
  mLValue->siPrefix(-2);
  return T_SIPREFIX;
}
milli {
  mLValue->siPrefix(-3);
  return T_SIPREFIX;
}
micro {
  mLValue->siPrefix(-6);
  return T_SIPREFIX;
}
nano {
  mLValue->siPrefix(-9);
  return T_SIPREFIX;
}
pico {
  mLValue->siPrefix(-12);
  return T_SIPREFIX;
}
femto {
  mLValue->siPrefix(-15);
  return T_SIPREFIX;
}
atto {
  mLValue->siPrefix(-18);
  return T_SIPREFIX;
}
zepto {
  mLValue->siPrefix(-21);
  return T_SIPREFIX;
}
yocto {
  mLValue->siPrefix(-24);
  return T_SIPREFIX;
}

 /* Interface directions... */
in {
  mLValue->variableInterfaceType(iface::cellml_api::INTERFACE_IN);
  return T_INTERFACETYPE;
}
out {
  mLValue->variableInterfaceType(iface::cellml_api::INTERFACE_OUT);
  return T_INTERFACETYPE;
}
none {
  mLValue->variableInterfaceType(iface::cellml_api::INTERFACE_NONE);
  return T_INTERFACETYPE;
}

 /* Reversibility */
irrev {
  mLValue->boolean(false);
  return T_REVERSIBILITY;
}
rev {
  mLValue->boolean(true);
  return T_REVERSIBILITY;
}

and       return T_AND;
as        return T_AS;
base      return T_BASE;
between return T_BETWEEN;
case return T_CASE;
comp return T_COMP;
d\( return T_DIFF;
def return T_DEF;
del\( return T_PARTIALDIFF;
dir return T_DIR;
dvar return T_DVAR;
else return T_ELSE;
endcomp return T_ENDCOMP;
enddef return T_ENDDEF;
endrole return T_ENDROLE;
endvar return T_ENDVAR;
expo return T_EXPO;
for return T_FOR;
group return T_GROUP;
import return T_IMPORT;
incl return T_INCL;
init return T_INIT;
map return T_MAP;
math return T_MATH;
model return T_MODEL;
mult return T_MULT;
name return T_NAME;
namespace return T_NAMESPACE;
not return T_NOT;
off return T_OFF;
or return T_OR;
piecewise return T_PIECEWISE;
pref return T_PREF;
priv return T_PRIV;
pub return T_PUB;
react return T_REACT;
role return T_ROLE;
stoichio return T_STOICHIO;
then return T_THEN;
type return T_TYPE;
unit return T_UNIT;
var return T_VAR;
vars return T_VARS;
with return T_WITH;

== return T_EQEQ;
!= return T_NEQ;
\<\> return T_NEQ;
\>= return T_GE;
\<= return T_LE;

<stringsc>\\. {
  char c = YYText()[1];

  switch (c)
  {
    case 'n':
      mLValue->plusstring('\n');
      break;

    case 'r':
      mLValue->plusstring('\r');
      break;

    case 't':
      mLValue->plusstring('\t');
      break;

    case 'f':
      mLValue->plusstring('\f');
      break;

    default:
      mLValue->plusstring(c);
  }
}

<stringsc>\" {
  BEGIN(INITIAL);
  return T_QUOTED;
}

<stringsc>. mLValue->plusstring(YYText()[0]);

\" {
  BEGIN(stringsc);
  mLValue->string("");
}

\#[a-zA-Z_]+[a-zA-Z_0-9]* {
  mLValue->string(YYText() + 1);
  mLValue->mOverrideBuiltin = true;
  return T_IDENTIFIER;
}

[a-zA-Z_]+[a-zA-Z_0-9]* {
  mLValue->string(YYText());
  mLValue->mOverrideBuiltin = false;
  return T_IDENTIFIER;
}

(\-)?[0-9]+(\.[0-9]*)?((E|e)(\-|\+)?[0-9]+)? {
  mLValue->number(strtod(YYText(), NULL));
  return T_NUMBER;
}

. return YYText()[0];
