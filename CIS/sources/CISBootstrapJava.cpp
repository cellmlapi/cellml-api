#include <exception>
#include <jni.h>
#include "j2pCIS.hxx"
#include "CISBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_CISBootstrap_createIntegrationService(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_CISBootstrap_createIntegrationService(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(cis, iface::cellml_services::CellMLIntegrationService, CreateIntegrationService());
  return wrap_cellml_services_CellMLIntegrationService(env, cis);
}
