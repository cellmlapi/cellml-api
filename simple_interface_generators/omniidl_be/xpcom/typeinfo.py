from omniidl import idltype
import nameutils, string

def GetTypeInformation(type):
    if isinstance(type, idltype.Base):
        kind = type.kind()
        #if ((kind == idltype.tk_boolean) or
        #    (kind == idltype.tk_long) or (kind == idltype.tk_ulong)):
        #    return Base_Convert(type)
        #else:
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

lastTmpName = 0

class XPCOMLocalStorage:
    def __init__(self, typename, name=None):
        global lastTmpName
        vname = name
        if vname == None:
            x = lastTmpName
            lastTmpName = lastTmpName + 1
            vname = '_var' + str(x)
        self.typename = typename
        self.name = vname
    def GetStorageDefinition(self, initialValue=None):
        if initialValue == None:
            return self.typename + ' ' + self.name + ';'
        else:
            return self.typename + ' ' + self.name + ' = ' + initialValue + ';'
    def GetAssignmentLValue(self):
        return self.name
    def GetAssignmentRValue(self):
        return '&' + self.name
    def GetInArgument(self):
        return self.name
    def GetInOutArgument(self):
        return '&' + self.name
    def GetDestroyer(self):
        return ''
    def GetAssignOut(self, varname):
        return '*' + varname + ' = ' + self.name + ';'

class COMPtrLocalStorage(XPCOMLocalStorage):
    def __init__(self, typename, name=None):
        XPCOMLocalStorage.__init__(self, typename, name)
    
    def GetStorageDefinition(self, initialValue=None):
        if initialValue == None:
            return 'nsCOMPtr<' + self.typename + '> ' + self.name + ';'
        else:
            return 'nsCOMPtr<' + self.typename + '> ' + self.name + '(' +\
                   initialValue + ');'
    def GetAssignmentRValue(self):
        return 'getter_AddRefs(' + self.name + ')'
    def GetInOutArgument(self):
        return self.name
    def GetAssignOut(self, varname):
        return '*' + varname + ' = ' + self.name + '; NS_IF_ADDREF(*' + varname +\
               ');'

class XPCOMStringLocalStorage(XPCOMLocalStorage):
    def __init__(self, typename, name=None):
        XPCOMLocalStorage.__init__(self, typename, name)
    def GetAssignmentRValue(self):
        return 'getter_Copies(' + self.name + ')'
    def GetAssignOut(self, varname):
        return varname + '.Assign(' + self.name + ');'
    def GetInOutArgument(self):
        return self.name
    def GetAssignmentRValue(self):
        return self.name

class XPCOMArrayLocalStorage(XPCOMLocalStorage):
    def __init__(self, atype, name=None):
        XPCOMLocalStorage.__init__(self, atype.type_xpcom_seq, name)
        self.atype = atype
        self.lenname = self.name + '_len'
    def GetStorageDefinition(self):
        return 'uint32_t ' + self.lenname + ' = 0; ' +\
               self.typename + '* ' + self.name + ' = nsnull;'
    def GetAssignmentRValue(self):
        return '&' + self.lenname + ', &' + self.name
    def GetDestroyer(self):
        return "{\n  uint32_t _idx;\n  for (_idx = 0; _idx < " +\
               self.lenname + "; _idx++)\n" +\
               "  {\n" + self.GetDestroyIndex('_idx') + "  }\n" +\
               'if (' + self.name + ' != nsnull) nsMemory::Free(' + self.name + ");\n}"
    def GetDestroyIndex(self, idx):
        return self.atype.DestroyXPCOMArrayMember(self.name + '[' + idx + ']')
    def GetAssignOut(self, varname):
        return varname + '_len' + ' = ' + self.lenname + ';'
    def GetInOutArgument(self):
        return '&' + self.lenname + ', &' + self.name
    def GetInArgument(self):
        return self.lenname + ', ' + self.name

