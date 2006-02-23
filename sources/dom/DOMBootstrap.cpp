#include "DOMImplementation.hxx"
#include "DOMBootstrap.hxx"

iface::dom::DOMImplementation*
CreateDOMImplementation()
{
  return new CDA_DOMImplementation();
}
