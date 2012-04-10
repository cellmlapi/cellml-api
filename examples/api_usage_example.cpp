// This is not an official part of the API, but including it is not a problem
// because it doesn't directly access any private APIs.
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
  // Note that RETURN_INTO_OBJREF is a macro from Utilities.hxx. It is creating
  // a variable called cbs, and assigning into it. CreateCellMLBootstrap
  // returns iface::cellml_api::CellMLBootstrap*. We could assign this directly
  // into a variable. We would then have to call cbs->release_ref() to release
  // the object when we have finished with it. The ObjRef<> template does this
  // for us, so there is no chance we will forget to clean up on a certain
  // exit path.
  RETURN_INTO_OBJREF(cbs, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());

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
  RETURN_INTO_OBJREF(ml, iface::cellml_api::DOMModelLoader, cbs->modelLoader());

  // Start a try, because we might get an exception...
  try
  {
    // We now have a DOMModelLoader, stored in ml. DOMModelLoader inherits from
    // ModelLoader, which defines a loadFromURL operation (check in the IDL).
    // Be warned that is a synchronous (blocking) load. In a real application,
    // you are probably better to download the file using another asynchronous
    // http library, and then creating the model from the serialised text.
    RETURN_INTO_OBJREF(model, iface::cellml_api::Model,
                       ml->loadFromURL(L"http://www.cellml.org/models/beeler_reuter_1977_version04/download"));

    // Fetch the models cmeta:id (there obviously lots of other things we could
    // do here!)
    RETURN_INTO_WSTRING(cmid, model->cmetaId());
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
    RETURN_INTO_WSTRING(msg, ml->lastErrorMessage());
    printf("Got a CellML Exception loading a model. Error was %S\n",
           msg.c_str());
    return 1;
  }

  return 0;
}