class PCMLocalStorage:
    def __init__(self, typename, name=None):
        global lastTmpName
        vname = name
        if vname == None:
            x = lastTmpName
            lastTmpName = lastTmpName + 1
            vname = '_var' + str(x)
        self.name = vname
        self.typename = typename
    def GetStorageDefinition(self, initialValue=None):
        if initialValue == None:
            return self.typename + ' ' + self.name + ';'
        else:
            return self.typename + ' ' + self.name + ' = ' + initialValue + ';'
    def GetInArgument(self):
        return self.name
    def GetAssignmentLValue(self):
        return self.name
    def GetAssignmentRValue(self):
        return '&' + self.name
    def GetDestroyer(self):
        return ''
    def GetAssignOut(self, varname):
        return '*' + varname + ' = ' + self.name + ';'
        self.name = vname
    def GetReturn(self):
        return 'return ' + self.name + ';'

class PCMObjRefLocalStorage(PCMLocalStorage):
    def __init__(self, typename, name=None):
        PCMLocalStorage.__init__(self, typename, name)
    def GetDestroyer(self):
        return 'if (' + self.name + " != NULL)\n  " + self.name +\
               '->release_ref();'

class PCMStringLocalStorage(PCMLocalStorage):
    def __init__(self, typename, name=None):
        PCMLocalStorage.__init__(self, typename, name)
    def GetDestroyer(self):
        return 'free(' + self.name + ');'

class PCMArrayLocalStorage(PCMLocalStorage):
    def __init__(self, atype, name=None):
        PCMLocalStorage.__init__(self, atype.type_pcm, name)
        self.atype = atype
        self.lenname = self.name + '_len'
    def GetStorageDefinition(self):
        return 'uint32_t ' + self.lenname + ' = 0; ' +\
               self.typename + '* ' + self.name + ' = nsnull;'
    def GetInArgument(self):
        return self.lenname + ', ' + self.name
    def GetDestroyer(self):
        return "{\n  uint32_t _idx;\n  for (_idx = 0; _idx < " +\
               self.lenname + "; _idx++)\n" +\
               "  {\n" + self.GetDestroyIndex('_idx') + "  }\n" +\
               'if (' + self.name + ' != nsnull) delete [] ' + self.name +\
               ";\n}"
    def GetDestroyIndex(self, idx):
        return self.atype.DestroyPCMArrayMember(self.name + '[' + idx + ']')
        
class Type:
    def XPIDLArgument(self, ident, is_in, is_out):
        if is_in:
            if is_out:
                direct = 'inout '
            else:
                direct = 'in '
        else:
            direct = 'out '
        return direct + self.type_xpidl + ' ' + ident
    def PCMArgument(self, ident, is_in, is_out):
        if is_out:
            return self.type_pcm + '* ' + ident
        else:
            return self.type_pcm_const + ' ' + ident
    def XPCOMArgument(self, ident, is_in, is_out):
        if is_out:
            return self.type_xpcom + '* ' + ident
        else:
            return self.type_xpcom + ' ' + ident
    def GetXPCOMStorage(self):
        return XPCOMLocalStorage(self.type_xpcom)
    def GetPCMStorage(self):
        return PCMLocalStorage(self.type_pcm)
    def DestroyXPCOMArrayMember(self, name):
        return ';'
    def DestroyPCMArrayMember(self, name):
        return ';'

BASE_MAP = {
    idltype.tk_short: {'type_xpcom': 'PRInt16', 'type_xpidl': 'short',
                       'type_pcm': 'int16_t'},
    idltype.tk_long: {'type_xpcom': 'PRInt32', 'type_xpidl': 'long',
                      'type_pcm': 'int32_t'},
    idltype.tk_ushort: {'type_xpcom': 'PRUint16',
                        'type_xpidl': 'unsigned short', 'type_pcm': 'uint16_t'},
    idltype.tk_ulong: {'type_xpcom': 'PRUint32', 'type_xpidl': 'unsigned long',
                       'type_pcm': 'uint32_t'},
    idltype.tk_float: {'type_xpcom': 'float', 'type_xpidl': 'float',
                       'type_pcm': 'float'},
    idltype.tk_double: {'type_xpcom': 'double', 'type_xpidl': 'double',
                        'type_pcm': 'double'},
    idltype.tk_boolean: {'type_xpcom': 'PRBool', 'type_xpidl': 'boolean',
                         'type_pcm': 'bool'},
    idltype.tk_char: {'type_xpcom': 'char', 'type_xpidl': 'char',
                      'type_pcm': 'char'},
    idltype.tk_octet: {'type_xpcom': 'PRUint8', 'type_xpidl': 'octet',
                       'type_pcm': 'uint8_t'},
    idltype.tk_ulonglong: {'type_xpcom': 'PRUint64',
                           'type_xpidl': 'unsigned long long',
                           'type_pcm': 'uint64_t'},
}

