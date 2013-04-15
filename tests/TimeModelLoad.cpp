#include "Utilities.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include <sys/time.h>
#include <stdio.h>

void usage()
{
  puts("Usage: TimeModelLoad modelURL numberOfRepeats");
}

int main(int argc, char**argv)
{
  if (argc < 3)
  {
    usage();
    return 1;
  }

  const char* modelURL = argv[1];
  int numRepeats = strtoul(argv[2], NULL, 10);
  if (numRepeats == 0)
  {
    usage();
    return 1;
  }

  ObjRef<iface::cellml_api::CellMLBootstrap> cellmlBootstrap = CreateCellMLBootstrap();
  ObjRef<iface::cellml_api::DOMModelLoader> modelLoader = cellmlBootstrap->modelLoader();

  struct timeval tv1, tv2;

  wchar_t buf[1024];
  mbstowcs(buf, modelURL, 1024);
  buf[1023] = 0;
  std::wstring wmodelURL(buf);

  gettimeofday(&tv1, NULL);

  for (int i = 0; i < numRepeats; i++)
    modelLoader->loadFromURL(wmodelURL)->release_ref();

  gettimeofday(&tv2, NULL);
  
  printf("Total time = %d micros\n",
         (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec);
}
