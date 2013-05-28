#include "SRuSBootstrap.hpp"
#include "Utilities.hxx"
#include "IfaceCIS.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceSRuS.hxx"
#include "IfaceSProS.hxx"
#include "CellMLBootstrap.hpp"
#include "XPathBootstrap.hpp"
#include "CISBootstrap.hpp"
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <set>

class CDA_SRuSProcessor
  : public iface::SRuS::SEDMLProcessor
{
public:
  CDA_SRuSProcessor()
    : mRecursionDepth(0) {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::SEDMLProcessor);
  CDA_IMPL_REFCOUNT;

  bool supportsModellingLanguage(const std::wstring& aLang) throw();
  already_AddRefd<iface::SRuS::TransformedModel> buildOneModel(iface::SProS::Model* aModel)
    throw(std::exception&);
  already_AddRefd<iface::SRuS::TransformedModelSet> buildAllModels
    (iface::SProS::SEDMLElement* aElement) throw();
  void generateData(iface::SRuS::TransformedModelSet* aSet,
                    iface::SProS::SEDMLElement* aElement,
                    iface::SRuS::GeneratedDataMonitor* aMonitor) throw(std::exception&);

private:
  uint32_t mRecursionDepth;

  void doBasicTask(std::map<std::wstring, iface::SRuS::TransformedModel*>& modelsById,
                   std::multimap<std::wstring, std::wstring>& dataGeneratorIdsByTaskId,
                   std::map<std::wstring, iface::SProS::DataGenerator*>& dataGeneratorsById,
                   iface::cellml_services::CellMLIntegrationService* is,
                   iface::SProS::Task* t,
                   iface::SRuS::GeneratedDataMonitor* aMonitor);
  void doRepeatedTask(std::map<std::wstring, iface::SRuS::TransformedModel*>& modelsById,
                      std::multimap<std::wstring, std::wstring>& dataGeneratorIdsByTaskId,
                      std::map<std::wstring, iface::SProS::DataGenerator*>& dataGeneratorsById,
                      iface::cellml_services::CellMLIntegrationService* is,
                      iface::SProS::RepeatedTask* t,
                      iface::SRuS::GeneratedDataMonitor* aMonitor);
};

class CDA_SRuSTransformedModel
  : public iface::SRuS::TransformedModel
{
public:
  CDA_SRuSTransformedModel(iface::dom::Document* aDoc, iface::SProS::Model* aModel);

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::TransformedModel);
  CDA_IMPL_REFCOUNT;

  already_AddRefd<iface::dom::Document> xmlDocument() throw();
  already_AddRefd<iface::XPCOM::IObject> modelDocument() throw(std::exception&);
  already_AddRefd<iface::SProS::Model> sedmlModel() throw();

private:
  void ensureModelOrRaise();

  ObjRef<iface::dom::Document> mDocument;
  ObjRef<iface::cellml_api::Model> mModel;
  ObjRef<iface::SProS::Model> mSEDMLModel;
};

class CDA_SRuSTransformedModelSet
  : public iface::SRuS::TransformedModelSet
{
public:
  CDA_SRuSTransformedModelSet();

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::TransformedModelSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length() throw();
  already_AddRefd<iface::SRuS::TransformedModel> item(uint32_t aIdx)
    throw(std::exception&);
  already_AddRefd<iface::SRuS::TransformedModel> getItemByID(const std::wstring& aMatchID)
    throw(std::exception&);
  void addTransformed(iface::SRuS::TransformedModel* aTransformed);

private:
  std::vector<iface::SRuS::TransformedModel*> mTransformed;
  scoped_destroy<std::vector<iface::SRuS::TransformedModel*> > mTransformedRAII;
};

class CDA_SRuSGeneratedData
  : public iface::SRuS::GeneratedData
{
public:
  CDA_SRuSGeneratedData(iface::SProS::DataGenerator* aDG);
  
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::GeneratedData);
  CDA_IMPL_REFCOUNT;

  already_AddRefd<iface::SProS::DataGenerator> sedmlDataGenerator() throw();
  uint32_t length() throw();
  double dataPoint(uint32_t idex) throw(std::exception&);

  std::vector<double> mData;
  ObjRef<iface::SProS::DataGenerator> mDataGenerator;
};

