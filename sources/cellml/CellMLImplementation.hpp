#include <exception>
#include <string>
#include <map>
#include <wchar.h>
#include "Utilities.hxx"
#include "Ifacexpcom.hxx"
#include "IfaceDOM-APISPEC.hxx"
#include "IfaceCellML-APISPEC.hxx"
#include <typeinfo>

class CDA_RDFXMLDOMRepresentation
  : public iface::cellml_api::RDFXMLDOMRepresentation
{
public:
  CDA_RDFXMLDOMRepresentation(iface::dom::Element* idata);
  virtual ~CDA_RDFXMLDOMRepresentation();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RDFRepresentation, cellml_api::RDFXMLDOMRepresentation)

  wchar_t* type() throw(std::exception&);
  iface::dom::Element* data() throw(std::exception&);
private:
  iface::dom::Element* datastore;
};

class CDA_RDFXMLStringRepresentation
  : public iface::cellml_api::RDFXMLStringRepresentation
{
public:
  CDA_RDFXMLStringRepresentation(iface::dom::Element* idata);
  virtual ~CDA_RDFXMLStringRepresentation();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RDFRepresentation, cellml_api::RDFXMLStringRepresentation)

  wchar_t* type() throw(std::exception&);
  wchar_t* serialisedData() throw(std::exception&);
  void serialisedData(const wchar_t* attr) throw(std::exception&);
private:
  iface::dom::Element* datastore;
};

class CDA_URI
  : public iface::cellml_api::URI
{
public:
  CDA_URI(iface::dom::Attr* idata);
  virtual ~CDA_URI();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::URI)

  wchar_t* asText() throw(std::exception&);
  void asText(const wchar_t* attr) throw(std::exception&);
private:
  iface::dom::Attr* datastore;
};

class CDA_CellMLElementSet;

class CDA_CellMLElement
  : public virtual iface::cellml_api::CellMLElement
{
public:
  CDA_CellMLElement(iface::XPCOM::IObject* parent, iface::dom::Element* idata);
  virtual ~CDA_CellMLElement();

  void add_ref()
    throw(std::exception&)
  {
    _cda_refcount++;
    
    if (mParent != NULL)
      mParent->add_ref();
  }

  void release_ref()
    throw(std::exception&)
  {
    if (_cda_refcount == 0)
    {
      printf("Warning: release_ref called too many times on %s.\n",
             typeid(this).name());
    }
    _cda_refcount--;
    if (mParent == NULL)
    {
      if (_cda_refcount == 0)
        delete this;
    }
    else /* if the owner model is non-null, we will be destroyed when there are
          * no remaining references to the model.
          */
      mParent->release_ref();
  }

  wchar_t* cellmlVersion()
    throw(std::exception&);
  wchar_t* cmetaId()
    throw(std::exception&);
  void cmetaId(const wchar_t* attr)
    throw(std::exception&);
  iface::cellml_api::RDFRepresentation*
    getRDFRepresentation(const wchar_t* type) throw(std::exception&);
  iface::cellml_api::ExtensionElementList* extensionElements()
    throw(std::exception&);
  void insertExtensionElementAfter(const iface::cellml_api::ExtensionElement marker,
                                   const iface::cellml_api::ExtensionElement newEl)
    throw(std::exception&);
  void appendExtensionElement(const iface::cellml_api::ExtensionElement x) throw(std::exception&);
  void prependExtensionElement(const iface::cellml_api::ExtensionElement x) throw(std::exception&);
  void removeExtensionElement(const iface::cellml_api::ExtensionElement x) throw(std::exception&);
  void replaceExtensionElement(const iface::cellml_api::ExtensionElement x, const iface::cellml_api::ExtensionElement y) throw(std::exception&);
  void clearExtensionElements() throw(std::exception&);
  iface::cellml_api::CellMLElementSet* childElements() throw(std::exception&);
  void addElement(iface::cellml_api::CellMLElement* x) throw(std::exception&);
  void removeElement(iface::cellml_api::CellMLElement* x) throw(std::exception&);
  void replaceElement(iface::cellml_api::CellMLElement* x, iface::cellml_api::CellMLElement* y) throw(std::exception&);
  void removeByName(const wchar_t* type, const wchar_t* name) throw(std::exception&);
  bool validate() throw(std::exception&);
  iface::cellml_api::CellMLElement* parentElement() throw(std::exception&);
  iface::cellml_api::Model* modelElement() throw(std::exception&);
  void setUserData(const wchar_t* key, iface::cellml_api::UserData* data) throw(std::exception&);
  iface::cellml_api::UserData* getUserData(const wchar_t* key) throw(std::exception&);

  iface::XPCOM::IObject* mParent;
  iface::dom::Element* datastore;
protected:
  friend class CDA_CellMLElementSet;
  // This is used only when ownerModel is null. When ownerModel is assigned,
  // add_ref() must be called on ownerModel the correct number of times.
  u_int32_t _cda_refcount;

  CDA_CellMLElementSet* children;

  std::map<std::wstring,iface::cellml_api::UserData*> userData;
};

class CDA_NamedCellMLElement
  : public virtual iface::cellml_api::NamedCellMLElement,
    public virtual CDA_CellMLElement
{
public:
  CDA_NamedCellMLElement(iface::XPCOM::IObject* parent, iface::dom::Element* idata);
  virtual ~CDA_NamedCellMLElement();

  wchar_t* name() throw(std::exception&);
  void name(const wchar_t* attr) throw(std::exception&);
};

