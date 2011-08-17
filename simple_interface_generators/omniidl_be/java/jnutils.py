import string
from omniidl import idltype

def GetTypeInformation(type):
    if isinstance(type, idltype.Base):
        return Base(type)
    elif isinstance(type, idltype.String):
        return String(type)
    elif isinstance(type, idltype.WString):
        return WString(type)
    elif isinstance(type, idltype.Sequence):
        return Sequence(type)
    elif isinstance(type, idltype.Declared):
        if type.kind() == idltype.tk_struct:
            return Struct(type)
        elif type.kind() == idltype.tk_objref:
            return Objref(type)
        elif type.kind() == idltype.tk_enum:
            return Enum(type)
        else:
            return Declared(type)

class Type:
    IN = 1
    OUT = 2
    INOUT = 3
    RETURN = 4
    DERIVE = 5

    def javaType(self, direction):
        return {Type.IN: self.java_type,
                Type.OUT: 'pjm.Reference<' + self.java_type + '>',
                Type.INOUT: 'pjm.Reference<' + self.java_type + '>',
                Type.RETURN: self.java_type,
                Type.DERIVE: self.java_type
               }[direction]

    def jniType(self, direction):
        return {Type.IN: 'jobject',
                Type.OUT: 'jobject',
                Type.INOUT: 'jobject',
                Type.RETURN: 'jobject',
                Type.DERIVE: 'jobject'
               }[direction]

    def javaSig(self, direction):
        return {Type.IN: self.java_sig,
                Type.OUT: 'Lpjm/Reference;',
                Type.INOUT: 'Lpjm/Reference;',
                Type.RETURN: self.java_sig,
                Type.DERIVE: self.java_sig
               }[direction]

    def jniCallType(self):
        return 'Object'

    def needLength(self):
        return 0

    def pcmDestroy(self, name):
        return ''

    def jniDestroy(self, name):
        return ''

    def readJNIReference(self, code, jniname):
        return "{\n" +\
               "  jclass tmpclazz = env->FindClass(\"pjm/reference\");\n" +\
               "  jmethodID tmpgetref = env->GetMethodID(tmpclazz, \"get\", \"()Ljava/lang/Object;\");\n" +\
               "  jobject tmpobj = env->CallObjectMethod(" + jniname + ", tmpgetref);\n" +\
               code +\
               "}\n"

    def writeJNIReference(self, code, jniname):
        return "{\n" +\
               "  jclass tmpclazz = env->FindClass(\"pjm/reference\");\n" +\
               "  jmethodID tmpsetref = env->GetMethodID(tmpclazz, \"set\", \"(Ljava/lang/Object;)V\");\n" +\
               "  jobject tmpobj;\n" +\
               code +\
               "  env->CallVoidMethod(" + jniname + ", tmpsetref, tmpobj);\n" +\
               "}\n"

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        if indirectIn or indirectOut or unbox:
            raise "Not implemented: convertToPCM for non-direct out on Type."
        return pcmname + ' = ' + jniname + ';'

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        if indirectIn or indirectOut or box:
            raise "Not implemented: convertToJNI for non-direct out on Type."
        return jniname + ' = ' + pcmname + ';'
        
