#define IN_SProS_MODULE
#include "Utilities.hxx"
#include "IfaceSProS.hxx"
#include "IfaceDOM_events.hxx"
#include <string>
#include <map>
#include <vector>

class CDA_SProSBootstrap
  : public iface::SProS::Bootstrap
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SProS::Bootstrap);

  CDA_SProSBootstrap() {}

  already_AddRefd<iface::SProS::SEDMLElement> createEmptySEDML() throw(std::exception&);
  already_AddRefd<iface::SProS::SEDMLElement> parseSEDMLFromURI(const std::wstring& uri, const std::wstring& relativeTo) throw(std::exception&);
  already_AddRefd<iface::SProS::SEDMLElement> parseSEDMLFromText(const std::wstring& text, const std::wstring& baseURI) throw();
  already_AddRefd<iface::SProS::SEDMLElement> makeSEDMLFromElement(iface::dom::Element* el) throw();
  std::wstring sedmlToText(iface::SProS::SEDMLElement* el) throw();
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
  already_AddRefd<iface::dom::Element> domElement() throw();
  already_AddRefd<iface::dom::NodeList> notes() throw();
  already_AddRefd<iface::dom::NodeList> annotations() throw();
  already_AddRefd<iface::SProS::Base> parent() throw();

  // Semi-private: for use within SProSImpl only.
  void reparent(CDA_SProSBase* aParent);
  void addBaseToCache(iface::SProS::Base* aEl);
  void removeBaseFromCache(iface::SProS::Base* aEl);

  CDA_SProSBase* mParent;
  ObjRef<iface::dom::Element> mDomEl;
private:
  CDA_RefCount _cda_refcount;
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
  already_AddRefd<iface::SProS::BaseIterator> iterateElements() throw();

  // semi-private (SProS internal use only):

  // These two methods don't AddRef except by setting mListElement
  void emptyElementCache();
  void checkListParentAndInvalidateIfWrong();
  iface::dom::Element* tryFindListElement();
  iface::dom::Element* findOrCreateListElement();
  bool checkLocalNameMatch(const std::wstring& aCheck);
  already_AddRefd<CDA_SProSBase> wrapOrFindElement(CDA_SProSBase*, iface::dom::Element*);

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

  already_AddRefd<iface::SProS::Base> nextElement() throw();

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

  std::wstring name() throw()
  {
    return mDomEl->getAttribute(L"name");
  }

  void
  name(const std::wstring& aName) throw()
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

  already_AddRefd<iface::SProS::NamedElementIterator> iterateNamedElement() throw();
};