class Base(Type):
    def __init__(self, type):
        k = type.kind()
        info = BASE_MAP[k]
        if info == None:
            info = {'type_xpcom': '/* unknown*/',
                    'type_xpidl': '/* unknown */',
                    'type_pcm': '/* unknown */'}
        self.__dict__.update(info)
        self.type_pcm_const = self.type_pcm
        self.type_xpcom_seq = self.type_xpcom
    
    def ConvertXPCOMToPCM(self, source, dest):
        return dest + ' = ' + source + ';'
    def ConvertPCMToXPCOM(self, source, dest):
        return dest + ' = ' + source + ';'

class String(Type):
    def __init__(self, type):
        self.type_xpcom = 'nsACString'
        self.type_xpidl = 'ACString'
        self.type_pcm = 'char*'
        self.type_pcm_const = 'const ' + self.type_pcm
        self.type_xpcom_seq = 'char*'
    
    def GetXPCOMStorage(self):
        return XPCOMStringLocalStorage('nsCString')

    def GetPCMStorage(self):
        return PCMStringLocalStorage(self.type_pcm)
    
    def DestroyXPCOMArrayMember(self, name):
        return 'nsMemory::Free(' + name + ');'

    def DestroyPCMArrayMember(self, name):
        return 'free(' + name + ');'
    
    def ConvertXPCOMToPCM(self, source, dest):
        return dest + ' = strdup(' + source +\
               '.BeginReading());'

    def ConvertPCMToXPCOM(self, source, dest):
        return dest + ' = ' + source + ';'

    def XPCOMArgument(self, ident, is_in, is_out):
        if not is_out:
            return 'const nsACString& ' + ident
        return 'nsACString& ' + ident

class WString(Type):
    def __init__(self, type):
        self.type_xpcom = 'nsAString'
        self.type_xpidl = 'AString'
        self.type_pcm = 'wchar_t*'
        self.type_pcm_const = 'const ' + self.type_pcm
        self.type_xpcom_seq = 'PRUnichar*'
    
    def GetXPCOMStorage(self):
        return XPCOMStringLocalStorage('nsString')

    def GetPCMStorage(self):
        return PCMStringLocalStorage(self.type_pcm)

    def DestroyXPCOMArrayMember(self, name):
        return 'nsMemory::Free(' + name + ');'

    def DestroyPCMArrayMember(self, name):
        return 'free(' + name + ');'

    def ConvertXPCOMToPCM(self, source, dest):
        return 'ConvertAStringToWchar(' + source + ', &' + dest + ');'

    def ConvertPCMToXPCOM(self, source, dest):
        return 'ConvertWcharToAString(' + source + ', ' + dest + ');'

    def XPCOMArgument(self, ident, is_in, is_out):
        if not is_out:
            return 'const nsAString& ' + ident
        return 'nsAString& ' + ident

