#include <exception>
#include "pick-jni.h"
#include "j2pCeLEDS.hxx"
#include "CeLEDSBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CeLEDSBootstrap_createCeLEDSBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CeLEDSBootstrap_createCeLEDSBootstrap(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(mb, iface::cellml_services::CeLEDSBootstrap, CreateCeLEDSBootstrap());
  return wrap_cellml_services_CeLEDSBootstrap(env, mb);
}

