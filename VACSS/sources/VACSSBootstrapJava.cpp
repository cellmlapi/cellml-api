#include <exception>
#include "pick-jni.h"
#include "j2pVACSS.hxx"
#include "VACSSBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_VACSSBootstrap_createVACSService(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_VACSSBootstrap_createVACSService(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(vs, iface::cellml_services::VACSService, CreateVACSService());
  return wrap_cellml_services_VACSService(env, vs);
}