class Sequence(Type):
    def __init__(self, type):
        self.type_xpidl = '/* unsupported */'
        self.seqTypeInfo = GetTypeInformation(type.seqType().unalias())
        self.type_xpcom = self.seqTypeInfo.type_xpcom + '*'
        self.type_xpcom_const = 'const ' + self.type_xpcom
        self.type_pcm = self.seqTypeInfo.type_pcm + '*'
        self.type_pcm_const = 'const ' + self.type_pcm

    def XPIDLArgument(self, ident, is_in, is_out):
        if is_in:
            if is_out:
                direct = 'inout '
            else:
                direct = 'in '
        else:
            direct = 'out '
        return direct + 'unsigned long len_' + ident + ', ' +\
               '[array, size_is(len_' + ident + ')] ' + direct + ' ' +\
               self.seqTypeInfo.type_xpidl + ' ' + ident
    
    def GetXPCOMStorage(self):
        return XPCOMArrayLocalStorage(self.seqTypeInfo)

    def GetPCMStorage(self):
        return PCMArrayLocalStorage(self.seqTypeInfo)

    def ConvertXPCOMToPCM(self, source, dest):
        lstr = 'sizeof(' + self.seqTypeInfo.type_xpcom + ') * ' + source +\
               '_len'
        return "{\n" +\
               "  " + dest + "_len = " + source + "_len;\n" +\
               '  ' + dest + ' = new ' + self.seqTypeInfo.type_pcm +\
               '[' + source + "_len];\n" +\
               "  PRUint32 _idx;\n" +\
               "  for (_idx = 0; _idx < " + source + "_len; _idx++)\n" +\
               "  {\n" +\
               "    " + self.ConvertIndexXPCOMToPCM(source + "[_idx]",
                                                    dest + "[_idx]") + "\n" +\
               "  }\n" +\
               "}"
    
    def ConvertIndexXPCOMToPCM(self, source, dest):
        return self.seqTypeInfo.ConvertXPCOMToPCM(source, dest)

    def ConvertPCMToXPCOM(self, source, dest):
        lstr = 'sizeof(' + self.seqTypeInfo.type_xpcom + ') * ' + source +\
               '_len'
        return "{\n" +\
               "  " + dest + "_len = " + source + "_len;\n" +\
               '  ' + dest + ' = reinterpret_cast<' + self.type_xpcom +\
               '>(nsMemory::Alloc(' + lstr + "));\n" +\
               "  PRUint32 _idx;\n" +\
               "  for (_idx = 0; _idx < " + source + "_len; _idx++)\n" +\
               "  {\n" +\
               "    " + self.ConvertIndexPCMToXPCOM(source + "[_idx]",
                                                    dest + "[_idx]") + "\n" +\
               "  }\n" +\
               "}"
    
    def ConvertIndexPCMToXPCOM(self, source, dest):
        return self.seqTypeInfo.ConvertPCMToXPCOM(source, dest)

    def XPCOMArgument(self, ident, is_in, is_out):
        if is_out:
            return 'PRUint32 *' + ident + '_len' + ', ' +\
                   self.type_xpcom + '* ' + ident
        else:
            return 'PRUint32 ' + ident + '_len' + ', ' +\
                   self.type_xpcom + ' ' + ident

    def PCMArgument(self, ident, is_in, is_out):
        if is_out:
            return 'uint32_t* ' + ident + '_len, ' +\
                   self.seqTypeInfo.type_pcm + '** ' + ident
        else:
            return 'uint32_t ' + ident + '_len, ' +\
                   self.seqTypeInfo.type_pcm + '* ' + ident

