#include <exception>
#include "pick-jni.h"
#include "j2pSRuS.hxx"
#include "SRuSBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_SRuSBootstrap_createSRuSBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_SRuSBootstrap_createSRuSBootstrap(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(cgb, iface::SRuS::Bootstrap, CreateSRuSBootstrap());
  return wrap_SRuS_Bootstrap(env, cgb);
}
