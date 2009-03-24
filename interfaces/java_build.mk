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
$(top_srcdir)/interfaces/%.idl $(top_srcdir)/interfaces/Iface%.hxx
	SAVEDIR=`pwd` && \
	mkdir -p $(top_builddir)/interfaces && \
	cd $(top_builddir)/interfaces && \
	$(CYGWIN_WRAPPER) omniidl -p$$SAVEDIR/$(top_srcdir)/simple_interface_generators/omniidl_be \
         -bjava -DIN_CELLML_MODULE $$SAVEDIR/$< && \
	cd $$SAVEDIR

cellml_jar_java += \
  $(top_srcdir)/simple_interface_generators/glue/java/pjm/Reference.java \
  $(top_srcdir)/java/cellml_bootstrap/CellMLBootstrap.java \
  $(top_builddir)/interfaces/rdf_api/DataSource.java \
  $(top_builddir)/interfaces/rdf_api/Node.java \
  $(top_builddir)/interfaces/rdf_api/Resource.java \
  $(top_builddir)/interfaces/rdf_api/BlankNode.java \
  $(top_builddir)/interfaces/rdf_api/URIReference.java \
  $(top_builddir)/interfaces/rdf_api/Literal.java \
  $(top_builddir)/interfaces/rdf_api/PlainLiteral.java \
  $(top_builddir)/interfaces/rdf_api/TypedLiteral.java \
  $(top_builddir)/interfaces/rdf_api/Triple.java \
  $(top_builddir)/interfaces/rdf_api/TripleEnumerator.java \
  $(top_builddir)/interfaces/rdf_api/TripleSet.java \
  $(top_builddir)/interfaces/rdf_api/Bootstrap.java \
  $(top_builddir)/interfaces/rdf_api/RDFAPIRepresentation.java \
  $(top_builddir)/interfaces/dom/DOMImplementation.java \
  $(top_builddir)/interfaces/dom/Node.java \
  $(top_builddir)/interfaces/dom/NodeList.java \
  $(top_builddir)/interfaces/dom/NamedNodeMap.java \
  $(top_builddir)/interfaces/dom/CharacterData.java \
  $(top_builddir)/interfaces/dom/Attr.java \
  $(top_builddir)/interfaces/dom/Element.java \
  $(top_builddir)/interfaces/dom/Text.java \
  $(top_builddir)/interfaces/dom/Comment.java \
  $(top_builddir)/interfaces/dom/CDATASection.java \
  $(top_builddir)/interfaces/dom/DocumentType.java \
  $(top_builddir)/interfaces/dom/Notation.java \
  $(top_builddir)/interfaces/dom/Entity.java \
  $(top_builddir)/interfaces/dom/Document.java \
  $(top_builddir)/interfaces/dom/EntityReference.java \
  $(top_builddir)/interfaces/dom/ProcessingInstruction.java \
  $(top_builddir)/interfaces/dom/DocumentFragment.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLDOMImplementation.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLDocument.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLNodeList.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLContainer.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLMathElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLContentElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLContentToken.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLCnElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLCiElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLCsymbolElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLContentContainer.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLApplyElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLFnElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLLambdaElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLSetElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLListElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLBvarElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLPredefinedSymbol.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLTendsToElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLIntervalElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLConditionElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLDeclareElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLVectorElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLMatrixElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLMatrixrowElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLPiecewiseElement.java \
  $(top_builddir)/interfaces/mathml_dom/MathMLCaseElement.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RDFRepresentation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RDFXMLDOMRepresentation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RDFXMLStringRepresentation.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/UserData.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportInstantiationListener.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/URI.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLElement.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLDOMElement.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/NamedCellMLElement.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Model.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MathContainer.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLComponent.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Units.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Unit.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLImport.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportComponent.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportUnits.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLVariable.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ComponentRef.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RelationshipRef.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Group.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Connection.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MapComponents.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MapVariables.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Reaction.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/VariableRef.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/Role.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLElementIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MathMLElementIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ExtensionElementList.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MathList.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLElementSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/NamedCellMLElementSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ModelIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ModelSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLComponentIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLComponentSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportComponentIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportComponentSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLVariableIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLVariableSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/UnitsIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/UnitsSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportUnitsIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ImportUnitsSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLImportIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLImportSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/UnitIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/UnitSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ConnectionIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ConnectionSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/GroupIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/GroupSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RelationshipRefIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RelationshipRefSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ComponentRefIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ComponentRefSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MapVariablesIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/MapVariablesSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ReactionIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ReactionSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/VariableRefIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/VariableRefSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RoleIterator.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/RoleSet.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ModelLoadedListener.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/DocumentLoadedListener.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/ModelLoader.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/DOMURLLoader.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/DOMModelLoader.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_api/CellMLBootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/events/EventTarget.java \
  $(top_builddir)/interfaces/pjm2pcm/events/EventListener.java \
  $(top_builddir)/interfaces/pjm2pcm/events/Event.java \
  $(top_builddir)/interfaces/pjm2pcm/events/DocumentEvent.java \
  $(top_builddir)/interfaces/pjm2pcm/events/MutationEvent.java \
  $(top_builddir)/interfaces/pjm2pcm/cellml_events/MutationEvent.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/DataSource.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/Node.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/Resource.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/BlankNode.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/URIReference.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/Literal.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/PlainLiteral.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/TypedLiteral.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/Triple.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/TripleEnumerator.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/TripleSet.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/Bootstrap.java \
  $(top_builddir)/interfaces/pjm2pcm/rdf_api/RDFAPIRepresentation.java

