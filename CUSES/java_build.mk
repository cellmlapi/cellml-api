libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jCUSES.cpp \
  $(top_builddir)/interfaces/j2pCUSES.cpp \
  $(top_srcdir)/CUSES/sources/CUSESBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libcuses.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jCUSES.cpp \
  $(top_builddir)/interfaces/j2pCUSES.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/CUSESBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/BaseUnit.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/UserBaseUnit.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/BaseUnitInstance.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CanonicalUnitRepresentation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CUSES.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CUSESBootstrap.java \
  $(top_builddir)/interfaces/cellml_services/BaseUnit.java \
  $(top_builddir)/interfaces/cellml_services/UserBaseUnit.java \
  $(top_builddir)/interfaces/cellml_services/BaseUnitInstance.java \
  $(top_builddir)/interfaces/cellml_services/CanonicalUnitRepresentation.java \
  $(top_builddir)/interfaces/cellml_services/CUSES.java \
  $(top_builddir)/interfaces/cellml_services/CUSESBootstrap.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/CUSESBootstrap.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/BaseUnit.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/UserBaseUnit.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/BaseUnitInstance.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CanonicalUnitRepresentation.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CUSES.java \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CUSESBootstrap.java \
  $(top_builddir)/javacp/cellml_services/BaseUnit.java \
  $(top_builddir)/javacp/cellml_services/UserBaseUnit.java \
  $(top_builddir)/javacp/cellml_services/BaseUnitInstance.java \
  $(top_builddir)/javacp/cellml_services/CanonicalUnitRepresentation.java \
  $(top_builddir)/javacp/cellml_services/CUSES.java \
  $(top_builddir)/javacp/cellml_services/CUSESBootstrap.java
