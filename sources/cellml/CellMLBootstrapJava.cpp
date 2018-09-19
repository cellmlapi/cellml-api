#include <exception>
#include "pick-jni.h"
#include "j2pCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CellMLBootstrap_createCellMLBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CellMLBootstrap_createCellMLBootstrap(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap, CreateCellMLBootstrap());
  return wrap_cellml_api_CellMLBootstrap(env, cb);
}
