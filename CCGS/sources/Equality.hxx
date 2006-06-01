#include "CodeGenerationState.hxx"
#include <set>

class Equation
  : public iface::XPCOM::IObject
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI0;

  Equation();

  void AddPart(iface::cellml_api::CellMLComponent* aComp,
               iface::mathml_dom::MathMLElement* aEl);
  bool AllowsComputationFrom(std::list<VariableInformation*>& vi);
  void FlagVariables(CodeGenerationState* aCGS,
                     std::list<InitialAssignment>& aInitialAssignments);
  bool AttemptEvaluation(CodeGenerationState* aCGS,
                         std::set<VariableInformation*>& aAvailable,
                         std::set<VariableInformation*>& aWanted,
                         std::stringstream& aExpression,
                         std::stringstream& aSupplementary);
  bool AttemptRateEvaluation(CodeGenerationState* aCGS,
                             std::stringstream& aRateStream,
                             std::stringstream& aSupplementary);
  void AddIfTriggersNR(std::vector<iface::dom::Element*>& aNRList);

private:
  std::list<std::pair<iface::cellml_api::CellMLComponent*,
                      iface::mathml_dom::MathMLElement*> > equal;
  iface::mathml_dom::MathMLCiElement* mDiffCI, * mBoundCI;
  iface::mathml_dom::MathMLElement* mDiff;
  bool mTriggersNewtonRaphson;
};
