#ifndef _CCGSImplementation_hpp
#define _CCGSImplementation_hpp
#include "cda_compiler_support.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <exception>
#include "IfaceCellML_APISPEC.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "IfaceCCGS.hxx"
#include "Utilities.hxx"
#include <sstream>
#include <vector>
#include <list>
#include <memory>
#include <set>

// Disabled for now because modules aren't used by anyone and breaks MingW builds.
// We can either fix the build system or remove it altogether later.
#undef ENABLE_CONTEXT

#ifdef ENABLE_CONTEXT
#include "IfaceCellML_Context.hxx"
#endif

class CDA_CodeInformation;

class CDA_ComputationTarget
  : public iface::cellml_services::ComputationTarget
{
  ptr_tag<CDA_ComputationTarget> self;
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::ComputationTarget);

  CDA_ComputationTarget() : self(this), mDegree(0), mAssignedIndex(0),
                            mIsReset(false),
                            mEvaluationType(iface::cellml_services::FLOATING),
                            mInfDelayedAssignedIndex(-1)
  {};
  ~CDA_ComputationTarget() {};

  already_AddRefd<iface::cellml_api::CellMLVariable> variable() throw();
  uint32_t degree() throw();
  iface::cellml_services::VariableEvaluationType type() throw();
  std::wstring name() throw();
  uint32_t assignedIndex() throw();

  ptr_tag<CDA_ComputationTarget>& getSelf() { return self; }

  // CCGS implementation access only...
  void setNameAndIndex(uint32_t aIndex, const std::wstring& aName) throw();
  void setDelayedName(const std::wstring& aSetTo) throw();
  ObjRef<iface::cellml_api::CellMLVariable> mVariable;
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;
  uint32_t mDegree, mAssignedIndex;
  bool mIsReset;
  iface::cellml_services::VariableEvaluationType mEvaluationType;
  // Only available during code generation...
  ptr_tag<CDA_ComputationTarget> mUpDegree, mDownDegree;
  uint32_t mHighestDegree;
  // -1 if there is no index for the infinitesimally delayed version.
  int32_t mInfDelayedAssignedIndex;
  bool mStateHasIV;

  // Disjoint set utilities...
  uint32_t rank;
  ptr_tag<CDA_ComputationTarget> parent;
  void resetSetMembership()
  {
    rank = 0;
    parent = self;
  }

  ptr_tag<CDA_ComputationTarget> findRoot()
  {
    if (parent == this)
      return self;

    parent = parent->findRoot();
    return parent;
  }

  void unionWith(ptr_tag<CDA_ComputationTarget> b)
  {
    findRoot();
    b->findRoot();

    if (parent == b->parent)
      return;

    if (parent->rank > b->parent->rank)
      b->parent->parent = parent;
    else if (b->parent->rank > parent->rank)
      parent->parent = b->parent;
    else
    {
      b->parent->parent = parent;
      parent->rank++;
    }
  }
};

class CDA_TargetSet
  : public virtual iface::XPCOM::IObject
{
public:
  ~CDA_TargetSet();
  std::list<ptr_tag<CDA_ComputationTarget> > mTargets;
};

class CDA_ComputationTargetIterator
  : public iface::cellml_services::ComputationTargetIterator
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::ComputationTargetIterator);

  CDA_ComputationTargetIterator
  (
   CDA_TargetSet* aOwner
  ) : mTargets(aOwner->mTargets),
      mOwner(aOwner) {
    mTargetsIt = mTargets.begin();
  };
  ~CDA_ComputationTargetIterator() {};

  already_AddRefd<iface::cellml_services::ComputationTarget> nextComputationTarget() throw();

private:
  // These will not go away before mOwner, so we don't ref them.
  std::list<ptr_tag<CDA_ComputationTarget> >& mTargets;
  std::list<ptr_tag<CDA_ComputationTarget> >::iterator mTargetsIt;
  ObjRef<CDA_TargetSet> mOwner;
};

