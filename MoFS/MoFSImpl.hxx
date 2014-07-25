#include "IfaceMoFS.hxx"
#include "IfaceAnnoTools.hxx"
#include "IfaceCeVAS.hxx"
#include "Utilities.hxx"
#include <set>

class CDA_ModelFlatteningService
  : public iface::mofs::ModelFlatteningService
{
public:
  CDA_ModelFlatteningService() {};
  CDA_IMPL_QI1(mofs::ModelFlatteningService);
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  already_AddRefd<iface::mofs::ModelFlattener> createFlattener()
    throw();
};

class CDA_ModelFlattener
  : public iface::mofs::ModelFlattener
{
public:
  CDA_ModelFlattener() {};
  CDA_IMPL_QI1(mofs::ModelFlattener);
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  already_AddRefd<iface::cellml_api::Model> flatten(iface::cellml_api::Model* aModel)
    throw(std::exception&);
  std::wstring lastError() throw();

private:
  std::wstring mLastError;
};

class CDA_MoFSFailure
{
public:
  CDA_MoFSFailure(std::wstring aWhy) : mWhy(aWhy) {};
  std::wstring why() { return mWhy; }

private:
  std::wstring mWhy;
};

class CDA_MoFSConverter
{
private:
  ObjRef<iface::cellml_api::Model>
    // The model we're converting
    mModelIn,
    // The model we're creating
    mModelOut;

  // Manager for annotations on models
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;

  // Units we have already copied into the new model
  std::set<std::pair<std::wstring,std::wstring> > mCopiedUnits;

  // Component names used in the new model, to avoid duplicates
  std::set<std::wstring> mCompNames;

    /**
     * A utility method which employs CeVAS's algorithm to find the
     * 'real' component for the given component.  That is, if the
     * component is an ImportComponent, the template CellMLComponent
     * that it is based on in an imported model.  Otherwise returns
     * the given component.
     *
     * Requires imports to have been fully instantiated.  Returns NULL
     * if the real component does not exist.
     *
     * Also requires that a component is not imported more than once
     * in the same import element, as then multiple components share
     * one real component.
     */
  already_AddRefd<iface::cellml_api::CellMLComponent>
  FindRealComponent(iface::cellml_api::CellMLComponent* component);

  /**
   * Annotate all imported components (whether direct or indirect)
   * with the name they are given by the importing model.
   */
  void StoreImportRenamings(iface::cellml_api::Model* importingModel);

  /**
   * Ensure that component names in the generated model are unique.
   *
   * If the given name has already been used, it is modified so as
   * to be unique, by appending the string "_n" where n is the least
   * natural number giving an unused name.
   */
  void EnsureComponentNameUnique(std::wstring& cname);

  /**
   * Copy any relevant connections into the new model.
   *
   * Copies across any connections defined in the given model
   * between 2 components which have been copied, and recursively
   * processes imported models.
   */
  void CopyConnections(iface::cellml_api::Model* model);

  /**
   * Copy a connection, possibly involving imported components, into
   * our new model.
   *
   * The connection will only be copied if both components involved
   * have been copied across first, and hence have a "copy"
   * annotation.
   */
  void CopyConnection(iface::cellml_api::Connection* conn);

  /**
   * Copy all the units in the given set into the provided model/component.
   *
   * The target is in a different model to the units, so we can't use clone.
   */
  void CopyUnits(iface::cellml_api::UnitsSet* unitset,
                 iface::cellml_api::CellMLElement* target);

  /**
   * Copy any extension elements.
   */
  void CopyExtensionElements(iface::cellml_api::CellMLElement* from,
                             iface::cellml_api::CellMLElement* to);

  /**
   * Copy relevant components into the output model, using the given
   * CeVAS to find components to copy.
   */
  void CopyComponents(iface::cellml_services::CeVAS* cevas);

  /**
   * Copy the given component into the given model.
   *
   * We create a new component, and manually transfer the content.
   */
  void CopyComponent(iface::cellml_api::CellMLComponent* comp,
                     iface::cellml_api::Model* model);

  /**
   * This method is used to reconstruct the encapsulation hierarchy
   * in the new model, recursively processing imported models.
   *
   * It uses the CopyGroup method to read the encapsulation
   * hierarchy, and if it finds a reference to a component that has
   * been copied over, it creates a new group containing the
   * hierarchy rooted at that point.
   */
  void CopyGroups(iface::cellml_api::Model* model);

  /**
   * This method does the actual copying of groups.
   */
  void CopyGroup(iface::cellml_api::Model* model,
                 iface::cellml_api::ComponentRefSet* crefs,
                 iface::cellml_api::ComponentRef* copyInto);

  /**
   * Create and return a (manual) deep copy of the given DOM element, changing
   * any CellML 1.1 namespaces to CellML 1.0 in the process.
   *
   * If the element is in the MathML namespace, it will be converted
   * to a iface::cellml_api::MathMLElement instance prior to being returned.
   *
   * Note: it appears that when adding a child element or text node,
   * which we are subsequently going to release_ref ourselves, we
   * must also release_ref the parent, or its refcount never reaches
   * zero.
   */
  already_AddRefd<iface::dom::Element> CopyDOMElement(iface::dom::Element* in);

  /**
   * Create and return a (manual) deep copy of the given MathML element.
   */
  already_AddRefd<iface::mathml_dom::MathMLElement>
  CopyMathElement(iface::cellml_api::MathMLElement in);


  /**
   * Try to make all initial_value attributes valid CellML 1.0.
   *
   * Where a variable is specified, look at its source variable.  If
   * it has a numeric initial value, use that.  If not, it's an
   * unavoidable error condition.
   */
  void PropagateInitialValues();

public:
  /**
   * The main interface to the converter: creates and returns a new
   * model which is a CellML 1.0 version of the input.
   *
   * The input model must have had all imports fully instantiated.
   */
  already_AddRefd<iface::cellml_api::Model> ConvertModel(iface::cellml_api::Model* modelIn);
};
