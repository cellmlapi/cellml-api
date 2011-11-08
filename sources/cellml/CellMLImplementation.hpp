#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include <string>
#include <map>
#include <wchar.h>
#include "Utilities.hxx"
#include "Ifacexpcom.hxx"
#include "IfaceDOM_APISPEC.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceDOM_events.hxx"
#include <typeinfo>
#include <assert.h>
#ifdef ENABLE_RDF
#include "IfaceRDF_APISPEC.hxx"
#endif

typedef uint32_t cda_serial_t;

class CDA_ConnectionSet;
class CDA_GroupSet;
class CDA_CellMLImportSet;
class CDA_CellMLComponentSet;
class CDA_AllComponentSet;
class CDA_UnitsSet;
class CDA_AllUnitsSet;
class CDA_ReactionSet;
class CDA_CellMLVariableSet;
class CDA_ComponentConnectionSet;
class CDA_UnitsSet;
class CDA_UnitSet;
class CDA_ImportConnectionSet;
class CDA_ImportComponentSet;
class CDA_ImportUnitsSet;
class CDA_ConnectedCellMLVariableSet;
class CDA_ComponentRefSet;
class CDA_RelationshipRefSet;
class CDA_ComponentRefSet;
class CDA_MapVariablesSet;
class CDA_VariableRefSet;
class CDA_RoleSet;
class CDA_Model;

class CDA_RDFXMLDOMRepresentation
  : public iface::cellml_api::RDFXMLDOMRepresentation
{
public:
  CDA_RDFXMLDOMRepresentation(CDA_Model* aModel);
  virtual ~CDA_RDFXMLDOMRepresentation();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(cellml_api::RDFRepresentation, cellml_api::RDFXMLDOMRepresentation);
  CDA_IMPL_ID;

  std::wstring type() throw(std::exception&);
  already_AddRefd<iface::dom::Document> data() throw(std::exception&);
  void data(iface::dom::Document* aData) throw(std::exception&);
private:
  ObjRef<CDA_Model> mModel;
};

class CDA_RDFXMLStringRepresentation
  : public iface::cellml_api::RDFXMLStringRepresentation
{
public:
  CDA_RDFXMLStringRepresentation(CDA_Model* aModel);
  virtual ~CDA_RDFXMLStringRepresentation();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RDFRepresentation, cellml_api::RDFXMLStringRepresentation)
  CDA_IMPL_ID;

  std::wstring type() throw(std::exception&);
  std::wstring serialisedData() throw(std::exception&);
  void serialisedData(const std::wstring& attr) throw(std::exception&);
private:
  ObjRef<CDA_Model> mModel;
};

#ifdef ENABLE_RDF
class CDA_RDFAPIRepresentation
  : public iface::rdf_api::RDFAPIRepresentation
{
public:
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RDFRepresentation, rdf_api::RDFAPIRepresentation)
  CDA_IMPL_ID

  CDA_RDFAPIRepresentation(CDA_Model* aModel);

  std::wstring type() throw(std::exception&);
  already_AddRefd<iface::rdf_api::DataSource> source() throw(std::exception&);
  void source(iface::rdf_api::DataSource*) throw(std::exception&);

private:
  ObjRef<CDA_Model> mModel;
};
#endif

class CDA_URI
  : public iface::cellml_api::URI
{
public:
  CDA_URI(iface::dom::Element* aDatastore, const std::wstring& aNamespace, const std::wstring& aLocalname,
          const std::wstring& aQualifiedName);
  virtual ~CDA_URI();

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(cellml_api::URI)
  CDA_IMPL_ID;

  std::wstring asText() throw(std::exception&);
  void asText(const std::wstring& attr) throw(std::exception&);
private:
  ObjRef<iface::dom::Element> mDatastore;
  std::wstring mNamespace, mLocalname, mQualifiedName;
};

class CDA_CellMLElementSet;
class CDA_CellMLElementEventAdaptor;

class CDA_CellMLElement
  : public virtual iface::cellml_api::CellMLElement,
    public iface::cellml_api::CellMLDOMElement,
    public iface::events::EventTarget
{
public:
  CDA_CellMLElement(iface::XPCOM::IObject* parent, iface::dom::Element* idata);
  virtual ~CDA_CellMLElement();

  CDA_IMPL_ID;

  void add_ref()
    throw(std::exception&)
  {
    ++_cda_refcount;
    
    if (mParent != NULL)
      mParent->add_ref();
  }

  void release_ref()
    throw(std::exception&)
  {
    if (_cda_refcount == 0)
    {
      //printf("Warning: release_ref called too many times on %s.\n",
      //       typeid(this).name());
      assert(0);
    }
    bool hitZero = !--_cda_refcount;
    if (mParent == NULL)
    {
      if (hitZero)
        delete this;
    }
    else /* if the owner model is non-null, we will be destroyed when there are
          * no remaining references to the model.
          */
      mParent->release_ref();
  }

  std::wstring cellmlVersion()
    throw(std::exception&);
  std::wstring cmetaId()
    throw(std::exception&);
  void cmetaId(const std::wstring& attr)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::ExtensionElementList> extensionElements()
    throw(std::exception&);
  void insertExtensionElementAfter(iface::dom::Element* marker,
                                   iface::dom::Element* newEl)
    throw(std::exception&);
  void appendExtensionElement(iface::dom::Element* x) throw(std::exception&);
  void prependExtensionElement(iface::dom::Element* x) throw(std::exception&);
  void removeExtensionElement(iface::dom::Element* x) throw(std::exception&);
  void replaceExtensionElement(iface::dom::Element* x, iface::dom::Element* y) throw(std::exception&);
  void clearExtensionElements() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLElementSet> childElements() throw(std::exception&);
  void addElement(iface::cellml_api::CellMLElement* x) throw(std::exception&);
  void removeElement(iface::cellml_api::CellMLElement* x) throw(std::exception&);
  void removeLinkFromHereToParent();
  void replaceElement(iface::cellml_api::CellMLElement* x, iface::cellml_api::CellMLElement* y) throw(std::exception&);
  void removeByName(const std::wstring& type, const std::wstring& name) throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLElement> parentElement() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Model> modelElement() throw(std::exception&);
  void setUserData(const std::wstring& key, iface::cellml_api::UserData* data) throw(std::exception&);
  already_AddRefd<iface::cellml_api::UserData> getUserData(const std::wstring& key) throw(std::exception&);
  
  void addEventListener(const std::wstring& aType,
                        iface::events::EventListener* aListener,
                        bool aUseCapture)
    throw(std::exception&);
  void removeEventListener(const std::wstring& aType,
                           iface::events::EventListener* aListener,
                           bool aUseCapture)
    throw(std::exception&);
  bool dispatchEvent(iface::events::Event* aEvent)
    throw(std::exception&);

  already_AddRefd<iface::cellml_api::CellMLElement> clone(bool aDeep)
    throw(std::exception&);

  already_AddRefd<iface::dom::Element> domElement()
    throw(std::exception&);

  std::wstring getExtensionAttributeNS(const std::wstring& aNS, const std::wstring& aLocalName)
    throw(std::exception&);
  void setExtensionAttributeNS(const std::wstring& aNS, const std::wstring& aLocalName, const std::wstring& aValue)
    throw(std::exception&);
  void removeExtensionAttributeNS(const std::wstring& aNS, const std::wstring& aLocalName)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::ExtensionAttributeSet> extensionAttributes()
    throw(std::exception&);

  iface::XPCOM::IObject* mParent;
  iface::dom::Element* datastore;
protected:
  friend class CDA_CellMLElementSet;
  friend class CDA_CellMLImport;
  friend class CDA_Debugger;
  // This is used only when ownerModel is null. When ownerModel is assigned,
  // add_ref() must be called on ownerModel the correct number of times.
  CDA_RefCount _cda_refcount;

  CDA_CellMLElementSet* children;

  std::map<std::wstring,iface::cellml_api::UserData*> userData;

  friend class CDA_CellMLElementEventAdaptor;

  typedef std::map<iface::events::EventListener*,CDA_CellMLElementEventAdaptor*,
                   XPCOMComparator> ListenerToAdaptor_t;
  ListenerToAdaptor_t mListenerToAdaptor;
private:
  void cleanupEvents();
};

