#define CDA_IMPL_REFCOUNT \
  private: \
    u_int32_t _cda_refcount; \
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

#define CDA_IMPL_QI1(c1) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
         return this; \
      return NULL; \
    }

#define CDA_IMPL_QI2(c1, c2) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
         return this; \
      if (!strcmp(id, #c2)) \
         return this; \
      return NULL; \
    }

#define CDA_IMPL_QI3(c1, c2, c3) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
         return this; \
      if (!strcmp(id, #c2)) \
         return this; \
      if (!strcmp(id, #c3)) \
         return this; \
      return NULL; \
    }

#define CDA_IMPL_QI4(c1, c2, c3, c4) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
         return this; \
      if (!strcmp(id, #c2)) \
         return this; \
      if (!strcmp(id, #c3)) \
         return this; \
      if (!strcmp(id, #c4)) \
         return this; \
      return NULL; \
    }

#define CDA_IMPL_QI5(c1, c2, c3, c4, c5) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
        return this; \
      if (!strcmp(id, #c2)) \
        return this; \
      if (!strcmp(id, #c3)) \
        return this; \
      if (!strcmp(id, #c4)) \
        return this; \
      if (!strcmp(id, #c5)) \
        return this; \
      return NULL; \
    }

#define CDA_IMPL_QI6(c1, c2, c3, c4, c5, c6) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
        return this; \
      if (!strcmp(id, #c2)) \
        return this; \
      if (!strcmp(id, #c3)) \
        return this; \
      if (!strcmp(id, #c4)) \
        return this; \
      if (!strcmp(id, #c5)) \
        return this; \
      if (!strcmp(id, #c6)) \
        return this; \
      return NULL; \
    }

#define CDA_IMPL_QI7(c1, c2, c3, c4, c5, c6, c7) \
    IObject* query_interface(const iface::XPCOM::utf8string id) \
      throw(std::exception&) \
    { \
      if (!strcmp(id, "xpcom::Object")) \
        return this; \
      if (!strcmp(id, #c1)) \
        return this; \
      if (!strcmp(id, #c2)) \
        return this; \
      if (!strcmp(id, #c3)) \
        return this; \
      if (!strcmp(id, #c4)) \
        return this; \
      if (!strcmp(id, #c5)) \
        return this; \
      if (!strcmp(id, #c6)) \
        return this; \
      if (!strcmp(id, #c7)) \
        return this; \
      return NULL; \
    }

#define TRDOMSTRING(x) \
  GdomeDOMString *gd##x = gdome_str_mkref_own(g_utf16_to_utf8((const gunichar2*)x, -1, NULL, NULL, NULL))
#define DDOMSTRING(x) \
  gdome_str_unref(gd##x)

#define TRGDOMSTRING(x) \
  iface::dom::DOMString cxx##x = x ? ((wchar_t*)g_utf8_to_utf16(x->str, -1, NULL, NULL, NULL)) : wcsdup(L""); \
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

  T* getPointer()
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

  ObjRef(T* aPtr)
    : mPtr(aPtr)
  {
    mPtr->add_ref();
  }

  ObjRef(already_AddRefd<T> aar)
  {
    mPtr = aar.getPointer();
  }

  ~ObjRef()
  {
    if (mPtr != NULL)
      mPtr->release_ref();
  }

  T* operator-> ()
  {
    return mPtr;
  }

  T* operator& () const
  {
    return mPtr;
  }

  T* getPointer()
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

  void operator= (already_AddRefd<T> newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  void operator= (ObjRef<T> newAssign)
  {
    T* nap = newAssign->getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    nap->add_ref();
  }
private:
  T* mPtr;
};

#define RETURN_INTO_WSTRING(lhs, rhs) \
  wchar_t* tmp_##lhs = rhs; \
  std::wstring lhs(tmp_##lhs); \
  free(tmp_##lhs);
