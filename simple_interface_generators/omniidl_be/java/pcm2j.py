import os
from omniidl import idlast, idlvisitor, output
import jnutils
import string

REPOID_ISUPPORTS = 'DCE:00000000-0000-0000-c000-000000000046:1'

class NativePCM2JVisitor (idlvisitor.AstVisitor):
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
        directory, basename = os.path.split(node.file())
        
        self._included = ['xpcom.idl']
        if string.lower(basename[-4:]) == '.idl':
            basename = basename[:-4]
        self.hxx = output.Stream(open('p2j' + basename + '.hxx', 'w'))
        self.cpp = output.Stream(open('p2j' + basename + '.cpp', 'w'))
        self.hxx.out(
            "// This output is automatically generated. Do not edit.")
        self.modname = basename
        self.defname = 'P2J__' + self.modname
        guardname= self.defname + '__INCLUDED'
        self.hxx.out('#ifndef ' + guardname)
        self.hxx.out("#define " + guardname)
        self.hxx.out('#include <exception>')
        self.hxx.out('#include "cda_compiler_support.h"')
        self.hxx.out('#include "pick-jni.h"')
        self.hxx.out('#include "Iface' + basename + '.hxx"')
        self.hxx.out('#include "p2jxpcom.hxx"')
        self.cpp.out('#define MODULE_CONTAINS_' + self.defname)
        self.cpp.out('#include <exception>')
        self.cpp.out('#include "cda_compiler_support.h"')
        self.cpp.out('#include "j2p' + basename + '.hxx"')
        self.cpp.out('#include "p2j' + basename + '.hxx"')
        self.cpp.out('#include <Utilities.hxx>')

        self.hxx.out('#ifdef MODULE_CONTAINS_' + self.defname)
        self.hxx.out('#define PUBLIC_' + self.defname + '_PRE CDA_EXPORT_PRE')
        self.hxx.out('#define PUBLIC_' + self.defname + '_POST CDA_EXPORT_POST')
        self.hxx.out('#else')
        self.hxx.out('#define PUBLIC_' + self.defname + '_PRE CDA_IMPORT_PRE')
        self.hxx.out('#define PUBLIC_' + self.defname + '_POST CDA_IMPORT_POST')
        self.hxx.out('#endif')
        
        for n in node.declarations():
            if n.mainFile():
                self.contextNamespaces = ['p2j']
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
                        self.hxx.out('#include "p2j@filename@"',
                                      filename=filename)

                    self.contextNamespaces = ['p2j']
                    self.visitingOther = 1
                    n.accept(self)
        self.leaveNamespaces()
        self.hxx.out('#endif // not ' + guardname)

    def visitModule(self, node):
        self.contextNamespaces.append(node.identifier())
        for defn in node.definitions():
            defn.accept(self)
        self.contextNamespaces.pop()

    def visitForward(self, node):
        self.syncNamespaces()
        if not self.visitingOther:
            self.hxx.out('class ' + jnutils.CppName(node.identifier()) + ';')

    def visitInterface(self, node):
        if not self.visitingOther:
            self.syncNamespaces()
            self.hxx.out('PUBLIC_%s_PRE class PUBLIC_%s_POST %s' %
                         (self.defname, self.defname, jnutils.CppName(node.identifier())))

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

        virtual = ''
        if not isTerminal:
            virtual ='virtual '

        inh = filter(lambda x: x.repoId() != REPOID_ISUPPORTS, node.inherits())

        scopedname = jnutils.ScopedCppName(node)
        classname = 'p2j::' + scopedname
        self.classname = classname
        self.javaclass = string.join(node.scopedName(), '/')

        if not self.visitingOther:
            self.hxx.out('    : public @virtual@::iface::@scopedname@',
                         virtual=virtual, scopedname=scopedname)
            
            if len(inh) == 0:
                self.hxx.out('    , public @virtual@::p2j::XPCOM::IObject',
                             virtual=virtual)
            else:
                for c in inh:
                    if isinstance(c, idlast.Declarator) and c.alias():
                        c = c.alias().aliasType().unalias().decl()
                    
                    isAmbiguous = 0
                    iclassname = jnutils.ScopedCppName(c)
                    target = 'ambiguous-inheritance(' + iclassname + ')'
                    for p in node.pragmas():
                        if p.text() == target:
                            isAmbiguous = 1
                            break
                    if isAmbiguous:
                        virtual = 'virtual '
                    else:
                        virtual = ''
                    self.hxx.out('    , public @virtual@::p2j::@classname@',
                                 virtual=virtual, classname=iclassname)

            self.hxx.out('{')
            self.hxx.out('public:')
            self.hxx.inc_indent()
            self.hxx.out(jnutils.CppName(node.identifier()) + '() {}')
            self.hxx.out('PUBLIC_@defname@_PRE @classname@(JNIEnv* aEnv, ' +
                         'jobject aObject) PUBLIC_@defname@_POST;',
                         classname=jnutils.CppName(node.identifier()),
                         defname=self.defname)

        self.cpp.out(classname + '::' + jnutils.CppName(node.identifier()) + '(JNIEnv* aEnv, jobject aObject)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('env = aEnv;')
        self.cpp.out('mObject = aObject;')
        self.cpp.dec_indent()
        self.cpp.out('}')
        
        for n in node.contents():
            n.accept(self)

        if not self.visitingOther:
            self.hxx.dec_indent()
            self.hxx.out('};')

            fname = jnutils.CppName(string.join(node.scopedName(), '_')) + 'Factory'
            self.cpp.out('class ' + fname)
            self.cpp.out('  : public P2JFactory')
            self.cpp.out('{')
            self.cpp.out('public:')
            self.cpp.inc_indent()
            self.cpp.out(fname + '() : ::P2JFactory("' +\
                         scopedname + '", "' + self.javaclass +\
                         '") {}')
            self.cpp.out('void* create(JNIEnv* env, jobject obj) { return reinterpret_cast<void*>(' +\
                         'static_cast< ::iface::' + scopedname + '*>(new ' +\
                         classname + '(env, obj))); }')
            self.cpp.dec_indent()
            self.cpp.out('};')
            self.cpp.out(fname + ' s' + fname + ';');

    def visitOperation(self, node):
        rti = jnutils.GetTypeInformation(node.returnType().unalias())
        params = []
        for p in node.parameters():
            dirn = [jnutils.Type.IN, jnutils.Type.OUT, jnutils.Type.INOUT][p.direction()]
            pti = jnutils.GetTypeInformation(p.paramType().unalias())
            params.append([p.identifier(), pti, dirn])
        self.writeMethod(jnutils.CppName(node.identifier()), jnutils.JavaMangleName(node.identifier()),
                         rti, params)

    def visitAttribute(self, node):
        ti = jnutils.GetTypeInformation(node.attrType().unalias())

        for n in node.declarators():
            self.writeMethod(jnutils.CppName(n.identifier()),
                             jnutils.JavaMangleName(jnutils.AccessorName(n, 0)),
                             ti, [])

        if not node.readonly():
            for n in node.declarators():
                self.writeMethod(jnutils.CppName(n.identifier()),
                                 jnutils.JavaMangleName(jnutils.AccessorName(n, 1)),
                                 None, [['param', ti, jnutils.Type.IN]])

    def writeMethod(self, name, javaName, rtype, params):
        if rtype == None:
            rtypeName = 'void'
        else:
            rtypeName = rtype.pcmType(jnutils.Type.RETURN)

        javasig = '('
        paramstr = '('
        paramComma = 0
        for (pname, ti, dirn) in params:
            if ti.needLength():
                if paramComma:
                    paramstr = paramstr + ', '
                paramComma = 1
                if dirn == jnutils.Type.IN:
                    paramstr = paramstr + 'uint32_t _length_' + pname
                else:
                    paramstr = paramstr + 'uint32_t* _length_' + pname
            if paramComma:
                paramstr = paramstr + ', '
            paramstr = paramstr + ti.pcmType(dirn) + ' ' + pname
            javasig = javasig + ti.javaSig(dirn)
            paramComma = 1

        javasig = javasig + ')'
        if rtype != None:
            javasig = javasig + rtype.javaSig(jnutils.Type.RETURN)
        else:
            javasig = javasig + 'V'
        
        if rtype and rtype.needLength():
            if paramComma:
                paramstr = paramstr + ', '
            paramstr = paramstr + 'uint32_t* _length__ret'
            paramComma = 1
        
        paramstr = paramstr + ')'

        if not self.visitingOther:
            self.hxx.out('PUBLIC_' + self.defname + '_PRE ' + rtypeName + ' ' + name + paramstr + ' throw(std::exception&) ' + 'PUBLIC_' + self.defname + '_POST;')
        self.cpp.out(rtypeName + ' ' + self.classname + '::' + name + paramstr)
        self.cpp.out('  throw(std::exception&)')
        self.cpp.out('{')
        self.cpp.inc_indent()

        if (rtype != None and rtypeName != 'void'):
            needRet = 1
            self.cpp.out(rtype.jniType(jnutils.Type.RETURN) + ' _jni_ret;')
        else:
            needRet = 0
        
        # Set up storage for all parameters as the JNI type...
        jniParams = ''
        for (pname, ti, dirn) in params:
            self.cpp.out(ti.jniType(jnutils.Type.RETURN) + ' _jni_' + pname + ';')
            jniParams = jniParams + ', _jni_' + pname
            if dirn == jnutils.Type.OUT:
                continue
            self.cpp.out(ti.convertToJNI('_jni_' + pname, pname, dirn != jnutils.Type.IN))

        # Find the Java class...
        self.cpp.out('jclass thisclazz = env->FindClass(\"' + self.javaclass + '\");')
        self.cpp.out('jmethodID thismeth = env->GetMethodID(thisclazz, ' +\
                     '\"' + javaName + '\", \"' + javasig + '\");')
        
        # Find the method we are calling...
        if needRet:
            retsave = '_jni_ret = '
        else:
            retsave = ''

        retclose = ''
        
        if needRet and isinstance(rtype, jnutils.String):
            retsave = retsave + 'static_cast<jstring>('
            retclose = ')'

        calltype = 'Void'
        if rtype:
            calltype = rtype.jniCallType()
        self.cpp.out(retsave + 'env->Call' + calltype + 'Method(mObject, thismeth' +
                     jniParams + ')' + retclose + ';')

        # Convert out / inout parameters to PCM...
        for (pname, ti, dirn) in params:
            if dirn == jnutils.Type.IN:
                continue
            self.cpp.out(ti.convertToPCM('_jni_' + pname, pname, indirectOut = 1))

        if needRet:
            self.cpp.out(rtypeName + ' _pcm_ret;')
            self.cpp.out(rtype.convertToPCM('_jni_ret', '_pcm_ret'))

        # Clean up parameters...
        for (pname, ti, dirn) in params:
            self.cpp.out(ti.jniDestroy('_jni_' + pname))
        if needRet:
            self.cpp.out(rtype.jniDestroy('_jni_ret'))
            self.cpp.out('return _pcm_ret;')

        self.cpp.dec_indent()
        self.cpp.out('}')

def run(tree):
    iv = NativePCM2JVisitor()
    tree.accept(iv)
