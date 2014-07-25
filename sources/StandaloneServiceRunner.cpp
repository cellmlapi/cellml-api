#include "ServiceRegistration.hxx"
#include "CORBA_Server_Client.hxx"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <signal.h>
#include <unistd.h>
#endif

static iface::cellml_context::CellMLContext* gContext;
static iface::cellml_context::CellMLModuleManager* gModMan;

void
UnloadService()
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
      printf("Usage: %s [help|fork|foreground]\n", argv[0]);
      return strcasecmp(command, "help");
    }
  }
#endif

  gContext = GetCellMLContext(argc, argv);
  gModMan = gContext->moduleManager();

  do_registration(reinterpret_cast<void*>(gContext),
                  reinterpret_cast<void*>(gModMan),
                  UnloadService);

#ifdef WIN32
  while (true)
    sleep(100000);
#else
  sigset_t ss;
  sigemptyset(&ss);
  sigaddset(&ss, SIGTERM);
  sigaddset(&ss, SIGINT);
  if (!forked)
    printf("Blocking SIGTERM/SIGINT...\n");
  sigprocmask(SIG_BLOCK, &ss, NULL);
  siginfo_t si;
  if (!forked)
    printf("Waiting for SIGTERM/SIGINT...\n");
  while (sigwaitinfo(&ss, &si) < 0)
    ;

  do_deregistration(reinterpret_cast<void*>(gModMan));

  gModMan->release_ref();

#endif
}
