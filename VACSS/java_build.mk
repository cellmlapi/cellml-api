libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jVACSS.cpp \
  $(top_builddir)/interfaces/j2pVACSS.cpp \
  $(top_srcdir)/VACSS/sources/VACSSBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libvacss.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jVACSS.cpp \
  $(top_builddir)/interfaces/j2pVACSS.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/VACSSBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLValidityError.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLRepresentationValidityError.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLSemanticValidityError.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLValidityErrorSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/VACSService.java \
  $(top_builddir)/interfaces/cellml_services/CellMLValidityError.java \
  $(top_builddir)/interfaces/cellml_services/CellMLRepresentationValidityError.java \
  $(top_builddir)/interfaces/cellml_services/CellMLSemanticValidityError.java \
  $(top_builddir)/interfaces/cellml_services/CellMLValidityErrorSet.java \
  $(top_builddir)/interfaces/cellml_services/VACSService.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/VACSSBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLValidityError.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLRepresentationValidityError.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLSemanticValidityError.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLValidityErrorSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/VACSService.class \
  $(top_builddir)/javacp/cellml_services/CellMLValidityError.class \
  $(top_builddir)/javacp/cellml_services/CellMLRepresentationValidityError.class \
  $(top_builddir)/javacp/cellml_services/CellMLSemanticValidityError.class \
  $(top_builddir)/javacp/cellml_services/CellMLValidityErrorSet.class \
  $(top_builddir)/javacp/cellml_services/VACSService.class