class CDA_Model
  : public virtual iface::cellml_api::Model,
    public CDA_NamedCellMLElement,
    public WeakReferenceTarget<CDA_Model>
{
public:
  CDA_Model(iface::cellml_api::DOMURLLoader* aLoader,
            iface::dom::Document* doc,
            iface::dom::Element* modelElement);
  virtual ~CDA_Model();

  CDA_IMPL_QI3(cellml_api::CellMLElement, cellml_api::NamedCellMLElement,
               cellml_api::Model)

  iface::cellml_api::Model* getAlternateVersion(const wchar_t* cellmlVersion)
    throw(std::exception&);
  iface::cellml_api::GroupSet* groups() throw(std::exception&);
  iface::cellml_api::CellMLImportSet* imports() throw(std::exception&);
  iface::cellml_api::URI* base_uri() throw(std::exception&);
  iface::cellml_api::UnitsSet* localUnits() throw(std::exception&);
  iface::cellml_api::UnitsSet* modelUnits() throw(std::exception&);
  iface::cellml_api::UnitsSet* allUnits() throw(std::exception&);
  iface::cellml_api::CellMLComponentSet* localComponents()
    throw(std::exception&);
  iface::cellml_api::CellMLComponentSet* modelComponents()
    throw(std::exception&);
  iface::cellml_api::CellMLComponentSet* allComponents()
    throw(std::exception&);
  iface::cellml_api::ConnectionSet* connections() throw(std::exception&);
  iface::cellml_api::GroupSet* findGroupsWithRelationshipRefName
    (const wchar_t* name)
    throw(std::exception&);
  void fullyInstantiateImports() throw(std::exception&);
  iface::cellml_api::Model* generateFlattenedModel() throw(std::exception&);
  iface::cellml_api::CellMLComponent* createComponent() throw(std::exception&);
  iface::cellml_api::ImportComponent* createImportComponent()
    throw(std::exception&);
  iface::cellml_api::Units* createUnits() throw(std::exception&);
  iface::cellml_api::ImportUnits* createImportUnits() throw(std::exception&);
  iface::cellml_api::Unit* createUnit() throw(std::exception&);
  iface::cellml_api::CellMLImport* createCellMLImport() throw(std::exception&);
  iface::cellml_api::CellMLVariable* createCellMLVariable()
    throw(std::exception&);
  iface::cellml_api::ComponentRef* createComponentRef() throw(std::exception&);
  iface::cellml_api::RelationshipRef* createRelationshipRef()
    throw(std::exception&);
  iface::cellml_api::Group* createGroup() throw(std::exception&);
  iface::cellml_api::Connection* createConnection() throw(std::exception&);
  iface::cellml_api::MapComponents* createMapComponents() throw(std::exception&);
  iface::cellml_api::MapVariables* createMapVariables() throw(std::exception&);
  iface::cellml_api::Reaction* createReaction() throw(std::exception&);
  iface::cellml_api::ReactantVariableRef* createReactantVariableRef()
    throw(std::exception&);
  iface::cellml_api::RateVariableRef* createRateVariableRef()
    throw(std::exception&);
  iface::cellml_api::ReactantRole* createReactantRole() throw(std::exception&);
  iface::cellml_api::ProductRole* createProductRole() throw(std::exception&);
  iface::cellml_api::RateRole* createRateRole() throw(std::exception&);
  u_int32_t assignUniqueIdentifier();

  ObjRef<iface::cellml_api::DOMURLLoader> mLoader;

protected:
  iface::dom::Document* mDoc;
  u_int32_t mNextUniqueIdentifier;

  void RecursivelyChangeVersionCopy(
                                    const wchar_t* aNewNamespace,
                                    iface::dom::Node* aCopy,
                                    iface::dom::Node* aOriginal)
    throw(std::exception&);
};

class CDA_MathContainer
  : public virtual iface::cellml_api::MathContainer,
    public virtual CDA_CellMLElement
{
public:
  CDA_MathContainer(iface::XPCOM::IObject* parent,
                    iface::dom::Element* element);
  virtual ~CDA_MathContainer() {};

  iface::cellml_api::MathList* math() throw(std::exception&);
  void addMath(const iface::cellml_api::MathMLElement x)
    throw(std::exception&);
  void removeMath(const iface::cellml_api::MathMLElement x)
    throw(std::exception&);
  void replaceMath(const iface::cellml_api::MathMLElement x,
                   const iface::cellml_api::MathMLElement y)
    throw(std::exception&);
  void clearMath() throw(std::exception&);
};

class CDA_CellMLComponentGroupMixin
  : public virtual iface::cellml_api::CellMLComponent,
    public CDA_NamedCellMLElement,
    public CDA_MathContainer
{
public:
  CDA_CellMLComponentGroupMixin(iface::XPCOM::IObject* parent,
                                iface::dom::Element* compElement)
    : CDA_CellMLElement(parent, compElement),
      CDA_NamedCellMLElement(parent, compElement),
      CDA_MathContainer(parent, compElement)
  {}

  virtual ~CDA_CellMLComponentGroupMixin()
  {}

  iface::cellml_api::CellMLComponent* encapsulationParent() throw(std::exception&);
  iface::cellml_api::CellMLComponentSet* encapsulationChildren() throw(std::exception&);
  iface::cellml_api::CellMLComponent* containmentParent() throw(std::exception&);
  iface::cellml_api::CellMLComponentSet* containmentChildren() throw(std::exception&);
  
};

class CDA_CellMLComponent
  : public CDA_CellMLComponentGroupMixin
{
public:
  CDA_CellMLComponent(iface::XPCOM::IObject* parent,
                      iface::dom::Element* compElement)
    :CDA_CellMLElement(parent, compElement),
     CDA_CellMLComponentGroupMixin(parent, compElement)
  {}
  virtual ~CDA_CellMLComponent() {}

  CDA_IMPL_QI4(cellml_api::CellMLElement, cellml_api::NamedCellMLElement,
               cellml_api::MathContainer, cellml_api::CellMLComponent)

  iface::cellml_api::CellMLVariableSet* variables() throw(std::exception&);
  iface::cellml_api::UnitsSet* units() throw(std::exception&);
  iface::cellml_api::ConnectionSet* connections() throw(std::exception&);
  u_int32_t importNumber() throw(std::exception&);
};

class CDA_UnitsBase
  : public virtual iface::cellml_api::Units,
    public CDA_NamedCellMLElement
{
public:
  CDA_UnitsBase(iface::XPCOM::IObject* parent,
                iface::dom::Element* unitsElement)
    : CDA_CellMLElement(parent, unitsElement),
      CDA_NamedCellMLElement(parent, unitsElement) {}
  virtual ~CDA_UnitsBase() {};

  bool isBaseUnits() throw(std::exception&);
  void isBaseUnits(bool attr) throw(std::exception&);
  iface::cellml_api::UnitSet* unitCollection() throw(std::exception&);
};

class CDA_Units
  : public CDA_UnitsBase
{
public:
  CDA_Units(iface::XPCOM::IObject* parent,
            iface::dom::Element* unitsElement)
    : CDA_CellMLElement(parent, unitsElement),
      CDA_UnitsBase(parent, unitsElement) {}
  virtual ~CDA_Units() {};

  CDA_IMPL_QI3(iface::cellml_api::CellMLElement,
               iface::cellml_api::NamedCellMLElement, iface::cellml_api::Units)
};


class CDA_Unit
  : public virtual iface::cellml_api::Unit,
    public virtual CDA_CellMLElement
{
public:
  CDA_Unit(iface::XPCOM::IObject* parent, iface::dom::Element* unitElement)
    : CDA_CellMLElement(parent, unitElement) {}
  virtual ~CDA_Unit() {};

  CDA_IMPL_QI2(iface::cellml_api::Unit, iface::cellml_api::CellMLElement)

  int32_t prefix() throw(std::exception&);
  void prefix(int32_t attr) throw(std::exception&);
  double multiplier() throw(std::exception&);
  void multiplier(double attr) throw(std::exception&);
  double offset() throw(std::exception&);
  void offset(double attr) throw(std::exception&);
  double exponent() throw(std::exception&);
  void exponent(double attr) throw(std::exception&);
  wchar_t* units() throw(std::exception&);
  void units(const wchar_t* attr) throw(std::exception&);
};