class Declared(Type):
    def __init__(self, type):
        xs = nameutils.XPCOMInterfaceName(type.scopedName())
        if xs == 'XPCOMIIObject':
            self.type_xpidl = 'nsISupports'
            self.type_xpcom = 'nsISupports*'
            self.type_xpcom_class = 'nsISupports'
            self.type_pcm = '::iface::XPCOM::IObject*'
            self.qiclass = 'xpcom::IObject'
            self.x2pclass = '::x2p::XPCOM::IObject'
            self.p2xclass = '::p2x::XPCOM::IObject'
        else:
            self.type_xpidl = xs
            self.type_xpcom = xs + '*'
            self.type_xpcom_class = xs
            self.qiclass = string.join(type.scopedName(), '::')
            self.type_pcm = '::iface::' +\
                            string.join(type.scopedName(), '::') + '*'
            self.x2pclass = '::x2p::' +\
                            string.join(type.scopedName(), '::')
            self.p2xclass = '::p2x::' +\
                            string.join(type.scopedName(), '::')
        self.type_pcm_const = self.type_pcm
        self.type_xpcom_seq = self.type_xpcom

    def DestroyXPCOMArrayMember(self, name):
        return 'NS_RELEASE(' + name + ');'
    def DestroyPCMArrayMember(self, name):
        return 'delete ' + name + ';'
    def GetXPCOMStorage(self):
        return COMPtrLocalStorage(self.type_xpidl)
    def ConvertXPCOMToPCM(self, source, dest):
        return "if (" + source + " == nsnull)\n" +\
               '  ' + dest + " = nsnull;\n" +\
               "else\n" +\
               "{" +\
               "  nsCOMPtr<IWrappedPCM> _wp(do_QueryInterface(" + source +\
               "));\n" +\
               "  if (_wp == nsnull)\n" +\
               '    ' + dest + ' = reinterpret_cast< ' + self.type_pcm +\
               ' >(::P2XFactory::FindFactory("' +\
               self.qiclass + '")->MakeP2X(' + source + "));\n" +\
               "  else\n" +\
               "  {\n" +\
               "    iface::XPCOM::IObject* _pptr;\n" +\
               "    _wp->GetPcmObj(&_pptr);\n" +\
               "    " + dest + ' = reinterpret_cast< ' +\
               self.type_pcm + ' >(_pptr->query_interface("' + self.qiclass +\
               "\"));\n" +\
               "    _pptr->release_ref();\n" +\
               "  }\n" + '}'
    def ConvertPCMToXPCOM(self, source, dest):
        x2p = self.x2pclass
        # x2p::XPCOM::IObject doesn't implement ISupports...
        if x2p == "::x2p::XPCOM::IObject":
            x2p = "::X2PISupports"
        return "if (" + source + " == nsnull)\n" +\
               '  ' + dest + " = nsnull;\n" +\
               "else\n" +\
               "{\n" +\
               '  ::p2x::XPCOM::IObject* _obj = dynamic_cast< ::p2x::XPCOM::' +\
               'IObject*>(' + source + ");\n" +\
               "  if (_obj != nsnull)\n" +\
               '    ' + dest + ' = dont_AddRef<' + self.type_xpidl +\
               '>(static_cast< ' + self.type_xpcom +\
               ">(_obj->GetObject()));\n" +\
               "  else\n" +\
               "  {\n" +\
               '    ' + source + "->add_ref();\n" +\
               '    ' + dest + ' = dont_AddRef<' + self.type_xpidl +\
               '>(reinterpret_cast< ' + self.type_xpcom +\
               ' >(::X2PFactory::FindFactory(NS_GET_IID(' +\
               self.type_xpcom_class + '))->MakeX2P' + '(' + source + ")));\n" +\
               "  }\n" +\
               "}"

class Struct(Declared):
    def __init__(self, type):
        Declared.__init__(self, type)

class Objref(Declared):
    def __init__(self, type):
        Declared.__init__(self, type)
    def GetPCMStorage(self):
        return PCMObjRefLocalStorage(self.type_pcm)

class Enum(Declared):
    def __init__(self, omniidl_type):
        self.type_xpcom = 'PRUint32'
        self.type_xpcom_seq = self.type_xpcom
        self.type_xpidl = 'unsigned long'
        self.type_pcm = '::iface::' +\
                        string.join(omniidl_type.scopedName(), '::')
        self.type_pcm_const = self.type_pcm
    def GetXPCOMStorage(self):
        return XPCOMLocalStorage(self.type_xpcom)
    
    def DestroyXPCOMArrayMember(self, name):
        return ';'

    def DestroyPCMArrayMember(self, name):
        return ';'

    def ConvertXPCOMToPCM(self, source, dest):
        # The extra space is important (unintentional trigraph avoidance).
        return dest + ' = static_cast< ' + self.type_pcm + ' >(' + source +\
               ');'
    def ConvertPCMToXPCOM(self, source, dest):
        return dest + ' = static_cast<PRUint32>(' + source + ');'
