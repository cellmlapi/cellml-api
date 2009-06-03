import string

def XPCOMInterfaceName(scopedname):
    return string.join(scopedname[:-1], '_') + 'I' + scopedname[-1]
