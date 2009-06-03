from omniidl import idlast, idlvisitor, idlutil, idltype, output

import os, string, md5
import typeinfo

REPOID_ISUPPORTS = 'DCE:00000000-0000-0000-c000-000000000046:1'

class P2XVisitor(idlvisitor.AstVisitor):
    def __init__(self):
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
        directory, basename = os.path.split(node.file())
        self._included = ['xpcom.idl']
        if string.lower(basename[-4:]) == '.idl':
            basename = basename[:-4]
        self.hxx = output.Stream(open('p2x' + basename + '.hxx', 'w'))
        self.cpp = output.Stream(open('p2x' + basename + '.cpp', 'w'))
        gbasename = '';
        for i in basename:
            if (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z'):
                gbasename = gbasename + i
        self.hxx.out(
            "// This output is automatically generated. Do not edit.")
        self.modname = string.upper(gbasename)
        guardname='P2X__' + self.modname + '__INCLUDED'
        self.hxx.out('#ifndef ' + guardname)
        self.hxx.out("#define " + guardname)
        self.hxx.out('#include "I' + basename + '.h"')
        self.hxx.out('#include "Iface' + basename + '.hxx"')
        self.hxx.out('#include "p2xxpcom.hxx"')
        self.cpp.out('#include <exception>')
        self.cpp.out('#include "cda_compiler_support.h"')
        # self.cpp.out('#define IN_' + self.modname)
        self.cpp.out('#include "p2x' + basename + '.hxx"')
        self.cpp.out('#include "x2p' + basename + '.hxx"')
        self.cpp.out('#include "P2XFactory.hxx"')
        self.cpp.out('#include "X2PFactory.hxx"')
        self.cpp.out('#include "WideCharSupport.h"')
        self.cpp.out('#include "IWrappedPCM.h"')
        self.cpp.out('#include <nsMemory.h>')
        self.cpp.out('#include <Utilities.hxx>')
        #self.hxx.out('#ifdef IN_' + self.modname)
        #self.hxx.out('#define ' + self.modname + '_PUBLIC_PRE CDA_EXPORT_PRE')
        #self.hxx.out('#define ' + self.modname + '_PUBLIC_POST CDA_EXPORT_POST')
        #self.hxx.out('#else')
        #self.hxx.out('#define ' + self.modname + '_PUBLIC_PRE CDA_IMPORT_PRE')
        #self.hxx.out('#define ' + self.modname + '_PUBLIC_POST CDA_IMPORT_POST')
        #self.hxx.out('#endif')
        for n in node.declarations():
            if n.mainFile():
                self.contextNamespaces = ['p2x']
                self.visitingOther = 0
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
                        self.hxx.out('#include "p2x@filename@"',
                                      filename=filename)

                    self.contextNamespaces = ['p2x']
                    self.visitingOther = 1
                    n.accept(self)
        self.leaveNamespaces()
        self.hxx.out('#endif // not ' + guardname)

    def visitModule(self, node):
        self.contextNamespaces.append(node.simplename)
        for n in node.definitions():
            n.accept(self)
        self.contextNamespaces = self.contextNamespaces[:-1]

    def visitForward(self, node):
        self.syncNamespaces()
        if not self.visitingOther:
            self.hxx.out('class ' + node.simplename + ';')

    def visitInterface(self, node):
        self.syncNamespaces()
        
        isTerminal = 0
        everyModule = 0
        # See if this is a terminal interface...
        for p in node.pragmas():
            if p.text() == "terminal-interface":
                isTerminal = 1
            if p.text() == "every-module":
                everyModule = 1

        if self.visitingOther:
            # Look for the pragma...
            if everyModule == 0:
                return
        
        virtual=''
        if not isTerminal:
            virtual='virtual '

        if not self.visitingOther:
            self.hxx.out('class ' + node.simplename)
            self.hxx.out('  : public @virtual@::@name@',
                         virtual=virtual, name=node.simplecxxscoped)

        inh = filter(lambda x: x.repoId() != REPOID_ISUPPORTS, node.inherits())
        if not self.visitingOther:
            if len(inh) == 0:
                self.hxx.out('    , public @virtual@::p2x::XPCOM::IObject',
                             virtual=virtual)
            else:
                for c in inh:
                    isAmbiguous = 0
                    target = 'ambiguous-inheritance(' + c.corbacxxscoped + ')'
                    for p in node.pragmas():
                        if p.text() == target:
                            isAmbiguous = 1
                            break
                    if isAmbiguous:
                        virtual = 'virtual '
                    else:
                        virtual = ''
                    self.hxx.out('    , public @virtual@::@name@',
                                 virtual=virtual, name=c.p2xscoped)
            self.hxx.out('{')
            self.hxx.out('private:')
            self.hxx.inc_indent()
            
        self.dptr = 'mPtr' + node.simplename

        if not self.visitingOther:
            self.hxx.out('nsCOMPtr<' + node.xpcomscoped + '> ' + self.dptr +\
                         ';')
            self.hxx.dec_indent()
            self.hxx.out('public:')
            self.hxx.inc_indent()
            self.hxx.out(node.simplename +\
                         '(nsISupports* aPtr);')
            self.hxx.out(node.simplename + '();')

        self.cpp.out(node.p2xscoped + '::' + node.simplename +\
                     '()')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(self.dptr + ' = do_QueryInterface(mObj);')
        self.cpp.dec_indent()
        self.cpp.out('}')

        self.cpp.out(node.p2xscoped + '::' + node.simplename +\
                     '(nsISupports *aPtr)')
        self.cpp.out('  : ::p2x::XPCOM::IObject(aPtr)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out(self.dptr + ' = do_QueryInterface(mObj);')
        self.cpp.dec_indent()
        self.cpp.out('}')
        
        for c in node.contents():
            c.accept(self)

        if not self.visitingOther:
            self.hxx.dec_indent()
            self.hxx.out('};')
        
            self.cpp.out('class P2XFactory_' + node.xpcomscoped)
            self.cpp.out('  : public P2XFactory')
            self.cpp.out('{')
            self.cpp.out('public:')
            self.cpp.inc_indent()
            self.cpp.out('P2XFactory_' + node.xpcomscoped + '()')
            self.cpp.out('  : P2XFactory("' +
                         node.corbacxxscoped + '", NS_GET_IID(' +
                         node.xpcomscoped + '))')
            self.cpp.out('{}')
            self.cpp.out('void* MakeP2X(nsISupports* aObj)')
            self.cpp.out('{')
            self.cpp.inc_indent()
            self.cpp.out('  return static_cast< ::' + node.simplecxxscoped +\
                         '* >(new ' + node.p2xscoped + '(aObj));')
            self.cpp.dec_indent()
            self.cpp.out('}')
            self.cpp.dec_indent()
            self.cpp.out('};')
            self.cpp.out('P2XFactory_' + node.xpcomscoped + ' the_P2XFactory_' +
                         node.xpcomscoped + ';')

    def visitAttribute(self, node):
        nt = node.attrType().unalias()
        if isinstance(nt, idltype.Sequence):
            ti = typeinfo.GetTypeInformation(nt.seqType().unalias())
            rettype = ti.type_pcm + '*'
            intype = 'const ' + ti.type_pcm_const + '*'
            extra_getter_args = 'unsigned long* _ret_len'
            extra_setter_args = ', unsigned long _arg_len'
        else:
            ti = typeinfo.GetTypeInformation(nt)
            rettype = ti.type_pcm
            intype = ti.type_pcm_const
            extra_getter_args = ''
            extra_setter_args = ''
        for n in node.declarators():
            if not self.visitingOther:
                self.hxx.out('@rettype@ @name@(@args@) ' +
                             'throw(std::exception&);', rettype=rettype,
                             name=n.simplename, args=extra_getter_args)
            self.cpp.out('@rettype@ @name@(@args@)',
                         rettype=rettype, name=n.p2xscoped,
                         args=extra_getter_args)
            self.cpp.out('  throw(std::exception&)')
            self.cpp.out('{')
            self.cpp.inc_indent()
            # Make a storage variable...
            xs = ti.GetXPCOMStorage()
            self.cpp.out(xs.GetStorageDefinition())
            # self.cpp.out('  ' + rettype.type_xpcom)
            self.cpp.out('nsresult ret =')
            self.cpp.out('  ' + self.dptr + '->Get' +
                         string.upper(n.identifier()[0]) +
                         n.identifier()[1:] + '(' +
                         xs.GetAssignmentRValue() + ');')
            self.cpp.out('if (!NS_SUCCEEDED(ret))')
            self.cpp.out('  throw std::exception();')
            ps = ti.GetPCMStorage()
            self.cpp.out(ps.GetStorageDefinition())
            self.cpp.out(ti.ConvertXPCOMToPCM(xs.name, ps.name))
            self.cpp.out(xs.GetDestroyer())
            self.cpp.out(ps.GetReturn())
            self.cpp.dec_indent()
            self.cpp.out('}')
            if not node.readonly():
                if not self.visitingOther:
                    self.hxx.out('void @name@(@type@@args@) ' +
                                 'throw(std::exception&);',
                                 name=n.simplename, type=intype,
                                 args=extra_setter_args)
                self.cpp.out('void @name@(@type@ _arg@args@)',
                             name=n.p2xscoped, type=intype,
                             args=extra_setter_args)
                self.cpp.out('  throw(std::exception&)')
                self.cpp.out('{')
                self.cpp.inc_indent()
                xs = ti.GetXPCOMStorage()
                self.cpp.out(xs.GetStorageDefinition())
                self.cpp.out(ti.ConvertPCMToXPCOM('_arg', xs.name))
                self.cpp.out('nsresult ret =')
                self.cpp.out('  ' + self.dptr + '->Set' +
                             string.upper(n.identifier()[0]) + n.identifier()[1:] +
                             '(' + xs.GetInArgument() + ');'
                            )
                self.cpp.out(xs.GetDestroyer())
                self.cpp.out('if (!NS_SUCCEEDED(ret))')
                self.cpp.out('  throw std::exception();')
                self.cpp.dec_indent()
                self.cpp.out('}')

    def visitOperation(self, node):
        rtype = node.returnType().unalias()
        if isinstance(rtype, idltype.Sequence):
            ti = typeinfo.GetTypeInformation(rtype.seqType().unalias())
            rettype = ti.type_pcm + '*'
            extra_args = 'unsigned long* _ret_len'
            xret = ti.GetXPCOMStorage(self)
            pret = ti.GetPCMStorage(self)
        else:
            extra_args = ''
            if not(isinstance(rtype, idltype.Base) and
                   rtype.kind() == idltype.tk_void):
                ti = typeinfo.GetTypeInformation(rtype)
                rettype = ti.type_pcm
                xret = ti.GetXPCOMStorage()
                pret = ti.GetPCMStorage()
            else:
                rettype = 'void'
                xret = None
                pret = None
                ti = None
        retti = ti
        needcomma = 0
        parstr = ''
        for p in node.parameters():
            if needcomma:
                parstr = parstr + ', '
            else:
                needcomma = 1
            ptype = p.paramType().unalias()
            ti = typeinfo.GetTypeInformation(ptype)
            p.ti = ti
            parstr = parstr + p.ti.PCMArgument(p.simplename, p.is_in(),
                                               p.is_out())
        if extra_args != '':
            if needcomma:
                parstr = parstr + ', '
            parstr = parstr + extra_args
        if not self.visitingOther:
            self.hxx.out('@rettype@ @name@(@args@) throw(std::exception&);',
                         rettype=rettype, name=node.simplename, args=parstr)
        self.cpp.out('@rettype@ @name@(@args@)',
                     rettype=rettype, name=node.p2xscoped, args=parstr)
        self.cpp.out('  throw(std::exception&)')
        self.cpp.out('{')
        self.cpp.inc_indent()

        # Create storage for each parameter. In and In/Out parameters also get
        # converted...
        callargs = ''
        for p in node.parameters():
            p.xs = p.ti.GetXPCOMStorage()
            self.cpp.out(p.xs.GetStorageDefinition())
            if callargs != '':
                callargs = callargs + ', '
            if p.is_in():
                if p.is_out():
                    deref = '*'
                    callargs = callargs + p.xs.GetInOutArgument()
                else:
                    deref = ''
                    callargs = callargs + p.xs.GetInArgument()
                self.cpp.out(p.ti.ConvertPCMToXPCOM(deref + p.simplename,
                                                    p.xs.name))
            else:
                callargs = callargs + p.xs.GetAssignmentRValue()

        if xret != None:
            self.cpp.out(xret.GetStorageDefinition())
            if callargs != '':
                callargs = callargs + ', '
            callargs = callargs + xret.GetAssignmentRValue()

        # Call the XPCOM method...
        self.cpp.out('nsresult ret =')
        self.cpp.out('  ' + self.dptr + '->' + string.upper(node.simplename[0]) +
                     node.simplename[1:] + '(' + callargs + ');')
        rl = node.raises()
        if len(rl) == 0:
            exception = 'std::exception'
        else:
            exception = rl[0].simplecxxscoped
        self.cpp.out('if (!NS_SUCCEEDED(ret))')
        self.cpp.out('  throw ' + exception + '();')

        for p in node.parameters():
            if p.is_out():
                self.cpp.out(p.ti.ConvertXPCOMToPCM(p.xs.name, '*' +\
                                                    p.simplename))
            self.cpp.out(p.xs.GetDestroyer())
        
        if pret != None:
            self.cpp.out(pret.GetStorageDefinition())
            self.cpp.out(retti.ConvertXPCOMToPCM(xret.name, pret.name))
            self.cpp.out(xret.GetDestroyer())
            self.cpp.out(pret.GetReturn())
        
        self.cpp.dec_indent()
        self.cpp.out('}')
        
def run(tree):
    tree.accept(P2XVisitor())
