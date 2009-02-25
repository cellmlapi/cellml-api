libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jCeLEDS.cpp \
  $(top_builddir)/interfaces/j2pCeLEDS.cpp \
  $(top_srcdir)/CeLEDS/sources/CeLEDSBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libceleds.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jCeLEDS.cpp \
  $(top_builddir)/interfaces/j2pCeLEDS.cpp

cellml_jar_java += \
  $(top_srcdir)/java/cellml_bootstrap/CeLEDSBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/CeLEDSBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/LanguageDictionary.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_services/DictionaryGenerator.java \
  $(top_builddir)/interfaces/cellml_services/CeLEDSBootstrap.java \
  $(top_builddir)/interfaces/cellml_services/LanguageDictionary.java \
  $(top_builddir)/interfaces/cellml_services/DictionaryGenerator.java

cellml_jar_classes += \
  $(top_builddir)/javacp/cellml_bootstrap/CeLEDSBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/CeLEDSBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/LanguageDictionary.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_services/DictionaryGenerator.class \
  $(top_builddir)/javacp/cellml_services/CeLEDSBootstrap.class \
  $(top_builddir)/javacp/cellml_services/LanguageDictionary.class \
  $(top_builddir)/javacp/cellml_services/DictionaryGenerator.class