class CDA_CellMLImport
  : public virtual iface::cellml_api::CellMLImport,
    public virtual CDA_CellMLElement
{
public:
  CDA_CellMLImport(iface::XPCOM::IObject* parent,
                   iface::dom::Element* importElement)
    : CDA_CellMLElement(parent, importElement) {}
  virtual ~CDA_CellMLImport() { if (importedModel) importedModel->release_ref(); }


  CDA_IMPL_QI2(cellml_api::CellMLImport, cellml_api::CellMLElement)

  iface::cellml_api::URI* xlinkHref() throw(std::exception&);
  iface::cellml_api::ImportComponentSet* components() throw(std::exception&);
  iface::cellml_api::ImportUnitsSet* units() throw(std::exception&);
  iface::cellml_api::ConnectionSet* importedConnections()
    throw(std::exception&);
  void fullyInstantiate() throw(std::exception&);
  bool wasFullyInstantiated() throw(std::exception&);
  u_int32_t uniqueIdentifier() throw(std::exception&);

  iface::cellml_api::Model* importedModel;

  WeakReference<CDA_Model> lastIdentifierModel;
  // This number uniquely identifies the CellML import within the toplevel
  // parent model. It is computed lazily when requested and lastIdentifierModel
  // doesn't match.
  u_int32_t mUniqueIdentifier;
};

class CDA_ImportComponent
  : public virtual iface::cellml_api::ImportComponent,
    public CDA_CellMLComponentGroupMixin
{
public:
  CDA_ImportComponent(iface::XPCOM::IObject* parent,
                      iface::dom::Element* importComponent)
    : CDA_CellMLElement(parent, importComponent),
      CDA_CellMLComponentGroupMixin(parent, importComponent){}
  virtual ~CDA_ImportComponent() {}

  CDA_IMPL_QI5(cellml_api::CellMLElement, cellml_api::NamedCellMLElement,
               cellml_api::MathContainer, cellml_api::CellMLComponent,
               cellml_api::ImportComponent)

  iface::cellml_api::CellMLVariableSet* variables() throw(std::exception&);
  iface::cellml_api::UnitsSet* units() throw(std::exception&);
  iface::cellml_api::ConnectionSet* connections() throw(std::exception&);
  u_int32_t importNumber() throw(std::exception&);
  wchar_t* componentRef() throw(std::exception&);
  void componentRef(const wchar_t* attr) throw(std::exception&);

private:
  // This is an internal API only, and *does not* increment the refcount on
  // the return value.
  CDA_CellMLComponent* fetchDefinition() throw(std::exception&);
  
};

class CDA_ImportUnits
  : public iface::cellml_api::ImportUnits,
    public CDA_UnitsBase
{
public:
  CDA_ImportUnits(iface::XPCOM::IObject* parent,
                  iface::dom::Element* importUnits)
    : CDA_CellMLElement(parent, importUnits),
      CDA_UnitsBase(parent, importUnits) {}
  virtual ~CDA_ImportUnits() {}

  CDA_IMPL_QI3(cellml_api::ImportUnits, cellml_api::Units, cellml_api::CellMLElement)

  wchar_t* unitsRef() throw(std::exception&);
  void unitsRef(const wchar_t* attr) throw(std::exception&);
};

typedef enum _enum_VariableInterface
  {
    INTERFACE_IN = 0,
    INTERFACE_OUT = 1,
    INTERFACE_NONE = 2
  } VariableInterface;

class CDA_CellMLVariable
  : public iface::cellml_api::CellMLVariable,
    public CDA_NamedCellMLElement
{
public:
  CDA_CellMLVariable(iface::XPCOM::IObject* parent,
                     iface::dom::Element* cellmlVar)
    : CDA_CellMLElement(parent, cellmlVar),
      CDA_NamedCellMLElement(parent, cellmlVar) {}
  virtual ~CDA_CellMLVariable() {}
  CDA_IMPL_QI2(cellml_api::CellMLVariable, cellml_api::CellMLElement)

  wchar_t* initialValue() throw(std::exception&);
  void initialValue(const wchar_t* attr) throw(std::exception&);
  iface::cellml_api::VariableInterface privateInterface() throw(std::exception&);
  void privateInterface(iface::cellml_api::VariableInterface attr) throw(std::exception&);
  iface::cellml_api::VariableInterface publicInterface() throw(std::exception&);
  void publicInterface(iface::cellml_api::VariableInterface attr) throw(std::exception&);
  iface::cellml_api::CellMLVariableSet* connectedVariables() throw(std::exception&);
  iface::cellml_api::CellMLVariable* sourceVariable() throw(std::exception&);
  wchar_t* componentName() throw(std::exception&);
};

class CDA_ComponentRef
  : public iface::cellml_api::ComponentRef,
    public CDA_CellMLElement
{
public:
  CDA_ComponentRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* componentRef)
    : CDA_CellMLElement(parent, componentRef) {}
  virtual ~CDA_ComponentRef() {}

  CDA_IMPL_QI2(cellml_api::ComponentRef, cellml_api::CellMLElement)

  wchar_t* componentName() throw(std::exception&);
  void componentName(const wchar_t* attr) throw(std::exception&);
  iface::cellml_api::ComponentRefSet* componentRefs() throw(std::exception&);
  iface::cellml_api::ComponentRef* parentComponentRef() throw(std::exception&);
  iface::cellml_api::Group* parentGroup() throw(std::exception&);
};

class CDA_RelationshipRef
  : public iface::cellml_api::RelationshipRef,
    public CDA_CellMLElement
{
public:
  CDA_RelationshipRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* relationshipRef)
    : CDA_CellMLElement(parent, relationshipRef) {}
  virtual ~CDA_RelationshipRef() {}

  CDA_IMPL_QI2(cellml_api::ComponentRef, cellml_api::CellMLElement)

  wchar_t* name() throw(std::exception&);
  void name(const wchar_t* attr) throw(std::exception&);
  wchar_t* relationship() throw(std::exception&);
  wchar_t* relationshipNamespace() throw(std::exception&);
  void setRelationshipName(const wchar_t* namespaceURI, const wchar_t* name)
    throw(std::exception&);
};

