#ifndef _CodeGenerationState_hxx
#define _CodeGenerationState_hxx

#include "IfaceCCGS.hxx"
#include <vector>
#include <set>
#include <map>
#include "IfaceCellML_APISPEC.hxx"

class MathStatement
{
public:
  typedef enum
    {
      INITIAL_ASSIGNMENT,
      EQUATION,
      INEQUALITY,
      PIECEWISE,
      SAMPLE_FROM_DIST,
      UNCLASSIFIED_MATHML,
    } StatementType;

  MathStatement(StatementType aType) : mInvolvesDelays(false), mType(aType) {}
  virtual ~MathStatement() {}

  virtual uint32_t degFreedom() { return 1; }

  ObjRef<iface::cellml_api::CellMLComponent> mContext;
  std::list<ptr_tag<CDA_ComputationTarget> > mTargets, mDelayedTargets;
  bool mInvolvesDelays;

  StatementType mType;
  // Temporary annotations used in code generation...
  std::wstring mCode, mVarName;
};

// This is a special variable edge which is created when an initial_value="name"
// construct is found. It can only be used to compute the first target from the
// second (must have exactly two targets).
class InitialAssignment
  : public MathStatement
{
public:
  InitialAssignment() : MathStatement(MathStatement::INITIAL_ASSIGNMENT) {}
};

class MathMLMathStatement : public MathStatement
{
public:
  MathMLMathStatement(MathStatement::StatementType st) : MathStatement(st) {}
  ObjRef<iface::mathml_dom::MathMLElement> mMaths;
};

class Equation : public MathMLMathStatement
{
public:
  Equation() : MathMLMathStatement(MathStatement::EQUATION) {}

  // The left-hand side. NULL if mRHS has to be minimised.
  ObjRef<iface::mathml_dom::MathMLElement> mLHS;
  // The right-hand side.
  ObjRef<iface::mathml_dom::MathMLElement> mRHS;
};

class Inequality : public MathMLMathStatement
{
public:
  Inequality() : MathMLMathStatement(MathStatement::INEQUALITY) {}
};

class Piecewise : public MathMLMathStatement
{
public:
  Piecewise() : MathMLMathStatement(MathStatement::PIECEWISE) {}

  ~Piecewise()
  {
    std::list<std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> > >::iterator i;
    for (i = mPieces.begin(); i != mPieces.end(); i++)
    {
      delete (*i).first;
      delete (*i).second;
    }
  }

  std::list<std::pair<ptr_tag<Equation>, ptr_tag<MathMLMathStatement> > > mPieces;
};

class SampleFromDistribution : public MathMLMathStatement
{
public:
  SampleFromDistribution() : MathMLMathStatement(MathStatement::SAMPLE_FROM_DIST) {}
  ~SampleFromDistribution() {}

  std::set<ptr_tag<CDA_ComputationTarget> > mOutSet;
  std::vector<ptr_tag<CDA_ComputationTarget> > mOutTargets;
  ObjRef<iface::mathml_dom::MathMLElement> mDistrib;

  virtual uint32_t degFreedom() { return mOutTargets.size(); }
};

/*
 * A system, which is built up of n >=1 equations, and computes n unknowns from
 * an arbitrary number of knowns.
 */
struct System
{
public:
  System(std::set<ptr_tag<MathStatement> >& aMathStatements,
         std::set<ptr_tag<CDA_ComputationTarget> >& aKnowns,
         std::set<ptr_tag<CDA_ComputationTarget> >& aUnknowns)
    : mMathStatements(aMathStatements), mKnowns(aKnowns), mUnknowns(aUnknowns)
  {
  }
  ~System() {}

  std::set<ptr_tag<MathStatement> > mMathStatements;
  std::set<ptr_tag<CDA_ComputationTarget> > mKnowns;
  // The targets which are actually computed (determined by connectivity
  // analysis).
  std::set<ptr_tag<CDA_ComputationTarget> > mUnknowns;
};

class AssignmentOnlyRequestedNeedSolve
  : public std::exception
{
public:
  AssignmentOnlyRequestedNeedSolve() {}
  const char* why() { return "assignmentOnly requested, but solve is required."; }
};

