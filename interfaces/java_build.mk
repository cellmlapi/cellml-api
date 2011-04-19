# Java add-on to interfaces module...

lib_LTLIBRARIES += libcellml_java_bridge.la

libcellml_java_bridge_la_SOURCES += \
  $(top_builddir)/interfaces/p2jDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/p2jDOM_events.cpp \
  $(top_builddir)/interfaces/p2jMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/p2jCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/p2jCellML_events.cpp \
  $(top_builddir)/interfaces/j2pDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pDOM_events.cpp \
  $(top_builddir)/interfaces/j2pMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pCellML_events.cpp \
  $(top_builddir)/interfaces/p2jRDF_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pRDF_APISPEC.cpp \
  $(top_srcdir)/sources/cellml/CellMLBootstrapJava.cpp

libcellml_java_bridge_la_LIBADD += \
  $(STLLINK) \
  $(top_builddir)/libcellml.la \
  $(top_builddir)/libJavaSupport.la
libcellml_java_bridge_la_LDFLAGS = \
  -no-undefined -module

libcellml_java_bridge_la_CXXFLAGS = \
  -I$(top_builddir)/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/sources -I$(top_srcdir)/sources/rdf \
  -I$(top_srcdir)/simple_interface_generators/glue/java \
  -I$(top_builddir)/simple_interface_generators/glue/java \
  $(AM_CXXFLAGS) -DMODULE_CONTAINS_DOMAPISPEC \
  -DMODULE_CONTAINS_MathMLcontentAPISPEC -DMODULE_CONTAINS_CellMLAPISPEC \
  -DMODULE_CONTAINS_CellMLevents -DMODULE_CONTAINS_RDFAPISPEC -DIN_JAVA_WRAPPER

$(top_builddir)/interfaces/p2j%.cpp \
$(top_builddir)/interfaces/p2j%.hxx \
$(top_builddir)/interfaces/j2p%.cpp \
$(top_builddir)/interfaces/j2p%.hxx: \
$(top_srcdir)/interfaces/%.idl $(top_srcdir)/interfaces/Iface%.hxx $(top_srcdir)/simple_interface_generators/omniidl_be/java/j2pcm.py $(top_srcdir)/simple_interface_generators/omniidl_be/java/pcm2j.py $(top_srcdir)/simple_interface_generators/omniidl_be/java/jnutils.py
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -p$$SAVEDIR/$(top_srcdir)/simple_interface_generators/omniidl_be \
         -bjava -DIN_CELLML_MODULE $$SAVEDIR/$< && \
	cd $$SAVEDIR

interface_list =\
  cellml_events/MutationEvent \
  dom/ProcessingInstruction \
  dom/CharacterData \
  dom/NamedNodeMap \
  dom/EntityReference \
  dom/Node \
  dom/Notation \
  dom/DocumentType \
  dom/Document \
  dom/Entity \
  dom/Element \
  dom/CDATASection \
  dom/NodeList \
  dom/Attr \
  dom/Text \
  dom/Comment \
  dom/DocumentFragment \
  dom/DOMImplementation \
  cellml_api/RDFXMLStringRepresentation \
  cellml_api/ModelLoadedListener \
  cellml_api/CellMLComponent \
  cellml_api/CellMLVariableIterator \
  cellml_api/RelationshipRef \
  cellml_api/CellMLComponentSet \
  cellml_api/MathMLElementIterator \
  cellml_api/CellMLVariable \
  cellml_api/Group \
  cellml_api/VariableRefSet \
  cellml_api/MapVariablesSet \
  cellml_api/MapVariablesIterator \
  cellml_api/ConnectionIterator \
  cellml_api/CellMLBootstrap \
  cellml_api/ComponentRefSet \
  cellml_api/MapVariables \
  cellml_api/Role \
  cellml_api/ModelSet \
  cellml_api/DOMURLLoader \
  cellml_api/ModelLoader \
  cellml_api/ImportComponent \
  cellml_api/CellMLComponentIterator \
  cellml_api/RoleIterator \
  cellml_api/RDFRepresentation \
  cellml_api/RDFXMLDOMRepresentation \
  cellml_api/DOMModelLoader \
  cellml_api/ImportComponentIterator \
  cellml_api/UnitSet \
  cellml_api/ReactionSet \
  cellml_api/ComponentRefIterator \
  cellml_api/UnitsSet \
  cellml_api/Reaction \
  cellml_api/VariableRef \
  cellml_api/UnitsIterator \
  cellml_api/URI \
  cellml_api/CellMLElementIterator \
  cellml_api/VariableRefIterator \
  cellml_api/CellMLImport \
  cellml_api/ModelIterator \
  cellml_api/GroupSet \
  cellml_api/ImportUnitsSet \
  cellml_api/CellMLDOMElement \
  cellml_api/MathList \
  cellml_api/UnitIterator \
  cellml_api/CellMLImportSet \
  cellml_api/RoleSet \
  cellml_api/NamedCellMLElementSet \
  cellml_api/MathContainer \
  cellml_api/RelationshipRefSet \
  cellml_api/Model \
  cellml_api/ImportUnits \
  cellml_api/Connection \
  cellml_api/Units \
  cellml_api/ImportComponentSet \
  cellml_api/ImportInstantiationListener \
  cellml_api/RelationshipRefIterator \
  cellml_api/MapComponents \
  cellml_api/NamedCellMLElement \
  cellml_api/DocumentLoadedListener \
  cellml_api/Unit \
  cellml_api/ReactionIterator \
  cellml_api/ExtensionElementList \
  cellml_api/ExtensionAttributeSet \
  cellml_api/GroupIterator \
  cellml_api/ImportUnitsIterator \
  cellml_api/CellMLVariableSet \
  cellml_api/CellMLImportIterator \
  cellml_api/ComponentRef \
  cellml_api/UserData \
  cellml_api/ConnectionSet \
  cellml_api/CellMLElementSet \
  cellml_api/CellMLElement \
  rdf_api/TripleSet \
  rdf_api/Literal \
  rdf_api/BlankNode \
  rdf_api/DataSource \
  rdf_api/Node \
  rdf_api/NodeIterator \
  rdf_api/Container \
  rdf_api/Resource \
  rdf_api/RDFAPIRepresentation \
  rdf_api/Bootstrap \
  rdf_api/PlainLiteral \
  rdf_api/URIReference \
  rdf_api/TypedLiteral \
  rdf_api/TripleEnumerator \
  rdf_api/Triple \
  mathml_dom/MathMLApplyElement \
  mathml_dom/MathMLConditionElement \
  mathml_dom/MathMLPredefinedSymbol \
  mathml_dom/MathMLContentToken \
  mathml_dom/MathMLMathElement \
  mathml_dom/MathMLDOMImplementation \
  mathml_dom/MathMLBvarElement \
  mathml_dom/MathMLIntervalElement \
  mathml_dom/MathMLNodeList \
  mathml_dom/MathMLSetElement \
  mathml_dom/MathMLContentElement \
  mathml_dom/MathMLCnElement \
  mathml_dom/MathMLContentContainer \
  mathml_dom/MathMLMatrixrowElement \
  mathml_dom/MathMLCaseElement \
  mathml_dom/MathMLPiecewiseElement \
  mathml_dom/MathMLListElement \
  mathml_dom/MathMLTendsToElement \
  mathml_dom/MathMLFnElement \
  mathml_dom/MathMLLambdaElement \
  mathml_dom/MathMLCiElement \
  mathml_dom/MathMLCsymbolElement \
  mathml_dom/MathMLElement \
  mathml_dom/MathMLMatrixElement \
  mathml_dom/MathMLDeclareElement \
  mathml_dom/MathMLVectorElement \
  mathml_dom/MathMLContainer \
  mathml_dom/MathMLDocument \
  events/DocumentEvent \
  events/Event \
  events/EventListener \
  events/EventTarget \
  events/MutationEvent