class CDA_Group
  : public iface::cellml_api::Group,
    public CDA_CellMLElement
{
public:
  CDA_Group(iface::XPCOM::IObject* parent,
            iface::dom::Element* group)
    : CDA_CellMLElement(parent, group) {}
  virtual ~CDA_Group() {}
  CDA_IMPL_QI2(cellml_api::Group, cellml_api::CellMLElement)

  iface::cellml_api::RelationshipRefSet* relationshipRefs() throw(std::exception&);
  void relationshipRefs(iface::cellml_api::RelationshipRefSet* attr) throw(std::exception&);
  iface::cellml_api::ComponentRefSet* componentRefs() throw(std::exception&);
  void componentRefs(iface::cellml_api::ComponentRefSet* attr) throw(std::exception&);
  bool isEncapsulation() throw(std::exception&);
  bool isContainment() throw(std::exception&);
};

class CDA_Connection
  : public iface::cellml_api::Connection,
    public CDA_CellMLElement
{
public:
  CDA_Connection(iface::XPCOM::IObject* parent,
                 iface::dom::Element* connection)
    : CDA_CellMLElement(parent, connection) {};
  virtual ~CDA_Connection() {}
  CDA_IMPL_QI2(cellml_api::Connection, cellml_api::CellMLElement)

  iface::cellml_api::MapComponents* componentMapping() throw(std::exception&);
  iface::cellml_api::MapVariablesSet* variableMappings() throw(std::exception&);
};

class CDA_MapComponents
  : public iface::cellml_api::MapComponents,
    public CDA_CellMLElement
{
public:
  CDA_MapComponents(iface::XPCOM::IObject* parent,
                 iface::dom::Element* mapComponents)
    : CDA_CellMLElement(parent, mapComponents) {};
  virtual ~CDA_MapComponents() {}
  CDA_IMPL_QI2(cellml_api::MapComponents, cellml_api::CellMLElement)

  wchar_t* firstComponentName() throw(std::exception&);
  void firstComponentName(const wchar_t* attr) throw(std::exception&);
  wchar_t* secondComponentName() throw(std::exception&);
  void secondComponentName(const wchar_t* attr) throw(std::exception&);
  iface::cellml_api::CellMLComponent* firstComponent() throw(std::exception&);
  void firstComponent(iface::cellml_api::CellMLComponent* attr) throw(std::exception&);
  iface::cellml_api::CellMLComponent* secondComponent() throw(std::exception&);
  void secondComponent(iface::cellml_api::CellMLComponent* attr) throw(std::exception&);
};

class CDA_MapVariables
  : public iface::cellml_api::MapVariables,
    public CDA_CellMLElement
{
public:
  CDA_MapVariables(iface::XPCOM::IObject* parent,
                   iface::dom::Element* mapVariables)
    : CDA_CellMLElement(parent, mapVariables) {}
  virtual ~CDA_MapVariables() {}
  CDA_IMPL_QI2(cellml_api::MapVariables, cellml_api::CellMLElement)

  wchar_t* firstVariableName() throw(std::exception&);
  void firstVariableName(const wchar_t* attr) throw(std::exception&);
  wchar_t* secondVariableName() throw(std::exception&);
  void secondVariableName(const wchar_t* attr) throw(std::exception&);
  iface::cellml_api::CellMLVariable* firstVariable() throw(std::exception&);
  void firstVariable(iface::cellml_api::CellMLVariable* attr) throw(std::exception&);
  iface::cellml_api::CellMLVariable* secondVariable() throw(std::exception&);
  void secondVariable(iface::cellml_api::CellMLVariable* attr) throw(std::exception&);
};

class CDA_Reaction
  : public iface::cellml_api::Reaction,
    public CDA_CellMLElement
{
public:
  CDA_Reaction(iface::XPCOM::IObject* parent,
                   iface::dom::Element* reaction)
    : CDA_CellMLElement(parent, reaction) {}
  virtual ~CDA_Reaction() {}
  CDA_IMPL_QI2(cellml_api::Reaction, cellml_api::CellMLElement)

  iface::cellml_api::ReactantVariableRefSet* reactants() throw(std::exception&);
  iface::cellml_api::ProductVariableRefSet* products() throw(std::exception&);
  iface::cellml_api::RateVariableRef* rate() throw(std::exception&);
};

class CDA_VariableRef
  : public virtual iface::cellml_api::VariableRef,
    public CDA_CellMLElement
{
public:
  CDA_VariableRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* variableRef)
    : CDA_CellMLElement(parent, variableRef) {}
  virtual ~CDA_VariableRef() {}

  iface::cellml_api::CellMLVariable* variable() throw(std::exception&);
};

class CDA_ReactantVariableRef
  : public virtual iface::cellml_api::ReactantVariableRef,
    public CDA_VariableRef
{
public:
  CDA_ReactantVariableRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* reactantVariableRef)
    : CDA_VariableRef(parent, reactantVariableRef) {}
  virtual ~CDA_ReactantVariableRef() {}
  CDA_IMPL_QI3(cellml_api::ReactantVariableRef,
               cellml_api::VariableRef, cellml_api::CellMLElement)

  iface::cellml_api::ReactantRole* role() throw(std::exception&);
  void role(iface::cellml_api::ReactantRole* attr) throw(std::exception&);
  wchar_t* refType() throw(std::exception&);
};

class CDA_ProductVariableRef
  : public virtual iface::cellml_api::ProductVariableRef,
    public CDA_VariableRef
{
public:
  CDA_ProductVariableRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* productVariableRef);
  virtual ~CDA_ProductVariableRef() {}
  CDA_IMPL_QI3(cellml_api::ProductVariableRef,
               cellml_api::VariableRef, cellml_api::CellMLElement)

  iface::cellml_api::ProductRole* role() throw(std::exception&);
  void role(iface::cellml_api::ProductRole* attr) throw(std::exception&);
  wchar_t* refType() throw(std::exception&);
};

class CDA_RateVariableRef
  : public virtual iface::cellml_api::RateVariableRef,
    public CDA_VariableRef
{
public:
  CDA_RateVariableRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* rateVariableRef)
    : CDA_VariableRef(parent, rateVariableRef)
  {}
  virtual ~CDA_RateVariableRef() {}
  CDA_IMPL_QI3(cellml_api::RateVariableRef,
               cellml_api::VariableRef, cellml_api::CellMLElement)

  iface::cellml_api::RateRole* role() throw(std::exception&);
  void role(iface::cellml_api::RateRole* attr) throw(std::exception&);
  wchar_t* refType() throw(std::exception&);
};

class CDA_Role
  : public virtual iface::cellml_api::Role,
    public virtual CDA_CellMLElement
{
public:
  CDA_Role(iface::XPCOM::IObject* parent,
           iface::dom::Element* role)
    : CDA_CellMLElement(parent, role) {}
  virtual ~CDA_Role() {}
};

