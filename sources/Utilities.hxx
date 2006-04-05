#ifndef _UTILITIES_HXX
#define _UTILITIES_HXX

#include "config.h"

#if SIZEOF_WCHAR_TP == 4
#define WCHAR_T_IS_32BIT
#elif SIZEOF_WCHAR_TP != 2
#error "Only UTF16 and UCS4 wide characters are supported, but your "
#error "compiler has a different length type."
#endif

#include <list>
#include <exception>
#include <Ifacexpcom.hxx>
#include <wchar.h>

#define CDA_IMPL_REFCOUNT \
  private: \
    uint32_t _cda_refcount; \
  public: \
    void add_ref() \
      throw() \
    { \
      _cda_refcount++; \
    } \
    void release_ref() \
      throw() \
    { \
      _cda_refcount--; \
      if (_cda_refcount == 0) \
        delete this; \
    }

#define CDA_IMPL_COMPARE_NAIVE(c1) \
    int32_t compare(iface::XPCOM::IObject* obj) \
      throw(std::exception&) \
    { \
      return (reinterpret_cast<char*>(static_cast<iface::XPCOM::IObject*> \
                                      (this)) - \
              reinterpret_cast<char*>(obj)); \
    }

#define CDA_IMPL_QI0 \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
      { \
        add_ref(); \
        return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI1(c1) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1)) \
      { \
        add_ref(); \
        return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI2(c1, c2) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2)) \
      { \
         add_ref(); \
         return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI3(c1, c2, c3) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2) || \
          !strcmp(id, #c3)) \
      { \
         add_ref(); \
         return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI4(c1, c2, c3, c4) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2) || \
          !strcmp(id, #c3) || \
          !strcmp(id, #c4)) \
      { \
         add_ref(); \
         return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI5(c1, c2, c3, c4, c5) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2) || \
          !strcmp(id, #c3) || \
          !strcmp(id, #c4) || \
          !strcmp(id, #c5)) \
      { \
        add_ref(); \
        return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI6(c1, c2, c3, c4, c5, c6) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2) || \
          !strcmp(id, #c3) || \
          !strcmp(id, #c4) || \
          !strcmp(id, #c5) || \
          !strcmp(id, #c6)) \
      { \
        add_ref(); \
        return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI7(c1, c2, c3, c4, c5, c6, c7) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2) || \
          !strcmp(id, #c3) || \
          !strcmp(id, #c4) || \
          !strcmp(id, #c5) || \
          !strcmp(id, #c6) || \
          !strcmp(id, #c7)) \
      { \
        add_ref(); \
        return this; \
      } \
      return NULL; \
    }

#define CDA_IMPL_QI8(c1, c2, c3, c4, c5, c6, c7, c8) \
    iface::XPCOM::IObject* query_interface(const char* id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object") || \
          !strcmp(id, #c1) || \
          !strcmp(id, #c2) || \
          !strcmp(id, #c3) || \
          !strcmp(id, #c4) || \
          !strcmp(id, #c5) || \
          !strcmp(id, #c6) || \
          !strcmp(id, #c7) || \
          !strcmp(id, #c8)) \
      { \
        add_ref(); \
        return this; \
      } \
      return NULL; \
    }

#ifdef USE_GDOME // Various code only used with GDOME...

#ifdef WCHAR_T_IS_32BIT
#define g_wchar_to_char g_ucs4_to_utf8
#define g_char_to_wchar g_utf8_to_ucs4_fast
#define gwchar_t gunichar
#define POSSIBLE_EXTRA_NULLS
#else
#define g_wchar_to_char g_utf16_to_utf8
#define g_char_to_wchar g_utf8_to_utf16
#define gwchar_t gunichar2
#define POSSIBLE_EXTRA_NULLS ,NULL, NULL
#endif

#define TRDOMSTRING(x) \
  GdomeDOMString *gd##x = gdome_str_mkref_own(g_wchar_to_char((const gwchar_t*)x, -1, NULL, NULL, NULL))

#define TRDOMSTRING_EMPTYNULL(x) \
  GdomeDOMString *gd##x; \
  if (!wcscmp(x, L"")) \
    gd##x = NULL; \
  else \
    gd##x = gdome_str_mkref_own(g_wchar_to_char((const gwchar_t*)x, -1, NULL, NULL, NULL))

#define DDOMSTRING(x) \
  if (gd##x != NULL) \
    gdome_str_unref(gd##x)

#define TRGDOMSTRING(x) \
  iface::dom::DOMString cxx##x = ((x) && (x)->str) ? ((wchar_t*)g_char_to_wchar(x->str, -1, NULL POSSIBLE_EXTRA_NULLS)) : CDA_wcsdup(L""); \
  if (x) \
    gdome_str_unref(x);

#define EXCEPTION_TRY \
  GdomeException exc;  

#define EXCEPTION_CATCH \
  if (GDOME_EXCEPTION_CODE(exc) != GDOME_NOEXCEPTION_ERR) \
    throw iface::dom::DOMException();

#define LOCALCONVERT(x, t) \
  const CDA_##t* l##x = dynamic_cast<const CDA_##t*>(x); \
  if (l##x == NULL) \
    throw iface::dom::DOMException();

#define LOCALCONVERT_NULLOK(x, t) \
  const CDA_##t* l##x; \
  if (x == NULL) \
    l##x = NULL; \
  else \
  { \
    l##x = dynamic_cast<const CDA_##t*>(x); \
  if (l##x == NULL) \
    throw iface::dom::DOMException(); \
  }

#endif // USE_GDOME

template<class T>
class already_AddRefd
{
public:
  already_AddRefd(T* aPtr)
    : mPtr(aPtr)
  {
  }

  ~already_AddRefd()
  {
  }

  operator T*() const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }
private:
  T* mPtr;
};

template<class T>
class ObjRef
{
public:
  ObjRef()
    : mPtr(NULL)
  {
  }

  ObjRef(const ObjRef<T>& aPtr)
  {
    mPtr = aPtr.getPointer();
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  ObjRef(T* aPtr)
    : mPtr(aPtr)
  {
    mPtr->add_ref();
  }

  ObjRef(const already_AddRefd<T> aar)
  {
    mPtr = aar.getPointer();
  }

  ~ObjRef()
  {
    if (mPtr != NULL)
      mPtr->release_ref();
  }

  T* operator-> () const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }

  operator T* () const
  {
    return mPtr;
  }

  void operator= (T* newAssign)
  {
    if (mPtr == newAssign)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = newAssign;
    if (newAssign != NULL)
      mPtr->add_ref();
  }

  // We need these explicit forms or the default overloads the templates below.
  void operator= (const already_AddRefd<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  void operator= (const ObjRef<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  template<class U>
  void operator= (const already_AddRefd<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  template<class U>
  void operator= (const ObjRef<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

private:
  T* mPtr;
};

template<class T, class U> bool
operator==(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() == rhs.getPointer());
}

template<class T, class U> bool
operator!=(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() != rhs.getPointer());
}

#define RETURN_INTO_WSTRING(lhs, rhs) \
  wchar_t* tmp_##lhs = rhs; \
  std::wstring lhs(tmp_##lhs); \
  free(tmp_##lhs);

#define RETURN_INTO_OBJREF(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      static_cast<type*> \
      ( \
        rhs \
      ) \
    )\
  )

#define RETURN_INTO_OBJREFD(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      dynamic_cast<type*> \
      ( \
        rhs \
      ) \
    )\
  )


template<class T> class WeakReference;

template<class T>
class WeakReferenceTarget
{
public:
  WeakReferenceTarget()
  {
  }

  ~WeakReferenceTarget()
  {
    typename std::list<WeakReference<T>*>::iterator i;
    for (i = mActiveReferences.begin();
         i != mActiveReferences.end();
         i++)
      (*i)->targetWasDestroyed();
  }

  void
  referenceCreated(WeakReference<T>* wr)
  {
    mActiveReferences.push_back(wr);
  }

  void
  referenceDestroyed(WeakReference<T>* wr)
  {
    mActiveReferences.remove(wr);
  }
private:
  std::list<WeakReference<T>*> mActiveReferences;
};

template<class T>
class WeakReference
{
public:
  WeakReference()
    : mReferTo(NULL)
  {
  }

  WeakReference(WeakReferenceTarget<T>* aReferTo)
    : mReferTo(aReferTo)
  {
    mReferTo->referenceCreated(this);
  }

  ~WeakReference()
  {
    if (mReferTo != NULL)
      mReferTo->referenceDestroyed(this);
  }

  void
  operator= (WeakReferenceTarget<T>* aReferTo)
  {
    if (aReferTo == mReferTo)
      return;

    if (mReferTo != NULL)
      mReferTo->referenceDestroyed(this);

    mReferTo = aReferTo;
    mReferTo->referenceCreated(this);
  }

  void
  targetWasDestroyed()
  {
    mReferTo = NULL;
  }

  T* operator() ()
  {
    return static_cast<T*>(mReferTo);
  }
private:
  WeakReferenceTarget<T>* mReferTo;
};

#define QUERY_INTERFACE(lhs, rhs, type) \
  if (rhs != NULL) \
  { \
    iface::XPCOM::IObject* _qicast_obj = rhs->query_interface(#type); \
    if (_qicast_obj != NULL) \
    { \
      lhs = already_AddRefd<iface::type>(dynamic_cast<iface::type*>(_qicast_obj)); \
      if (lhs == NULL) \
      { \
        _qicast_obj->release_ref(); \
      } \
    } \
    else \
      lhs = NULL; \
  } \
  else \
    lhs = NULL;

#define QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  QUERY_INTERFACE(lhs, rhs, type) \
  if (rhs != NULL) \
  { \
    rhs->release_ref(); \
    rhs = NULL; \
  }

#define DECLARE_QUERY_INTERFACE(lhs, rhs, type) \
  iface::type* lhs; \
  QUERY_INTERFACE(lhs, rhs, type)

#define DECLARE_QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  iface::type* lhs; \
  QUERY_INTERFACE_REPLACE(lhs, rhs, type)

#define DECLARE_QUERY_INTERFACE_OBJREF(lhs, rhs, type) \
  ObjRef<iface::type> lhs; \
  QUERY_INTERFACE(lhs, rhs, type)

inline bool hasInterface(iface::XPCOM::IObject* obj, const char* iface)
{
  if (obj == NULL)
    return false;
  iface::XPCOM::IObject* ret = obj->query_interface(iface);
  if (ret == NULL)
    return false;
  ret->release_ref();
  return true;
}

inline bool isEqualAfterLeftQI(iface::XPCOM::IObject* lhs,
                               const iface::XPCOM::IObject* rhs,
                               const char* type)
{
  iface::XPCOM::IObject* lhsQI = NULL;
  if (lhs != NULL)
    lhsQI = lhs->query_interface(type);

  bool eq = (lhsQI == rhs);

  if (lhsQI != NULL)
    lhsQI->release_ref();

  return eq;
}

struct XPCOMComparator
{
  bool
  operator()(
             iface::XPCOM::IObject* o1,
             iface::XPCOM::IObject* o2
            ) const
  {
    /* In the strict ordering, NULL < x unless x == NULL. */
    if (o1 == NULL)
      return o2 ? true : false;
    return (o1->compare(o2) < 0);
  }
};

wchar_t* CDA_wcsdup(const wchar_t* str);

#endif // _UTILITIES_HXX
