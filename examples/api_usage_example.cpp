// The official C++ binding header, which provides C++ specific utilities like ObjRef
#include "cellml-api-cxx-support.hpp"

// This is the standard C++ interface for the core CellML API.
#include "IfaceCellML_APISPEC.hxx"

// This is a C++ binding specific header that defines how to get the bootstrap
// object.
#include "CellMLBootstrap.hpp"

#include <stdio.h>
#include <string>

int
main(int argc, char** argv)
{
  // Fetch a bootstrap object. The bootstrap object is a special 'root' object
  // from which you can get at everything else. CreateCellMLBootstrap() is a
  // C++ binding specific method, which fetches the CellMLBootstrap object.
  // It is the only non-OO method that you should ever call from the CellML
  // API.
  // ObjRef is a template defined in cellml-api-cxx-support.hpp. CreateCellMLBootstrap
  // has already_Addrefd<iface::cellml_api::CellMLBootstrap> as its return type,
  // which is the signal that a reference is added (i.e. the reference count is
  // incremented on the return value, and the caller must ensure it is decremented).
  // ObjRef will, when given an already_Addrefd argument, not increment the reference
  // count, but will decrement it when it goes out of scope. If an ObjRef is constructed
  // from a pointer or another ObjRef, it will add a reference on construction,
  // and release the reference on destruction.
  ObjRef<iface::cellml_api::CellMLBootstrap> cbs(CreateCellMLBootstrap());

  // Now would be a good time to see what methods we can call. In the
  // CellML_DOM_API source, find interfaces/CellML_APISPEC.idl.
  // This defines the interfaces you can call. Search down to find
  // this text...
  /*
  interface CellMLBootstrap
    : XPCOM::IObject
  {
    ...
   */
  // We want to load a model, so we want the modelLoader attribute. We fetch
  // the attribute like this...
  ObjRef<iface::cellml_api::DOMModelLoader> ml(cbs->modelLoader());

  // Suppose we only had a general model loader...
  ObjRef<iface::cellml_api::ModelLoader> generalModelLoader(ml);
  // if we wanted to get from this to a DOM model loader, we can't just cast,
  // because the API is designed so it can work through bridges and we might
  // need to switch to a different bridge (you can cast from an interface to a
  // parent interface in the inheritance hierarchy, but not the other way).
  // Instead, we call query_interface to ask the underlying object if it
  // supports the interface we want, and to return us a value.
  ObjRef<iface::cellml_api::DOMModelLoader> ml2(do_QueryInterface(generalModelLoader));
  // ml2 would be null if generalModelLoader didn't support DOMModelLoader.

  // Start a try, because we might get an exception...
  try
  {
    // We now have a DOMModelLoader, stored in ml. DOMModelLoader inherits from
    // ModelLoader, which defines a loadFromURL operation (check in the IDL).
    // Be warned that is a synchronous (blocking) load. In a real application,
    // you are probably better to download the file using another asynchronous
    // http library, and then creating the model from the serialised text.
    ObjRef<iface::cellml_api::Model> model(ml->loadFromURL(L"http://www.cellml.org/models/beeler_reuter_1977_version04/download"));

    // Fetch the models cmeta:id (there obviously lots of other things we could
    // do here!)
    std::wstring cmid = model->cmetaId();
    printf("Model's cmeta:id is %S\n", cmid.c_str());
  }
  // Most parts of the CellML API raise this exception. The DOM/MathML API, on the
  // other hand, raises iface::dom::DOMException.
  catch (iface::cellml_api::CellMLException&)
  {
    // Unfortunately, due to the need to support the 'lowest common
    // denominator' of functionality in our bindings, exceptions can't have
    // supplementary information (to suit XPCOM). However, many classes have
    // a way to get the last error, e.g. lastErrorMessage on ModelLoader.
    // However, threadsafety is potentially an issue with this.
    std::wstring msg = ml->lastErrorMessage();
    printf("Got a CellML Exception loading a model. Error was %S\n",
           msg.c_str());
    return 1;
  }

  return 0;
}
