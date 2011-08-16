#ifndef INCLUDED_PYTHON_SUPPORT
#define INCLUDED_PYTHON_SUPPORT

#ifdef IN_PYTHONSUPPORT_MODULE
#define PUBLIC_PYTHONSUPPORT_PRE CDA_EXPORT_PRE
#define PUBLIC_PYTHONSUPPORT_POST CDA_EXPORT_POST
#else
#define PUBLIC_PYTHONSUPPORT_PRE CDA_IMPORT_PRE
#define PUBLIC_PYTHONSUPPORT_POST CDA_IMPORT_POST
#endif

#include "cda_compiler_support.h"
#undef HAVE_SYS_TYPES_H // Prevent warning when it is redefined by Python.
#include <cstdlib>
#include <exception>
#include <map>
#include <cstdio>
#include <cstring>
#include <pyport.h>
#include <object.h>
#include <objimpl.h>
#include <stringobject.h>
#include <unicodeobject.h>
#include <listobject.h>
#include <pyerrors.h>
#include <modsupport.h>
#include <methodobject.h>
#include <dictobject.h>
#include <intobject.h>
#include <cobject.h>
#include <tupleobject.h>
#include <abstract.h>
#include <import.h>
#include <pystate.h>
#include <pyarena.h>
#include <pythonrun.h>
#include <ceval.h>
#include <string>
#include "Utilities.hxx"

#ifndef Py_TYPE
#define Py_TYPE(o) ((o)->ob_type)
#endif

#ifndef PyVarObject_HEAD_INIT
#define PyVarObject_HEAD_INIT(type, size)       \
        PyObject_HEAD_INIT(type) size,
#endif

PUBLIC_PYTHONSUPPORT_PRE
class PUBLIC_PYTHONSUPPORT_POST ScopedPyObjectReference
{
public:
  ScopedPyObjectReference(PyObject* aPtr)
    : mPtr(aPtr) {}

  ~ScopedPyObjectReference()
  {
    if (mPtr != NULL)
      Py_DECREF(mPtr);
  }

private:
  PyObject* mPtr;
};

template<class A>
class PyAutoFree
{
public:
  PyAutoFree(A* aPtr)
    : mPtr(aPtr) {}

  ~PyAutoFree()
  {
    if (mPtr != NULL)
      free(mPtr);
  }

private:
  A* mPtr;
};

template<class A>
class PyOutputPtrFree
{
public:
  PyOutputPtrFree(A** aPtr)
    : mPtr(aPtr)
  {
    *aPtr = NULL;
  }

  ~PyOutputPtrFree()
  {
    if (mPtr == NULL || *mPtr == NULL)
      return;

    free(*mPtr);
  }

private:
  A** mPtr;
};

template<class A>
class PyOutputIObjectRelease
{
public:
  PyOutputIObjectRelease(A** aPtr)
    : mPtr(aPtr)
  {
    *aPtr = NULL;
  }

  ~PyOutputIObjectRelease()
  {
    if (mPtr == NULL || *mPtr == NULL)
      return;

    (*mPtr)->release_ref();
  }

private:
  A** mPtr;
};


PUBLIC_PYTHONSUPPORT_PRE void PyBridge_VaSet_Output(PyObject* aList, const char* aFormat, va_list aArgs) PUBLIC_PYTHONSUPPORT_POST;

PUBLIC_PYTHONSUPPORT_PRE void PyBridge_Set_Output(PyObject* aList, const char *aFormat, ...) PUBLIC_PYTHONSUPPORT_POST;

PUBLIC_PYTHONSUPPORT_PRE class PUBLIC_PYTHONSUPPORT_POST P2PyFactory
{
public:
  PUBLIC_PYTHONSUPPORT_PRE P2PyFactory(const char* aIfaceName) PUBLIC_PYTHONSUPPORT_POST;
  virtual void* create(PyObject* aObj) = 0;
  static void* createByIface(const char* aIface, PyObject* aObj);

private:
  static std::map<std::string, P2PyFactory*> sLookup;
};

namespace p2py
{
namespace XPCOM
{
PUBLIC_PYTHONSUPPORT_PRE class PUBLIC_PYTHONSUPPORT_POST IObject
  : public virtual iface::XPCOM::IObject
{
public:
  IObject()
    : _cda_refcount(1), mObject(NULL)
  {
  }

  CDA_IMPL_REFCOUNT;

  IObject(PyObject* aObject)
    : mObject(aObject)
  {
    Py_INCREF(mObject);
  }

  ~IObject()
  {
    if (mObject)
      Py_DECREF(mObject);
  }

  PyObject* unwrap()
  {
    if (mObject)
      Py_INCREF(mObject);
    return mObject;
  }

  PUBLIC_PYTHONSUPPORT_PRE void* query_interface(const char* id)
    throw(std::exception&) PUBLIC_PYTHONSUPPORT_POST;

  PUBLIC_PYTHONSUPPORT_PRE char* objid() throw(std::exception&) PUBLIC_PYTHONSUPPORT_POST;

protected:
  PyObject* mObject;
};
};
};

class ScopedPyGIL
{
public:
  ScopedPyGIL()
  {
    PyEval_InitThreads();
    mSt = PyGILState_Ensure();
  }

  ~ScopedPyGIL()
  {
    PyGILState_Release(mSt);
  }

private:
  PyGILState_STATE mSt;
};

struct PyPCMObject
{
  PyObject_HEAD
  PyObject *pcm_dict;
};

#endif