class CodeGenerationState
{
public:
  CodeGenerationState(int aCompatLevel,
                      iface::cellml_api::Model* aModel,
                      std::wstring& aConstantPattern,
                      std::wstring& aStateVariableNamePattern,
                      std::wstring& aAlgebraicVariableNamePattern,
                      std::wstring& aRateNamePattern,
                      std::wstring& aVOIPattern,
                      std::wstring& aSampleDensityFunctionPattern,
                      std::wstring& aSampleRealisationsPattern,
                      std::wstring& aBoundVariableName,
                      std::wstring& aAssignPattern,
                      std::wstring& aAssignConstantPattern,
                      std::wstring& aSolvePattern,
                      std::wstring& aSolveNLSystemPattern,
                      std::wstring& aTemporaryVariablePattern,
                      std::wstring& aDeclareTemporaryPattern,
                      std::wstring& aConditionalAssignmentPattern,
                      std::wstring& aResidualPattern,
                      std::wstring& aConstrainedRateStateInfoPattern,
                      std::wstring& aUnconstrainedRateStateInfoPattern,
                      std::wstring& aInfDelayedRatePattern,
                      std::wstring& aInfDelayedStatePattern,
                      std::wstring& aConditionVariablePattern,
                      bool aTrackPiecewiseConditions,
                      uint32_t aArrayOffset,
                      iface::cellml_services::MaLaESTransform* aTransform,
                      iface::cellml_services::CeVAS* aCeVAS,
                      iface::cellml_services::CUSES* aCUSES,
                      iface::cellml_services::AnnotationSet* aAnnoSet,
                      bool aIDAStyle)
    : mCompatLevel(aCompatLevel),
      mModel(aModel), mConstantPattern(aConstantPattern),
      mStateVariableNamePattern(aStateVariableNamePattern),
      mAlgebraicVariableNamePattern(aAlgebraicVariableNamePattern),
      mRateNamePattern(aRateNamePattern),
      mVOIPattern(aVOIPattern),
      mSampleDensityFunctionPattern(aSampleDensityFunctionPattern),
      mSampleRealisationsPattern(aSampleRealisationsPattern),
      mBoundVariableName(aBoundVariableName),
      mAssignPattern(aAssignPattern),
      mAssignConstantPattern(aAssignConstantPattern),
      mSolvePattern(aSolvePattern),
      mSolveNLSystemPattern(aSolveNLSystemPattern),
      mTemporaryVariablePattern(aTemporaryVariablePattern),
      mDeclareTemporaryPattern(aDeclareTemporaryPattern),
      mConditionalAssignmentPattern(aConditionalAssignmentPattern),
      mResidualPattern(aResidualPattern),
      mConstrainedRateStateInfoPattern(aConstrainedRateStateInfoPattern),
      mUnconstrainedRateStateInfoPattern(aUnconstrainedRateStateInfoPattern),
      mInfDelayedRatePattern(aInfDelayedRatePattern),
      mInfDelayedStatePattern(aInfDelayedStatePattern),
      mConditionVariablePattern(aConditionVariablePattern),
      mTrackPiecewiseConditions(aTrackPiecewiseConditions),
      mArrayOffset(aArrayOffset),
      mTransform(aTransform),
      mCeVAS(aCeVAS),
      mCUSES(aCUSES),
      mAnnoSet(aAnnoSet),
      mNextConstantIndex(aArrayOffset),
      mNextStateVariableIndex(aArrayOffset),
      mNextAlgebraicVariableIndex(aArrayOffset),
      mNextVOI(aArrayOffset),
      mNextConditionVariable(aArrayOffset),
      mNextSolveId(0),
      mIDAStyle(aIDAStyle),
      mIsConstant(false),
      mDryRun(false)
  {
  }

  ~CodeGenerationState();

  already_AddRefd<iface::cellml_services::IDACodeInformation> GenerateCode();
  void IDAStyleCodeGeneration();
  void ODESolverStyleCodeGeneration();

  void ResetGenerator();

  already_AddRefd<iface::cellml_services::CustomGenerator> CreateCustomGenerator();
  already_AddRefd<iface::cellml_services::CustomCodeInformation> GenerateCustomCode
  (std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aTargetSet,
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aUserWanted,
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aUserKnown,
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aUserUnwanted);

