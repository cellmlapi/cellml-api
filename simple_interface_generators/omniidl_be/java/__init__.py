# -*- python -*-

from simple_cpp import identifier
import os
import interfacegen
import nativeclassgen
import pcm2j
import j2pcm

def run(tree, args):
    """Entrypoint to the Java backend"""

    tree.directory, tree.filename = os.path.split(tree.file())
    tree.filebase, extension = os.path.splitext(tree.filename)

    # Firstly, annotate everything with the correct identifiers...
    tree.accept(identifier.Annotator())
    interfacegen.run(tree)
    nativeclassgen.run(tree)
    pcm2j.run(tree)
    j2pcm.run(tree)
