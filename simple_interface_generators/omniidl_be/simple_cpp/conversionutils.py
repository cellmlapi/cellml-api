from omniidl import idltype, idlast
import simplecxx
import string

def convertArrayS2C(out, cname, sname, sz, at, vt):
    acs = ''
    # We need to create the array...
    out.out(cname + ' = new ::' + vt + '();')
    for i in range(0, len(sz)):
        out.ci_count = out.ci_count + 1
        acn = '_ac%u' % out.ci_count
        out.out('size_t ' + acn + ';')
        out.out('for (' + acn + ' = 0; ' + acn + ' < ' + sz[i] + '; ' + acn +\
                '++)')
        out.out('{')
        out.inc_indent()
        acs = acs + '[' + acn + ']'
    writeSimpleToCORBA(out, at, sname + acs, cname + acs)
    for i in range(0, len(sz)):
        out.dec_indent()
        out.out('}')

def convertArrayC2S(out, cname, sname, sz, at):
    acs = ''
    # We need to create the array...
    out.out(sname + ' = new ::' + at.decl().simplecxxscoped + '[' + sz[0] + '];')
    for i in range(0, len(sz)):
        out.ci_count = out.ci_count + 1
        acn = '_ac%u' % out.ci_count
        out.out('size_t ' + acn + ';')
        out.out('for (' + acn + ' = 0; ' + acn + ' < ' + sz[i] + '; ' + acn +\
                '++)')
        out.out('{')
        out.inc_indent()
        acs = acs + '[' + acn + ']'
    writeCORBAValueToSimple(out, at, cname + acs, sname + acs)
    for i in range(0, len(sz)):
        out.dec_indent()
        out.out('}')


def convertStructureS2C(out, type, cname, sname):
    str = type.decl()
    out.out(cname + ' = new ::' + str.corbacxxscoped + ';')
    for m in str.members():
        t = m.memberType()
        for d in m.declarators():
            if t.unalias().kind() == idltype.tk_sequence:
                writeSimpleSequenceToCORBA(out, t, sname + '->' + d.simplename,
                                           sname + '->_length_' + d.simplename,
                                           cname + '->' + d.simplename)
            else:
                writeSimpleToCORBA(out, t, sname + '->' + d.simplename,
                                   cname + '->' + d.simplename)

def convertStructureC2S(out, type, cname, sname):
    str = type.decl()
    out.out(sname + ' = new ::' + str.simplecxxscoped + ';')
    for m in str.members():
        t = m.memberType()
        for d in m.declarators():
            if t.unalias().kind() == idltype.tk_sequence:
                writeCORBASequenceToSimple(out, t, cname + '->' + d.simplename,
                                           sname + '->' + d.simplename,
                                           sname + '->_length_' + d.simplename)
            else:
                writeCORBAValueToSimple(out, t, cname + '->' + d.simplename,
                                        sname + '->' + d.simplename)

def convertObjRefS2C(out, type, cname, sname):
    # First see if we are trying to convert a CCI...
    out.ci_count = out.ci_count + 1
    cciname = '_cci%u' % out.ci_count
    out.out('CCI::' + type.decl().corbacxxscoped + ' *' + cciname + ' = ' +\
            'dynamic_cast<CCI::' + type.decl().corbacxxscoped + '*>(' +\
            sname + ');')
    out.out('if (' + cciname + ' != NULL)')
    out.out('{')
    # If it is a CCI, we unwrap rather than rewrap...
    out.inc_indent()
    unwrapName = '_unwrap_' + string.join(type.decl().scopedName(), '_')
    out.out(cname + ' = ' + cciname + '->' + unwrapName + '();')
    # unwrap automatically calls add_ref.
    out.dec_indent()
    out.out('}')
    out.out('else if (' + sname + ' == NULL)')
    out.out('{')
    out.inc_indent()
    out.out(cname + ' = ::' + type.decl().corbacxxscoped + '::_nil();')
    out.dec_indent()
    out.out('}')
    out.out('else')
    # otherwise, wrap it in an SCI
    out.out('{')
    out.inc_indent()
    out.out('::SCI::' + type.decl().corbacxxscoped + ('* _sci%u' % out.ci_count) +\
            ' = new ::SCI::' + type.decl().finalcciscoped + '(' +\
            sname + ', _getPOA());')
    # No point doing add_ref (for CORBA AOM), then release_ref (initial ref),
    # so I cancelled them out.
    # out.out(('_sci%u' % out.ci_count) + '->add_ref();')
    out.out('delete _getPOA()->activate_object(_sci%u);' % out.ci_count)
    out.out(cname + (' = _sci%u' % out.ci_count) + '->_this();')
    # out.out(('_sci%u' % out.ci_count) + '->release_ref();')
    out.dec_indent()
    out.out('}')

