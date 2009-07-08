from omniidl import idlast, idlvisitor, idlutil, idltype, output

import os, string, md5
import typeinfo

REPOID_ISUPPORTS = 'DCE:00000000-0000-0000-c000-000000000046:1'

class X2PVisitor(idlvisitor.AstVisitor):
    def __init__(self):
        pass

        self.outNamespaces = []
        self.contextNamespaces = []

    def leaveNamespaces(self):
        for n in self.outNamespaces:
            self.hxx.dec_indent()
            self.hxx.out('};')
        self.contextNamespaces = []
        self.outNamespaces = []

    def syncNamespaces(self):
        x=0
        for i in range(0, len(self.outNamespaces)):
            x=i+1
            try:
                if self.contextNamespaces[i] != self.outNamespaces[i]:
                    break
            except IndexError:
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
        directory, basename = os.path.split(node.file())
        self._included = ['xpcom.idl']
        if string.lower(basename[-4:]) == '.idl':
            basename = basename[:-4]
        self.hxx = output.Stream(open('x2p' + basename + '.hxx', 'w'))
        self.cpp = output.Stream(open('x2p' + basename + '.cpp', 'w'))
        gbasename = '';
        for i in basename:
            if (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z'):
                gbasename = gbasename + i
        self.hxx.out(
            "// This output is automatically generated. Do not edit.")
        guardname='X2P__' + string.upper(gbasename) + '__INCLUDED'
        self.hxx.out('#ifndef ' + guardname)
        self.hxx.out("#define " + guardname)
        self.hxx.out('#include "Iface' + basename + '.hxx"')
        self.hxx.out('#include "I' + basename + '.h"')
        self.hxx.out('#include "x2pxpcom.hxx"')
        self.cpp.out('#include <exception>')
        self.cpp.out('#include "cda_compiler_support.h"')
        self.cpp.out('#include "p2x' + basename + '.hxx"')
        self.cpp.out('#include "x2p' + basename + '.hxx"')
        self.cpp.out('#include "X2PFactory.hxx"')
        self.cpp.out('#include "P2XFactory.hxx"')
        self.cpp.out('#include "WideCharSupport.h"')
        self.cpp.out('#include "IWrappedPCM.h"')
        self.cpp.out('#include <Utilities.hxx>')
        self.cpp.out('#include <nsMemory.h>')
        for n in node.declarations():
            if n.mainFile():
                self.contextNamespaces = ['x2p']
                n.accept(self)
            else:
                self.leaveNamespaces()
                filename = n.file()
                pos = string.rfind(filename, '/')
                if pos != -1:
                    filename = filename[pos + 1:]

                if not filename in self._included:
                    self._included.append(filename)

                    if filename[-4:] == ".idl":
                        filename = filename[0:-4] + ".hxx"

                    if filename != "xpcom.idl":
                        self.hxx.out('#include "x2p@filename@"',
                                      filename=filename)
        self.leaveNamespaces()
        self.hxx.out('#endif // not ' + guardname)

    def visitModule(self, node):
        self.contextNamespaces.append(node.simplename)
        for n in node.definitions():
            n.accept(self)
        self.contextNamespaces = self.contextNamespaces[:-1]

    def visitForward(self, node):
        self.syncNamespaces()
        self.hxx.out('class ' + node.simplename + ';')

    def visitInterface(self, node):
        self.syncNamespaces()
        
        # XPCOM doesn't use virtual inheritance. Instead, we must implement
        # everything at the top-level implementation...
        icur = node
        ilist = [icur]
        while 1:
            inh = filter(lambda x: x.repoId() != REPOID_ISUPPORTS, icur.inherits())
            if len(inh) == 0:
                break
            icur = inh[0]
            ilist.append(icur)

        self.classn = node.x2pscoped
        
        self.hxx.out('class ' + node.simplename)
        self.hxx.out('  : public ::x2p::XPCOM::IObject')
        self.hxx.out('  , public ::@name@',
                     name=node.xpcomscoped)
        self.hxx.out('{')
        self.hxx.out('private:')
        self.hxx.inc_indent()
        self.dptr = 'mPtr' + node.simplename
        self.hxx.out(node.simplecxxscoped + ' * ' + self.dptr +\
                     ';')
        self.hxx.dec_indent()
        self.hxx.out('public:')
        self.hxx.inc_indent()
        self.hxx.out('NS_DECL_ISUPPORTS_INHERITED')
        self.hxx.out(node.simplename + '(iface::XPCOM::IObject *aPtr);')
        self.hxx.out(node.simplename + '();')
        self.hxx.out('~' + node.simplename + '();')

        self.cpp.out(node.x2pscoped + '::' + node.simplename +\
                     '()')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(self.dptr + ' = reinterpret_cast< ' + node.simplecxxscoped +
                     '*>(mObj->query_interface("' + node.corbacxxscoped +\
                     '"));')
        self.cpp.dec_indent()
        self.cpp.out('}')

        self.cpp.out(node.x2pscoped + '::' + node.simplename +\
                     '(iface::XPCOM::IObject *aPtr)')
        self.cpp.out('  : ::x2p::XPCOM::IObject(aPtr)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(self.dptr + ' = reinterpret_cast< ' + node.simplecxxscoped +
                     '*>(mObj->query_interface("' + node.corbacxxscoped +\
                     '"));')
        self.cpp.dec_indent()
        self.cpp.out('}')

        self.cpp.out(node.x2pscoped + '::~' + node.simplename + '()')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(self.dptr + '->release_ref();')
        self.cpp.dec_indent()
        self.cpp.out('}')

        self.cpp.out('NS_IMPL_ISUPPORTS_INHERITED0(' + node.x2pscoped +
                     ', x2p::XPCOM::IObject);')

        for i in ilist:
            for c in i.contents():
                c.accept(self)
            
        self.hxx.dec_indent()
        self.hxx.out('};')
        self.cpp.out('class X2PFactory_' + node.xpcomscoped)
        self.cpp.out('  : public X2PFactory')
        self.cpp.out('{')
        self.cpp.out('public:')
        self.cpp.inc_indent()
        self.cpp.out('X2PFactory_' + node.xpcomscoped + '()')
        self.cpp.out('  : X2PFactory("' +
                     node.corbacxxscoped + '", NS_GET_IID(' +
                     node.xpcomscoped + '))')
        self.cpp.out('{}')
        self.cpp.out('void* MakeX2P(void* aObj)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(node.simplecxxscoped + '* obji = reinterpret_cast< ::' +\
                     node.simplecxxscoped + '* >(aObj);')
        self.cpp.out(node.xpcomscoped + ' * obj = new ' +
                     node.x2pscoped + '(obji);')
        self.cpp.out('NS_ADDREF(obj);')
        self.cpp.out('obji->release_ref();')
        self.cpp.out('return static_cast< ::' + node.xpcomscoped + '* >(obj);')
        self.cpp.dec_indent()
        self.cpp.out('}')
        self.cpp.dec_indent()
        self.cpp.out('};')
        self.cpp.out('X2PFactory_' + node.xpcomscoped + ' the_X2PFactory_' +
                     node.xpcomscoped + ';')

    def visitAttribute(self, node):
        nt = node.attrType().unalias()
        ti = typeinfo.GetTypeInformation(nt)
        getter_args = ti.XPCOMArgument('_ret', 0, 1)
        setter_args = ti.XPCOMArgument('_arg', 1, 0)
        for n in node.declarators():
            name = string.upper(n.identifier()[0]) + n.identifier()[1:]
            self.hxx.out('NS_IMETHOD Get@name@(@args@);',
                         name=name, args=getter_args)
            self.cpp.out('NS_IMETHODIMP @classn@::Get@name@(@args@)',
                         classn=self.classn, name=name,
                         args=getter_args)
            self.cpp.out('{')
            self.cpp.inc_indent()
            ps = ti.GetPCMStorage()
            self.cpp.out(ps.GetStorageDefinition())
            self.cpp.out('try')
            self.cpp.out('{')
            self.cpp.inc_indent()
            callstr = ps.GetAssignmentLValue() + ' = ' +\
                      self.dptr + '->' + n.simplename + '('
            if ps.__dict__.has_key('lenname'):
                callstr = callstr + ps.lenname
            callstr = callstr + ');'
            self.cpp.out(callstr)
            self.cpp.dec_indent()
            self.cpp.out('}')
            self.cpp.out('catch (...)')
            self.cpp.out('{')
            self.cpp.inc_indent()
            self.cpp.out('return NS_ERROR_FAILURE;')
            self.cpp.dec_indent()
            self.cpp.out('}')
            xs = ti.GetXPCOMStorage()
            self.cpp.out(xs.GetStorageDefinition())
            self.cpp.out(ti.ConvertPCMToXPCOM(ps.name, xs.name))
            self.cpp.out(xs.GetAssignOut('_ret'))
            self.cpp.out(xs.GetDestroyer())
            self.cpp.out(ps.GetDestroyer())
            self.cpp.out('return NS_OK;')
            self.cpp.dec_indent()
            self.cpp.out('}')
            if not node.readonly():
                self.hxx.out('NS_IMETHOD Set@name@(@args@);',
                             name=name, args=setter_args)
                self.cpp.out('NS_IMETHODIMP @classn@::Set@name@(@args@)',
                             classn=self.classn, name=name,
                             args=setter_args)
                self.cpp.out('{')
                self.cpp.inc_indent()
                ps = ti.GetPCMStorage()
                self.cpp.out(ps.GetStorageDefinition())
                self.cpp.out(ti.ConvertXPCOMToPCM('_arg', ps.name))
                self.cpp.out('try')
                self.cpp.out('{')
                self.cpp.inc_indent()
                self.cpp.out(self.dptr + '->' + n.simplename + '(' +\
                             ps.GetInArgument() + ');')
                self.cpp.dec_indent()
                self.cpp.out('}')
                self.cpp.out('catch (...)')
                self.cpp.out('{')
                self.cpp.inc_indent()
                self.cpp.out(ps.GetDestroyer())
                self.cpp.out('return NS_ERROR_FAILURE;')
                self.cpp.dec_indent()
                self.cpp.out('}')
                self.cpp.out(ps.GetDestroyer())
                self.cpp.out('return NS_OK;')
                self.cpp.dec_indent()
                self.cpp.out('}')

    def visitOperation(self, node):
        rtype = node.returnType().unalias()
        if isinstance(rtype, idltype.Base) and \
           rtype.kind() == idltype.tk_void:
            rtype = None
            pret = None
        else:
            rti = typeinfo.GetTypeInformation(rtype)
            pret = rti.GetPCMStorage()
        needcomma = 0
        parstr = ''
        for p in node.parameters():
            if needcomma:
                parstr = parstr + ', '
            else:
                needcomma = 1
            ptype = p.paramType().unalias()
            p.ti = typeinfo.GetTypeInformation(ptype)
            parstr = parstr + p.ti.XPCOMArgument(p.simplename, p.is_in(),
                                                 p.is_out())
        if rtype != None:
            if needcomma:
                parstr = parstr + ', '
            parstr = parstr + rti.XPCOMArgument('_ret', 0, 1)

        name = string.upper(node.simplename[0]) + node.simplename[1:]
        self.hxx.out('NS_IMETHOD @name@(@args@);',
                     name=name, args=parstr)
        self.cpp.out('NS_IMETHODIMP @classn@::@name@(@args@)',
                     classn=self.classn, name=name, args=parstr)
        self.cpp.out('{')
        self.cpp.inc_indent()
        callargs = ''
        for p in node.parameters():
            p.ps = p.ti.GetPCMStorage()
            self.cpp.out(p.ps.GetStorageDefinition())
            if callargs != '':
                callargs = callargs + ', '
            if p.is_in():
                if p.is_out():
                    deref = '*'
                    callargs = callargs + p.ps.GetInOutArgument()
                else:
                    deref = ''
                    callargs = callargs + p.ps.GetInArgument()
                self.cpp.out(p.ti.ConvertXPCOMToPCM(deref + p.simplename,
                                                    p.ps.name))
            else:
                callargs = callargs + p.ps.GetAssignmentRValue()

        pref = ''
        if pret != None:
            self.cpp.out(pret.GetStorageDefinition())
            pref = pret.GetAssignmentLValue() + ' = '

        # Call the XPCOM method...
        self.cpp.out('try')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(pref + self.dptr + '->' + node.simplename + '(' +\
                     callargs + ');')
        self.cpp.dec_indent()
        self.cpp.out('}')
        self.cpp.out('catch (...)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        for p in node.parameters():
            if p.is_in():
                self.cpp.out(p.ps.GetDestroyer())
        self.cpp.out('return NS_ERROR_FAILURE;')
        self.cpp.dec_indent()
        self.cpp.out('}')
        for p in node.parameters():
            if not p.is_out():
                self.cpp.out(p.ps.GetDestroyer())
                continue
            xs = p.ti.GetXPCOMStorage()
            self.cpp.out(xs.GetStorageDefinition())
            self.cpp.out(p.ti.ConvertPCMToXPCOM(p.ps.name, xs.name))
            self.cpp.out(xs.GetAssignOut(p.simplename))
            self.cpp.out(p.ps.GetDestroyer())
            self.cpp.out(xs.GetDestroyer())

        if pret != None:
            xs = rti.GetXPCOMStorage()
            self.cpp.out(xs.GetStorageDefinition())
            self.cpp.out(rti.ConvertPCMToXPCOM(pret.name, xs.name))
            self.cpp.out(xs.GetAssignOut('_ret'))
            self.cpp.out(pret.GetDestroyer())
            self.cpp.out(xs.GetDestroyer())

        self.cpp.out('return NS_OK;')

        self.cpp.dec_indent()
        self.cpp.out('}')

def run(tree):
    tree.accept(X2PVisitor())
