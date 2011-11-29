# -*- python -*-
import os.path
import identifier
import typeinfo
import string
from omniidl import idlast, idlvisitor, idlutil, idltype, output

class PythonToCWalker(idlvisitor.AstVisitor):
    """Walks over the AST once and writes the Python wrapper in the process"""

    def visitAST(self, node):
        """Visit all the declarations in an AST"""

        self.out.out(
            "// This output is automatically generated. Do not edit.")
        
        self.out.out('#include "python_support.hxx"')
        self.out.out('#include "Iface' + node.filebase + '.hxx"')
        self.out.out('#include "P2Py' + node.filebase + '.hxx"')
        
        self.module = node.filebase

        for [k, v] in self.specialIncludes:
            if k == self.module:
                self.out.out('#include "%s"' % v)
        
        for n in node.declarations():
            n.accept(self)

        self.out.out('extern "C" { CDA_EXPORT_PRE void\ninit' + self.modulename + "(void) CDA_EXPORT_POST; }")
        self.out.out("PyMODINIT_FUNC\ninit" + self.modulename + "(void)\n{")
        self.out.inc_indent()

        self.out.out("PyObject* mod = Py_InitModule(\"%s\", NULL);" % (self.modulename))
        for n in node.declarations():
            if n.mainFile() and isinstance(n, idlast.Module):
                for n2 in n.definitions():
                    if not n2.mainFile():
                        pass
                    elif isinstance(n2, idlast.Interface) or isinstance(n2, idlast.Exception):
                        self.out.out("%sFinaliseType();" % n2.simplecscoped)
                        self.out.out("PyModule_AddObject(mod, \"%s\", (PyObject*)&%sType);" %
                                     (
                                      string.upper(n2.identifier()[0]) + n2.identifier()[1:],
                                      n2.simplecscoped
                                     )
                                    )
                    elif isinstance(n2, idlast.Enum):
                        self.out.out('{')
                        self.out.inc_indent()
                        self.out.out('PyObject* edict = PyDict_New();')
                        for en in n2.enumerators():
                            self.out.out('{')
                            self.out.inc_indent()
                            self.out.out('PyObject* env = PyInt_FromLong(%u);' % (en.value()))
                            self.out.out('PyDict_SetItemString(edict, "%s", env);' % en.identifier())
                            self.out.dec_indent()
                            self.out.out('}')
                        self.out.out('PyModule_AddObject(mod, "%s", edict);' %
                                     (n2.identifier()))
                        self.out.dec_indent()
                        self.out.out('}')
        self.out.dec_indent()
        self.out.out("}")

    def visitModule(self, node):
        """Visit all the definitions in a module."""
        for n in node.definitions():
            if n.mainFile():
                n.accept(self)

    def visitForward(self, node):
        pass

    def recursivelyPopulateCPTRs(self, node):
        if isinstance(node, idlast.Declarator) and node.alias():
            node = node.alias().aliasType().unalias().decl()
        if node.corbacxxscoped == 'XPCOM::IObject':
            return
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyObject* cptr_obj = PyCObject_FromVoidPtr(static_cast<%s*>(tobj), NULL);' % node.simplecxxscoped)
        self.out.out('PyObject_SetAttrString(aSelf, "_iobject_%s_cptr", cptr_obj);' % node.simplecscoped)
        self.out.out('Py_DECREF(cptr_obj);')
        self.out.dec_indent()
        self.out.out('}')
        for inh in node.inherits():
            self.recursivelyPopulateCPTRs(inh)

    def visitInterface(self, node):
        self.methods = []
        self.iface = node
        
        for n in node.contents():
            n.accept(self)

        # The initialisation function for objects...
        self.out.out('static int ' + node.simplecscoped +
                     'Init(PyObject* aSelf, PyObject* aArgs, PyObject* aKwds)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_Clear();')
        self.out.out('PyObject* iobject;')
        self.out.out('%s * tobj = NULL;' % node.simplecxxscoped)
        self.out.out('if (!PyArg_ParseTuple(aArgs, "O", &iobject))')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_Clear();')
        # See if this is a bootstrap special...
        bootstrapped = 0
        for [name, bootstrap] in self.bootstrapSpecials:
            if (not bootstrapped) and (name == node.corbacxxscoped):
                self.out.out('tobj = @bootstrap@();', bootstrap=bootstrap)
                bootstrapped = 1
        if not bootstrapped:
            self.out.out('PyErr_SetString(PyExc_TypeError, "Too few arguments to query_interface");')
            self.out.out('return -1;')
        self.out.dec_indent()
        self.out.out('}')

        self.out.out('if (tobj == NULL)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('Py_INCREF(iobject);')
        self.out.out('if (PyCObject_Check(iobject))')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('iface::XPCOM::IObject* obj = reinterpret_cast<iface::XPCOM::IObject*>(PyCObject_AsVoidPtr(iobject));')
        self.out.out('tobj = reinterpret_cast<%s*>(obj->query_interface("%s"));' %
                     (node.simplecxxscoped, node.corbacxxscoped))
        self.out.out('if (tobj == NULL)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_SetString(PyExc_ValueError, "query_interface: Object doesn\'t ' +
                     'support %s interface");' % node.corbacxxscoped)
        self.out.dec_indent()
        self.out.out('}')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('else') # not a PyCObject
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyObject* cptr = PyObject_GetAttrString(iobject, "_iobject_cptr");')
        self.out.out('if (cptr == NULL)')
        self.out.out('  PyErr_SetString(PyExc_TypeError, "Argument to py2pcm constructor is not a CObject or another py2pcm object");')
        self.out.out('else')
        self.out.out('{')
        
        self.out.inc_indent()
        self.out.out('if (PyCObject_Check(cptr))')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('iface::XPCOM::IObject* obj = reinterpret_cast<iface::XPCOM::IObject*>(PyCObject_AsVoidPtr(cptr));')
        self.out.out('tobj = reinterpret_cast<%s*>(obj->query_interface("%s"));' %
                     (node.simplecxxscoped, node.corbacxxscoped))
        self.out.out('if (tobj == NULL)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_SetString(PyExc_ValueError, "query_interface: Object doesn\'t ' +
                     'support %s interface");' % node.corbacxxscoped)
        self.out.dec_indent()
        self.out.out('}')
        
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('Py_DECREF(cptr);')
        self.out.dec_indent()
        self.out.out('}')
                
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('Py_DECREF(iobject);')

        self.out.dec_indent()
        self.out.out('}')

        self.out.out('if (tobj)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('{')
        self.out.inc_indent()        
        self.out.out('PyObject* cptr_obj = PyCObject_FromVoidPtr(static_cast<iface::XPCOM::IObject*>(tobj), NULL);')
        self.out.out('PyObject_SetAttrString(aSelf, "_iobject_cptr", cptr_obj);')
        self.out.out('Py_DECREF(cptr_obj);')
        self.out.dec_indent()
        self.out.out('}')
        self.recursivelyPopulateCPTRs(node)
        self.out.out('return 0;')
        self.out.dec_indent()
        self.out.out('}')
        
        self.out.out('return -1;')
        self.out.dec_indent()
        self.out.out('};')

        # Methods provided...
        self.out.out('static PyMethodDef %sMethods[] = {' % node.simplecscoped)
        self.out.inc_indent()
        for name, funcname in self.methods:
            self.out.out('{"%s",(PyCFunction)%s,METH_VARARGS,NULL},'%(name, funcname))
        self.out.out('{NULL, NULL, 0, NULL}')
        self.out.dec_indent()
        self.out.out('};')
        self.out.out('static void ' + node.simplecscoped + 'Del(PyObject* aSelf)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_Clear();')
        self.out.out('PyObject* cptr = PyObject_GetAttrString(aSelf, "_iobject_cptr");')
        self.out.out('if (cptr)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('reinterpret_cast<iface::XPCOM::IObject*>(PyCObject_AsVoidPtr(cptr))->release_ref();')
        self.out.out('Py_DECREF(cptr);')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('Py_TYPE(aSelf)->tp_free(aSelf);')
        self.out.dec_indent()
        self.out.out('}')

        # Type datastructure...
        self.out.out("static PyTypeObject " + node.simplecscoped + "Type = {")
        self.out.inc_indent()
        self.out.out("PyVarObject_HEAD_INIT(&PyType_Type, 0)")
        self.out.out('"' + self.moduledir + '.' + self.modulename + '.' + node.identifier() +
                     '\",/* tp_name */')
        self.out.out('sizeof(PyPCMObject),/* tp_size */')
        self.out.out('0,/* tp_itemsize */')
        self.out.out(node.simplecscoped + 'Del,/* tp_dealloc */')
        self.out.out('0,/* tp_print */')
        self.out.out('0,/* tp_getattr */')
        self.out.out('0,/* tp_setattr */')
        self.out.out('0,/* tp_compare */')
        self.out.out('0,/* tp_repr */')
        self.out.out('0,/* tp_as_number */')
        self.out.out('0,/* tp_as_sequence */')
        self.out.out('0,/* tp_as_mapping */')
        self.out.out('0,/* tp_hash */')
        self.out.out('0,/* tp_call */')
        self.out.out('0,/* tp_str */')
        self.out.out('0,/* tp_getattro */')
        self.out.out('0,/* tp_setattro */')
        self.out.out('0,/* tp_as_buffer */')
        self.out.out('Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */')
        self.out.out('0,/* tp_doc */')
        self.out.out('0,/* tp_traverse */')
        self.out.out('0,/* tp_clear */')
        self.out.out('0,/* tp_richcompare */')
        self.out.out('0,/* tp_weaklistoffset */')
        self.out.out('0,/* tp_iter */')
        self.out.out('0,/* tp_iternext */')
        self.out.out(node.simplecscoped + 'Methods,/* tp_methods */')
        self.out.out('0,/* tp_members */')
        self.out.out('0,/* tp_getset */')
        self.out.out('0,/* tp_base */')
        self.out.out('0,/* tp_dict */')
        self.out.out('0,/* tp_descr_get */')
        self.out.out('0,/* tp_descr_set */')
        self.out.out('offsetof(PyPCMObject, pcm_dict),/* tp_dictoffset */')
        self.out.out(node.simplecscoped + 'Init,/* tp_init */')
        self.out.out('0,/* tp_alloc */')
        self.out.out('PyType_GenericNew,/* tp_new */')
        self.out.out('0,/* tp_free */')
        self.out.out('0,/* tp_is_gc */')
        self.out.out('0,/* tp_bases */')
        self.out.out('0,/* tp_mro */')
        self.out.out('0,/* tp_cache */')
        self.out.out('0,/* tp_subclasses */')
        self.out.out('0,/* tp_weaklist */')
        self.out.out('0,/* tp_del */')
        self.out.dec_indent()
        self.out.out('};')

        # FinaliseType function used to set up type...
        self.out.out('static void ' + node.simplecscoped + 'FinaliseType()')
        self.out.out('{')
        self.out.inc_indent()
        if node.simplecscoped != 'XPCOM_IObject':
            bases = []
            nextbase = 0
            for base in node.inherits():
                if isinstance(base, idlast.Declarator) and base.alias():
                    base = base.alias().aliasType().unalias().decl()
                
                if base.file() == node.file():
                    bases.append('&' + base.simplecscoped + 'Type')
                else:
                    bn = 'base%u' % nextbase
                    nextbase = nextbase + 1
                    self.out.out('PyObject* %s;' % bn)
                    directory, filename = os.path.split(base.file())
                    filebase, extension = os.path.splitext(filename)
                    self.out.out('{')
                    self.out.inc_indent()
                    self.out.out('PyObject* %sMod = PyImport_ImportModule("%s.%s");' %
                                 (bn, self.moduledir, filebase))
                    self.out.out('if (%sMod == NULL) return;' % bn)
                    self.out.out('%s = PyObject_GetAttrString(%sMod, "%s");' % (bn, bn, base.identifier()))
                    self.out.out('Py_DECREF(%sMod);' % bn)
                    self.out.dec_indent()
                    self.out.out('}')
                    bases.append(bn)
            self.out.out('%sType.tp_bases = PyTuple_Pack(%u, %s);' %
                         (node.simplecscoped, len(bases), string.join(bases, ', ')))
            for i in range(0, nextbase):
                self.out.out('Py_DECREF(base%u);' % i)
        self.out.out('PyType_Ready(&%sType);' % node.simplecscoped)
        self.out.dec_indent()
        self.out.out('};')

    def raiseExceptionInContext(self, exNode, nodeContext):
        if nodeContext.file() == exNode.file():
            self.out.out('PyErr_SetString((PyObject*)&%sType, "Native exception raised");' %\
                         exNode.simplecscoped)
        else:
            self.out.out('{')
            self.out.inc_indent()

            # directory, filename = os.path.split(nodeContext.file())
            # filebase, extension = os.path.splitext(filename)
            
            self.out.out('PyObject* raiseMod = PyImport_ImportModule("%s.%s");' % (self.moduledir, self.modulename))
            self.out.out('ScopedPyObjectReference raiseMod_release(raiseMod);')
            self.out.out('if (raiseMod != NULL)')
            self.out.out('{')
            self.out.inc_indent()
            self.out.out('PyObject* raise = PyObject_GetAttrString(raiseMod, "@n@");', n=exNode.identifier())
            self.out.out('if (raise != NULL)')
            self.out.out('{')
            self.out.inc_indent()
            self.out.out('PyErr_SetString(raise, "Native exception raised");')
            self.out.out('Py_DECREF(raise);')
            self.out.dec_indent()
            self.out.out('}')
            
            self.out.dec_indent()
            self.out.out('}')
            
            self.out.dec_indent()
            self.out.out('}')
    
    def visitTypedef(self, node):
        pass

    def visitMember(self, node):
        pass

    def visitStruct(self, node):
        raise "Structs are not supported"

    def visitUnion(self, node):
        raise "Unions are not supported"

    def visitEnumerator(self, node):
        pass

    def visitEnum(self, node):
        pass
    
    def visitException(self, node):
        # Type datastructure...
        self.out.out("static PyTypeObject " + node.simplecscoped + "Type = {")
        self.out.inc_indent()
        self.out.out("PyObject_HEAD_INIT(NULL)")
        self.out.out("0,/* ob_size */")
        self.out.out('"' + self.moduledir + '.' + self.modulename + '.' + node.identifier() +
                     '\",/* tp_name */')
        self.out.out('0,/* tp_basicsize */')
        self.out.out('0,/* tp_itemsize */')
        self.out.out('0,/* tp_dealloc */')
        self.out.out('0,/* tp_print */')
        self.out.out('0,/* tp_getattr */')
        self.out.out('0,/* tp_setattr */')
        self.out.out('0,/* tp_compare */')
        self.out.out('0,/* tp_repr */')
        self.out.out('0,/* tp_as_number */')
        self.out.out('0,/* tp_as_sequence */')
        self.out.out('0,/* tp_as_mapping */')
        self.out.out('0,/* tp_hash */')
        self.out.out('0,/* tp_call */')
        self.out.out('0,/* tp_str */')
        self.out.out('0,/* tp_getattro */')
        self.out.out('0,/* tp_setattro */')
        self.out.out('0,/* tp_as_buffer */')
        self.out.out('Py_TPFLAGS_DEFAULT,/* tp_flags */')
        self.out.out('0,/* tp_doc */')
        self.out.out('0,/* tp_traverse */')
        self.out.out('0,/* tp_clear */')
        self.out.out('0,/* tp_richcompare */')
        self.out.out('0,/* tp_weaklistoffset */')
        self.out.out('0,/* tp_iter */')
        self.out.out('0,/* tp_iternext */')
        self.out.out('0,/* tp_methods */')
        self.out.out('0,/* tp_members */')
        self.out.out('0,/* tp_getset */')
        self.out.out('(PyTypeObject*)PyExc_Exception,/* tp_base */')
        self.out.out('0,/* tp_dict */')
        self.out.out('0,/* tp_descr_get */')
        self.out.out('0,/* tp_descr_set */')
        self.out.out('0,/* tp_dictoffset */')
        self.out.out('0,/* tp_init */')
        self.out.out('0,/* tp_alloc */')
        self.out.out('0,/* tp_new */')
        self.out.out('0,/* tp_free */')
        self.out.out('0,/* tp_is_gc */')
        self.out.out('0,/* tp_bases */')
        self.out.out('0,/* tp_mro */')
        self.out.out('0,/* tp_cache */')
        self.out.out('0,/* tp_subclasses */')
        self.out.out('0,/* tp_weaklist */')
        self.out.out('0,/* tp_del */')
        self.out.dec_indent()
        self.out.out('};')

        self.out.out('static void ' + node.simplecscoped + 'FinaliseType()')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyType_Ready(&%sType);' % node.simplecscoped)
        self.out.out('};')

    def pythonMethod(self, pythonName, methName, cxxName, raises, params, returns):
        self.methods.append((pythonName, methName))        
        self.out.out('static PyObject *')
        self.out.out('%s(PyObject *aSelf, PyObject *aArgs, PyObject *kwds)' % methName)
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_Clear();')
        self.out.out('try')
        self.out.out('{')
        self.out.inc_indent()
        # Extract CObject...
        self.out.out('PyObject* cobj = PyObject_GetAttrString(aSelf, "_iobject_%s_cptr");' %
                     self.iface.simplecscoped)
        self.out.out('if (cobj == NULL)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_SetString(PyExc_ValueError, "Call to native PCM method when applicable cptr not set. Has __init__ been successfully called?");')
        self.out.out('return NULL;')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('if (!PyCObject_Check(cobj))')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('Py_DECREF(cobj);')
        self.out.out('PyErr_SetString(PyExc_ValueError, "cptr attribute is not CObject");')
        self.out.out('return NULL;')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('%s * native = reinterpret_cast<%s*>(PyCObject_AsVoidPtr(cobj));' %
                     (self.iface.simplecxxscoped, self.iface.simplecxxscoped))
        self.out.out('Py_DECREF(cobj);')

        # Read PyArg params...
        pyargformat = ''
        pyarglist = ''
        for (p, i) in map(None, params, range(0, len(params))):
            ti = typeinfo.GetTypeInformation(p.paramType(), self)
            pyparam = "pyparam%u" % i
            self.out.out(ti.makePyArgStorage(pyparam, p.is_in(), p.is_out()))
            pyarglist = pyarglist + ', &' + pyparam
            if not p.is_out():
                pyargformat = pyargformat + ti.format_pyarg
            else:
                pyargformat = pyargformat + 'O'
        if pyargformat != '':
            self.out.out('if (!PyArg_ParseTuple(aArgs, \"%s\"%s)) return NULL;' % (pyargformat, pyarglist))

        # Build all parameters, copying for in & in/out parameters...
        pcmarglist = ''
        for (p, i) in map(None, params, range(0, len(params))):
            if i > 0:
                pcmarglist = pcmarglist + ', '
            if p.is_out():
                ti = typeinfo.GetTypeInformation(p.paramType(), self)
                pcmarglist = pcmarglist + ti.cref
            pcmarglist = pcmarglist + 'pcmparam%u' % i
            self.out.out(typeinfo.GetTypeInformation(p.paramType(), self).makePCMFromPyarg("pcmparam%u" % i, "pyparam%u" % i,\
                                                                                     p.is_in(), p.is_out()))

        retPrefix = ''
        retExtra = ''
        if returns.kind() != idltype.tk_void:
            rti = typeinfo.GetTypeInformation(returns, self)
            self.out.out(rti.makePCMFromPyarg("_ret_pcm", "", 0, 1))
            retPrefix = '_ret_pcm = '

        # Call the PCM function...
        self.out.out('%snative->%s(%s%s);' % (retPrefix, cxxName, pcmarglist, retExtra))

        # Copy out & in/out parameters from PCM to Python...
        for (p, i) in map(None, params, range(0, len(params))):
            if not p.is_out():
                continue
            self.out.out(typeinfo.GetTypeInformation(p.paramType(), self).makePyArgFromPCM("pyparam%u" % i, "pcmparam%u" % i, 0, 0, 1))

        if returns.kind() != idltype.tk_void:
            # And generate the return...
            self.out.out(typeinfo.GetTypeInformation(returns, self).makePyArgFromPCM("_ret", "_ret_pcm", 0, 1, 1))
        else:
            self.out.out('Py_RETURN_NONE;')

        self.out.dec_indent()
        self.out.out('}')
        for ex in raises:
            self.out.out('catch (%s& aEx)' % ex.simplecxxscoped)
            self.out.out('{')
            self.out.inc_indent()
            self.raiseExceptionInContext(ex, self.iface)
            self.out.out('return NULL;')
            self.out.dec_indent()
            self.out.out('}')
        self.out.out('catch (...)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyErr_SetString(PyExc_Exception, "Native exception; not listed in raises list");')
        self.out.out('return NULL;')
        self.out.dec_indent()
        self.out.out('}')
        self.out.dec_indent()
        self.out.out('}')

    def visitOperation(self, op):
        self.pythonMethod(op.identifier(), op.simplecscoped, op.simplename, op.raises(), op.parameters(), op.returnType())

    def visitAttribute(self, at):
        for n in at.declarators():
            self.pythonMethod("get" + n.identifier(), "get_" + n.simplecscoped, n.simplename, [], [], at.attrType())
            if not at.readonly():
              self.pythonMethod("set" + n.identifier(), "set_" + n.simplecscoped, n.simplename, [], [idlast.Parameter(n.file(), n.line(), n.mainFile(), [], [], 0, at.attrType(), "value")], idltype.baseTypeMap[idltype.tk_void])
    
