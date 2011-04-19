libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jTeLICeMS.cpp \
  $(top_builddir)/interfaces/j2pTeLICeMS.cpp \
  $(top_srcdir)/TeLICeMS/sources/TeLICeMServiceJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libtelicems.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jTeLICeMS.cpp \
  $(top_builddir)/interfaces/j2pTeLICeMS.cpp

cellml_jar_java += \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/TeLICeMService.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/TeLICeMMathResult.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/TeLICeMModelResult.java \
  $(top_builddir)/interfaces/cellml_services/TeLICeMService.java \
  $(top_builddir)/interfaces/cellml_services/TeLICeMMathResult.java \
  $(top_builddir)/interfaces/cellml_services/TeLICeMModelResult.java

cellml_jar_classes += \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/TeLICeMService.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/TeLICeMMathResult.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/TeLICeMModelResult.class \
  $(top_builddir)/javacp/cellml_services/TeLICeMService.class \
  $(top_builddir)/javacp/cellml_services/TeLICeMMathResult.class \
  $(top_builddir)/javacp/cellml_services/TeLICeMModelResult.class

pure_java += cellml_bootstrap/TeLICeMSBootstrap