class CDA_ReactantRole
  : public iface::cellml_api::ReactantRole,
    public CDA_Role
{
public:
  CDA_ReactantRole(iface::XPCOM::IObject* parent,
                   iface::dom::Element* reactantRole)
    : CDA_CellMLElement(parent, reactantRole), CDA_Role(parent, reactantRole)
  {}
  virtual ~CDA_ReactantRole() {}
  CDA_IMPL_QI3(cellml_api::ReactantRole, cellml_api::Role, cellml_api::CellMLElement)


  wchar_t* roleType() throw(std::exception&);
};

class CDA_ProductRole
  : public iface::cellml_api::ProductRole,
    public CDA_Role
{
public:
  CDA_ProductRole(iface::XPCOM::IObject* parent,
                   iface::dom::Element* productRole)
    : CDA_CellMLElement(parent, productRole),
      CDA_Role(parent, productRole) {}
  virtual ~CDA_ProductRole() {}
  CDA_IMPL_QI3(cellml_api::ProductRole, cellml_api::Role, cellml_api::CellMLElement)


  wchar_t* roleType() throw(std::exception&);
};

class CDA_RateRole
  : public iface::cellml_api::RateRole,
    public CDA_Role,
    public CDA_MathContainer
{
public:
  CDA_RateRole(iface::XPCOM::IObject* parent,
               iface::dom::Element* rateRole)
    : CDA_CellMLElement(parent, rateRole),
      CDA_Role(parent, rateRole),
      CDA_MathContainer(parent, rateRole)
  {}
  virtual ~CDA_RateRole() {}
  CDA_IMPL_QI3(cellml_api::RateRole, cellml_api::Role, cellml_api::CellMLElement)


  wchar_t* roleType() throw(std::exception&);
};

class CDA_DOMElementIteratorBase
  : public iface::events::EventListener
{
public:
  CDA_DOMElementIteratorBase(iface::dom::Element* parentElement);
  virtual ~CDA_DOMElementIteratorBase();

  iface::dom::Element* fetchNextElement();
  void handleEvent(iface::events::Event* evt)
    throw(std::exception&);

private:
  iface::dom::Element *mPrevElement, *mNextElement, *mParentElement;
  iface::dom::NodeList *mNodeList;

protected:
  void registerListener();
  void deregisterListener();
};

class CDA_CellMLElementIterator
  : public CDA_DOMElementIteratorBase,
    public iface::cellml_api::CellMLElementIterator
{
public:
  CDA_CellMLElementIterator(iface::dom::Element* parentElement,
                            CDA_CellMLElementSet* ownerSet);
  virtual ~CDA_CellMLElementIterator();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);

private:
  CDA_CellMLElementSet* parentSet;
};

class CDA_CellMLElementIteratorOuter
  : public virtual iface::cellml_api::CellMLElementIterator
{
public:
  CDA_CellMLElementIteratorOuter(CDA_CellMLElementIterator* aInner)
    : mInner(aInner)
  {
    mInner->add_ref();
  }

  virtual ~CDA_CellMLElementIteratorOuter()
  {
    mInner->release_ref();
  }

  iface::cellml_api::CellMLElement* next()
    throw(std::exception&)
  {
    return mInner->next();
  }
protected:
  CDA_CellMLElementIterator* mInner;
};

class CDA_MathMLElementIterator
  : public iface::cellml_api::MathMLElementIterator,
    public CDA_DOMElementIteratorBase
{
public:
  CDA_MathMLElementIterator(iface::dom::Element* parentEl)
    : CDA_DOMElementIteratorBase(parentEl), _cda_refcount(1)
  {
    registerListener();
  }
  virtual ~CDA_MathMLElementIterator()
  {
    deregisterListener();
  };

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(iface::cellml_api::MathMLElementIterator)

  iface::cellml_api::MathMLElement next() throw(std::exception&);
};

class CDA_ExtensionElementList
  : public iface::cellml_api::ExtensionElementList
{
public:
  CDA_ExtensionElementList(iface::dom::Element* el);
  virtual ~CDA_ExtensionElementList();
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::ExtensionElementList)

  u_int32_t length() throw(std::exception&);
  bool contains(const iface::cellml_api::ExtensionElement x) throw(std::exception&);
  int32_t getIndexOf(const iface::cellml_api::ExtensionElement x) throw(std::exception&);
  iface::cellml_api::ExtensionElement getAt(u_int32_t index) throw(std::exception&);

private:
  iface::dom::NodeList* nl;
};

class CDA_MathList
  : public iface::cellml_api::MathList
{
public:
  CDA_MathList(iface::dom::Element* aParentEl);
  virtual ~CDA_MathList();
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::MathList)

  u_int32_t length() throw(std::exception&);
  bool contains(const iface::cellml_api::MathMLElement x) throw(std::exception&);
  iface::cellml_api::MathMLElementIterator* iterate() throw(std::exception&);

private:
  iface::dom::Element* mParentEl;
};

class CDA_CellMLElementSetUseIteratorMixin
  : public virtual iface::cellml_api::CellMLElementSet
{
public:
  u_int32_t length() throw(std::exception&);
  bool contains(iface::cellml_api::CellMLElement* x) throw(std::exception&);
};

class CDA_CellMLElementSet
  : public virtual iface::cellml_api::CellMLElementSet,
    public CDA_CellMLElementSetUseIteratorMixin
{
public:
  CDA_CellMLElementSet(CDA_CellMLElement* parent, iface::dom::Element* parentEl);
  virtual ~CDA_CellMLElementSet();

  CDA_IMPL_QI1(dom::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);

  void add_ref()
    throw(std::exception&)
  {
    _cda_refcount++;
    
    if (mParent != NULL)
      mParent->add_ref();
  }

  void release_ref()
    throw(std::exception&)
  {
    if (_cda_refcount == 0)
    {
      printf("Warning: release_ref called too many times on %s.\n",
             typeid(this).name());
    }

    _cda_refcount--;
    // mParent must always be non-NULL for a set.
    //if (mParent == NULL)
    //{
    //  if (_cda_refcount == 0)
    //    delete this;
    //}
    //else /* if the owner model is non-null, we will be destroyed when there are
    //      * no remaining references to the model.
    //      */
      mParent->release_ref();
  }
  

  // This adds a child into the element set.
  void addChildToWrapper(iface::cellml_api::CellMLElement*);
  void removeChildFromWrapper(iface::cellml_api::CellMLElement*);

  iface::XPCOM::IObject* mParent;
private:
  friend class CDA_CellMLElementIterator;
  iface::dom::Element* mElement;
  std::map<iface::dom::Element*,iface::cellml_api::CellMLElement*> childMap;

  // This is used only when ownerModel is null. When ownerModel is assigned,
  // add_ref() must be called on ownerModel the correct number of times.
  u_int32_t _cda_refcount;
};

