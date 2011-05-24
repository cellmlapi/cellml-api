#include <exception>
#include "pick-jni.h"
#include "j2pTeLICeMS.hxx"
#include "TeLICeMService.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_TeLICeMSBootstrap_createTeLICeMService(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_TeLICeMSBootstrap_createTeLICeMService(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(cgb, iface::cellml_services::TeLICeMService, CreateTeLICeMService());
  return wrap_cellml_services_TeLICeMService(env, cgb);
}
