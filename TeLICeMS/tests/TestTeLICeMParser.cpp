#include "Utilities.hxx"
#include "IfaceTeLICeMS.hxx"
#include "TeLICeMService.hpp"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include <cstdio>
#include <string.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/errno.h>
#endif
#include <iostream>

int
main(int argc, char** argv)
{
  if (argc < 2)
  {
    puts("Usage: TestTeLICeMParser uri\n");
    return 1;
  }

  RETURN_INTO_OBJREF(ts, iface::cellml_services::TeLICeMService,
                     CreateTeLICeMService());

  struct stat statbuf;
  FILE* f = fopen(argv[1], "r");
  if (f == NULL)
  {
#ifdef WIN32
    printf("Error opening file.\n");
#else
    printf("Error opening file: %s\n", strerror(errno));
#endif
    return 2;
  }

  fstat(fileno(f), &statbuf);
  char* buf = new char[statbuf.st_size + 1];
  fread(buf, statbuf.st_size, 1, f);
  fclose(f);
  buf[statbuf.st_size] = 0;
  wchar_t* wcb = new wchar_t[statbuf.st_size + 1];
  mbstowcs(wcb, buf, statbuf.st_size + 1);
  delete [] buf;
  RETURN_INTO_OBJREF(res, iface::cellml_services::TeLICeMModelResult,
                     ts->parseModel(wcb));
  delete [] wcb;
  
  RETURN_INTO_WSTRING(errmsg, res->errorMessage());
  if (errmsg != L"")
  {
    fprintf(stderr, "TeLICeMS error message: %S\n", errmsg.c_str());
    return 1;
  }

  RETURN_INTO_OBJREF(mod, iface::cellml_api::Model, res->modelResult());
  RETURN_INTO_WSTRING(t, mod->serialisedText());
  printf("%S\n", t.c_str());

  return 0;
}
