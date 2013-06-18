#define IN_MOFS_MODULE
#include "MoFSImpl.hxx"
#include <sstream>
#include "CellMLBootstrap.hpp"
#include "AnnoToolsBootstrap.hpp"
#include "CeVASBootstrap.hpp"
#include "MoFSBootstrap.hpp"

/*
This file includes software derived from software by Jonathan Cooper and David
Nickerson. David Nickerson has agreed for his code to be licensed under the same
terms as the rest of the CellML API. Jonathan Cooper's code was licensed under the
following terms:

Copyright (c) 2010, The Chancellor, Masters and Scholars of the University of Oxford
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

o Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

o Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

o Neither the name of the University of Oxford nor the names of its contributors may be
used to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// XML Namespaces
#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"
#define CELLML_1_0_NS L"http://www.cellml.org/cellml/1.0#"
#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"

already_AddRefd<iface::mofs::ModelFlatteningService>
CreateModelFlatteningService()
{
  return new CDA_ModelFlatteningService();
}

already_AddRefd<iface::mofs::ModelFlattener>
CDA_ModelFlatteningService::createFlattener()
  throw()
{
  return new CDA_ModelFlattener();
}

already_AddRefd<iface::cellml_api::Model>
CDA_ModelFlattener::flatten(iface::cellml_api::Model* aModel)
  throw(std::exception&)
{
  mLastError = L"";

  try
  {
    aModel->fullyInstantiateImports();
  }
  catch (iface::cellml_api::CellMLException& e)
  {
    mLastError = L"Problem fully instantiating imports";
    throw e;
  }

  CDA_MoFSConverter converter;
  try
  {
    return converter.ConvertModel(aModel);
  }
  catch (CDA_MoFSFailure& eFail)
  {
    mLastError = eFail.why();
    throw iface::cellml_api::CellMLException(eFail.why());
  }
}

std::wstring
CDA_ModelFlattener::lastError() throw()
{
  return mLastError;
}

already_AddRefd<iface::cellml_api::CellMLComponent>
CDA_MoFSConverter::FindRealComponent(iface::cellml_api::CellMLComponent* aComponent)
{
  ObjRef<iface::cellml_api::CellMLComponent> component(aComponent);
  while (true)
  {
    ObjRef<iface::cellml_api::ImportComponent> impc(QueryInterface(component));
    if (impc == NULL)
      break; // Found a non-ImportComponent
    ObjRef<iface::cellml_api::CellMLElement> imp_elt(impc->parentElement());
    ObjRef<iface::cellml_api::CellMLImport> imp(QueryInterface(imp_elt));
    ObjRef<iface::cellml_api::Model> model(imp->importedModel());
    assert(model != NULL);
    ObjRef<iface::cellml_api::CellMLComponentSet> comps(model->modelComponents());
    std::wstring ref = impc->componentRef();
    component = comps->getComponent(ref);
    if (component == NULL)
      break; // Real component does not exist
  }

  return component.returnNewReference();
}

void
CDA_MoFSConverter::StoreImportRenamings(iface::cellml_api::Model* importingModel)
{
  ObjRef<iface::cellml_api::CellMLImportSet> imps(importingModel->imports());
  ObjRef<iface::cellml_api::CellMLImportIterator> impi(imps->iterateImports());
  for (ObjRef<iface::cellml_api::CellMLImport> import(impi->nextImport());
       import; import = impi->nextImport())
  {
    assert(import->wasInstantiated());
    ObjRef<iface::cellml_api::Model> importedModel(import->importedModel());

    // Recursively process components imported by the imported
    // model.  Depth first processing means components renamed
    // twice end up with the right name.
    StoreImportRenamings(importedModel);

    ObjRef<iface::cellml_api::ImportComponentSet> ics(import->components());
    ObjRef<iface::cellml_api::ImportComponentIterator> ici(ics->iterateImportComponents());
    for (ObjRef<iface::cellml_api::ImportComponent> comp(ici->nextImportComponent());
         comp; comp = ici->nextImportComponent())
    {
      // Now get the component object to annotate, using the
      // same algorithm as CeVAS.
      ObjRef<iface::cellml_api::CellMLComponent> realComp(FindRealComponent(comp));
      mAnnoSet->setStringAnnotation(realComp, L"renamed", comp->name());
    }
  }
}

void
CDA_MoFSConverter::EnsureComponentNameUnique(std::wstring& cname)
{
  std::wstring suffix = L"";
  unsigned n = 0;
  while (mCompNames.find(cname + suffix) != mCompNames.end())
  {
    // Current attempt already used; try the next
    std::wstringstream _suffix;
    _suffix << L"_" << (++n);
    _suffix >> suffix;
  }
  cname += suffix;
  mCompNames.insert(cname);
}

void
CDA_MoFSConverter::CopyConnections(iface::cellml_api::Model* model)
{
  // Copy local connections
  ObjRef<iface::cellml_api::ConnectionSet> cs(model->connections());
  ObjRef<iface::cellml_api::ConnectionIterator> ci(cs->iterateConnections());
  for (ObjRef<iface::cellml_api::Connection> conn(ci->nextConnection()); conn;
       conn = ci->nextConnection())
    CopyConnection(conn);

  // Process imported models
  ObjRef<iface::cellml_api::CellMLImportSet> imps(model->imports());
  ObjRef<iface::cellml_api::CellMLImportIterator> impi(imps->iterateImports());
  for (ObjRef<iface::cellml_api::CellMLImport> import(impi->nextImport());
       import; import = impi->nextImport())
  {
    ObjRef<iface::cellml_api::Model> impModel(import->importedModel());
    assert(impModel != NULL);
    CopyConnections(impModel);
  }
}

void
CDA_MoFSConverter::CopyConnection(iface::cellml_api::Connection* conn)
{
  ObjRef<iface::cellml_api::MapComponents> mc(conn->componentMapping());
  ObjRef<iface::cellml_api::CellMLComponent> c1(mc->firstComponent());
  ObjRef<iface::cellml_api::CellMLComponent> c2(mc->secondComponent());
  
  // Check we've copied the components involved, and get the
  // copies.
  ObjRef<iface::cellml_api::CellMLComponent> newc1 =
    QueryInterface(mAnnoSet->getObjectAnnotation(c1, L"copy"));
  if (newc1 == NULL)
    return;

  ObjRef<iface::cellml_api::CellMLComponent> newc2 =
    QueryInterface(mAnnoSet->getObjectAnnotation(c2, L"copy"));
  if (newc2 == NULL)
    return;
  
  // Create a new connection
  ObjRef<iface::cellml_api::Connection> newconn
    (mModelOut->createConnection());

  mModelOut->addElement(newconn);
  ObjRef<iface::cellml_api::MapComponents>
    newmc(newconn->componentMapping());
  newmc->firstComponentName(newc1->name());
  newmc->secondComponentName(newc2->name());

  // Add the variable maps
  ObjRef<iface::cellml_api::MapVariablesSet> varmaps(conn->variableMappings());
  ObjRef<iface::cellml_api::MapVariablesIterator> varmapi(varmaps->iterateMapVariables());
  for (ObjRef<iface::cellml_api::MapVariables> varmap(varmapi->nextMapVariable());
       varmap; varmap = varmapi->nextMapVariable())
  {
    ObjRef<iface::cellml_api::MapVariables>
      newMap(mModelOut->createMapVariables());
    newMap->firstVariableName(varmap->firstVariableName());
    newMap->secondVariableName(varmap->secondVariableName());
    newconn->addElement(newMap);
  }
}

void
CDA_MoFSConverter::CopyUnits(iface::cellml_api::UnitsSet* unitsset,
                             iface::cellml_api::CellMLElement* target)
{
  ObjRef<iface::cellml_api::Model> model(target->modelElement());
  
  ObjRef<iface::cellml_api::UnitsIterator> unitsIt(unitsset->iterateUnits());
  for (ObjRef<iface::cellml_api::Units> units(unitsIt->nextUnits());
       units; units = unitsIt->nextUnits())
  {
    ObjRef<iface::cellml_api::Model> unitsModel(units->modelElement());

    // Don't copy units defined in this model already
    if (!CDA_objcmp(unitsModel, model))
      continue;

    // Don't copy units we've already copied.
    // This is rather hackish, and probably not spec-compliant.
    std::pair<std::wstring, std::wstring> p(units->name(), unitsModel->name());
    if (mCopiedUnits.count(p) != 0)
      continue; // Duplicate units.
    mCopiedUnits.insert(p);

    ObjRef<iface::cellml_api::Units> newUnits(model->createUnits());
    newUnits->name(units->name());
    newUnits->isBaseUnits(units->isBaseUnits());
    
    // Copy each unit reference
    ObjRef<iface::cellml_api::UnitSet> unitset(units->unitCollection());
    ObjRef<iface::cellml_api::UnitIterator> uniti(unitset->iterateUnits());
    for (ObjRef<iface::cellml_api::Unit> unit(uniti->nextUnit());
         unit; unit = uniti->nextUnit())
    {
      ObjRef<iface::cellml_api::Unit> newUnit(model->createUnit());
      newUnit->prefix(unit->prefix());
      newUnit->multiplier(unit->multiplier());
      newUnit->offset(unit->offset());
      newUnit->exponent(unit->exponent());
      newUnit->units(unit->units());
      // Add
      newUnits->addElement(newUnit);
    }
    
    // And add to target
    target->addElement(newUnits);
  }
}

void
CDA_MoFSConverter::CopyExtensionElements(iface::cellml_api::CellMLElement* from,
                                         iface::cellml_api::CellMLElement* to)
{
  ObjRef<iface::cellml_api::ExtensionElementList> elts(from->extensionElements());
  ObjRef<iface::cellml_api::CellMLDOMElement> del(QueryInterface(to));
  ObjRef<iface::dom::Element> toEl(del->domElement());
  ObjRef<iface::dom::Document> toDoc(toEl->ownerDocument());

  for (unsigned long i=0; i<elts->length(); i++)
  {
    ObjRef<iface::dom::Element> elt(elts->getAt(i));

    std::wstring name(elt->nodeName());

    ObjRef<iface::dom::Node> copyNode(toDoc->importNode(elt, true));
    ObjRef<iface::dom::Element> copy(QueryInterface(copyNode));
    to->appendExtensionElement(copy);
  }
}

void
CDA_MoFSConverter::CopyComponents(iface::cellml_services::CeVAS* cevas)
{
  ObjRef<iface::cellml_api::CellMLComponentIterator> relCompi(cevas->iterateRelevantComponents());
  for (ObjRef<iface::cellml_api::CellMLComponent> comp(relCompi->nextComponent());
       comp; comp = relCompi->nextComponent())
    CopyComponent(comp, mModelOut);
}

void
CDA_MoFSConverter::CopyComponent(iface::cellml_api::CellMLComponent* comp,
                                 iface::cellml_api::Model* model)
{
  std::wstring cname(comp->name());

  // Paranoia: check we haven't already copied it
  ObjRef<iface::cellml_api::CellMLComponent> copy
    (QueryInterface(mAnnoSet->getObjectAnnotation(comp, L"copy")));

  if (copy != NULL)
    return;

  ObjRef<iface::cellml_api::Model> sourceModel(comp->modelElement());
    
  // Create the new component and set its name & id
  copy = model->createComponent();
  mAnnoSet->setObjectAnnotation(comp, L"copy", copy);

  // Check for a renaming
  std::wstring renamed(mAnnoSet->getStringAnnotation(comp, L"renamed"));
  if (renamed != L"")
  {
    // It was an imported component, and may have been renamed
    cname = renamed;
  }

  // Ensure name is unique in the 1.0 model
  EnsureComponentNameUnique(cname);

  copy->name(cname);

  std::wstring cmetaId(comp->cmetaId());
  if (cmetaId != L"")
    copy->cmetaId(cmetaId);
  
  // Copy units
  ObjRef<iface::cellml_api::UnitsSet> units(comp->units());
  CopyUnits(units, copy);
        
  // Copy variables
  ObjRef<iface::cellml_api::CellMLVariableSet> vars(comp->variables());
  ObjRef<iface::cellml_api::CellMLVariableIterator> vari(vars->iterateVariables());
  for (ObjRef<iface::cellml_api::CellMLVariable> var(vari->nextVariable());
       var; var = vari->nextVariable())
  {
    ObjRef<iface::cellml_api::CellMLVariable> varCopy(model->createCellMLVariable());
    
    varCopy->name(var->name());
    std::wstring cmetaId = var->cmetaId();
    if (cmetaId != L"")
      varCopy->cmetaId(cmetaId);
    varCopy->initialValue(var->initialValue());
    varCopy->privateInterface(var->privateInterface());
    varCopy->publicInterface(var->publicInterface());
    varCopy->unitsName(var->unitsName());
    copy->addElement(varCopy);
  }

  // Copy mathematics
  ObjRef<iface::cellml_api::MathList> mathlist(comp->math());
  ObjRef<iface::cellml_api::MathMLElementIterator> mei(mathlist->iterate());
  for (ObjRef<iface::mathml_dom::MathMLElement> mel(mei->next()); mel;
       mel = mei->next())
  {
    ObjRef<iface::mathml_dom::MathMLElement> newMel(CopyMathElement(mel));
    copy->addMath(newMel);
  }

  // Copy extension elements (TODO: & attributes)
  CopyExtensionElements(comp, copy);

  // Add copy to model
  model->addElement(copy);
}

void
CDA_MoFSConverter::CopyGroups(iface::cellml_api::Model* model)
{
  // Iterate only groups defining the encapsulation hierarchy.
  ObjRef<iface::cellml_api::GroupSet> groups(model->groups());
  ObjRef<iface::cellml_api::GroupSet> groupsenc(groups->subsetInvolvingEncapsulation());
  ObjRef<iface::cellml_api::GroupIterator> groupsenci(groupsenc->iterateGroups());
  for (ObjRef<iface::cellml_api::Group> group(groupsenci->nextGroup());
       group; group = groupsenci->nextGroup())
  {
    // Now recurse down this subtree
    ObjRef<iface::cellml_api::ComponentRefSet> crefs(group->componentRefs());
    CopyGroup(model, crefs, NULL);
  }
  
  // Now check imported models
  ObjRef<iface::cellml_api::CellMLImportSet> imports(model->imports());
  ObjRef<iface::cellml_api::CellMLImportIterator> importi(imports->iterateImports());
  for (ObjRef<iface::cellml_api::CellMLImport> import(importi->nextImport());
       import; import = importi->nextImport())
  {
    ObjRef<iface::cellml_api::Model> impModel(import->importedModel());
    assert(impModel != NULL);
    CopyGroups(impModel);
  }
}

already_AddRefd<iface::dom::Element>
CDA_MoFSConverter::CopyDOMElement(iface::dom::Element* in)
{
  std::wstring nsURI = in->namespaceURI();
  std::wstring qname = in->nodeName();

  // Create a blank copied element
  ObjRef<iface::dom::Element> out =
    mModelOut->createExtensionElement(nsURI.c_str(),
                                      qname.c_str());
  ObjRef<iface::dom::Document> doc = out->ownerDocument();

  // Copy attributes
  ObjRef<iface::dom::NamedNodeMap> attrs = in->attributes();

  for (unsigned long i=0; i<attrs->length(); ++i)
  {
    ObjRef<iface::dom::Attr> attr = QueryInterface(attrs->item(i));

    std::wstring attr_ns = attr->namespaceURI();

    if (attr_ns == CELLML_1_1_NS)
      attr_ns = CELLML_1_0_NS;

    std::wstring attr_name = attr->name();
    ObjRef<iface::dom::Attr> copy = doc->createAttributeNS(attr_ns, attr_name);
    copy->value(attr->value());
    ObjRef<iface::dom::Attr> tmp(out->setAttributeNodeNS(copy));
  }

  // Copy child elements & text
  ObjRef<iface::dom::NodeList> children = in->childNodes();
  for (unsigned long i=0; i<children->length(); ++i)
  {
    ObjRef<iface::dom::Node> child = children->item(i);
    switch (child->nodeType())
    {
    case iface::dom::Node::ELEMENT_NODE:
      {
        ObjRef<iface::dom::Element> elt = QueryInterface(child);
        ObjRef<iface::dom::Element> copy = CopyDOMElement(elt);
        out->appendChild(copy)->release_ref();
      }
      break;
    case iface::dom::Node::TEXT_NODE:
      {
        std::wstring text = child->nodeValue();
        ObjRef<iface::dom::Text> textNode = doc->createTextNode(text);
        out->appendChild(textNode)->release_ref();
      }
      break;
    }
  }

  return out.returnNewReference();
}

already_AddRefd<iface::mathml_dom::MathMLElement>
CDA_MoFSConverter::CopyMathElement(iface::mathml_dom::MathMLElement* aIn)
{
  ObjRef<iface::mathml_dom::MathMLElement> mel(QueryInterface(CopyDOMElement(aIn)));
  return mel.returnNewReference();
}

void
CDA_MoFSConverter::CopyGroup
(
 iface::cellml_api::Model* model,
 iface::cellml_api::ComponentRefSet* crefs,
 iface::cellml_api::ComponentRef* copyInto                             
)
{
  ObjRef<iface::cellml_api::CellMLComponentSet> comps(model->modelComponents());

  ObjRef<iface::cellml_api::ComponentRefIterator> crefi(crefs->iterateComponentRefs());
  for (ObjRef<iface::cellml_api::ComponentRef> cref(crefi->nextComponentRef());
       cref; cref = crefi->nextComponentRef())
  {
    // Find the referenced component
    ObjRef<iface::cellml_api::CellMLComponent> comp
      (comps->getComponent(cref->componentName()));
    if (comp == NULL)
      continue;

    // Find the real component object
    ObjRef<iface::cellml_api::CellMLComponent> realComp(FindRealComponent(comp));

    // Has it been copied?
    ObjRef<iface::cellml_api::CellMLComponent> copy =
      QueryInterface(mAnnoSet->getObjectAnnotation(realComp, L"copy"));

    if (copy == NULL)
      continue;

    // Create a component ref for the copy
    ObjRef<iface::cellml_api::ComponentRef> newref
      (mModelOut->createComponentRef());
    newref->componentName(copy->name());

    if (copyInto == NULL)
    {
      // Create a new group
      ObjRef<iface::cellml_api::Group> group(mModelOut->createGroup());
      mModelOut->addElement(group);

      ObjRef<iface::cellml_api::RelationshipRef> rref
        (mModelOut->createRelationshipRef());
      rref->setRelationshipName(L"", L"encapsulation");
      group->addElement(rref);

      // Add this component as the root
      group->addElement(newref);
    }
    else
    {
      // Add this component into the existing group
      copyInto->addElement(newref);
    }

    // Copy any children of this component
    ObjRef<iface::cellml_api::ComponentRefSet> childrefs
      (cref->componentRefs());
    CopyGroup(model, childrefs, newref);
  }
}

void
CDA_MoFSConverter::PropagateInitialValues()
{
  ObjRef<iface::cellml_api::CellMLComponentSet> comps(mModelOut->localComponents());
  ObjRef<iface::cellml_api::CellMLComponentIterator> compi(comps->iterateComponents());
  for (ObjRef<iface::cellml_api::CellMLComponent> comp(compi->nextComponent());
       comp; comp = compi->nextComponent())
  {
    ObjRef<iface::cellml_api::CellMLVariableSet> vars(comp->variables());
    ObjRef<iface::cellml_api::CellMLVariableIterator> vari(vars->iterateVariables());
    for (ObjRef<iface::cellml_api::CellMLVariable> var(vari->nextVariable());
         var; var = vari->nextVariable())
    {
      std::wstring init(var->initialValue());
      if (init == L"")
        continue;

      // Is the initial value a double?
      std::wstringstream init_(init);
      double value;
      init_ >> value;
      if (init_.fail())
      {
        // Find the initial variable
        ObjRef<iface::cellml_api::CellMLVariableSet> vars(comp->variables());
        ObjRef<iface::cellml_api::CellMLVariable> initvar(vars->getVariable(init));
        if (initvar == NULL)
          throw CDA_MoFSFailure(L"Could not find variable " + init + L" from initial_value attribute on variable " +
                                var->name() + L" in component " + comp->name());

        // Find its source
        ObjRef<iface::cellml_api::CellMLVariable> src
          (initvar->sourceVariable());

        // And copy the initial value
        var->initialValue(src->initialValue());
      }
    }
  }
}

already_AddRefd<iface::cellml_api::Model>
CDA_MoFSConverter::ConvertModel(iface::cellml_api::Model* modelIn)
{
  mModelIn = modelIn;

  // Create the output model
  ObjRef<iface::cellml_api::CellMLBootstrap> cbs
    (CreateCellMLBootstrap());
  mModelOut = cbs->createModel(L"1.0");

  // Set name & id
  mModelOut->name(modelIn->name());

  std::wstring modelId(modelIn->cmetaId());
  if (modelId != L"")
    mModelOut->cmetaId(modelId);

  // Create an annotation set to manage annotations
  ObjRef<iface::cellml_services::AnnotationToolService> ats(CreateAnnotationToolService());
  ObjRef<iface::cellml_services::AnnotationSet> annoset(ats->createAnnotationSet());
  mAnnoSet = annoset;

  // Create a CeVAS to find relevant components
  ObjRef<iface::cellml_services::CeVASBootstrap> cevas_bs(CreateCeVASBootstrap());
  ObjRef<iface::cellml_services::CeVAS> cevas(cevas_bs->createCeVASForModel(modelIn));

  std::wstring err(cevas->modelError());
  if (err != L"")
    throw CDA_MoFSFailure(L"Error creating CeVAS: " + err);

  // Copy model-level units to the new model, both local and
  // imported definitions.
  ObjRef<iface::cellml_api::UnitsSet> units(modelIn->allUnits());
  CopyUnits(units, mModelOut);

  // Annotate potentially renamed components
  StoreImportRenamings(modelIn);

  // Copy all needed components to the new model
  CopyComponents(cevas);

  // Copy connections
  CopyConnections(modelIn);

  // Copy groups
  CopyGroups(modelIn);

  // Deal with 'initial_value="var_name"' occurrences
  PropagateInitialValues();

  // And finally, return the result
  return mModelOut.returnNewReference();
}
