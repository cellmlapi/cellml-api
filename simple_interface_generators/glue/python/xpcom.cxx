#include "python_support.hxx"

static PyObject*
get_xpcom_IObject_objid(PyObject* aSelf, PyObject* aArgs, PyObject* aKwds)
{
  PyObject* cobj = PyObject_GetAttrString(aSelf, "_iobject_cptr");
  if (cobj == NULL)
  {
    PyErr_SetString(PyExc_ValueError, "Call to native PCM method when applicable cptr not set. Has __init__ been successfully called?");
    return NULL;
  }
  if (!PyCObject_Check(cobj))
  {
    Py_DECREF(cobj);
    PyErr_SetString(PyExc_ValueError, "cptr attribute is not CObject");
    return NULL;
  }
  iface::XPCOM::IObject * native = reinterpret_cast<iface::XPCOM::IObject*>(PyCObject_AsVoidPtr(cobj));
  std::string ret = native->objid();
  return PyString_FromString(ret.c_str());
}

static PyMethodDef xpcom_IObjectMethods[] = {
  {"objid",(PyCFunction)get_xpcom_IObject_objid,METH_VARARGS,NULL},
  {"supported_interfaces",(PyCFunction)get_xpcom_IObject_objid,METH_VARARGS,NULL},
  {NULL, NULL, 0, NULL}
};
static PyTypeObject xpcom_IObjectType = {
  PyObject_HEAD_INIT(NULL)
  0,/* ob_size */
  "xpcom.IObject",/* tp_name */
  sizeof(xpcom_IObjectType),/* tp_basicsize */
  0,/* tp_itemsize */
  0,/* tp_dealloc */
  0,/* tp_print */
  0,/* tp_getattr */
  0,/* tp_setattr */
  0,/* tp_compare */
  0,/* tp_repr */
  0,/* tp_as_number */
  0,/* tp_as_sequence */
  0,/* tp_as_mapping */
  0,/* tp_hash */
  0,/* tp_call */
  0,/* tp_str */
  0,/* tp_getattro */
  0,/* tp_setattro */
  0,/* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,/* tp_flags */
  0,/* tp_doc */
  0,/* tp_traverse */
  0,/* tp_clear */
  0,/* tp_richcompare */
  0,/* tp_weaklistoffset */
  0,/* tp_iter */
  0,/* tp_iternext */
  xpcom_IObjectMethods,/* tp_methods */
  0,/* tp_members */
  0,/* tp_getset */
  0,/* tp_base */
  0,/* tp_dict */
  0,/* tp_descr_get */
  0,/* tp_descr_set */
  0,/* tp_dictoffset */
  0,/* tp_init */
  0,/* tp_alloc */
  0,/* tp_new */
  0,/* tp_free */
  0,/* tp_is_gc */
  0,/* tp_bases */
  0,/* tp_mro */
  0,/* tp_cache */
  0,/* tp_subclasses */
  0,/* tp_weaklist */
  0,/* tp_del */
};

extern "C" { CDA_EXPORT_PRE void initxpcom(void) CDA_EXPORT_POST; }
void
initxpcom(void)
{
  PyObject* xpcomMod = Py_InitModule("xpcom", NULL);
  PyType_Ready(&xpcom_IObjectType);
  PyModule_AddObject(xpcomMod, "IObject", (PyObject*)&xpcom_IObjectType);
}
