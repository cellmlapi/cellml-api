#include <inttypes.h>
#include "DOMImplementation.hpp"

CellML_DOMImplementationBase*
CreateDOMImplementation()
{
  return new CDA_DOMImplementation();
}
