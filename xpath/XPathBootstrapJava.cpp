#include <exception>
#include "pick-jni.h"
#include "j2pxpath.hxx"
#include "XPathBootstrap.hpp"
#include "Utilities.hxx"

extern "C" { JWRAP_PUBLIC_PRE jobject Java_cellml_1bootstrap_XPathBootstrap_createXPathEvaluator(JNIEnv* env, jclass clazz) JWRAP_PUBLIC_POST; }

jobject
Java_cellml_1bootstrap_XPathBootstrap_createXPathEvaluator(JNIEnv* env, jclass clazz)
{
  RETURN_INTO_OBJREF(xe, iface::xpath::XPathEvaluator, CreateXPathEvaluator());
  return wrap_xpath_XPathEvaluator(env, xe);
}
