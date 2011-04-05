/*
 * Please note that CellML2C is included as a testing program to help test the
 * API, and not as a general purpose tool. Please carefully consider whether
 * adding a new CellML2* program is justified on the grounds of testing the API
 * more extensively before doing so; if your only reason is you want to provide
 * a command line tool for another language, please do so in a separate package
 * instead. See also:
 *  [1] https://tracker.physiomeproject.org/show_bug.cgi?id=1279
 *  [2] https://tracker.physiomeproject.org/show_bug.cgi?id=1514#c16
 *  [3] http://www.cellml.org/pipermail/cellml-tools-developers/2009-January/000140.html
 */
#include "cda_compiler_support.h"
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCCGS.hxx"
#include "IfaceMaLaES.hxx"
#include "IfaceAnnoTools.hxx"
#include "CCGSBootstrap.hpp"
#include "MaLaESBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "AnnoToolsBootstrap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <algorithm>
#include <string>
#include "Utilities.hxx"

#ifdef _WIN32
#define swprintf _snwprintf
#endif

const wchar_t*
TypeToString(iface::cellml_services::VariableEvaluationType vet)
{
  switch (vet)
  {
  case iface::cellml_services::VARIABLE_OF_INTEGRATION:
    return L"variable of integration";
  case iface::cellml_services::CONSTANT:
    return L"constant";
  case iface::cellml_services::STATE_VARIABLE:
    return L"state variable";
  case iface::cellml_services::PSEUDOSTATE_VARIABLE:
    return L"pseudo-state variable";
  case iface::cellml_services::ALGEBRAIC:
    return L"algebraic variable";
  case iface::cellml_services::FLOATING:
    return L"uncomputed";
  case iface::cellml_services::LOCALLY_BOUND:
    return L"locally bound";
  }

  return L"invalid type";
}

