// A wrapper for a mutex...
class CDAMutex
{
public:
  CDAMutex()
  {
#ifdef ENABLE_THREADSAFETY
    if (!g_thread_supported())
      g_thread_init(NULL);
    gmutex = g_mutex_new();
#endif
  }

  ~CDAMutex()
  {
#ifdef ENABLE_THREADSAFETY
    g_mutex_free(gmutex);
#endif
  }

  void Lock()
  {
#ifdef ENABLE_THREADSAFETY
    g_mutex_lock(gmutex);
#endif
  }

  void Unlock()
  {
#ifdef ENABLE_THREADSAFETY
    g_mutex_unlock(gmutex);
#endif
  }
private:
#ifdef ENABLE_THREADSAFETY
  GMutex* gmutex;
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
