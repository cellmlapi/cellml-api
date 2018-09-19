#define IN_DOM_MODULE
#define MODULE_CONTAINS_xpcom
#define MODULE_CONTAINS_DOMAPISPEC

#include "cda_compiler_support.h"
#include "DOMImplementation.hpp"

CellML_DOMImplementationBase*
CreateDOMImplementation()
{
  return new CDA_DOMImplementation();
}
