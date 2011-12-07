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
        return "ObjRef<iface::CGRS::GenericsService> cgs = CreateGenericsService();\n" +\
               '%scgs->getTypeByName("%s");' % (doWhat, self.typename)
    def makeStorage(self, name):
        return '%s %s;' % (self.cppType, name)
    def destroyStorage(self, name):
        return ''
    def returnStorage(self, name):
        return 'return %s;' % name
    def defaultStorageValue(self, name):
        return '%s = %s;' % (name, self.defaultCxxValue)
    def convertGenericToNative(self, genName, natName):
        return 'DECLARE_QUERY_INTERFACE_OBJREF(%s_specific, %s, %s); %s = %s_specific->%s();' % (genName, genName, self.genericIface,
                                                                                                 natName, genName, self.nativeGetter)
    def convertNativeToGeneric(self, natName, genName):
        return '%s = cgs->%s(%s);' % (genName, self.genericCreationCall, natName)

BASE_MAP = {
    idltype.tk_short: {'typename': 'short', 'cppType': 'int16_t', 'defaultCxxValue': '0', 'nativeGetter': 'asShort',
                       'genericCreationCall': 'makeShort', 'genericIface': 'CGRS::ShortValue'},
    idltype.tk_long: {'typename': 'long', 'cppType': 'int32_t', 'defaultCxxValue': '0', 'nativeGetter': 'asLong',
                       'genericCreationCall': 'makeLong', 'genericIface': 'CGRS::LongValue'},
    idltype.tk_ushort: {'typename': 'unsigned short', 'cppType': 'uint16_t', 'defaultCxxValue': '0', 'nativeGetter': 'asUShort',
                       'genericCreationCall': 'makeUShort', 'genericIface': 'CGRS::UShortValue'},
    idltype.tk_ulong: {'typename': 'unsigned long', 'cppType': 'uint32_t', 'defaultCxxValue': '0', 'nativeGetter': 'asULong',
                       'genericCreationCall': 'makeULong', 'genericIface': 'CGRS::ULongValue'},
    idltype.tk_float: {'typename': 'float', 'cppType': 'float', 'defaultCxxValue': '0.0', 'nativeGetter': 'asFloat',
                       'genericCreationCall': 'makeFloat', 'genericIface': 'CGRS::FloatValue'},
    idltype.tk_double: {'typename': 'double', 'cppType': 'double', 'defaultCxxValue': '0.0', 'nativeGetter': 'asDouble',
                       'genericCreationCall': 'makeDouble', 'genericIface': 'CGRS::DoubleValue'},
    idltype.tk_boolean: {'typename': 'boolean', 'cppType': 'bool', 'defaultCxxValue': 'false', 'nativeGetter': 'asBoolean',
                       'genericCreationCall': 'makeBoolean', 'genericIface': 'CGRS::BooleanValue'},
    idltype.tk_char: {'typename': 'char', 'cppType': 'char', 'defaultCxxValue': '0', 'nativeGetter': 'asChar',
                       'genericCreationCall': 'makeChar', 'genericIface': 'CGRS::CharValue'},
    idltype.tk_octet: {'typename': 'octet', 'cppType': 'uint8_t', 'defaultCxxValue': '0', 'nativeGetter': 'asOctet',
                       'genericCreationCall': 'makeOctet', 'genericIface': 'CGRS::OctetValue'},
    idltype.tk_ulonglong: {'typename': 'ulonglong', 'cppType': 'uint64_t', 'defaultCxxValue': '0', 'nativeGetter': 'asULongLong',
                       'genericCreationCall': 'makeULongLong', 'genericIface': 'CGRS::ULongLongValue'},
    idltype.tk_void: {'typename': 'void', 'cppType': 'void'}
}

class VoidType(Type):
    def __init__(self):
        self.typename = 'void'
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

class Base(Type):
    def __init__(self, type):
        k = type.kind()
        info = BASE_MAP[k]
        self.__dict__.update(info)

class String(Type):
    def __init__(self, type):
        self.typename = 'string'
        self.cppType = 'std::string'
        self.defaultCxxValue = '""'
        self.nativeGetter = 'asString'
        self.genericIface = 'StringValue'
        self.genericCreationCall = 'makeString'
class WString(Type):
    def __init__(self, type):
        self.typename = 'wstring'
        self.cppType = 'std::wstring'
        self.defaultCxxValue = 'L""'
        self.nativeGetter = 'asWString'
        self.genericIface = 'WStringValue'
        self.genericCreationCall = 'makeWString'