class CDA_ExtensionAttributeSet
  : public iface::cellml_api::ExtensionAttributeSet
{
public:
  CDA_ExtensionAttributeSet(iface::dom::Element* aDataStore);
  virtual ~CDA_ExtensionAttributeSet() {}

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::ExtensionAttributeSet);
  CDA_IMPL_ID;

  already_AddRefd<iface::cellml_api::ExtensionAttributeIterator> iterate()
    throw(std::exception&);

private:
  ObjRef<iface::dom::Element> mDataStore;
};

class CDA_ExtensionAttributeIterator
  : public iface::cellml_api::ExtensionAttributeIterator
{
public:
  CDA_ExtensionAttributeIterator(iface::dom::Element* aDataStore);
  virtual ~CDA_ExtensionAttributeIterator();
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::ExtensionAttributeIterator);

  already_AddRefd<iface::dom::Attr> nextAttribute() throw(std::exception&);
private:
  already_AddRefd<iface::dom::Attr> nextAttributeInternal() throw(std::exception&);

  ObjRef<iface::dom::NamedNodeMap> mNodeMap;
  ObjRef<iface::dom::Node> mPrev;
  int mPrevIndex;
  bool mDone;

  class ExtensionAttributeEventListener
    : public iface::events::EventListener
  {
  public:
    ExtensionAttributeEventListener(CDA_ExtensionAttributeIterator* aParent)
      : mParent(aParent) {}
    
    CDA_IMPL_ID;
    void add_ref() throw(std::exception&) {}
    void release_ref() throw(std::exception&) {}
    CDA_IMPL_QI1(events::EventListener);

    void handleEvent(iface::events::Event* evt)
      throw(std::exception&);

  private:
    CDA_ExtensionAttributeIterator* mParent;
  };
  friend class ExtensionAttributeEventListener;
  ExtensionAttributeEventListener eventListener;
};

