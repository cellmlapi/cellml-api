libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jCeLEDSExporter.cpp \
  $(top_builddir)/interfaces/j2pCeLEDSExporter.cpp \
  $(top_srcdir)/CeLEDSExporter/sources/CeLEDSExporterBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libceledsexporter.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jCeLEDSExporter.cpp \
  $(top_builddir)/interfaces/j2pCeLEDSExporter.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/CeLEDSExporterBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CeLEDSExporterBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CodeExporter.java \
  $(top_builddir)/interfaces/cellml_services/CeLEDSBootstrap.java \
  $(top_builddir)/interfaces/cellml_services/CodeExporter.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/CeLEDSExporterBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CeLEDSExporterBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CodeExporter.class \
  $(top_builddir)/javacp/cellml_services/CeLEDSExporterBootstrap.class \
  $(top_builddir)/javacp/cellml_services/CodeExporter.class
