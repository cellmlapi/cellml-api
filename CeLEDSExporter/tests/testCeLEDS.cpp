#include "cda_compiler_support.h"
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCeLEDSExporter.hxx"
#include "CeLEDSExporterBootstrap.hpp"
#include "CellMLBootstrap.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <algorithm>
#include <string>
#include "cellml-api-cxx-support.hpp"
#include <iostream>

int
main(int argc, char** argv)
{
  // Get the URLs from which to load the model and the
  // the language definition file
  if (argc != 3)
  {
    printf("Usage: testCeLEDS modelURL languageDefinitionURL\n");
    return -1;
  }

  wchar_t* modelURL;
  size_t l = strlen(argv[1]);
  modelURL = new wchar_t[l + 1];
  memset(modelURL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(modelURL, &mbrurl, l, NULL);

  wchar_t* languageURL;
  l = strlen(argv[2]);
  languageURL = new wchar_t[l + 1];
  memset(languageURL, 0, (l + 1) * sizeof(wchar_t));
  const char* lbrurl = argv[2];
  mbsrtowcs(languageURL, &lbrurl, l, NULL);

  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
    CreateCellMLBootstrap());

  RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader,
    cb->modelLoader());

  ObjRef<iface::cellml_api::Model> mod;
  try
  {
    mod = already_AddRefd<iface::cellml_api::Model>(ml->loadFromURL(modelURL));
    if (mod == NULL)
      throw L"";
  }
  catch (...)
  {
    printf("Error loading model URL.\n");
    delete [] modelURL;
    delete [] languageURL;
    return -1;
  }

  delete [] modelURL;

  RETURN_INTO_OBJREF(ceb, iface::cellml_services::CeLEDSExporterBootstrap,
    CreateCeLEDSExporterBootstrap());
  RETURN_INTO_OBJREF(ce, iface::cellml_services::CodeExporter,
      ceb->createExporter(languageURL));

  RETURN_INTO_WSTRING(le, ceb->loadError());
  if (!le.empty()) {
    wprintf(L"Could not load language definition file:\n");
    wprintf(L"    %ls\n",le.c_str());
    return -1;
  }
  delete [] languageURL;

  std::wstring code = ce->generateCode(mod);
  wprintf(L"%ls", code.c_str());

  return 0;
}
