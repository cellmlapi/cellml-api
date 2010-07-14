/*
 * Please note that CustomGen is included as a testing program to help test the
 * API, and not as a general purpose tool. Please carefully consider whether
 * adding a new program is justified on the grounds of testing the API more
 * extensively before doing so; if your only reason is you want to provide a
 * command line tool for other purposes, please do so in a separate package
 * instead. See also:
 *  [1] https://tracker.physiomeproject.org/show_bug.cgi?id=1279
 *  [2] https://tracker.physiomeproject.org/show_bug.cgi?id=1514#c16
 *  [3] http://www.cellml.org/pipermail/cellml-tools-developers/2009-January/000140.html
 */
#include "Utilities.hxx"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "cda_compiler_support.h"
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCCGS.hxx"
#include "IfaceAnnoTools.hxx"
#include "CCGSBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include "AnnoToolsBootstrap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <string>
#include <set>

std::wstring
MakeWideString(const char* input)
{
  wchar_t* tmp;
  size_t l = strlen(input);
  tmp = new wchar_t[l + 1];
  memset(tmp, 0, (l + 1) * sizeof(wchar_t));
  const char* mbr = input;
  mbsrtowcs(tmp, &mbr, l, NULL);
  std::wstring str(tmp);
  delete [] tmp;

  return str;
}

void
TargetDescriptionToSet(char* descr,
                       std::set<std::pair<std::pair<std::wstring, std::wstring>, uint32_t> >&
                       targs)
{
  int phase = 0;
  char * comp = descr, * var = NULL, * number = NULL;
  for (char c = *descr; c != 0; c = *++descr)
  {
    switch (phase)
    {
    case 0:
      if (c == ',')
      {
        if (comp != descr)
          printf("Warning: premature comma in command line variable list. Parse results will be wrong.\n");
        return;
      }
      if (c == '/')
      {
        *descr = 0;
        var = descr + 1;
        phase = 1;
      }
      continue;
    case 1:
      if (c == ',')
      {
        printf("Warning: Comma after / but before : in command line variable list. Parse results will be wrong.\n");
        return;
      }
      if (c == ':')
      {
        *descr = 0;
        number = descr + 1;
        phase = 2;
      }
      continue;
    case 2:
      if (c == ',')
      {
        targs.insert(std::pair<std::pair<std::wstring, std::wstring>,
                               uint32_t>
                     (std::pair<std::wstring, std::wstring>
                      (
                       MakeWideString(comp), MakeWideString(var)
                      ),
                      strtoul(number, NULL, 10)
                     )
                    );

        comp = descr + 1;
        phase = 0;
      }
      continue;
    }
  }
  if (phase == 2)
  {
    targs.insert(std::pair<std::pair<std::wstring, std::wstring>,
                 uint32_t>
      (std::pair<std::wstring, std::wstring>
       (
        MakeWideString(comp), MakeWideString(var)
        ),
       strtoul(number, NULL, 10)
       )
                 );
  }
  else
    printf("Warning: unexpected end of command line variable list in phase %u. Parse results will be wrong.\n",
           phase);

}

