# -*- python -*-
import os.path
import identifier
import typeinfo
from omniidl import idlast, idlvisitor, idlutil, idltype, output
import PythonToCWalker
import CToPythonWalker
import string

def run(tree, args):
    """Entrypoint to the Python backend"""

    bootstrapSpecials = []
    specialIncludes = []
    idlToExtMap = {}
    moduledir = ''
    modulename = ''

    for a in args:
        if a[0:9] == "bootstrap":
            bootstrapSpecials.append(string.split(a[9:], "="))
        elif a[0:7] == "include":
            specialIncludes.append(string.split(a[7:], "="))
        elif a[0:10] == "moduledir=":
            moduledir = a[10:]
        elif a[0:11] == "modulename=":
            modulename = a[11:]

    tree.directory, tree.filename = os.path.split(tree.file())
    tree.filebase, extension = os.path.splitext(tree.filename)

    p2cw = PythonToCWalker.PythonToCWalker()
    p2cw.bootstrapSpecials = bootstrapSpecials
    p2cw.specialIncludes = specialIncludes
    p2cw.moduledir = moduledir
    p2cw.modulename = modulename
    p2cw.out = output.Stream(open('Py2P' + tree.filebase + '.cxx', 'w'), 2)
    c2pw = CToPythonWalker.CToPythonWalker()
    c2pw.moduledir = moduledir
    c2pw.cpp = output.Stream(open('P2Py' + tree.filebase + '.cxx', 'w'), 2)
    c2pw.hxx = output.Stream(open('P2Py' + tree.filebase + '.hxx', 'w'), 2)

    annot = identifier.Annotator()

    tree.accept(annot)
    tree.accept(p2cw)
    tree.accept(c2pw)