  void CreateBaseComputationTargets();
  ptr_tag<CDA_ComputationTarget>  GetTargetOfDegree(ptr_tag<CDA_ComputationTarget>  aBase,
                                           uint32_t aDegree);
  void CreateMathStatements();
  void CheckStateVariableIVConstraints(const std::list<System*>& aSystems);
  void MakeSystemsForResetRulesAndClearKnown
    (
     std::set<std::pair<ptr_tag<CDA_ComputationTarget>, ptr_tag<MathStatement> > >&
       aResets,
     std::list<System*>& aSystems,
     std::set<ptr_tag<CDA_ComputationTarget> >& aKnown,
     std::set<ptr_tag<CDA_ComputationTarget> >& aFloating
    );
  void MapExternalTargetsToInternal
  (
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExTargetSet,
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExUserWanted,
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExUserKnown,
   std::set<iface::cellml_services::ComputationTarget*, XPCOMComparator>& aExUserUnwanted,
   std::set<ptr_tag<CDA_ComputationTarget> >& aInWanted,
   std::set<ptr_tag<CDA_ComputationTarget> >& aInKnown,
   std::set<ptr_tag<CDA_ComputationTarget> >& aInUnwanted
  );
  void SplitPiecewiseByResetRule();
  void ContextError(const std::wstring& details,
                    iface::mathml_dom::MathMLElement* context1,
                    iface::cellml_api::CellMLElement* context2);
  void FirstPassTargetClassification();
  void ClassifyAndBuildFloatingForCustom(
         std::set<ptr_tag<CDA_ComputationTarget> >& wanted,
         std::set<ptr_tag<CDA_ComputationTarget> >& known,
         std::set<ptr_tag<CDA_ComputationTarget> >& unwanted
                                        );
  bool FindSystemsNeededForTargets(
         const std::map<ptr_tag<CDA_ComputationTarget>, System*>&
           aSysByTargReq,
         const std::set<ptr_tag<CDA_ComputationTarget> >& aWantedTargets,
         bool aMarkOnly,
         std::set<ptr_tag<CDA_ComputationTarget> >& aKnownTargets,
         std::list<System*>& aNeededSystems
                                  );

  void AllocateVariable(ptr_tag<CDA_ComputationTarget>  aCT, std::wstring& aStr,
                        std::wstring& aPattern, uint32_t& aNextIndex,
                        uint32_t& aCount);
  void AllocateDelayed(ptr_tag<CDA_ComputationTarget>  aCT,
                        std::wstring& aPattern, uint32_t aNextIndex);
  void GenerateVariableName(std::wstring& aStr,
                            std::wstring& aPattern, uint32_t index);
  void AllocateVariablesInSet(std::list<System*>& aSet,
                              iface::cellml_services::VariableEvaluationType aET,
                              std::wstring& aPattern,
                              uint32_t& aNextIndex,
                              uint32_t& aCountVar);
  void AllocateConstant(ptr_tag<CDA_ComputationTarget> aCT, std::wstring& aStr);
  void AllocateStateVariable(ptr_tag<CDA_ComputationTarget> aCT, std::wstring& aStr);
  void AllocateAlgebraicVariable(ptr_tag<CDA_ComputationTarget> aCT, std::wstring& aStr);
  void AllocateVOI(ptr_tag<CDA_ComputationTarget>  aCT, std::wstring& aStr);
  void CloneNamesIntoDelayedNames();
  void AppendAssign(std::wstring& aAppendTo,
                    const std::wstring& aLHS,
                    const std::wstring& aRHS,
                    const std::wstring& aXMLId);
  void AppendConstantAssign(std::wstring& aAppendTo,
                            const std::wstring& aLHS,
                            const std::wstring& aRHS,
                            const std::wstring& aXMLId);
  void BuildFloatingAndConstantLists();
  void BuildFloatingAndKnownLists(bool includeRates = true);
  void WriteForcedInitialVariables();
  void BuildStateAndConstantLists();
  bool DecomposeIntoSystems(std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
                            std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
                            std::set<ptr_tag<CDA_ComputationTarget> >& aUnwanted,
                            std::list<System*>& aSystems,
                            bool aIgnoreInfdelayed = false);
  void DecomposeIntoAssignments(std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
                                std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
                                std::set<ptr_tag<CDA_ComputationTarget> >& aUnwanted,
                                std::list<System*>& aSystems);
  bool FindSmallSystem(
                       std::set<ptr_tag<MathStatement> >& aUseEquations,
                       std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
                       std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
                       std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
                       std::list<System*>& aSystems
                      );

  bool FindBigSystem(
                     std::set<ptr_tag<MathStatement> >& aUseMathStatements,
                     std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
                     std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
                     std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
                     std::list<System*>& aSystems
                    );

