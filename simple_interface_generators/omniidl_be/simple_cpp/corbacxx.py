from omniidl import idlast, idltype
import string

DIRECTION_IN=0
DIRECTION_INOUT=1
DIRECTION_OUT=2
DIRECTION_RET=3

# Table from omniORB...
# CORBA2.3 P1-21 1.9 Mapping for Structured Types
typeIsVariableLength = {
    idltype.tk_null:               0,
    idltype.tk_void:               0,
    idltype.tk_short:              0,
    idltype.tk_long:               0,
    idltype.tk_ushort:             0,
    idltype.tk_ulong:              0,
    idltype.tk_float:              0,
    idltype.tk_double:             0,
    idltype.tk_boolean:            0,
    idltype.tk_char:               0,
    idltype.tk_octet:              0,
    idltype.tk_any:                1,
    idltype.tk_objref:             1,
    idltype.tk_string:             1,
    idltype.tk_sequence:           1,
    idltype.tk_except:             1,
    idltype.tk_longlong:           0,
    idltype.tk_ulonglong:          0,
    idltype.tk_longdouble:         0,
    idltype.tk_wchar:              0,
    idltype.tk_wstring:            1,
    idltype.tk_fixed:              0,
    idltype.tk_value:              1,
    idltype.tk_value_box:          1,
    idltype.tk_abstract_interface: 1,
    idltype.tk_any:                1,
    idltype.tk_TypeCode:           1,
    idltype.tk_enum:               0
    }

basic_map = {
    idltype.tk_short: ("::CORBA::Short", "::CORBA::Short_out"),
    idltype.tk_long: ("::CORBA::Long", "::CORBA::Long_out"),
    idltype.tk_longlong: ("::CORBA::LongLong", "::CORBA::LongLong_out"),
    idltype.tk_ushort: ("::CORBA::UShort", "::CORBA::UShort_out"),
    idltype.tk_ulong: ("::CORBA::ULong", "::CORBA::ULong_out"),
    idltype.tk_ulonglong: ("::CORBA::ULongLong", "::CORBA::ULongLong_out"),
    idltype.tk_float: ("::CORBA::Float", "::CORBA::Float_out"),
    idltype.tk_double: ("::CORBA::Double", "::CORBA::Double_out"),
    idltype.tk_longdouble: ("::CORBA::LongDouble", "::CORBA::LongDouble_out"),
    idltype.tk_char: ("::CORBA::Char", "::CORBA::Char_out"),
    idltype.tk_wchar: ("::CORBA::WChar", "::CORBA::WChar_out"),
    idltype.tk_boolean: ("::CORBA::Boolean", "::CORBA::Boolean_out"),
    idltype.tk_octet: ("::CORBA::Octet" "::CORBA::Octet_out")
    }

def isTypeVariable(type):
    uatype = type.unalias()
    kind = uatype.kind()
    # Sometimes, knowing the type is enough to know is its variable...
    if typeIsVariableLength.has_key(kind):
        return typeIsVariableLength[kind]
    elif kind == idltype.tk_struct:
        for sm in uatype.decl().members():
            if isTypeVariable(sm.memberType()):
                return 1
    elif kind == idltype.tk_union:
        for sm in uatype.decl().cases():
            if isTypeVariable(sm.caseType()):
                return 1
    else:
        raise "Unknown type encountered"