class CDA_CellMLElementSetOuter
  : public virtual iface::cellml_api::CellMLElementSet,
    public CDA_CellMLElementSetUseIteratorMixin
{
public:
  CDA_CellMLElementSetOuter(CDA_CellMLElementSet* aInner)
    : mInner(aInner)
  {
    if (mInner != NULL)
      mInner->add_ref();
  }

  virtual ~CDA_CellMLElementSetOuter()
  {
    if (mInner != NULL)
      mInner->release_ref();
  }

protected:
  CDA_CellMLElementSet* mInner;
};

class CDA_NamedCellMLElementSetBase
  : public virtual iface::cellml_api::NamedCellMLElementSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_NamedCellMLElementSetBase(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner)
  {
  }

  virtual ~CDA_NamedCellMLElementSetBase() {};

  iface::cellml_api::NamedCellMLElement*
  get(const wchar_t* name)
    throw(std::exception&);
};

class CDA_CellMLComponentIteratorBase
  : public virtual iface::cellml_api::CellMLComponentIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_CellMLComponentIteratorBase(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner)
  {
  }
  virtual ~CDA_CellMLComponentIteratorBase() {}

  iface::cellml_api::CellMLElement* next()
    throw(std::exception&);
  iface::cellml_api::CellMLComponent* nextComponent()
    throw(std::exception&);
};

class CDA_CellMLComponentIterator
  : public CDA_CellMLComponentIteratorBase
{
public:
  CDA_CellMLComponentIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLComponentIteratorBase(aInner) {}
  virtual ~CDA_CellMLComponentIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator)
};

class CDA_CellMLComponentSetBase
  : public virtual iface::cellml_api::CellMLComponentSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_CellMLComponentSetBase(CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aInner) {};
  virtual ~CDA_CellMLComponentSetBase() {};

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);

  iface::cellml_api::CellMLComponentIterator*
  iterateComponents()
    throw(std::exception&);
  iface::cellml_api::CellMLComponent*
  getComponent(const wchar_t* name)
    throw(std::exception&);
};

class CDA_CellMLComponentSet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_CellMLComponentSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLComponentSetBase(aInner), _cda_refcount(1) {};
  virtual ~CDA_CellMLComponentSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)
};

class CDA_CellMLComponentFromComponentRefSet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_CellMLComponentFromComponentRefSet
  (
   iface::cellml_api::Model* aModel,
   iface::cellml_api::ComponentRefIterator* aCompRefIterator
  )
    : CDA_CellMLComponentSetBase(NULL),
      _cda_refcount(1), mModel(aModel), mCompRefIterator(aCompRefIterator)
  {
    mModel->add_ref();
    aCompRefIterator->add_ref();
  }

  virtual ~CDA_CellMLComponentFromComponentRefSet()
  {
  }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
private:
  iface::cellml_api::Model *mModel;
  iface::cellml_api::ComponentRefIterator *mCompRefIterator;
};

class CDA_ImportComponentIterator
  : public iface::cellml_api::ImportComponentIterator,
    public CDA_CellMLComponentIteratorBase
{
public:
  CDA_ImportComponentIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLComponentIteratorBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_ImportComponentIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::ImportComponentIterator,
               cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::ImportComponent* nextImportComponent()
    throw(std::exception&);
};

class CDA_ImportComponentSet
  : public virtual iface::cellml_api::ImportComponentSet,
    public CDA_CellMLComponentSetBase
{
public:
  CDA_ImportComponentSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLComponentSetBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_ImportComponentSet() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI4(cellml_api::ImportComponentSet, cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet, cellml_api::CellMLElementSet)

  iface::cellml_api::ImportComponentIterator* iterateImportComponents()
    throw(std::exception&);

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);

  iface::cellml_api::ImportComponent*
  getImportComponent(const wchar_t* name)
    throw(std::exception&);
};

class CDA_AllComponentSet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_AllComponentSet(iface::cellml_api::CellMLElementIterator* aLocalIterator,
                      iface::cellml_api::CellMLElementIterator* aImportIterator,
                      bool aRecurseIntoImports)
    : CDA_CellMLComponentSetBase(NULL), _cda_refcount(1),
      mLocalIterator(aLocalIterator), mImportIterator(aImportIterator),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllComponentSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)
  
  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
private:
  ObjRef<iface::cellml_api::CellMLElementIterator> mLocalIterator;
  ObjRef<iface::cellml_api::CellMLElementIterator> mImportIterator;
  bool mRecurseIntoImports;
};

class CDA_AllComponentIterator
  : public CDA_CellMLComponentIteratorBase
{
public:
  CDA_AllComponentIterator(iface::cellml_api::CellMLElementIterator*
                             aLocalIterator,
                             iface::cellml_api::CellMLElementIterator*
                             aImportIterator,
                           bool aRecurseIntoImports)
    : CDA_CellMLComponentIteratorBase(NULL), _cda_refcount(1),
      mLocalIterator(aLocalIterator), mImportIterator(aImportIterator),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllComponentIterator() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitsIterator, cellml_api::CellMLElementIterator);

  iface::cellml_api::CellMLElement* next() throw(std::exception&);

private:
  ObjRef<iface::cellml_api::CellMLElementIterator> mLocalIterator;
  ObjRef<iface::cellml_api::CellMLElementIterator> mImportIterator;
  bool mRecurseIntoImports;
};

class CDA_CellMLVariableIterator
  : public virtual iface::cellml_api::CellMLVariableIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_CellMLVariableIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_CellMLVariableIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLVariableIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::CellMLVariable* nextVariable()
    throw(std::exception&);
};

class CDA_CellMLVariableSet
  : public virtual iface::cellml_api::CellMLVariableSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_CellMLVariableSet(CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_CellMLVariableSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::CellMLVariableSet, cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::CellMLVariableIterator* iterateVariables()
    throw(std::exception&);

  iface::cellml_api::CellMLVariable*
  getVariable(const wchar_t* name)
    throw(std::exception&);
};

class CDA_ConnectedCellMLVariableSet
  : public CDA_CellMLVariableSet
{
public:
  CDA_ConnectedCellMLVariableSet(CDA_CellMLVariable* aConnectedToWhat)
    : CDA_CellMLVariableSet(NULL), mConnectedToWhat(aConnectedToWhat)
  {
    mConnectedToWhat->add_ref();
  }
  virtual ~CDA_ConnectedCellMLVariableSet()
  {
    mConnectedToWhat->release_ref();
  }

  iface::cellml_api::CellMLElementIterator* iterate()
    throw(std::exception&);

private:
  CDA_CellMLVariable* mConnectedToWhat;
};

