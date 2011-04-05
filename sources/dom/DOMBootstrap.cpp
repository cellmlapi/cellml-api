#define IN_DOM_MODULE

#include "cda_compiler_support.h"
#include "DOMImplementation.hpp"

CellML_DOMImplementationBase*
CreateDOMImplementation()
{
  return new CDA_DOMImplementation();
}