void
WriteCode(iface::cellml_services::CodeInformation* cci)
{
  // Scoped locale change.
  CNumericLocale locobj;

  iface::cellml_services::ModelConstraintLevel mcl =
    cci->constraintLevel();
  if (mcl == iface::cellml_services::UNDERCONSTRAINED)
  {
    printf("Model is underconstrained.\n"
           "List of undefined targets follows...\n");
    iface::cellml_services::ComputationTargetIterator* cti = cci->iterateTargets();
    iface::cellml_services::ComputationTarget* ct;
    std::vector<std::wstring> messages;
    while (true)
    {
      ct = cti->nextComputationTarget();
      if (ct == NULL)
        break;
      if (ct->type() != iface::cellml_services::FLOATING)
      {
        ct->release_ref();
        continue;
      }
      iface::cellml_api::CellMLVariable* v = ct->variable();
      wchar_t* n = v->name();
      wchar_t* c = v->componentName();
      std::wstring str = L"  ";
      uint32_t deg = ct->degree();
      if (deg != 0)
      {
        str += L"d^";
        wchar_t buf[20];
        swprintf(buf, 20, L"%u", deg);
        str += buf;
        str += L"/dt^";
        str += buf;
        str += L" ";
      }
      str += n;
      str += L" (in ";
      str += c;
      str += L")\n";
      free(n);
      free(c);
      messages.push_back(str);
      v->release_ref();
      ct->release_ref();
    }
    cti->release_ref();
    // Sort the messages...
    std::sort(messages.begin(), messages.end());
    std::vector<std::wstring>::iterator msgi;
    for (msgi = messages.begin(); msgi != messages.end(); msgi++)
      printf("%S", (*msgi).c_str());
    return;
  }
  else if (mcl == iface::cellml_services::OVERCONSTRAINED)
  {
    printf("Model is overconstrained.\n"
           "List variables defined at time of error follows...\n");
    iface::cellml_services::ComputationTargetIterator* cti = cci->iterateTargets();
    iface::cellml_services::ComputationTarget* ct;
    std::vector<std::wstring> messages;
    while (true)
    {
      ct = cti->nextComputationTarget();
      if (ct == NULL)
        break;
      if (ct->type() == iface::cellml_services::FLOATING)
      {
        ct->release_ref();
        continue;
      }
      iface::cellml_api::CellMLVariable* v = ct->variable();
      wchar_t* n = v->name();
      std::wstring str = L"  ";
      uint32_t deg = ct->degree();
      if (deg != 0)
      {
        str += L"d^";
        wchar_t buf[20];
        swprintf(buf, 20, L"%u", deg);
        str += buf;
        str += L"/dt^";
        str += buf;
        str += L" ";
      }
      str += n;
      free(n);
      str += L"\n";
      messages.push_back(str);
      v->release_ref();
      ct->release_ref();
    }
    cti->release_ref();

    // Sort the messages...
    std::sort(messages.begin(), messages.end());
    std::vector<std::wstring>::iterator msgi;
    for (msgi = messages.begin(); msgi != messages.end(); msgi++)
      printf("%S", (*msgi).c_str());

    // Get flagged equations...
    iface::mathml_dom::MathMLNodeList* mnl = cci->flaggedEquations();
    printf("Extraneous equation was:\n");
    iface::dom::Node* n = mnl->item(0);
    mnl->release_ref();
    iface::dom::Element* el =
      reinterpret_cast<iface::dom::Element*>(n->query_interface("dom::Element"));
    n->release_ref();

    wchar_t* cmeta = el->getAttributeNS(L"http://www.cellml.org/metadata/1.0#",
                                        L"id");
    if (!wcscmp(cmeta, L""))
      printf("  <equation with no cmeta ID>\n");
    else
      printf("  %S\n", cmeta);
    free(cmeta);

    n = el->parentNode();
    el->release_ref();

    el = reinterpret_cast<iface::dom::Element*>
      (n->query_interface("dom::Element"));
    n->release_ref();

    cmeta = el->getAttributeNS(L"http://www.cellml.org/metadata/1.0#", L"id");
    if (!wcscmp(cmeta, L""))
      printf("  in <math with no cmeta ID>\n");
    else
      printf("  in math with cmeta:id %S\n", cmeta);
    free(cmeta);
    el->release_ref();

    return;
  }
  else if (mcl == iface::cellml_services::UNSUITABLY_CONSTRAINED)
  {
    printf("Model is unsuitably constrained (i.e. would need capabilities"
           "beyond those of the CCGS to solve).\n"
           "The status of variables at time of error follows...\n");
    iface::cellml_services::ComputationTargetIterator* cti = cci->iterateTargets();
    iface::cellml_services::ComputationTarget* ct;
    std::vector<std::wstring> messages;
    while (true)
    {
      ct = cti->nextComputationTarget();
      if (ct == NULL)
        break;
      std::wstring str = L"  ";
      if (ct->type() == iface::cellml_services::FLOATING)
        str += L" Undefined: ";
      else
        str += L" Defined: ";

      uint32_t deg = ct->degree();
      if (deg != 0)
      {
        str += L"d^";
        wchar_t buf[20];
        swprintf(buf, 20, L"%u", deg);
        str += buf;
        str += L"/dt^";
        str += buf;
        str += L" ";
      }
      iface::cellml_api::CellMLVariable* v = ct->variable();
      wchar_t* n = v->name();
      str += n;
      free(n);
      str += L"\n";
      messages.push_back(str);
      v->release_ref();
      ct->release_ref();
    }
    cti->release_ref();

    // Sort the messages...
    std::sort(messages.begin(), messages.end());
    std::vector<std::wstring>::iterator msgi;
    for (msgi = messages.begin(); msgi != messages.end(); msgi++)
      printf("%S", (*msgi).c_str());

    return;
  }

  printf("%% Model is correctly constrained.\n");
  iface::mathml_dom::MathMLNodeList* mnl = cci->flaggedEquations();
  uint32_t i, l = mnl->length();
  if (l == 0)
    printf("%% No equations needed Newton-Raphson evaluation.\n");
  else
    printf("%% The following equations needed Newton-Raphson evaluation:\n");

  std::vector<std::wstring> messages;
  for (i = 0; i < l; i++)
  {
    iface::dom::Node* n = mnl->item(i);
    iface::dom::Element* el =
      reinterpret_cast<iface::dom::Element*>(n->query_interface("dom::Element"));
    n->release_ref();

    wchar_t* cmeta = el->getAttributeNS(L"http://www.cellml.org/metadata/1.0#",
                                        L"id");
    std::wstring str;
    if (!wcscmp(cmeta, L""))
      str += L"%  <equation with no cmeta ID>\n";
    else
    {
      str += L"%  ";
      str += cmeta;
      str += L"\n";
    }
    free(cmeta);

    n = el->parentNode();
    el->release_ref();

    el = reinterpret_cast<iface::dom::Element*>
      (n->query_interface("dom::Element"));
    n->release_ref();

    cmeta = el->getAttributeNS(L"http://www.cellml.org/metadata/1.0#", L"id");
    if (!wcscmp(cmeta, L""))
      str += L"%   in <math with no cmeta ID>\n";
    else
    {
      str += L"%  in math with cmeta:id ";
      str += cmeta;
      str += L"\n";
    }
    free(cmeta);
    el->release_ref();

    messages.push_back(str);
  }
  mnl->release_ref();

  // Sort the messages...
  std::sort(messages.begin(), messages.end());
  std::vector<std::wstring>::iterator msgi;
  for (msgi = messages.begin(); msgi != messages.end(); msgi++)
    printf("%S", (*msgi).c_str());
  
  printf("%%\r\n%% There are a total of %u entries in the algebraic variable array.\n", cci->algebraicIndexCount());
  printf("%% There are a total of %u entries in each of the rates & state variable arrays.\n", cci->rateIndexCount());
  printf("%% There are a total of %u entries in the constant variable array.\n%%\r\n", cci->constantIndexCount());
  
  messages.clear();
  iface::cellml_services::ComputationTargetIterator* cti = cci->iterateTargets();
  while (true)
  {
    iface::cellml_services::ComputationTarget* ct = cti->nextComputationTarget();
    if (ct == NULL)
      break;
    iface::cellml_api::CellMLVariable* v = ct->variable();
    iface::cellml_api::CellMLElement* el = v->parentElement();
    iface::cellml_api::CellMLComponent* c =
      reinterpret_cast<iface::cellml_api::CellMLComponent*>
      (el->query_interface("cellml_api::CellMLComponent"));
    el->release_ref();

    std::wstring str;
    wchar_t* vn = v->name(), * cn = c->name();

    str += L"    LEGEND_";
    wchar_t * vsn;
    vsn = ct->name();
    str += vsn;
    free(vsn);

    str += L" = strpad('";
    uint32_t deg = ct->degree();
    if (deg != 0)
    {
      str += L"d^";
      wchar_t buf[20];
      swprintf(buf, 20, L"%u", deg);
      str += buf;
      str += L"/dt^";
      str += buf;
      str += L" ";
    }
    str += vn;
    str += L" in component ";
    str += cn;
    str += L"');\n";
    free(vn);
    free(cn);

    c->release_ref();
    v->release_ref();
    ct->release_ref();

    messages.push_back(str);
  }
  cti->release_ref();

  // Now start the code...

  printf("\r\n" 
         "function [VOI, STATES, ALGEBRAIC, CONSTANTS] = solveModel()\r\n" 
         "    %% Initialise constants and state variables\r\n" 
         "    [INIT_STATES, CONSTANTS] = initConsts;\r\n\r\n" 
         "    %% Set timespan to solve over\r\n" 
         "    tspan = [0,10];\r\n"
         "    %% Set numerical accuracy options for ODE solver\r\n" 
         "    options = odeset('RelTol', 1E-6, 'AbsTol', 1E-6, 'MaxStep', 0.1);\r\n\r\n" 
         "    %% Solve model with ODE solver\r\n" 
         "    [VOI, STATES] = ode15s(@(VOI, STATES)computeRates(VOI, STATES, CONSTANTS), tspan, INIT_STATES, options);\r\n\r\n" 
         "    %% Compute algebraic variables\r\n" 
         "    ALGEBRAIC = computeAlgebraic(CONSTANTS, STATES, VOI);\r\n\r\n" 
         "    %% Plot state variables against variable of integration\r\n" 
         "    [LEGEND_STATES, LEGEND_ALGEBRAIC, LEGEND_VOI, LEGEND_CONSTANTS] = createLegends();\r\n" 
         "    figure();\r\n" 
         "    plot(VOI, STATES);\r\n" 
         "    xlabel(LEGEND_VOI);\r\n" 
         "    l = legend(LEGEND_STATES);\r\n" 
         "    set(l,'Interpreter','none');\r\n" 
         "end\r\n\r\n");

  // Sort and print the list of variables
  std::sort(messages.begin(), messages.end());
  printf("function [LEGEND_STATES, LEGEND_ALGEBRAIC, LEGEND_VOI, LEGEND_CONSTANTS] = createLegends()\r\n"
   "    LEGEND_STATES = ''; LEGEND_ALGEBRAIC = ''; LEGEND_VOI = ''; LEGEND_CONSTANTS = '';\r\n");
  for (msgi = messages.begin(); msgi != messages.end(); msgi++)
    printf("%S", (*msgi).c_str());
  printf("    LEGEND_STATES  = LEGEND_STATES';\r\n" 
         "    LEGEND_ALGEBRAIC = LEGEND_ALGEBRAIC';\r\n"
         "    LEGEND_RATES = LEGEND_RATES';\r\n"
         "    LEGEND_CONSTANTS = LEGEND_CONSTANTS';\r\n"
         "end\r\n\r\n");

  wchar_t* frag = cci->initConstsString();
  printf("function [STATES, CONSTANTS] = initConsts()\r\n    CONSTANTS = []; STATES = [];\r\n");
  printf("%S", frag);
  printf("    if (isempty(STATES)), warning('Initial values for states not set');, end\r\nend\r\n\r\n");
  free(frag);

  frag = cci->ratesString();
  printf("function RATES = computeRates(VOI, STATES, CONSTANTS)\r\n"
    "    STATES = STATES'; RATES = []; ALGEBRAIC = [0];\r\n");
  printf("%S", frag);
  printf("    RATES = RATES';\r\nend\r\n\r\n");
  free(frag);

  printf("%% Calculate algebraic variables\r\n"
         "function ALGEBRAIC = computeAlgebraic(CONSTANTS, STATES, VOI)\r\n"
         "    ALGEBRAIC = zeros(length(VOI),1);\r\n");

  // Copy algebraic variable calculations from rate calculations
  // Use lines beginning with 'A'
  frag = cci->ratesString();
  i = 0;
  bool algebraic = false, newline = true;

  while(frag[i] != '\0') {
    if(frag[i] == ' ' || frag[i] == '\t') {
      i++;
      continue;
    }
    if(newline && frag[i] == 'A') {
      algebraic = true;
      newline = false;
      printf("    ");
    }
    else if (newline) {
      newline = false;
    }
    if(algebraic) {
      printf("%C", frag[i]);
    }
    if(frag[i] == '\n') {
      newline = true;
      algebraic = false;
    }
    i++;
  }
  free(frag);

  frag = cci->variablesString();
  printf("%S",frag);
  free(frag);
  printf("end\r\n\r\n"
         "%% Compute result of a piecewise function\r\n" 
         "function x = piecewise(cases, default)\r\n" 
         "    set = [0];\r\n" 
         "    for i = 1:2:length(cases)\r\n" 
         "        if (length(cases{i+1}) == 1)\r\n" 
         "            x(cases{i} & ~set,:) = cases{i+1};\r\n" 
         "        else\r\n" 
         "            x(cases{i} & ~set,:) = cases{i+1}(cases{i} & ~set);\r\n" 
         "        end\r\n" 
         "        set = set | cases{i};\r\n" 
         "        if(set), break, end\r\n" 
         "    end\r\n" 
         "    if (length(default) == 1)\r\n" 
         "        x(~set,:) = default;\r\n" 
         "    else\r\n" 
         "        x(~set,:) = default(~set);\r\n" 
         "    end\r\n" 
         "end\r\n" 
         "%% Pad out or shorten strings to a set length\r\n" 
         "function strout = strpad(strin)\r\n" 
         "    req_length = 50;\r\n" 
         "    insize = size(strin,2);\r\n" 
         "    if insize > req_length\r\n" 
         "       strout = strin(1:req_length);\r\n" 
         "    else\r\n" 
         "       strout = [strin, blanks(req_length - insize)];\r\n" 
         "    end\r\n" 
         "end\r\n" 
         "%% Compute a logarithm to any base\r\n" 
         "function x = arbitrary_log(a, base)\r\n" 
         "    x = log(a) ./ log(base);\r\n" 
         "end\r\n" 
         "%% Least common multiple\r\n" 
         "function m = lcm_multi(x)\r\n" 
         "    m = x(1);\r\n" 
         "    for (i = 1:size(x,2))\r\n" 
         "        if(x(i) ~= 0), m = m./gcd(m,x(i)).*x(i);, end\r\n" 
         "    end\r\n" 
         "end\r\n" 
         "%% Greatest common divisor\r\n" 
         "function d = gcd_multi(x)\r\n" 
         "    d = 0;\r\n" 
         "    for (i = 1:size(x,2)), d = gcd(d, x(:,i));, end\r\n" 
         "end\r\n\r\n");

  frag = cci->functionsString();
  printf("%S\n", frag);
  free(frag);
}

