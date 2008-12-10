#include <exception>
#include <jni.h>
#include "j2pCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CellMLBootstrap_createCellMLBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CellMLBootstrap_createCellMLBootstrap(JNIEnv* env, jclass clazz)
{
  return wrap_cellml_api_CellMLBootstrap(env, CreateCellMLBootstrap());
}
