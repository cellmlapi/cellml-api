libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jAnnoTools.cpp \
  $(top_builddir)/interfaces/j2pAnnoTools.cpp \
  $(top_srcdir)/AnnoTools/sources/AnnoToolsBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libannotools.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jAnnoTools.cpp \
  $(top_builddir)/interfaces/j2pAnnoTools.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/AnnoToolsBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/StringAnnotation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/ObjectAnnotation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/AnnotationSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/AnnotationToolService.java \
  $(top_builddir)/interfaces/cellml_services/StringAnnotation.java \
  $(top_builddir)/interfaces/cellml_services/ObjectAnnotation.java \
  $(top_builddir)/interfaces/cellml_services/AnnotationSet.java \
  $(top_builddir)/interfaces/cellml_services/AnnotationToolService.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/AnnoToolsBootstrap.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/StringAnnotation.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/ObjectAnnotation.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/AnnotationSet.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/AnnotationToolService.java \
  $(top_builddir)/javacp/cellml_services/StringAnnotation.java \
  $(top_builddir)/javacp/cellml_services/ObjectAnnotation.java \
  $(top_builddir)/javacp/cellml_services/AnnotationSet.java \
  $(top_builddir)/javacp/cellml_services/AnnotationToolService.java
