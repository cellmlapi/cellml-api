#include <exception>
#include "pick-jni.h"
#include "j2pSProSBootstrap.hxx"
#include "SProSBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_SProSBootstrap_createSProSBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_SProSBootstrap_createSProSBootstrap(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(cgb, iface::SProS::Bootstrap, CreateSProSBootstrap());
  return wrap_SProS_Bootstrap(env, cgb);
}
