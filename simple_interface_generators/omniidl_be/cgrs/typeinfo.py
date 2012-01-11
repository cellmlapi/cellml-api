from omniidl import idltype
import string
import os.path
from identifier import AnnotateByRepoID

def GetTypeInformation(type, context = None):
    if (type == None):
        return VoidType()
    type = type.unalias()
    if isinstance(type, idltype.Base):
        if type.kind() == idltype.tk_void:
            return VoidType()
        else:
            return Base(type)
    elif isinstance(type, idltype.String):
        return String(type)
    elif isinstance(type, idltype.WString):
        return WString(type)
    elif isinstance(type, idltype.Sequence):
        return Sequence(type, context)
    elif isinstance(type, idltype.Declared):
        if type.kind() == idltype.tk_struct:
            return Struct(type, context)
        elif type.kind() == idltype.tk_objref:
            return Objref(type, context)
        elif type.kind() == idltype.tk_enum:
            return Enum(type)
        else:
            return Declared(type, context)

class Type:
    def fetchType(self, doWhat = 'return '):

        return '%scgs->getTypeByName("%s");' % (doWhat, self.typename)
    def makeStorage(self, name):
        return '%s %s;' % (self.cppType, name)
    def makeScopedDestructor(self, name):
        return 'scoped_destroy<%s > %sScopedDestroy(%s, %s);' % (self.cppType, name, name, self.destructorValue())

    def returnStorage(self, name):
        return 'return %s;' % name
    def defaultStorageValue(self, name):
        return '%sScopedDestroy.manual_destroy(); %s = %s;' % (name, name, self.defaultCxxValue)
    def convertGenericToNative(self, genName, natName):
        return '{ DECLARE_QUERY_INTERFACE_OBJREF(tmp_specific, %s, %s); %s = tmp_specific->%s(); }' % (genName, self.genericIface,
                                                                                                       natName, self.nativeGetter)
    def convertNativeToGeneric(self, natName, genName):
        return '%s = cgs->%s(%s);' % (genName, self.genericCreationCall, natName)

    def deref(self, isOut):
        if isOut == 0:
            return ''
        return ('*' if self.cppOutSignatureType == self.cppInSignatureType + '*' else '')

    def ref(self, isOut):
        if isOut == 0:
            return ''
        return ('&' if self.cppOutSignatureType == self.cppInSignatureType + '*' else '')

    def destructorValue(self):
        return 'new void_destructor<%s>()' % self.cppType

