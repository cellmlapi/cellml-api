#include "cda_compiler_support.h"
#ifdef HAVE_JNI_H
#include <jni.h>
#else
#ifdef HAVE_JAVAVM_JNI_H
#include <JavaVM/jni.h>
#else
#include <jni.h>
#endif
#endif