BASE_MAP = {
    idltype.tk_void: {'wrapper_class': 'invalid!', 'java_type': 'void', 'jni_type': 'void',
                      'pcm_type': 'void', 'java_sig': 'V', 'call_type': 'Void'},
    idltype.tk_short: {'wrapper_class': 'Short', 'java_type': 'short', 'jni_type': 'jshort',
                       'pcm_type': 'int16_t', 'java_sig': 'S', 'call_type': 'Short'},
    idltype.tk_long: {'wrapper_class': 'Integer', 'java_type': 'int', 'jni_type': 'jint',
                      'pcm_type': 'int32_t', 'java_sig': 'I', 'call_type': 'Int'},
    # We have to go up one size for unsigned numbers, unfortunately.
    idltype.tk_ushort: {'wrapper_class': 'Integer', 'java_type': 'int', 'jni_type': 'jint',
                        'pcm_type': 'uint16_t', 'java_sig': 'I', 'call_type': 'Int'},
    idltype.tk_ulong: {'wrapper_class': 'Long', 'java_type': 'long', 'jni_type': 'jlong',
                       'pcm_type': 'uint32_t', 'java_sig': 'J', 'call_type': 'Long'},
    idltype.tk_float: {'wrapper_class': 'Float', 'java_type': 'float', 'jni_type': 'jfloat',
                       'pcm_type': 'float', 'java_sig': 'F', 'call_type': 'Float'},
    idltype.tk_double: {'wrapper_class': 'Double', 'java_type': 'double', 'jni_type': 'jdouble',
                        'pcm_type': 'double', 'java_sig': 'D', 'call_type': 'Double'},
    idltype.tk_boolean: {'wrapper_class': 'Boolean', 'java_type': 'boolean', 'jni_type': 'jboolean',
                         'pcm_type': 'bool', 'java_sig': 'Z', 'call_type': 'Boolean'},
    idltype.tk_char: {'wrapper_class': 'Character', 'java_type': 'char', 'jni_type': 'jchar',
                      'pcm_type': 'char', 'java_sig': 'C', 'call_type': 'Char'},
    idltype.tk_wchar: {'wrapper_class': 'Character', 'java_type': 'char', 'jni_type': 'jchar',
                       'pcm_type': 'wchar_t', 'java_sig': 'C', 'call_type': 'Char'},
    idltype.tk_octet: {'wrapper_class': 'Byte', 'java_type': 'byte', 'jni_type': 'jbyte',
                       'pcm_type': 'uint8_t', 'java_sig': 'B', 'call_type': 'Byte'},
    idltype.tk_longlong: {'wrapper_class': 'Long', 'java_type': 'long', 'jni_type': 'jlong',
                          'pcm_type': 'int64_t', 'java_sig': 'J', 'call_type': 'Long'},
    idltype.tk_ulonglong: {'wrapper_class': 'Long', 'java_type': 'long', 'jni_type': 'jlong',
                           'pcm_type': 'uint64_t', 'java_sig': 'J', 'call_type': 'Long'}
}

class Base(Type):
    def __init__(self, type):
        k = type.kind()
        info = BASE_MAP[k]
        if info == None:
            info = {'wrapper_class': 'unknown',
                    'java_type': 'unknown', 'jni_type': 'unknown'}
        self.__dict__.update(info)

    def javaType(self, direction):
        return {Type.IN: self.java_type,
                Type.OUT: 'pjm.Reference<' + self.wrapper_class + '>',
                Type.INOUT: 'pjm.Reference<' + self.wrapper_class + '>',
                Type.RETURN: self.java_type,
                Type.DERIVE: self.wrapper_class
               }[direction]

    def jniType(self, direction):
        return {Type.IN: self.jni_type,
                Type.OUT: 'jobject',
                Type.INOUT: 'jobject',
                Type.RETURN: self.jni_type,
                Type.DERIVE: 'jobject'
               }[direction]

    def jniCallType(self):
        return self.call_type

    def pcmType(self, direction):
        indirection = ''
        ckw = ''
        if direction == Type.INOUT or direction == Type.OUT:
            indirection = '*'
        return self.pcm_type + indirection

    def javaSig(self, direction):
        return {Type.IN: self.java_sig,
                Type.OUT: 'Lpjm/Reference;',
                Type.INOUT: 'Lpjm/Reference;',
                Type.RETURN: self.java_sig,
                Type.DERIVE: 'Ljava/lang/' + self.wrapper_class + ';'
               }[direction]

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        indo = ''
        if indirectOut:
            indo = '*'

        if indirectIn:
            unbox = "jclass boxclazz = env->FindClass(\"java/lang/" +\
                    self.wrapper_class + "\");\n" +\
                    "jmethodID unboxmethod = env->GetMethodID(boxclazz, \"" +\
                    self.java_type + "Value\", \"()" + self.java_sig + "\");\n" +\
                    indo + pcmname + ' = static_cast<' + self.pcm_type + ">(" +\
                    'env->Call' + string.capitalize(self.java_type) +\
                    "Method(tmpobj, unboxmethod));\n"
            return self.readJNIReference(unbox, jniname)

        if unbox:
            return "{\n" +\
                   "jclass boxclazz = env->FindClass(\"java/lang/" +\
                   self.wrapper_class + "\");\n" +\
                   "jmethodID unboxmethod = env->GetMethodID(boxclazz, \"" +\
                    self.java_type + "Value\", \"()" + self.java_sig + "\");\n" +\
                    indo + pcmname + ' = static_cast<' + self.pcm_type + ">(" +\
                    'env->Call' + string.capitalize(self.java_type) +\
                    "Method(" + jniname + ", unboxmethod));\n}\n"
        
        return indo + pcmname + ' = static_cast<' + self.pcm_type + '>(' + jniname + ');'

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        indi = ''
        if indirectIn:
            indi = '*'

        if indirectOut:
            box = "jclass boxclazz = env->FindClass(\"java/lang/" +\
                  self.wrapper_class + "\");\n" +\
                  "jmethodID initmethod = env->GetMethodID(boxclazz, \"" +\
                  "<init>\", \"(" + self.java_sig + ")V\");" +\
                  "tmpobj = env->NewObject(boxclazz, initmethod, static_cast<" +\
                  self.jni_type + ">(" + indi + pcmname + "));\n"
            return self.writeJNIReference(box, jniname)

        if box:
            return "{\n" +\
                   "jclass boxclazz = env->FindClass(\"java/lang/" +\
                   self.wrapper_class + "\");\n" +\
                   "jmethodID initmethod = env->GetMethodID(boxclazz, \"" +\
                   "<init>\", \"(" + self.java_sig + ")V\");" +\
                   jniname + " = env->NewObject(boxclazz, initmethod, static_cast<" +\
                   self.jni_type + ">(" + indi + pcmname + "));\n}\n"
        
        return indi + jniname + ' = static_cast<' + self.jni_type + '>(' + pcmname + ');'

