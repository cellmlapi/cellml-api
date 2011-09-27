#include "cellml-api-cxx-support.hpp"
#include "IfaceSRuS.hxx"
#include "SProSBootstrap.hpp"
#include "SRuSBootstrap.hpp"
#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#define sleep Sleep
#else
#include <pthread.h>
#endif

// A wrapper for a mutex...
class CDAMutex
{
public:
  CDAMutex()
  {
#ifdef WIN32
    mMutex = CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(&mMutex, NULL);
#endif
  }

  ~CDAMutex()
  {
#ifdef WIN32
    CloseHandle(mMutex);
#else
    pthread_mutex_destroy(&mMutex);
#endif
  }

  void Lock()
  {
#ifdef WIN32
    WaitForSingleObject(mMutex, INFINITE);
#else
    pthread_mutex_lock(&mMutex);
#endif
  }

  void Unlock()
  {
#ifdef WIN32
    ReleaseMutex(mMutex);
#else
    pthread_mutex_unlock(&mMutex);
#endif
  }
private:
#ifdef WIN32
  HANDLE mMutex;
#else
  pthread_mutex_t mMutex;
#endif
};

// A class to provide a scoped lock...
class CDALock
{
public:
  CDALock(CDAMutex& m)
    : mutex(m)
  {
    mutex.Lock();
  }

  ~CDALock()
  {
    mutex.Unlock();
  }
private:
  CDAMutex& mutex;
};

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_sedml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_sedml/"
#endif
#endif

class Monitor
  : public iface::SRuS::GeneratedDataMonitor
{
public:
  Monitor()
    : tasksFinished(0)
  {
  }

  void add_ref() throw() {}
  void release_ref() throw() {}
  char* objid() throw() { return strdup("RunSEDMLMonitor"); }
  void* query_interface(const char* aInterface) throw()
  {
    if (!strcmp(aInterface, "xpcom::IObject") ||
        !strcmp(aInterface, "SRuS::GeneratedDataMonitor"))
      return static_cast<iface::XPCOM::IObject*>(this);
    else
      return NULL;
  }
  char** supported_interfaces(uint32_t* len) throw()
  {
    *len = 2;
    char** ret = static_cast<char**>(malloc(sizeof(char*) * 2));
    ret[0] = strdup("xpcom::IObject");
    ret[1] = strdup("SRuS::GeneratedDataMonitor");
    return ret;
  }

  void progress(iface::SRuS::GeneratedDataSet* aData) throw()
  {
    // We may have multiple concurrent tasks.
    CDALock lock(mStoringData);

    uint32_t l = aData->length();
    if (l == 0)
      return;

    RETURN_INTO_OBJREF(gd0, iface::SRuS::GeneratedData, aData->item(0));
    uint32_t dl = gd0->length();

    for (uint32_t j = 0; j < l; j++)
    {
      RETURN_INTO_OBJREF(gd, iface::SRuS::GeneratedData, aData->item(j));
      RETURN_INTO_OBJREF(dg, iface::SProS::DataGenerator, gd->sedmlDataGenerator());
      RETURN_INTO_WSTRING(dgid, dg->id());

      std::map<std::wstring, std::vector<double> >::iterator
        it(mResults.find(dgid));
      
      if (it == mResults.end())
      {
        std::vector<double> l;
        mResults.insert(std::pair<std::wstring, std::vector<double> >(dgid, l));
        it = mResults.find(dgid);
      }

      for (uint32_t i = 0; i < dl; i++)
      {
        (*it).second.push_back(gd->dataPoint(i));
      }
    }
  }

  void failure(const char* aMsg) throw()
  {
    printf("Task Failure: %s\n", aMsg);
    tasksFinished++;
  }

  void done() throw()
  {
    printf("Task finished successfully.\n");
    tasksFinished++;
  }

  void printResults() throw()
  {
    std::map<std::wstring, std::vector<double> >::iterator i;
    uint32_t nMax = 0;
    for (i = mResults.begin(); i != mResults.end(); i++)
    {
#undef max
      nMax = std::max(nMax, static_cast<uint32_t>((*i).second.size()));
      printf("%S,", (*i).first.c_str());
    }
    printf("\n");

    for (uint32_t j = 0; j < nMax; j++)
    {
      for (i = mResults.begin(); i != mResults.end(); i++)
      {
        if ((*i).second.size() <= j)
          printf(",");
        else
          printf("%g,", (*i).second[j]);
      }
      printf("\n");
    }
  }

  uint32_t tasksFinished;

private:
  std::map<std::wstring, std::vector<double> > mResults;
  CDAMutex mStoringData;
};

int
main(int argc, char** argv)
{
  RETURN_INTO_OBJREF(sr, iface::SRuS::Bootstrap, CreateSRuSBootstrap());
  RETURN_INTO_OBJREF(spb, iface::SProS::Bootstrap, CreateSProSBootstrap());
  RETURN_INTO_OBJREF(sp, iface::SRuS::SEDMLProcessor,
                     sr->makeDefaultProcessor());

  if (argc < 2)
  {
    printf("Usage: RunSEDML url-to-sedml-file\n");
    return 1;
  }

  uint32_t taskCount = 0;
  size_t l = strlen(argv[1]);
  wchar_t* URL = new wchar_t[l + 1];
  memset(URL, 0, (l + 1) * sizeof(wchar_t));
  const char* mbrurl = argv[1];
  mbsrtowcs(URL, &mbrurl, l, NULL);

  ObjRef<iface::SProS::SEDMLElement> sedml;
  try
  {
    sedml = already_AddRefd<iface::SProS::SEDMLElement>
      (spb->parseSEDMLFromURI(URL, BASE_DIRECTORY));
  }
  catch (...)
  {
  }

  delete [] URL;

  if (sedml == NULL)
  {
    printf("Failure loading SEDML model.\n");
    return 1;
  }

  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet, sedml->tasks());
  RETURN_INTO_OBJREF(ti, iface::SProS::TaskIterator, ts->iterateTasks());
  while (true)
  {
    RETURN_INTO_OBJREF(t, iface::SProS::Task, ti->nextTask());
    if (t == NULL)
      break;
    RETURN_INTO_OBJREF(s, iface::SProS::Simulation, t->simulationReference());
    DECLARE_QUERY_INTERFACE_OBJREF(ssi, s, SProS::SamplingSensitivityAnalysis);
    if (ssi == NULL)
      taskCount++;
    else
      taskCount += ssi->numberOfSamples();
  }
  if (taskCount == 0)
  {
    printf("No tasks found - probably not a valid SEDML file.\n");
    return 2;
  }

  Monitor monitor;
  RETURN_INTO_OBJREF(tms, iface::SRuS::TransformedModelSet,
                     sp->buildAllModels(sedml));
  try
  {
    sp->generateData(tms, sedml, &monitor);
  }
  catch (...)
  {
    printf("Failure processing the model; exception was raised.\n");
    exit(1);
  }
  while (monitor.tasksFinished < taskCount)
    sleep(1);
  monitor.printResults();
}