class CDA_CodeInformation
  : public iface::cellml_services::IDACodeInformation, public CDA_TargetSet
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(cellml_services::CodeInformation, cellml_services::IDACodeInformation);

  CDA_CodeInformation() {};
  ~CDA_CodeInformation();

  std::wstring errorMessage() throw();
  iface::cellml_services::ModelConstraintLevel constraintLevel() throw();
  uint32_t algebraicIndexCount() throw();
  uint32_t rateIndexCount() throw();
  uint32_t constantIndexCount() throw();
  std::wstring initConstsString() throw();
  std::wstring ratesString() throw();
  std::wstring variablesString() throw();
  std::wstring functionsString() throw();
  std::wstring essentialVariablesString() throw();
  std::wstring stateInformationString() throw();
  uint32_t conditionVariableCount() throw();
  std::wstring rootInformationString() throw();
  already_AddRefd<iface::cellml_services::ComputationTargetIterator> iterateTargets()
    throw();
  already_AddRefd<iface::mathml_dom::MathMLNodeList> flaggedEquations()
    throw();

  // CCGS implementation access only...
  std::wstring mErrorMessage;
  iface::cellml_services::ModelConstraintLevel mConstraintLevel;
  uint32_t mAlgebraicIndexCount, mRateIndexCount, mConstantIndexCount, mConditionVariableCount;
  std::wstring mInitConstsStr, mRatesStr, mVarsStr, mFuncsStr, mEssentialVarsStr, mStateInformationStr,
               mRootInformationStr;
  std::vector<iface::dom::Element*> mFlaggedEquations;
};

class CDA_CustomCodeInformation
  : public iface::cellml_services::CustomCodeInformation, public CDA_TargetSet
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CustomCodeInformation);

  CDA_CustomCodeInformation();

  iface::cellml_services::ModelConstraintLevel constraintLevel() throw();
  uint32_t indexCount() throw();
  already_AddRefd<iface::cellml_services::ComputationTargetIterator> iterateTargets() throw();
  std::wstring generatedCode() throw();
  std::wstring functionsString() throw();

  iface::cellml_services::ModelConstraintLevel mConstraintLevel;
  uint32_t mIndexCount;
  std::wstring mGeneratedCode;
  std::wstring mFunctionsString;
};

class CDA_CustomGenerator
  : public iface::cellml_services::CustomGenerator, public CDA_TargetSet
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI1(cellml_services::CustomGenerator);
  
  CDA_CustomGenerator(
                      iface::cellml_api::Model* aModel,
                      iface::cellml_services::MaLaESTransform* aTransform,
                      iface::cellml_services::CeVAS* aCeVAS,
                      iface::cellml_services::CUSES* aCUSES,
                      iface::cellml_services::AnnotationSet* aAnnoSet,
                      std::wstring& aStateVariableNamePattern,
                      std::wstring& aAssignPattern,
                      std::wstring& aSolvePattern,
                      std::wstring& aSolveNLSystemPattern,
                      uint32_t aArrayOffset
                     );

  already_AddRefd<iface::cellml_services::ComputationTargetIterator> iterateTargets() throw();
  void requestComputation(iface::cellml_services::ComputationTarget* aTarget) throw(std::exception&);
  void markAsKnown(iface::cellml_services::ComputationTarget* aTarget) throw(std::exception&);
  void markAsUnwanted(iface::cellml_services::ComputationTarget* aTarget) throw(std::exception&);
  already_AddRefd<iface::cellml_services::CustomCodeInformation> generateCode() throw(std::exception&);

  void indexTargets();

