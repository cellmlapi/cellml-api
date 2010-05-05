# -*- python -*-
import os.path
import identifier

class PythonToCWalker(idlvisitor.AstVisitor):
    """Walks over the AST once and writes the Python wrapper in the process"""

    def visitAST(self, node):
        """Visit all the declarations in an AST"""
        self.module = node.filebase
        
        for n in node.declarations():
            n.accept(self)

        self.out.out("PyMODINIT_FUNC\ninit" + self.module + "(void)\n{")
        self.out.inc_indent()
        
        self.out.dec_indent()
        self.out.out("}")

    def visitModule(self, node):
        """Visit all the definitions in a module."""
        pass

    def visitForward(self, node):
        pass

    def visitInterface(self, node):
        self.methods = []
        
        for n in node.declarations():
            n.accept(self)
        
        self.out.out("static PyTypeObject " + node.simplecscoped + "Type = {")
        self.out.inc_indent()
        self.out.out("PyObject_HEAD_INIT(NULL)")
        self.out.out("0,/* ob_size */")
        self.out.out('"' + self.module + '.' + node.simplecscoped +
                     '\",/* tp_name */')
        self.out.out('sizeof(' + node.simplecscoped + 'Type),/* tp_basicsize */')
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
        self.out.out(node.simplecscoped + 'Methods,/* tp_methods */')
        self.out.out('0,/* tp_members */')
        self.out.out('0,/* tp_getset */')
        self.out.out('0,/* tp_base */')
        self.out.out('0,/* tp_dict */')
        self.out.out('0,/* tp_descr_get */')
        self.out.out('0,/* tp_descr_set */')
        self.out.out('0,/* tp_dictoffset */')
        self.out.out(node.simplecscoped + 'Init,/* tp_init */')
        self.out.out('0,/* tp_alloc */')
        self.out.out('0,/* tp_new */')
        self.out.out('0,/* tp_free */')
        self.out.out('0,/* tp_is_gc */')
        self.out.out('0,/* tp_bases */')
        self.out.out('0,/* tp_mro */')
        self.out.out('0,/* tp_cache */')
        self.out.out('0,/* tp_subclasses */')
        self.out.out('0,/* tp_weaklist */')
        self.out.out(node.simplecscoped + 'Del,/* tp_del */')
        self.out.dec_indent()
        self.out.out('};')
        self.out.out('static PyMethodDef[] %sMethods = {' % node.simplecscoped)
        self.out.inc_indent()
        for name, funcname in self.methods:
            self.out.out('{"%s",%s,METH_VARARGS,NULL},'%(name, funcname))
        self.out.out('{NULL, NULL, 0, NULL}')
        self.out.dec_indent()
        self.out.out('};')
        self.out.out('static void ' + node.simplecscoped + 'Del(PyObject* aSelf)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyObject* cptr = PyObject_GetAttrString(aSelf, "_iobject_cptr");')
        self.out.out('reinterpret_cast<iface::XPCOM::IObject*>(PyCObject_AsVoidPtr(cptr))->release_ref();')
        self.out.out('Py_DECREF(cptr);')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('static void ' + node.simplecscoped + 'FinaliseType()')
        self.out.out('{')
        self.out.inc_indent()
        if node.simplecscoped != 'XPCOM_IObject':
            bases = []
            nextbase = 0
            for base in node.inherits():
                if base.file() == node.file:
                    bases.append(node.simplecscoped + 'Type')
                else:
                    bn = 'base%u' % nextbase
                    nextbase = nextbase + 1
                    self.out.out('PyObject* %s;' % bn)
                    directory, filename = os.path.split(base.file())
                    filebase, extension = os.path.splittex(filename)
                    self.out.out('{')
                    self.out.inc_indent()
                    self.out.out('PyObject* %sMod = PyImport_ImportModule("%s");' % (bn, filename))
                    self.out.out('if (module == NULL) return -1;')
                    self.out.out('%s = PyObject_GetAttrString(%sMod, "%s");' % (bn, bn, base.simplecscoped))
                    self.out.out('Py_DECREF(%sMod);' % bn)
                    self.out.dec_indent()
                    self.out.out('}')
                    bases.append(bn)
            self.out.out('%sType->tp_bases = PyTuple_Pack(%u, %s);' %
                         (node.simplecscoped, len(bases), str.join(', ', bases)))
            for i in range(0, nextbase):
                self.out.out('Py_DECREF(base%u);' % i)
        self.out.out('PyType_Ready(%sType);' % node.simplecscoped)
        self.out.dec_indent()
        self.out.out('};')
        self.out.out('static int ' + node.simplecscoped +
                     'Init(PyTypeObject* aType, PyObject* aArgs, PyObject* aKwds)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('PyObject* iobject;')
        self.out.out('if (!PyArg_ParseTuple(args, "O", &iobject)) return NULL;')
        self.out.out('Py_INCREF(iobject);')
        self.out.out('if (PyCObject_Check(iobject))')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('iface::XPCOM::IObject* obj = reinterpret_cast<iface::XPCOM::IObject*>(PyCObject_AsVoidPtr(iobject));')
        self.out.out('iface::%s * tobj = obj->query_interface("%s");' % (node.simplecxxscoped, node.simplecxxscoped))
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('else')
        self.out.out('{')
        self.out.inc_indent()
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('Py_DECREF(iobject);')
        self.out.out('return 0;')
        self.out.dec_indent()
        self.out.out('};')
    
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
        pass

    def visitOperation(self, op):
        pass

    def visitAttribute(self, at):
        pass
    
class CToPythonWalker(idlvisitor.AstVisitor):
    """Walks over the AST once and writes the Python wrapper in the process
    """

    def visitAST(self, node):
        """Visit all the declarations in an AST"""
        pass

    def visitModule(self, node):
        """Visit all the definitions in a module."""
        pass

    def visitForward(self, node):
        pass

    def visitInterface(self, node):
        pass
    
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
        pass

    def visitOperation(self, op):
        pass

    def visitAttribute(self, at):
        pass

def run(tree, args):
    """Entrypoint to the Python backend"""

    tree.directory, tree.filename = os.path.split(tree.file())
    tree.filebase, extension = os.path.splittex(tree.filename)

    p2cw = PythonToCWalker()
    p2cw.out = output.Stream(open('PyWrap_' + tree.filebase + '.cxx', 'w'), 2)
    c2pw = CToPythonWalker()
    c2pw.out = p2cw.out

    annot = Annotator()

    tree.accept(annot)
    tree.accept(p2cw)
    tree.accept(c2pw)