def convertObjRefC2S(out, type, cname, sname):
    # First see if we are trying to convert an OR implemented by an SCI...
    out.ci_count = out.ci_count + 1
    srvname = '_srv%u' % out.ci_count
    out.out('if (CORBA::is_nil(' + cname + '))')
    out.out('{')
    out.inc_indent()
    out.out(sname + ' = NULL;')
    out.dec_indent()
    out.out('}')
    out.out('else')
    out.out('{')
    out.inc_indent()
    out.out('try')
    out.out('{')
    out.inc_indent()
    out.out('::PortableServer::Servant ' + srvname +\
            ' = _getPOA()->reference_to_servant(' + cname + ');')
    # See if we can dynamic_cast it to the SCI class...
    sciname = '_sci%u' % out.ci_count
    unwrapName = '_unwrap_' + string.join(type.decl().scopedName(), '_')
    out.out('::SCI::' + type.decl().corbacxxscoped + ' *' + sciname + ' = ' +\
            'dynamic_cast<SCI::' + type.decl().corbacxxscoped + '*>(' +\
            srvname + ');')
    # If we failed, _remove_ref() and go to the catch block...
    out.out('if (' + sciname + ' == NULL)')
    out.out('{')
    out.inc_indent()
    out.out(srvname + '->_remove_ref();')
    out.out('throw std::exception();')
    out.dec_indent()
    out.out('}')
    out.out(sname + ' = ' + sciname + '->' + unwrapName + '();')
    out.out(srvname + '->_remove_ref();')
    out.dec_indent()
    out.out('}')
    out.out('catch (...)')
    out.out('{')
    out.inc_indent()
    # We either have a remote CORBA object reference, or a local one which is
    # implemented by means other than via the simple C/C++ mapping(perhaps via
    # the CORBA C++ mapping) on the same POA. Wrap it in a CCI
    out.out('  ' + sname + ' = ' + 'new ::CCI::' +\
            type.decl().finalcciscoped + '(' + cname + ', _getPOA());')
    out.dec_indent()
    out.out('}')
    out.dec_indent()
    out.out('}')

def convertStringC2S(out, cname, sname, fromCall):
    out.ci_count = out.ci_count + 1
    if fromCall:
        svn = cname
    else:
        svn = '_sv%u' % out.ci_count
    svln = '_svl%u' % out.ci_count
    if not fromCall:
        out.out('const char* ' + svn + ' = ' + cname + '.in();')
    out.out('uint32_t ' + svln + ' = strlen(' + svn + ') + 1;')
    # Do we really need to worry about 4GB strings? If we do, overflow could be
    # a security/stability risk, so I am leaving the check in just in case. Of
    # course, this will produce wrong results, but it won't crash. We have no
    # exception path for handling failed allocations anyway, so perhaps this is
    # no bigger a risk.
    out.out('if (' + svln + ' == 0) ' + svln + ' = 1;')
    out.out(sname + ' = (char*)malloc(' + svln + ');')
    out.out('memcpy(' + sname + ', ' + svn + ', ' + svln + ' - 1);')
    out.out(sname + '[' + svln + ' - 1] = 0;')

def convertWStringC2S(out, cname, sname, fromCall):
    out.ci_count = out.ci_count + 1
    if fromCall:
        svn = cname
    else:
        svn = '_sv%u' % out.ci_count
    svln = '_svl%u' % out.ci_count
    if not fromCall:
        out.out('const wchar_t* ' + svn + ' = ' + cname + '.in();')
    out.out('uint32_t ' + svln + ' = wcslen(' + svn + ') + 1;')
    # In this case, there is a more genuine security issue...
    out.out('if (' + svln + ' == 0 || ' + svln + ' * sizeof(wchar_t) < ' + svln + ') ' +\
            svln + ' = 1;')
    out.out(sname + ' = (wchar_t*)malloc(sizeof(wchar_t) * ' + svln + ');')
    out.out('memcpy(' + sname + ', ' + svn + ', (' + svln + ' - 1) * sizeof(wchar_t));')
    out.out(sname + '[' + svln + ' - 1] = 0;')

