# -*- python -*-
import os.path
import identifier
import xpidlgen
import x2pgen
import p2xgen

def run(tree, args):
    """Entrypoint to the simple C++ backend"""

    tree.directory, tree.filename = os.path.split(tree.file())
    tree.filebase, extension = os.path.splitext(tree.filename)

    # Firstly, annotate everything with the correct identifiers...
    tree.accept(identifier.Annotator())
    xpidlgen.run(tree)
    x2pgen.run(tree)
    p2xgen.run(tree)