  bool RecursivelyTestSmallSystem(
                                  std::set<ptr_tag<MathStatement> >& aSystem,
                                  std::set<ptr_tag<MathStatement> >::iterator& aEqIt,
                                  uint32_t aNeedToAdd,
                                  std::set<ptr_tag<MathStatement> >& aUseMathStatements,
                                  std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
                                  std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
                                  std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates,
                                  std::list<System*>& aSystems
                                 );

  bool RecursivelyTestBigSystem(
                                std::set<ptr_tag<MathStatement> >& aNonSystem,
                                std::set<ptr_tag<MathStatement> >::iterator& aEqIt,
                                uint32_t aNeedToRemove,
                                std::set<ptr_tag<MathStatement> >& aUseMathStatements,
                                std::set<ptr_tag<CDA_ComputationTarget> >& aUseVars,
                                std::set<ptr_tag<CDA_ComputationTarget> >& aStart,
                                std::set<ptr_tag<CDA_ComputationTarget> >& aCandidates
                               );

  void BuildSystemsByTargetsRequired(std::list<System*>& aSystems,
                                     std::map<ptr_tag<CDA_ComputationTarget>, System*>&
                                     aSysByTargReq);

  void GenerateCodeForSet(std::wstring& aCodeTo,
                          std::set<ptr_tag<CDA_ComputationTarget> >& aKnown,
                          std::list<System*>& aTargets,
                          std::map<ptr_tag<CDA_ComputationTarget>, System*>&
                            aSysByTargReq
                         );
  void GenerateCodeForSetByType
  (
   std::set<ptr_tag<CDA_ComputationTarget> >& aKnown,
   std::list<System*>& aSystems,
   std::map<ptr_tag<CDA_ComputationTarget>, System*>&
     aSysByTargReq
  );
  void GenerateStateToRateCascades();
  void GenerateInfDelayUpdates();
  void GenerateCasesIntoTemplate(std::wstring& aCodeTo,
                                 std::list<std::pair<std::wstring, std::wstring> >& aCases);
  void GenerateCodeForSystem(std::wstring& aCodeTo, System* aSys);
  iface::cellml_api::CellMLVariable* GetVariableInComponent
  (
   iface::cellml_api::CellMLComponent* aComp,
   iface::cellml_api::CellMLVariable* aVar
  );

  void GenerateCodeForEquation(std::wstring& aCodeTo, Equation* aEq, ptr_tag<CDA_ComputationTarget> aComputedTarget,
                               bool aAssignmentOnly = false);
  void GenerateCodeForSampleFromDist(std::wstring& aCodeTo, SampleFromDistribution* aSFD);

  void GenerateAssignmentMaLaESResult
  (
   std::wstring& aCodeTo,
   ptr_tag<CDA_ComputationTarget> aTarget,
   iface::cellml_services::MaLaESResult* aMR,
   const std::wstring& aXMLId
  );
  void GenerateSolveCode
  (
   std::wstring& aCodeTo,
   Equation* aEq,
   ptr_tag<CDA_ComputationTarget> aComputedTarget
  );
  void GenerateMultivariateSolveCode
  (
   std::wstring& aCodeTo,
   System* aSys
  );
  void GenerateMultivariateSolveCodeTo
  (
   std::wstring& aCodeTo,
   System* aSys,
   const std::wstring& aPattern,
   const wchar_t* aId
  );
  void GenerateMultivariateSolveCodeEq
  (
   std::wstring& aCodeTo,
   ptr_tag<MathStatement> aMS,
   const std::wstring& aPattern,
   const wchar_t* aId,
   const wchar_t* aIndex,
   const wchar_t* aIV
  );

  std::wstring ReplaceIDs
  (
   std::wstring aReplaceFrom,
   const wchar_t* aId,
   const wchar_t* aIndex,
   const wchar_t* aCount
  );
  