int
main(int argc, char** argv)
{
  if (argc < 5)
  {
    printf("Usage: CustomGen modelURL wanted known unwanted\n"
           "Each of wanted, known, and unwanted is a comma separated list.\n"
           "Use a single comma to specify an empty list.\n"
           "Each entry in the list should be in the form:\n"
           "  component/variable:degreeOfDerivative\n");
    return -1;
  }

  std::wstring URL(MakeWideString(argv[1]));

  std::set<std::pair<std::pair<std::wstring, std::wstring>, uint32_t> > wanted, known, unwanted;

  TargetDescriptionToSet(argv[2], wanted);
  TargetDescriptionToSet(argv[3], known);
  TargetDescriptionToSet(argv[4], unwanted);

  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader,
                     cb->modelLoader());
  // These assignments to NULL are only here in the test code to help look for
  // memory errors by freeing things early... in production code, they wouldn't
  // be necessary.
  cb = NULL;

  ObjRef<iface::cellml_api::Model> mod;
  try
  {
    mod = already_AddRefd<iface::cellml_api::Model>(ml->loadFromURL(URL.c_str()));
  }
  catch (...)
  {
    printf("Error loading model URL.\n");
    return -1;
  }

  RETURN_INTO_OBJREF(cgb, iface::cellml_services::CodeGeneratorBootstrap,
                     CreateCodeGeneratorBootstrap()
                    );
  RETURN_INTO_OBJREF(cg, iface::cellml_services::CodeGenerator,
                     cgb->createCodeGenerator());
  cgb = NULL;

  cg->stateVariableNamePattern(L"VARS[%]");
  RETURN_INTO_OBJREF(ccg, iface::cellml_services::CustomGenerator,
                     cg->createCustomGenerator(mod));
  cg = NULL;
  mod = NULL;
  RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator,
                     ccg->iterateTargets());
  while (true)
  {
    RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget,
                       cti->nextComputationTarget());
    if (ct == NULL)
      break;

    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, ct->variable());
    RETURN_INTO_WSTRING(compname, cv->componentName());
    RETURN_INTO_WSTRING(varname, cv->name());
    std::pair<std::pair<std::wstring, std::wstring>, uint32_t> p
      (std::pair<std::wstring, std::wstring>(compname, varname), ct->degree());
    if (wanted.count(p))
      ccg->requestComputation(ct);
    else if (known.count(p))
      ccg->markAsKnown(ct);
    else if (unwanted.count(p))
      ccg->markAsUnwanted(ct);
  }
  cti = NULL;

  RETURN_INTO_OBJREF(cci, iface::cellml_services::CustomCodeInformation,
                     ccg->generateCode());
  ccg = NULL;
  printf("Constraint level = ");
  switch (cci->constraintLevel())
  {
  case iface::cellml_services::UNDERCONSTRAINED:
    printf("UNDERCONSTRAINED\n");
    break;
  case iface::cellml_services::UNSUITABLY_CONSTRAINED:
    printf("UNSUITABLY_CONSTRAINED\n");
    break;
  case iface::cellml_services::OVERCONSTRAINED:
    printf("OVERCONSTRAINED\n");
    break;
  case iface::cellml_services::CORRECTLY_CONSTRAINED:
    printf("CORRECTLY_CONSTRAINED\n");
    break;
  default:
    printf("Unkown value\n");
  }
  printf("Index count: %u\n", cci->indexCount());
  cti = already_AddRefd<iface::cellml_services::ComputationTargetIterator>(cci->iterateTargets());
  while (true)
  {
    RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget,
                       cti->nextComputationTarget());
    if (ct == NULL)
      break;

    RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, ct->variable());
    RETURN_INTO_WSTRING(compname, cv->componentName());
    RETURN_INTO_WSTRING(varname, cv->name());
    printf("* Computation target %S/%S:%u:\n", compname.c_str(), varname.c_str(),
           ct->degree());
    printf("  => Type = ");
    switch (ct->type())
    {
    case iface::cellml_services::VARIABLE_OF_INTEGRATION:
      printf("VARIABLE_OF_INTEGRATION - was marked as independent.\n");
      break;
    case iface::cellml_services::CONSTANT:
      printf("CONSTANT - this should not happen!\n");
      break;
    case iface::cellml_services::STATE_VARIABLE:
      printf("STATE_VARIABLE - was requested, and is available.\n");
      break;
    case iface::cellml_services::ALGEBRAIC:
      printf("ALGEBRAIC - is used as an intermediate.\n");
      break;
    case iface::cellml_services::FLOATING:
      printf("FLOATING - unused and not requested.\n");
      break;
    case iface::cellml_services::LOCALLY_BOUND:
      printf("LOCALLY_BOUND - locally bound in expressions only.\n");
      break;
    case iface::cellml_services::PSEUDOSTATE_VARIABLE:
      printf("PSEUDOSTATE_VARIABLE - target was requested, but could "
             "not be computed from the independent variables and model.\n");
      break;
    default:
      printf("Unknown type!\n");
    }
    RETURN_INTO_WSTRING(targname, ct->name());
    printf("  => Name = %S\n", targname.c_str());
    printf("  => Index = %u\n", ct->assignedIndex());
  }
  // To do: Print output from cci->iterateTargets();
  RETURN_INTO_WSTRING(functionsString, cci->functionsString());
  printf("Functions: %S\n", functionsString.c_str());
  RETURN_INTO_WSTRING(code, cci->generatedCode());
  printf("Code: %S\n", code.c_str());
}
