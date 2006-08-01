#include <inttypes.h>
#include <exception>
#include <string>
#include "CORBA_Server_Client.hxx"
#include "corba_support/WrapperRepository.hxx"
#include "CCICellML_Context.hxx"
#include <omniORB4/CORBA.h>

static void
StartServer(std::string& dir)
{
  std::string bin = dir + "/cellml_corba_server";
#ifdef WIN32
  bin += ".exe";
  STARTUPINFO startup;
  memset(&startup, 0, sizeof(startup));
  startup.cb = sizeof(startup);
  PROCESS_INFORMATION procInfo;
  CreateProcess(dir.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startup, &procInfo);
#else
  if (fork() == 0)
  {
    execl(bin.c_str(), bin.c_str(), NULL);
    exit(-1);
  }
#endif
}

iface::cellml_context::CellMLContext*
GetCellMLContext(int argc, char** argv)
{
  while (true)
  {
    std::string chome;
    if (getenv("CELLML_HOME"))
      chome = getenv("CELLML_HOME");
    else if (getenv("HOME"))
    {
      chome = getenv("HOME");
      chome += "/.cellml";
    }
    else
    {
      chome = "/.cellml";
    }

    std::string iorfilename = chome + "/corba_server";
    FILE* f = fopen(iorfilename.c_str(), "r");
    if (f == NULL)
    {
      StartServer(chome);
#ifdef WIN32
      Sleep(500);
#else
      usleep(500000);
#endif
      continue;
    }
    char buf[1024];
    buf[0] = 0;
    fgets(buf, 1024, f);
    fclose(f);

    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
    try
    {
      CORBA::Object_var obj = orb->string_to_object(buf);
      if (obj->_non_existent())
        throw std::exception();
      cellml_context::CellMLContext_ptr cpp =
        cellml_context::CellMLContext::_narrow(obj);
      CORBA::Object_var rootPoaObj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var rootPoa =
        PortableServer::POA::_narrow(rootPoaObj);
      PortableServer::POAManager_var poaMan = rootPoa->the_POAManager();
      poaMan->activate();

      return new CCI::cellml_context::_final_CellMLContext(cpp, rootPoa);
    }
    catch (...)
    {
      StartServer(chome);
#ifdef WIN32
      Sleep(500);
#else
      usleep(500000);
#endif
      continue;
    }
  }
}
