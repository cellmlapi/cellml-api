libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jTeLICeMS.cpp \
  $(top_builddir)/interfaces/j2pTeLICeMS.cpp \
  $(top_srcdir)/TeLICeMS/sources/TeLICeMServiceJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(top_builddir)/libxpath.la

BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jxpath.cpp \
  $(top_builddir)/interfaces/j2pxpath.cpp

cellml_jar_java += \
  $(top_builddir)/interfaces/pjm2pcm/xpath/XPathException.java \
  $(top_builddir)/interfaces/pjm2pcm/xpath/XPathEvaluator.java \
  $(top_builddir)/interfaces/pjm2pcm/xpath/XPathExpression.java \
  $(top_builddir)/interfaces/pjm2pcm/xpath/XPathNSResolver.java \
  $(top_builddir)/interfaces/pjm2pcm/xpath/XPathNSResult.java \
  $(top_builddir)/interfaces/pjm2pcm/xpath/XPathNamespace.java \
  $(top_builddir)/interfaces/xpath/XPathException.java \
  $(top_builddir)/interfaces/xpath/XPathEvaluator.java \
  $(top_builddir)/interfaces/xpath/XPathExpression.java \
  $(top_builddir)/interfaces/xpath/XPathNSResolver.java \
  $(top_builddir)/interfaces/xpath/XPathNSResult.java \
  $(top_builddir)/interfaces/xpath/XPathNamespace.java

cellml_jar_classes += \
  $(top_builddir)/javacp/pjm2pcm/xpath/XPathException.class \
  $(top_builddir)/javacp/pjm2pcm/xpath/XPathEvaluator.class \
  $(top_builddir)/javacp/pjm2pcm/xpath/XPathExpression.class \
  $(top_builddir)/javacp/pjm2pcm/xpath/XPathNSResolver.class \
  $(top_builddir)/javacp/pjm2pcm/xpath/XPathNSResult.class \
  $(top_builddir)/javacp/pjm2pcm/xpath/XPathNamespace.class \
  $(top_builddir)/javacp/xpath/XPathException.class \
  $(top_builddir)/javacp/xpath/XPathEvaluator.class \
  $(top_builddir)/javacp/xpath/XPathExpression.class \
  $(top_builddir)/javacp/xpath/XPathNSResolver.class \
  $(top_builddir)/javacp/xpath/XPathNSResult.class \
  $(top_builddir)/javacp/xpath/XPathNamespace.class

pure_java += cellml_bootstrap/XPathBootstrap
