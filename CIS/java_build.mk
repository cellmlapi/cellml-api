libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jCIS.cpp \
  $(top_builddir)/interfaces/j2pCIS.cpp \
  $(top_srcdir)/CIS/sources/CISBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libcis.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jCIS.cpp \
  $(top_builddir)/interfaces/j2pCIS.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/CISBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/IntegrationProgressObserver.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLIntegrationRun.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLCompiledModel.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CellMLIntegrationService.java \
  $(top_builddir)/interfaces/cellml_services/IntegrationStepType.java \
  $(top_builddir)/interfaces/cellml_services/IntegrationProgressObserver.java \
  $(top_builddir)/interfaces/cellml_services/CellMLIntegrationRun.java \
  $(top_builddir)/interfaces/cellml_services/CellMLCompiledModel.java \
  $(top_builddir)/interfaces/cellml_services/CellMLIntegrationService.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/CISBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/IntegrationProgressObserver.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLIntegrationRun.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLCompiledModel.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CellMLIntegrationService.class \
  $(top_builddir)/javacp/cellml_services/IntegrationStepType.class \
  $(top_builddir)/javacp/cellml_services/IntegrationProgressObserver.class \
  $(top_builddir)/javacp/cellml_services/CellMLIntegrationRun.class \
  $(top_builddir)/javacp/cellml_services/CellMLCompiledModel.class \
  $(top_builddir)/javacp/cellml_services/CellMLIntegrationService.class
