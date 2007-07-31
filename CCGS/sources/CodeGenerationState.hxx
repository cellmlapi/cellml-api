#ifndef _CodeGenerationState_hxx
#define _CodeGenerationState_hxx

#include "IfaceCCGS.hxx"
#include <vector>
#include <set>
#include <map>
#include "IfaceCellML_APISPEC.hxx"

// A variable edge is an equation which allows one or more
// variables to be found from another variable.
struct VariableEdge
{
public:
  VariableEdge() {}
  ~VariableEdge() {}
  
  std::list<CDA_ComputationTarget*> mTargets;
  // If mMaths == null, this is a special variable edge which is created when
  // an initial_value="name" construct is found. It can only be used to compute
  // the first target from the second (must have exactly two targets).
  ObjRef<iface::mathml_dom::MathMLApplyElement> mMaths;
  ObjRef<iface::cellml_api::CellMLComponent> mContext;

  // The target which is actually computed (determined by connectivity
  // analysis).
  CDA_ComputationTarget* mComputedTarget;
  // The left-hand side. NULL if mRHS has to be minimised.
  ObjRef<iface::mathml_dom::MathMLElement> mLHS;
  // The right-hand side.
  ObjRef<iface::mathml_dom::MathMLElement> mRHS;
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
  void CreateVariableEdges();
  void ContextError(const std::wstring& details,
                    iface::mathml_dom::MathMLElement* context1,
                    iface::cellml_api::CellMLElement* context2);
  void FirstPassTargetClassification();
  void AllocateVariable(CDA_ComputationTarget* aCT, std::wstring& aStr,
                        std::wstring& aPattern, uint32_t& aNextIndex);
  void GenerateVariableName(CDA_ComputationTarget* aCT, std::wstring& aStr,
                            std::wstring& aPattern, uint32_t index);
  void AllocateVariablesInSet(std::set<CDA_ComputationTarget*>& aSet,
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
  uint32_t BuildTargetSet(std::list<CDA_ComputationTarget*>& aStart,
                          std::list<CDA_ComputationTarget*>& aCandidates,
                          std::set<CDA_ComputationTarget*>& aTargetSet);
  bool ConsiderEdgeInTargetSet(VariableEdge* aVarEdge,
                               std::set<CDA_ComputationTarget*>& aStart,
                               std::set<CDA_ComputationTarget*>& aCandidates,
                               std::set<CDA_ComputationTarget*>& aTargetSet);
  void GenerateCodeForSet(std::wstring& aCodeTo,
                          std::set<CDA_ComputationTarget*>& aTargets);
  void GenerateCodeForSetByType(std::set<CDA_ComputationTarget*>& aTargets);
  void GenerateStateToRateCascades();
  void GenerateCodeForEdge(std::wstring& aCodeTo, VariableEdge* aVE);
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
   VariableEdge* aVE
  );

  ObjRef<iface::cellml_api::Model> mModel;
  std::wstring & mConstantPattern, & mStateVariableNamePattern,
    & mAlgebraicVariableNamePattern, & mRateNamePattern,
    & mVOIPattern, & mAssignPattern, & mSolvePattern;
  uint32_t mArrayOffset;
  ObjRef<iface::cellml_services::MaLaESTransform> mTransform;
  ObjRef<iface::cellml_services::CeVAS> mCeVAS;
  ObjRef<iface::cellml_services::CUSES> mCUSES;
  ObjRef<iface::cellml_services::AnnotationSet> mAnnoSet;
  ObjRef<CDA_CodeInformation> mCodeInfo;
  std::list<CDA_ComputationTarget*> mBaseTargets, mKnown, mFloating;
  std::list<VariableEdge*> mVariableEdges;
  std::list<VariableEdge*> mUnusedEdges;
  std::map<CDA_ComputationTarget*, VariableEdge*> mEdgesInto;
  std::map<iface::cellml_api::CellMLVariable*, CDA_ComputationTarget*>
    mTargetsBySource;
  std::set<CDA_ComputationTarget*> mBoundTargs;
  uint32_t mNextConstantIndex, mNextStateVariableIndex,
    mNextAlgebraicVariableIndex, mNextVOI, mNextSolveId;
};

#endif // _CodeGenerationState_hxx