class String(Type):
    def __init__(self, type):
        self.java_type = 'String'

    def jniType(self, direction):
        return {Type.IN: 'jstring',
                Type.OUT: 'jobject',
                Type.INOUT: 'jobject',
                Type.RETURN: 'jstring',
                Type.DERIVE: 'jobject'
               }[direction]

    def pcmType(self, direction):
        return {Type.IN: 'const char*',
                Type.OUT: 'char**',
                Type.INOUT: 'char**',
                Type.RETURN: 'char*',
                Type.DERIVE: 'char*'
               }[direction]

    def pcmDestroy(self, name):
        return 'free(' + name + ');'

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        if indirectOut:
            oname = '*' + pcmname
        else:
            oname = pcmname

        if indirectIn:
            xname = 'tmpobj'
        elif unbox:
            xname = 'static_cast<jstring>(' + jniname + ')'
        else:
            xname = jniname
        
        extract = "{\n" +\
                  "  uint32_t tmplen = env->GetStringUTFLength(" + xname + ");\n" +\
                  "  " + oname + " = (char*)malloc(tmplen + 1);\n" +\
                  "  const char* tmpstr = env->GetStringUTFChars(" + xname + ", NULL);\n" +\
                  "  memcpy(" + oname + ", tmpstr, tmplen);\n" +\
                  "  (" + oname + ")[tmplen] = 0;\n" +\
                  "  env->ReleaseStringUTFChars(" + xname + ", tmpstr);\n" +\
                  "}\n"

        if indirectIn:
            return self.readJNIReference(extract, jniname)
        
        return extract

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        if indirectIn:
            iname = '*' + pcmname
        else:
            iname = pcmname

        xname = jniname
        if indirectOut:
            xname = 'tmpobj'

        if box:
            return xname + ' = static_cast<jobject>(env->NewStringUTF(' + iname + '));'
        extract = xname + " = env->NewStringUTF(" + iname + ");"

        if indirectOut:
            return self.writeJNIReference(extract, jniname)

        return extract

    def javaSig(self, direction):
        return {Type.IN: 'Ljava/lang/String;',
                Type.OUT: 'Lpjm/Reference;',
                Type.INOUT: 'Lpjm/Reference;',
                Type.RETURN: 'Ljava/lang/String;',
                Type.DERIVE: 'Ljava/lang/String;'
               }[direction]        

