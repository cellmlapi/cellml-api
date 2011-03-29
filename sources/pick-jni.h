#ifdef HAVE_JNI_H
#include <jni.h>
#else
#ifdef HAVE_JAVAVM_JNI_H
#include <JavaVM/jni.h>
#else
#error "Don't know where to find jni.h on your system; try installing the JDK, making sure it is in the path, or turning off Java support."
#endif
#endif
