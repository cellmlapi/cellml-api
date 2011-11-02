from omniidl import idltype
import string
import os.path

def GetTypeInformation(type, context):
    type = type.unalias()
    if isinstance(type, idltype.Base):
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
    arg_prefix=''
    has_length = 0
    def makePyArgStorage(self, pyargName, hasIn, hasOut):
        if hasOut:
            return "PyObject* %s;\n" % pyargName
        else:
            return "%s %s;" % (self.pyarg_c_type, pyargName)

    def pcmType(self, isOut=0, isRet=0):
        if isOut:
            return self.type_pcm + '*'
        elif not isRet:
            return self.arg_prefix + self.type_pcm
        else:
            return self.type_pcm
    


BASE_MAP = {
    idltype.tk_short: {'format_pyarg': 'h',
                       'pyarg_c_type': 'short',
                       'pyarg_cast_in': '',
                       'pyarg_cast_out': '',
                       'type_pcm': 'int16_t'},
    idltype.tk_long: {'format_pyarg': 'l',
                      'pyarg_c_type': 'long',
                      'pyarg_cast_in': '',
                      'pyarg_cast_out': '',
                      'type_pcm': 'int32_t'},
    idltype.tk_ushort: {'format_pyarg': 'H',
                       'pyarg_c_type': 'unsigned short',
                       'pyarg_cast_in': '',
                       'pyarg_cast_out': '',
                        'type_pcm': 'uint16_t'},
    idltype.tk_ulong: {'format_pyarg': 'k',
                       'pyarg_c_type': 'unsigned long',
                       'pyarg_cast_in': '',
                       'pyarg_cast_out': '',
                       'type_pcm': 'uint32_t'},
    idltype.tk_float: {'format_pyarg': 'f',
                       'pyarg_c_type': 'float',
                       'pyarg_cast_in': '',
                       'pyarg_cast_out': '',
                       'type_pcm': 'float'},
    idltype.tk_double: {'format_pyarg': 'd',
                        'pyarg_c_type': 'double',
                        'pyarg_cast_in': '',
                        'pyarg_cast_out': '',
                        'type_pcm': 'double'},
    idltype.tk_boolean: {'format_pyarg': 'i',
                         'pyarg_c_type': 'int',
                         'pyarg_cast_in': '(int)',
                         'pyarg_cast_out': '(bool)',
                         'type_pcm': 'bool'},
    idltype.tk_char: {'format_pyarg': 'c',
                      'pyarg_c_type': 'char',
                      'pyarg_cast_in': '',
                      'pyarg_cast_out': '',
                      'type_pcm': 'char'},
    idltype.tk_octet: {'format_pyarg': 'b',
                       'pyarg_c_type': 'unsigned char',
                       'pyarg_cast_in': '',
                       'pyarg_cast_out': '',
                       'type_pcm': 'uint8_t'},
    idltype.tk_ulonglong: {'format_pyarg': 'K',
                           'pyarg_c_type': 'unsigned long long',
                           'pyarg_cast_in': '',
                           'pyarg_cast_out': '',
                           'type_pcm': 'uint64_t'}
}

