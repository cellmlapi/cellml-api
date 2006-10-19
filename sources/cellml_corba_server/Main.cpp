#include <omniORB4/CORBA.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef WIN32
#define _WIN32_IE 0x400
#include <shlobj.h>
#else
#include <sys/errno.h>
#endif
#include <inttypes.h>
#include "CellML_APISPEC.hh"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLContextBootstrap.hxx"
#include "SCICellML_Context.hxx"
#ifndef WIN32
#include <signal.h>
#endif
#include <ltdl.h>
#include "ServiceRegistration.hxx"
#include <list>

CORBA::ORB_var gBroker;

bool gShutdownServer = false;

struct DlopenService
{
  int (*do_registration)(void* aContext, void* aModuleManager,
                         void (*UnloadService)());
};

std::list<DlopenService> gOpenServiceList;

void
UnloadService()
{
  // Silently ignore requests to unload linked in services.
}

static void
ProcessConfiguration(FILE* aConf)
{
  char buf[1024];
  while (fgets(buf, 1024, aConf))
  {
    char* end = strchr(buf, '\n');
    if (end != NULL)
      *end = 0;
    end = strchr(buf, '\r');
    if (end != NULL)
      *end = 0;
    if (buf[0] == '#')
      continue;
    char* cmd = strtok(buf, " ");
    if (cmd == NULL)
      continue;
    if (!strcmp(cmd, "load_service"))
    {
      char* fn = strtok(NULL, "");
      if (fn == NULL)
      {
        printf("Warning: load_service command takes an argument.\n");
        continue;
      }
      lt_dlhandle dlh = lt_dlopen(fn);
      if (dlh == NULL)
      {
        printf("Warning: Can't dlopen %s for load_service command: %s\n",
               fn, lt_dlerror());
        continue;
      }
      DlopenService dos;
      dos.do_registration =
        (int (*)(void*, void*,void (*)()))
        lt_dlsym(dlh, "do_registration");
      if (dos.do_registration == NULL)
      {
        printf("Warning: load_service shared object %s lacks do_registration "
               "symbol. Skipping. Check it is a service.\n", fn);
      }
      gOpenServiceList.push_back(dos);
    }
  }
}


static FILE*
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
#ifdef _WIN32
    char path[MAX_PATH];
    SHGetSpecialFolderPath(NULL, path, CSIDL_LOCAL_APPDATA, true);
    chome = path;
#else
    chome = getenv("HOME");
#endif
    if (chome == NULL)
      chome = "";
    cellml_home = chome;
    cellml_home += "/.cellml";
  }
  else
    cellml_home = chome;
  if (chome == NULL)
    cellml_home = "/.cellml";

  // Grant everyone permission, umask will restrict if needed.
  int ret = mkdir(cellml_home.c_str()
#ifndef WIN32
		  ,0777
#endif
                 );
  if (ret != 0 && errno != EEXIST)
  {
    perror("Creating CellML home directory");
    exit(-1);
  }

  std::string config_file = cellml_home + "/server_config";
  FILE* cf = fopen(config_file.c_str(), "r");
  if (cf != NULL)
  {
    ProcessConfiguration(cf);
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
  lt_dlinit();
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

  // Next, start any configured link-in services...
  std::list<DlopenService>::iterator i;
  iface::cellml_context::CellMLModuleManager* mman =
    cbs->moduleManager();
  for (i = gOpenServiceList.begin(); i != gOpenServiceList.end(); i++)
  {
    (*i).do_registration(reinterpret_cast<void*>(cbs),
                         reinterpret_cast<void*>(mman), UnloadService);
  }
  mman->release_ref();

  while (!gShutdownServer)
#ifdef WIN32
    Sleep(1000);
#else
    sleep(1);
#endif

  // Finally, clean up...
  fcb->release_ref();
  gBroker->shutdown(true);
}
