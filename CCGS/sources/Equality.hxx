#include "CodeGenerationState.hxx"
#include <set>

class ProceduralStep
{
public:
  ProceduralStep();
  virtual ~ProceduralStep() {}

  void AddDependency(ProceduralStep* dep)
  {
    mDependencies.insert(dep);
  }

  void RecursivelyGenerateCode(CodeGenerationState* aCGS,
                               std::stringstream& aMainCode,
                               std::stringstream& aSupplementary);
protected:
  virtual void GenerateCode(CodeGenerationState* aCGS,
                            std::stringstream& aMainCode,
                            std::stringstream& aSupplementary) = 0;
  bool stepSeen, stepDone;
  std::set<ProceduralStep*> mDependencies;
};

class AssignmentProceduralStep
  : public ProceduralStep
{
public:
  /**
   * Creates a new procedural step that assigns one variable from a function of
   * other variables.
   * @param aAssignInto The source (no in interfaces) variable into which the
   *                    value should be saved.
   * @param aComponent The component from which this assignment arose. This is
   *                   used to look up variables mentioned in the maths.
   * @param aRHSMaths The expression used to compute the value of the variable.
   * @param aHaveBound True if BOUND will be available (to pass to functions
   *                    like the definite integral solver).
   * @param aFactor The factor to multiply the value by before assigning into
   *                the aAssignInto variable (for units conversion).
   * @param aOffset The offset to add to the value before assigning into the
   *                aAssignInto variable (for units conversion).
   */
  AssignmentProceduralStep(iface::cellml_api::CellMLVariable* aAssignInto,
                           iface::cellml_api::CellMLComponent* aComponent,
                           iface::mathml_dom::MathMLElement* aRHSMaths,
                           bool aHaveBound,
                           double aFactor,
                           double aOffset)
    : ProceduralStep(), mAssignInto(aAssignInto), mComponent(aComponent),
      mRHSMaths(aRHSMaths), mHaveBound(aHaveBound), mFactor(aFactor),
      mOffset(aOffset)
  {
  }

protected:
  void GenerateCode(CodeGenerationState* aCGS,
                    std::stringstream& aMainCode,
                    std::stringstream& aSupplementary);
  iface::cellml_api::CellMLVariable* mAssignInto;
  iface::cellml_api::CellMLComponent* mComponent;
  iface::mathml_dom::MathMLElement* mRHSMaths;
  bool mHaveBound;
  double mFactor;
  double mOffset;
};

class InitialValueCopyProceduralStep
  : public ProceduralStep
{
public:
  InitialValueCopyProceduralStep(InitialAssignment& aIA)
    : IA(aIA)
  {
  }

protected:
  void GenerateCode(CodeGenerationState* aCGS,
                    std::stringstream& aMainCode,
                    std::stringstream& aSupplementary);

private:
  InitialAssignment IA;
};

class NewtonRaphsonProceduralStep
  : public ProceduralStep
{
 public:
  NewtonRaphsonProceduralStep(iface::cellml_api::CellMLComponent* aComp,
                              iface::mathml_dom::MathMLElement* aEl1,
                              iface::mathml_dom::MathMLElement* aEl2,
                              iface::cellml_api::CellMLVariable* aVar,
                              bool aHaveBound)
    : mComp(aComp), mEl1(aEl1), mEl2(aEl2), mVar(aVar), mHaveBound(aHaveBound)
  {
  }

protected:
  void GenerateCode(CodeGenerationState* aCGS,
                    std::stringstream& aMainCode,
                    std::stringstream& aSupplementary);

private:
  iface::cellml_api::CellMLComponent* mComp;
  iface::mathml_dom::MathMLElement* mEl1;
  iface::mathml_dom::MathMLElement* mEl2;
  iface::cellml_api::CellMLVariable* mVar;
  bool mHaveBound;
};

class Equation
  : public iface::XPCOM::IObject
{
public:
  CDA_IMPL_ID;
  CDA_IMPL_REFCOUNT;
  CDA_IMPL_QI0;

  Equation();
  ~Equation();

  /**
   * Adds a new part to the equation.
   * @param aComp The component this part belongs to.
   * @param aEl The MathML element to add.
   */
  void AddPart(iface::cellml_api::CellMLComponent* aComp,
               iface::mathml_dom::MathMLElement* aEl);

  /**
   * Tells it to recurse through all variables touched by this equation.
   * This both sets up data structures needed to compute procedural steps,
   * and sets flags on the variables, needed by code throughout CCGS.
   * Also sets up the data structures for tracking assignments of initial
   * values from variables.
   */
  void FlagVariables(CodeGenerationState* aCGS,
                     std::list<InitialAssignment>& aInitialAssignments);

  /**
   * Computes a DAG describing the procedural steps that have to be followed
   * and the dependencies between them.
   * @param aCGS The code generation service.
   * @param aAvailable The set of available variables.
   * @param aWanted The set of wanted variables.
   * @param stepsForVariable A mapping between variables and procedural steps.
   * @param aHaveBound True if the BOUND array will be defined.
   */
  bool ComputeProceduralSteps(CodeGenerationState* aCGS,
                              std::set<VariableInformation*>& aAvailable,
                              std::set<VariableInformation*>& aWanted,
                              std::map<VariableInformation*,ProceduralStep*>&
                                stepsForVariable,
                              bool aHaveBound);
  bool AttemptRateEvaluation(CodeGenerationState* aCGS,
                             std::stringstream& aRateStream,
                             std::stringstream& aSupplementary,
                             std::set<VariableInformation*>& aTouchedVariables);
  void AddIfTriggersNR(std::vector<iface::dom::Element*>& aNRList);

  void release_state();

private:
  std::list<std::pair<iface::cellml_api::CellMLComponent*,
                      iface::mathml_dom::MathMLElement*> > equal;
  ObjRef<iface::mathml_dom::MathMLCiElement> mDiffCI, mBoundCI;
  ObjRef<iface::mathml_dom::MathMLElement> mDiff;
  std::list<std::set<VariableInformation*> > mEquationVariables;
  bool mTriggersNewtonRaphson;
};
