#ifndef _CodeGenerationState_hxx
#define _CodeGenerationState_hxx

#include "Units.hxx"
#include "Variables.hxx"
#include "IfaceCCGS.hxx"
#include <vector>

class Equation;

struct InitialAssignment
{
  VariableInformation* source;
  VariableInformation* destination;
  double factor, offset;
};

class CodeGenerationState
{
public:
  CodeGenerationState();
  ~CodeGenerationState();

  void SetupBuiltinUnits();
  void CreateModelScopes(iface::cellml_api::Model* aModel);
  void PutUnitsIntoScope(CellMLScope* aScope,
                         iface::cellml_api::Units* aUnits);
  void CreateComponentScope(CellMLScope* aModelScope,
                            iface::cellml_api::CellMLComponent* aComp);
  void CreateComponentList(iface::cellml_api::Model* aModel);
  void AddEncapsulationDescendentComponents(iface::cellml_api::CellMLComponent*
                                            aComponent);
  void ProcessMath();
  void ComponentHasMath(iface::cellml_api::CellMLComponent* aComp,
                        iface::mathml_dom::MathMLApplyElement* aApply);
  void ProcessMathInComponent(iface::cellml_api::CellMLComponent* aComp);
  void ProcessVariables();
  void AssignVariableIndices();
  void DetermineConstants(std::stringstream& aConstStream);
  void DetermineComputedConstants(std::stringstream& aCompConstStream,
                                  std::stringstream& aSupplementary);
  void DetermineIterationVariables(std::stringstream& aIterationStream,
                                   std::stringstream& aSupplementary);
  void DetermineRateVariables(std::stringstream& aRateStream,
                              std::stringstream& aSupplementary);
  /**
   * Gets the conversion between vfrom and vto.
   * @param in vfrom The first ("from") variable.
   * @param in vto The second ("to") variable.
   * @return A number by which units in vfrom should be multiplied to convert
   *         into the right units to store in the to variable.
   * @param out offset A number which should be added to the number multiplied
   *                   by the return value before storing into the to variable.
   */
  double GetConversion(iface::cellml_api::CellMLVariable* vfrom,
                       iface::cellml_api::CellMLVariable* vto, double& offset);
  std::string GetVariableText(iface::cellml_api::CellMLVariable* var);
  uint32_t GetVariableIndex(iface::cellml_api::CellMLVariable* var);
  void EquationFullyUsed(Equation* aUsedEquation);

  VariableInformation*
  FindOrAddVariableInformation(iface::cellml_api::CellMLVariable* var);

  uint32_t AssignFunctionId();

  bool UnitsValid(iface::cellml_api::CellMLComponent* aComponent,
                  const std::wstring& aUnits);

  void CountVariablesAndRates(uint32_t& aVariableCount, uint32_t& aRateCount);
  void VariablesToCCodeVariables
  (std::list<iface::cellml_services::CCodeVariable*>& aVarList);
  void ListFlaggedEquations(std::vector<iface::dom::Element*>& aFlaggedEqns);

private:
  TemporaryAnnotationManager annot;
  TemporaryAnnotationKey scopeKey, varinfoKey;
  CellMLScope mGlobalScope;
  std::list<Equation*> mEquations;
  std::list<CellMLScope*> mModelScopes, mComponentScopes;
  // Note that we don't hold references to these components, because we
  // will always be destroyed before the components...
  std::list<iface::cellml_api::CellMLComponent*> mComponentList;
  // A map between the ID of the variable and the VariableInformation...
  std::map<std::string,VariableInformation*> mVariableByObjid;
  std::list<VariableInformation*> mVariableList;
  std::list<InitialAssignment> mInitialAssignments;
  uint32_t mLastFunctionId;
};

#endif // _CodeGenerationState_hxx
