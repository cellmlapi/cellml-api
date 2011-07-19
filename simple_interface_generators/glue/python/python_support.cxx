#define IN_PYTHONSUPPORT_MODULE
#include "python_support.hxx"

void*
P2PyFactory::createByIface(const char* aIface, PyObject* aObj)
{
  std::map<std::string, P2PyFactory*>::iterator i = sLookup.find(aIface);
  if (i == sLookup.end())
    return NULL;

  return (*i).second->create(aObj);
}

std::map<std::string, P2PyFactory*> P2PyFactory::sLookup;

char*
p2py::XPCOM::IObject::objid()
  throw(std::exception&)
{
  PyObject* ret = PyObject_CallMethod(mObject, const_cast<char*>("objid"), const_cast<char*>(""));
  if (ret == NULL)
    return strdup("");
  char* str = PyString_AsString(ret);
  return strdup(str ? str : "");
}

void*
p2py::XPCOM::IObject::query_interface(const char* id)
  throw(std::exception&)
{
  PyObject* ret = PyObject_CallMethod(mObject, const_cast<char*>("query_interface"), const_cast<char*>("s"), id);
  if (ret == Py_None)
  {
    Py_DECREF(ret);
    return NULL;
  }

  void* retpcm = P2PyFactory::createByIface(id, ret);
  Py_DECREF(ret);

  return retpcm;
}

void PyBridge_Set_Output(PyObject* aList, const char *aFormat, ...)
{
  va_list val;
  va_start(val, aFormat);
  PyBridge_VaSet_Output(aList, aFormat, val);
  va_end(val);
}

void PyBridge_VaSet_Output(PyObject* aList, const char* aFormat, va_list aArgs)
{
  PyObject* ob = Py_VaBuildValue(const_cast<char*>(aFormat), aArgs);
  // -1: Not a list. 0: Empty list.
  if (PyList_Size(aList) > 0)
    // Steals a reference to ob...
    PyList_SetItem(aList, 0, ob);
  else
  {
    PyList_Append(aList, ob);
    Py_DECREF(ob);
  }
}
