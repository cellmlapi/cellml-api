#include <omniORB4/CORBA.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <inttypes.h>
#include "CellML_APISPEC.hh"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLContextBootstrap.hxx"
#include "SCICellML_Context.hxx"
#ifndef WIN32
#include <signal.h>
#endif

CORBA::ORB_var gBroker;

bool gShutdownServer = false;

FILE*
PrepareCellMLHome(void)
{
  // This use of environment variables becomes a security risk if used with
  // setuid/setgid programs, so don't allow it...
#ifndef _WIN32
  if ((getuid() != geteuid()) || (getgid() != getegid()))
  {
    printf("Do not run this program setuid or setgid.\n");
    exit(-1);
  }
#endif

  // Try to load the IOR file. This is very much platform specific, and we need
  // to add support for this on other platforms too...
  const char* chome = getenv("CELLML_HOME");
  std::string cellml_home;
  if (chome == NULL)
  {
    chome = getenv("HOME");
    cellml_home = chome;
    cellml_home += "/.cellml";
  }
  else
    cellml_home = chome;
  if (chome == NULL)
    cellml_home = "/.cellml";

  // Grant everyone permission, umask will restrict if needed.
  int ret = mkdir(cellml_home.c_str(), 0777);
  if (ret != 0 && errno != EEXIST)
  {
    perror("Creating CellML home directory");
    exit(-1);
  }

  std::string ior_file = cellml_home + "/corba_server";
  FILE* f = fopen(ior_file.c_str(), "r");
  if (f == NULL)
  {
    if (errno != ENOENT)
    {
      perror("Cannot open corba_server IOR file");
      exit(-1);
    }
    // No server, so open the file to write the new IOR.
#ifdef DEBUG
    printf("No IOR file now, creating new server...\n");
#endif
    return fopen(ior_file.c_str(), "w");
  }

  // We have found an IOR. It could either be new or stale.
  char buf[4096];
  size_t len = fread(buf, 1, 4095, f);
  if (len == 0)
  {
    fclose(f);
    // Invalid IOR, so return...
#ifdef DEBUG
    printf("Unreadable IOR file, creating new server...\n");
#endif
    return fopen(ior_file.c_str(), "w");
  }
  buf[len] = 0;
  try
  {
#ifdef DEBUG
    printf("Converting string to object...\n");
#endif
    CORBA::Object_var op = gBroker->string_to_object(buf);
#ifdef DEBUG
    printf("Narrowing...\n");
#endif
    cellml_context::CellMLContext_var cbv =
      cellml_context::CellMLContext::_narrow(op);
#ifdef DEBUG
    printf("Checking exists...\n");
#endif
    if (cbv->_non_existent())
    {
#ifdef DEBUG
      printf("Doesn't exist, throwing exception.\n");
#endif
      throw std::exception();
    }
  }
  catch (...)
  {
#ifdef DEBUG
    printf("Caught exception, creating new server...\n");
#endif
    fclose(f);
    // Invalid IOR, so return...
    return fopen(ior_file.c_str(), "w");
  }
  fclose(f);
  printf("There is a valid CORBA server already running.\n");
  exit(-2);
}

int
main(int argc, char** argv)
{
#ifndef WIN32
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
      setsid();
      signal(SIGHUP, SIG_IGN);
    }
    else if (!strcasecmp(command, "foreground"))
      ;
    else
    {
      printf("Usage: cellml_corba_server [help|fork|foreground]\n");
      return strcasecmp(command, "help");
    }
  }
#endif

  gBroker = CORBA::ORB_init(argc, argv);

  FILE* fIOR = PrepareCellMLHome();

  CORBA::Object_var rootPoaObj = gBroker->resolve_initial_references("RootPOA");
  PortableServer::POA_var rootPoa = PortableServer::POA::_narrow(rootPoaObj);
  if (CORBA::is_nil(rootPoa))
  {
    printf("Problem with ORB: Could not resolve the root POA.\n");
    return -1;
  }

  // Just use the root POA for now, but perhaps later we could create a sub-POA
  // with different policies?

  // Activate the root POA...
  PortableServer::POAManager_var pm = rootPoa->the_POAManager();
  pm->activate();

  // Next, create the bootstrap object...
  iface::cellml_context::CellMLContext* cbs = CreateCellMLContext();

  // Now wrap it...
  SCI::cellml_context::_final_CellMLContext* fcb = 
    new SCI::cellml_context::_final_CellMLContext(cbs, rootPoa);
  delete rootPoa->activate_object(fcb);
  cbs->release_ref();

  cellml_context::CellMLContext_var cbsV = fcb->_this();
  char* iorStr = gBroker->object_to_string(cbsV);
  fprintf(fIOR, "%s", iorStr);
  CORBA::string_free(iorStr);
  fclose(fIOR);

  while (!gShutdownServer)
    sleep(1);

  // Finally, clean up...
  fcb->release_ref();
  gBroker->shutdown(true);
}
