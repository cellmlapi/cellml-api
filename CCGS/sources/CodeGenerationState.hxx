#ifndef _CodeGenerationState_hxx
#define _CodeGenerationState_hxx

#include "IfaceCCGS.hxx"
#include <vector>
#include <set>
#include <map>
#include "IfaceCellML_APISPEC.hxx"

typedef std::set<CDA_ComputationTarget> ComputationGroup;

/*
 * An equation, which relates its targets to each other.
 */
struct Equation
{
public:
  Equation() {}
  ~Equation() {}
  
  std::list<CDA_ComputationTarget*> mTargets;
  // If mMaths == null, this is a special variable edge which is created when
  // an initial_value="name" construct is found. It can only be used to compute
  // the first target from the second (must have exactly two targets).
  ObjRef<iface::mathml_dom::MathMLApplyElement> mMaths;
  ObjRef<iface::cellml_api::CellMLComponent> mContext;

  // The left-hand side. NULL if mRHS has to be minimised.
  ObjRef<iface::mathml_dom::MathMLElement> mLHS;
  // The right-hand side.
  ObjRef<iface::mathml_dom::MathMLElement> mRHS;

  std::wstring mVarName, mLHSCode, mRHSCode;
};

/*
 * A system, which is built up of n >=1 equations, and computes n unknowns from
 * an arbitrary number of knowns.
 */
struct System
{
public:
  System(std::set<Equation*>& aEquations,
         std::set<CDA_ComputationTarget*>& aKnowns,
         std::set<CDA_ComputationTarget*>& aUnknowns)
    : mEquations(aEquations), mKnowns(aKnowns), mUnknowns(aUnknowns)
  {
  }
  ~System() {}

  std::set<Equation*> mEquations;
  std::set<CDA_ComputationTarget*> mKnowns;
  // The targets which are actually computed (determined by connectivity
  // analysis).
  std::set<CDA_ComputationTarget*> mUnknowns;
};

class CodeGenerationState
{
public:
  CodeGenerationState(iface::cellml_api::Model* aModel,
                      std::wstring& aConstantPattern,
                      std::wstring& aStateVariableNamePattern,
                      std::wstring& aAlgebraicVariableNamePattern,
                      std::wstring& aRateNamePattern,
                      std::wstring& aVOIPattern,
                      std::wstring& aAssignPattern,
                      std::wstring& aSolvePattern,
                      std::wstring& aSolveNLSystemPattern,
                      uint32_t aArrayOffset,
                      iface::cellml_services::MaLaESTransform* aTransform,
                      iface::cellml_services::CeVAS* aCeVAS,
                      iface::cellml_services::CUSES* aCUSES,
                      iface::cellml_services::AnnotationSet* aAnnoSet)
    : mModel(aModel), mConstantPattern(aConstantPattern),
      mStateVariableNamePattern(aStateVariableNamePattern),
      mAlgebraicVariableNamePattern(aAlgebraicVariableNamePattern),
      mRateNamePattern(aRateNamePattern),
      mVOIPattern(aVOIPattern),
      mAssignPattern(aAssignPattern),
      mSolvePattern(aSolvePattern),
      mSolveNLSystemPattern(aSolveNLSystemPattern),
      mArrayOffset(aArrayOffset),
      mTransform(aTransform),
      mCeVAS(aCeVAS),
      mCUSES(aCUSES),
      mAnnoSet(aAnnoSet),
      mNextConstantIndex(aArrayOffset),
      mNextStateVariableIndex(aArrayOffset),
      mNextAlgebraicVariableIndex(aArrayOffset),
      mNextVOI(aArrayOffset),
      mNextSolveId(0)
  {
  }

  ~CodeGenerationState();

  iface::cellml_services::CodeInformation* GenerateCode();
  void CreateBaseComputationTargets();
  CDA_ComputationTarget* GetTargetOfDegree(CDA_ComputationTarget* aBase,
                                           uint32_t aDegree);
  void CreateEquations();
  void ContextError(const std::wstring& details,
                    iface::mathml_dom::MathMLElement* context1,
                    iface::cellml_api::CellMLElement* context2);
  void FirstPassTargetClassification();
  void AllocateVariable(CDA_ComputationTarget* aCT, std::wstring& aStr,
                        std::wstring& aPattern, uint32_t& aNextIndex,
                        uint32_t& aCount);
  void GenerateVariableName(CDA_ComputationTarget* aCT, std::wstring& aStr,
                            std::wstring& aPattern, uint32_t index);
  void AllocateVariablesInSet(std::list<System*>& aSet,
                              iface::cellml_services::VariableEvaluationType aET,
                              std::wstring& aPattern,
                              uint32_t& aNextIndex,
                              uint32_t& aCountVar);
  void AllocateConstant(CDA_ComputationTarget* aCT, std::wstring& aStr);
  void AllocateStateVariable(CDA_ComputationTarget* aCT, std::wstring& aStr);
  void AllocateAlgebraicVariable(CDA_ComputationTarget* aCT, std::wstring& aStr);
  void AllocateVOI(CDA_ComputationTarget* aCT, std::wstring& aStr);
  void AppendAssign(std::wstring& aAppendTo,
                    std::wstring& aLHS,
                    std::wstring& aRHS);
  void BuildFloatingAndConstantLists();
  void BuildFloatingAndKnownLists();
  void BuildStateAndConstantLists();
  bool DecomposeIntoSystems(std::set<CDA_ComputationTarget*>& aStart,
                            std::set<CDA_ComputationTarget*>& aCandidates,
                            std::list<System*>& aSystems);
  bool FindSmallSystem(
                       std::set<Equation*>& aUseEquations,
                       std::set<CDA_ComputationTarget*>& aUseVars,
                       std::set<CDA_ComputationTarget*>& aStart,
                       std::set<CDA_ComputationTarget*>& aCandidates,
                       std::list<System*>& aSystems
                      );