class CDA_SProSNamedElementIteratorBase
  : public CDA_SProSIteratorBase, public virtual iface::SProS::NamedElementIterator
{
public:
  CDA_SProSNamedElementIteratorBase(CDA_SomeSet* aS)
    : CDA_SProSIteratorBase(aS) {}
  ~CDA_SProSNamedElementIteratorBase() {}

  already_AddRefd<iface::SProS::NamedElement>
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

  std::wstring
  id() throw()
  {
    return mDomEl->getAttribute(L"id");
  }

  void
  id(const std::wstring& aValue) throw()
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

  already_AddRefd<iface::SProS::NamedIdentifiedElementIterator>
  iterateNamedIdentifiedElements() throw();

  already_AddRefd<iface::SProS::NamedIdentifiedElement>
  getNamedIdentifiedElementByIdentifier(const std::wstring& aIdMatch)
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

  already_AddRefd<iface::SProS::NamedIdentifiedElement>
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
  already_AddRefd<iface::SProS::whatUpper##Iterator> iterate##whatUpper##s() throw(); \
  already_AddRefd<iface::SProS::whatUpper> get##whatUpper##ByIdentifier(const std::wstring& aId) \
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
  already_AddRefd<iface::SProS::whatUpper> next##whatUpper() throw()    \
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
  already_AddRefd<iface::SProS::whatUpper##Iterator> iterate##whatUpper##s() throw(); \
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
  already_AddRefd<iface::SProS::whatUpper> next##whatUpper() throw()    \
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
  
  already_AddRefd<iface::SProS::ModelSet> models() throw();
  already_AddRefd<iface::SProS::TaskSet> tasks() throw();
  already_AddRefd<iface::SProS::SimulationSet> simulations() throw();
  already_AddRefd<iface::SProS::DataGeneratorSet> generators() throw();
  already_AddRefd<iface::SProS::OutputSet> outputs() throw();

  already_AddRefd<iface::SProS::Model> createModel() throw();
  already_AddRefd<iface::SProS::UniformTimeCourse> createUniformTimeCourse() throw();
  already_AddRefd<iface::SProS::SamplingSensitivityAnalysis> createSamplingSensitivityAnalysis() throw();
  already_AddRefd<iface::SProS::Task> createTask() throw();
  already_AddRefd<iface::SProS::DataGenerator> createDataGenerator() throw();
  already_AddRefd<iface::SProS::Plot2D> createPlot2D() throw();
  already_AddRefd<iface::SProS::Plot3D> createPlot3D() throw();
  already_AddRefd<iface::SProS::Report> createReport() throw();
  already_AddRefd<iface::SProS::ComputeChange> createComputeChange() throw();
  already_AddRefd<iface::SProS::ChangeAttribute> createChangeAttribute() throw();
  already_AddRefd<iface::SProS::AddXML> createAddXML() throw();
  already_AddRefd<iface::SProS::RemoveXML> createRemoveXML() throw();
  already_AddRefd<iface::SProS::ChangeXML> createChangeXML() throw();
  already_AddRefd<iface::SProS::Variable> createVariable() throw();
  already_AddRefd<iface::SProS::Parameter> createParameter() throw();
  already_AddRefd<iface::SProS::Curve> createCurve() throw();
  already_AddRefd<iface::SProS::Surface> createSurface() throw();
  already_AddRefd<iface::SProS::DataSet> createDataSet() throw();
  std::wstring originalURL() throw();
  void originalURL(const std::wstring& aURL) throw();

private:
  CDA_SProSModelSet mModelSet;
  CDA_SProSTaskSet mTaskSet;
  CDA_SProSSimulationSet mSimulationSet;
  CDA_SProSDataGeneratorSet mDataGeneneratorSet;
  CDA_SProSOutputSet mOutputSet;
  std::wstring mOriginalURL;
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

  std::wstring language() throw();
  void language(const std::wstring&) throw();
  std::wstring source() throw();
  void source(const std::wstring&) throw();

  already_AddRefd<iface::SProS::ChangeSet> changes() throw();

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

  std::wstring algorithmKisaoID() throw();
  void algorithmKisaoID(const std::wstring& aID) throw();
};

