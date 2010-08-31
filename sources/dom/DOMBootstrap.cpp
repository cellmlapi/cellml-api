#define IN_DOM_MODULE

#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include "DOMImplementation.hpp"

CellML_DOMImplementationBase*
CreateDOMImplementation()
{
  return new CDA_DOMImplementation();
}
