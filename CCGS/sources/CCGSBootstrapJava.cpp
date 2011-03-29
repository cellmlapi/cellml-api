#include <exception>
#include "pick-jni.h"
#include "j2pCCGS.hxx"
#include "CCGSBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CCGSBootstrap_createCodeGeneratorBootstrap(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CCGSBootstrap_createCodeGeneratorBootstrap(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(cgb, iface::cellml_services::CodeGeneratorBootstrap, CreateCodeGeneratorBootstrap());
  return wrap_cellml_services_CodeGeneratorBootstrap(env, cgb);
}
