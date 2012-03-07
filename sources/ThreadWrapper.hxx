#ifdef WIN32
#include <windows.h>
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
    InitializeCriticalSection(&mMutex);
#else
    pthread_mutex_init(&mMutex, NULL);
#endif
  }

  ~CDAMutex()
  {
#ifdef WIN32
    DeleteCriticalSection(&mMutex);
#else
    pthread_mutex_destroy(&mMutex);
#endif
  }

  void Lock()
  {
#ifdef WIN32
    EnterCriticalSection(&mMutex);
#else
    pthread_mutex_lock(&mMutex);
#endif
  }

  void Unlock()
  {
#ifdef WIN32
    LeaveCriticalSection(&mMutex);
#else
    pthread_mutex_unlock(&mMutex);
#endif
  }
private:
#ifdef WIN32
  CRITICAL_SECTION mMutex;
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

class CDAThread
{
public:
  CDAThread()
    : mRunning(false)
  {
  }

  virtual ~CDAThread() {}

  void startthread()
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

protected:
  virtual void runthread()
  {
  }

private:
#ifdef WIN32
  static DWORD WINAPI ThreadProc(LPVOID lpparam)
  {
    (reinterpret_cast<CDAThread*>(lpparam))->runthread();
    return 0;
  }
#else
  static void* start_routine(void* arg)
  {
    (reinterpret_cast<CDAThread*>(arg))->runthread();
    return NULL;
  }
#endif
  bool mRunning;
};