class WString(String):
    def __init__(self, type):
        String.__init__(self, type)

    def jniType(self, direction):
        return {Type.IN: 'jstring',
                Type.OUT: 'jobject',
                Type.INOUT: 'jobject',
                Type.RETURN: 'jstring',
                Type.DERIVE: 'jobject',
               }[direction]
    
    def pcmType(self, direction):
        return {Type.IN: 'const wchar_t*',
                Type.OUT: 'wchar_t**',
                Type.INOUT: 'wchar_t**',
                Type.RETURN: 'wchar_t*',
                Type.DERIVE: 'wchar_t*',
               }[direction]
    
    def pcmDestroy(self, name):
        return 'free(' + name + ');'

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        if indirectOut:
            oname = '*' + pcmname
        else:
            oname = pcmname

        if indirectIn:
            xname = 'tmpobj'
        elif unbox:
            xname = 'static_cast<jstring>(' + jniname + ')'
        else:
            xname = jniname
        
        extract = "{\n" +\
                  "  uint32_t tmplen = env->GetStringLength(" + xname + ");\n" +\
                  "  " + oname + " = (wchar_t*)malloc((tmplen + 1) * sizeof(wchar_t));\n" +\
                  "  const jchar* tmpstr = env->GetStringChars(" + xname + ", NULL);\n" +\
                  "  for (uint32_t tmpidx = 0; tmpidx < tmplen; tmpidx++)\n" +\
                  "    (" + oname + ")[tmpidx] = static_cast<wchar_t>(tmpstr[tmpidx]);\n" +\
                  "  (" + oname + ")[tmplen] = 0;\n" +\
                  "  env->ReleaseStringChars(" + xname + ", tmpstr);\n" +\
                  "}\n"
        if indirectIn:
            return self.readJNIReference(extract, jniname)
        
        return extract

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        if indirectIn:
            iname = '*' + pcmname
        else:
            iname = pcmname

        if indirectOut:
            xname = 'tmpobj'
        else:
            xname = jniname

        if box:
            return xname + ' = static_cast<jobject>(ConvertWcharStringToJString(env, ' + iname + '));'
        extract = xname + " = ConvertWcharStringToJString(env, " + iname + ");"

        if indirectOut:
            return self.writeJNIReference(extract, jniname)

        return extract

    def javaSig(self, direction):
        return {Type.IN: 'Ljava/lang/String;',
                Type.OUT: 'Lpjm/Reference;',
                Type.INOUT: 'Lpjm/Reference;',
                Type.RETURN: 'Ljava/lang/String;',
                Type.DERIVE: 'Ljava/lang/String;'
               }[direction]

class Sequence(Type):
    def __init__(self, type):
        self.seqType = GetTypeInformation(type.seqType().unalias())
        self.derive_name = self.seqType.javaType(Type.DERIVE)
        self.java_type = 'java.util.Vector<' + self.derive_name + '>'
        self.java_sig = 'Ljava/util/Vector;'

    def pcmType(self, direction):
        deriv = self.seqType.pcmType(Type.DERIVE)
        return {Type.IN: deriv + '*', # Should have 'const' but Iface* doesn't.
                Type.OUT: deriv + '**',
                Type.INOUT: deriv + '**',
                Type.RETURN: deriv + '*',
                Type.DERIVE: deriv + '*'
               }[direction]

    def needLength(self):
        return 1

    def pcmDestroy(self, name):
        return "{\nfor (uint32_t idx = 0; idx < _length_" + name + "; idx++) {\n" +\
               self.seqType.pcmDestroy(name + '[idx]') + "}\ndelete [] " + name + ";\n}"

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        if indirectOut:
            oname = '*' + pcmname
            olength = '*_length_' + pcmname
        else:
            oname = pcmname
            olength = '_length_' + pcmname

        if indirectIn:
            iname = 'tmpobj'
        else:
            iname = jniname

        code = "{\n" +\
               "  jclass vec = env->FindClass(\"java/util/Vector\");\n" +\
               "  jmethodID vecsize = env->GetMethodID(vec, \"size\", \"()I\");\n"+\
               "  jmethodID vecget = env->GetMethodID(vec, \"get\", \"(I)Ljava/lang/Object;\");\n"+\
               '  ' + olength + ' = env->CallIntMethod(' + iname + ", vecsize);\n"+\
               '  ' + oname + ' = new ' + self.seqType.pcmType(Type.DERIVE) + "[" + olength + "];\n"+\
               "  for (uint32_t tmpidx = 0; tmpidx < " + olength + "; tmpidx++)\n"+\
               "  {\n"+\
               "    jobject tmparrayobj = env->CallObjectMethod(" + iname + ", vecget, tmpidx);\n"+\
               self.seqType.convertToPCM('tmparrayobj', '(' + oname +\
                                         ")[tmpidx]", 0, 0, 1)+\
               "    env->DeleteLocalRef(tmparrayobj);\n"+\
               "  }\n" +\
               "}\n"

        if indirectIn:
            return self.readJNIReference(code, jniname)
        
        return code

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        if indirectIn:
            iname = '*' + pcmname
            ilength = '*_length_' + pcmname
        else:
            iname = pcmname
            ilength = '_length_' + pcmname

        if indirectIn:
            oname = 'tmpobj'
        else:
            oname = jniname

        code = "{\n" +\
               "  jclass vec = env->FindClass(\"java/util/Vector\");\n" +\
               "  jmethodID initmethod = env->GetMethodID(vec, \"<init>\", " +\
               "\"(I)V\");\n" +\
               "  " + oname + " = env->NewObject(vec, initmethod, " + ilength + ");\n" +\
               "  jmethodID vecadd = env->GetMethodID(vec, \"add\", \"(Ljava/lang/Object;)V\");"+\
               "  for (uint32_t tmpidx = 0; tmpidx < " + ilength + "; tmpidx++)\n"+\
               "  {\n"+\
               "    jobject tmparrayobj;\n"+\
               self.seqType.convertToJNI('tmparrayobj', '(' + iname +\
                                         ')[tmpidx]', 0, 0, 1)+\
               "    env->CallVoidMethod(" + oname + ", vecadd, tmparrayobj);\n"+\
               "    env->DeleteLocalRef(tmparrayobj);\n"+\
               "  }\n" +\
               "}\n"

        if indirectOut:
            return self.writeJNIReference(code, jniname)

        return code

