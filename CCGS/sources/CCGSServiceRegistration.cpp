#include <inttypes.h>
#include <exception>
#include "CORBA_Server_Client.hxx"
#include <stdlib.h>
#include "CCGSImplementation.hpp"

iface::cellml_context::CellMLContext* gContext;
iface::cellml_context::CellMLModuleManager* gModMan;

void
UnloadCCGS()
{
  gModMan->release_ref();
  exit(-1);
}

int
main(int argc, char** argv)
{
  gContext = GetCellMLContext(argc, argv);
  gModMan = gContext->moduleManager();

  CDA_CGenerator* cg = new CDA_CGenerator();
  cg->SetUnloadCCGS(UnloadCCGS);
  gModMan->registerModule(cg);
  cg->release_ref();

  while (true)
    sleep(10);

  UnloadCCGS();
  return 0;
}