void
doNameAnnotations(iface::cellml_api::Model* aModel,
                  iface::cellml_services::CodeGenerator* aCG)
{
  // Make an annotation set...
  iface::cellml_services::AnnotationToolService* ats
    (CreateAnnotationToolService());
  iface::cellml_services::AnnotationSet* as(ats->createAnnotationSet());
  ats->release_ref();

  aCG->useAnnoSet(as);

  // Now we go through all variables in the model and set their annotations...
  iface::cellml_api::CellMLComponentSet* ccs = aModel->allComponents();

  iface::cellml_api::CellMLComponentIterator* cci = ccs->iterateComponents();
  ccs->release_ref();

  iface::cellml_api::CellMLComponent* comp;
  while ((comp = cci->nextComponent()) != NULL)
  {
    iface::cellml_api::CellMLVariableSet* vs(comp->variables());
    wchar_t* compname = comp->name();
    comp->release_ref();
    iface::cellml_api::CellMLVariableIterator* vi(vs->iterateVariables());
    vs->release_ref();

    iface::cellml_api::CellMLVariable* v;
    while ((v = vi->nextVariable()) != NULL)
    {
      wchar_t* name = v->name();
      std::wstring varn = compname;
      varn += L"_";
      varn += name;
      free(name);
      std::wstring raten = L"rate_";
      raten += varn;
      as->setStringAnnotation(v, L"expression", varn.c_str());
      as->setStringAnnotation(v, L"expression_d1", raten.c_str());
      v->release_ref();
    }

    vi->release_ref();

    free(compname);
  }

  cci->release_ref();

  as->release_ref();
}