  bool FindBigSystem(
                     std::set<Equation*>& aUseEquations,
                     std::set<CDA_ComputationTarget*>& aUseVars,
                     std::set<CDA_ComputationTarget*>& aStart,
                     std::set<CDA_ComputationTarget*>& aCandidates,
                     std::list<System*>& aSystems
                    );

  bool RecursivelyTestSmallSystem(
                                  std::set<Equation*>& aSystem,
                                  std::set<Equation*>::iterator& aEqIt,
                                  uint32_t aNeedToAdd,
                                  std::set<Equation*>& aUseEquations,
                                  std::set<CDA_ComputationTarget*>& aUseVars,
                                  std::set<CDA_ComputationTarget*>& aStart,
                                  std::set<CDA_ComputationTarget*>& aCandidates,
                                  std::list<System*>& aSystems
                                 );

  bool RecursivelyTestBigSystem(
                                std::set<Equation*>& aNonSystem,
                                std::set<Equation*>::iterator& aEqIt,
                                uint32_t aNeedToRemove,
                                std::set<Equation*>& aUseEquations,
                                std::set<CDA_ComputationTarget*>& aUseVars,
                                std::set<CDA_ComputationTarget*>& aStart,
                                std::set<CDA_ComputationTarget*>& aCandidates
                               );

  void BuildSystemsByTargetsRequired(std::list<System*>& aSystems,
                                     std::map<CDA_ComputationTarget*, System*>&
                                     aSysByTargReq);

  void GenerateCodeForSet(std::wstring& aCodeTo,
                          std::set<CDA_ComputationTarget*>& aKnown,
                          std::list<System*>& aTargets,
                          std::map<CDA_ComputationTarget*, System*>&
                            aSysByTargReq
                         );
  void GenerateCodeForSetByType
  (
   std::set<CDA_ComputationTarget*>& aKnown,
   std::list<System*>& aSystems,
   std::map<CDA_ComputationTarget*, System*>&
     aSysByTargReq
  );
  void GenerateStateToRateCascades();
  void GenerateCodeForSystem(std::wstring& aCodeTo, System* aSys);
  iface::cellml_api::CellMLVariable* GetVariableInComponent
  (
   iface::cellml_api::CellMLComponent* aComp,
   iface::cellml_api::CellMLVariable* aVar
  );
  void GenerateAssignmentMaLaESResult
  (
   std::wstring& aCodeTo,
   CDA_ComputationTarget* aTarget,
   iface::cellml_services::MaLaESResult* aMR
  );
  void GenerateSolveCode
  (
   std::wstring& aCodeTo,
   Equation* aVE,
   CDA_ComputationTarget* aComputedTarget
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
   Equation* aEq,
   const std::wstring& aPattern,
   const wchar_t* aId,
   const wchar_t* aIndex
  );

  std::wstring ReplaceIDs
  (
   std::wstring aReplaceFrom,
   const wchar_t* aId,
   const wchar_t* aIndex,
   const wchar_t* aCount
  );

  ObjRef<iface::cellml_api::Model> mModel;
  std::wstring & mConstantPattern, & mStateVariableNamePattern,
    & mAlgebraicVariableNamePattern, & mRateNamePattern,
    & mVOIPattern, & mAssignPattern, & mSolvePattern,
    & mSolveNLSystemPattern;
  uint32_t mArrayOffset;
  ObjRef<iface::cellml_services::MaLaESTransform> mTransform;
  ObjRef<iface::cellml_services::CeVAS> mCeVAS;
  ObjRef<iface::cellml_services::CUSES> mCUSES;
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;
  ObjRef<CDA_CodeInformation> mCodeInfo;
  std::list<CDA_ComputationTarget*> mBaseTargets;
  std::set<CDA_ComputationTarget*> mKnown, mFloating;
  std::list<Equation*> mEquations;
  std::list<System*> mSystems;
  std::set<Equation*> mUnusedEquations;
  std::map<std::set<CDA_ComputationTarget*>, std::set<Equation*> > mEdgesInto;
  std::map<iface::cellml_api::CellMLVariable*, CDA_ComputationTarget*>
    mTargetsBySource;
  std::set<CDA_ComputationTarget*> mBoundTargs;
  uint32_t mNextConstantIndex, mNextStateVariableIndex,
    mNextAlgebraicVariableIndex, mNextVOI, mNextSolveId;
};

#endif // _CodeGenerationState_hxx
