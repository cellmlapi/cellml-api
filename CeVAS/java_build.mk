libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jCeVAS.cpp \
  $(top_builddir)/interfaces/j2pCeVAS.cpp \
  $(top_srcdir)/CeVAS/sources/CeVASBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libcevas.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jCeVAS.cpp \
  $(top_builddir)/interfaces/j2pCeVAS.cpp

cellml_jar_java += \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/ConnectedVariableSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CeVAS.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CeVASBootstrap.java \
  $(top_builddir)/interfaces/cellml_services/ConnectedVariableSet.java \
  $(top_builddir)/interfaces/cellml_services/CeVAS.java \
  $(top_builddir)/interfaces/cellml_services/CeVASBootstrap.java

cellml_jar_classes += \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/ConnectedVariableSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CeVAS.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CeVASBootstrap.class \
  $(top_builddir)/javacp/cellml_services/ConnectedVariableSet.class \
  $(top_builddir)/javacp/cellml_services/CeVAS.class \
  $(top_builddir)/javacp/cellml_services/CeVASBootstrap.class

pure_java += cellml_bootstrap/CeVASBootstrap
