libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jSRuS.cpp \
  $(top_builddir)/interfaces/j2pSRuS.cpp \
  $(top_srcdir)/SRuS/SRuSBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libsrus.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jSRuS.cpp \
  $(top_builddir)/interfaces/j2pSRuS.cpp

cellml_jar_java += \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/SEDMLProcessor.java \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/TransformedModel.java \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/TransformedModelSet.java \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/GeneratedDataMonitor.java \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/GeneratedData.java \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/GeneratedDataSet.java \
  $(top_builddir)/interfaces/pjm2pcm/SRuS/Bootstrap.java \
  $(top_builddir)/interfaces/SRuS/SEDMLProcessor.java \
  $(top_builddir)/interfaces/SRuS/TransformedModel.java \
  $(top_builddir)/interfaces/SRuS/TransformedModelSet.java \
  $(top_builddir)/interfaces/SRuS/GeneratedDataMonitor.java \
  $(top_builddir)/interfaces/SRuS/GeneratedData.java \
  $(top_builddir)/interfaces/SRuS/GeneratedDataSet.java \
  $(top_builddir)/interfaces/SRuS/Bootstrap.java

cellml_jar_classes += \
  $(top_builddir)/javacp/pjm2pcm/SRuS/SEDMLProcessor.class \
  $(top_builddir)/javacp/pjm2pcm/SRuS/TransformedModel.class \
  $(top_builddir)/javacp/pjm2pcm/SRuS/TransformedModelSet.class \
  $(top_builddir)/javacp/pjm2pcm/SRuS/GeneratedDataMonitor.class \
  $(top_builddir)/javacp/pjm2pcm/SRuS/GeneratedData.class \
  $(top_builddir)/javacp/pjm2pcm/SRuS/GeneratedDataSet.class \
  $(top_builddir)/javacp/pjm2pcm/SRuS/Bootstrap.class \
  $(top_builddir)/javacp/SRuS/SEDMLProcessor.class \
  $(top_builddir)/javacp/SRuS/TransformedModel.class \
  $(top_builddir)/javacp/SRuS/TransformedModelSet.class \
  $(top_builddir)/javacp/SRuS/GeneratedDataMonitor.class \
  $(top_builddir)/javacp/SRuS/GeneratedData.class \
  $(top_builddir)/javacp/SRuS/GeneratedDataSet.class \
  $(top_builddir)/javacp/SRuS/Bootstrap.class

pure_java += cellml_bootstrap/SRuSBootstrap