class CDA_NamedCellMLElement
  : public virtual iface::cellml_api::NamedCellMLElement,
    public virtual CDA_CellMLElement
{
public:
  CDA_NamedCellMLElement(iface::XPCOM::IObject* parent, iface::dom::Element* idata);
  virtual ~CDA_NamedCellMLElement();

  std::wstring name() throw(std::exception&);
  void name(const std::wstring& attr) throw(std::exception&);
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

  CDA_IMPL_QI5(events::EventTarget, cellml_api::CellMLElement,
               cellml_api::NamedCellMLElement, cellml_api::Model,
               cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::Model> getAlternateVersion(const std::wstring& cellmlVersion)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::GroupSet> groups() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLImportSet> imports() throw(std::exception&);
  already_AddRefd<iface::cellml_api::URI> xmlBase() throw(std::exception&);
  already_AddRefd<iface::cellml_api::URI> base_uri() throw(std::exception&);
  void clearXMLBase() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitsSet> localUnits() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitsSet> modelUnits() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitsSet> allUnits() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponentSet> localComponents()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponentSet> modelComponents()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponentSet> allComponents()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::ConnectionSet> connections() throw(std::exception&);
  already_AddRefd<iface::cellml_api::GroupSet> findGroupsWithRelationshipRefName(const std::wstring& name)
    throw(std::exception&);
  void fullyInstantiateImports() throw(std::exception&);
  void asyncFullyInstantiateImports
  (iface::cellml_api::ImportInstantiationListener* listener)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponent> createComponent() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportComponent> createImportComponent()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::Units> createUnits() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportUnits> createImportUnits() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Unit> createUnit() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLImport> createCellMLImport() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariable> createCellMLVariable()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::ComponentRef> createComponentRef() throw(std::exception&);
  already_AddRefd<iface::cellml_api::RelationshipRef> createRelationshipRef()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::Group> createGroup() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Connection> createConnection() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MapComponents> createMapComponents() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MapVariables> createMapVariables() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Reaction> createReaction() throw(std::exception&);
  already_AddRefd<iface::cellml_api::VariableRef> createVariableRef() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Role> createRole() throw(std::exception&);
  uint32_t assignUniqueIdentifier();
  already_AddRefd<iface::mathml_dom::MathMLMathElement> createMathElement() throw(std::exception&);
  already_AddRefd<iface::dom::Element> createExtensionElement(const std::wstring& namespaceURI,
							      const std::wstring& qualifiedName)
    throw(std::exception&);
  std::wstring serialisedText() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLElement> clone(bool aDeep)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::RDFRepresentation>
  getRDFRepresentation(const std::wstring& type) throw(std::exception&);

  ObjRef<iface::cellml_api::DOMURLLoader> mLoader;

  // public within this module only...
  iface::dom::Document* mDoc;
  uint32_t mNextUniqueIdentifier;

  void RecursivelyChangeVersionCopy(
                                    const std::wstring& aNewNamespace,
                                    iface::dom::Node* aCopy,
                                    iface::dom::Node* aOriginal,
                                    iface::dom::Document* aNewDoc)
    throw(std::exception&);

  already_AddRefd<iface::cellml_api::Model> cloneAcrossImports()
    throw(std::exception&);
private:
  CDA_ConnectionSet* mConnectionSet;
  CDA_GroupSet* mGroupSet;
  CDA_CellMLImportSet* mImportSet;
  CDA_CellMLComponentSet* mComponentSet;
  CDA_AllComponentSet* mAllComponents, * mModelComponents;
  CDA_UnitsSet* mLocalUnits;
  CDA_AllUnitsSet* mAllUnits, * mModelUnits;
};

class CDA_MathContainer
  : public virtual iface::cellml_api::MathContainer,
    public virtual CDA_CellMLElement
{
public:
  CDA_MathContainer(iface::XPCOM::IObject* parent,
                    iface::dom::Element* element);
  virtual ~CDA_MathContainer() {};

  already_AddRefd<iface::cellml_api::MathList> math() throw(std::exception&);
  void addMath(iface::mathml_dom::MathMLElement* x)
    throw(std::exception&);
  void removeMath(iface::mathml_dom::MathMLElement* x)
    throw(std::exception&);
  void replaceMath(iface::mathml_dom::MathMLElement* x,
                   iface::mathml_dom::MathMLElement* y)
    throw(std::exception&);
  void clearMath() throw();
};

class CDA_CellMLComponentGroupMixin
  : public virtual iface::cellml_api::CellMLComponent,
    public CDA_NamedCellMLElement
{
public:
  CDA_CellMLComponentGroupMixin(iface::XPCOM::IObject* parent,
                                iface::dom::Element* compElement)
    : CDA_CellMLElement(parent, compElement),
      CDA_NamedCellMLElement(parent, compElement)
  {}

  virtual ~CDA_CellMLComponentGroupMixin()
  {}

  already_AddRefd<iface::cellml_api::CellMLComponent> encapsulationParent() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponentSet> encapsulationChildren() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponent> containmentParent() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponentSet> containmentChildren() throw(std::exception&);
};

class CDA_CellMLComponent
  : public CDA_CellMLComponentGroupMixin,
    public CDA_MathContainer
{
public:
  CDA_CellMLComponent(iface::XPCOM::IObject* parent,
                      iface::dom::Element* compElement)
    :CDA_CellMLElement(parent, compElement),
     CDA_CellMLComponentGroupMixin(parent, compElement),
     CDA_MathContainer(parent, compElement), mReactionSet(NULL),
     mVariableSet(NULL), mConnectionSet(NULL), mUnitsSet(NULL)
  {}
  virtual ~CDA_CellMLComponent();

  CDA_IMPL_QI6(events::EventTarget, cellml_api::CellMLElement,
               cellml_api::NamedCellMLElement, cellml_api::MathContainer,
               cellml_api::CellMLComponent, cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::CellMLVariableSet> variables() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitsSet> units() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ConnectionSet> connections() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ReactionSet> reactions() throw(std::exception&);
  uint32_t importNumber() throw(std::exception&);

private:
  CDA_ReactionSet* mReactionSet;
  CDA_CellMLVariableSet* mVariableSet;
  CDA_ComponentConnectionSet* mConnectionSet;
  CDA_UnitsSet* mUnitsSet;
};

class CDA_UnitsBase
  : public virtual iface::cellml_api::Units,
    public CDA_NamedCellMLElement
{
public:
  CDA_UnitsBase(iface::XPCOM::IObject* parent,
                iface::dom::Element* unitsElement)
    : CDA_CellMLElement(parent, unitsElement),
      CDA_NamedCellMLElement(parent, unitsElement),
      mUnitSet(NULL) {}
  virtual ~CDA_UnitsBase();

  bool isBaseUnits() throw(std::exception&);
  void isBaseUnits(bool attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitSet> unitCollection() throw(std::exception&);

private:
  CDA_UnitSet* mUnitSet;
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

  CDA_IMPL_QI5(events::EventTarget, cellml_api::CellMLElement,
               cellml_api::NamedCellMLElement, cellml_api::Units,
               cellml_api::CellMLDOMElement)
};


class CDA_Unit
  : public virtual iface::cellml_api::Unit,
    public virtual CDA_CellMLElement
{
public:
  CDA_Unit(iface::XPCOM::IObject* parent, iface::dom::Element* unitElement)
    : CDA_CellMLElement(parent, unitElement) {}
  virtual ~CDA_Unit() {};

  CDA_IMPL_QI4(events::EventTarget, cellml_api::Unit,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  int32_t prefix() throw(std::exception&);
  void prefix(int32_t attr) throw(std::exception&);
  double multiplier() throw(std::exception&);
  void multiplier(double attr) throw(std::exception&);
  double offset() throw(std::exception&);
  void offset(double attr) throw(std::exception&);
  double exponent() throw(std::exception&);
  void exponent(double attr) throw(std::exception&);
  std::wstring units() throw(std::exception&);
  void units(const std::wstring& attr) throw(std::exception&);
};

class CDA_CellMLImport
  : public virtual iface::cellml_api::CellMLImport,
    public virtual CDA_CellMLElement
{
public:
  CDA_CellMLImport(iface::XPCOM::IObject* parent,
                   iface::dom::Element* importElement)
    : CDA_CellMLElement(parent, importElement), mImportedModel(NULL),
      mImportConnectionSet(NULL), mImportComponentSet(NULL),
      mImportUnitsSet(NULL) {}
  virtual ~CDA_CellMLImport();

  CDA_IMPL_QI4(events::EventTarget, cellml_api::CellMLImport,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::URI> xlinkHref() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportComponentSet> components() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportUnitsSet> units() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ConnectionSet> importedConnections()
    throw(std::exception&);
  void instantiate() throw(std::exception&);
  void asyncInstantiate
  (iface::cellml_api::ImportInstantiationListener* listener)
    throw(std::exception&);
  void instantiateFromText(const std::wstring& aText) throw(std::exception&);
  bool wasInstantiated() throw(std::exception&);
  uint32_t uniqueIdentifier() throw(std::exception&);

  iface::cellml_api::Model* mImportedModel;
  already_AddRefd<iface::cellml_api::Model> importedModel() throw(std::exception&);
  
  void uninstantiate() throw(std::exception&);

  WeakReference<CDA_Model> lastIdentifierModel;
  // This number uniquely identifies the CellML import within the toplevel
  // parent model. It is computed lazily when requested and lastIdentifierModel
  // doesn't match.
  uint32_t mUniqueIdentifier;

private:
  CDA_ImportConnectionSet* mImportConnectionSet;
  CDA_ImportComponentSet* mImportComponentSet;
  CDA_ImportUnitsSet* mImportUnitsSet;
};

class CDA_ImportComponent
  : public virtual iface::cellml_api::ImportComponent,
    public CDA_CellMLComponentGroupMixin,
    public virtual iface::cellml_api::MathContainer
{
public:
  CDA_ImportComponent(iface::XPCOM::IObject* parent,
                      iface::dom::Element* importComponent)
    : CDA_CellMLElement(parent, importComponent),
      CDA_CellMLComponentGroupMixin(parent, importComponent){}
  virtual ~CDA_ImportComponent() {}

  CDA_IMPL_QI7(events::EventTarget, cellml_api::CellMLElement,
               cellml_api::NamedCellMLElement, cellml_api::MathContainer,
               cellml_api::CellMLComponent, cellml_api::ImportComponent,
               cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::CellMLVariableSet> variables() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitsSet> units() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ConnectionSet> connections() throw(std::exception&);
  uint32_t importNumber() throw(std::exception&);
  std::wstring componentRef() throw(std::exception&);
  void componentRef(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::ReactionSet> reactions() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MathList> math() throw(std::exception&);
  void addMath(iface::cellml_api::MathMLElement el) throw(std::exception&);
  void removeMath(iface::cellml_api::MathMLElement el) throw(std::exception&);
  void replaceMath(iface::cellml_api::MathMLElement x,
                   iface::cellml_api::MathMLElement y) throw(std::exception&);
  void clearMath() throw(std::exception&);

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

  CDA_IMPL_QI6(events::EventTarget, cellml_api::ImportUnits, cellml_api::Units,
               cellml_api::NamedCellMLElement, cellml_api::CellMLElement,
               cellml_api::CellMLDOMElement)

  std::wstring unitsRef() throw(std::exception&);
  void unitsRef(const std::wstring& attr) throw(std::exception&);
  bool isBaseUnits() throw(std::exception&);
  void isBaseUnits(bool attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitSet> unitCollection() throw(std::exception&);

private:
  // This is an internal API only, and *does not* increment the refcount on
  // the return value.
  CDA_Units* fetchDefinition() throw(std::exception&);
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
      CDA_NamedCellMLElement(parent, cellmlVar),
      mConnectedCellMLVariableSet(NULL) {}
  virtual ~CDA_CellMLVariable();
  CDA_IMPL_QI5(events::EventTarget, cellml_api::CellMLVariable,
               cellml_api::NamedCellMLElement, cellml_api::CellMLElement,
               cellml_api::CellMLDOMElement)

  std::wstring initialValue() throw(std::exception&);
  void initialValue(const std::wstring& attr) throw(std::exception&);
  iface::cellml_api::VariableInterface privateInterface() throw(std::exception&);
  void privateInterface(iface::cellml_api::VariableInterface attr) throw(std::exception&);
  iface::cellml_api::VariableInterface publicInterface() throw(std::exception&);
  void publicInterface(iface::cellml_api::VariableInterface attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariableSet> connectedVariables() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariable> sourceVariable() throw(std::exception&);
  std::wstring componentName() throw(std::exception&);
  std::wstring unitsName() throw(std::exception&);
  void unitsName(const std::wstring& aUnitsName) throw(std::exception&);
  already_AddRefd<iface::cellml_api::Units> unitsElement() throw(std::exception&);
  void unitsElement(iface::cellml_api::Units* aUnits) throw(std::exception&);

private:
  CDA_ConnectedCellMLVariableSet* mConnectedCellMLVariableSet;
};

class CDA_ComponentRef
  : public iface::cellml_api::ComponentRef,
    public CDA_CellMLElement
{
public:
  CDA_ComponentRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* componentRef)
    : CDA_CellMLElement(parent, componentRef), mCRSet(NULL) {}
  virtual ~CDA_ComponentRef();

  CDA_IMPL_QI4(events::EventTarget, cellml_api::ComponentRef,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  std::wstring componentName() throw(std::exception&);
  void componentName(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::ComponentRefSet> componentRefs() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ComponentRef> parentComponentRef() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Group> parentGroup() throw(std::exception&);

private:
  CDA_ComponentRefSet* mCRSet;
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

  CDA_IMPL_QI4(events::EventTarget, cellml_api::RelationshipRef,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  std::wstring name() throw(std::exception&);
  void name(const std::wstring& attr) throw(std::exception&);
  std::wstring relationship() throw(std::exception&);
  std::wstring relationshipNamespace() throw(std::exception&);
  void setRelationshipName(const std::wstring& namespaceURI, const std::wstring& name)
    throw(std::exception&);
};

class CDA_Group
  : public iface::cellml_api::Group,
    public CDA_CellMLElement
{
public:
  CDA_Group(iface::XPCOM::IObject* parent,
            iface::dom::Element* group)
    : CDA_CellMLElement(parent, group), mRRSet(NULL), mCRSet(NULL) {}
  virtual ~CDA_Group();

  CDA_IMPL_QI4(events::EventTarget, cellml_api::Group,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::RelationshipRefSet> relationshipRefs() throw(std::exception&);
  void relationshipRefs(iface::cellml_api::RelationshipRefSet* attr)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::ComponentRefSet> componentRefs() throw(std::exception&);
  void componentRefs(iface::cellml_api::ComponentRefSet* attr)
    throw(std::exception&);
  bool isEncapsulation() throw(std::exception&);
  bool isContainment() throw(std::exception&);

private:
  CDA_RelationshipRefSet* mRRSet;
  CDA_ComponentRefSet* mCRSet;
};

class CDA_Connection
  : public iface::cellml_api::Connection,
    public CDA_CellMLElement
{
public:
  CDA_Connection(iface::XPCOM::IObject* parent,
                 iface::dom::Element* connection)
    : CDA_CellMLElement(parent, connection), mMVS(NULL), mCacheSerial(0) {};
  virtual ~CDA_Connection();

  CDA_IMPL_QI4(events::EventTarget, cellml_api::Connection,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::MapComponents> componentMapping() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MapVariablesSet> variableMappings() throw(std::exception&);

private:
  CDA_MapVariablesSet* mMVS;
  cda_serial_t mCacheSerial;
  iface::cellml_api::MapComponents* mMapComponents;
};

class CDA_MapComponents
  : public iface::cellml_api::MapComponents,
    public CDA_CellMLElement
{
public:
  CDA_MapComponents(iface::XPCOM::IObject* parent,
                 iface::dom::Element* mapComponents)
    : CDA_CellMLElement(parent, mapComponents), mCacheSerial(0) {};
  virtual ~CDA_MapComponents() {}
  CDA_IMPL_QI4(events::EventTarget, cellml_api::MapComponents,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  std::wstring firstComponentName() throw(std::exception&);
  void firstComponentName(const std::wstring& attr) throw(std::exception&);
  std::wstring secondComponentName() throw(std::exception&);
  void secondComponentName(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponent> firstComponent() throw(std::exception&);
  void firstComponent(iface::cellml_api::CellMLComponent* attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponent> secondComponent() throw(std::exception&);
  void secondComponent(iface::cellml_api::CellMLComponent* attr) throw(std::exception&);

private:
  cda_serial_t mCacheSerial;
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
  CDA_IMPL_QI4(events::EventTarget, cellml_api::MapVariables,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  std::wstring firstVariableName() throw(std::exception&);
  void firstVariableName(const std::wstring& attr) throw(std::exception&);
  std::wstring secondVariableName() throw(std::exception&);
  void secondVariableName(const std::wstring& attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariable> firstVariable() throw(std::exception&);
  void firstVariable(iface::cellml_api::CellMLVariable* attr) throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariable> secondVariable() throw(std::exception&);
  void secondVariable(iface::cellml_api::CellMLVariable* attr) throw(std::exception&);
};

class CDA_Reaction
  : public iface::cellml_api::Reaction,
    public CDA_CellMLElement
{
public:
  CDA_Reaction(iface::XPCOM::IObject* parent,
                   iface::dom::Element* reaction)
    : CDA_CellMLElement(parent, reaction),
      mVariableRefSet(NULL) {}
  virtual ~CDA_Reaction();
  CDA_IMPL_QI4(events::EventTarget, cellml_api::Reaction,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement);

  already_AddRefd<iface::cellml_api::VariableRefSet> variableReferences()
    throw(std::exception&);

  bool reversible() throw(std::exception&);
  void reversible(bool attr) throw(std::exception&);

  already_AddRefd<iface::cellml_api::VariableRef>
  getVariableRef(const std::wstring& varName, bool create)
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::Role> getRoleByDeltaVariable(const std::wstring& role)
    throw(std::exception&);

private:
  CDA_VariableRefSet* mVariableRefSet;
};

class CDA_VariableRef
  : public virtual iface::cellml_api::VariableRef,
    public CDA_CellMLElement
{
public:
  CDA_VariableRef(iface::XPCOM::IObject* parent,
                   iface::dom::Element* variableRef)
    : CDA_CellMLElement(parent, variableRef), mRoleSet(NULL) {}

  virtual ~CDA_VariableRef();

  CDA_IMPL_QI4(events::EventTarget, cellml_api::VariableRef,
               cellml_api::CellMLElement, cellml_api::CellMLDOMElement)

  already_AddRefd<iface::cellml_api::CellMLVariable> variable() throw(std::exception&);
  void variable(iface::cellml_api::CellMLVariable* v) throw(std::exception&);

  std::wstring variableName() throw(std::exception&);
  void variableName(const std::wstring& varName) throw(std::exception&);

  already_AddRefd<iface::cellml_api::RoleSet> roles() throw(std::exception&);

private:
  CDA_RoleSet* mRoleSet;
};

class CDA_Role
  : public virtual iface::cellml_api::Role,
    public virtual CDA_CellMLElement,
    public CDA_MathContainer
{
public:
  CDA_Role(iface::XPCOM::IObject* parent,
           iface::dom::Element* role)
    : CDA_CellMLElement(parent, role), CDA_MathContainer(parent, role) {}
  virtual ~CDA_Role() {}

  CDA_IMPL_QI5(events::EventTarget, cellml_api::Role,
               cellml_api::CellMLElement,
               cellml_api::MathContainer, cellml_api::CellMLDOMElement)

  iface::cellml_api::Role::RoleType variableRole() throw(std::exception&);
  void variableRole(iface::cellml_api::Role::RoleType attr)
    throw(std::exception&);

  iface::cellml_api::Role::DirectionType direction() throw(std::exception&);
  void direction(iface::cellml_api::Role::DirectionType attr)
    throw(std::exception&);

  double stoichiometry() throw(std::exception&);
  void stoichiometry(double attr) throw(std::exception&);

  already_AddRefd<iface::cellml_api::CellMLVariable> deltaVariable() throw(std::exception&);
  void deltaVariable(iface::cellml_api::CellMLVariable* attr)
    throw(std::exception&);

  std::wstring deltaVariableName() throw(std::exception&);
  void deltaVariableName(const std::wstring& attr) throw(std::exception&);
};

class CDA_DOMElementIteratorBase
{
public:
  CDA_DOMElementIteratorBase(iface::dom::Element* parentElement);
  virtual ~CDA_DOMElementIteratorBase();

  already_AddRefd<iface::dom::Element> fetchNextElement();
  /*
   * fetch next element, with a hint that anything without localName aWantEl
   * can safely be skipped.
   * Notes:
   *  1) Don't call this if you ever want anything which doesn't match aWantEl
   *     on this iterator, even on a subsequent call, as this sets up state for
   *     the next call on the assumption it will be the same.
   *  2) There is no guarantee that the returned value will be aWantEl.
   *     Instead, this is a hint only, used for optimisation purposes. In
   *     particular, previously computed values from past calls to
   *     fetchNextElement() and newly inserted nodes can be returned which
   *     don't match aWantEl.
   */
  already_AddRefd<iface::dom::Element> fetchNextElement(const std::wstring& aWantEl);

private:
  iface::dom::Element *mPrevElement, *mNextElement, *mParentElement;
  iface::dom::NodeList *mNodeList;

  class IteratorChildrenModificationListener
    : public iface::events::EventListener
  {
  public:
    IteratorChildrenModificationListener(CDA_DOMElementIteratorBase* aIt)
      : mIterator(aIt)
    {
    }

    CDA_IMPL_ID;
    void add_ref() throw(std::exception&) {}
    void release_ref() throw(std::exception&) {}
    CDA_IMPL_QI1(events::EventListener);

    void handleEvent(iface::events::Event* evt)
      throw(std::exception&);
  private:
    CDA_DOMElementIteratorBase* mIterator;
  };
  IteratorChildrenModificationListener icml;
  friend class IteratorChildrenModificationListener;
};

class CDA_CellMLElementIterator
  : public CDA_DOMElementIteratorBase,
    public iface::cellml_api::CellMLElementIterator
{
public:
  CDA_CellMLElementIterator(iface::dom::Element* parentElement,
                            CDA_CellMLElementSet* ownerSet);
  virtual ~CDA_CellMLElementIterator();

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::CellMLElementIterator);
  CDA_IMPL_ID;

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  // Same as the previous, but uses aWantEl (see DOMElementIteratorBase for
  // information on the aWantEl optimisation).
  already_AddRefd<iface::cellml_api::CellMLElement> next(const std::wstring& aWantEl)
    throw(std::exception&);

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
    if (mInner)
      mInner->add_ref();
  }

  virtual ~CDA_CellMLElementIteratorOuter()
  {
    if (mInner)
      mInner->release_ref();
  }

  CDA_IMPL_ID;

  already_AddRefd<iface::cellml_api::CellMLElement> next()
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
    : CDA_DOMElementIteratorBase(parentEl)
  {
  }

  virtual ~CDA_MathMLElementIterator()
  {
  }

  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::MathMLElementIterator);
  CDA_IMPL_ID;

  already_AddRefd<iface::mathml_dom::MathMLElement> next() throw(std::exception&);
};

class CDA_ExtensionElementList
  : public iface::cellml_api::ExtensionElementList
{
public:
  CDA_ExtensionElementList(iface::dom::Element* el);
  virtual ~CDA_ExtensionElementList();
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_api::ExtensionElementList);
  CDA_IMPL_ID;

  uint32_t length() throw(std::exception&);
  bool contains(iface::dom::Element* x) throw(std::exception&);
  int32_t getIndexOf(iface::dom::Element* x) throw(std::exception&);
  already_AddRefd<iface::dom::Element> getAt(uint32_t index) throw(std::exception&);

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
  CDA_IMPL_ID;

  uint32_t length() throw(std::exception&);
  bool contains(iface::mathml_dom::MathMLElement* x) throw(std::exception&);
  already_AddRefd<iface::cellml_api::MathMLElementIterator> iterate() throw(std::exception&);

private:
  iface::dom::Element* mParentEl;
};

class CDA_CellMLElementSetUseIteratorMixin
  : public virtual iface::cellml_api::CellMLElementSet
{
public:
  uint32_t length() throw(std::exception&);
  bool contains(iface::cellml_api::CellMLElement* x) throw(std::exception&);
};

class CDA_CellMLElementSet
  : public virtual iface::cellml_api::CellMLElementSet,
    public CDA_CellMLElementSetUseIteratorMixin
{
public:
  CDA_CellMLElementSet(CDA_CellMLElement* parent, iface::dom::Element* parentEl);
  virtual ~CDA_CellMLElementSet();

  CDA_IMPL_QI1(cellml_api::CellMLElementSet)
  CDA_IMPL_ID;

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);

  void add_ref()
    throw(std::exception&)
  {
    ++_cda_refcount;
    
    if (mParent != NULL)
      mParent->add_ref();
  }

  void release_ref()
    throw(std::exception&)
  {
    if (_cda_refcount == 0)
    {
      //printf("Warning: release_ref called too many times on %s.\n",
      //       typeid(this).name());
    }

    --_cda_refcount;
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
  void addChildToWrapper(CDA_CellMLElement*);
  void removeChildFromWrapper(CDA_CellMLElement*);

  iface::XPCOM::IObject* mParent;
private:
  friend class CDA_CellMLElementIterator;
  iface::dom::Element* mElement;
  std::map<iface::dom::Element*,iface::cellml_api::CellMLElement*,XPCOMComparator> childMap;

  // This is used only when ownerModel is null. When ownerModel is assigned,
  // add_ref() must be called on ownerModel the correct number of times.
  CDA_RefCount _cda_refcount;
};

class CDA_CellMLElementSetOuter
  : public virtual iface::cellml_api::CellMLElementSet,
    public CDA_CellMLElementSetUseIteratorMixin
{
public:
  CDA_CellMLElementSetOuter(CDA_CellMLElement* aParent,
                            CDA_CellMLElementSet* aInner)
    : mParent(aParent), mInner(aInner)
  {
    if (mParent == NULL && mInner != NULL)
      mInner->add_ref();
    if (mParent != NULL)
      mParent->add_ref();
  }

  virtual ~CDA_CellMLElementSetOuter()
  {
    if (mParent == NULL && mInner != NULL)
      mInner->release_ref();
  }

  CDA_IMPL_ID;

  void add_ref()
    throw(std::exception&)
  {
    ++_cda_refcount;
    
    if (mParent != NULL)
      mParent->add_ref();
  }

  void release_ref()
    throw(std::exception&)
  {
    assert(_cda_refcount != 0);

    bool hitZero = !--_cda_refcount;

    if (mParent == NULL)
    {
      if (hitZero)
        delete this;
    }
    else /* if the owner model is non-null, we will be destroyed when there are
          * no remaining references to the model.
          */
      mParent->release_ref();
  }

protected:
  CDA_CellMLElement* mParent;
  CDA_CellMLElementSet* mInner;

private:
  CDA_RefCount _cda_refcount;
};

class CDA_NamedCellMLElementSetBase
  : public virtual iface::cellml_api::NamedCellMLElementSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_NamedCellMLElementSetBase(CDA_CellMLElement* aParent,
                                CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner), mCacheSerial(0),
      mHighWaterMark(0), mCacheComplete(false)
  {
  }

  virtual ~CDA_NamedCellMLElementSetBase() {};

  already_AddRefd<iface::cellml_api::NamedCellMLElement>
  get(const std::wstring& name)
    throw(std::exception&);

private:
  cda_serial_t mCacheSerial;
  uint32_t mHighWaterMark;
  bool mCacheComplete;
  // Warning: NamedCellMLElements in the map are not add_refd. They are
  // guaranteed to exist as long a mCacheSerial is latest, but after that, they
  // may be dangling (so don't touch them in this case).
  std::map<std::wstring, iface::cellml_api::NamedCellMLElement*> mMap;
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

  already_AddRefd<iface::cellml_api::CellMLElement> next()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponent> nextComponent()
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

class CDA_CellMLComponentEmptyIterator
  : public CDA_CellMLComponentIteratorBase
{
public:
  CDA_CellMLComponentEmptyIterator()
    : CDA_CellMLComponentIteratorBase(NULL) {}
  virtual ~CDA_CellMLComponentEmptyIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next()
    throw(std::exception&)
  {
    return NULL;
  }
};

class CDA_CellMLComponentSetBase
  : public virtual iface::cellml_api::CellMLComponentSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_CellMLComponentSetBase(CDA_CellMLElement* aParent,
                             CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aParent, aInner) {};
  virtual ~CDA_CellMLComponentSetBase() {};

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);

  already_AddRefd<iface::cellml_api::CellMLComponentIterator>
  iterateComponents()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLComponent>
  getComponent(const std::wstring& name)
    throw(std::exception&);
};

class CDA_CellMLComponentSet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_CellMLComponentSet(CDA_CellMLElement* aParent,
                         CDA_CellMLElementSet* aInner)
    : CDA_CellMLComponentSetBase(aParent, aInner) {};
  virtual ~CDA_CellMLComponentSet() {}

  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)
};

class CDA_CellMLComponentEmptySet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_CellMLComponentEmptySet()
    : CDA_CellMLComponentSetBase(NULL, NULL)
  {
  }

  virtual ~CDA_CellMLComponentEmptySet()
  {
  }

  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&)
  {
    return new CDA_CellMLComponentEmptyIterator();
  }
};

class CDA_CellMLComponentFromComponentRefSet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_CellMLComponentFromComponentRefSet
  (
   iface::cellml_api::Model* aModel,
   iface::cellml_api::ComponentRefSet* aCompRefSet
  )
    : CDA_CellMLComponentSetBase(NULL, NULL),
      mModel(aModel), mCompRefSet(aCompRefSet)
  {
  }

  virtual ~CDA_CellMLComponentFromComponentRefSet()
  {
  }

  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
private:
  ObjRef<iface::cellml_api::Model> mModel;
  ObjRef<iface::cellml_api::ComponentRefSet> mCompRefSet;
};

class CDA_ImportComponentIterator
  : public iface::cellml_api::ImportComponentIterator,
    public CDA_CellMLComponentIteratorBase
{
public:
  CDA_ImportComponentIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLComponentIteratorBase(aInner) {}
  virtual ~CDA_ImportComponentIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(cellml_api::ImportComponentIterator,
               cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportComponent> nextImportComponent()
    throw(std::exception&);
};

class CDA_ImportComponentSet
  : public virtual iface::cellml_api::ImportComponentSet,
    public CDA_CellMLComponentSetBase
{
public:
  CDA_ImportComponentSet(CDA_CellMLElement* aParent,
                         CDA_CellMLElementSet* aInner)
    : CDA_CellMLComponentSetBase(aParent, aInner) {}
  virtual ~CDA_ImportComponentSet() {};

  CDA_IMPL_QI4(cellml_api::ImportComponentSet, cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::ImportComponentIterator> iterateImportComponents()
    throw(std::exception&);

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);

  already_AddRefd<iface::cellml_api::ImportComponent>
  getImportComponent(const std::wstring& name)
    throw(std::exception&);
};

class CDA_AllComponentSet
  : public CDA_CellMLComponentSetBase
{
public:
  CDA_AllComponentSet(
                      CDA_CellMLElement* aParent,
                      iface::cellml_api::CellMLComponentSet* aLocalSet,
                      iface::cellml_api::CellMLImportSet* aImportSet,
                      bool aRecurseIntoImports)
    : CDA_CellMLComponentSetBase(aParent, NULL),
      mLocalSet(aLocalSet), mImportSet(aImportSet),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllComponentSet() {}

  CDA_IMPL_QI3(cellml_api::CellMLComponentSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)
  
  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
private:
  // These are held by the model, no need to add_ref.
  iface::cellml_api::CellMLComponentSet* mLocalSet;
  iface::cellml_api::CellMLImportSet* mImportSet;
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
    : CDA_CellMLComponentIteratorBase(NULL),
      mLocalIterator(aLocalIterator), mImportIterator(aImportIterator),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllComponentIterator() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLComponentIterator, cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);

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
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_CellMLVariableIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLVariableIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariable> nextVariable()
    throw(std::exception&);
};

class CDA_CellMLVariableSet
  : public virtual iface::cellml_api::CellMLVariableSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_CellMLVariableSet(CDA_CellMLElement* aParent,
                        CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aParent, aInner) {}
  virtual ~CDA_CellMLVariableSet() {}

  CDA_IMPL_QI3(cellml_api::CellMLVariableSet, cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLVariableIterator> iterateVariables()
    throw(std::exception&);

  already_AddRefd<iface::cellml_api::CellMLVariable>
  getVariable(const std::wstring& name)
    throw(std::exception&);
};

class CDA_ConnectedCellMLVariableSet
  : public CDA_CellMLVariableSet
{
public:
  CDA_ConnectedCellMLVariableSet(CDA_CellMLElement* aParent,
                                 CDA_CellMLVariable* aConnectedToWhat)
    : CDA_CellMLVariableSet(aParent, NULL), mConnectedToWhat(aConnectedToWhat)
  {
    // No need to add_ref mConnectedToWhat, because it is our parent.
  }
  virtual ~CDA_ConnectedCellMLVariableSet()
  {
  }

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate()
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

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Units> nextUnits() throw(std::exception&);
};

class CDA_UnitsIterator
  : public CDA_UnitsIteratorBase
{
public:
  CDA_UnitsIterator(CDA_CellMLElementIterator* aInner)
    : CDA_UnitsIteratorBase(aInner) {};
  virtual ~CDA_UnitsIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitsIterator, cellml_api::CellMLElementIterator)
};

class CDA_UnitsSetBase
  : public virtual iface::cellml_api::UnitsSet,
    public CDA_NamedCellMLElementSetBase
{
public:
  CDA_UnitsSetBase(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_NamedCellMLElementSetBase(aParent, aInner) {}
  virtual ~CDA_UnitsSetBase() {}

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitsIterator> iterateUnits() throw(std::exception&);

  already_AddRefd<iface::cellml_api::Units>
  getUnits(const std::wstring& name)
    throw(std::exception&);
};

class CDA_UnitsSet
  : public CDA_UnitsSetBase
{
public:
  CDA_UnitsSet(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_UnitsSetBase(aParent, aInner) {}
  virtual ~CDA_UnitsSet() {}
  
  CDA_IMPL_QI3(cellml_api::UnitsSet, cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet)
};

class CDA_AllUnitsSet
  : public CDA_UnitsSetBase
{
public:
  CDA_AllUnitsSet(CDA_CellMLElement* aParent,
                  iface::cellml_api::UnitsSet* aLocalSet,
                  iface::cellml_api::CellMLImportSet* aImportSet,
                  bool aRecurseIntoImports)
    : CDA_UnitsSetBase(aParent, NULL), mLocalSet(aLocalSet),
      mImportSet(aImportSet), mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllUnitsSet() {}

  CDA_IMPL_QI3(cellml_api::UnitsSet, cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet);
  
  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
private:
  // No need to hold references to mLocalSet and mImportSet, because they are
  // all kept by the model, which is our parent.
  iface::cellml_api::UnitsSet* mLocalSet;
  iface::cellml_api::CellMLImportSet* mImportSet;
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
    : CDA_UnitsIteratorBase(NULL),
      mLocalIterator(aLocalIterator), mImportIterator(aImportIterator),
      mRecurseIntoImports(aRecurseIntoImports)
  {}
  virtual ~CDA_AllUnitsIterator() {};

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitsIterator, cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);

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
    : CDA_UnitsIteratorBase(aInner) {}
  virtual ~CDA_ImportUnitsIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI3(
               cellml_api::ImportUnitsIterator,
               cellml_api::UnitsIterator,
               cellml_api::CellMLElementIterator
              )

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportUnits> nextImportUnits()
    throw(std::exception&);
};

class CDA_ImportUnitsSet
  : public iface::cellml_api::ImportUnitsSet,
    public CDA_UnitsSetBase
{
public:
  CDA_ImportUnitsSet(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_UnitsSetBase(aParent, aInner) {}
  virtual ~CDA_ImportUnitsSet() {};

  CDA_IMPL_QI4(
               cellml_api::ImportUnitsSet,
               cellml_api::UnitsSet,
               cellml_api::NamedCellMLElementSet,
               cellml_api::CellMLElementSet
              )

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ImportUnitsIterator> iterateImportUnits()
    throw(std::exception&);

  already_AddRefd<iface::cellml_api::ImportUnits>
  getImportUnits(const std::wstring& name)
    throw(std::exception&);
};

class CDA_CellMLImportIterator
  : public iface::cellml_api::CellMLImportIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_CellMLImportIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_CellMLImportIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLImportIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLImport> nextImport()
    throw(std::exception&);
};

class CDA_CellMLImportSet
  : public iface::cellml_api::CellMLImportSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_CellMLImportSet(CDA_CellMLElement* aParent,
                      CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_CellMLImportSet() {}

  CDA_IMPL_QI2(cellml_api::CellMLImportSet,
               cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::CellMLImportIterator> iterateImports()
    throw(std::exception&);
};

class CDA_UnitIterator
  : public iface::cellml_api::UnitIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_UnitIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_UnitIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::UnitIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Unit> nextUnit()
    throw(std::exception&);
};

class CDA_UnitSet
  : public iface::cellml_api::UnitSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_UnitSet(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_UnitSet() {};

  CDA_IMPL_QI2(cellml_api::UnitSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::UnitIterator> iterateUnits()
    throw(std::exception&);
};

class CDA_ConnectionIterator
  : public iface::cellml_api::ConnectionIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_ConnectionIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_ConnectionIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::ConnectionIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Connection> nextConnection()
    throw(std::exception&);
};

class CDA_ConnectionSet
  : public iface::cellml_api::ConnectionSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_ConnectionSet(CDA_CellMLElement* aParent,
                    CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_ConnectionSet() {};

  CDA_IMPL_QI2(cellml_api::ConnectionSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ConnectionIterator> iterateConnections() throw(std::exception&);
};

class CDA_GroupIterator
  : public iface::cellml_api::GroupIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_GroupIterator(CDA_CellMLElementIterator* aInner, bool aFilterByRRName,
                    const std::wstring& aFilterRRName)
    : CDA_CellMLElementIteratorOuter(aInner),
      filterByRRName(aFilterByRRName), mFilterRRName(aFilterRRName) {}
  virtual ~CDA_GroupIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::GroupIterator, cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Group> nextGroup() throw(std::exception&);

private:
  bool filterByRRName;
  std::wstring mFilterRRName;
};

class CDA_GroupSet
  : public iface::cellml_api::GroupSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_GroupSet(CDA_CellMLElement* aParent,
               CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner),
      filterByRRName(false) {}
  CDA_GroupSet(CDA_CellMLElementSet* aInner, const std::wstring& aFilterRRName)
    : CDA_CellMLElementSetOuter(NULL, aInner),
      filterByRRName(true), mFilterRRName(aFilterRRName) {}

  virtual ~CDA_GroupSet() {}

  CDA_IMPL_QI2(cellml_api::GroupSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::GroupIterator> iterateGroups() throw(std::exception&);
  already_AddRefd<iface::cellml_api::GroupSet> getSubsetInvolvingRelationship
  (const std::wstring& relName) throw(std::exception&);
  already_AddRefd<iface::cellml_api::GroupSet> subsetInvolvingEncapsulation()
    throw(std::exception&);
  already_AddRefd<iface::cellml_api::GroupSet> subsetInvolvingContainment()
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
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_RelationshipRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RelationshipRefIterator,
               cellml_api::CellMLElementIterator)

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::RelationshipRef> nextRelationshipRef() throw(std::exception&);
};

class CDA_RelationshipRefSet
  : public iface::cellml_api::RelationshipRefSet,
  public CDA_CellMLElementSetOuter
{
public:
  CDA_RelationshipRefSet(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_RelationshipRefSet() {}

  CDA_IMPL_QI2(cellml_api::RelationshipRefSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::RelationshipRefIterator> iterateRelationshipRefs() throw(std::exception&);
};

class CDA_ComponentRefIterator
  : public iface::cellml_api::ComponentRefIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_ComponentRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_ComponentRefIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::ComponentRefIterator, cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ComponentRef> nextComponentRef() throw(std::exception&);
};

class CDA_ComponentRefSet
  : public iface::cellml_api::ComponentRefSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_ComponentRefSet(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_ComponentRefSet() {}

  CDA_IMPL_QI2(cellml_api::ComponentRefSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ComponentRefIterator> iterateComponentRefs() throw(std::exception&);
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
    : CDA_CellMLComponentIteratorBase(NULL), mModel(aModel),
      mCompRefIterator(aCompRefIterator)
  {
  }

  virtual ~CDA_CellMLComponentFromComponentRefIterator()
  {
  }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::CellMLComponentIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);

private:
  ObjRef<iface::cellml_api::Model> mModel;
  ObjRef<iface::cellml_api::ComponentRefIterator> mCompRefIterator;
};

class CDA_MapVariablesIterator
  : public iface::cellml_api::MapVariablesIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_MapVariablesIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_MapVariablesIterator() {}

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::MapVariablesIterator, cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MapVariables> nextMapVariable() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MapVariables> nextMapVariables() throw(std::exception&);
};

class CDA_MapVariablesSet
  : public iface::cellml_api::MapVariablesSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_MapVariablesSet(CDA_CellMLElement* aParent, CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_MapVariablesSet() {}

  CDA_IMPL_QI2(cellml_api::MapVariablesSet, cellml_api::CellMLElementSet)

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::MapVariablesIterator> iterateMapVariables() throw(std::exception&);
};

class CDA_ReactionIterator
  : public virtual iface::cellml_api::ReactionIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_ReactionIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_ReactionIterator() {}
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::ReactionIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Reaction> nextReaction() throw(std::exception&);
};

class CDA_ReactionSet
  : public virtual iface::cellml_api::ReactionSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_ReactionSet(CDA_CellMLElement* aElement, CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aElement, aInner) {}
  virtual ~CDA_ReactionSet() {}

  CDA_IMPL_QI2(cellml_api::ReactionSet, cellml_api::CellMLElementSet);

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::ReactionIterator> iterateReactions()
    throw(std::exception&);
};

class CDA_VariableRefIterator
  : public virtual iface::cellml_api::VariableRefIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_VariableRefIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_VariableRefIterator() {}
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::VariableRefIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::VariableRef> nextVariableRef() throw(std::exception&);
};

class CDA_VariableRefSet
  : public virtual iface::cellml_api::VariableRefSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_VariableRefSet(CDA_CellMLElement* aParent,
                     CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_VariableRefSet() {}

  CDA_IMPL_QI2(cellml_api::VariableRefSet, cellml_api::CellMLElementSet);

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::VariableRefIterator> iterateVariableRefs()
    throw(std::exception&);
};

class CDA_RoleIterator
  : public virtual iface::cellml_api::RoleIterator,
    public CDA_CellMLElementIteratorOuter
{
public:
  CDA_RoleIterator(CDA_CellMLElementIterator* aInner)
    : CDA_CellMLElementIteratorOuter(aInner) {}
  virtual ~CDA_RoleIterator() {}
  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI2(cellml_api::RoleIterator,
               cellml_api::CellMLElementIterator);

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);
  already_AddRefd<iface::cellml_api::Role> nextRole() throw(std::exception&);
};

class CDA_RoleSet
  : public virtual iface::cellml_api::RoleSet,
    public CDA_CellMLElementSetOuter
{
public:
  CDA_RoleSet(CDA_CellMLElement* aParent,
              CDA_CellMLElementSet* aInner)
    : CDA_CellMLElementSetOuter(aParent, aInner) {}
  virtual ~CDA_RoleSet() {}

  CDA_IMPL_QI2(cellml_api::RoleSet, cellml_api::CellMLElementSet);

  already_AddRefd<iface::cellml_api::CellMLElementIterator> iterate() throw(std::exception&);
  already_AddRefd<iface::cellml_api::RoleIterator> iterateRoles()
    throw(std::exception&);
};

class CDA_ComponentConnectionIterator
  : public CDA_ConnectionIterator
{
public:
  CDA_ComponentConnectionIterator(iface::cellml_api::CellMLComponent* aWhatComponent);
  virtual ~CDA_ComponentConnectionIterator();

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);

private:
  iface::cellml_api::CellMLComponent *mWhatComponent;
  iface::cellml_api::ConnectionIterator* mConnectionIterator;
};

class CDA_ComponentConnectionSet
  : public CDA_ConnectionSet
{
public:
  CDA_ComponentConnectionSet(CDA_CellMLElement* aParent,
                             iface::cellml_api::CellMLComponent* aWhatComponent)
    : CDA_ConnectionSet(aParent, NULL), mWhatComponent(aWhatComponent)
  {
    // No need to add_ref mWhatComponent, it is also our parent.
  }

  virtual ~CDA_ComponentConnectionSet()
  {
  }

  already_AddRefd<iface::cellml_api::CellMLElementIterator>
  iterate()
    throw(std::exception&)
  {
    return new CDA_ComponentConnectionIterator(mWhatComponent);
  }
private:
  iface::cellml_api::CellMLComponent* mWhatComponent;
};

class CDA_ConnectedCellMLVariableIterator
  : public CDA_CellMLVariableIterator
{
public:
  CDA_ConnectedCellMLVariableIterator(CDA_CellMLVariable* aConnectedToWhat);
  virtual ~CDA_ConnectedCellMLVariableIterator();

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);

private:
  struct VariableStackFrame
  {
    ObjRef<CDA_CellMLVariable> whichVariable;
    ObjRef<CDA_CellMLComponent> whichComponent;
    ObjRef<iface::cellml_api::CellMLComponent> whichCompLevel;
    ObjRef<iface::cellml_api::ConnectionIterator> connectionIterator;
    ObjRef<iface::cellml_api::MapVariablesIterator> mapVariableIterator;
  };

  std::list<VariableStackFrame*> variableStack;
  bool mConsider1, mConsider2;
};

