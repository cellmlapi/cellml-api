// A wrapper for a mutex...
class CDAMutex
{
public:
  CDAMutex()
  {
    gmutex = g_mutex_new();
  }

  ~CDAMutex()
  {
    g_mutex_free(gmutex);
  }

  void Lock()
  {
    g_mutex_lock(gmutex);
  }

  void Unlock()
  {
    g_mutex_unlock(gmutex);
  }
private:
  GMutex* gmutex;
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
