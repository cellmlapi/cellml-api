import os
from omniidl import idlvisitor, output
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
        self.cpp = output.Stream(open('j2p' + basename + '.cpp', 'w'))
        gbasename = '';
        for i in basename:
            if (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z'):
                gbasename = gbasename + i
        self.hxx.out(
            "// This output is automatically generated. Do not edit.")
        self.modname = string.upper(gbasename)
        guardname='J2P__' + self.modname + '__INCLUDED'
        self.hxx.out('#ifndef ' + guardname)
        self.hxx.out("#define " + guardname)
        self.hxx.out('#include <jni.h>')
        self.hxx.out('#include "j2psupport.hxx"')
        self.hxx.out('#include "Iface' + basename + '.hxx"')
        self.cpp.out('#include <exception>')
        self.cpp.out('#include "cda_compiler_support.h"')
        self.cpp.out('#include "j2p' + basename + '.hxx"')
        self.cpp.out('#include "p2j' + basename + '.hxx"')
        self.cpp.out('#include <Utilities.hxx>')
        for n in node.declarations():
            if n.mainFile():
                n.accept(self)
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
        self.hxx.out('JWRAP_PUBLIC_PRE jobject ' + constructor + '(JNIEnv* env, ' + cxxclass + '* obj) JWRAP_PUBLIC_POST;')
        self.cpp.out('jobject ' + constructor + '(JNIEnv* env, ' + cxxclass + '* obj)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out("if (obj == NULL)")
        self.cpp.inc_indent()
        self.cpp.out('return NULL;')
        self.cpp.dec_indent()
        # If it is a p2j object, unwrap it...
        self.cpp.out('p2j::' + scopedn + ' * wrap = dynamic_cast<p2j::' + scopedn + '*>(obj);')
        self.cpp.out('if (wrap != NULL)')
        self.cpp.inc_indent()
        self.cpp.out('return env->NewLocalRef(wrap->unwrap());')
        self.cpp.dec_indent()
        
        # It is a non-Java C++ object, so make a Java wrapper for it...
        
        self.cpp.out('jclass clazz = env->FindClass("pjm2pcm/' + classsig + '");')
        self.cpp.out('jobject wrapper = env->AllocObject(clazz);')
        self.cpp.out('jfieldID fid = env->GetFieldID(clazz, "nativePtr", "J");')
        self.cpp.out('obj->add_ref();')
        self.cpp.out('jlong field = reinterpret_cast<int64_t>(obj);')
        self.cpp.out('env->SetLongField(wrapper, fid, field);')

        self.cpp.out('fid = env->GetFieldID(clazz, "nativePtr_xpcom_iobject", "J");')
        self.cpp.out('field = reinterpret_cast<int64_t>(static_cast<iface::XPCOM::IObject*>(obj));')
        self.cpp.out('env->SetLongField(wrapper, fid, field);')

        self.recurseBuildInheritedFieldSetup(node)

        self.cpp.out('return wrapper;')
        self.cpp.dec_indent()
        self.cpp.out('}')

        # Write a finalizer...
        self.pushManglePart('finalize')
        self.calculateMangled()
        self.cpp.out('extern "C" { JWRAP_PUBLIC_PRE void ' + self.mangled +
                     '(JNIEnv* env, jobject thisptr) JWRAP_PUBLIC_POST; }')
        self.cpp.out('void ' + self.mangled + '(JNIEnv* env, jobject thisptr)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('jclass thisclazz = env->GetObjectClass(thisptr);')
        self.cpp.out('jfieldID fid = env->GetFieldID(thisclazz, "nativePtr", "J");')
        self.cpp.out('reinterpret_cast<' + self.cxxclass +
                     '*>(env->GetLongField(thisptr, fid))->release_ref();')
        self.cpp.dec_indent()
        self.cpp.out('}')
        self.popManglePart()

        self.pushManglePart('nqueryInterface')
        self.calculateMangled()
        self.cpp.out('extern "C" { JWRAP_PUBLIC_PRE jobject ' + self.mangled +
                     '(JNIEnv* env, jclass* clazz, jlong fromptr) JWRAP_PUBLIC_POST; }')
        self.cpp.out('jobject ' + self.mangled + '(JNIEnv* env, jclass* clazz, jlong fromptr)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('iface::XPCOM::IObject * obj = reinterpret_cast<iface::XPCOM::IObject*>' +
                     '(fromptr);')
        self.cpp.out('if (obj == NULL) { env->ExceptionClear(); return NULL; }')
        self.cpp.out('DECLARE_QUERY_INTERFACE_OBJREF(qiobj, obj, ' + scopedn + ');')
        # If qiobj is null, it doesn't implement the requested interface...
        self.cpp.out('if (qiobj == NULL) return NULL;')
        self.cpp.out('jobject objj = ' + constructor + '(env, qiobj);')
        self.cpp.out('return objj;')
        self.cpp.dec_indent()
        self.cpp.out('}')
        self.popManglePart()

        self.pushManglePart('hashCode')
        self.calculateMangled()
        self.cpp.out('extern "C" { JWRAP_PUBLIC_PRE jint ' + self.mangled +
                     '(JNIEnv* env, jobject thisptr) JWRAP_PUBLIC_POST; }')
        self.cpp.out('jint ' + self.mangled + '(JNIEnv* env, jobject thisptr)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('jclass thisclazz = env->GetObjectClass(thisptr);')
        self.cpp.out('jfieldID fid = env->GetFieldID(thisclazz, "nativePtr", "J");')
        self.cpp.out('char* oid1 = reinterpret_cast<' + self.cxxclass +
                     '*>(env->GetLongField(thisptr, fid))->objid();')
        self.cpp.out('jint hash = 0;')
        self.cpp.out('size_t l = strlen(oid1);');
        self.cpp.out('for (uint32_t i = 0; i < l; i++)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('uint32_t n = (i * 13) % 32;')
        self.cpp.out('hash ^= (oid1[i] << n) | (oid1[i] >> (32 - n));')
        self.cpp.dec_indent()
        self.cpp.out('}')
        self.cpp.out('free(oid1);')
        self.cpp.out('return hash;')
        self.cpp.dec_indent()
        self.cpp.out('}')
        self.popManglePart()

        self.recurseAcceptInheritedContents(node)
        
        self.inameParts.pop()
        self.popManglePart()

    def recurseBuildInheritedFieldSetup(self, node):
        self.cpp.out('{')
        self.cpp.inc_indent()

        fieldName = 'nativePtr_' + string.join(node.scopedName(), '_')
        self.cpp.out('jfieldID fid = env->GetFieldID(clazz, "' + fieldName + '", "J");')
        className = string.join(node.scopedName(), '::')
        self.cpp.out('field = reinterpret_cast<int64_t>' +\
                     '(static_cast<iface::' + className + '*>(obj));')
        self.cpp.out('env->SetLongField(wrapper, fid, field);')
        
        self.cpp.dec_indent()
        self.cpp.out('}')
        for i in node.inherits():
            if i.scopedName() != ['XPCOM', 'IObject']:
                self.recurseBuildInheritedFieldSetup(i)
        
    def recurseAcceptInheritedContents(self, node):
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
        
        self.hxx.out('extern "C" { JWRAP_PUBLIC_PRE ' + rtypeName + ' ' + name +
                     '(' + paramString + ') JWRAP_PUBLIC_POST; };')
        self.cpp.out(rtypeName + ' ' + name + '(' + paramString + ')')
        self.cpp.out('{')
        self.cpp.inc_indent()

        if (rtype != None and rtypeName != 'void'):
            needRet = 1
            self.cpp.out(rtype.pcmType(jnutils.Type.DERIVE) + ' _pcm_ret;')
            if rtype.needLength():
                self.cpp.out('uint32_t _length__pcm_ret;')
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
                self.cpp.out('uint32_t _length__pcm_' + pname + ';')
                pcmParams = pcmParams + indirect + '_length__pcm_' + pname
            if pcmParams != '':
                pcmParams = pcmParams + ', '
            self.cpp.out(ti.pcmType(jnutils.Type.DERIVE) + ' _pcm_' + pname + ';')
            pcmParams = pcmParams + indirect + '_pcm_' + pname
            if dirn == jnutils.Type.OUT:
                continue
            self.cpp.out(ti.convertToPCM(pname, '_pcm_' + pname, dirn != jnutils.Type.IN))

        if needRet and rtype.needLength():
            if pcmParams != '':
                pcmParams = pcmParams + ', '
            pcmParams = pcmParams + '&_length__pcm_ret';

        # Next, we need to extract the 'this' pointer...
        self.cpp.out(self.cxxclass + '* pcm_this;')
        self.cpp.out('{')
        self.cpp.inc_indent()
        self.cpp.out('jclass thisclazz = env->GetObjectClass(thisptr);')
        self.cpp.out('jfieldID fid = env->GetFieldID(thisclazz, "nativePtr", "J");')
        self.cpp.out('pcm_this = reinterpret_cast<' + self.cxxclass + '*>(env->GetLongField(thisptr, fid));')
        self.cpp.dec_indent()
        self.cpp.out('}')
        
        # Make the call to the PCM interface...
        if needRet:
            retsave = '_pcm_ret = '
        else:
            retsave = ''

        self.cpp.out('try')
        self.cpp.out('{')
        self.cpp.inc_indent()
        
        self.cpp.out(retsave + 'pcm_this->' + pcmName + '(' + pcmParams + ');')

        self.cpp.dec_indent()
        self.cpp.out('}')
        self.cpp.out ('catch (...)')
        self.cpp.out('{')
        self.cpp.inc_indent()
        # Clean up parameters...
        for (pname, ti, dirn) in params:
            self.cpp.out(ti.pcmDestroy('_pcm_' + pname))
        # Raise an exception...
        self.cpp.out('jclass eclazz = env->FindClass("java/lang/RuntimeException");')
        self.cpp.out('env->ThrowNew(eclazz, "Native code threw exception");');
        if needRet:
            self.cpp.out('return NULL;');
        else:
            self.cpp.out('return;');        
        self.cpp.dec_indent()
        self.cpp.out('}')

        # Convert out / inout parameters to JNI...
        for (pname, ti, dirn) in params:
            if dirn == jnutils.Type.IN:
                continue
            self.cpp.out(ti.convertToJNI(pname, '_pcm_' + pname, indirectOut = 1))

        if needRet:
            self.cpp.out(rtypeName + ' _jni_ret;')
            self.cpp.out(rtype.convertToJNI('_jni_ret', '_pcm_ret'))
        
        # Clean up parameters...
        for (pname, ti, dirn) in params:
            self.cpp.out(ti.pcmDestroy('_pcm_' + pname))
        if needRet:
            self.cpp.out(rtype.pcmDestroy('_pcm_ret'))
            self.cpp.out('return _jni_ret;')

        self.cpp.dec_indent()
        self.cpp.out('}')
    
def run(tree):
    iv = NativeStubVisitor()
    tree.accept(iv)