class CDA_SRuSGeneratedDataSet
  : public iface::SRuS::GeneratedDataSet
{
public:
  CDA_SRuSGeneratedDataSet();
  
  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::GeneratedDataSet);
  CDA_IMPL_REFCOUNT;

  uint32_t length() throw();
  already_AddRefd<iface::SRuS::GeneratedData> item(uint32_t aIdx)
    throw(std::exception&);

  std::vector<iface::SRuS::GeneratedData*> mData;
private:
  scoped_destroy<std::vector<iface::SRuS::GeneratedData*> > mDataRAII;
};

/*
 * This class receives all the raw results emitted from simulations (but only
 * at 'time' points that the SED-ML has requested, no intermediate steps),
 * and applies the data generators to that raw data to get the final results.
 */
class CDA_SRuSRawResultProcessor
  : public iface::cellml_services::IntegrationProgressObserver
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI1(cellml_services::IntegrationProgressObserver);

  CDA_SRuSRawResultProcessor
  (
   iface::SRuS::GeneratedDataMonitor* aMonitor,
   iface::cellml_services::CodeInformation* aCodeInfo,
   const std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >&
     aVarInfoByDataGeneratorId,
   const std::map<std::wstring, iface::SProS::DataGenerator*>& aDataGeneratorsById
  );

  ~CDA_SRuSRawResultProcessor() {}

  void computedConstants(const std::vector<double>& aValues) throw();
  void done() throw(std::exception&);
  void failed(const std::string& aErrorMessage) throw(std::exception&);
  void results(const std::vector<double>& state) throw(std::exception&);

private:
  ObjRef<iface::SRuS::GeneratedDataMonitor> mMonitor;
  ObjRef<iface::cellml_services::CodeInformation> mCodeInfo;
  // 0: Unknown. 1: Need aggregate. -1: Don't need aggregate.
  int mAggregateMode;
  std::map<std::wstring, std::list<std::pair<std::wstring, int32_t> > >
    mVarInfoByDataGeneratorId;
  std::vector<double> mConstants;
  std::map<std::wstring, std::map<std::wstring, std::vector<double> > > mAggregateData;
  std::map<std::wstring, iface::SProS::DataGenerator*> mDataGeneratorsById;
  scoped_destroy<std::map<std::wstring, iface::SProS::DataGenerator*> > mDataGeneratorsByIdRAII;
  uint32_t mRecSize, mTotalN;
};

struct CDA_SRuSModelSimulationState
{
  std::vector<double> mInitialData, mCurrentData, mCurrentConstants;
  double mCurrentBvar, mInitialBvar;
  std::map<int, double> mOverrideConstants;
  std::map<int, double> mOverrideData;
  ObjRef<iface::cellml_services::CodeInformation> mCodeInfo;
};

/**
 * Stores information about the current state of a model that is shared between
 * steps and therefore updated as the simulation experiment proceeds.
 */
class CDA_SRuSSimulationState
  : public iface::XPCOM::IObject
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;
  CDA_IMPL_QI0;
  
  void resetToInitial();

  ObjRef<iface::SRuS::TransformedModelSet> mTMS;
  ObjRef<CDA_SRuSRawResultProcessor> mResultsTo;

  std::map<std::wstring, CDA_SRuSModelSimulationState>
    mPerModelState;
};

class SEDMLMathEvaluator;

/*
 * The abstract class for generating code for a step that has to be done for a
 * simulation experiment.
 */
class CDA_SRuSSimulationStep
  : public iface::XPCOM::IObject
{
public:
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_ID;

  CDA_SRuSSimulationStep(CDA_SRuSSimulationState* aState,
                         CDA_SRuSSimulationStep* aSuccessor);

  /*
   * Performs the simulation. The implementation calls performNext when it is
   * finished. May be called on any thread.
   */
  virtual void perform() = 0;

  /*
   * Makes a shallow clone of this simulation step.
   */
  virtual already_AddRefd<CDA_SRuSSimulationStep> shallowClone() = 0;

  /*
   * Makes another simulation step that is identical to this one except that a
   * deep clone is made of each part of the simulation and the argument is
   * put in place of the last successor in the chain.
   */
  already_AddRefd<CDA_SRuSSimulationStep> cloneChangingLastSuccessor(CDA_SRuSSimulationStep* aNewEnd);

  void performNext();

  ObjRef<CDA_SRuSSimulationState> mState;
  ObjRef<CDA_SRuSSimulationStep> mSuccessor;

protected:
  int
  getIndexAndTypeAndModelForVariable
  (
   iface::SProS::Variable* aVariable,
   iface::cellml_services::VariableEvaluationType& aType,
   CDA_SRuSModelSimulationState** aState
  );
  double& findVariable
  (
   iface::SProS::Variable* aVariable
  );
};