class Sequence(Type):
    def __init__(self, type, context):
        bt = type.seqType().unalias()
        self.st = GetTypeInformation(bt)
        self.typename = 'sequence<' + self.st.typename + '>'
        self.cppType = 'std::vector<' + self.st.cppType + '>'

    def defaultStorageValue(self, name):
        return '%s.clear();' % name

    def fetchType(self, doWhat = 'return '):
        return "ObjRef<CDA_GenericsService> cgs = CreateGenericsService();\n" +\
               "ObjRef<iface::CGRS::GenericType> innerType%d;\n" % len(self.typename) +\
               "{" + self.st.fetchType('innerType%d = ' % len(self.typename)) + "}\n" +\
               'return cgs->makeSequenceType(innerType%d);' % len(self.typename)

    def convertGenericToNative(self, genName, natName):
        return "%s.clear();\n" +\
               "for (size_t %s_seqi = 0; %s_seqi < %s->valueCount(); %s_seqi++)\n" % (genName, genName, genName, genName) +\
               "{\n" +\
               "  ObjRef<GenericValue> %s_sval = %s->getValueByIndex(%s_seqi);\n" % (genName, genName, genName) +\
               "  " + self.st.makeStorage('%s_nval' % genName) + "\n" +\
               "  %s.push_back(%s_nval);\n" % (natName, genName) +\
               self.st.convertGenericToNative("%s_sval" % genName, "%s_nval" % genName) +\
               "}"
    def convertNativeToGeneric(self, natName, genName):
        return "ObjRef<CDA_GenericsService> cgs = CreateGenericsService();\n" +\
               "ObjRef<iface::CGRS::GenericType> innerType%d;\n" % len(self.typename) +\
               "{" + self.st.fetchType('innerType%d = ' % len(self.typename)) + "}\n" +\
               "%s = cgs->makeSequence(innerType%d);\n" % (genName, len(self.typename)) +\
               "for (std::vector<%s>::iterator %s_i = %s.begin(); %s_i != %s.end(); %s_i++)\n" % (natName, natName, natName, natName, natName, natName) +\
               "{\n" +\
               "ObjRef<iface::CGRS::GenericValue> %s_gval;\n" % genName +\
               self.st.convertNativeToGeneric("(*%s_i)" % natName, "%s_gval" % genName) +\
               "%s.appendValue(%s_gval);\n" % (genName, genName) +\
               "}\n"

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
        self.cppType = type.simplecxxscoped
        self.rawIface = type.corbacxxscoped
        self.className = type.simplename
        self.defaultCxxValue = 'NULL'
        self.genericCreationCall = 'makeObject'
    def convertGenericToNative(self, genName, natName):
        # Note: this line leaks void objrefs, but it doesn't matter since it is a singleton.
        return "if (!CDA_objcmp(%s, cgs->makeVoid())) %s = NULL;\n" % (genName, natName) +\
               "else {\n" +\
               "  DECLARE_QUERY_INTERFACE_OBJREF(%s_qi, %s, CGRS::ObjectValue);\n" % (genName, genName) +\
               "  if (%s_qi == NULL)\n" % genName +\
               "  {\n" +\
               "    DECLARE_QUERY_INTERFACE_OBJREF(%s_cb, %s, CGRS::CallbackObjectValue);\n" % (genName, genName) +\
               "    if (%s_cb != NULL) %s = new Callback_%s(%s_cb);\n" % (genName, natName, self.className, genName) +\
               "    else throw iface::CGRS::CGRSError();\n" +\
               "  }\n" +\
               "  ObjRef<iface::XPCOM::IObject> %s_tmp = %s_qi->asObject();\n" % (natName, genName) +\
               "  QUERY_INTERFACE(%s, %s_tmp, %s);\n" % (natName, natName, self.rawIface) +\
               "}"
    def convertNativeToGeneric(self, natName, genName):
        return "if (!CDA_objcmp(%s, cgs->makeVoid())) %s = NULL;\n" % (genName, natName) +\
               "else {\n" +\
               "Callback_%s* %s_cb = dynamic_cast<Callback_%s*>(static_cast<%s>(%s));\n" % (self.className, natName, self.className, self.typename, natName) +\
               "if (%s_cb != NULL) %s = %s_cb->unwrap();\n" % (natName, genName, natName) +\
               'else %s = cgs->%s(%s);' % (genName, self.genericCreationCall, natName) +\
               '}'

class Enum(Declared, Base):
    def __init__(self, type):
        AnnotateByRepoID(type)
        self.typename = type.simplecxxscoped
        self.cppType = type.simplecxxscoped
        self.defaultCxxValue = type.decl().enumerators()[0].simplecxxscoped
    def convertGenericToNative(self, genName, natName):
        return '%s = reinterpret_cast<%s>(%s->asLong());' % (natName, self.typename, genName)
    def convertNativeToGeneric(self, natName, genName):
        return "ObjRef<iface::CGRS::GenericType> %s_type = cgs->getTypeByName(\"%s\");\n" % (genName, self.typename) +\
               "DECLARE_QUERY_INTERFACE_OBJREF(%s_etype, %s_type, iface::CGRS::EnumType);\n" % (genName, genName) +\
               '%s = cgs->makeEnumFromIndex(%s_etype, %s);' % (genName, genName, natName)
