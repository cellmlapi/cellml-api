#include <exception>
#include <jni.h>
#include "j2pCeVAS.hxx"
#include "CeVASBootstrap.hpp"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CeVASBootstrap_createCeVASBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CeVASBootstrap_createCeVASBootstrap(JNIEnv* env, jclass clazz)
{
  return wrap_cellml_services_CeVASBootstrap(env, CreateCeVASBootstrap());
}
