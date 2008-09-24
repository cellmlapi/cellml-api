#include "cda_compiler_support.h"
#include <exception>
#include <string>
#include "IfaceDOM_APISPEC.hxx"
#include "DOMBootstrap.hxx"

int
main(int argc, char** argv)
{
  iface::dom::DOMImplementation* imp = CreateDOMImplementation();
  iface::dom::Document* doc = imp->createDocument(L"http://www.example.org/blah",
                                                  L"blargh", NULL);
  iface::dom::Element* de = doc->documentElement();
  iface::dom::Element* ne = doc->createElement(L"hello");
  de->appendChild(ne)->release_ref();
  ne->release_ref();
  de->release_ref();
  doc->release_ref();
  imp->release_ref();
}
