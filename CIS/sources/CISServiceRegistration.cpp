#include <inttypes.h>
#include <exception>
#include "CORBA_Server_Client.hxx"
#include <stdlib.h>
#include "Utilities.hxx"
#include "CISImplementation.hxx"
#ifndef WIN32
#include <signal.h>
#endif

iface::cellml_context::CellMLContext* gContext;
iface::cellml_context::CellMLModuleManager* gModMan;

void
UnloadCIS()
{
  gModMan->release_ref();
  exit(-1);
}

int
main(int argc, char** argv)
{
  gContext = GetCellMLContext(argc, argv);
  gModMan = gContext->moduleManager();

  CDA_CellMLIntegrationService* cis = new CDA_CellMLIntegrationService();
  cis->SetUnloadCIS(UnloadCIS);
  gModMan->registerModule(cis);
  cis->release_ref();

#ifdef WIN32
  while (true)
    sleep(100000);
#else
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss, SIGTERM);
  sigaddset(&ss, SIGINT);
  printf("Blocking SIGTERM/SIGINT...\n");
  sigprocmask(SIG_BLOCK, &ss, NULL);
  siginfo_t si;
  printf("Waiting for SIGTERM/SIGINT...\n");
  sigwaitinfo(&ss, &si);
  printf("Got SIGTERM/SIGINT, deregistering module...\n");

  gModMan->deregisterModule(cis);
  printf("Module deregistered; releasing manager...\n");

  gModMan->release_ref();
  printf("Manager released, ready to exit.\n");
  return 0;
#endif
}
