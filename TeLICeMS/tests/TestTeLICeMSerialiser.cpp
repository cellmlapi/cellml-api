#include "Utilities.hxx"
#include "IfaceTeLICeMS.hxx"
#include "TeLICeMService.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include <cstdio>
#include <string.h>
#include <sys/stat.h>
#include <iostream>

int
main(int argc, char** argv)
{
  if (argc < 2)
  {
    puts("Usage: TestTeLICeMSerialiser uri\n");
    return 1;
  }

  RETURN_INTO_OBJREF(ts, iface::cellml_services::TeLICeMService,
                     CreateTeLICeMService());
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(ml, iface::cellml_api::ModelLoader, cb->modelLoader());

  wchar_t* URL;
  size_t l = strlen(argv[1]);
  URL = new wchar_t[l + 1];
  memset(URL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(URL, &mbrurl, l, NULL);
  ObjRef<iface::cellml_api::Model> mod;
  try
  {
    mod = already_AddRefd<iface::cellml_api::Model>(ml->loadFromURL(URL));
  }
  catch (...)
  {
    printf("Error loading model URL.\n");
    return -1;
  }

  delete [] URL;

  RETURN_INTO_WSTRING(str, ts->showModel(mod));

  printf("%S\n", str.c_str());

  return 0;
}