def getCORBAVarType(itype):
    type = itype.unalias()
    tk = type.kind()

    if tk == idltype.tk_null or tk == idltype.tk_void:
        raise "Can't convert a null/void"
    elif tk == idltype.tk_short:
        return '::CORBA::Short'
    elif tk == idltype.tk_long:
        return '::CORBA::Long'
    elif tk == idltype.tk_ushort:
        return '::CORBA::UShort'
    elif tk == idltype.tk_ulong:
        return '::CORBA::ULong'
    elif tk == idltype.tk_float:
        return '::CORBA::Float'
    elif tk == idltype.tk_double:
        return '::CORBA::Double'
    elif tk == idltype.tk_boolean:
        return '::CORBA::Boolean'
    elif tk == idltype.tk_char:
        return '::CORBA::Char'
    elif tk == idltype.tk_octet:
        return '::CORBA::Octet'
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_objref:
        return '::' + type.decl().corbacxxscoped + '_var'
    elif tk == idltype.tk_struct:
        return '::' + type.decl().corbacxxscoped + '_var'
    elif tk == idltype.tk_sequence:
        if itype.kind() != idltype.tk_alias:
            raise 'Anonymous sequence types are not supported.'
        return '::' + itype.decl().corbacxxscoped + '_var'
    elif tk == idltype.tk_enum:
        return '::' + type.decl().corbacxxscoped
    elif tk == idltype.tk_alias or tk == idltype.tk_array:
        # Array => _var
        return '::' + type.decl().corbacxxscoped + '_var'
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_string:
        # Casting to const forces String_var to perform a copy...
        return '::CORBA::String_var'
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_longlong:
        return '::CORBA::LongLong'
    elif tk == idltype.tk_ulonglong:
        return '::CORBA::ULongLong'
    elif tk == idltype.tk_longdouble:
        return '::CORBA::LongDouble'
    elif tk == idltype.tk_wchar:
        return '::CORBA::WChar'
    elif tk == idltype.tk_wstring:
        return '::CORBA::WString_var'
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or \
             tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        raise 'Unknown type kind %u' % tk

def returnExpr(itype, name):
    type = itype.unalias()
    tk = type.kind()

    if tk == idltype.tk_null or tk == idltype.tk_void:
        raise "Can't return a null/void"
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_objref or tk == idltype.tk_struct or \
         tk == idltype.tk_sequence or tk == idltype.tk_alias or \
         tk == idltype.tk_array or tk == idltype.tk_string or \
         tk == idltype.tk_wstring:
        return name + '._retn()'
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or \
             tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        return name

def castAndAssign(out, tovar, typestr, fromvar):
    if tovar == None:
        out.out('return (' + typestr + ')' + fromvar + ';')
    else:
        out.out(tovar + ' = (' + typestr + ')' + fromvar + ';')

def writeSimpleToCORBA(out, type, sname, cname, fromCall=0, toParam=0):
    type = type.unalias()
    tk = type.kind()

    if tk == idltype.tk_null or tk == idltype.tk_void:
        raise "Can't convert a null/void"
    elif tk == idltype.tk_short:
        castAndAssign(out, cname, '::CORBA::Short', sname)
    elif tk == idltype.tk_long:
        castAndAssign(out, cname, '::CORBA::Long', sname)
    elif tk == idltype.tk_ushort:
        castAndAssign(out, cname, '::CORBA::UShort', sname)
    elif tk == idltype.tk_ulong:
        castAndAssign(out, cname, '::CORBA::ULong', sname)
    elif tk == idltype.tk_float:
        castAndAssign(out, cname, '::CORBA::Float', sname)
    elif tk == idltype.tk_double:
        castAndAssign(out, cname, '::CORBA::Double', sname)
    elif tk == idltype.tk_boolean:
        castAndAssign(out, cname, '::CORBA::Boolean', sname)
    elif tk == idltype.tk_char:
        castAndAssign(out, cname, '::CORBA::Char', sname)
    elif tk == idltype.tk_octet:
        castAndAssign(out, cname, '::CORBA::Octet', sname)
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_array or tk == idltype.tk_alias:
        # If we get here, it must be an array...
        convertArrayS2C(out, cname, sname, type.decl().sizes(),
                        type.decl().alias().aliasType(),
                        type.decl().corbacxxscoped + '_var')
    elif tk == idltype.tk_objref:
        convertObjRefS2C(out, type, cname, sname)
    elif tk == idltype.tk_struct:
        convertStructureS2C(out, type, cname, sname)
    elif tk == idltype.tk_sequence:
        raise "Internal error: you are not supposed to call " +\
              "WriteSimpleToCORBA on a sequence!"
    elif tk == idltype.tk_enum:
        # A simple cast will do here, because we match the values...
        castAndAssign(out, cname, '::' + type.decl().corbacxxscoped, sname)
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_string:
        if not toParam:
            # Casting to const forces String_var to perform a copy...
            castAndAssign(out, cname, 'const char*', sname)
        else:
            # We are assigning to a raw pointer, so have to duplicate...
            out.out(cname + ' = CORBA::string_dup(' + sname + ');')
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_longlong:
        castAndAssign(out, cname, '::CORBA::LongLong', sname)
    elif tk == idltype.tk_ulonglong:
        castAndAssign(out, cname, '::CORBA::ULongLong', sname)
    elif tk == idltype.tk_longdouble:
        castAndAssign(out, cname, '::CORBA::LongDouble', sname)
    elif tk == idltype.tk_wchar:
        castAndAssign(out, cname, '::CORBA::WChar', sname)
    elif tk == idltype.tk_wstring:
        if not toParam:
            castAndAssign(out, cname, 'const wchar_t*', sname)
        else:
            out.out(cname + ' = CORBA::wstring_dup(' + sname + ');')
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or \
             tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        raise 'Unknown type kind %u' % tk