class CDA_SRuSSimulationStepLoop
  : public CDA_SRuSSimulationStep
{
public:
  CDA_IMPL_QI0;

  CDA_SRuSSimulationStepLoop(CDA_SRuSSimulationState* aState,
                             CDA_SRuSSimulationStep* aSuccessor,
                             const std::wstring& aMainRange,
                             std::list<iface::SProS::Range*>& aRanges,
                             iface::SProS::SetValueSet* aSetValues,
                             CDA_SRuSSimulationStep* aLoopChain);
  ~CDA_SRuSSimulationStepLoop() {};

  void perform();

  already_AddRefd<CDA_SRuSSimulationStep> shallowClone();

private:
  CDA_SRuSSimulationStepLoop(CDA_SRuSSimulationState* aState,
                             CDA_SRuSSimulationStep* aSuccessor,
                             int aNumPoints, int aCurrentIndex,
                             std::list<iface::SProS::Range*>& aRanges,
                             iface::SProS::SetValueSet* aSetValues,
                             CDA_SRuSSimulationStep* aLoopChain);

  int mNumPoints, mCurrentIndex;
  std::list<iface::SProS::Range*> mRanges;
  scoped_destroy<std::list<iface::SProS::Range*> > mRangesRAII;
  ObjRef<iface::SProS::SetValueSet> mSetValues;
  ObjRef<CDA_SRuSSimulationStep> mLoopChain;

  double getRangeValueFor
  (
   iface::SProS::Range* aRange,
   std::map<std::wstring, double>& aCurrentRangeValues
  );
};

class CDA_SRuSSimulationStepDropUntil
  : public CDA_SRuSSimulationStep
{
public:
  CDA_IMPL_QI0;

  CDA_SRuSSimulationStepDropUntil
  (
   std::wstring aModelId,
   double aTargetBvar,
   CDA_SRuSSimulationState* aState,
   CDA_SRuSSimulationStep* aSuccessor);
  void perform();
  already_AddRefd<CDA_SRuSSimulationStep> shallowClone();

private:
  std::wstring mModelId;
  double mTargetBvar;
};

class CDA_SRuSBootstrap
  : public iface::SRuS::Bootstrap
{
public:
  CDA_SRuSBootstrap() {}

  CDA_IMPL_ID;
  CDA_IMPL_QI1(SRuS::Bootstrap);
  CDA_IMPL_REFCOUNT;

  already_AddRefd<iface::SRuS::SEDMLProcessor> makeDefaultProcessor()
    throw();
};

class NeedsAggregate
{
};

// An evaluator for MathML expressions making use of the SED-ML subset.
class SEDMLMathEvaluator
{
public:
  SEDMLMathEvaluator();
  void setExploreEverything(bool aExploreEverything);
  double eval(iface::mathml_dom::MathMLElement* aME);
  double evalConstant(iface::mathml_dom::MathMLCnElement* mcne);
  double evalApply(iface::mathml_dom::MathMLApplyElement* mae);
  double evalVariable(iface::mathml_dom::MathMLCiElement* mcie);
  double evalPiecewise(iface::mathml_dom::MathMLPiecewiseElement* mpe);
  double evalPredefined(iface::mathml_dom::MathMLPredefinedSymbol* mpds);
  virtual double evalAggregate(iface::mathml_dom::MathMLCsymbolElement* aOp, iface::mathml_dom::MathMLApplyElement* aApply);
  void setVariable(const std::wstring& aName, double aValue);

protected:
  std::map<std::wstring, double> mVariableValues;
  bool mExploreEverything;
};

class SEDMLMathEvaluatorWithAggregate
  : public SEDMLMathEvaluator
{
public:
  SEDMLMathEvaluatorWithAggregate(const std::map<std::wstring, std::vector<double> >& aHistory);
  double evalAggregate(iface::mathml_dom::MathMLCsymbolElement* aOp,
                       iface::mathml_dom::MathMLApplyElement* aApply);

private:
  std::map<std::wstring, std::vector<double> > mHistory;
};