class CDA_UnitsIteratorBase
  : public virtual iface::cellml_api::UnitsIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_UnitsIteratorBase(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_UnitsIteratorBase() {}

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::Units* nextUnits() throw(std::exception&);
};

class CDA_UnitsIterator
  : public CDA_UnitsIteratorBase
{
public:
  CDA_UnitsIterator(CDA_CellMLElementIterator* aInner)
    : CDA_UnitsIteratorBase(aInner), _cda_refcount(1) {};
  virtual ~CDA_UnitsIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitsIterator, cellml_api::CellMLElementIterator)
};

class CDA_UnitsSetBase
  : public virtual iface::cellml_api::UnitsSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_UnitsSetBase(CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aInner) {}
  virtual ~CDA_UnitsSetBase() {}

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::UnitsIterator* iterateUnits() throw(std::exception&);

  iface::cellml_api::Units*
  getUnits(const wchar_t* name)
    throw(std::exception&);
};

class CDA_UnitsSet
  : public CDA_UnitsSetBase
{
public:
  CDA_UnitsSet(CDA_CellMLElementSet* aInner)
    : CDA_UnitsSetBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_UnitsSet() {}
  
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::UnitsSet, cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)
};

class CDA_AllUnitsSet
  : public CDA_UnitsSetBase
{
public:
  CDA_AllUnitsSet(iface::cellml_api::CellMLElementIterator* aLocalIterator,
                  iface::cellml_api::CellMLElementIterator* aImportIterator,
                  bool aRecurseIntoImports)
    : CDA_UnitsSetBase(NULL), _cda_refcount(1),
      mLocalIterator(aLocalIterator), mImportIterator(aImportIterator),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllUnitsSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::UnitsSet, cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet);
  
  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
private:
  ObjRef<iface::cellml_api::CellMLElementIterator> mLocalIterator;
  ObjRef<iface::cellml_api::CellMLElementIterator> mImportIterator;
  bool mRecurseIntoImports;
};

class CDA_AllUnitsIterator
  : public CDA_UnitsIteratorBase
{
public:
  CDA_AllUnitsIterator(iface::cellml_api::CellMLElementIterator*
                         aLocalIterator,
                         iface::cellml_api::CellMLElementIterator*
                         aImportIterator,
                       bool aRecurseIntoImports)
    : CDA_UnitsIteratorBase(NULL), _cda_refcount(1),
      mLocalIterator(aLocalIterator), mImportIterator(aImportIterator),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllUnitsIterator() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitsIterator, cellml_api::CellMLElementIterator);

  iface::cellml_api::CellMLElement* next() throw(std::exception&);

private:
  ObjRef<iface::cellml_api::CellMLElementIterator> mLocalIterator;
  ObjRef<iface::cellml_api::CellMLElementIterator> mImportIterator;
  bool mRecurseIntoImports;
};

class CDA_ImportUnitsIterator
  : public iface::cellml_api::ImportUnitsIterator,
    public CDA_UnitsIteratorBase
{
public:
  CDA_ImportUnitsIterator(CDA_CellMLElementIterator* aInner)
    : CDA_UnitsIteratorBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_ImportUnitsIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(
               cellml_api::ImportUnitsIterator,
               cellml_api::UnitsIterator,
               cellml_api::CellMLElementIterator
              )

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::ImportUnits* nextImportUnits()
    throw(std::exception&);
};

class CDA_ImportUnitsSet
  : public iface::cellml_api::ImportUnitsSet,
    public CDA_UnitsSetBase
{
public:
  CDA_ImportUnitsSet(CDA_CellMLElementSet* aInner)
    : CDA_UnitsSetBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_ImportUnitsSet() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI4(
               cellml_api::ImportUnitsSet,
               cellml_api::UnitsSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet
              )

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::ImportUnitsIterator* iterateImportUnits()
    throw(std::exception&);

  iface::cellml_api::ImportUnits*
  getImportUnits(const wchar_t* name)
    throw(std::exception&);
};

class CDA_CellMLImportIterator
  : public iface::cellml_api::CellMLImportIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_CellMLImportIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_CellMLImportIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLImportIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::CellMLImport* nextImport()
    throw(std::exception&);
};

class CDA_CellMLImportSet
  : public iface::cellml_api::CellMLImportSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_CellMLImportSet(CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_CellMLImportSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::CellMLImportSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::CellMLImportIterator* iterateImports()
    throw(std::exception&);

  iface::cellml_api::CellMLImport*
  getImport(const wchar_t* name)
    throw(std::exception&);
};

class CDA_UnitIterator
  : public iface::cellml_api::UnitIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_UnitIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_UnitIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::Unit* nextUnit()
    throw(std::exception&);
};

class CDA_UnitSet
  : public iface::cellml_api::UnitSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_UnitSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_UnitSet() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLUnitSet,
               cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::UnitIterator* iterateUnits()
    throw(std::exception&);
};

class CDA_ConnectionIterator
  : public iface::cellml_api::ConnectionIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_ConnectionIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_ConnectionIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::Connection* nextConnection()
    throw(std::exception&);
};

class CDA_ConnectionSet
  : public iface::cellml_api::ConnectionSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_ConnectionSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_ConnectionSet() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::ConnectionSet, cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::ConnectionIterator* iterateConnections() throw(std::exception&);
};

class CDA_GroupIterator
  : public iface::cellml_api::GroupIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_GroupIterator(CDA_CellMLElementIterator* aInner, bool aFilterByRRName,
                    const wchar_t* aFilterRRName)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1),
      filterByRRName(aFilterByRRName), mFilterRRName(aFilterRRName) {}
  virtual ~CDA_GroupIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::GroupIterator, cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::Group* nextGroup() throw(std::exception&);

private:
  bool filterByRRName;
  std::wstring mFilterRRName;
};

class CDA_GroupSet
  : public iface::cellml_api::GroupSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_GroupSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner), _cda_refcount(1),
      filterByRRName(false) {}
  CDA_GroupSet(CDA_CellMLElementSet* aInner, const wchar_t* aFilterRRName)
    : CDA_CellMLElementSetOuter(aInner), filterByRRName(true),
      mFilterRRName(aFilterRRName) {}

  virtual ~CDA_GroupSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::GroupSet, cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::GroupIterator* iterateGroups() throw(std::exception&);
  iface::cellml_api::GroupSet* getSubsetInvolvingRelationship
  (const wchar_t* relName) throw(std::exception&);
  iface::cellml_api::GroupSet* subsetInvolvingEncapsulation()
    throw(std::exception&);
  iface::cellml_api::GroupSet* subsetInvolvingContainment()
    throw(std::exception&);
