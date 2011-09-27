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

P2PyFactory::P2PyFactory(const char* aIfaceName)
{
  sLookup.insert(std::pair<std::string, P2PyFactory*>(aIfaceName, this));
}

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

char**
p2py::XPCOM::IObject::supported_interfaces(uint32_t* len) throw()
{
  PyObject* o = PyObject_GetAttrString(mObject, const_cast<char*>("supported_interfaces"));
  if (!o || !PySequence_Check(o))
  {
    PyErr_Clear();
    *len = 1;
    char** ret = static_cast<char**>(malloc(sizeof(char*)));
    *ret = strdup("xpcom::IObject");
    if (o != NULL)
      Py_DECREF(o);
    return ret;
  }

  uint32_t l = PySequence_Length(o);
  char** ret = static_cast<char**>(malloc(sizeof(char*) * (l + 1)));
  *ret = strdup("xpcom::IObject");
  uint32_t j = 1;
  for (uint32_t i = 0; i < l; i++)
  {
    PyObject* so = PySequence_GetItem(o, i);
    if (PyString_Check(so))
      ret[j++] = strdup(PyString_AS_STRING(so));

    Py_DECREF(so);
  }

  *len = j;
  Py_DECREF(o);

  return ret;
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

PyObject*
PyBridge_AutoQI_GetAttr(PyObject* obj, PyObject* name)
{
  PyObject* ret = PyObject_GenericGetAttr(obj, name);
  if (ret)
    return ret;

  PyObject* cobj = PyObject_GetAttrString(aSelf, "_iobject_cptr");
  if (cobj == NULL)
  {
    PyErr_SetString(PyExc_ValueError, "Call to native PCM method when applicable cptr not set. Has __init__ been successfully called?");
    return NULL;
  }
}