class CDA_ImportConnectionIterator
  : public CDA_ConnectionIterator
{
public:
  CDA_ImportConnectionIterator(CDA_CellMLImport* aImport)
    : CDA_ConnectionIterator(NULL), mImport(aImport)
  {
    pushStackFrame(aImport);
  }

  ~CDA_ImportConnectionIterator();

  already_AddRefd<iface::cellml_api::CellMLElement> next() throw(std::exception&);

private:

  struct ImportStackFrame
  {
    ImportStackFrame() {}
    ~ImportStackFrame() {}
    
    enum { DEEP_CONNECTIONS, SHALLOW_CONNECTIONS } mState;
    ObjRef<iface::cellml_api::CellMLImportIterator> mImportIterator;
    ObjRef<iface::cellml_api::ConnectionIterator> mConnectionIterator;
  };

  std::list<ImportStackFrame*> importStack;
  void pushStackFrame(iface::cellml_api::CellMLImport* aImport)
    throw(std::exception&);
  ObjRef<iface::cellml_api::CellMLImport> mImport;
};

class CDA_ImportConnectionSet
  : public CDA_ConnectionSet
{
public:
  CDA_ImportConnectionSet(CDA_CellMLElement* aParent,
                          CDA_CellMLImport* aImport)
    : CDA_ConnectionSet(aParent, NULL), mImport(aImport)
  {
    // No need to add_ref the import, because it is our parent.
  }

  ~CDA_ImportConnectionSet()
  {
  }

  already_AddRefd<iface::cellml_api::CellMLElementIterator>
  iterate()
    throw(std::exception&)
  {
    return new CDA_ImportConnectionIterator(mImport);
  }
private:
  CDA_CellMLImport* mImport;
};
