libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jMaLaES.cpp \
  $(top_builddir)/interfaces/j2pMaLaES.cpp \
  $(top_srcdir)/MaLaES/sources/MaLaESBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libmalaes.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jMaLaES.cpp \
  $(top_builddir)/interfaces/j2pMaLaES.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/MaLaESBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/DegreeVariable.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/DegreeVariableIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/MaLaESResult.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/MaLaESTransform.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/MaLaESBootstrap.java \
  $(top_builddir)/interfaces/cellml_services/DegreeVariable.java \
  $(top_builddir)/interfaces/cellml_services/DegreeVariableIterator.java \
  $(top_builddir)/interfaces/cellml_services/MaLaESResult.java \
  $(top_builddir)/interfaces/cellml_services/MaLaESTransform.java \
  $(top_builddir)/interfaces/cellml_services/MaLaESBootstrap.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/MaLaESBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/DegreeVariable.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/DegreeVariableIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/MaLaESResult.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/MaLaESTransform.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/MaLaESBootstrap.class \
  $(top_builddir)/javacp/cellml_services/DegreeVariable.class \
  $(top_builddir)/javacp/cellml_services/DegreeVariableIterator.class \
  $(top_builddir)/javacp/cellml_services/MaLaESResult.class \
  $(top_builddir)/javacp/cellml_services/MaLaESTransform.class \
  $(top_builddir)/javacp/cellml_services/MaLaESBootstrap.class

pure_java += cellml_bootstrap/MaLaESBootstrap