  void GenerateStateInformation(std::wstring& aStr);
  void AllocateRateNamesAsConstants(std::list<System*>& aSystems);
  void RestoreSavedRates(std::wstring& aCode);
  void ProcessModellerSuppliedIVHints();
  double GetPseudoStateIV(ptr_tag<CDA_ComputationTarget>);
  void InitialisePseudoStates(std::wstring& aCode);
  void FindSystemsForResiduals(std::list<System*>& aSystems,
                               std::list<System*>& aSysForResid);
  void ComputeInfDelayedName(ptr_tag<CDA_ComputationTarget> aCT, std::wstring& aStr);
  void SetupMathMLMathStatement(MathMLMathStatement* mms, iface::mathml_dom::MathMLElement* mn,
                                iface::cellml_api::CellMLComponent* c);
  void MarkRemainingVariablesAsPseudoState();
  void GenerateResiduals(std::wstring& aCode);
  void GenerateResidualForEquation(std::wstring& aCode, uint32_t aResidNo, Equation* aEq);
  void GenerateResidualForString(std::wstring& aCode, uint32_t aResidNo,
                                 const std::wstring& e1, const std::wstring& e2, const std::wstring& aXmlId);
  void TransformPiecewiseConditions();
  void TransformPiecewiseStatement(MathStatement* aStatement);
  void TransformPiecewisesInMaths(iface::mathml_dom::MathMLElement* aChange,
                                  iface::cellml_api::CellMLComponent* aContext);
  void TransformCaseCondition(iface::mathml_dom::MathMLElement* aEl,
                              iface::cellml_api::CellMLComponent* aContext);
  void GenerateRootInformation();
  void CheckInappropriateStateAssignments(std::list<System*>& aSystems);

  int mCompatLevel;

  ObjRef<iface::cellml_api::Model> mModel;
  std::wstring & mConstantPattern, & mStateVariableNamePattern,
    & mAlgebraicVariableNamePattern, & mRateNamePattern,
    & mVOIPattern, & mSampleDensityFunctionPattern,
    & mSampleRealisationsPattern,  & mBoundVariableName,
    & mAssignPattern, & mAssignConstantPattern, & mSolvePattern,
    & mSolveNLSystemPattern, & mTemporaryVariablePattern,
    & mDeclareTemporaryPattern, & mConditionalAssignmentPattern, & mResidualPattern,
    & mConstrainedRateStateInfoPattern, & mUnconstrainedRateStateInfoPattern,
    & mInfDelayedRatePattern, & mInfDelayedStatePattern,
    & mConditionVariablePattern;
  bool mTrackPiecewiseConditions;
  uint32_t mArrayOffset;
  ObjRef<iface::cellml_services::MaLaESTransform> mTransform;
  ObjRef<iface::cellml_services::CeVAS> mCeVAS;
  ObjRef<iface::cellml_services::CUSES> mCUSES;
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;
  ObjRef<CDA_CodeInformation> mCodeInfo;
  std::list<ptr_tag<CDA_ComputationTarget> > mBaseTargets;
  std::set<ptr_tag<CDA_ComputationTarget> > mKnown, mFloating, mUnwanted;
  std::map<ptr_tag<CDA_ComputationTarget>, double> mInitialOverrides;
  std::list<ptr_tag<MathStatement> > mMathStatements;
  std::set<std::pair<ptr_tag<CDA_ComputationTarget>, ptr_tag<MathStatement> > > mResets;
  std::list<System*> mSystems;
  std::set<ptr_tag<MathStatement> > mUnusedMathStatements;
  std::map<std::set<ptr_tag<CDA_ComputationTarget> >, std::set<ptr_tag<MathStatement> > > mEdgesInto;
  std::map<iface::cellml_api::CellMLVariable*, ptr_tag<CDA_ComputationTarget> >
    mTargetsBySource;
  std::set<ptr_tag<CDA_ComputationTarget> > mBoundTargs, mLocallyBoundTargs, mDelayedTargs;
  uint32_t mNextConstantIndex, mNextStateVariableIndex,
    mNextAlgebraicVariableIndex, mNextVOI, mNextConditionVariable, mNextSolveId;
  std::list<std::pair<ptr_tag<CDA_ComputationTarget>, std::wstring> > mRateNameBackup;
  std::list<ptr_tag<CDA_ComputationTarget> > mInfDelayedTargets;
  bool mIDAStyle;
  bool mIsConstant;
  struct RootInformation {
    RootInformation(const std::wstring& aStorageName,
                    iface::cellml_api::CellMLComponent* aComponent,
                    iface::mathml_dom::MathMLContentElement* aMathEl)
      : storageName(aStorageName), component(aComponent),
        mathEl(aMathEl) {}
    std::wstring storageName;
    iface::cellml_api::CellMLComponent* component;
    ObjRef<iface::mathml_dom::MathMLContentElement> mathEl;
  };

  std::list<RootInformation> mRootInformation;
  bool mDryRun;
};

#endif // _CodeGenerationState_hxx
