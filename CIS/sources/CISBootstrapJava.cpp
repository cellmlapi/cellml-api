#include <exception>
#include <jni.h>
#include "j2pCIS.hxx"
#include "CISBootstrap.hpp"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CISBootstrap_createIntegrationService(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CISBootstrap_createIntegrationService(JNIEnv* env, jclass clazz)
{
  return wrap_cellml_services_CellMLIntegrationService(env, CreateIntegrationService());
}