enum_list = \
  cellml_api/VariableInterface

pure_java += \
  cellml_bootstrap/CellMLBootstrap

cellml_jar_java += \
  $(addprefix $(top_srcdir)/java/, $(addsuffix .java, $(pure_java))) \
  $(top_srcdir)/simple_interface_generators/glue/java/pjm/Reference.java \
  $(top_srcdir)/simple_interface_generators/glue/java/pjm/XPCOMDerived.java \
  $(addprefix $(top_builddir)/interfaces/, $(addsuffix .java, $(interface_list))) \
  $(addprefix $(top_builddir)/interfaces/, $(addsuffix .java, $(enum_list))) \
  $(addprefix $(top_builddir)/interfaces/pjm2pcm/, $(addsuffix .java, $(interface_list)))

cellml_jar_classes += \
  $(addprefix $(top_builddir)/javacp/, $(addsuffix .class, $(pure_java))) \
  $(top_builddir)/javacp/pjm/Reference.class \
  $(top_builddir)/javacp/pjm/XPCOMDerived.class \
  $(addprefix $(top_builddir)/javacp/, $(addsuffix .class, $(interface_list))) \
  $(addprefix $(top_builddir)/javacp/, $(addsuffix .class, $(enum_list))) \
  $(addprefix $(top_builddir)/javacp/pjm2pcm/, $(addsuffix .class, $(interface_list)))

$(top_builddir)/cellml.jar: $(cellml_jar_classes)
	SAVEDIR=`pwd` && \
	cd $(top_builddir)/javacp && \
	rm -f ../cellml.jar && \
	zip -r ../cellml.jar * && \
	cd $$SAVEDIR
$(cellml_jar_java): libcellml_java_bridge.la
$(cellml_jar_classes): $(cellml_jar_java)
	mkdir -p $(top_builddir)/javacp
	javac -cp "$(top_builddir)/interfaces:$(top_srcdir)/simple_interface_generators/glue/java:$(top_srcdir)/java" \
	-d $(top_builddir)/javacp $(cellml_jar_java)


BUILT_SOURCES += \
  $(top_builddir)/interfaces/p2jDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/p2jDOM_events.cpp \
  $(top_builddir)/interfaces/p2jMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/p2jCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/p2jCellML_events.cpp \
  $(top_builddir)/interfaces/p2jRDF_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pDOM_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pDOM_events.cpp \
  $(top_builddir)/interfaces/j2pMathML_content_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pCellML_APISPEC.cpp \
  $(top_builddir)/interfaces/j2pCellML_events.cpp \
  $(top_builddir)/interfaces/j2pRDF_APISPEC.cpp
dist_pkgdata_DATA = $(top_builddir)/cellml.jar
