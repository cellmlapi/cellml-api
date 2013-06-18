import os
from omniidl import idlvisitor, output, idlast
import jnutils
import string

class InterfaceVisitor (idlvisitor.AstVisitor):
    def __init__(self):
        self.directoryParts = []
        self.directory = ''
        self.package = ''
        self.out = None

    def calculateDirectory(self):
        self.directory = string.join(self.directoryParts, '/')
        self.package = string.join(self.directoryParts, '.')
        
    def visitAST(self, node):
        for declaration in node.declarations():
            declaration.accept(self)
            
    def visitModule(self, node):
        try:
            os.mkdir(node.identifier())
        except OSError:
            pass
        self.directoryParts.append(node.identifier())
        self.calculateDirectory()
        for defn in node.definitions():
            defn.accept(self)
        self.directoryParts.pop()
        self.calculateDirectory()
        
    def visitInterface(self, node):
        if not node.mainFile():
            return
        self.out = output.Stream(open(self.directory + '/' + jnutils.JavaName(node) + ".java", 'w'))
        self.out.out('package ' + self.package + ';')
        self.out.out('public interface ' + jnutils.JavaName(node))
        inh = node.inherits()
        extends = ''
        if len(inh) != 0:
            for inherit in inh:
                if isinstance(inherit, idlast.Declarator) and inherit.alias():
                    inherit = inherit.alias().aliasType().unalias().decl()                
                if inherit.scopedName() == ['XPCOM', 'IObject']:
                    continue
                if extends == '':
                    extends = 'extends ' + jnutils.GetClassName(inherit)
                else:
                    extends = extends + ", " + jnutils.GetClassName(inherit)
        self.out.out('  ' + extends)
        self.out.out('{')
        self.out.inc_indent()
        for i in node.contents():
            i.accept(self)
        self.out.dec_indent()
        self.out.out('};')
        self.out = None

    def visitAttribute(self, node):
        ti = jnutils.GetTypeInformation(node.attrType().unalias())
        for n in node.declarators():
            if not node.readonly():
                self.writeSetter(n, ti)
            self.writeGetter(n, ti)

    def writeSetter(self, node, ti):
        self.out.out('public void ' + jnutils.AccessorName(node, 1) + '(' + ti.javaType(jnutils.Type.IN) + ' arg);')

    def writeGetter(self, node, ti):
        self.out.out('public ' + ti.javaType(jnutils.Type.RETURN) + ' ' + jnutils.AccessorName(node, 0) + '();')
    
    def visitOperation(self, node):
        paramsig = ''
        for p in node.parameters():
            direction = [jnutils.Type.IN, jnutils.Type.OUT, jnutils.Type.INOUT][p.direction()]
            v = jnutils.GetTypeInformation(p.paramType().unalias()).\
                  javaType(direction) + ' ' +\
                  jnutils.JavaName(p)
            if paramsig != '':
                paramsig = paramsig + ', '
            paramsig = paramsig + v
        
        rti = jnutils.GetTypeInformation(node.returnType().unalias())
        self.out.out('public ' + rti.javaType(jnutils.Type.RETURN) + ' ' +
                     jnutils.JavaName(node) + '(' + paramsig + ');')

    def visitEnum(self, node):
        setupOut = 0
        if self.out == None:
            if not node.mainFile():
                return
            self.out = output.Stream(open(self.directory + '/' + jnutils.JavaName(node) + ".java", 'w'))
            self.out.out('package ' + self.directory + ';')
            setupOut = 1

        self.firstEnumerator = 1
        self.out.out('public enum ' + jnutils.JavaName(node) + '{')
        self.out.inc_indent()
        for e in node.enumerators():
            e.accept(self)
        self.out.dec_indent()
        self.out.out('};')
        
        if setupOut:
            self.out = None

    def visitEnumerator(self, node):
        prefix = ''
        if not self.firstEnumerator:
            prefix = ','
        else:
            self.firstEnumerator = 0
        self.out.out(prefix + jnutils.JavaName(node))

    def visitMember(self, node):
        alln = ''
        needcomma = 0
        for n in node.declarators():
            if needcomma:
                alln = alln + ', '
            needcomma = 1
            alln = alln + n.simplename
            sa = n.sizes()
            if sa != None:
                for s in sa:
                    alln = alln + '[%u]'%s
        self.out.out('public ' +\
                         jnutils.GetTypeInformation(node.memberType()).javaType(jnutils.Type.IN) +\
                         ' ' + alln + ';')

    def visitException(self, node):
        setupOut = 0
        if self.out == None:
            if not node.mainFile():
                return
            self.out = output.Stream(open(self.directory + '/' + jnutils.JavaName(node) + ".java", 'w'))
            self.out.out('package ' + self.directory + ';')
            setupOut = 1

        self.out.out('public class ' + jnutils.JavaName(node) + ' extends RuntimeException')
        self.out.out('{')
        self.out.inc_indent()

        constructorArgs = ''
        constructorSave = ''
        for n in node.members():
            if constructorArgs != '':
                constructorArgs = constructorArgs + ', '
            for dn in n.declarators():
                constructorSave = constructorSave + ('%s = _%s;' % (jnutils.JavaName(dn), jnutils.JavaName(dn)))
                
                constructorArgs = \
                    constructorArgs + \
                    jnutils.GetTypeInformation(n.memberType()).javaType(jnutils.Type.IN) +\
                    ' _' + jnutils.JavaName(dn)
                if dn.sizes() != None:
                    constructorArgs = constructorArgs + ''.join(map(lambda x: '[%s]'%x, dn.sizes()))

        self.out.out('  public ' + node.simplename + '(' + constructorArgs + '){ ' + constructorSave + ' }')
        for n in node.members():
            n.accept(self)
        self.out.dec_indent()
        self.out.out('};')

        if setupOut:
            self.out = None

def run(tree):
    iv = InterfaceVisitor()
    tree.accept(iv)
