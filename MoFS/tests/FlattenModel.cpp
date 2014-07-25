/*
Please note that FlattenModel is included as a testing program to help test the
API, and not as a general purpose tool.
*/
#include <iostream>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceMoFS.hxx"
#include "CellMLBootstrap.hpp"
#include "MoFSBootstrap.hpp"
#include <cstring>
#include "cellml-api-cxx-support.hpp"

int
main(int argc, char** argv)
{

  if (argc < 2)
  {
    std::wcout << L"Usage: FlattenModel url-to-cellml-1-1-file\n";
    return 1;
  }

  size_t l = strlen(argv[1]);
  wchar_t* URL = new wchar_t[l + 1];
  memset(URL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(URL, &mbrurl, l, NULL);

  std::wstring modelURL(URL);
  delete [] URL;

  ObjRef<iface::cellml_api::CellMLBootstrap> cbs
    (CreateCellMLBootstrap());
  ObjRef<iface::cellml_api::DOMModelLoader> ml(cbs->modelLoader());
  ObjRef<iface::cellml_api::Model> mod;

  try
  {
    mod = ml->loadFromURL(modelURL);
  }
  catch (iface::cellml_api::CellMLException&)
  {
    std::wcerr << L"Cannot load model from " << modelURL << std::endl;
    return 2;
  }

  ObjRef<iface::mofs::ModelFlatteningService> mofs
    (CreateModelFlatteningService());
  ObjRef<iface::mofs::ModelFlattener> mf(mofs->createFlattener());

  try
  {
    ObjRef<iface::cellml_api::Model> result(mf->flatten(mod));
    std::wcout << result->serialisedText();
    return 0;
  }
  catch (iface::cellml_api::CellMLException&)
  {
    std::wcout << L"Problem flattening model: " << mf->lastError() << std::endl;
    return 3;
  }
}
