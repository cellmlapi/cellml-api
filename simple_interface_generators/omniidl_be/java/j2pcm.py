import os
from omniidl import idlvisitor, output, idlast
import jnutils
import string

class NativeStubVisitor (idlvisitor.AstVisitor):
    def __init__(self):
        self.mangleParts = ['Java', 'pjm2pcm']
        self.inameParts = []
        self.mangled = ''

    def visitAST(self, node):
        directory, basename = os.path.split(node.file())
        
        self._included = ['xpcom.idl']
        if string.lower(basename[-4:]) == '.idl':
            basename = basename[:-4]

        self.hxx = output.Stream(open('j2p' + basename + '.hxx', 'w'))
        self.cppMod = output.Stream(open('j2p' + basename + 'Mod.cpp', 'w'))
        self.cppSup = output.Stream(open('j2p' + basename + 'Sup.cpp', 'w'))

        self.hxx.out(
            "// This output is automatically generated. Do not edit.")
        self.modname = basename
        self.defname='J2P__' + self.modname
        guardname=self.defname + '__INCLUDED'
        self.hxx.out('#ifndef ' + guardname)
        self.hxx.out("#define " + guardname)
        self.hxx.out('#include "pick-jni.h"')
        self.hxx.out('#include "j2psupport.hxx"')
        self.hxx.out('#include "Iface' + basename + '.hxx"')

        self.cppMod.out('#include <exception>')
        self.cppMod.out('#include "cda_compiler_support.h"')
        self.cppMod.out('#include "j2p' + basename + '.hxx"')
        self.cppMod.out('#include "p2j' + basename + '.hxx"')
        self.cppMod.out('#include <Utilities.hxx>')
        self.cppSup.out('#include <exception>')
        self.cppSup.out('#include "cda_compiler_support.h"')
        self.cppSup.out('#include "j2p' + basename + '.hxx"')
        self.cppSup.out('#include "p2j' + basename + '.hxx"')
        self.cppSup.out('#include <Utilities.hxx>')
        for n in node.declarations():
            if n.mainFile():
                pass
            else:
                filename = n.file()
                pos = string.rfind(filename, '/')
                if pos != -1:
                    filename = filename[pos + 1:]

                if not filename in self._included:
                    self._included.append(filename)

                    if filename[-4:] == ".idl":
                        filename = filename[0:-4] + ".hxx"

                    if filename != "xpcom.idl":
                        self.hxx.out('#include "j2p@filename@"',
                                      filename=filename)

        self.hxx.out('#undef PUBLIC_JAVAMOD_PRE')
        self.hxx.out('#undef PUBLIC_JAVAMOD_POST')
        self.hxx.out('#ifdef IN_MODULE_%s' % self.defname)
        self.hxx.out('#define PUBLIC_JAVAMOD_PRE CDA_EXPORT_PRE')
        self.hxx.out('#define PUBLIC_JAVAMOD_POST CDA_EXPORT_POST')
        self.hxx.out('#else')
        self.hxx.out('#define PUBLIC_JAVAMOD_PRE CDA_IMPORT_PRE')
        self.hxx.out('#define PUBLIC_JAVAMOD_POST CDA_IMPORT_POST')
        self.hxx.out('#endif')
        self.hxx.out('#undef PUBLIC_JAVALIB_PRE')
        self.hxx.out('#undef PUBLIC_JAVALIB_POST')
        self.hxx.out('#ifdef IN_LIBRARY_%s' % self.defname)
        self.hxx.out('#define PUBLIC_JAVALIB_PRE CDA_EXPORT_PRE')
        self.hxx.out('#define PUBLIC_JAVALIB_POST CDA_EXPORT_POST')
        self.hxx.out('#else')
        self.hxx.out('#define PUBLIC_JAVALIB_PRE CDA_IMPORT_PRE')
        self.hxx.out('#define PUBLIC_JAVALIB_POST CDA_IMPORT_POST')
        self.hxx.out('#endif')

        for n in node.declarations():
            if n.mainFile():
                n.accept(self)
        self.hxx.out('#endif // not ' + guardname)
    
    def pushManglePart(self, name):
        self.mangleParts.append(string.replace(string.replace(name, '_', '_1'), '/', '_'))
    
    def popManglePart(self):
        self.mangleParts.pop()
    
    def calculateMangled(self):
        self.mangled = string.join(self.mangleParts, '_')

    def visitModule(self, node):
        self.inameParts.append(node.identifier());
        self.pushManglePart(jnutils.JavaName(node))
        for defn in node.definitions():
            defn.accept(self)
        self.popManglePart()
        self.inameParts.pop()

    def findAllInherits(self, node):
        inherits = []
        for n in node.inherits():
            inherits.append(n)
            inherits = inherits + self.findAllInherits(n)
        return inherits

    def visitInterface(self, node):
        self.pushManglePart(jnutils.JavaName(node))
        self.inameParts.append(node.identifier())
        
        constructor = 'wrap_' + string.join(self.inameParts, '_')
        classsig = string.join(self.inameParts, '/')
        scopedn = jnutils.ScopedCppName(node)
        cxxclass = 'iface::' + scopedn
        self.cxxclass = cxxclass
        self.hxx.out('PUBLIC_JAVALIB_PRE jobject ' + constructor + '(JNIEnv* env, ' + cxxclass + '* obj) PUBLIC_JAVALIB_POST;')
        self.cppSup.out('jobject ' + constructor + '(JNIEnv* env, ' + cxxclass + '* obj)')
        self.cppSup.out('{')
        self.cppSup.inc_indent()
        self.cppSup.out("if (obj == NULL)")
        self.cppSup.inc_indent()
        self.cppSup.out('return NULL;')
        self.cppSup.dec_indent()
        # If it is a p2j object, unwrap it...
        self.cppSup.out('p2j::' + scopedn + ' * wrap = dynamic_cast<p2j::' + scopedn + '*>(obj);')
        self.cppSup.out('if (wrap != NULL)')
        self.cppSup.inc_indent()
        self.cppSup.out('return env->NewLocalRef(wrap->unwrap());')
        self.cppSup.dec_indent()
        
        # It is a non-Java C++ object, so make a Java wrapper for it...
        
        self.cppSup.out('jclass clazz = env->FindClass("pjm2pcm/' + classsig + '");')
        self.cppSup.out('jobject wrapper = env->AllocObject(clazz);')
        self.cppSup.out('jfieldID fid = env->GetFieldID(clazz, "nativePtr", "J");')
        self.cppSup.out('obj->add_ref();')
        self.cppSup.out('jlong field = reinterpret_cast<int64_t>(obj);')
        self.cppSup.out('env->SetLongField(wrapper, fid, field);')

        self.cppSup.out('fid = env->GetFieldID(clazz, "nativePtr_xpcom_iobject", "J");')
        self.cppSup.out('field = reinterpret_cast<int64_t>(static_cast<iface::XPCOM::IObject*>(obj));')
        self.cppSup.out('env->SetLongField(wrapper, fid, field);')

        self.recurseBuildInheritedFieldSetup(node)

        self.cppSup.out('return wrapper;')
        self.cppSup.dec_indent()
        self.cppSup.out('}')

        # Write a finalizer...
        self.pushManglePart('finalize')
        self.calculateMangled()
        self.hxx.out('extern "C" { PUBLIC_JAVAMOD_PRE void ' + self.mangled +
                     '(JNIEnv* env, jobject thisptr) PUBLIC_JAVAMOD_POST; }')
        self.cppMod.out('void ' + self.mangled + '(JNIEnv* env, jobject thisptr)')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        self.cppMod.out('jclass thisclazz = env->GetObjectClass(thisptr);')
        self.cppMod.out('jfieldID fid = env->GetFieldID(thisclazz, "nativePtr", "J");')
        self.cppMod.out('reinterpret_cast<' + self.cxxclass +
                        '*>(env->GetLongField(thisptr, fid))->release_ref();')
        self.cppMod.dec_indent()
        self.cppMod.out('}')
        self.popManglePart()

        self.pushManglePart('nqueryInterface')
        self.calculateMangled()
        self.hxx.out('extern "C" { PUBLIC_JAVAMOD_PRE jobject ' + self.mangled +
                       '(JNIEnv* env, jclass* clazz, jlong fromptr) PUBLIC_JAVAMOD_POST; }')
        self.cppMod.out('jobject ' + self.mangled + '(JNIEnv* env, jclass* clazz, jlong fromptr)')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        self.cppMod.out('iface::XPCOM::IObject * obj = reinterpret_cast<iface::XPCOM::IObject*>' +
                        '(fromptr);')
        self.cppMod.out('if (obj == NULL) { env->ExceptionClear(); return NULL; }')
        self.cppMod.out('DECLARE_QUERY_INTERFACE_OBJREF(qiobj, obj, ' + scopedn + ');')
        # If qiobj is null, it doesn't implement the requested interface...
        self.cppMod.out('if (qiobj == NULL) return NULL;')
        self.cppMod.out('jobject objj = ' + constructor + '(env, qiobj);')
        self.cppMod.out('return objj;')
        self.cppMod.dec_indent()
        self.cppMod.out('}')
        self.popManglePart()

        self.pushManglePart('hashCode')
        self.calculateMangled()
        self.hxx.out('extern "C" { PUBLIC_JAVAMOD_PRE jint ' + self.mangled +
                        '(JNIEnv* env, jobject thisptr) PUBLIC_JAVAMOD_POST; }')
        self.cppMod.out('jint ' + self.mangled + '(JNIEnv* env, jobject thisptr)')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        self.cppMod.out('jclass thisclazz = env->GetObjectClass(thisptr);')
        self.cppMod.out('jfieldID fid = env->GetFieldID(thisclazz, "nativePtr", "J");')
        self.cppMod.out('char* oid1 = reinterpret_cast<' + self.cxxclass +
                        '*>(env->GetLongField(thisptr, fid))->objid();')
        self.cppMod.out('jint hash = 0;')
        self.cppMod.out('size_t l = strlen(oid1);');
        self.cppMod.out('for (uint32_t i = 0; i < l; i++)')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        self.cppMod.out('uint32_t n = (i * 13) % 32;')
        self.cppMod.out('hash ^= (oid1[i] << n) | (oid1[i] >> (32 - n));')
        self.cppMod.dec_indent()
        self.cppMod.out('}')
        self.cppMod.out('free(oid1);')
        self.cppMod.out('return hash;')
        self.cppMod.dec_indent()
        self.cppMod.out('}')
        self.popManglePart()

        self.recurseAcceptInheritedContents(node)
        
        self.inameParts.pop()
        self.popManglePart()

    def recurseBuildInheritedFieldSetup(self, node):
        self.cppSup.out('{')
        self.cppSup.inc_indent()

        if isinstance(node, idlast.Declarator) and node.alias():
            node = node.alias().aliasType().unalias().decl()

        fieldName = 'nativePtr_' + string.join(node.scopedName(), '_')
        self.cppSup.out('jfieldID fid = env->GetFieldID(clazz, "' + fieldName + '", "J");')
        className = string.join(node.scopedName(), '::')
        self.cppSup.out('field = reinterpret_cast<int64_t>' +\
                        '(static_cast<iface::' + className + '*>(obj));')
        self.cppSup.out('env->SetLongField(wrapper, fid, field);')
        
        self.cppSup.dec_indent()
        self.cppSup.out('}')
        for i in node.inherits():
            if i.scopedName() != ['XPCOM', 'IObject']:
                self.recurseBuildInheritedFieldSetup(i)
        
    def recurseAcceptInheritedContents(self, node):
        if isinstance(node, idlast.Declarator) and node.alias():
            node = node.alias().aliasType().unalias().decl()
        
        for i in node.contents():
            i.accept(self)
        for i in node.inherits():
            if i.scopedName() != ['XPCOM', 'IObject']:
                self.recurseAcceptInheritedContents(i)

    def visitOperation(self, node):
        rti = jnutils.GetTypeInformation(node.returnType().unalias())
        params = []
        for p in node.parameters():
            dirn = [jnutils.Type.IN, jnutils.Type.OUT, jnutils.Type.INOUT][p.direction()]
            pti = jnutils.GetTypeInformation(p.paramType().unalias())
            params.append([p.identifier(), pti, dirn])
        self.pushManglePart(node.identifier())
        self.calculateMangled()
        self.writeMethod(self.mangled, jnutils.CppName(node.identifier()), rti, params)
        self.popManglePart()

    def visitAttribute(self, node):
        ti = jnutils.GetTypeInformation(node.attrType().unalias())

        for n in node.declarators():
            self.pushManglePart(jnutils.AccessorName(n, 0))
            self.calculateMangled()
            self.writeMethod(self.mangled, jnutils.CppName(n.identifier()),
                             ti, [])
            self.popManglePart()

        if not node.readonly():
            itype = ti.jniType(jnutils.Type.IN)
            for n in node.declarators():
                self.pushManglePart(jnutils.AccessorName(n, 1))
                self.calculateMangled()
                self.writeMethod(self.mangled, jnutils.CppName(n.identifier()),
                                 None, [['param', ti, jnutils.Type.IN]])
                self.popManglePart()

    def writeMethod(self, name, pcmName, rtype, params):
        if rtype == None:
            rtypeName = 'void'
        else:
            rtypeName = rtype.jniType(jnutils.Type.RETURN)

        paramString = 'JNIEnv* env, jobject thisptr'
        for (pname, ti, dirn) in params:
            tiName = ti.jniType(dirn)
            paramString = paramString + ', ' + tiName + ' ' + pname
        
        self.hxx.out('extern "C" { PUBLIC_JAVAMOD_PRE ' + rtypeName + ' ' + name +
                     '(' + paramString + ') PUBLIC_JAVAMOD_POST; };')
        self.cppMod.out(rtypeName + ' ' + name + '(' + paramString + ')')
        self.cppMod.out('{')
        self.cppMod.inc_indent()

        if (rtype != None and rtypeName != 'void'):
            needRet = 1
            self.cppMod.out(rtype.pcmType(jnutils.Type.DERIVE) + ' _pcm_ret;')
            if rtype.needLength():
                self.cppMod.out('uint32_t _length__pcm_ret;')
        else:
            needRet = 0

        pcmParams = ''
        for (pname, ti, dirn) in params:
            # We need to convert in parameters to the CXX type...
            indirect = ''
            if dirn != jnutils.Type.IN:
                indirect = '&'
            if ti.needLength():
                if pcmParams != '':
                    pcmParams = pcmParams + ', '
                self.cppMod.out('uint32_t _length__pcm_' + pname + ';')
                pcmParams = pcmParams + indirect + '_length__pcm_' + pname
            if pcmParams != '':
                pcmParams = pcmParams + ', '
            self.cppMod.out(ti.pcmType(jnutils.Type.DERIVE) + ' _pcm_' + pname + ';')
            pcmParams = pcmParams + indirect + '_pcm_' + pname
            if dirn == jnutils.Type.OUT:
                continue
            self.cppMod.out(ti.convertToPCM(pname, '_pcm_' + pname, dirn != jnutils.Type.IN))

        if needRet and rtype.needLength():
            if pcmParams != '':
                pcmParams = pcmParams + ', '
            pcmParams = pcmParams + '&_length__pcm_ret';

        # Next, we need to extract the 'this' pointer...
        self.cppMod.out(self.cxxclass + '* pcm_this;')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        self.cppMod.out('jclass thisclazz = env->GetObjectClass(thisptr);')
        self.cppMod.out('jfieldID fid = env->GetFieldID(thisclazz, "nativePtr", "J");')
        self.cppMod.out('pcm_this = reinterpret_cast<' + self.cxxclass + '*>(env->GetLongField(thisptr, fid));')
        self.cppMod.dec_indent()
        self.cppMod.out('}')
        
        # Make the call to the PCM interface...
        if needRet:
            retsave = '_pcm_ret = '
        else:
            retsave = ''

        self.cppMod.out('try')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        
        self.cppMod.out(retsave + 'pcm_this->' + pcmName + '(' + pcmParams + ');')

        self.cppMod.dec_indent()
        self.cppMod.out('}')
        self.cppMod.out ('catch (...)')
        self.cppMod.out('{')
        self.cppMod.inc_indent()
        # Clean up parameters...
        for (pname, ti, dirn) in params:
            self.cppMod.out(ti.pcmDestroy('_pcm_' + pname))
        # Raise an exception...
        self.cppMod.out('jclass eclazz = env->FindClass("java/lang/RuntimeException");')
        self.cppMod.out('env->ThrowNew(eclazz, "Native code threw exception");');
        if needRet:
            self.cppMod.out('return NULL;');
        else:
            self.cppMod.out('return;');        
        self.cppMod.dec_indent()
        self.cppMod.out('}')

        # Convert out / inout parameters to JNI...
        for (pname, ti, dirn) in params:
            if dirn == jnutils.Type.IN:
                continue
            self.cppMod.out(ti.convertToJNI(pname, '_pcm_' + pname, indirectOut = 1))

        if needRet:
            self.cppMod.out(rtypeName + ' _jni_ret;')
            self.cppMod.out(rtype.convertToJNI('_jni_ret', '_pcm_ret'))
        
        # Clean up parameters...
        for (pname, ti, dirn) in params:
            self.cppMod.out(ti.pcmDestroy('_pcm_' + pname))
        if needRet:
            self.cppMod.out(rtype.pcmDestroy('_pcm_ret'))
            self.cppMod.out('return _jni_ret;')

        self.cppMod.dec_indent()
        self.cppMod.out('}')
    
def run(tree):
    iv = NativeStubVisitor()
    tree.accept(iv)