class Declared(Type):
    def __init__(self, type, skipLast = 0):
        if type.scopedName() == ['XPCOM', 'IObject']:
            self.java_type = 'Object'
            self.java_sig = 'Ljava/lang/Object;'
        else:
            self.java_type = GetClassName(type)
            self.java_sig = 'L' + string.join(type.scopedName(), '/') + ';'
            
        self.cpp_type = 'iface::' + ScopedCppName(type.decl(), skipLast)

    def pcmType(self, direction):
        return {Type.IN: self.cpp_type + '*',
                Type.OUT: self.cpp_type + '**',
                Type.INOUT: self.cpp_type + '**',
                Type.RETURN: self.cpp_type + '*',
                Type.DERIVE: self.cpp_type + '*'
               }[direction]

class Struct(Declared):
    def __init__(self, type):
        Declared.__init__(self, type)

class Objref(Declared):
    def __init__(self, type):
        Declared.__init__(self, type)
        self.java_class_name = string.join(type.scopedName(), '/')
        self.p2j_type = 'p2j::' + string.join(type.scopedName(), '::')
        self.field_name = 'nativePtr_' + string.join(type.scopedName(), '_')
        self.wrap_name = 'wrap_' + string.join(type.scopedName(), '_')
        
    def pcmDestroy(self, name):
        return 'if (' + name + ') ' + name + '->release_ref();'

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        if indirectOut:
            oname = '*' + pcmname
        else:
            oname = pcmname

        return "if (" + jniname + " == NULL)\n" +\
               "  " + oname + " = NULL;\n" +\
               "else\n" +\
               "{\n" +\
               "  jclass outerclazz = env->GetObjectClass(" + jniname + ");\n" +\
               "  jfieldID nmid = env->GetFieldID(outerclazz, \"" + self.field_name + "\", \"J\");\n" +\
               "  if (nmid != NULL)\n" +\
               "  {\n" +\
               "    " + oname + " = reinterpret_cast<" + self.cpp_type +\
               "*>(env->GetLongField(" + jniname + ", nmid));\n" +\
               "    (" + oname + ")->add_ref();\n" +\
               "  }\n" +\
               "  else\n" +\
               "  {" +\
               "    env->ExceptionClear();\n" +\
               "    // It's a native Java object, so wrap it...\n" +\
               "    " + oname + " = new " + self.p2j_type + "(env, " + jniname + ");\n" +\
               "  }\n" +\
               "}\n"

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        if indirectIn:
            iname = '*' + pcmname
        else:
            iname = pcmname

        if indirectOut:
            xname = "tmpobj"
        else:
            xname = jniname

        code = xname + " = " + self.wrap_name + "(env, " + iname + ");"

        if indirectOut:
            return self.writeJNIReference(code, jniname)

        return code

