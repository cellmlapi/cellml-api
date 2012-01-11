# -*- python -*-
reservedWords = [
    "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
    "case", "catch", "char", "class", "compl", "const", "const_cast",
    "continue", "default", "delete", "do", "double", "dynamic_cast",
    "else", "enum", "explicit", "export", "extern", "false", "float",
    "for", "friend", "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "not", "not_eq", "operator", "or", "or_eq",
    "private", "protected", "public", "register", "reinterpret_cast",
    "return", "short", "signed", "sizeof", "static", "static_cast",
    "struct", "switch", "template", "this", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq" ]

reservedDict = {}
for i in reservedWords:
    reservedDict[i] = 1

import string

def EscapeCXX(name):
    """ Escape a string to avoid a reserved word. Used for all mappings. """
    global reservedDict
    if reservedDict.has_key(name):
        return "_cxx_" + name
    return name

def AnnotateByRepoID(node, skiplastscope=0):
    sn = map(EscapeCXX, node.scopedName())
    node.simplename = sn[-1]
    # This is used, e.g. for enums, where the enum scope isn't part of the C++
    # scoped name.
    if skiplastscope:
        sn = sn[:-1]
        sn.append(node.simplename)
    node.simplecscoped = string.join(sn, '_')
    node.corbacxxscoped = string.join(sn, '::')
    node.finalcciscoped = string.join(sn[:-1], '::') + '::_final_' + sn[-1]
    node.factoryscoped = string.join(sn[:-1], '::') + '::_factory_' + sn[-1]
    node.simplecxxscoped = 'iface::' + node.corbacxxscoped
    node.lengthprefixed = ''
    for part in sn:
        node.lengthprefixed = node.lengthprefixed + ('%d%s' % (len(part), part))

from omniidl import idlvisitor

class Annotator(idlvisitor.AstVisitor):
    """Walks over the AST once and annotates all the nodes.
    Will also be on the lookout for anything we don't support.
    """

    def visitAST(self, node):
        """Visit all the declarations in an AST"""
        for n in node.declarations():
            n.accept(self)
    
    def visitModule(self, node):
        """Visit all the definitions in a module."""
        AnnotateByRepoID(node)
        for n in node.definitions():
            n.accept(self)
        
    def visitInterface(self, node):
        AnnotateByRepoID(node)
        for n in node.contents():
            n.accept(self)

    def visitForward(self, node):
        AnnotateByRepoID(node)
    
    def visitConst(self, node):
        AnnotateByRepoID(node)
    
    def visitDeclarator(self, node):
        AnnotateByRepoID(node)
    
    def visitTypedef(self, node):
        for n in node.declarators():
            n.accept(self)
    
    def visitMember(self, node):
        for n in node.declarators():
            n.accept(self)

    def visitStruct(self, node):
        AnnotateByRepoID(node)
        for n in node.members():
            n.accept(self)
    
    def visitStructForward(self, node):
        AnnotateByRepoID(node)
    
    def visitException(self, node):
        AnnotateByRepoID(node)
        for n in node.members():
            n.accept(self)
    
    def visitCaseLabel(self, node):
        return
    
    def visitUnionCase(self, node):
        pass # not called.
    
    def visitUnion(self, node):
        raise "Unions are not supported."
        # AnnotateByRepoID(node)
        # for n in node.cases():
        #    n.accept(self)
    
    def visitUnionForward(self, node):
        AnnotateByRepoID(node)

    def visitEnumerator(self, node):
        AnnotateByRepoID(node, 1)
    
    def visitEnum(self, node):
        AnnotateByRepoID(node)
        for n in node.enumerators():
            n.accept(self)
    
    def visitAttribute(self, node):
        for n in node.declarators():
            n.accept(self)
    
    def visitParameter(self, node):
        node.simplename = EscapeCXX(node.identifier())
    
    def visitOperation(self, node):
        AnnotateByRepoID(node)
        for n in node.parameters():
            n.accept(self)
    
    def visitNative(self, node):
        raise "Native is not supported."
    
    def visitStateMember(self, node):
        for n in node.declarators():
            n.accept(self)
    
    def visitFactory(self, node):
        for n in node.parameters():
            n.accept(self)
    
    def visitValueForward(self, node):
        raise "valuetypes are not supported."
        
    def visitValueBox(self, node):
        raise "valuetypes are not supported."
    
    def visitValueAbs(self, node):
        raise "valuetypes are not supported."
    
    def visitValue(self, node):
        raise "valuetypes are not supported."
    
    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)
