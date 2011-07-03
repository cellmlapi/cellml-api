#define IN_SProS_MODULE
#include "Utilities.hxx"
#include "IfaceSProS.hxx"
#include "IfaceDOM_events.hxx"
#include <map>
#include <vector>

class CDA_SProSBootstrap
  : public iface::SProS::Bootstrap
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SProS::Bootstrap);

  CDA_SProSBootstrap()
    : _cda_refcount(1) {}

  iface::SProS::SEDMLElement* createEmptySEDML() throw(std::exception&);
  iface::SProS::SEDMLElement* parseSEDMLFromURI(const wchar_t* uri, const wchar_t* relativeTo) throw(std::exception&);
  iface::SProS::SEDMLElement* parseSEDMLFromText(const wchar_t* text, const wchar_t* baseURI) throw();
  iface::SProS::SEDMLElement* makeSEDMLFromElement(iface::dom::Element* el) throw();
  wchar_t* sedmlToText(iface::SProS::SEDMLElement* el) throw();
};

class CDA_SProSBase
  : public virtual iface::SProS::Base
{
public:
  CDA_SProSBase(CDA_SProSBase* aParent, iface::dom::Element* aEl);
  ~CDA_SProSBase();
  CDA_IMPL_ID;
  void add_ref() throw();
  void release_ref() throw();

  // Base:
  iface::dom::Element* domElement() throw();
  iface::dom::NodeList* notes() throw();
  iface::dom::NodeList* annotations() throw();
  iface::SProS::Base* parent() throw();

  // Semi-private: for use within SProSImpl only.
  void reparent(CDA_SProSBase* aParent);
  void addBaseToCache(iface::SProS::Base* aEl);
  void removeBaseFromCache(iface::SProS::Base* aEl);

  CDA_SProSBase* mParent;
  ObjRef<iface::dom::Element> mDomEl;
private:
  uint32_t _cda_refcount;
};

class CDA_SomeSet
  : public virtual iface::SProS::BaseSet
{
public:
  // Note: the parent and the set share each other's reference count. They do
  // not hold counted references to each other.
  CDA_SomeSet(CDA_SProSBase* aParent,
              const wchar_t* aListName,
              const wchar_t** aElNames);
  ~CDA_SomeSet();

  CDA_IMPL_ID;
  void add_ref() throw();
  void release_ref() throw();
  void insert(iface::SProS::Base* b) throw(std::exception&);
  void remove(iface::SProS::Base* b) throw(std::exception&);
  iface::SProS::BaseIterator* iterateElements() throw();

  // semi-private (SProS internal use only):

  // These two methods don't AddRef except by setting mListElement
  void emptyElementCache();
  void checkListParentAndInvalidateIfWrong();
  iface::dom::Element* tryFindListElement();
  iface::dom::Element* findOrCreateListElement();
  bool checkLocalNameMatch(const wchar_t* aCheck);
  CDA_SProSBase* wrapOrFindElement(CDA_SProSBase*, iface::dom::Element*);

  void cache(const std::string& aIDS, CDA_SProSBase* aSB);
  void decache(CDA_SProSBase* aSB);

  const wchar_t* mListName, ** mElNames;
  CDA_SProSBase* mParent;
  ObjRef<iface::dom::Element> mListElement;
  std::map<std::string, CDA_SProSBase*> mElCache;
};

class CDA_SProSDOMIteratorBase
{
public:
  CDA_SProSDOMIteratorBase(iface::dom::Element* parentElement);
  virtual ~CDA_SProSDOMIteratorBase();
  
  iface::dom::Element* fetchNextElement();
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
  iface::dom::Element* fetchNextElement(const wchar_t* aWantEl);

private:
  ObjRef<iface::dom::Element> mPrevElement, mNextElement, mParentElement;
  iface::dom::NodeList *mNodeList;

  class IteratorChildrenModificationListener
    : public iface::events::EventListener
  {
  public:
    IteratorChildrenModificationListener(CDA_SProSDOMIteratorBase* aIt)
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
    CDA_SProSDOMIteratorBase* mIterator;
  };
  IteratorChildrenModificationListener icml;
  friend class IteratorChildrenModificationListener;  
};