int
main(int argc, char** argv)
{
  // Get the URL from which to load the model...
  if (argc < 2)
  {
    printf("Usage: CellML2Matlab modelURL\n");
    return -1;
  }

  uint32_t usenames = 0;

  if (argc > 2)
  {
    if (!strcmp(argv[2], "usenames"))
      usenames = 1;
  }

  wchar_t* URL;
  size_t l = strlen(argv[1]);
  URL = new wchar_t[l + 1];
  memset(URL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(URL, &mbrurl, l, NULL);

  iface::cellml_api::CellMLBootstrap* cb =
    CreateCellMLBootstrap();

  iface::cellml_api::ModelLoader* ml =
    cb->modelLoader();
  cb->release_ref();

  iface::cellml_api::Model* mod;
  try
  {
    mod = ml->loadFromURL(URL);
  }
  catch (...)
  {
    printf("Error loading model URL.\n");
    // Well, a leak on exit wouldn't be so bad, but someone might reuse this
    // code, so...
    delete [] URL;
    ml->release_ref();
    return -1;
  }

  ml->release_ref();
  delete [] URL;

  iface::cellml_services::CodeGeneratorBootstrap* cgb =
    CreateCodeGeneratorBootstrap();
  iface::cellml_services::CodeGenerator* cg =
    cgb->createCodeGenerator();
  cgb->release_ref();

  // The code generator is designed to generate C code by default, so
  // we need to customise it to generate Matlab code instead

  cg->constantPattern(L"CONSTANTS(:,%)");
  cg->stateVariableNamePattern(L"STATES(:,%)");
  cg->algebraicVariableNamePattern(L"ALGEBRAIC(:,%)");
  cg->rateNamePattern(L"RATES(:,%)");
  cg->voiPattern(L"VOI");
  cg->assignPattern(L"    <LHS> = <RHS>;\r\n");
  cg->arrayOffset(1);
  cg->solvePattern(L"ALGEBRAIC = rootfind_<ID>(VOI, CONSTANTS, STATES, ALGEBRAIC);\r\n" 
  L"<SUP>" 
  L"% Functions required for solving differential algebraic equation\r\n" 
  L"function ALGEBRAIC = rootfind_<ID>(VOI, CONSTANTS, STATES, ALGEBRAIC_IN)\r\n" 
  L"    ALGEBRAIC = ALGEBRAIC_IN;\r\n" 
  L"    global initialGuess;\r\n" 
  L"    if (length(initialGuess) ~= 1), initialGuess = getInitialGuess"
  L"    options = optimset('Display', 'off', 'TolX', 1E-6);\r\n"
  L"    if length(VOI) == 1\r\n"
  L"        residualfn = @(algebraicCandidate)residualSN_<ID>(algebraicCandidate, ALGEBRAIC, VOI, CONSTANTS, STATES);\r\n"
  L"        <VAR> = fsolve(residualfn, initialGuess, options);\r\n"
  L"        initialGuess = <VAR>;\r\n" 
  L"    else\r\n" 
  L"        SET_<VAR> = logical(1);\r\n"
  L"        for i=1:length(VOI)\r\n" 
  L"            residualfn = @(algebraicCandidate)residualSN_<ID>(algebraicCandidate, ALGEBRAIC(i,:), VOI(i), CONSTANTS, STATES(i,:));\r\n"
  L"            TEMP_<VAR> = fsolve(residualfn, initialGuess, options);\r\n" 
  L"            ALGEBRAIC(i,SET_ALGEBRAIC) = TEMP_ALGEBRAIC(SET_ALGEBRAIC);\r\n"
  L"            initialGuess = TEMP_<VAR>;\r\n" 
  L"        end\r\n" 
  L"    end\r\n" 
  L"end\r\n" 
  L"function resid = residualSN_<ID>(algebraicCandidate, ALGEBRAIC, VOI, CONSTANTS, STATES)\r\n" 
  L"    <VAR> = algebraicCandidate;\r\n" 
  L"    resid = fixnans((<LHS>) - (<RHS>));\r\n" 
  L"end\r\n" 
  L"function algebraicGuess = getInitialGuess(count)\r\n" 
  L"    algebraicGuess = ones(count,1) * 0.1;\r\n" 
  L"end\r\n" 
  L"function xfixed = fixnans(x)\r\n" 
  L"    xfixed = x;\r\n" 
  L"    xfixed(isnan(x)) = 1.0;\r\n"
  L"    xfixed(isinf(x)) = 1E1000;\r\n"
  L"end\r\n");
  cg->solveNLSystemPattern(L"ALGEBRAIC = rootfind_<ID>(VOI, CONSTANTS, STATES, ALGEBRAIC);\r\n" 
  L"<SUP>" 
  L"% Functions required for solving differential algebraic equation\r\n" 
  L"function ALGEBRAIC = rootfind_<ID>(VOI, CONSTANTS, STATES, ALGEBRAIC_IN)\r\n" 
  L"    ALGEBRAIC = ALGEBRAIC_IN;\r\n" 
  L"    global initialGuess;\r\n" 
  L"    if (length(initialGuess) ~= <COUNT>), initialGuess = getInitialGuess(<COUNT>);, end\r\n" 
  L"    options = optimset('Display', 'off', 'TolX', 1E-6);\r\n" 
  L"    if length(VOI) == 1\r\n" 
  L"        residualfn = @(algebraicCandidate)residualSN_<ID>(algebraicCandidate, ALGEBRAIC, VOI, CONSTANTS, STATES);\r\n" 
  L"        soln = fsolve(residualfn, initialGuess, options);\r\n" 
  L"        initialGuess = soln;\r\n" 
  L"        <EQUATIONS><VAR> = soln(<INDEX>);<JOIN>\r\n" 
  L"        </EQUATIONS>\r\n" 
  L"    else\r\n" 
  L"        <EQUATIONS>SET_<VAR> = logical(1);<JOIN>\r\n" 
  L"        </EQUATIONS>\r\n" 
  L"        for i=1:length(VOI)\r\n" 
  L"            residualfn = @(algebraicCandidate)residualSN_<ID>(algebraicCandidate, ALGEBRAIC(i,:), VOI(i), CONSTANTS, STATES(i,:));\r\n" 
  L"            soln = fsolve(residualfn, initialGuess, options);\r\n" 
  L"            initialGuess = soln;\r\n" 
  L"            <EQUATIONS>TEMP_<VAR> = soln(<INDEX>);<JOIN>\r\n" 
  L"            </EQUATIONS>\r\n" 
  L"            ALGEBRAIC(i,SET_ALGEBRAIC) = TEMP_ALGEBRAIC(SET_ALGEBRAIC);\r\n" 
  L"        end\r\n" 
  L"    end\r\n" 
  L"end\r\n" 
  L"function resid = residualSN_<ID>(algebraicCandidate, ALGEBRAIC, VOI, CONSTANTS, STATES)\r\n" 
  L"    <EQUATIONS><VAR> = algebraicCandidate(<INDEX>);<JOIN>\r\n" 
  L"    </EQUATIONS>\r\n" 
  L"    <EQUATIONS>resid(<INDEX>) = fixnans((<LHS>) - (<RHS>));<JOIN>\r\n" 
  L"    </EQUATIONS>\r\n" 
  L"end\r\n" 
  L"function algebraicGuess = getInitialGuess(count)\r\n" 
  L"    algebraicGuess = ones(count,1) * 0.1;\r\n" 
  L"end\r\n" 
  L"function xfixed = fixnans(x)\r\n" 
  L"    xfixed = x;\r\n" 
  L"    xfixed(isnan(x)) = 1.0;\r\n" 
  L"    xfixed(isinf(x)) = 1E1000;\r\n" 
  L"end\r\n");

  iface::cellml_services::MaLaESBootstrap* mb = CreateMaLaESBootstrap();

  iface::cellml_services::MaLaESTransform* mt =
    mb->compileTransformer
    (
  L"opengroup: (\r\n" 
  L"closegroup: )\r\n" 
  L"abs: #prec[H]abs(#expr1)\r\n" 
  L"and: #prec[20]#exprs[&]\r\n" 
  L"arccos: #prec[H]acos(#expr1)\r\n" 
  L"arccosh: #prec[H]acosh(#expr1)\r\n" 
  L"arccot: #prec[H]acot(#expr1)\r\n" 
  L"arccoth: #prec[H]acoth(#expr1)\r\n" 
  L"arccsc: #prec[H]acsc(#expr1)\r\n" 
  L"arccsch: #prec[H]acsch(#expr1)\r\n" 
  L"arcsec: #prec[H]asec(#expr1)\r\n" 
  L"arcsech: #prec[H]asech(#expr1)\r\n" 
  L"arcsin: #prec[H]asin(#expr1)\r\n" 
  L"arcsinh: #prec[H]asinh(#expr1)\r\n" 
  L"arctan: #prec[H]atan(#expr1)\r\n" 
  L"arctanh: #prec[H]atanh(#expr1)\r\n" 
  L"ceiling: #prec[H]ceil(#expr1)\r\n" 
  L"cos: #prec[H]cos(#expr1)\r\n" 
  L"cosh: #prec[H]cosh(#expr1)\r\n" 
  L"cot: #prec[H]cot(#expr1)\r\n" 
  L"coth: #prec[H]coth(#expr1)\r\n" 
  L"csc: #prec[H]csc(#expr1)\r\n" 
  L"csch: #prec[H]csch(#expr1)\r\n" 
  L"diff: #lookupDiffVariable\r\n" 
  L"divide: #prec[900]#expr1./#expr2\r\n" 
  L"eq: #prec[30]#exprs[==]\r\n" 
  L"exp: #prec[H]exp(#expr1)\r\n" 
  L"factorial: #prec[H]factorial(#expr1)\r\n" 
  L"factorof: #prec[30(900)]mod(#expr1, #expr2) == 0\r\n" 
  L"floor: #prec[H]floor(#expr1)\r\n" 
  L"gcd: #prec[H]gcd_multi([#exprs[, ]])\r\n" 
  L"geq: #prec[30]#exprs[>=]\r\n" 
  L"gt: #prec[30]#exprs[>]\r\n" 
  L"implies: #prec[10(950)] ~#expr1 | #expr2\r\n" 
  L"int: #prec[H]defint(func#unique1, BOUND, CONSTANTS, RATES, VARIABLES, " 
  L"#bvarIndex)#supplement double func#unique1(double* BOUND, " 
  L"double* CONSTANTS, double* RATES, double* VARIABLES) { return #expr1; }\r\n" 
  L"lcm: #prec[H]lcm_multi([#exprs[, ]])\r\n" 
  L"leq: #prec[30]#exprs[<=]\r\n" 
  L"ln: #prec[H]log(#expr1)\r\n" 
  L"log: #prec[H]arbitrary_log(#expr1, #logbase)\r\n" 
  L"lt: #prec[30]#exprs[<]\r\n" 
  L"max: #prec[H]max([#exprs[, ]],[],2)\r\n" 
  L"min: #prec[H]min([#exprs[, ]],[],2)\r\n" 
  L"minus: #prec[500]#expr1 - #expr2\r\n" 
  L"neq: #prec[30]#expr1 ~= #expr2\r\n" 
  L"not: #prec[950]~#expr1\r\n" 
  L"or: #prec[10]#exprs[|]\r\n" 
  L"plus: #prec[500]#exprs[+]\r\n" 
  L"power: #prec[H]#expr1 .^ #expr2\r\n" 
  L"quotient: #prec[900(0)] floor(#expr1 ./ #expr2)\r\n" 
  L"rem: #prec[900(0)] rem(#expr1, #expr2)\r\n" 
  L"root: #prec[1000(900)] #expr1 .^ (1.0 / #degree)\r\n" 
  L"sec: #prec[H]sec(#expr1)\r\n" 
  L"sech: #prec[H]sech(#expr1)\r\n" 
  L"sin: #prec[H] sin(#expr1)\r\n" 
  L"sinh: #prec[H] sinh(#expr1)\r\n" 
  L"tan: #prec[H] tan(#expr1)\r\n" 
  L"tanh: #prec[H] tanh(#expr1)\r\n" 
  L"times: #prec[900] #exprs[.*]\r\n" 
  L"unary_minus: #prec[950] - #expr1\r\n" 
  L"units_conversion: #prec[500(900)]#expr1.*#expr2 + #expr3\r\n" 
  L"units_conversion_factor: #prec[900]#expr1.*#expr2\r\n" 
  L"units_conversion_offset: #prec[500]#expr1+#expr2\r\n" 
  L"xor: #prec[25(30)]xor(#expr1 , #expr2)\r\n" 
  L"piecewise_first_case: #prec[5]piecewise({#expr1, #expr2 \r\n" 
  L"piecewise_extra_case: #prec[5], #expr1, #expr2 \r\n" 
  L"piecewise_otherwise: #prec[5]}, #expr1)\r\n" 
  L"piecewise_no_otherwise: #prec[5]}, NaN)\r\n" 
  L"pi: #prec[999] pi\r\n" 
  L"eulergamma: #prec[999]0.577215664901533\r\n" 
  L"infinity: #prec[900] Inf\r\n"
    );

  cg->transform(mt);

  mt->release_ref();
  mb->release_ref();

  if (usenames)
    doNameAnnotations(mod, cg);

  iface::cellml_services::CodeInformation* cci = NULL;
  try
  {
    cci = cg->generateCode(mod);
  }
  catch (iface::cellml_api::CellMLException&)
  {
    printf("Caught a CellMLException while generating code.\n");
    cg->release_ref();
    mod->release_ref();
    return -1;
  }
  catch (...)
  {
    printf("Unexpected exception calling generateCode!\n");
    // this is a leak, but it should also never happen :)
    return -1;
  }
  mod->release_ref();
  cg->release_ref();

  wchar_t* m = cci->errorMessage();
  if (wcscmp(m, L""))
  {
    printf("Error generating code: %S\n", m);
    cci->release_ref();
    free(m);
    return -1;
  }
  free(m);

  // We now have the code information...
  WriteCode(cci);
  cci->release_ref();

  return 0;
}