cellml_jar_classes += \
  $(top_builddir)/javacp/pjm/Reference.class \
  $(top_builddir)/javacp/cellml_bootstrap/CellMLBootstrap.class \
  $(top_builddir)/javacp/rdf_api/DataSource.class \
  $(top_builddir)/javacp/rdf_api/Node.class \
  $(top_builddir)/javacp/rdf_api/Resource.class \
  $(top_builddir)/javacp/rdf_api/BlankNode.class \
  $(top_builddir)/javacp/rdf_api/URIReference.class \
  $(top_builddir)/javacp/rdf_api/Literal.class \
  $(top_builddir)/javacp/rdf_api/PlainLiteral.class \
  $(top_builddir)/javacp/rdf_api/TypedLiteral.class \
  $(top_builddir)/javacp/rdf_api/Triple.class \
  $(top_builddir)/javacp/rdf_api/TripleEnumerator.class \
  $(top_builddir)/javacp/rdf_api/TripleSet.class \
  $(top_builddir)/javacp/rdf_api/Bootstrap.class \
  $(top_builddir)/javacp/rdf_api/RDFAPIRepresentation.class \
  $(top_builddir)/javacp/dom/DOMImplementation.class \
  $(top_builddir)/javacp/dom/Node.class \
  $(top_builddir)/javacp/dom/NodeList.class \
  $(top_builddir)/javacp/dom/NamedNodeMap.class \
  $(top_builddir)/javacp/dom/CharacterData.class \
  $(top_builddir)/javacp/dom/Attr.class \
  $(top_builddir)/javacp/dom/Element.class \
  $(top_builddir)/javacp/dom/Text.class \
  $(top_builddir)/javacp/dom/Comment.class \
  $(top_builddir)/javacp/dom/CDATASection.class \
  $(top_builddir)/javacp/dom/DocumentType.class \
  $(top_builddir)/javacp/dom/Notation.class \
  $(top_builddir)/javacp/dom/Entity.class \
  $(top_builddir)/javacp/dom/Document.class \
  $(top_builddir)/javacp/dom/EntityReference.class \
  $(top_builddir)/javacp/dom/ProcessingInstruction.class \
  $(top_builddir)/javacp/dom/DocumentFragment.class \
  $(top_builddir)/javacp/mathml_dom/MathMLDOMImplementation.class \
  $(top_builddir)/javacp/mathml_dom/MathMLDocument.class \
  $(top_builddir)/javacp/mathml_dom/MathMLNodeList.class \
  $(top_builddir)/javacp/mathml_dom/MathMLElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLContainer.class \
  $(top_builddir)/javacp/mathml_dom/MathMLMathElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLContentElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLContentToken.class \
  $(top_builddir)/javacp/mathml_dom/MathMLCnElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLCiElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLCsymbolElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLContentContainer.class \
  $(top_builddir)/javacp/mathml_dom/MathMLApplyElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLFnElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLLambdaElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLSetElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLListElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLBvarElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLPredefinedSymbol.class \
  $(top_builddir)/javacp/mathml_dom/MathMLTendsToElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLIntervalElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLConditionElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLDeclareElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLVectorElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLMatrixElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLMatrixrowElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLPiecewiseElement.class \
  $(top_builddir)/javacp/mathml_dom/MathMLCaseElement.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RDFRepresentation.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RDFXMLDOMRepresentation.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RDFXMLStringRepresentation.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/UserData.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportInstantiationListener.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/URI.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLElement.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLDOMElement.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/NamedCellMLElement.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Model.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MathContainer.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLComponent.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Units.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Unit.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLImport.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportComponent.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportUnits.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLVariable.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ComponentRef.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RelationshipRef.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Group.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Connection.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MapComponents.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MapVariables.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Reaction.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/VariableRef.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/Role.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLElementIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MathMLElementIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ExtensionElementList.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MathList.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLElementSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/NamedCellMLElementSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ModelIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ModelSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLComponentIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLComponentSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportComponentIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportComponentSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLVariableIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLVariableSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/UnitsIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/UnitsSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportUnitsIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ImportUnitsSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLImportIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLImportSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/UnitIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/UnitSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ConnectionIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ConnectionSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/GroupIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/GroupSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RelationshipRefIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RelationshipRefSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ComponentRefIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ComponentRefSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MapVariablesIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/MapVariablesSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ReactionIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ReactionSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/VariableRefIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/VariableRefSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RoleIterator.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/RoleSet.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ModelLoadedListener.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/DocumentLoadedListener.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/ModelLoader.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/DOMURLLoader.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/DOMModelLoader.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_api/CellMLBootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/events/EventTarget.class \
  $(top_builddir)/javacp/pjm2pcm/events/EventListener.class \
  $(top_builddir)/javacp/pjm2pcm/events/Event.class \
  $(top_builddir)/javacp/pjm2pcm/events/DocumentEvent.class \
  $(top_builddir)/javacp/pjm2pcm/events/MutationEvent.class \
  $(top_builddir)/javacp/pjm2pcm/cellml_events/MutationEvent.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/DataSource.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/Node.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/Resource.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/BlankNode.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/URIReference.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/Literal.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/PlainLiteral.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/TypedLiteral.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/Triple.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/TripleEnumerator.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/TripleSet.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/Bootstrap.class \
  $(top_builddir)/javacp/pjm2pcm/rdf_api/RDFAPIRepresentation.class

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
  $(top_builddir)/interfaces/j2pRDF_APISPEC.cpp \
  $(top_builddir)/cellml.jar
