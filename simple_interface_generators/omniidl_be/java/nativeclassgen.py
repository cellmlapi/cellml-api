import os
from omniidl import idlvisitor, output, idlast
import jnutils
import string

class NativeClassVisitor (idlvisitor.AstVisitor):
    def __init__(self):
        self.directoryParts = ['pjm2pcm']
        self.directory = ''
        self.package = ''
        try:
            os.mkdir('pjm2pcm')
        except OSError:
            pass

    def calculateDirectory(self):
        self.directory = string.join(self.directoryParts, '/')
        self.package = string.join(self.directoryParts, '.')
        
    def visitAST(self, node):
        for declaration in node.declarations():
            declaration.accept(self)
            
    def visitModule(self, node):
        try:
            os.mkdir(self.directory + '/' + node.identifier())
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
        self.out.out('public class ' + jnutils.JavaName(node))
        self.out.out('  implements ' + jnutils.GetClassName(node))
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('private ' + jnutils.JavaName(node) + '() {};')
        self.out.out('protected native void finalize();')
        self.out.out('public int compareTo(Object obj)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('try')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('long v1 = fetchIObject();')
        self.out.out('long v2 = ((pjm.XPCOMDerived)obj).fetchIObject();')
        self.out.out('if (v1 == v2) return 0;')
        self.out.out('if (v1 > v2) return 1;')
        self.out.out('return -1;')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('catch (ClassCastException cce) { return -1; }')
        self.out.dec_indent()
        self.out.out('}')
        
        self.out.out('public boolean equals(Object obj)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('try')
        self.out.out('{')
        self.out.out('long v1 = fetchIObject();')
        self.out.out('long v2 = ((pjm.XPCOMDerived)obj).fetchIObject();')
        self.out.out('return (v1 == v2);')
        self.out.out('}')
        self.out.out('catch (ClassCastException cce) { return false; }')
        self.out.dec_indent()
        self.out.out('}')
        
        self.out.out('public native int hashCode();')
        self.recurseAcceptInheritedContents(node)
        self.out.out('private long nativePtr;')
        self.out.out('private long nativePtr_xpcom_iobject;')
        self.out.out('public long fetchIObject() { return nativePtr_xpcom_iobject; }')
        self.out.out('public static ' + jnutils.JavaName(node) + ' queryInterface(pjm.XPCOMDerived obj)')
        self.out.out('{')
        self.out.inc_indent()
        self.out.out('return nqueryInterface(obj.fetchIObject());')
        self.out.dec_indent()
        self.out.out('}')
        self.out.out('private static native ' + jnutils.JavaName(node) + ' nqueryInterface(long obj);')
        self.out.dec_indent()
        self.out.out('};')
        self.out = None

    def recurseAcceptInheritedContents(self, node):
        if isinstance(node, idlast.Declarator) and node.alias():
            node = node.alias().aliasType().unalias().decl()
        
        self.out.out('private long nativePtr_' + string.join(node.scopedName(), '_') + ';')
        for i in node.contents():
            i.accept(self)
        for i in node.inherits():
            if i.scopedName() != ['XPCOM', 'IObject']:
                self.recurseAcceptInheritedContents(i)
        
    def visitAttribute(self, node):
        ti = jnutils.GetTypeInformation(node.attrType().unalias())
        for n in node.declarators():
            if not node.readonly():
                self.writeSetter(n, ti)
            self.writeGetter(n, ti)

    def writeSetter(self, node, ti):
        self.out.out('public native void ' + jnutils.AccessorName(node, 1) + '(' + ti.javaType(jnutils.Type.IN) + ' arg);')

    def writeGetter(self, node, ti):
        self.out.out('public native ' + ti.javaType(jnutils.Type.RETURN) + ' ' + jnutils.AccessorName(node, 0) + '();')
    
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
        self.out.out('public native ' + rti.javaType(jnutils.Type.RETURN) + ' ' +
                     jnutils.JavaName(node) + '(' + paramsig + ');')

def run(tree):
    iv = NativeClassVisitor()
    tree.accept(iv)
