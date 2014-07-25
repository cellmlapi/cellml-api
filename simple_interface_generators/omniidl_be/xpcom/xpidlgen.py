from omniidl import idlast, idlvisitor, idlutil, idltype, output
import os, string, md5
import typeinfo

REPOID_ISUPPORTS = 'DCE:00000000-0000-0000-c000-000000000046:1'

class XPIDLVisitor(idlvisitor.AstVisitor):
    def __init__(self):
        pass

    def visitAST(self, node):
        directory, basename = os.path.split(node.file())
        self._included = ['xpcom.idl']
        if string.lower(basename[-4:]) == '.idl':
            basename = basename[:-4]

        self._idl = output.Stream(open('I' + basename + '.idl', 'w'), 2)
        gbasename = '';
        for i in basename:
            if (i >= 'A' and i <= 'Z') or (i >= 'a' and i <= 'z'):
                gbasename = gbasename + i
        self._idl.out(
            "// This output is automatically generated. Do not edit.")
        guardname='XPIDL__' + string.upper(gbasename) + '__INCLUDED'
        self._idl.out("#ifndef " + guardname)
        self._idl.out("#define " + guardname)
        self._idl.out('#include "nsISupports.idl"')
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

                    if filename != "xpcom.idl":
                        self._idl.out('#include "I@filename@"\n',
                                      filename=filename)
        self._idl.out('#endif // not ' + guardname)
        self._idl = None

    def visitModule(self, node):
        for n in node.definitions():
            n.accept(self)

    def visitForward(self, node):
        self._idl.out('interface ' + node.xpcomscoped + ';')

    def visitInterface(self, node):
        idlname = node.identifier()
        if node.repoId()[:4] == 'DCE:':
            uuid = string.split(node.repoId()[4:], ':')[0]
        else:
            uuid = self.computeHexDigest(node.repoId())
            uuid = uuid[:8] + '-' + uuid[8:12] + '-' + uuid[12:16] + '-' +\
                   uuid[16:20] + '-' + uuid[20:]

        # There is no multiple inheritance in XPIDL. Therefore, in cases where
        # the IDL uses it, always only use the first interface...
        inh = filter(lambda x: x.repoId() != REPOID_ISUPPORTS, node.inherits())
        if len(inh) == 0:
            inherit = 'nsISupports'
        else:
            inherit = inh[0].xpcomscoped
        self._idl.out('[scriptable, uuid(@uuid@)]', uuid=uuid)
        self._idl.out('interface @intfname@', intfname=node.xpcomscoped)
        self._idl.out('  : @supername@', supername=inherit)
        self._idl.out('{')
        self._idl.inc_indent()
        for c in node.contents():
            c.accept(self)
        self._idl.dec_indent()
        self._idl.out('};')

    def visitTypedef(self, node):
        if not isinstance(node.aliasType().unalias(), idltype.Sequence):
            ti = typeinfo.GetTypeInformation(node.aliasType().unalias())

            for d in node.declarators():
                # Treated specially by XPIDL due to nsrootidl.idl
                if (d.identifier() == 'DOMString'):
                    continue
                # If the alias type has the same name as the aliases, we just
                # ignore the typedef. This allows typedefs designed to forward
                # definitions between modules to work in XPIDL (which doesn't
                # have the concept of modules).
                if isinstance(node.aliasType(), idltype.Declared):
                    if d.identifier() == node.aliasType().scopedName()[-1]:
                        continue
                self._idl.out('typedef @alias_type@ @name@;',
                              alias_type=ti.type_xpidl, name=d.identifier())

    def visitConst(self, node):
        scoped_name = node.scopedName()[:-1]

        if len(scoped_name) and isinstance(idlast.findDecl(scoped_name),
                                           idlast.Interface):
            name = node.identifier()

            ti = typeinfo.GetTypeInformation(node.constType().unalias())

            cleaned_value = str(node.value())
            if cleaned_value[-1] == 'L':
                cleaned_value = cleaned_value[0:-1]

            self._idl.out("const @type@ @name@ = @value@;",
                          type=ti.type_xpidl, name=name,
                          value=cleaned_value)

    def visitEnum(self, node):
        scoped_name = node.scopedName()[:-1]
        if len(scoped_name) and isinstance(idlast.findDecl(scoped_name),
                                           idlast.Interface):
            value = 0

            for enum in node.enumerators():
                name = enum.identifier()
                self._idl.out("const unsigned long @name@ = @value@;",
                              name=name, value=str(value))
                value = value + 1

    def visitAttribute(self, node):
        ti = typeinfo.GetTypeInformation(node.attrType().unalias())
        for identifier in node.identifiers():
            if node.readonly():
                readonly = 'readonly '
            else:
                readonly = ''
            self._idl.out('@readonly@attribute @type@ @name@;',
                          readonly=readonly, type=ti.type_xpidl,
                          name=identifier)

    def visitOperation(self, node):
        name = node.identifier()
        paramspec = []
        for p in node.parameters():
            ti = typeinfo.GetTypeInformation(p.paramType().unalias())
            paramspec.append(ti.XPIDLArgument(p.identifier(), p.is_in(),
                                              p.is_out()))
        rettype = node.returnType().unalias()
        if not (isinstance(rettype, idltype.Base) and
                (rettype.kind() == idltype.tk_void)):
            ti = typeinfo.GetTypeInformation(rettype)
            if not(isinstance(rettype, idltype.Sequence)):
                retval = ti.type_xpidl
            else:
                # Sequences can't be returned in XPCOM...
                retval = 'void'
                paramspec.append(ti.XPIDLArgument(retval.identifier(), 0, 1))
        else:
            retval = 'void'
        self._idl.out('@retval@ @name@(@paramspec@);',
                      retval=retval, name=name,
                      paramspec=string.join(paramspec, ', '))

    def computeHexDigest(self, inputString):
        try:
           return md5.md5(inputString).hexdigest()
        except AttributeError:
           digest = '';
           for char in md5.md5(inputString).digest():
              digest = digest + ('%02x' % ord(char))
           return digest

def run(tree):
    tree.accept(XPIDLVisitor())