def writeSimpleSequenceToCORBA(out, type, sarray, slength, cname, \
                               fromCall=0, toParam=0):
    out.ci_count = out.ci_count + 1
    # Firstly, construct the sequence. Assume managed storage.
    if not toParam:
        out.out(cname + ' = new ::' + type.decl().corbacxxscoped + '();')
    # Next, set the length...
    if fromCall:
        out.out(cname + '.length(' + slength + ');')
    else:
        out.out(cname + '->length(' + slength + ');')
    # Finally, we have to copy everything over...
    ciname = '_ci%u' % out.ci_count
    out.out('size_t ' + ciname + ';')
    out.out('for (' + ciname + ' = 0; ' + ciname + ' < ' + slength +\
                 '; ' + ciname + '++)')
    out.out('{')
    out.inc_indent()
    assignto = cname + '[' + ciname + ']'
    assignfrom = sarray + '[' + ciname + ']'
    # Nested sequences can't be done, because they are incompatible with the
    # array/length form. Instead, need to define a struct with just that
    # sequence. We could define that this implicitly happens in our binding
    # if it becomes important later.
    writeSimpleToCORBA(out, type.unalias().seqType(), assignfrom, assignto)
    out.dec_indent()
    out.out('}')

def writeCORBASequenceToSimple(out, type, cname, sarray, slength,
                               fromCall=0, needAlloc=0):
    addRet = 0
    if sarray == None:
        out.out(simplecxx.typeToSimpleCXX(type) + ' _myreturn;')
        addRet = 1
        sarray = '_myreturn'
    # Do the easy part first...
    if fromCall:
        out.out(slength + ' = ' + cname + '.length();')
    else:
        out.out(slength + ' = ' + cname + '->length();')
    if needAlloc:
        out.out(sarray + ' = new ' +\
                simplecxx.typeToSimpleCXX(type, extrapointer=-1) + '[' +\
                slength + '];')
    # Allocate a temp var
    out.ci_count = out.ci_count + 1
    ciname = '_ci%u' % out.ci_count
    out.out('size_t ' + ciname + ';')
    out.out('for (' + ciname + ' = 0; ' + ciname + ' < ' + slength +\
                 '; ' + ciname + '++)')
    out.out('{')
    out.inc_indent()
    assignfrom = cname + '[' + ciname + ']'
    assignto = sarray + '[' + ciname + ']'
    writeCORBAValueToSimple(out, type.unalias().seqType(), assignfrom, assignto)
    out.dec_indent()
    out.out('}')
    if addRet:
        out.out('return _myreturn;')

