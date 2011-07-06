# -*- python -*-
import os.path
import identifier
import cxxheadergen
#import ccigen
#import scigen

def run(tree, args):
    """Entrypoint to the simple C++ backend"""

    tree.directory, tree.filename = os.path.split(tree.file())
    tree.filebase, extension = os.path.splitext(tree.filename)
    
    # Firstly, annotate everything with the correct identifiers...
    tree.accept(identifier.Annotator())
    cxxheadergen.run(tree)
    #ccigen.run(tree)
    #scigen.run(tree)
