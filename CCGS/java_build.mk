libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jCCGS.cpp \
  $(top_builddir)/interfaces/j2pCCGS.cpp \
  $(top_srcdir)/CCGS/sources/CCGSBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libccgs.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jCCGS.cpp \
  $(top_builddir)/interfaces/j2pCCGS.cpp

cellml_jar_java += \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/VariableEvaluationType.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/ModelConstraintLevel.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/ComputationTarget.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/ComputationTargetIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CodeInformation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/IDACodeInformation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CodeGenerator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/IDACodeGenerator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CustomGenerator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CodeGeneratorBootstrap.java \
  $(top_builddir)/interfaces/cellml_services/VariableEvaluationType.java \
  $(top_builddir)/interfaces/cellml_services/ModelConstraintLevel.java \
  $(top_builddir)/interfaces/cellml_services/ComputationTarget.java \
  $(top_builddir)/interfaces/cellml_services/ComputationTargetIterator.java \
  $(top_builddir)/interfaces/cellml_services/CodeInformation.java \
  $(top_builddir)/interfaces/cellml_services/IDACodeInformation.java \
  $(top_builddir)/interfaces/cellml_services/CodeGenerator.java \
  $(top_builddir)/interfaces/cellml_services/IDACodeGenerator.java \
  $(top_builddir)/interfaces/cellml_services/CustomGenerator.java \
  $(top_builddir)/interfaces/cellml_services/CodeGeneratorBootstrap.java

cellml_jar_classes += \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/VariableEvaluationType.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/ModelConstraintLevel.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/ComputationTarget.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/ComputationTargetIterator.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CodeInformation.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/IDACodeInformation.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CodeGenerator.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/IDACodeGenerator.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CustomGenerator.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CodeGeneratorBootstrap.java
  $(top_builddir)/javacp/cellml_services/VariableEvaluationType.java \
  $(top_builddir)/javacp/cellml_services/ModelConstraintLevel.java \
  $(top_builddir)/javacp/cellml_services/ComputationTarget.java \
  $(top_builddir)/javacp/cellml_services/ComputationTargetIterator.java \
  $(top_builddir)/javacp/cellml_services/CodeInformation.java \
  $(top_builddir)/javacp/cellml_services/IDACodeInformation.java \
  $(top_builddir)/javacp/cellml_services/CodeGenerator.java \
  $(top_builddir)/javacp/cellml_services/IDACodeGenerator.java \
  $(top_builddir)/javacp/cellml_services/CustomGenerator.java \
  $(top_builddir)/javacp/cellml_services/CodeGeneratorBootstrap.java
  $(top_builddir)/interfaces/cellml_services/IDACodeGenerator.class \
  $(top_builddir)/interfaces/cellml_services/CustomGenerator.class \
  $(top_builddir)/javacp/cellml_services/CodeGeneratorBootstrap.class

pure_java += cellml_bootstrap/CCGSBootstrap