class Base(Type):
    def __init__(self, type):
        k = type.kind()
        info = BASE_MAP[k]
        self.__dict__.update(info)

    def makePyArgFromPCM(self, pyargName, pcmName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return 'return Py_BuildValue("%s", %s%s);' % (self.format_pyarg, self.pyarg_cast_in, pcmName)
            return 'PyBridge_Set_Output(%s, "%s", %s%s);' % (pyargName, self.format_pyarg, self.pyarg_cast_in, pcmName)
        elif hasIn and not hasOut:
            return '%s %s = %s%s;' % (self.pyarg_c_type, pyargName, self.pyarg_cast_in, pcmName)
        elif hasOut and not hasIn:
            return 'PyObject* %s = PyList_New(0);' % pyargName
        else:
            return "PyObject* %s;\n" % pyargName +\
                   "{\n" +\
                   "  %s = PyList_New(1);\n" % pyargName +\
                   "  PyList_SetItem(%s, 0, Py_BuildValue(\"%s\", %s*%s));\n" %\
                     (pyargName, self.format_pyarg, self.pyarg_cast_in, pcmName) +\
                   "}\n" +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)

    def makePCMFromPyarg(self, pcmName, pyargName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return '{ %s tmp; PyArg_Parse(%s, \"(%s)\", &tmp); return %stmp; }' %\
                       (self.type_pcm, pyargName, self.format_pyarg, self.pyarg_cast_out)
            return 'PyArg_Parse(%s, \"(%s)\", (%s*)%s);' % (pyargName, self.format_pyarg, self.pyarg_c_type, pcmName)
        if hasIn and not hasOut:
            return '%s %s = %s%s;' % (self.type_pcm, pcmName, self.pyarg_cast_out, pyargName)
        elif hasOut and not hasIn:
            return '%s %s;' % (self.type_pcm, pcmName)
        else:
            return "%s %s;\n" % (self.type_pcm, pcmName) +\
                   "{\n" +\
                   "  %s tmp;\n" % (self.pyarg_c_type) +\
                   "  PyArg_Parse(%s, \"(%s)\", &tmp);\n" % (pyargName, self.format_pyarg) +\
                   "  %s = %stmp;\n" % (pcmName, self.pyarg_cast_out) +\
                   "}\n"

class String(Type):
    format_pyarg = 'O'
    pyarg_c_type = 'PyObject*'
    type_pcm = 'std::string'
    arg_prefix = 'const '
    
    def __init__(self, type):
        pass

    def pcmType(self, isOut=0, isRet=0):
        if isOut:
            return 'std::string&'
        elif not isRet:
            return 'const std::string&'
        else:
            return 'std::string'

    def makePyArgFromPCM(self, pyargName, pcmName, hasIn, hasOut, copyOut=0):
        if copyOut:
            c = self.makePyArgFromPCM(pyargName + '_tmp', pcmName, 1, 0, 0) + "\n"
            if hasOut:
                return c + 'return Py_BuildValue("O", %s_tmp);' % (pyargName)
            return c + 'PyBridge_Set_Output(%s, "O", %s_tmp);' % (pyargName, pyargName)
        elif hasIn and not hasOut:
            return "PyObject* %s = PyString_FromString(%s.c_str());\n" % (pyargName, pcmName) +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        elif hasOut and not hasIn:
            return 'PyObject* %s = PyList_New(0);' % pyargName +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        else:
            return "PyObject* %s;\n" % pyargName +\
                   "{\n" +\
                   "  %s = PyList_New(1);\n" % pyargName +\
                   "  PyList_SetItem(%s, 0, PyString_FromString((*%s).c_str()));\n" % (pyargName, pcmName) +\
                   "}\n" +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)

    def makePCMFromPyarg(self, pcmName, pyargName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return "char* _retpcm = PyString_AsString(%s);\n" % (pyargName) +\
                       "return _retpcm ? _retpcm : \"\";"
            return "{\nchar* _tmp = PyString_AsString(%s);\n" % (pyargName) +\
                   "%s = _retpcm ? _tmp : \"\"; \n}" % (pcmName)
        if hasIn and not hasOut:
            return "char* %s_tmp = PyString_AsString(%s);\n" % (pcmName, pyargName) +\
                   "std::string %s = %s_tmp ? %s_tmp : \"\";\n" % (pcmName, pcmName, pcmName)
        elif hasOut and not hasIn:
            return "std::string %s;" % (pcmName)
        else:
            return "std::string %s;\n" % pcmName +\
                   "{\n" +\
                   "  PyObject* stmp = PyList_GetItem(%s, 0);\n" % pyargName +\
                   "  char* _tmp_%s = stmp ? PyString_AsString(stmp) : \"\";\n" % pcmName +\
                   "  %s = %s ? %s : \"\";\n" % (pcmName, pcmName, pcmName) +\
                   "}"

class WString(Type):
    format_pyarg = 'O'
    pyarg_c_type = 'PyObject*'
    type_pcm = 'std::wstring'
    arg_prefix = 'const '
    
    def __init__(self, type):
        pass

    def pcmType(self, isOut=0, isRet=0):
        if isOut:
            return 'std::wstring&'
        elif not isRet:
            return 'const std::wstring&'
        else:
            return 'std::wstring'

    def makePyArgFromPCM(self, pyargName, pcmName, hasIn, hasOut, copyOut=0):
        if copyOut:
            c = self.makePyArgFromPCM(pyargName + '_tmp', pcmName, 1, 0, 0) + "\n"
            if hasOut:
                return c + 'return Py_BuildValue("O", %s_tmp);' % (pyargName)
            return c + 'PyBridge_Set_Output(%s, "O", %s_tmp);' % (pyargName, pyargName)
        elif hasIn and not hasOut:
            return "PyObject* %s;\n" % pyargName +\
                   "{\n" +\
                   "  size_t ltmp = %s.size() * 4 + 1;\n" % pcmName +\
                   "  char* stmp = new char[ltmp];\n" +\
                   "  wcstombs(stmp, %s.c_str(), ltmp);\n" % pcmName +\
                   "  %s = PyString_FromString(stmp);\n" % pyargName +\
                   "  delete [] stmp;\n" +\
                   "}\n" +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        elif hasOut and not hasIn:
            return "PyObject* %s = PyList_New(0);\n" % pyargName +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        else:
            return "PyObject* %s;\n" % pyargName +\
                   "{\n" +\
                   "  size_t ltmp = (*%s).size() * 4 + 1;\n" % pcmName +\
                   "  char* stmp = new char[ltmp];\n" +\
                   "  wcstombs(stmp, (*%s).c_str(), ltmp);\n" % pcmName +\
                   "  %s = PyList_New(1);\n" % pyargName +\
                   "  PyList_SetItem(%s, 0, PyString_FromString(stmp));\n" % pyargName +\
                   "  delete [] stmp;\n" +\
                   "}\n" +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)

    def makePCMFromPyarg(self, pcmName, pyargName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return "{\n" +\
                       "  std::wstring _retpcm;\n" +\
                       "  char* stmp = PyString_AsString(%s);\n" % pyargName +\
                       "  if (stmp == NULL) _retpcm = L\"\";\n" +\
                       "  else\n" +\
                       "  {\n" +\
                       "    size_t ltmp = strlen(stmp);\n" +\
                       "    wchar_t* _retpcm_tmp = (wchar_t*)malloc(sizeof(wchar_t) * (ltmp + 1));\n" +\
                       "    mbstowcs(_retpcm_tmp, stmp, ltmp);\n" +\
                       "    _retpcm_tmp[ltmp] = 0;\n" +\
                       "    _retpcm = _retpcm_tmp;\n" +\
                       "    free(_retpcm_tmp);\n" +\
                       "  }\n" +\
                       "  return _retpcm;\n" +\
                       "}\n"
            return "{\n" +\
                   "  wchar_t* _tmp;\n" +\
                   "  char* stmp = PyString_AsString(%s);\n" % pyargName +\
                   "  if (stmp == NULL) _retpcm = CDA_wcsdup(L\"\");\n" +\
                   "  else\n" +\
                   "  {\n" +\
                   "    size_t ltmp = strlen(stmp);\n" +\
                   "    _tmp = (wchar_t*)malloc(sizeof(wchar_t) * (ltmp + 1));\n" +\
                   "    mbstowcs(_retpcm, stmp, ltmp);\n" +\
                   "    _tmp[ltmp] = 0;\n" +\
                   "  }\n" +\
                   "  %s = _tmp;\n" % pcmName +\
                   "  free(_tmp);\n" +\
                   "}\n"
        if hasIn and not hasOut:
            return "std::wstring %s;\n" % pcmName +\
                   "{\n" +\
                   "  char* stmp = PyString_AsString(%s);\n" % pyargName +\
                   "  if (stmp == NULL) %s = L\"\";\n" % pcmName +\
                   "  else\n" +\
                   "  {\n" +\
                   "    size_t ltmp = strlen(stmp);\n" +\
                   "    wchar_t* %s_tmp = (wchar_t*)malloc(sizeof(wchar_t) * (ltmp + 1));\n" % pcmName +\
                   "    mbstowcs(%s_tmp, stmp, ltmp);\n" % pcmName +\
                   "    %s_tmp[ltmp] = 0;\n" % (pcmName) +\
                   "    %s = %s_tmp;\n" % (pcmName, pcmName) +\
                   "    free(%s_tmp);\n" % pcmName +\
                   "  }\n" +\
                   "}\n"
        elif hasOut and not hasIn:
            return "std::wstring %s;" % pcmName
        else:
            return "std::wstring %s;\n" % pcmName +\
                   "{\n" +\
                   "  PyObject* stmp = PyList_GetItem(%s, 0);\n" % pyargName +\
                   "  char* strtmp = PyString_AsString(stmp);\n" +\
                   "  if (strtmp == NULL) %s = CDA_wcsdup(L\"\");\n" % pcmName +\
                   "  else\n" +\
                   "  {\n" +\
                   "    size_t ltmp = strlen(strtmp);\n" +\
                   "    wchar_t* %s_tmp = (wchar_t*)malloc(sizeof(wchar_t) * (ltmp + 1));\n" % pcmName +\
                   "    mbstowcs(%s_tmp, strtmp, ltmp);\n" % pcmName +\
                   "    %s_tmp[ltmp] = 0;\n" % pcmName +\
                   "    %s = %s_tmp;\n" % pcmName +\
                   "    free(%s_tmp);\n" % pcmName +\
                   "  }\n" +\
                   "}"

class Sequence(Type):
    format_pyarg = 'O'
    pyarg_c_type = 'PyObject*'
    has_length = 1
    
    def __init__(self, type, context):
        self.superti = GetTypeInformation(type.seqType(), context)
        self.type_pcm = 'std::vector<' + self.superti.type_pcm + '>'

    def pcmType(self, isOut=0, isRet=0):
        if isOut:
            return self.type_pcm + '&'
        elif not isRet:
            return 'const ' + self.type_pcm + '&'
        else:
            return self.type_pcm

    def makePyArgFromPCM(self, pyargName, pcmName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return "{\n" +\
                       makePyArgFromPCM('_retpy', pcmName, 1, 0, 0) + "\n" +\
                       "  Py_INCREF(_retpy);" +\
                       "  return _retpy;\n" +\
                       "}"
            else:
                return "{\n" +\
                       makePyArgFromPCM('_outpy', pcmName, 1, 0, 0) + "\n" +\
                       "  Py_INCREF(_outpy);" +\
                       "  %s = _outpy;\n" % pyargName +\
                       "}"
        if hasIn and not hasOut:
            if self.superti.format_pyarg == 'O':
                # Already an object...
                maybeo = 'o'
                doconvert = ''
            else:
                maybeo = ''
                doconvert = "PyObject* _tmpo = Py_BuildValue(\"%s\", _tmp);\n" % self.superti.format_pyarg +\
                            "ScopedPyObjectReference _tmpo_release(_tmpo);\n"
                
            return "PyObject* %s = PyList_New(%s.size());\n" % (pyargName, pcmName) +\
                   "for (uint32_t _i = 0; _i < %s.size(); _i++)\n" % (pcmName) +\
                   "{\n" +\
                   self.superti.makePyArgFromPCM("_tmp%s" % maybeo, '(%s[_i])' % pcmName, 1, 0) +\
                   doconvert +\
                   "  Py_INCREF(_tmpo);\n" +\
                   "  PyList_SET_ITEM(%s, _i, _tmpo);\n" % pyargName +\
                   "}"
        elif hasOut and not hasIn:
            return "PyObject* %s = PyList_New(0);\n" % pyargName +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        else:
            # This does raise the question of whether we should use a double list like we do
            # here. It is strictly speaking unnecessary, but it makes the interface more
            # consistent.
            return "PyObject* %s = PyList_New(1);\n" % pyargName +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName) +\
                   "{\n" + self.makePyArgFromPCM("_member", pcmName, 1, 0) +\
                   "  PyList_SET_ITEM(%s, 0, _member);\n" % pyargName +\
                   "}"
    
    def makePCMFromPyarg(self, pcmName, pyargName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return self.makePCMFromPyarg('_retpcm', pyargName, 1, 0, 0) +\
                       "_retpcm->add_ref();\n" +\
                       "return _retpcm;" +\
                       "}"
            return "{\n" +\
                   self.makePCMFromPyArg('_outpcm', pyargName, 1, 0, 0) +\
                   "  *%s = _outpcm;\n" % pcmName +\
                   "}"
        
        if hasIn and not hasOut:
            return "uint32_t _length_%s = PyList_Size(%s);\n" % (pcmName, pyargName) +\
                   "std::vector<%s> %s;" % (self.superti.type_pcm, pcmName) +\
                   "for (uint32_t _i = 0; _i < %s.size(); _i++)" % pcmName +\
                   "{\n" +\
                   "  PyObject* _pytmp = PyList_GetItem(%s, _i);\n" % pyargName +\
                   self.superti.makePCMFromPyarg("_add_tmp", "_pytmp", 1, 1, 0) +\
                   "  %s.push_back(_add_tmp);\n" % pcmName +\
                   "}"
        elif hasOut and not hasIn:
            return "%s %s;" % (self.type_pcm, pcmName)
        else:
            return "{\n" +\
                   "  PyObject* stmp = PyList_GetItem(%s, 0);\n" % pyargName +\
                   "  *_length_%s = stmp ? PyList_Size(stmp) : 0;\n" % (pcmName) +\
                   "  if (*%s) delete [] %s;" % (pcmName, pcmName) +\
                   "  %s.clear();" % pcmName +\
                   "  for (uint32_t _i = 0; _i < _length_%s; _i++)" % pcmName +\
                   "  {\n" +\
                   "    PyObject* _pytmp = PyList_GetItem(%s, _i);\n" % pyargName +\
                   self.superti.makePCMFromPyarg("_add_tmp" % pcmName, "_pytmp", 1, 1, 0) +\
                   "  %s.push_back(_add_tmp);\n" % pcmName +\
                   "  }\n" +\
                   "}"

class Declared(Type):
    def __init__(self, type, context):
        pass

class Struct(Declared):
    def __init__(self, type, context):
        raise "Sequence type encountered - but we don't support sequence types."

class Objref(Declared):
    format_pyarg = 'O'
    pyarg_c_type = 'PyObject*'

    def __init__(self, type, context):
        self.context = context
        self.type_pcm = type.decl().simplecxxscoped + '*'
        self.base_type_pcm = type.decl().simplecxxscoped
        self.iface_flat = type.decl().simplecscoped
        self.scopedname = type.decl().corbacxxscoped
        directory, filename = os.path.split(type.decl().file())
        filebase, extension = os.path.splitext(filename)
        self.getType = "PyObject* typeMod = PyImport_ImportModule(\"%s.%s\");\n" % \
                       (self.context.moduledir, filebase) +\
                       "PyObject* type = typeMod ? PyObject_GetAttrString(typeMod, \"%s\") : NULL;\n"\
                         % type.decl().identifier() +\
                       'if (typeMod) Py_DECREF(typeMod);'

    def pcmType(self, isOut=0, isRet=0):
        if isOut:
            return self.type_pcm + '*'
        elif not isRet:
            return self.type_pcm
        else:
            if isRet == 2:
                return 'already_AddRefd<' + self.base_type_pcm + '>'

            return self.type_pcm

    def makePyArgFromPCM(self, pyargName, pcmName, hasIn, hasOut, copyOut=0):
        if copyOut:
            c = self.makePyArgFromPCM(pyargName + '_tmp', pcmName, 1, 0, 0) + "\n"
            if hasOut:
                return c + 'Py_INCREF(%s_tmp); return %s_tmp;' % (pyargName, pyargName)
            return c + 'PyBridge_Set_Output(%s, "O", %s_tmp);' % (pyargName, pyargName)
        elif hasIn and not hasOut:
            return "PyObject* %s;\n" % pyargName +\
                   "{\n" +\
                   "  ::p2py::XPCOM::IObject *_wrapper = dynamic_cast< ::p2py::XPCOM::IObject*>(%s);\n" % pcmName +\
                   "  if (_wrapper != NULL) %s = _wrapper->unwrap();\n" % (pyargName) +\
                   "  else if (%s == NULL) { %s = Py_None; Py_INCREF(Py_None);}\n" % (pcmName, pyargName) +\
                   "  else\n" +\
                   "  {\n" +\
                   self.getType +\
                   "    PyObject* cobj = PyCObject_FromVoidPtr(reinterpret_cast<void*>(static_cast<iface::XPCOM::IObject*>(%s)), NULL);\n" % pcmName +\
                   "    %s = PyObject_CallFunctionObjArgs(type, cobj, NULL);\n" % pyargName +\
                   "    Py_DECREF(cobj);\n" +\
                   "    Py_DECREF(type);\n" +\
                   "  }\n" +\
                   "}\n" +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        elif hasOut and not hasIn:
            return "PyObject* %s = PyList_New(0);\n" % pyargName +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
        else:
            return "PyObject* %s;\n" % pyargName +\
                   "{\n" +\
                   "  %s = PyList_New(1);\n" % pyargName +\
                   "  ::p2py::XPCOM::IObject *_wrapper = dynamic_cast< ::p2py::XPCOM::IObject*>(%s);\n" % pcmName +\
                   "  PyObject* _item;\n" +\
                   "  if (_wrapper != NULL) _item = _wrapper->unwrap();\n" +\
                   "  else if (%s == NULL) { _item = Py_None; Py_INCREF(Py_None); }\n" % pcmName +\
                   "  else\n" +\
                   "  {\n" +\
                   self.getType +\
                   "    PyObject* cobj = PyCObject_FromVoidPtr(reinterpret_cast<void*>(static_cast<iface::XPCOM::IObject*>(*%s)), NULL);\n" % pcmName +\
                   "    if (%s) (*%s)->add_ref();\n" % (pcmName, pcmName) +\
                   "    _item = PyObject_CallFunctionObjArgs(type, cobj, NULL);\n" +\
                   "    Py_DECREF(cobj);\n" +\
                   "    Py_DECREF(type);\n" +\
                   "  }\n" +\
                   "  PyList_SetItem(%s, 0, _item);\n" % pyargName +\
                   "}\n" +\
                   "ScopedPyObjectReference %sScoped(%s);" % (pyargName, pyargName)
    
    def makePCMFromPyarg(self, pcmName, pyargName, hasIn, hasOut, copyOut=0):
        if copyOut:
            if hasOut:
                return "{\n" + self.makePCMFromPyarg('_retpcm', pyargName, 1, 0, 0) +\
                       "if (_retpcm) _retpcm->add_ref();\n" +\
                       "return _retpcm;\n" +\
                       "}"
            return "{\n" +\
                   self.makePCMFromPyArg('_outpcm', pyargName, 1, 0, 0) +\
                   "  if (_outpcm) _outpcm->add_ref();\n" +\
                   "  *%s = _outpcm;\n" % pcmName +\
                   "}"
        
        if hasIn and not hasOut:
            return "%s %s;\n" % (self.type_pcm, pcmName) +\
                   "if (%s == Py_None)\n" % pyargName +\
                   "{\n" +\
                   "  %s = NULL;\n" % pcmName +\
                   "}\n" +\
                   "{\n" +\
                   "  PyObject* cptr = PyObject_GetAttrString(%s, \"_iobject_%s_cptr\");\n" %\
                   (pyargName, self.iface_flat) +\
                   "  if (cptr == NULL)" +\
                   "  {\n" +\
                   "    PyErr_Clear();\n" +\
                   "    %s = new ::p2py::%s(%s);" % (pcmName, self.scopedname, pyargName) +\
                   "  }\n" +\
                   "  else\n" +\
                   "  {\n" +\
                   "    %s = reinterpret_cast<%s>(PyCObject_AsVoidPtr(cptr));\n" % (pcmName, self.type_pcm) +\
                   "    if (%s != NULL) %s->add_ref();" % (pcmName, pcmName) +\
                   "    Py_DECREF(cptr);\n" +\
                   "  }\n" +\
                   "}\n" +\
                   "ObjRef<%s> %s_release = already_AddRefd<%s>(%s);" % (self.base_type_pcm, pcmName, self.base_type_pcm, pcmName)
        elif hasOut and not hasIn:
            return "%s %s;\nPyOutputIObjectRelease<%s> %s_release(&%s);" % (self.type_pcm, pcmName, self.base_type_pcm, pcmName, pcmName)
        else:
            return "%s %s;\n" % (self.type_pcm, pcmName) +\
                   "{\n" +\
                   "  PyObject* stmp = PyList_GetItem(%s, 0);\n" % pyargName +\
                   "  if (stmp == Py_None)\n" +\
                   "  {\n" +\
                   "    *%s = NULL;\n" +\
                   "  }\n" +\
                   "  } else {\n" +\
                   "    PyObject* cptr = PyObject_GetAttrString(stmp, \"_iobject_%s_cptr\");\n" %\
                   node.simplecscoped +\
                   "    if (cptr == NULL)\n" +\
                   "    {\n" +\
                   "      *%s = new ::p2py::%s(stmp);" % (pcmName, self.scopedname) +\
                   "    }\n" +\
                   "    else\n" +\
                   "    {\n" +\
                   "      *%s = reinterpret_cast<%s>(PyCObject_AsVoidPtr(cptr));\n" % (pcmName, self.type_pcm) +\
                   "      if (*%s != NULL) *%s->add_ref();\n" % (pcmName, pcmName) +\
                   "      Py_DECREF(cptr);\n" +\
                   "    }\n" +\
                   "    Py_DECREF(stmp);\n" +\
                   "  }\n" +\
                   "}\n" +\
                   "ObjRef<%s> %s_release = already_AddRefd<%s>(*%s);" % (self.type_pcm, pcmName, self.type_pcm, pcmName)

# Not actually a base type, but we mixin Base since it has all aspects in common.
class Enum(Declared, Base):
    format_pyarg = 'k'
    pyarg_c_type = 'unsigned long'
    pyarg_cast_in = '(unsigned long)'

    def __init__(self, omniidl_type):
        self.pyarg_cast_out = '(%s)' % omniidl_type.decl().simplecxxscoped
        self.type_pcm = omniidl_type.decl().simplecxxscoped
