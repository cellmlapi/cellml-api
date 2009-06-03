import os
from omniidl import idlvisitor, output
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
        extends = 'extends Comparable<Object>'
        if len(inh) != 0:
            for inherit in inh:
                if inherit.scopedName() == ['XPCOM', 'IObject']:
                    continue
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

def run(tree):
    iv = InterfaceVisitor()
    tree.accept(iv)