private:
  std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator> mTargetSet, mRequestComputation,
    mKnown, mUnwanted;
  ObjRef<iface::cellml_api::Model> mModel;
  ObjRef<iface::cellml_services::MaLaESTransform> mTransform;
  ObjRef<iface::cellml_services::CeVAS> mCeVAS;
  ObjRef<iface::cellml_services::CUSES> mCUSES;
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;
  std::wstring mStateVariableNamePattern, mAssignPattern, mSolvePattern,
               mSolveNLSystemPattern;
  uint32_t mArrayOffset;
};

class CodeGenerationState;

class CDA_CodeGenerator
  : public iface::cellml_services::IDACodeGenerator
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI2(cellml_services::CodeGenerator, cellml_services::IDACodeGenerator);

  CDA_CodeGenerator(bool aIDAStyle);
  ~CDA_CodeGenerator() {};

  std::wstring constantPattern() throw();
  void constantPattern(const std::wstring& aPattern) throw();
  std::wstring stateVariableNamePattern() throw();
  void stateVariableNamePattern(const std::wstring& aPattern) throw();
  std::wstring algebraicVariableNamePattern() throw();
  void algebraicVariableNamePattern(const std::wstring& aPattern) throw();
  std::wstring rateNamePattern() throw();
  void rateNamePattern(const std::wstring& aPattern) throw();
  std::wstring voiPattern(void) throw();
  void voiPattern(const std::wstring& aPattern) throw();
  std::wstring sampleDensityFunctionPattern(void) throw();
  void sampleDensityFunctionPattern(const std::wstring& aPattern) throw();
  std::wstring sampleRealisationsPattern(void) throw();
  void sampleRealisationsPattern(const std::wstring& aPattern) throw();
  std::wstring boundVariableName(void) throw();
  void boundVariableName(const std::wstring& aPattern) throw();
  uint32_t arrayOffset() throw();
  void arrayOffset(uint32_t offset) throw();
  std::wstring assignPattern() throw();
  void assignPattern(const std::wstring& aPattern) throw();
  std::wstring solvePattern() throw();
  void solvePattern(const std::wstring& aPattern) throw();
  std::wstring solveNLSystemPattern() throw();
  void solveNLSystemPattern(const std::wstring& aPattern) throw();
  std::wstring temporaryVariablePattern() throw();
  void temporaryVariablePattern(const std::wstring& aPattern) throw();
  std::wstring declareTemporaryPattern() throw();
  void declareTemporaryPattern(const std::wstring& aPattern) throw();
  std::wstring conditionalAssignmentPattern() throw();
  void conditionalAssignmentPattern(const std::wstring& aPattern) throw();
  std::wstring residualPattern() throw();
  void residualPattern(const std::wstring& aPattern) throw();
  std::wstring constrainedRateStateInfoPattern() throw();
  void constrainedRateStateInfoPattern(const std::wstring& aPattern) throw();
  std::wstring unconstrainedRateStateInfoPattern() throw();
  void unconstrainedRateStateInfoPattern(const std::wstring& aPattern) throw();
  std::wstring infDelayedRatePattern() throw();
  void infDelayedRatePattern(const std::wstring& aPattern) throw();
  std::wstring infDelayedStatePattern() throw();
  void infDelayedStatePattern(const std::wstring& aPattern) throw();
  std::wstring conditionVariablePattern() throw();
  void conditionVariablePattern(const std::wstring& aPattern) throw();
  bool trackPiecewiseConditions() throw();
  void trackPiecewiseConditions(bool aTrack) throw();

  already_AddRefd<iface::cellml_services::MaLaESTransform> transform() throw();
  void transform(iface::cellml_services::MaLaESTransform* aTransform)
     throw();
  already_AddRefd<iface::cellml_services::CeVAS> useCeVAS()
     throw();
  void useCeVAS(iface::cellml_services::CeVAS* aCeVAS)
     throw();
  already_AddRefd<iface::cellml_services::CUSES> useCUSES() throw();
  void useCUSES(iface::cellml_services::CUSES* aCUSES) throw();
  already_AddRefd<iface::cellml_services::AnnotationSet> useAnnoSet() throw();
  void useAnnoSet(iface::cellml_services::AnnotationSet* aAnnoSet) throw();

  already_AddRefd<iface::cellml_services::CodeInformation> generateCode
    (iface::cellml_api::Model* aSourceModel) throw()
  {
    return generateIDACode(aSourceModel);
  }

  already_AddRefd<iface::cellml_services::CustomGenerator> createCustomGenerator(iface::cellml_api::Model* aSourceModel)
    throw(std::exception&);

  already_AddRefd<iface::cellml_services::IDACodeInformation> generateIDACode
    (iface::cellml_api::Model* aSourceModel) throw();

  bool allowPassthrough() throw();
  void allowPassthrough(bool aPT) throw();

