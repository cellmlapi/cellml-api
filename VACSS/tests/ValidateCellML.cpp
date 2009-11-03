#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceVACSS.hxx"
#include "Utilities.hxx"
#include "VACSSBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include <string>
#include <stdio.h>

#ifdef _WIN32
#define swprintf _snwprintf
#endif

std::wstring
ConvertRepresentationValidityError
(
 iface::cellml_services::VACSService* vacss,
 iface::cellml_api::Model* model,
 iface::cellml_services::CellMLRepresentationValidityError* crve
)
{
  iface::dom::Node* errorNode = crve->errorNode();
  uint32_t col;
  uint32_t row = vacss->getPositionInXML(errorNode,
                                         crve->errorNodalOffset(),
                                         &col);
  wchar_t buf[40];
  swprintf(buf, 40, L"line %u, column %u", row, col);
  errorNode->release_ref();

  return buf;
}

std::wstring
ConvertSemanticValidityError
(
 iface::cellml_services::VACSService* vacss,
 iface::cellml_api::Model* model,
 iface::cellml_services::CellMLSemanticValidityError* csve
)
{
  RETURN_INTO_OBJREF(ee, iface::cellml_api::CellMLElement, csve->errorElement());
  DECLARE_QUERY_INTERFACE_OBJREF(eede, ee, cellml_api::CellMLDOMElement);
  RETURN_INTO_OBJREF(de, iface::dom::Element, eede->domElement());

  uint32_t col;
  uint32_t row = vacss->getPositionInXML(de, 0, &col);
  wchar_t buf[40];
  swprintf(buf, 40, L"line %u, column %u", row, col);

  // Up to the model...
  while (true)
  {
    ee = already_AddRefd<iface::cellml_api::CellMLElement>(ee->parentElement());
    if (ee == NULL)
      break;

    DECLARE_QUERY_INTERFACE_OBJREF(mod, ee, cellml_api::Model);
    if (mod == NULL)
      continue;

    RETURN_INTO_OBJREF(impel, iface::cellml_api::CellMLElement, mod->parentElement());
    if (impel == NULL)
      break;

    DECLARE_QUERY_INTERFACE(imp, impel, cellml_api::CellMLImport);
    if (imp == NULL)
      break;

    std::wstring ret(buf);
    ret += L" in import from ";
    RETURN_INTO_OBJREF(href, iface::cellml_api::URI, imp->xlinkHref());
    RETURN_INTO_WSTRING(hreft, href->asText());
    ret += hreft;

    return ret;
  }

  return buf;
}

void
DisplayValidityError
(
 iface::cellml_services::VACSService* vacss,
 iface::cellml_api::Model* model,
 iface::cellml_services::CellMLValidityError* cve
)
{
  RETURN_INTO_WSTRING(descr, cve->description());
  const wchar_t* type = (cve->isWarningOnly() ? L"Warning: " :
                                                  L"Error: ");

  std::wstring location;
  DECLARE_QUERY_INTERFACE_OBJREF(crve, cve,
                                 cellml_services::CellMLRepresentationValidityError);

  if (crve != NULL)
    location = ConvertRepresentationValidityError(vacss, model, crve);
  else
  {
    DECLARE_QUERY_INTERFACE_OBJREF(csve, cve,
                                   cellml_services::CellMLSemanticValidityError);

    if (csve != NULL)
      location = ConvertSemanticValidityError(vacss, model, csve);
    else
      location = L"*unknown - can't QI to CellMLSemanticValidityError*";
  }

  printf("%S%S at %S\n",
         type, descr.c_str(), location.c_str());
}

void
DisplayValidityErrors
(
 iface::cellml_services::VACSService* vacss,
 iface::cellml_api::Model* model,
 iface::cellml_services::CellMLValidityErrorSet* cves
)
{
  uint32_t i = 0, l = cves->nValidityErrors();
  printf("There were %u errors detected.\n",
         l);

  for (i = 0; i < l; i++)
  {
    iface::cellml_services::CellMLValidityError* err = cves->getValidityError(i);
    DisplayValidityError(vacss, model, err);
    err->release_ref();
  }
}

int
main(int argc, char** argv)
{
  // Get the URL from which to load the model...
  if (argc < 2 || !strcmp(argv[1], "--help"))
  {
    printf("Usage: ValidateCellML modelURL\n"
          );
    return -1;
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
  printf("# Loading model...\n");
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

  // Create the validation service...
  iface::cellml_services::VACSService* vacss = CreateVACSService();
  iface::cellml_services::CellMLValidityErrorSet* cves =
    vacss->validateModel(mod);

  DisplayValidityErrors(vacss, mod, cves);
  mod->release_ref();
  vacss->release_ref();
  cves->release_ref();

  return 0;
}
