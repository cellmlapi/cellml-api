# -*- python -*-
import os.path
import identifier
import typeinfo
import string
from omniidl import idlast, idlvisitor, idlutil, idltype, output

REPOID_ISUPPORTS = 'DCE:00000000-0000-0000-c000-000000000046:1'

class CToPythonWalker(idlvisitor.AstVisitor):
    """Walks over the AST once and writes the Python wrapper in the process
    """

    def __init__(self):
        self.outNamespaces = []
        self.contextNamespaces = []

    def leaveNamespaces(self):
        for n in self.outNamespaces:
            self.hxx.dec_indent()
            self.hxx.out('};')
        self.outNamespaces = []

    def syncNamespaces(self):
        x=0
        for i in range(0, len(self.outNamespaces)):
            x=i + 1
            try:
                if self.contextNamespaces[i] != self.outNamespaces[i]:
                    x=i
                    break
            except IndexError:
                x=i
                break
        for j in range(x, len(self.outNamespaces)):
            self.hxx.dec_indent()
            self.hxx.out('};')
        for j in range(x, len(self.contextNamespaces)):
            self.hxx.out('namespace ' + self.contextNamespaces[j])
            self.hxx.out('{')
            self.hxx.inc_indent()
        self.outNamespaces = self.outNamespaces[0:x] +\
                             self.contextNamespaces[x:]
    
    def visitAST(self, node):
        """Visit all the declarations in an AST"""

        self._included = ['xpcom.idl']
        self.hxx.out(
            "// This output is automatically generated. Do not edit.")
        self.cpp.out(
            "// This output is automatically generated. Do not edit.")

        gbasename = ''
        for i in node.filebase:
            if (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z'):
                gbasename = gbasename + i

        self.filebase = node.filebase
        self.modname = string.upper(gbasename)

        guardname='P2Py__' + self.modname + '__INCLUDED'
        self.hxx.out('#ifndef ' + guardname)
        self.hxx.out("#define " + guardname)
        self.hxx.out('#include <exception>')
        self.hxx.out('#include <memory>')
        self.hxx.out('#include "Utilities.hxx"')
        self.hxx.out('#include "Iface' + node.filebase + '.hxx"')
        self.hxx.out('#include "python_support.hxx"')
        self.cpp.out('#include "P2Py' + node.filebase + '.hxx"')

        self.hxx.out('#ifdef IN_PYTHON_LIB_' + self.filebase)
        self.hxx.out('#define PUBLIC_' + self.modname + '_PRE CDA_EXPORT_PRE')
        self.hxx.out('#define PUBLIC_' + self.modname + '_POST CDA_EXPORT_POST')
        self.hxx.out('#else')
        self.hxx.out('#define PUBLIC_' + self.modname + '_PRE CDA_IMPORT_PRE')
        self.hxx.out('#define PUBLIC_' + self.modname + '_POST CDA_IMPORT_POST')
        self.hxx.out('#endif')

        for n in node.declarations():
            if n.mainFile():
                self.contextNamespaces = ['p2py']
                n.accept(self)
            else:
                filename = n.file()
                pos = string.rfind(filename, '/')
                if pos != -1:
                    filename = filename[pos + 1:]

                if not filename in self._included:
                    self._included.append(filename)

                    self.leaveNamespaces()
                    
                    if filename[-4:] == ".idl":
                        filename = filename[0:-4] + ".hxx"

                    if filename != "xpcom.idl":
                        self.leaveNamespaces()
                        self.hxx.out('#include "P2Py@filename@"',
                                      filename=filename)

                    self.contextNamespaces = ['p2py']
        self.leaveNamespaces()
        self.hxx.out('#endif // not ' + guardname)

    def visitModule(self, node):
        """Visit all the definitions in a module."""
        self.contextNamespaces.append(node.identifier())
        for n in node.definitions():
            n.accept(self)
        self.contextNamespaces.pop()

    def visitForward(self, node):
        self.syncNamespaces()
        self.hxx.out('class @classname@;', classname=node.simplename)

    def visitInterface(self, node):
        self.syncNamespaces()
        self.hxx.out('PUBLIC_@modname@_PRE class PUBLIC_@modname@_POST @classname@',
                     modname=self.modname, classname=node.simplename)

        isTerminal = 0
        # See if this is a terminal interface...
        for p in node.pragmas():
            if p.text() == "terminal-interface":
                isTerminal = 1

        self.iface = node

        virtual = ''
        if not isTerminal:
            virtual ='virtual '

        inh = []
        for i in node.inherits():
            if isinstance(i, idlast.Declarator) and i.alias():
                i = i.alias().aliasType().unalias().decl()
            inh.append(i)
            
        inh = filter(lambda x: x.repoId() != REPOID_ISUPPORTS, inh)

        classname = 'p2py::' + node.corbacxxscoped
        self.classname = classname
        self.hxx.out('    : public @virtual@::@scopedname@',
                     virtual=virtual, scopedname=node.simplecxxscoped)
            
        if len(inh) == 0:
            self.hxx.out('    , public @virtual@::p2py::XPCOM::IObject',
                         virtual=virtual)
        else:
            for c in inh:
                isAmbiguous = 0
                iclassname = c.corbacxxscoped
                target = 'ambiguous-inheritance(' + iclassname + ')'
                for p in node.pragmas():
                    if p.text() == target:
                        isAmbiguous = 1
                        break
                    if isAmbiguous:
                        virtual = 'virtual '
                    else:
                        virtual = ''
                self.hxx.out('    , public @virtual@::p2py::@classname@',
                             virtual=virtual, classname=iclassname)

        self.hxx.out('{')
        self.hxx.out('public:')
        self.hxx.inc_indent()
        self.hxx.out('%s() {}' % node.simplename)
        self.hxx.out('PUBLIC_%s_PRE %s(PyObject *aObject) PUBLIC_%s_POST;' % (self.modname, node.simplename, self.modname))
        
        self.cpp.out('@classname@::@constrname@(PyObject* aObject)',
                     classname=classname, constrname=node.simplename)
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('mObject = aObject;')
        self.cpp.out('Py_INCREF(mObject);')
        self.cpp.dec_indent()
        self.cpp.out('}')

        for n in node.contents():
            n.accept(self)

        self.hxx.dec_indent()
        self.hxx.out('};')

        for ns in self.contextNamespaces:
            self.cpp.out('namespace %s' % ns)
            self.cpp.out('{')
            self.cpp.inc_indent()
        self.cpp.out('class %sFactory' % node.simplename)
        self.cpp.out('  : public ::P2PyFactory')
        self.cpp.out('{')
        self.cpp.out('public:')
        self.cpp.inc_indent()
        self.cpp.out('%sFactory() : ::P2PyFactory("%s") {};' % (node.simplename, node.corbacxxscoped))
        self.cpp.out('virtual ~%sFactory() {};' % (node.simplename))
        self.cpp.out("void* create(PyObject* aObj)")
        self.cpp.out("{")
        self.cpp.inc_indent()
        self.cpp.out('return reinterpret_cast<void*>(static_cast< ::iface::%s*>(new ::p2py::%s(aObj)));' % (node.corbacxxscoped, node.corbacxxscoped))
        self.cpp.dec_indent()
        self.cpp.out("}")
        self.cpp.dec_indent()
        self.cpp.out('};')
        for ns in self.contextNamespaces:
            self.cpp.dec_indent()
            self.cpp.out('};')
        self.cpp.out('static ::p2py::%sFactory s%sFactory;' % (node.corbacxxscoped, node.simplecscoped))
            
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

    def ctopyMethod(self, pythonName, cxxName, raises, params, returns):
        if returns.kind() == idltype.tk_void:
            rettype = 'void'
        else:
            rettype = typeinfo.GetTypeInformation(returns, self).pcmType(isRet=2)

        paramsigs = []
        i = 0
        for p in params:
            p.pcmname = 'param%u' % i
            p.pyname = 'pyparam%u' % i
            i = i + 1
            p.ti = typeinfo.GetTypeInformation(p.paramType(), self)
            paramsigs.append(p.ti.pcmType(isOut=p.is_out()) + ' ' + p.pcmname)
        paramsig = string.join(paramsigs, ', ')
        self.hxx.out('PUBLIC_@modname@_PRE @rettype@ @cxxName@(@paramsig@) throw(std::exception&) PUBLIC_@modname@_POST;',
                     rettype=rettype, cxxName=cxxName, paramsig=paramsig, modname=self.modname)
        self.cpp.out('@rettype@ @classname@::@cxxName@(@paramsig@)',
                     rettype=rettype, classname=self.classname, cxxName=cxxName, paramsig=paramsig)
        self.cpp.out('  throw(std::exception&)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('ScopedPyGIL _lock;')

        callParams = ['']
        # Set up parameters for call to Python...
        for p in params:
            ti = typeinfo.GetTypeInformation(p.paramType(), self)
            if p.is_out():
                pt = 'O'
            else:
                pt = ti.format_pyarg
            callParams[0] = callParams[0] + pt
            callParams.append(p.pyname)
            self.cpp.out(ti.makePyArgFromPCM(p.pyname, p.pcmname, p.is_in(), p.is_out()))

        callParams[0] = 'const_cast<char*>("' + callParams[0] + '")'

        # Make the call...
        self.cpp.out('PyObject* _pyret = PyObject_CallMethod(mObject, const_cast<char*>("@name@"), @params@);',
                     name=pythonName, params=string.join(callParams, ', '))
        self.cpp.out('ScopedPyObjectReference _pyretScoped(_pyret);')

        # Do we need to raise an exception?
        self.cpp.out('PyObject* exc = PyErr_Occurred();')
        self.cpp.out('if (exc != NULL)')
        self.cpp.out('{')
        self.cpp.inc_indent()

        for r in raises:
            self.cpp.out('{')
            self.cpp.inc_indent()
            directory, filename = os.path.split(self.iface.file())
            filebase, extension = os.path.splitext(filename)
            self.cpp.out('PyObject* raiseMod = PyImport_ImportModule("%s");' % filebase)
            self.cpp.out('ScopedPyObjectReference raiseMod_release(raiseMod);')
            self.cpp.out('if (raiseMod != NULL)')
            self.cpp.out('{')
            self.cpp.inc_indent()
            self.cpp.out('PyObject* raise = PyObject_GetAttrString(raiseMod, "@raiseName@");', raiseName=r.identifier())
            self.cpp.out('ScopedPyObjectReference raise_release(raise);')
            self.cpp.out('if (raise != NULL && PyErr_GivenExceptionMatches(exc, raise))')
            self.cpp.out('  throw ::@ex@();', ex=r.simplecxxscoped)
            self.cpp.dec_indent()
            self.cpp.out('}')
            self.cpp.dec_indent()
            self.cpp.out('}')
        
        # Catch all for everything else...
        self.cpp.out('throw std::exception();')
        self.cpp.dec_indent()
        self.cpp.out('}')

        # Now prepare out & in/out arguments for output...
        for p in params:
            if p.is_out():
                self.cpp.out(p.ti.makePCMFromPyarg(p.pcmname, p.pyname, 0, 0, 1))

        if returns.kind() != idltype.tk_void:
            self.cpp.out(typeinfo.GetTypeInformation(returns, self).makePCMFromPyarg('', '_pyret', 0, 1, 1))
        
        self.cpp.dec_indent()
        self.cpp.out('}')
        
    def visitOperation(self, op):
        self.ctopyMethod(op.identifier(), op.simplename, op.raises(), op.parameters(), op.returnType())

    def visitAttribute(self, at):
        for n in at.declarators():
            self.ctopyMethod("get" + n.identifier(), n.simplename, [], [], at.attrType())
            if not at.readonly():
              self.ctopyMethod("set" + n.identifier(), n.simplename, [], [idlast.Parameter(n.file(), n.line(), n.mainFile(), [], [], 0, at.attrType(), "value")], idltype.baseTypeMap[idltype.tk_void])