class CDA_SProSUniformTimeCourseBase
  : public CDA_SProSSimulation, public virtual iface::SProS::UniformTimeCourse
{
public:
  CDA_SProSUniformTimeCourseBase(CDA_SProSBase* aParent,
                                 iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSSimulation(aParent, aEl)
  {
  }
  ~CDA_SProSUniformTimeCourseBase() {}

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

class CDA_SProSUniformTimeCourse
  : public CDA_SProSUniformTimeCourseBase
{
public:
  CDA_SProSUniformTimeCourse(CDA_SProSBase* aParent,
                             iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSUniformTimeCourseBase(aParent, aEl)
  {
  }

  CDA_IMPL_QI5(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement, SProS::Simulation,
               SProS::UniformTimeCourse);
};

class CDA_SProSSamplingSensitivityAnalysis
  : public iface::SProS::SamplingSensitivityAnalysis, public CDA_SProSUniformTimeCourseBase
{
public:
  CDA_SProSSamplingSensitivityAnalysis(CDA_SProSBase* aParent,
                                       iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSUniformTimeCourseBase(aParent, aEl)
  {
  }
  ~CDA_SProSSamplingSensitivityAnalysis() {}

  CDA_IMPL_QI6(SProS::Base, SProS::NamedElement, SProS::NamedIdentifiedElement, SProS::Simulation,
               SProS::UniformTimeCourse, SProS::SamplingSensitivityAnalysis);

  uint32_t numberOfSamples() throw()
  {
    RETURN_INTO_WSTRING(it, mDomEl->getAttribute(L"numberOfSamples"));
    return wcstoul(it.c_str(), NULL, 10);
  };

  void numberOfSamples(uint32_t aSamples) throw()
  {
    wchar_t buf[32];
    any_swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"%lu", aSamples);
    mDomEl->setAttribute(L"numberOfSamples", buf);
  }
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

  std::wstring simulationReferenceIdentifier() throw();
  void simulationReferenceIdentifier(const std::wstring& aId) throw();
  already_AddRefd<iface::SProS::Simulation> simulationReference() throw();
  void simulationReference(iface::SProS::Simulation* aSim) throw();

  std::wstring modelReferenceIdentifier() throw();
  void modelReferenceIdentifier(const std::wstring& aId) throw();
  already_AddRefd<iface::SProS::Model> modelReference() throw();
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
  
  already_AddRefd<iface::SProS::ParameterSet> parameters() throw();
  already_AddRefd<iface::SProS::VariableSet> variables() throw();
  already_AddRefd<iface::mathml_dom::MathMLMathElement> math() throw();
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
  
  already_AddRefd<iface::SProS::CurveIterator> iterateCurves() throw();
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
  
  already_AddRefd<iface::SProS::Curve> nextCurve() throw()
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

  already_AddRefd<iface::SProS::SurfaceIterator> iterateSurfaces() throw();
};

class CDA_SProSSurfaceIterator
  : public CDA_SProSCurveIteratorBase, public iface::SProS::SurfaceIterator
{
public:
  CDA_SProSSurfaceIterator(CDA_SomeSet* aParent)
    : CDA_SProSCurveIteratorBase(aParent) {}
  ~CDA_SProSSurfaceIterator() {}

  CDA_IMPL_QI3(SProS::BaseIterator, SProS::CurveIterator, SProS::SurfaceIterator);
  
  already_AddRefd<iface::SProS::Surface> nextSurface() throw()
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

  already_AddRefd<iface::SProS::CurveSet> curves() throw();

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

  already_AddRefd<iface::SProS::SurfaceSet> surfaces() throw();

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
  
  already_AddRefd<iface::SProS::DataSetIterator> iterateDataSets() throw();
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
  
  already_AddRefd<iface::SProS::DataSet> nextDataSet() throw()
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

  already_AddRefd<iface::SProS::DataSetSet> datasets() throw();

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

  std::wstring target() throw();
  void target(const std::wstring& aTarg) throw();
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

  already_AddRefd<iface::SProS::VariableSet> variables() throw();
  already_AddRefd<iface::SProS::ParameterSet> parameters() throw();
  already_AddRefd<iface::mathml_dom::MathMLMathElement> math() throw();
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

  std::wstring newValue() throw();
  void newValue(const std::wstring& aValue) throw();
};

class CDA_SProSAddXML
  : public virtual iface::SProS::AddXML,
    public CDA_SProSChange
{
public:
  CDA_SProSAddXML(CDA_SProSBase* aParent, iface::dom::Element* aEl)
    : CDA_SProSBase(aParent, aEl), CDA_SProSChange(aParent, aEl) {}
  ~CDA_SProSAddXML() {}

  already_AddRefd<iface::dom::NodeList> anyXML() throw();

  CDA_IMPL_QI3(SProS::Base, SProS::Change, SProS::AddXML);
};

class CDA_SProSPrecomputedNodeList
  : public iface::dom::NodeList
{
public:
  CDA_SProSPrecomputedNodeList() {};
  ~CDA_SProSPrecomputedNodeList();

  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(dom::NodeList);

  already_AddRefd<iface::dom::Node> item(uint32_t aIndex) throw(std::exception&);
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

  std::wstring target() throw();
  void target(const std::wstring& aTarget) throw();
  std::wstring symbol() throw();
  void symbol(const std::wstring& aTarget) throw();
  std::wstring taskReferenceID() throw();
  void taskReferenceID(const std::wstring& aTarget) throw();
  already_AddRefd<iface::SProS::Task> taskReference() throw();
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
  std::wstring xDataGeneratorID() throw();
  void xDataGeneratorID(const std::wstring& aValue) throw();
  std::wstring yDataGeneratorID() throw();
  void yDataGeneratorID(const std::wstring& aValue) throw();
  already_AddRefd<iface::SProS::DataGenerator> xDataGenerator() throw();
  void xDataGenerator(iface::SProS::DataGenerator* aValue) throw();
  already_AddRefd<iface::SProS::DataGenerator> yDataGenerator() throw();
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

  std::wstring zDataGeneratorID() throw();
  void zDataGeneratorID(const std::wstring& aValue) throw();
  already_AddRefd<iface::SProS::DataGenerator> zDataGenerator() throw();
  void zDataGenerator(iface::SProS::DataGenerator* aValue) throw();
};
  
class CDA_SProSDataSet
  : public CDA_SProSNamedElement, public iface::SProS::DataSet
{
public:
  CDA_SProSDataSet(CDA_SProSBase* aParent, iface::dom::Element* aEl);
  ~CDA_SProSDataSet() {}
  
  CDA_IMPL_QI3(SProS::Base, SProS::NamedElement, SProS::DataSet);

  std::wstring dataGeneratorID() throw();
  void dataGeneratorID(const std::wstring&) throw();
  already_AddRefd<iface::SProS::DataGenerator> dataGen() throw();
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
