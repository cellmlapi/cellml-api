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
    prefixIndirect = '';
    suffixIndirect = '*'*extrapointer
    if extrapointer != 0:
        if is_const:
            prefixIndirect = 'const ' + prefixIndirect

    type = type.unalias()

    if not is_ret:
        amp = '&'
    else:
        amp = ''

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
        return prefixIndirect + 'int16_t' + suffixIndirect
    elif tk == idltype.tk_long:
        return prefixIndirect + 'int32_t' + suffixIndirect
    elif tk == idltype.tk_ushort:
        return prefixIndirect + 'uint16_t' + suffixIndirect
    elif tk == idltype.tk_ulong:
        return prefixIndirect + 'uint32_t' + suffixIndirect
    elif tk == idltype.tk_float:
        return prefixIndirect + 'float' + suffixIndirect
    elif tk == idltype.tk_double:
        return prefixIndirect + 'double' + suffixIndirect
    elif tk == idltype.tk_boolean:
        return prefixIndirect + 'bool' + suffixIndirect
    elif tk == idltype.tk_char:
        return prefixIndirect + 'char' + suffixIndirect
    elif tk == idltype.tk_octet:
        return prefixIndirect + 'uint8_t' + suffixIndirect
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_objref or tk == idltype.tk_struct:
        if is_ret == 1:
            return prefixIndirect + 'already_AddRefd<' + type.decl().simplecxxscoped + '> ' + suffixIndirect
        else:
            return prefixIndirect + type.decl().simplecxxscoped + '*' + suffixIndirect
    elif tk == idltype.tk_array:
        return ckw + 'std::vector<' + prefixIndirect + type.decl().simplecxxscoped + suffixIndirect + '> ' + suffixIndirect + amp
    elif tk == idltype.tk_sequence:
        base = typeToSimpleCXX(type.seqType(), 0, is_ret = 2)
        return ckw + 'std::vector<' + base + '>' + amp
    elif tk == idltype.tk_enum:
        return prefixIndirect + type.decl().simplecxxscoped + suffixIndirect
    elif tk == idltype.tk_alias:
        raise 'Unalias didn\'t work!'
        #if type.unalias().kind == idltype.tk_objref or \
        #   type.unalias().kind == idltype.tk_struct:
        #    ckw = '';
        #return ckw + type.decl().simplecxxscoped + extrastar
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_string:
        return ckw + prefixIndirect + 'std::string' + suffixIndirect + amp
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_longlong:
        return prefixIndirect + 'int64_t' + suffixIndirect
    elif tk == idltype.tk_ulonglong:
        return prefixIndirect + 'uint64_t' + suffixIndirect
    elif tk == idltype.tk_longdouble:
        return prefixIndirect + 'long double' + suffixIndirect
    elif tk == idltype.tk_wchar:
        return prefixIndirect + 'wchar_t' + suffixIndirect
    elif tk == idltype.tk_wstring:
        return ckw + prefixIndirect + 'std::wstring' + suffixIndirect + amp
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

def enumOrInt(eoi):
    if isinstance(eoi, idlast.Enumerator):
        return eoi.simplecxxscoped
    return '%d' % eoi
