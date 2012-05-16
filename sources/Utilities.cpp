#include "Utilities.hxx"
#include <map>
#include <list>

wchar_t*
CDA_wcsdup(const wchar_t* str)
{
  size_t l = (wcslen(str) + 1) * sizeof(wchar_t);
  wchar_t* xstr = reinterpret_cast<wchar_t*>(malloc(l));
  memcpy(xstr, str, l);
  return xstr;
}

static int sWasInitialised = 0;

static std::list<std::pair<void*, void(*)(void*)> >* sAllThreadDestructors;
static ThreadLocal<std::list<std::pair<void*, void(*)(void*)> > >* tlDestructors;
static ThreadLocal<std::map<std::string, std::pair<void*,void(*)(void*)> > >* tlNamedDestructors;

static void EnsureInitialised()
{
  if (sWasInitialised)
    return;

  sWasInitialised = 1;

  sAllThreadDestructors = new std::list<std::pair<void*, void(*)(void*)> >();
  tlDestructors = new ThreadLocal<std::list<std::pair<void*, void(*)(void*)> > >(1, std::list<std::pair<void*, void(*)(void*)> >());
  tlNamedDestructors = new ThreadLocal<std::map<std::string, std::pair<void*, void(*)(void*)> > >(1, std::map<std::string, std::pair<void*, void(*)(void*)> >());
}

#define ctlDestructors static_cast<std::list<std::pair<void*, void(*)(void*)> >&>(*tlDestructors)
#define ctlNamedDestructors static_cast<std::map<std::string, std::pair<void*, void(*)(void*)> >&> (*tlNamedDestructors)

void CDA_RegisterDestructorEveryThread(void* aData, void (*aFunc)(void*))
{
  EnsureInitialised();
  sAllThreadDestructors->push_back(std::pair<void*,void(*)(void*)>(aData, aFunc));
}

void CDA_RegisterDestructorThreadLocal(void* aData, void (*aFunc)(void*))
{
  EnsureInitialised();
  ctlDestructors.push_back(std::pair<void*,void(*)(void*)>(aData, aFunc));
}

void CDA_RegisterNamedDestructorThreadLocal(const char* aName, void* aData, void (*aFunc)(void*))
{
  EnsureInitialised();
  ctlNamedDestructors.insert(std::pair<std::string, std::pair<void*,void(*)(void*)> >
                             (aName, std::pair<void*,void(*)(void*)>(aData, aFunc)));
}

void CDAThread::startthread()
{
    if (!mRunning)
    {
      mRunning = true;
#ifdef WIN32
      DWORD tid;
      HANDLE h = CreateThread(NULL, 0, ThreadProc,
                              reinterpret_cast<LPVOID>(this), 0, &tid);
      CloseHandle(h);
#else
      pthread_t thread;
      pthread_create(&thread, NULL, start_routine,
                     reinterpret_cast<void*>(this));
      pthread_detach(thread);
#endif
    }
}

void CDAThread::runThreadCleanup()
{
  EnsureInitialised();

  // On some platforms, the TLS implemenetation uses thread cleanup to call
  // the destructors. So we have to copy the list of destructors first.
  std::list<std::pair<void*, void(*)(void*)> > theDestructors
    (sAllThreadDestructors->begin(), sAllThreadDestructors->end());

  for (std::list<std::pair<void*, void(*)(void*)> >::iterator i = ctlDestructors.begin();
       i != ctlDestructors.end(); i++)
    theDestructors.push_back(*i);

  for (std::map<std::string, std::pair<void*, void(*)(void*)> >::iterator i = ctlNamedDestructors.begin();
       i != ctlNamedDestructors.end(); i++)
    theDestructors.push_back((*i).second);

  for (std::list<std::pair<void*, void(*)(void*)> >::iterator i = theDestructors.begin();
       i != theDestructors.end(); i++)
    (*i).second((*i).first);
}

#ifdef WIN32
DWORD WINAPI CDAThread::ThreadProc(LPVOID lpparam)
{
  (reinterpret_cast<CDAThread*>(lpparam))->runthread();
  runThreadCleanup();
  return 0;
}
#else
void* CDAThread::start_routine(void* arg)
{
  (reinterpret_cast<CDAThread*>(arg))->runthread();
  runThreadCleanup();
  return NULL;
}
#endif