BASE_MAP = {
    idltype.tk_short: {'typename': 'short', 'cppType': 'int16_t', 'defaultCxxValue': '0', 'nativeGetter': 'asShort',
                       'genericCreationCall': 'makeShort', 'genericIface': 'CGRS::ShortValue',
                       'cppReturnSignatureType': 'int16_t', 'cppInSignatureType': 'int16_t', 'cppOutSignatureType': 'int16_t*'},
    idltype.tk_long: {'typename': 'long', 'cppType': 'int32_t', 'defaultCxxValue': '0', 'nativeGetter': 'asLong',
                      'genericCreationCall': 'makeLong', 'genericIface': 'CGRS::LongValue',
                      'cppReturnSignatureType': 'int32_t', 'cppInSignatureType': 'int32_t', 'cppOutSignatureType': 'int32_t*'},
    idltype.tk_longlong: {'typename': 'long long', 'cppType': 'int64_t', 'defaultCxxValue': '0', 'nativeGetter': 'asLongLong',
                          'genericCreationCall': 'makeLongLong', 'genericIface': 'CGRS::LongLongValue',
                          'cppReturnSignatureType': 'int64_t', 'cppInSignatureType': 'int64_t', 'cppOutSignatureType': 'int64_t*'},
    idltype.tk_ushort: {'typename': 'unsigned short', 'cppType': 'uint16_t', 'defaultCxxValue': '0', 'nativeGetter': 'asUShort',
                        'genericCreationCall': 'makeUShort', 'genericIface': 'CGRS::UShortValue',
                        'cppReturnSignatureType': 'uint16_t', 'cppInSignatureType': 'uint16_t', 'cppOutSignatureType': 'uint16_t*'},
    idltype.tk_ulong: {'typename': 'unsigned long', 'cppType': 'uint32_t', 'defaultCxxValue': '0', 'nativeGetter': 'asULong',
                       'genericCreationCall': 'makeULong', 'genericIface': 'CGRS::ULongValue',
                       'cppReturnSignatureType': 'uint32_t', 'cppInSignatureType': 'uint32_t', 'cppOutSignatureType': 'uint32_t*'},
    idltype.tk_float: {'typename': 'float', 'cppType': 'float', 'defaultCxxValue': '0.0', 'nativeGetter': 'asFloat',
                       'genericCreationCall': 'makeFloat', 'genericIface': 'CGRS::FloatValue',
                       'cppReturnSignatureType': 'float', 'cppInSignatureType': 'float', 'cppOutSignatureType': 'float*'},
    idltype.tk_double: {'typename': 'double', 'cppType': 'double', 'defaultCxxValue': '0.0', 'nativeGetter': 'asDouble',
                        'genericCreationCall': 'makeDouble', 'genericIface': 'CGRS::DoubleValue',
                        'cppReturnSignatureType': 'double', 'cppInSignatureType': 'double', 'cppOutSignatureType': 'double*'},
    idltype.tk_boolean: {'typename': 'boolean', 'cppType': 'bool', 'defaultCxxValue': 'false', 'nativeGetter': 'asBoolean',
                         'genericCreationCall': 'makeBoolean', 'genericIface': 'CGRS::BooleanValue',
                         'cppReturnSignatureType': 'bool', 'cppInSignatureType': 'bool', 'cppOutSignatureType': 'bool*'},
    idltype.tk_char: {'typename': 'char', 'cppType': 'char', 'defaultCxxValue': '0', 'nativeGetter': 'asChar',
                      'genericCreationCall': 'makeChar', 'genericIface': 'CGRS::CharValue',
                      'cppReturnSignatureType': 'char', 'cppInSignatureType': 'char', 'cppOutSignatureType': 'char*'},
    idltype.tk_octet: {'typename': 'octet', 'cppType': 'uint8_t', 'defaultCxxValue': '0', 'nativeGetter': 'asOctet',
                       'genericCreationCall': 'makeOctet', 'genericIface': 'CGRS::OctetValue',
                       'cppReturnSignatureType': 'uint8_t', 'cppInSignatureType': 'uint8_t', 'cppOutSignatureType': 'uint8_t*'},
    idltype.tk_ulonglong: {'typename': 'unsigned long long', 'cppType': 'uint64_t', 'defaultCxxValue': '0', 'nativeGetter': 'asULongLong',
                           'genericCreationCall': 'makeULongLong', 'genericIface': 'CGRS::ULongLongValue',
                           'cppReturnSignatureType': 'uint64_t', 'cppInSignatureType': 'uint64_t', 'cppOutSignatureType': 'uint64_t*'},
    idltype.tk_void: {'typename': 'void', 'cppType': 'void', 'cppReturnSignatureType': 'void'}
}

class VoidType(Type):
    def __init__(self):
        self.typename = 'void'
        self.cppReturnSignatureType = 'void'
    def makeStorage(self, name):
        return ''
    def returnStorage(self, name):
        return ''
    def defaultStorageValue(self, name):
        return ''
    def convertGenericToNative(self, genName, natName):
        return ''
    def convertNativeToGeneric(self, natName, genName):
        return ''
    def makeScopedDestructor(self, name):
        return ''

class Base(Type):
    def __init__(self, type):
        k = type.kind()
        info = BASE_MAP[k]
        self.__dict__.update(info)

