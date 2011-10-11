from omniidl import idlast, idltype

def shouldWarnIfUnused(type):
    type = type.unalias()
    tk = type.kind()
    if tk == idltype.tk_objref or tk == idltype.tk_struct or \
           tk == idltype.tk_array or tk == idltype.tk_sequence or \
           tk == idltype.tk_string or tk == idltype.tk_wstring:
        return ' WARN_IF_RETURN_UNUSED'
    return ''

def typeToSimpleCXX(type, extrapointer=0, is_const=0, is_ret=0):
    extrastar = '*'*extrapointer;

    type = type.unalias()

    if is_const:
        ckw = 'const '
    else:
        ckw = ''
    
    tk = type.kind()
    if tk == idltype.tk_null or tk == idltype.tk_void:
        if extrapointer != 0:
            raise "Can't make pointer to nothing"
        return 'void'
    elif tk == idltype.tk_short:
        return 'int16_t' + extrastar
    elif tk == idltype.tk_long:
        return 'int32_t' + extrastar
    elif tk == idltype.tk_ushort:
        return 'uint16_t' + extrastar
    elif tk == idltype.tk_ulong:
        return 'uint32_t' + extrastar
    elif tk == idltype.tk_float:
        return 'float' + extrastar
    elif tk == idltype.tk_double:
        return 'double' + extrastar
    elif tk == idltype.tk_boolean:
        return 'bool' + extrastar
    elif tk == idltype.tk_char:
        return 'char' + extrastar
    elif tk == idltype.tk_octet:
        return 'uint8_t' + extrastar
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_objref or tk == idltype.tk_struct:
        if is_ret:
            return 'already_AddRefd<' + type.decl().simplecxxscoped + '>'
        else:
            return type.decl().simplecxxscoped + '*' + extrastar
    elif tk == idltype.tk_array:
        return ckw + type.decl().simplecxxscoped + '*' + extrastar
    elif tk == idltype.tk_sequence:
        return typeToSimpleCXX(type.seqType(), extrapointer + 1, is_const)
    elif tk == idltype.tk_enum:
        return type.decl().simplecxxscoped + extrastar
    elif tk == idltype.tk_alias:
        raise 'Unalias didn\'t work!'
        #if type.unalias().kind == idltype.tk_objref or \
        #   type.unalias().kind == idltype.tk_struct:
        #    ckw = '';
        #return ckw + type.decl().simplecxxscoped + extrastar
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_string:
        return ckw + 'char*' + extrastar
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_longlong:
        return 'int64_t' + extrastar
    elif tk == idltype.tk_ulonglong:
        return 'uint64_t' + extrastar
    elif tk == idltype.tk_longdouble:
        return 'long double' + extrastar
    elif tk == idltype.tk_wchar:
        return 'wchar_t' + extrastar
    elif tk == idltype.tk_wstring:
        if is_const:
            ckw = 'const '
        if is_ret:
            r = 'std::wstring'
        else:
            r = 'std::wstring&'
        return ckw + r + extrastar
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

def doesTypeNeedLength(type):
    return type.unalias().kind() == idltype.tk_sequence

def enumOrInt(eoi):
    if isinstance(eoi, idlast.Enumerator):
        return eoi.simplecxxscoped
    return '%d' % eoi