def writeCORBAValueToSimple(out, type, cname, sname, fromCall=0):
    type = type.unalias()
    tk = type.kind()

    addRet = 0
    if sname == None:
        out.out(simplecxx.typeToSimpleCXX(type) + ' _myreturn;')
        addRet = 1
        sname = '_myreturn'

    if tk == idltype.tk_null or tk == idltype.tk_void:
        raise "Can't convert a null/void"
    elif tk == idltype.tk_short:
        castAndAssign(out, sname, 'int16_t', cname)
    elif tk == idltype.tk_long:
        castAndAssign(out, sname, 'int32_t', cname)
    elif tk == idltype.tk_ushort:
        castAndAssign(out, sname, 'uint16_t', cname)
    elif tk == idltype.tk_ulong:
        castAndAssign(out, sname, 'uint32_t', cname)
    elif tk == idltype.tk_float:
        castAndAssign(out, sname, 'float', cname)
    elif tk == idltype.tk_double:
        castAndAssign(out, sname, 'double', cname)
    elif tk == idltype.tk_boolean:
        castAndAssign(out, sname, 'bool', cname)
    elif tk == idltype.tk_char:
        castAndAssign(out, sname, 'char', cname)
    elif tk == idltype.tk_octet:
        castAndAssign(out, sname, 'uint8_t', cname)
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_array or tk == idltype.tk_alias:
        # If we get here, it must be an array...
        convertArrayC2S(out, cname, sname, type.decl().sizes(),
                        type.decl().alias().aliasType())
    elif tk == idltype.tk_objref:
        convertObjRefC2S(out, type, cname, sname)
    elif tk == idltype.tk_struct:
        convertStructureC2S(out, type, cname, sname)
    elif tk == idltype.tk_sequence:
        raise "Internal error: you are not supposed to call " +\
              "writeCORBAValueToSimple() on a sequence!"
    elif tk == idltype.tk_enum:
        # A simple cast will do here, because we match the values...
        castAndAssign(out, sname, type.decl().simplecxxscoped, cname)
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_string:
        convertStringC2S(out, cname, sname, fromCall)
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_longlong:
        castAndAssign(out, sname, 'int64_t', cname)
    elif tk == idltype.tk_ulonglong:
        castAndAssign(out, sname, 'uint64_t', cname)
    elif tk == idltype.tk_longdouble:
        castAndAssign(out, sname, 'long double', cname)
    elif tk == idltype.tk_wchar:
        castAndAssign(out, sname, 'wchar_t', cname)
    elif tk == idltype.tk_wstring:
        convertWStringC2S(out, cname, sname, fromCall)
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or \
             tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        raise 'Unknown type kind %u' % tk

    if addRet:
        out.out('return _myreturn;')

def destroySimpleStructure(out, type, sname):
    for m in str.members():
        t = m.memberType()
        for d in m.declarators():
            if t.unalias().kind() == idltype.tk_sequence:
                destroySimpleSequence(out, t, sname + '->' + d.simplename,
                                      sname + '->_length_' + d.simplename)
            else:
                destroySimpleValue(out, t, sname + '->' + d.simplename)

def destroySimpleValue(out, type, sname):
    type = type.unalias()
    tk = type.kind()
    if tk == idltype.tk_null or tk == idltype.tk_void or \
       tk == idltype.tk_short or tk == idltype.tk_long or \
       tk == idltype.tk_ushort or tk == idltype.tk_ulong or \
       tk == idltype.tk_float or tk == idltype.tk_double or \
       tk == idltype.tk_boolean or tk == idltype.tk_char or \
       tk == idltype.tk_octet or tk == idltype.tk_enum or \
       tk == idltype.tk_longlong or tk == idltype.tk_ulonglong or \
       tk == idltype.tk_longdouble or tk == idltype.tk_wchar:
        pass
    elif tk == idltype.tk_string or tk == idltype.tk_wstring:
        out.out('free(' + sname + ');')
    elif tk == idltype.tk_array or tk == idltype.tk_alias:
        # If we get here, it must be an array...
        destroySimpleArray(out, sname, type.decl().sizes(),
                           type.decl().alias().aliasType())
    elif tk == idltype.tk_struct:
        destroySimpleStructure(out, type, sname)
    elif tk == idltype.tk_objref:
        out.out('if (' + sname + ')')
        out.inc_indent()
        out.out(sname + '->release_ref();')
        out.dec_indent()
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_sequence:
        raise "Internal error: you are not supposed to call " +\
              "WriteSimpleToCORBA on a sequence!"
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or \
             tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        raise 'Unknown type kind %u' % tk