class String(Type):
    def __init__(self, type):
        self.typename = 'string'
        self.cppType = 'std::string'
        self.cppReturnSignatureType = 'std::string'
        self.cppInSignatureType = 'const std::string&'
        self.cppOutSignatureType = 'std::string&'
        self.defaultCxxValue = '""'
        self.nativeGetter = 'asString'
        self.genericIface = 'CGRS::StringValue'
        self.genericCreationCall = 'makeString'
class WString(Type):
    def __init__(self, type):
        self.typename = 'wstring'
        self.cppType = 'std::wstring'
        self.cppReturnSignatureType = 'std::wstring'
        self.cppInSignatureType = 'const std::wstring&'
        self.cppOutSignatureType = 'std::wstring&'
        self.defaultCxxValue = 'L""'
        self.nativeGetter = 'asWString'
        self.genericIface = 'CGRS::WStringValue'
        self.genericCreationCall = 'makeWString'

class Sequence(Type):
    def __init__(self, type, context):
        bt = type.seqType().unalias()
        self.st = GetTypeInformation(bt)
        self.typename = 'sequence<' + self.st.typename + '>'
        self.cppType = 'std::vector<' + self.st.cppType + '>'
        self.genericIface = 'CGRS::SequenceValue'
        self.cppReturnSignatureType = 'std::vector<' + self.st.cppType + '>'
        self.cppInSignatureType = 'const std::vector<' + self.st.cppType + '>&'
        self.cppOutSignatureType = 'std::vector<' + self.st.cppType + '>&'

    def defaultStorageValue(self, name):
        return '%s.clear();' % name

    def fetchType(self, doWhat = 'return '):
        return "ObjRef<iface::CGRS::GenericType> innerType%d;\n" % len(self.typename) +\
               "{" + self.st.fetchType('innerType%d = ' % len(self.typename)) + "}\n" +\
               'return cgs->makeSequenceType(innerType%d);' % len(self.typename)

    def convertGenericToNative(self, genName, natName):
        return "%s.clear();\n" % natName +\
               "for (size_t %s_seqi = 0; %s_seqi < %s->valueCount(); %s_seqi++)\n" % (genName, genName, genName, genName) +\
               "{\n" +\
               "  ObjRef<iface::CGRS::GenericValue> %s_sval = %s->getValueByIndex(%s_seqi);\n" % (genName, genName, genName) +\
               "  " + self.st.makeStorage('%s_nval' % genName) + "\n" +\
               "  %s.push_back(%s_nval);\n" % (natName, genName) +\
               self.st.convertGenericToNative("%s_sval" % genName, "%s_nval" % genName) +\
               "}"
    def convertNativeToGeneric(self, natName, genName):
        return "ObjRef<iface::CGRS::GenericType> innerType%d;\n" % len(self.typename) +\
               "{" + self.st.fetchType('innerType%d = ' % len(self.typename)) + "}\n" +\
               "%s = cgs->makeSequence(innerType%d);\n" % (genName, len(self.typename)) +\
               "for (%s::const_iterator %s_i = %s.begin(); %s_i != %s.end(); %s_i++)\n" % (self.cppType, natName, natName, natName, natName, natName) +\
               "{\n" +\
               "ObjRef<iface::CGRS::GenericValue> %s_gval;\n" % genName +\
               self.st.makeStorage('%s_iv' % natName) + "\n" +\
               "%s_iv = *%s_i;\n" % (natName, natName) +\
               self.st.convertNativeToGeneric("%s_iv" % natName, "%s_gval" % genName) +\
               "%s->appendValue(%s_gval);\n" % (genName, genName) +\
               "}\n"

    def destructorValue(self):
        return 'new container_destructor<%s >(%s)' % (self.cppType, self.st.destructorValue())

class Declared(Type):
    def __init__(self, type, context):
        pass

class Struct(Declared):
    def __init__(self, type, context):
        raise "Structs are not currently supported"

