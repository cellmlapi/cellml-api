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
#ifndef WIN32
  bool forked = false;
  if (argc > 1)
  {
    char* command = argv[1];
    while (command[0] == '-')
      command++;
    if (!strcasecmp(command, "fork"))
    {
      pid_t p = fork();
      if (p < 0)
      {
        perror("fork");
        return -1;
      }
      else if (p > 0)
      {
        return 0;
      }
      forked = true;
      setsid();
      signal(SIGHUP, SIG_IGN);
    }
    else if (!strcasecmp(command, "foreground"))
      ;
    else
    {
      printf("Usage: CISService [help|fork|foreground]\n");
      return strcasecmp(command, "help");
    }
  }
#endif

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
  if (forked)
    printf("Blocking SIGTERM/SIGINT...\n");
  sigprocmask(SIG_BLOCK, &ss, NULL);
  siginfo_t si;
  if (forked)
    printf("Waiting for SIGTERM/SIGINT...\n");
  sigwaitinfo(&ss, &si);

  gModMan->deregisterModule(cis);

  gModMan->release_ref();
  return 0;
#endif
}