def typeToCORBACXX(itype, direction):
    type = itype.unalias()
    kind = type.kind()
    
    # Arrays need special treatment...
    if (kind == idltype.tk_alias):
        if kind == idltype.tk_void:
            raise "Can't have an array of void"
        bt = typeTypeCORBACXX(type.decl().alias().aliasType(), DIRECTION_RET)
        ar_sz = string.join(map(lambda x: ('[' + x + ']'), type.sizes()), '')
        ar_sz_sl = '[]' + string.join(map(lambda x: ('[' + x + ']'),
                                          type.sizes()[1:]), '')
        if direction == DIRECTION_IN:
            return 'const ' + bt + ar_sz
        elif direction == DIRECTION_INOUT:
            return bt + ar_sz
        elif direction == DIRECTION_RET:
            return bt + ar_sz_sl
        elif isTypeVariable(type):
            return bt + ar_sz_sl + '*&'
        else:
            return bt + ar_sz

    # Get basic types over and done with...
    if basic_map.has_key(kind):
        if direction in [DIRECTION_RET, DIRECTION_IN]:
            return basic_map[kind][0]
        else:
            return basic_map[kind][1]

    # String types are just a table lookup too...
    if kind == idltype.tk_string:
        return ('const char*', 'char*&',
                '::CORBA::String_out', 'char*')[direction]
    elif kind == idltype.tk_wstring:
        return ('const ::CORBA::WChar*', '::CORBA::WChar*&',
                '::CORBA::WString_out', '::CORBA::WChar*')[direction]

    if kind == idltype.tk_void:
        if direction != DIRECTION_RET:
            raise 'void only allowed as a return type.'
        return 'void'

    if kind in [idltype.tk_null, idltype.tk_any, idltype.tk_TypeCode,
                idltype.tk_Principal, idltype.tk_array, idltype.tk_except,
                idltype.tk_fixed, idltype.tk_value, idltype.tk_value_box,
                idltype.tk_native, idltype.tk_abstract_interface]:
        raise 'Type not supported in this context.'

    # We now know we have a declared type with a name, but we may have unaliased it...
    while itype.kind() == idltype.tk_alias:
        type = itype
        itype = itype.decl().alias().aliasType()

    if not isinstance(type, idltype.Declared):
        raise "Cannot process non-typedef'd struct/sequence/union"
    typename = '::' + type.decl().corbacxxscoped

    if kind == idltype.tk_objref:
        if direction == DIRECTION_IN or direction == DIRECTION_RET:
            return typename + '_ptr'
        elif direction == DIRECTION_OUT:
            return typename + '_out'
        else:
            return typename + '_ptr&'

    if kind == idltype.tk_enum:
        if direction == DIRECTION_IN or direction == DIRECTION_RET:
            return typename
        elif direction == DIRECTION_OUT:
            return typename + '_out'
        else:
            return typename + '&'

    if direction == DIRECTION_IN:
        return 'const ' + typename + '&'
    elif direction == DIRECTION_INOUT:
        return typename + '&'
    elif direction == DIRECTION_RET:
        if kind == idltype.tk_sequence or isTypeVariable(type):
            return typename + '*&'
        else:
            return typename + '&'
    else:
        return typename + '_out'
    return 'const ' + typename + ''

    # Get argmapping with information on the type(based on omniidl code)...
    #   argmapping[0]: 0 if non-const, 1 if const.
    #   argmapping[1]: 0 if no &, 1 if &(reference) in type.
    #   argmapping[2]: 0 if no *, 1 if *(pointer) in type.
    if type.dims() != []:
        if isTypeVariable(type):
            argmapping = ( (1, 0, 0), (0, 0, 0),
                           (0, 0, 0), (0, 0, 1) )[direction]
        else:
            argmapping = ( (1, 0, 0), (0, 1, 1),
                           (0, 0, 0), (0, 0, 1) )[direction]
    else:
        kind = type.unalias().kind()
        if kind in [ idltype.tk_short, idltype.tk_long, idltype.tk_longlong,
                     idltype.tk_ushort, idltype.tk_ulong, idltype.tk_ulonglong,
                     idltype.tk_float, idltype.tk_double, idltype.tk_enum,
                     idltype.tk_longdouble, idltype.tk_boolean,
                     idltype.tk_char, idltype.tk_wchar, idltype.tk_octet ]:
            argmapping = ( (0, 0, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        elif kind in [ idltype.tk_objref, idltype.tk_TypeCode ]:
            argmapping = ( (0, 0, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        elif kind in [idltype.tk_struct, idltype.tk_union]:
            if isTypeVariable(type):
                argmapping = ( (1, 1, 0), (0, 1, 1), (0, 1, 0),
                               (0, 0, 1) )[direction]
            else:
                argmapping = ( (1, 1, 0), (0, 1, 0), (0, 1, 0),
                               (0, 0, 0) )[direction]
        elif kind in [idltype.tk_string, idltype.tk_wstring]:
            argmapping = ( (1, 0, 0), (0, 1, 0), (0, 1, 0),
                           (0, 0, 0) )[direction]
        elif kind == idltype.tk_sequence:
            argmapping = ( (1, 0, 0), (0, 1, 0), (0, 1, 0),
                           (0, 0, 0) )[direction]
        elif kind == idltype.tk_void:
            if direction == RET:
                return 'void'
            else:
                raise 'Cannot use tk_void except as a return type.'
        else:
            raise 'Unknown type encountered.'

    # See if it is a basic data type...
    if basic_map.has_key(type):
        return applyMapping(basic_map[type], argmapping)

    # See if it is a string
    
    ra = '::'
    
    extrastar = ''
    if is_out:
        extrastar = '*'
    
    tk = type.kind()
    if tk == idltype.tk_null or tk == idltype.tk_void:
        if is_out:
            raise "Can't make an out void"
        return 'void'
    elif tk == idltype.tk_short:
        return ra + 'CORBA::Short' + extrastar
    elif tk == idltype.tk_long:
        return ra + 'CORBA::Long' + extrastar
    elif tk == idltype.tk_ushort:
        return ra + 'CORBA::UShort' + extrastar
    elif tk == idltype.tk_ulong:
        return ra + 'CORBA::ULong' + extrastar
    elif tk == idltype.tk_float:
        return ra + 'CORBA::Float' + extrastar
    elif tk == idltype.tk_double:
        return ra + 'CORBA::Double' + extrastar
    elif tk == idltype.tk_boolean:
        return ra + 'CORBA::Boolean' + extrastar
    elif tk == idltype.tk_char:
        return ra + 'CORBA::Char' + extrastar
    elif tk == idltype.tk_octet:
        return ra + 'CORBA::Octet' + extrastar
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_objref or tk == idltype.tk_struct:
        if is_out:
            return ra + type.decl().corbacxxscoped + '_out'
        else:
            return ra + type.decl().corbacxxscoped + '_ptr'
    elif tk == idltype.tk_array:
        if is_out:
            return ra + type.decl().corbacxxscoped + '_out'
        else:
            return ckw + ra + type.decl().corbacxxscoped + '_ptr'
    elif tk == idltype.tk_sequence:
        # Don't Unalias and get the type...
        if is_out:
            return ra + itype.decl().corbacxxscoped + '_out'
        else:
            return ckw + ra + itype.decl().corbacxxscoped + '_ptr'
    elif tk == idltype.tk_enum:
        return ra + type.decl().simplecxxscoped + extrastar
    elif tk == idltype.tk_alias:
        raise 'Unalias didn\'t work!'
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_string:
        if is_out:
            return 'CORBA::String_out'
        else:
            return ckw + 'char*'
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_longlong:
        return ra + 'CORBA::LongLong' + extrastar
    elif tk == idltype.tk_ulonglong:
        return ra + 'CORBA::ULongLong' + extrastar
    elif tk == idltype.tk_longdouble:
        return ra + 'CORBA::LongDouble' + extrastar
    elif tk == idltype.tk_wchar:
        return ra + 'CORBA::WChar' + extrastar
    elif tk == idltype.tk_wstring:
        if is_out:
            return ra + 'CORBA::WString_out'
        else:
            return ckw + ra + 'CORBA::WChar*'
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        raise 'Unknown type kind %u' % tk