private:
  bool filterByRRName;
  std::wstring mFilterRRName;
};

class CDA_RelationshipRefIterator
  : public iface::cellml_api::RelationshipRefIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_RelationshipRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_RelationshipRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RelationshipRefIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::RelationshipRef* nextRelationshipRef() throw(std::exception&);
};

class CDA_RelationshipRefSet
  : public iface::cellml_api::RelationshipRefSet,
  public CDA_CellMLElementSetOuter
{
public:
  CDA_RelationshipRefSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_RelationshipRefSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RelationshipRefSet, cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::RelationshipRefIterator* iterateRelationshipRefs() throw(std::exception&);
};

class CDA_ComponentRefIterator
  : public iface::cellml_api::ComponentRefIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_ComponentRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_ComponentRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::ComponentRefIterator, cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::ComponentRef* nextComponentRef() throw(std::exception&);
};

class CDA_ComponentRefSet
  : public iface::cellml_api::ComponentRefSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_ComponentRefSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_ComponentRefSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::ComponentRefSet, cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::ComponentRefIterator* iterateComponentRefs() throw(std::exception&);
};

class CDA_CellMLComponentFromComponentRefIterator
  : public CDA_CellMLComponentIteratorBase
{
public:
  CDA_CellMLComponentFromComponentRefIterator
  (
   iface::cellml_api::Model* aModel,
   iface::cellml_api::ComponentRefIterator* aCompRefIterator
  )
    : CDA_CellMLComponentIteratorBase(NULL), _cda_refcount(1), mModel(aModel),
      mCompRefIterator(aCompRefIterator)
  {
    mModel->add_ref();
    aCompRefIterator->add_ref();
  }

  virtual ~CDA_CellMLComponentFromComponentRefIterator()
  {
  }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);

private:
  iface::cellml_api::Model *mModel;
  iface::cellml_api::ComponentRefIterator *mCompRefIterator;
};

class CDA_MapVariablesIterator
  : public iface::cellml_api::MapVariablesIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_MapVariablesIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_MapVariablesIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::MapVariablesIterator, cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::MapVariables* nextMapVariable() throw(std::exception&);
};

class CDA_MapVariablesSet
  : public iface::cellml_api::MapVariablesSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_MapVariablesSet(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner), _cda_refcount(1) {}
  virtual ~CDA_MapVariablesSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::MapVariablesSet, cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::MapVariablesIterator* iterateMapVariables() throw(std::exception&);
};

class CDA_VariableRefIteratorBase
  : public virtual iface::cellml_api::VariableRefIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_VariableRefIteratorBase(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_VariableRefIteratorBase() {}

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::VariableRef* nextVariableRef() throw(std::exception&);
};

class CDA_VariableRefIterator
  : public CDA_VariableRefIteratorBase
{
public:
  CDA_VariableRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_VariableRefIteratorBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_VariableRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::VariableRefIterator,
               cellml_api::CellMLElementIterator)
};

class CDA_VariableRefSetBase
  : public virtual iface::cellml_api::VariableRefSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_VariableRefSetBase(CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aInner) {}
  virtual ~CDA_VariableRefSetBase() {}

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::VariableRefIterator* iterateVariableRefs()
    throw(std::exception&);
};

class CDA_VariableRefSet
  : public CDA_VariableRefSetBase
{
public:
  CDA_VariableRefSet(CDA_CellMLElementSet* aInner)
    : CDA_VariableRefSetBase(aInner), _cda_refcount(1) {}
  virtual ~CDA_VariableRefSet() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::VariableRefSet, cellml_api::CellMLElementSet)
};

class CDA_ReactantVariableRefIterator
  : public iface::cellml_api::ReactantVariableRefIterator,
    public CDA_VariableRefIterator
{
public:
  CDA_ReactantVariableRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_VariableRefIterator(aInner), _cda_refcount(1) {}
  virtual ~CDA_ReactantVariableRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::ReactantVariableRefIterator,
               cellml_api::VariableRefIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::ReactantVariableRef* nextReactantVariableRef()
    throw(std::exception&);
};

class CDA_ReactantVariableRefSet
  : public iface::cellml_api::ReactantVariableRefSet,
    public CDA_VariableRefSet
{
public:
  CDA_ReactantVariableRefSet(CDA_CellMLElementSet* aInner)
    : CDA_VariableRefSet(aInner), _cda_refcount(1) {}
  virtual ~CDA_ReactantVariableRefSet() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::ReactantVariableRefSet,
               cellml_api::VariableRefSet,
               cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::ReactantVariableRefIterator* iterateReactantVariableRefs() throw(std::exception&);
};

class CDA_ProductVariableRefIterator
  : public iface::cellml_api::ProductVariableRefIterator,
    public CDA_VariableRefIterator
{
public:
  CDA_ProductVariableRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_VariableRefIterator(aInner), _cda_refcount(1) {}
  virtual ~CDA_ProductVariableRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::ProductVariableRefIterator,
               cellml_api::VariableRefIterator,
               cellml_api::CellMLElementIterator)

  iface::cellml_api::CellMLElement* next() throw(std::exception&);
  iface::cellml_api::ProductVariableRef* nextProductVariableRef()
    throw(std::exception&);
};

class CDA_ProductVariableRefSet
  : public iface::cellml_api::ProductVariableRefSet,
    public CDA_VariableRefSet
{
public:
  CDA_ProductVariableRefSet(CDA_CellMLElementSet* aInner)
    : CDA_VariableRefSet(aInner), _cda_refcount(1) {}
  virtual ~CDA_ProductVariableRefSet() {}
  
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::ProductVariableRefSet, cellml_api::VariableRefSet,
               cellml_api::CellMLElementSet)

  iface::cellml_api::CellMLElementIterator* iterate() throw(std::exception&);
  iface::cellml_api::ProductVariableRefIterator*
  iterateProductVariableRefs()
    throw(std::exception&);
};

class CDA_ConnectedCellMLVariableIterator
  : public CDA_CellMLVariableIterator
{
public:
  CDA_ConnectedCellMLVariableIterator(CDA_CellMLVariable* aConnectedToWhat);
  virtual ~CDA_ConnectedCellMLVariableIterator();

  iface::cellml_api::CellMLElement* next() throw(std::exception&);

private:
  struct VariableStackFrame
  {
    ObjRef<CDA_CellMLVariable> whichVariable;
    ObjRef<CDA_CellMLComponent> whichComponent;
    ObjRef<iface::cellml_api::ConnectionIterator> connectionIterator;
    ObjRef<iface::cellml_api::MapVariablesIterator> mapVariableIterator;
  };

  std::list<VariableStackFrame*> variableStack;
  bool mConsider1, mConsider2;
};
