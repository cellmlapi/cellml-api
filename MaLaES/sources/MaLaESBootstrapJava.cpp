#include <exception>
#include "pick-jni.h"
#include "j2pMaLaES.hxx"
#include "MaLaESBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_MaLaESBootstrap_createMaLaESBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_MaLaESBootstrap_createMaLaESBootstrap(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(mb, iface::cellml_services::MaLaESBootstrap, CreateMaLaESBootstrap());
  return wrap_cellml_services_MaLaESBootstrap(env, mb);
}