class Objref(Declared):
    def __init__(self, type, context):
        AnnotateByRepoID(type)
        self.typename = type.simplecxxscoped
        self.cppType = type.simplecxxscoped + '*'
        self.rawIface = type.corbacxxscoped
        # It could be either a callback or a normal wrapper, so...
        self.genericIface = 'CGRS::GenericValue'
        self.className = type.simplename
        self.defaultCxxValue = 'NULL'
        self.genericCreationCall = 'makeObject'
        self.cppReturnSignatureType = 'already_AddRefd<' + self.typename + '>'
        self.cppInSignatureType = self.typename + '*'
        self.cppOutSignatureType = self.typename + '**'

    def convertGenericToNative(self, genName, natName):
        # Note: this line leaks void objrefs, but it doesn't matter since it is a singleton.
        return "if (!CDA_objcmp(%s, cgs->makeVoid())) %s = NULL;\n" % (genName, natName) +\
               "else {\n" +\
               "  DECLARE_QUERY_INTERFACE_OBJREF(%s_qi, %s, CGRS::ObjectValue);\n" % (genName, genName) +\
               "  if (%s_qi == NULL)\n" % genName +\
               "  {\n" +\
               "    DECLARE_QUERY_INTERFACE_OBJREF(%s_cb, %s, CGRS::CallbackObjectValue);\n" % (genName, genName) +\
               "    if (%s_cb == NULL) throw iface::CGRS::CGRSError();\n" % genName +\
               "    RETURN_INTO_OBJREF(gi, iface::CGRS::GenericInterface, cgs->getInterfaceByName(\"%s\"));\n" % self.rawIface +\
               "    %s = reinterpret_cast<%s>(static_cast<CDA_GenericInterfaceBase*>(" % (natName, self.cppType) +\
               "static_cast<iface::CGRS::GenericInterface*>(gi))->makeCallbackProxy(%s_cb));\n" % genName +\
               "    if (%s == NULL) return cgs->makeVoid();\n" % natName +\
               "  }\n" +\
               "  ObjRef<iface::XPCOM::IObject> %s_tmp = %s_qi->asObject();\n" % (natName, genName) +\
               "  QUERY_INTERFACE(%s, %s_tmp, %s);\n" % (natName, natName, self.rawIface) +\
               "}"
    def convertNativeToGeneric(self, natName, genName):
        return "if (!CDA_objcmp(%s, cgs->makeVoid())) %s = NULL;\n" % (genName, natName) +\
               "else {\n" +\
               "CGRSCallback* %s_cb = dynamic_cast<CGRSCallback*>(static_cast<%s*>(%s));\n" % (natName, self.typename, natName) +\
               "if (%s_cb != NULL) %s = %s_cb->unwrap();\n" % (natName, genName, natName) +\
               'else %s = cgs->%s(%s);' % (genName, self.genericCreationCall, natName) +\
               '}'

    def destructorValue(self):
        return 'new objref_destructor<%s>()' % (self.typename)

class Enum(Declared, Base):
    def __init__(self, type):
        AnnotateByRepoID(type)
        self.typename = type.simplecxxscoped
        self.cppType = type.simplecxxscoped
        self.cppReturnSignatureType = self.typename
        self.cppInSignatureType = self.typename
        self.genericIface = 'CGRS::EnumValue'
        self.cppOutSignatureType = self.typename + '**'
        self.defaultCxxValue = type.decl().enumerators()[0].simplecxxscoped
    def convertGenericToNative(self, genName, natName):
        return '%s = static_cast<%s>(%s->asLong());' % (natName, self.typename, genName)
    def convertNativeToGeneric(self, natName, genName):
        return "ObjRef<iface::CGRS::GenericType> %s_type = cgs->getTypeByName(\"%s\");\n" % (genName, self.typename) +\
               "DECLARE_QUERY_INTERFACE_OBJREF(%s_etype, %s_type, CGRS::EnumType);\n" % (genName, genName) +\
               '%s = cgs->makeEnumFromIndex(%s_etype, %s);' % (genName, genName, natName)
