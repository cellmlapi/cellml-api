#include <exception>
#include <jni.h>
#include "j2pVACSS.hxx"
#include "VACSSBootstrap.hpp"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_VACSSBootstrap_createVACSService(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_VACSSBootstrap_createVACSService(JNIEnv* env, jclass clazz)
{
  return wrap_cellml_services_VACSService(env, CreateVACSService());
}