class CDA_SProSIteratorBase
  : public virtual iface::SProS::BaseIterator,
    public CDA_SProSDOMIteratorBase
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  CDA_SProSIteratorBase(CDA_SomeSet*);
  ~CDA_SProSIteratorBase();

  iface::SProS::Base* nextElement() throw();

protected:

  ObjRef<CDA_SomeSet> mSet;
  ObjRef<iface::dom::Node> mCurChild;
};

class CDA_SProSBaseIterator
  : public CDA_SProSIteratorBase
{
public:
  CDA_SProSBaseIterator(CDA_SomeSet* s) : CDA_SProSIteratorBase(s) {}
  ~CDA_SProSBaseIterator() {}
  CDA_IMPL_QI1(SProS::BaseIterator);
};


class CDA_SProSNamedElement
  : public virtual CDA_SProSBase, public virtual iface::SProS::NamedElement
{
public:
  CDA_SProSNamedElement(CDA_SProSBase* aParent,
                        iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl) {}
  ~CDA_SProSNamedElement() {}

  wchar_t* name() throw()
  {
    return mDomEl->getAttribute(L"name");
  }

  void
  name(const wchar_t* aName) throw()
  {
    mDomEl->setAttribute(L"name", aName);
  }
};

class CDA_SProSNamedElementSet
  : public CDA_SomeSet, public virtual iface::SProS::NamedElementSet
{
public:
  CDA_SProSNamedElementSet(CDA_SProSBase* aParent,
                           const wchar_t* aListName,
                           const wchar_t** aElNames)
    : CDA_SomeSet(aParent, aListName, aElNames)
  {
  }

  ~CDA_SProSNamedElementSet()
  {
  }

  iface::SProS::NamedElementIterator* iterateNamedElement() throw();
};

class CDA_SProSNamedElementIteratorBase
  : public CDA_SProSIteratorBase, public virtual iface::SProS::NamedElementIterator
{
public:
  CDA_SProSNamedElementIteratorBase(CDA_SomeSet* aS)
    : CDA_SProSIteratorBase(aS) {}
  ~CDA_SProSNamedElementIteratorBase() {}

  iface::SProS::NamedElement*
  nextNamedElement() throw()
  {
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());
    DECLARE_QUERY_INTERFACE(ret, el, SProS::NamedElement);
    return ret;
  }
};

class CDA_SProSNamedElementIterator
  : public CDA_SProSNamedElementIteratorBase
{
public:
  CDA_SProSNamedElementIterator(CDA_SomeSet* aS)
    : CDA_SProSNamedElementIteratorBase(aS) {}
  ~CDA_SProSNamedElementIterator() {}

  CDA_IMPL_QI2(SProS::BaseIterator, SProS::NamedElementIterator);
};

class CDA_SProSNamedIdentifiedElement
  : public CDA_SProSNamedElement,
    public virtual iface::SProS::NamedIdentifiedElement
{
public:
  CDA_SProSNamedIdentifiedElement(CDA_SProSBase* aParent,
                                  iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedElement(aParent, aEl) {}
  ~CDA_SProSNamedIdentifiedElement() {}

  wchar_t*
  id() throw()
  {
    return mDomEl->getAttribute(L"id");
  }

  void
  id(const wchar_t* aValue) throw()
  {
    mDomEl->setAttribute(L"id", aValue);
  }
};

class CDA_SProSNamedIdentifiedElementSet
  : public CDA_SProSNamedElementSet,
    public virtual iface::SProS::NamedIdentifiedElementSet
{
public:
  CDA_SProSNamedIdentifiedElementSet(CDA_SProSBase* aParent,
                                     const wchar_t* aListName,
                                     const wchar_t** aElNames)
    : CDA_SProSNamedElementSet(aParent, aListName, aElNames)
  {
  }

  ~CDA_SProSNamedIdentifiedElementSet()
  {
  }

  iface::SProS::NamedIdentifiedElementIterator*
  iterateNamedIdentifiedElements() throw();

  iface::SProS::NamedIdentifiedElement*
  getNamedIdentifiedElementByIdentifier(const wchar_t* aIdMatch)
    throw();
};

class CDA_SProSNamedIdentifiedElementIteratorBase
  : public CDA_SProSNamedElementIteratorBase, public virtual iface::SProS::NamedIdentifiedElementIterator
{
public:
  CDA_SProSNamedIdentifiedElementIteratorBase(CDA_SomeSet* aS)
    : CDA_SProSNamedElementIteratorBase(aS)
  {}
  ~CDA_SProSNamedIdentifiedElementIteratorBase() {}

  iface::SProS::NamedIdentifiedElement*
  nextNamedIdentifiedElement() throw()
  {
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());
    DECLARE_QUERY_INTERFACE(ret, el, SProS::NamedIdentifiedElement);
    return ret;
  }
};

