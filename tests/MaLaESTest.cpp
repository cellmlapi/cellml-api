#include "MaLaESTest.hpp"
#include "MaLaESBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "CeVASBootstrap.hpp"
#include "CUSESBootstrap.hpp"
#include "AnnoToolsBootstrap.hpp"
#include "Utilities.hxx"

#include "cda_compiler_support.h"
#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( MaLaESTest );

void
MaLaESTest::setUp()
{
  mMaLaESBootstrap = CreateMaLaESBootstrap();
}

void
MaLaESTest::tearDown()
{
  if (mMaLaESBootstrap != NULL)
    mMaLaESBootstrap->release_ref();
}

void
MaLaESTest::testMaLaESBootstrap()
{
  CPPUNIT_ASSERT(mMaLaESBootstrap);
}

void
MaLaESTest::testMaLaESObject()
{
  iface::cellml_services::MaLaESTransform* mt =
    mMaLaESBootstrap->compileTransformer
    (
L"opengroup: (\r\n"
L"closegroup: )\r\n"
L"abs: #prec[H]fabs(#expr1)\r\n"
L"and: #prec[20]#exprs[&&]\r\n"
L"arccos: #prec[H]acos(#expr1)\r\n"
L"arccosh: #prec[H]acosh(#expr1)\r\n"
L"arccot: #prec[1000(900)]atan(1.0/#expr1)\r\n"
L"arccoth: #prec[1000(900)]atanh(1.0/#expr1)\r\n"
L"arccsc: #prec[1000(900)]asin(1/#expr1)\r\n"
L"arccsch: #prec[1000(900)]asinh(1/#expr1)\r\n"
L"arcsec: #prec[1000(900)]acos(1/#expr1)\r\n"
L"arcsech: #prec[1000(900)]acosh(1/#expr1)\r\n"
L"arcsin: #prec[H]asin(#expr1)\r\n"
L"arcsinh: #prec[H]asinh(#expr1)\r\n"
L"arctan: #prec[H]atan(#expr1)\r\n"
L"arctanh: #prec[H]atanh(#expr1)\r\n"
L"ceiling: #prec[H]ceil(#expr1)\r\n"
L"cos: #prec[H]cos(#expr1)\r\n"
L"cosh: #prec[H]cosh(#expr1)\r\n"
L"cot: #prec[900(0)]1.0/tan(#expr1)\r\n"
L"coth: #prec[900(0)]1.0/tanh(#expr1)\r\n"
L"csc: #prec[900(0)]1.0/sin(#expr1)\r\n"
L"csch: #prec[900(0)]1.0/sinh(#expr1)\r\n"
L"diff: #lookupDiffVariable\r\n"
L"divide: #prec[900]#expr1/#expr2\r\n"
L"eq: #prec[30]#exprs[==]\r\n"
L"exp: #prec[H]exp(#expr1)\r\n"
L"factorial: #prec[H]factorial(#expr1)\r\n"
L"factorof: #prec[30(900)]#expr1 % #expr2 == 0\r\n"
L"floor: #prec[H]floor(#expr1)\r\n"
L"gcd: #prec[H]gcd_multi(#count, #exprs[, ])\r\n"
L"geq: #prec[30]#exprs[>=]\r\n"
L"gt: #prec[30]#exprs[>]\r\n"
L"implies: #prec[10(950)]!#expr1 || #expr2\r\n"
L"int: #prec[H]defint(func#unique1, BOUND, CONSTANTS, RATES, VARIABLES, #bvarIndex)#supplement double func#unique1(double* BOUND, double* CONSTANTS, double* RATES, double* VARIABLES) { return #expr1; }\r\n"
L"lcm: #prec[H]lcm_multi(#count, #exprs[, ])\r\n"
L"leq: #prec[30]#exprs[<=]\r\n"
L"ln: #prec[H]log(#expr1)\r\n"
L"log: #prec[H]arbitrary_log(#expr1, #logbase)\r\n"
L"lt: #prec[30]#exprs[<]\r\n"
L"max: #prec[H]multi_max(#count, #exprs[, ])\r\n"
L"min: #prec[H]multi_min(#count, #exprs[, ])\r\n"
L"minus: #prec[500]#expr1 - #expr2\r\n"
L"neq: #prec[30]#expr1 != #expr2\r\n"
L"not: #prec[950]!#expr1\r\n"
L"or: #prec[10]#exprs[||]\r\n"
L"plus: #prec[500]#exprs[+]\r\n"
L"power: #prec[H]pow(#expr1, #expr2)\r\n"
L"quotient: #prec[900(0)](int)(#expr1) / (int)(#expr2)\r\n"
L"rem: #prec[900(0)](int)(#expr1) % (int)(#expr2)\r\n"
L"root: #prec[1000(900)]pow(#expr1, 1.0 / #degree)\r\n"
L"sec: #prec[900(0)]1.0 / cos(#expr1)\r\n"
L"sech: #prec[900(0)]1.0 / cosh(#expr1)\r\n"
L"sin: #prec[H]sin(#expr1)\r\n"
L"sinh: #prec[H]sinh(#expr1)\r\n"
L"tan: #prec[H]tan(#expr1)\r\n"
L"tanh: #prec[H]tanh(#expr1)\r\n"
L"times: #prec[900]#exprs[*]\r\n"
L"unary_minus: #prec[950]-#expr1\r\n"
L"units_conversion: #prec[500(900)]#expr1*#expr2 + #expr3\r\n"
L"units_conversion_offset: #prec[500]#expr1+#expr2\r\n"
L"units_conversion_factor: #prec[900]#expr1*#expr2\r\n"
L"xor: #prec[25(30)](#expr1 != 0) ^ (#expr2 != 0)\r\n"
    );

  iface::cellml_api::CellMLBootstrap* cellbs = CreateCellMLBootstrap();
  iface::cellml_api::DOMModelLoader* ml = cellbs->modelLoader();
  cellbs->release_ref();

  iface::cellml_api::Model* m = ml->loadFromURL
    (BASE_DIRECTORY L"glycolysis_pathway_1997.xml");
  ml->release_ref();

  iface::cellml_services::CeVASBootstrap* cb =
    CreateCeVASBootstrap();

  iface::cellml_services::CeVAS* cev = cb->createCeVASForModel(m);
  cb->release_ref();

  iface::cellml_services::CUSESBootstrap* cub = CreateCUSESBootstrap();
  iface::cellml_services::CUSES* cu =
    cub->createCUSESForModel(m, false);
  cub->release_ref();

  iface::cellml_services::AnnotationToolService* ats =
    CreateAnnotationToolService();
  iface::cellml_services::AnnotationSet* as = ats->createAnnotationSet();
  ats->release_ref();

  iface::cellml_api::CellMLComponentSet* ccs = m->localComponents();
  iface::cellml_api::CellMLComponent * glcC, * gts, * gp;
  glcC = ccs->getComponent(L"Glc_C");
  gts = ccs->getComponent(L"glucose_transport_system");
  gp = ccs->getComponent(L"glucose_phosphorylation");
  ccs->release_ref();

  iface::cellml_api::CellMLVariableSet* vs = glcC->variables();
  iface::cellml_api::CellMLVariable * vglcC, * vdelta_Glc_C_rxn1, * vdelta_Glc_C_rxn2;
  vglcC = vs->getVariable(L"Glc_C");
  vs->release_ref();
  vs = gts->variables();
  vdelta_Glc_C_rxn1 = vs->getVariable(L"delta_Glc_C_rxn1");
  vs->release_ref();
  vs = gp->variables();
  vdelta_Glc_C_rxn2 = vs->getVariable(L"delta_Glc_C_rxn2");
  vs->release_ref();

  as->setStringAnnotation(vglcC, L"expression_d1",
                          L"first_derivative_of_glcC");
  as->setStringAnnotation(vglcC, L"expression",
                          L"glcC");
  as->setStringAnnotation(vdelta_Glc_C_rxn1, L"expression",
                          L"delta_Glc_C_rxn1");
  as->setStringAnnotation(vdelta_Glc_C_rxn2, L"expression",
                          L"delta_Glc_C_rxn2");

  iface::cellml_api::MathList* matl =
    glcC->math();
  iface::cellml_api::MathMLElementIterator* mei = matl->iterate();
  iface::mathml_dom::MathMLElement* math = mei->next();
  DECLARE_QUERY_INTERFACE(mmel, math, mathml_dom::MathMLMathElement);
  math->release_ref();
  mei->release_ref();
  matl->release_ref();
  iface::mathml_dom::MathMLElement* expr = mmel->getArgument(1);
  mmel->release_ref();

  iface::cellml_services::MaLaESResult* mr =
    mt->transform(cev, cu, as, expr, glcC, NULL, NULL, 0);

  CPPUNIT_ASSERT(mr);

  std::wstring str = mr->compileErrors();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L""), str);

  str = mr->expression();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"first_derivative_of_glcC*0.00100000==1000.00*(1000.00*(delta_Glc_C_rxn1*0.00100000)+delta_Glc_C_rxn2)"), str);

  uint32_t l = mr->supplementariesLength();
  CPPUNIT_ASSERT_EQUAL(0, (int)l);

  std::set<iface::cellml_api::CellMLVariable*> invVars;
  iface::cellml_api::CellMLVariableIterator* cvi = mr->iterateInvolvedVariables();
  iface::cellml_api::CellMLVariable* var;
  while ((var = cvi->nextVariable()) != NULL)
  {
    CPPUNIT_ASSERT_EQUAL(0, (int)invVars.count(var));
    invVars.insert(var);
  }
  cvi->release_ref();

  std::set<iface::cellml_api::CellMLVariable*>::iterator invVarsI;

  // Time is included in the count as well.
  CPPUNIT_ASSERT_EQUAL(4, (int)invVars.size());
  CPPUNIT_ASSERT_EQUAL(1, (int)invVars.count(vglcC));
  CPPUNIT_ASSERT_EQUAL(1, (int)invVars.count(vdelta_Glc_C_rxn1));
  CPPUNIT_ASSERT_EQUAL(1, (int)invVars.count(vdelta_Glc_C_rxn2));
  for (invVarsI = invVars.begin(); invVarsI != invVars.end(); invVarsI++)
    (*invVarsI)->release_ref();

  cvi = mr->iterateBoundVariables();
  var = cvi->nextVariable();
  CPPUNIT_ASSERT(var != vglcC);
  var->release_ref();
  CPPUNIT_ASSERT(!cvi->nextVariable().getPointer());
  cvi->release_ref();

  CPPUNIT_ASSERT_EQUAL(1, (int)mr->getDiffDegree(vglcC));
  CPPUNIT_ASSERT(!mr->involvesExternalCode());

  mr->release_ref();
  expr->release_ref();
  vglcC->release_ref();
  vdelta_Glc_C_rxn1->release_ref();
  vdelta_Glc_C_rxn2->release_ref();
  gts->release_ref();
  gp->release_ref();
  glcC->release_ref();
  as->release_ref();
  cu->release_ref();
  cev->release_ref();
  mt->release_ref();
  m->release_ref();
}
