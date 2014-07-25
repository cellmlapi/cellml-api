#include <exception>
#include "pick-jni.h"
#include "j2pAnnoTools.hxx"
#include "AnnoToolsBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_AnnoToolsBootstrap_createAnnotationToolService(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_AnnoToolsBootstrap_createAnnotationToolService(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(ats, iface::cellml_services::AnnotationToolService, CreateAnnotationToolService());
  return wrap_cellml_services_AnnotationToolService(env, ats);
}