private:
  std::wstring mConstantPattern, mStateVariableNamePattern,
    mAlgebraicVariableNamePattern, mRateNamePattern, mVOIPattern,
    mSampleDensityFunctionPattern, mSampleRealisationsPattern, mBoundVariableName,
    mAssignPattern, mSolvePattern, mSolveNLSystemPattern,
    mTemporaryVariablePattern, mDeclareTemporaryPattern,
    mConditionalAssignmentPattern, mResidualPattern, mConstrainedRateStateInfoPattern,
    mUnconstrainedRateStateInfoPattern, mInfDelayedRatePattern, mInfDelayedStatePattern,
    mConditionVariablePattern;
  bool mTrackPiecewiseConditions, mAllowPassthrough;
  uint32_t mArrayOffset;
  bool mIDAStyle;
  ObjRef<iface::cellml_services::MaLaESTransform> mTransform;
  ObjRef<iface::cellml_services::CeVAS> mCeVAS;
  ObjRef<iface::cellml_services::CUSES> mCUSES;
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;
  std::auto_ptr<CodeGenerationState> makeCodeGenerationState(int aCompat, iface::cellml_api::Model* aSourceModel);
};

class CDA_CodeGeneratorBootstrap
  : public iface::cellml_services::CodeGeneratorBootstrap
#ifdef ENABLE_CONTEXT 
  , public iface::cellml_context::CellMLModule
#endif 
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
#ifdef ENABLE_CONTEXT
  CDA_IMPL_QI2(cellml_services::CodeGeneratorBootstrap, cellml_context::CellMLModule);
#else
  CDA_IMPL_QI1(cellml_services::CodeGeneratorBootstrap);
#endif

  CDA_CodeGeneratorBootstrap()
#ifdef ENABLE_CONTEXT
    : mUnload(NULL)
#endif
  {}
  ~CDA_CodeGeneratorBootstrap() {}

  already_AddRefd<iface::cellml_services::CodeGenerator>
  createCodeGenerator() throw(std::exception&)
  {
    return new CDA_CodeGenerator(false);
  }

  already_AddRefd<iface::cellml_services::IDACodeGenerator>
  createIDACodeGenerator() throw(std::exception&)
  {
    return new CDA_CodeGenerator(true);
  }

#ifdef ENABLE_CONTEXT
  iface::cellml_context::CellMLModule::ModuleTypes moduleType()
    throw(std::exception&)
  {
    return iface::cellml_context::CellMLModule::SERVICE;
  }

  std::wstring moduleName() throw (std::exception&)
  {
    return L"CCCGS";
  }

  std::wstring moduleDescription() throw (std::exception&)
  {
    return L"The CellML C Code Generation Service";
  }

  std::wstring moduleVersion() throw (std::exception&)
  {
    return L"0.0";
  }

  std::wstring moduleIconURL() throw (std::exception&)
  {
    return L"";
  }

  void unload() throw (std::exception&)
  {
    if (mUnload != NULL)
      mUnload();
  }

  void SetUnloadCCGS(void (*unload)(void))
  {
    mUnload = unload;
  }

private:
  void (*mUnload)();
#endif
};

#endif // _CCGSImplementation_hpp
