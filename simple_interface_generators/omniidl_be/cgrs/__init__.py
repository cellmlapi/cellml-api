# -*- python -*-
from omniidl import idlast, idlvisitor, idlutil, idltype, output
import os.path
import CGRSWalker

def run(tree, args):
    """Entrypoint to the CGRS backend"""

    bootstrapSpecials = []
    specialIncludes = []

    for a in args:
        if a[0:9] == "bootstrap":
            bootstrapSpecials.append(string.split(a[9:], "="))
        elif a[0:7] == "include":
            specialIncludes.append(string.split(a[7:], "="))

    tree.directory, tree.filename = os.path.split(tree.file())
    tree.filebase, extension = os.path.splitext(tree.filename)

    cgrsw = CGRSWalker.CGRSWalker()
    cgrsw.bootstrapSpecials = bootstrapSpecials
    cgrsw.specialIncludes = specialIncludes
    cgrsw.cxx = output.Stream(open('CGRS' + tree.filebase + '.cxx', 'w'), 2)

    annot = identifier.Annotator()

    tree.accept(annot)
    tree.accept(cgrsw)