class Enum(Declared):
    def __init__(self, type):
        Declared.__init__(self, type)
        self.java_class_name = string.join(type.scopedName(), '/')

    def pcmType(self, direction):
        return {Type.IN: self.cpp_type,
                Type.OUT: self.cpp_type + '*',
                Type.INOUT: self.cpp_type + '*',
                Type.RETURN: self.cpp_type,
                Type.DERIVE: self.cpp_type
               }[direction]

    def convertToPCM(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, unbox = 0):
        if indirectOut:
            oname = '*' + pcmname
        else:
            oname = pcmname
        return "{\n"+\
               "  jclass eclazz = env->FindClass(\"java/lang/Enum\");\n"+\
               "  jmethodID ordmeth = env->GetMethodID(eclazz, \"ordinal\", \"()I\");\n"+\
               "  " + oname + " = static_cast<" + self.pcmType(Type.DERIVE)+\
               ">(env->CallIntMethod(" + jniname + ", ordmeth));\n"+\
               "}\n"

    def convertToJNI(self, jniname, pcmname, indirectIn = 0, indirectOut = 0, box = 0):
        if indirectIn:
            iname = '*' + pcmname
        else:
            iname = pcmname

        if indirectOut:
            xname = 'tmpobj'
        else:
            xname = jniname
        
        code = "{\n"+\
               "  jclass eclazz = env->FindClass(\"" + self.java_class_name + "\");\n"+\
               "  jmethodID vmeth = env->GetStaticMethodID(eclazz, \"values\", \"()V\");\n"+\
               "  " + xname + " = env->GetObjectArrayElement(static_cast" +\
               "<jobjectArray>(env->CallStaticObjectMethod(eclazz, vmeth)), static_cast<jsize>("+\
               iname + "));\n" +\
               "}\n"
        
        if indirectOut:
            return self.writeJNIReference(code, jniname)
        return code

def GetClassName(iface):
    sn = iface.scopedName()
    return string.join(sn, '.')

def AccessorName(node, setter):
    ident = node.identifier()
    if setter:
        prefix = 'set'
    else:
        prefix = 'get'
    return prefix + string.upper(ident[0]) + ident[1:len(ident)]


Javakw = ['abstract', 'continue', 'for', 'new', 'switch',
          'assert', 'default', 'goto', 'package', 'synchronized',
          'boolean', 'do', 'if', 'private', 'this',
          'break', 'double', 'implements', 'protected', 'throw',
          'byte', 'else', 'import', 'public', 'throws',
          'case', 'enum', 'instanceof', 'return', 'transient',
          'catch', 'extends', 'int', 'short', 'try',
          'char', 'final', 'interface', 'static', 'void',
          'class', 'finally', 'long', 'strictfp', 'volatile',
          'const', 'float', 'native', 'super', 'while']

def JavaName(node):
    ident = node.identifier()
    if ident in Javakw:
        return '_java_' + ident
    return ident

Cppkw = [
    "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
    "case", "catch", "char", "class", "compl", "const", "const_cast",
    "continue", "default", "delete", "do", "double", "dynamic_cast",
    "else", "enum", "explicit", "export", "extern", "false", "float",
    "for", "friend", "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "not", "not_eq", "operator", "or", "or_eq",
    "private", "protected", "public", "register", "reinterpret_cast",
    "return", "short", "signed", "sizeof", "static", "static_cast",
    "struct", "switch", "template", "this", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq" ]

def CppName(name):
    """ Escape a string to avoid a reserved word. Used for all mappings. """
    if name in Cppkw:
        return "_cxx_" + name
    return name

def JavaMangleName(name):
    return string.replace(string.replace(name, '_', '_1'), '/', '_')

def ScopedCppName(node, skiplastscope=0):
    sn = map(CppName, node.scopedName())
    simple = sn[-1]
    if skiplastscope:
        sn = sn[:-2]
        sn.append(simple)
    return string.join(sn, '::')