def destroyCORBAValue(out, type, cname, isRaw=0):
    type = type.unalias()
    tk = type.kind()
    if tk == idltype.tk_null or tk == idltype.tk_void or \
       tk == idltype.tk_short or tk == idltype.tk_long or \
       tk == idltype.tk_ushort or tk == idltype.tk_ulong or \
       tk == idltype.tk_float or tk == idltype.tk_double or \
       tk == idltype.tk_boolean or tk == idltype.tk_char or \
       tk == idltype.tk_octet or tk == idltype.tk_enum or \
       tk == idltype.tk_longlong or tk == idltype.tk_ulonglong or \
       tk == idltype.tk_longdouble or tk == idltype.tk_wchar:
        pass
    elif tk == idltype.tk_string:
        if isRaw:
            out.out('CORBA::string_free(' + cname + ');')
    elif tk == idltype.tk_wstring:
        if isRaw:
            out.out('CORBA::wstring_free(' + cname + ');')
    elif tk == idltype.tk_array or tk == idltype.tk_alias:
        # If we get here, it must be an array...
        destroyCORBAArray(out, cname, type.decl().sizes(),
                          type.decl().alias().aliasType())
    elif tk == idltype.tk_struct:
        destroyCORBAStructure(out, type, cname)
    elif tk == idltype.tk_objref:
        out.out('if (!CORBA::is_nil(' + cname + '))')
        out.inc_indent()
        out.out(cname + '->release_ref();')
        out.dec_indent()
    elif tk == idltype.tk_any:
        raise 'Any is not supported'
    elif tk == idltype.tk_TypeCode:
        raise 'TypeCode is not supported'
    elif tk == idltype.tk_Principal:
        raise 'Principal is not supported'
    elif tk == idltype.tk_sequence:
        raise "Internal error: you are not supposed to call " +\
              "WriteSimpleToCORBA on a sequence!"
    elif tk == idltype.tk_union:
        raise 'Union is not supported'
    elif tk == idltype.tk_except:
        raise 'Can\'t pass an exception as a type.'
    elif tk == idltype.tk_fixed:
        raise 'Fixed precision is not supported.'
    elif tk == idltype.tk_value:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_value_box:
        raise 'valuetype is not supported.'
    elif tk == idltype.tk_native:
        raise 'native is not supported.'
    elif tk == idltype.tk_abstract_interface or \
             tk == idltype.tk_local_interface:
        raise 'Passing interfaces is not supported.'
    else:
        raise 'Unknown type kind %u' % tk

def destroySimpleSequence(out, type, sname, slength):
    type = type.unalias().seqType()
    tk = type.kind()
    if not (tk == idltype.tk_null or tk == idltype.tk_void or \
            tk == idltype.tk_short or tk == idltype.tk_long or \
            tk == idltype.tk_ushort or tk == idltype.tk_ulong or \
            tk == idltype.tk_float or tk == idltype.tk_double or \
            tk == idltype.tk_boolean or tk == idltype.tk_char or \
            tk == idltype.tk_octet or tk == idltype.tk_enum or \
            tk == idltype.tk_longlong or tk == idltype.tk_ulonglong or \
            tk == idltype.tk_longdouble or tk == idltype.tk_wchar):
        out.ci_count = out.ci_count + 1
        ciname = '_ci%u' % out.ci_count
        out.out('size_t ' + ciname + ';')
        out.out('for (' + ciname + ' = 0; ' + ciname + ' < ' +\
                slength + '; ' + ciname + '++)')
        out.out('{')
        out.inc_indent()
        destroySimpleValue(out, type, sname + '[' + ciname + ']')
        out.dec_indent()
        out.out('}')
    out.out('delete [] ' + sname + ';')

def destroyCORBASequence(out, type, cname, fromCall=0):
    type = type.unalias().seqType()
    tk = type.kind()
    if not (tk == idltype.tk_null or tk == idltype.tk_void or \
            tk == idltype.tk_short or tk == idltype.tk_long or \
            tk == idltype.tk_ushort or tk == idltype.tk_ulong or \
            tk == idltype.tk_float or tk == idltype.tk_double or \
            tk == idltype.tk_boolean or tk == idltype.tk_char or \
            tk == idltype.tk_octet or tk == idltype.tk_enum or \
            tk == idltype.tk_longlong or tk == idltype.tk_ulonglong or \
            tk == idltype.tk_longdouble or tk == idltype.tk_wchar or \
            tk == idltype.tk_string or tk == idltype.tk_wstring):
        out.ci_count = out.ci_count + 1
        ciname = '_ci%u' % out.ci_count
        out.out('size_t ' + ciname + ';')
        if fromCall:
            out.out('for (' + ciname + ' = 0; ' + ciname + ' < ' +\
                    cname + '.length(); ' + ciname + '++)')
        else:
            out.out('for (' + ciname + ' = 0; ' + ciname + ' < ' +\
                    cname + '->length(); ' + ciname + '++)')
        out.out('{')
        out.inc_indent()
        destroyCORBAValue(out, type, cname + '[' + ciname + ']')
        out.dec_indent()
        out.out('}')