class CDA_SProSNamedIdentifiedElementIterator
  : public CDA_SProSNamedIdentifiedElementIteratorBase
{
public:
  CDA_SProSNamedIdentifiedElementIterator(CDA_SomeSet* aS)
    : CDA_SProSNamedIdentifiedElementIteratorBase(aS)
  {}
  ~CDA_SProSNamedIdentifiedElementIterator() {}

  CDA_IMPL_QI3(SProS::BaseIterator, SProS::NamedElementIterator, SProS::NamedIdentifiedElementIterator);
};

#define SomeSProSSet(whatUpper) \
class CDA_SProS##whatUpper##Set \
  : public CDA_SProSNamedIdentifiedElementSet, public iface::SProS::whatUpper##Set \
{ \
public: \
  CDA_SProS##whatUpper##Set(CDA_SProSBase* aParent); \
  ~CDA_SProS##whatUpper##Set() {} \
  \
  CDA_IMPL_QI4(SProS::BaseSet, SProS::NamedElementSet, SProS::NamedIdentifiedElementSet, SProS::whatUpper##Set); \
  \
  iface::SProS::whatUpper##Iterator* iterate##whatUpper##s() throw(); \
  iface::SProS::whatUpper* get##whatUpper##ByIdentifier(const wchar_t* aId) \
    throw() \
  { \
    RETURN_INTO_OBJREF(el, iface::SProS::NamedIdentifiedElement, getNamedIdentifiedElementByIdentifier(aId)); \
    DECLARE_QUERY_INTERFACE(ret, el, SProS::whatUpper); \
    return ret; \
  } \
}; \
class CDA_SProS##whatUpper##Iterator \
  : public CDA_SProSNamedIdentifiedElementIteratorBase, \
    public virtual iface::SProS::whatUpper##Iterator \
{ \
public: \
  CDA_SProS##whatUpper##Iterator(CDA_SomeSet* aParent) : CDA_SProSNamedIdentifiedElementIteratorBase(aParent) {}; \
  ~CDA_SProS##whatUpper##Iterator() {} \
  \
  CDA_IMPL_QI4(SProS::BaseIterator, SProS::NamedElementIterator, \
               SProS::NamedIdentifiedElementIterator, SProS::whatUpper##Iterator); \
  \
  iface::SProS::whatUpper* next##whatUpper() throw() \
  { \
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());  \
    DECLARE_QUERY_INTERFACE(ret, el, SProS::whatUpper); \
    return ret; \
  } \
};
#define SomeAnonSProSSet(whatUpper) \
class CDA_SProS##whatUpper##Set \
  : public CDA_SomeSet, public iface::SProS::whatUpper##Set \
{ \
public: \
  CDA_SProS##whatUpper##Set(CDA_SProSBase* aParent); \
  ~CDA_SProS##whatUpper##Set() {} \
  \
  CDA_IMPL_QI2(SProS::BaseSet, SProS::whatUpper##Set); \
  \
  iface::SProS::whatUpper##Iterator* iterate##whatUpper##s() throw(); \
}; \
class CDA_SProS##whatUpper##Iterator \
  : public CDA_SProSIteratorBase, public iface::SProS::whatUpper##Iterator \
{ \
public: \
  CDA_SProS##whatUpper##Iterator(CDA_SomeSet* aParent) \
    : CDA_SProSIteratorBase(aParent) {} \
  ~CDA_SProS##whatUpper##Iterator() {} \
  \
  CDA_IMPL_QI2(SProS::BaseIterator, SProS::whatUpper##Iterator); \
  \
  iface::SProS::whatUpper* next##whatUpper() throw() \
  { \
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());  \
    DECLARE_QUERY_INTERFACE(ret, el, SProS::whatUpper); \
    return ret; \
  } \
};

class CDA_SProSModel;
class CDA_SProSTask;
class CDA_SProSSimulation;
class CDA_SProSDataGenerator;
class CDA_SProSOutput;
class CDA_SProSVariable;
class CDA_SProSParameter;
class CDA_SProSChange;
class CDA_SProSCurve;
SomeSProSSet(Model);
SomeSProSSet(Task);
SomeSProSSet(Simulation);
SomeSProSSet(DataGenerator);
SomeSProSSet(Output);
SomeSProSSet(Variable);
SomeSProSSet(Parameter);
SomeAnonSProSSet(Change);

class CDA_SProSSEDMLElement
  : public CDA_SProSBase, public iface::SProS::SEDMLElement
{
public:
  CDA_SProSSEDMLElement(iface::dom::Element* aElement)
    : CDA_SProSBase(NULL, aElement), mModelSet(this), mTaskSet(this),
      mSimulationSet(this), mDataGeneneratorSet(this), mOutputSet(this) {}

  CDA_IMPL_QI2(SProS::Base, SProS::SEDMLElement);

  uint32_t level() throw();
  void level(uint32_t aLevel) throw();
  uint32_t version() throw();
  void version(uint32_t aVersion) throw();
  
  iface::SProS::ModelSet* models() throw();
  iface::SProS::TaskSet* tasks() throw();
  iface::SProS::SimulationSet* simulations() throw();
  iface::SProS::DataGeneratorSet* generators() throw();
  iface::SProS::OutputSet* outputs() throw();

  iface::SProS::Model* createModel() throw();
  iface::SProS::UniformTimeCourse* createUniformTimeCourse() throw();
  iface::SProS::Task* createTask() throw();
  iface::SProS::DataGenerator* createDataGenerator() throw();
  iface::SProS::Plot2D* createPlot2D() throw();
  iface::SProS::Plot3D* createPlot3D() throw();
  iface::SProS::Report* createReport() throw();
  iface::SProS::ComputeChange* createComputeChange() throw();
  iface::SProS::ChangeAttribute* createChangeAttribute() throw();
  iface::SProS::AddXML* createAddXML() throw();
  iface::SProS::RemoveXML* createRemoveXML() throw();
  iface::SProS::ChangeXML* createChangeXML() throw();
  iface::SProS::Variable* createVariable() throw();
  iface::SProS::Parameter* createParameter() throw();
  iface::SProS::Curve* createCurve() throw();
  iface::SProS::Surface* createSurface() throw();
  iface::SProS::DataSet* createDataSet() throw();

private:
  CDA_SProSModelSet mModelSet;
  CDA_SProSTaskSet mTaskSet;
  CDA_SProSSimulationSet mSimulationSet;
  CDA_SProSDataGeneratorSet mDataGeneneratorSet;
  CDA_SProSOutputSet mOutputSet;
};

class CDA_SProSModel
  : public CDA_SProSNamedIdentifiedElement, public iface::SProS::Model
{
public:
  CDA_SProSModel(CDA_SProSBase* aParent,
                 iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedIdentifiedElement(aParent, aEl),
      mChangeSet(this) {}
  ~CDA_SProSModel() {}

  CDA_IMPL_QI4(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement, SProS::Model);

  wchar_t* language() throw();
  void language(const wchar_t*) throw();
  wchar_t* source() throw();
  void source(const wchar_t*) throw();

  iface::SProS::ChangeSet* changes() throw();

private:
  CDA_SProSChangeSet mChangeSet;
};


class CDA_SProSSimulation
  : public CDA_SProSNamedIdentifiedElement, public virtual iface::SProS::Simulation
{
public:
  CDA_SProSSimulation(CDA_SProSBase* aParent,
                      iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedIdentifiedElement(aParent, aEl)
  {
  }
  ~CDA_SProSSimulation() {}

  wchar_t* algorithmKisaoID() throw();
  void algorithmKisaoID(const wchar_t* aID) throw();
};

class CDA_SProSUniformTimeCourse
  : public CDA_SProSSimulation, public iface::SProS::UniformTimeCourse
{
public:
  CDA_SProSUniformTimeCourse(CDA_SProSBase* aParent,
                             iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSSimulation(aParent, aEl)
  {
  }
  ~CDA_SProSUniformTimeCourse() {}

  CDA_IMPL_QI5(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement, SProS::Simulation,
               SProS::UniformTimeCourse);

  double initialTime() throw();
  void initialTime(double aValue) throw();
  double outputStartTime() throw();
  void outputStartTime(double aValue) throw();
  double outputEndTime() throw();
  void outputEndTime(double aValue) throw();
  void numberOfPoints(uint32_t aNumPoints) throw();
  uint32_t numberOfPoints() throw();
};

class CDA_SProSTask
  : public iface::SProS::Task,
    public CDA_SProSNamedIdentifiedElement
{
public:
  CDA_SProSTask(CDA_SProSBase* aParent,
                iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedIdentifiedElement(aParent, aEl)
  {
  }
  ~CDA_SProSTask() {}

  CDA_IMPL_QI4(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement, SProS::Task);

  wchar_t* simulationReferenceIdentifier() throw();
  void simulationReferenceIdentifier(const wchar_t* aId) throw();
  iface::SProS::Simulation* simulationReference() throw();
  void simulationReference(iface::SProS::Simulation* aSim) throw();

  wchar_t* modelReferenceIdentifier() throw();
  void modelReferenceIdentifier(const wchar_t* aId) throw();
  iface::SProS::Model* modelReference() throw();
  void modelReference(iface::SProS::Model* aModel) throw();
};

class CDA_SProSDataGenerator
  : public iface::SProS::DataGenerator,
    public CDA_SProSNamedIdentifiedElement
{
public:
  CDA_SProSDataGenerator(CDA_SProSBase* aParent,
                         iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl),
      CDA_SProSNamedIdentifiedElement(aParent, aEl),
      mParameterSet(this), mVariableSet(this)
  {
  }
  ~CDA_SProSDataGenerator() {}

  CDA_IMPL_QI4(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement, SProS::DataGenerator);

  iface::SProS::ParameterSet* parameters() throw();
  iface::SProS::VariableSet* variables() throw();
  iface::mathml_dom::MathMLMathElement* math() throw();
  void math(iface::mathml_dom::MathMLMathElement* aEl) throw();

private:
  CDA_SProSParameterSet mParameterSet;
  CDA_SProSVariableSet mVariableSet;
};

class CDA_SProSCurve;
class CDA_SProSSurface;
class CDA_SProSCurveSetBase
  : public CDA_SProSNamedElementSet, public virtual iface::SProS::CurveSet
{
public:
  CDA_SProSCurveSetBase(CDA_SProSBase* aParent, const wchar_t* aName, const wchar_t** aElNames);
  ~CDA_SProSCurveSetBase() {}
  
  iface::SProS::CurveIterator* iterateCurves() throw();
};

class CDA_SProSCurveSet
  : public CDA_SProSCurveSetBase
{
public:
  CDA_SProSCurveSet(CDA_SProSBase* aParent);
  ~CDA_SProSCurveSet() {}

  CDA_IMPL_QI3(SProS::BaseSet, SProS::NamedElementSet, SProS::CurveSet);
};

class CDA_SProSCurveIteratorBase
  : public CDA_SProSNamedElementIteratorBase, public virtual iface::SProS::CurveIterator
{
public:
  CDA_SProSCurveIteratorBase(CDA_SomeSet* aParent)
    : CDA_SProSNamedElementIteratorBase(aParent) {}
  ~CDA_SProSCurveIteratorBase() {}
  
  iface::SProS::Curve* nextCurve() throw()
  {
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());
    DECLARE_QUERY_INTERFACE(ret, el, SProS::Curve);
    return ret;
  }
};

class CDA_SProSCurveIterator
  : public CDA_SProSCurveIteratorBase
{
public:
  CDA_SProSCurveIterator(CDA_SomeSet* aParent)
    : CDA_SProSCurveIteratorBase(aParent)
  {
  }
  ~CDA_SProSCurveIterator() {}

  CDA_IMPL_QI3(SProS::BaseIterator, SProS::NamedElementIterator, SProS::CurveIterator);
};

class CDA_SProSSurfaceSet
  : public CDA_SProSCurveSetBase, public iface::SProS::SurfaceSet
{
public:
  CDA_SProSSurfaceSet(CDA_SProSBase* aParent);
  ~CDA_SProSSurfaceSet() {}

  CDA_IMPL_QI3(SProS::BaseSet, SProS::CurveSet, SProS::SurfaceSet);

  iface::SProS::SurfaceIterator* iterateSurfaces() throw();
};

class CDA_SProSSurfaceIterator
  : public CDA_SProSCurveIteratorBase, public iface::SProS::SurfaceIterator
{
public:
  CDA_SProSSurfaceIterator(CDA_SomeSet* aParent)
    : CDA_SProSCurveIteratorBase(aParent) {}
  ~CDA_SProSSurfaceIterator() {}

  CDA_IMPL_QI3(SProS::BaseIterator, SProS::CurveIterator, SProS::SurfaceIterator);
  
  iface::SProS::Surface* nextSurface() throw()
  {
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());
    DECLARE_QUERY_INTERFACE(ret, el, SProS::Surface);
    return ret;
  }
};

class CDA_SProSOutput
  : public virtual iface::SProS::Output,
    public CDA_SProSNamedIdentifiedElement
{
public:
  CDA_SProSOutput(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedIdentifiedElement(aParent, aEl)
  {
  }
  ~CDA_SProSOutput() {};
};

class CDA_SProSPlot2D
  : public iface::SProS::Plot2D,
    public CDA_SProSOutput
{
public:
  CDA_SProSPlot2D(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSOutput(aParent, aEl),
      mCurveSet(this) {}
  ~CDA_SProSPlot2D() {}

  CDA_IMPL_QI5(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement,
               SProS::Output, SProS::Plot2D);

  iface::SProS::CurveSet* curves() throw();

private:
  CDA_SProSCurveSet mCurveSet;
};

class CDA_SProSPlot3D
  : public iface::SProS::Plot3D,
    public CDA_SProSOutput
{
public:
  CDA_SProSPlot3D(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSOutput(aParent, aEl),
      mSurfaceSet(this) {}
  ~CDA_SProSPlot3D() {}

  CDA_IMPL_QI5(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement,
               SProS::Output, SProS::Plot3D);

  iface::SProS::SurfaceSet* surfaces() throw();

private:
  CDA_SProSSurfaceSet mSurfaceSet;
};

class CDA_SProSDataSet;

class CDA_SProSDataSetSet
  : public CDA_SProSNamedElementSet, public iface::SProS::DataSetSet
{
public:
  CDA_SProSDataSetSet(CDA_SProSBase* aParent);
  ~CDA_SProSDataSetSet() {}

  CDA_IMPL_QI3(SProS::BaseSet, SProS::NamedElementSet, SProS::DataSetSet);
  
  iface::SProS::DataSetIterator* iterateDataSets() throw();
};

class CDA_SProSDataSetIterator
  : public CDA_SProSNamedElementIterator, public iface::SProS::DataSetIterator
{
public:
  CDA_SProSDataSetIterator(CDA_SProSDataSetSet* aSet)
    : CDA_SProSNamedElementIterator(aSet) {}
  ~CDA_SProSDataSetIterator() {}

  CDA_IMPL_QI3(SProS::BaseIterator, SProS::NamedElementIterator,
               SProS::DataSetIterator);
  
  iface::SProS::DataSet* nextDataSet() throw()
  {
    RETURN_INTO_OBJREF(el, iface::SProS::Base, nextElement());
    DECLARE_QUERY_INTERFACE(ret, el, SProS::DataSet);
    return ret;
  }
};

class CDA_SProSReport
  : public iface::SProS::Report,
    public CDA_SProSOutput
{
public:
  CDA_SProSReport(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSOutput(aParent, aEl),
      mDataSetSet(this) {}
  ~CDA_SProSReport() {}

  CDA_IMPL_QI5(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement,
               SProS::Output, SProS::Report);

  iface::SProS::DataSetSet* datasets() throw();

private:
  CDA_SProSDataSetSet mDataSetSet;
};

class CDA_SProSChange
  : public virtual iface::SProS::Change,
    public virtual CDA_SProSBase
{
public:
  CDA_SProSChange(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl) {}
  ~CDA_SProSChange() {}

  wchar_t* target() throw();
  void target(const wchar_t* aTarg) throw();
};

class CDA_SProSComputeChange
  : public iface::SProS::ComputeChange,
    public CDA_SProSChange
{
public:
  CDA_SProSComputeChange(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSChange(aParent, aEl),
      mVariables(this), mParameters(this) {}
  ~CDA_SProSComputeChange() {}

  CDA_IMPL_QI3(SProS::Base, SProS::Change, SProS::ComputeChange)

  iface::SProS::VariableSet* variables() throw();
  iface::SProS::ParameterSet* parameters() throw();
  iface::mathml_dom::MathMLMathElement* math() throw();
  void math(iface::mathml_dom::MathMLMathElement*) throw();
private:
  CDA_SProSVariableSet mVariables;
  CDA_SProSParameterSet mParameters;
};

class CDA_SProSChangeAttribute
  : public iface::SProS::ChangeAttribute,
    public CDA_SProSChange
{
public:
  CDA_SProSChangeAttribute(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSChange(aParent, aEl) {}
  ~CDA_SProSChangeAttribute() {}

  CDA_IMPL_QI3(SProS::Base, SProS::Change, SProS::ChangeAttribute);

  wchar_t* newValue() throw();
  void newValue(const wchar_t* aValue) throw();
};

class CDA_SProSAddXML
  : public virtual iface::SProS::AddXML,
    public CDA_SProSChange
{
public:
  CDA_SProSAddXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSChange(aParent, aEl) {}
  ~CDA_SProSAddXML() {}

  iface::dom::NodeList* anyXML() throw();

  CDA_IMPL_QI3(SProS::Base, SProS::Change, SProS::AddXML);
};

class CDA_SProSPrecomputedNodeList
  : public iface::dom::NodeList
{
public:
  CDA_SProSPrecomputedNodeList() : _cda_refcount(1) {};
  ~CDA_SProSPrecomputedNodeList();

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(dom::NodeList);

  iface::dom::Node* item(uint32_t aIndex) throw(std::exception&);
  uint32_t length() throw();

  // SProS internal use only:
  void add(iface::dom::Node*);

private:
  std::vector<iface::dom::Node*> mNodeList;
};

class CDA_SProSChangeXML
  : public iface::SProS::ChangeXML,
    public CDA_SProSAddXML
{
public:
  CDA_SProSChangeXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSAddXML(aParent, aEl) {}
  ~CDA_SProSChangeXML() {}

  CDA_IMPL_QI4(SProS::Base, SProS::Change, SProS::AddXML, SProS::ChangeXML);
};

class CDA_SProSRemoveXML
  : public iface::SProS::RemoveXML,
    public CDA_SProSChange
{
public:
  CDA_SProSRemoveXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSChange(aParent, aEl) {}
  ~CDA_SProSRemoveXML() {}

  CDA_IMPL_QI3(SProS::Base, SProS::Change, SProS::RemoveXML);
};

class CDA_SProSVariable
  : public iface::SProS::Variable,
    public CDA_SProSNamedIdentifiedElement
{
public:
  CDA_SProSVariable(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedIdentifiedElement(aParent, aEl)
  {
  }
  ~CDA_SProSVariable() {}

  CDA_IMPL_QI4(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement,
               SProS::Variable);

  wchar_t* target() throw();
  void target(const wchar_t* aTarget) throw();
  wchar_t* symbol() throw();
  void symbol(const wchar_t* aTarget) throw();
  wchar_t* taskReferenceID() throw();
  void taskReferenceID(const wchar_t* aTarget) throw();
  iface::SProS::Task* taskReference() throw();
  void taskReference(iface::SProS::Task* aTask) throw(std::exception&);
};

class CDA_SProSParameter
  : public iface::SProS::Parameter,
    public CDA_SProSNamedIdentifiedElement
{
public:
  CDA_SProSParameter(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedIdentifiedElement(aParent, aEl)
  {
  }
  ~CDA_SProSParameter() {}

  CDA_IMPL_QI4(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement,
               SProS::Parameter);

  double value() throw();
  void value(double aTarget) throw();
};

class CDA_SProSCurve
  : public CDA_SProSNamedElement,
    public virtual iface::SProS::Curve
{
public:
  CDA_SProSCurve(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSNamedElement(aParent, aEl)
  {
  }
  ~CDA_SProSCurve() {}

  CDA_IMPL_QI3(SProS::Base, SProS::NamedElement, SProS::Curve);

  bool logX() throw();
  void logX(bool aValue) throw();
  bool logY() throw();
  void logY(bool aValue) throw();
  wchar_t* xDataGeneratorID() throw();
  void xDataGeneratorID(const wchar_t* aValue) throw();
  wchar_t* yDataGeneratorID() throw();
  void yDataGeneratorID(const wchar_t* aValue) throw();
  iface::SProS::DataGenerator* xDataGenerator() throw();
  void xDataGenerator(iface::SProS::DataGenerator* aValue) throw();
  iface::SProS::DataGenerator* yDataGenerator() throw();
  void yDataGenerator(iface::SProS::DataGenerator* aValue) throw();
};

class CDA_SProSSurface
  : public CDA_SProSCurve,
    public iface::SProS::Surface
{
public:
  CDA_SProSSurface(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSCurve(aParent, aEl) {}
  ~CDA_SProSSurface() {}

  CDA_IMPL_QI4(SProS::Base, SProS::NamedElement, SProS::Curve, SProS::Surface);
  
  bool logZ() throw();
  void logZ(bool aValue) throw();

  wchar_t* zDataGeneratorID() throw();
  void zDataGeneratorID(const wchar_t* aValue) throw();
  iface::SProS::DataGenerator* zDataGenerator() throw();
  void zDataGenerator(iface::SProS::DataGenerator* aValue) throw();
};
  
class CDA_SProSDataSet
  : public CDA_SProSNamedElement, public iface::SProS::DataSet
{
public:
  CDA_SProSDataSet(CDA_SProSBase* aParent, iface::dom::Element* aEl);
  ~CDA_SProSDataSet() {}
  
  CDA_IMPL_QI3(SProS::Base, SProS::NamedElement, SProS::DataSet);

  wchar_t* dataGeneratorID() throw();
  void dataGeneratorID(const wchar_t*) throw();
  iface::SProS::DataGenerator* dataGen() throw();
  void dataGen(iface::SProS::DataGenerator*) throw();
};

#if 0
class CDA_SProSNotes
  : public CDA_SProSBase, public iface::SProS::Notes
{
public:
  CDA_SProSNotes(iface::SProS::Base* aParent)
    : CDA_SProSBase(aParent) {}
  ~CDA_SProSNotes() {}

  CDA_IMPL_QI2(SProS::Base, SProS::Notes);
};

SomeAnonSProSSet(Notes);

class CDA_SProSAnnotation
  : public CDA_SProSBase, public iface::SProS::Annotation
{
public:
  CDA_SProSAnnotation(iface::SProS::Base* aParent)
    : CDA_SProSBase(aParent) {}
  ~CDA_SProSAnnotation() {}

  CDA_IMPL_QI2(SProS::Base, SProS::Annotation);
};
SomeAnonSProSSet(Annotation);
#endif
