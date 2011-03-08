#include "SProSTest.hpp"

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( SProSTest );

void
SProSTest::setUp()
{
}

void
SProSTest::tearDown()
{
}


// module SProS
// {
//   interface Bootstrap
//     : XPCOM::IObject
void
SProSTest::testSProSBootstrap()
{
  RETURN_INTO_OBJREF(cb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
//   {
//     /**
//      * Creates an empty SEDML element in a new document.
//      */
//     SEDMLElement createEmptySEDML();
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement, cb->createEmptySEDML());
  CPPUNIT_ASSERT(es);
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, es->models());
  RETURN_INTO_OBJREF(msi, iface::SProS::BaseIterator, ms->iterateElements());
  CPPUNIT_ASSERT(NULL == msi->nextElement());

// 
//     /**
//      * Parses SEDML from a specified URL, using the specified relative URL.
//      */
//     SEDMLElement parseSEDMLFromURI(in wstring uri, in wstring relativeTo);
  es = already_AddRefd<iface::SProS::SEDMLElement>(cb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(sims, iface::SProS::SimulationSet,
                     es->simulations());
  RETURN_INTO_OBJREF(sim, iface::SProS::Simulation,
                     sims->getSimulationByIdentifier(L"simulation1"));
  RETURN_INTO_WSTRING(kisaoID, sim->algorithmKisaoID());
  CPPUNIT_ASSERT(kisaoID == L"KISAO:0000019");

// 
//     /**
//      * Parses SEDML from a specified blob of text, using the specified base URI.
//      */
//     SEDMLElement parseSEDMLFromText(in wstring txt, in wstring baseURI);
// 
//     /**
//      * Makes a SEDML structure from an element.
//      */
//     SEDMLElement makeSEDMLFromElement(in dom::Element el);
// 
//     /**
//      * Serialises a SEDML element to text.
//      */
//     wstring sedmlToText(in SEDMLElement el);
//   };
}
// 
//   /**
//    * Base is implemented by all types of element in SEDML.
//    */
//   interface Base
//     : XPCOM::IObject
//   {
//     /**
//      * The underlying DOM element.
//      */
//     readonly attribute dom::Element domElement;
// 
//     /**
//      * The list of all note elements associated with this element.
//      */
//     readonly attribute dom::NodeList notes;
// 
//     /**
//      * The list of all annotations associated with this element.
//      */
//     readonly attribute dom::NodeList annotations;
//   };
// 
//   exception SProSException
//   {
//   };
// 
//   /**
//    * The base interface for sets of elements in SEDML.
//    */
//   interface BaseSet
//     : XPCOM::IObject
//   {
//     /**
//      * Obtain an iterator for iterating through the elements.
//      */
//     BaseIterator iterateElements();
// 
//     /**
//      * Inserts an element into the set. Raises an exception if the element is
//      * already in another set, or was created for the wrong document.
//      */
//     void insert(in Base b) raises(SProSException);
// 
//     /**
//      * Removes an element from the set.
//      */
//     void remove(in Base b);
//   };
// 
//   /**
//    * The base interface for iterating sets of SEDML elements.
//    */
//   interface BaseIterator
//     : XPCOM::IObject
//   {
//     /**
//      * Fetch the next element from the iterator, or null if there are no more
//      * elements.
//      */
//     Base nextElement();
//   };
// 
//   /**
//    * The top-level SEDML element.
//    */
//   interface SEDMLElement
//     : Base
//   {
//     /**
//      * The SEDML level; defaults to the latest level supported by the
//      * implementation.
//      */
//     attribute unsigned long level;
//     
//     /**
//      * The SEDML version; defaults to the latest version of SEDML supported by
//      * the implementation.
//      */
//     attribute unsigned long version;
// 
//     /**
//      * The set of all models.
//      */
//     readonly attribute ModelSet models;
// 
//     /**
//      * The set of all tasks.
//      */
//     readonly attribute TaskSet tasks;
// 
//     /**
//      * The set of all simulations.
//      */
//     readonly attribute SimulationSet simulations;
// 
//     /**
//      * The set of all generators.
//      */
//     readonly attribute DataGeneratorSet generators;
// 
//     /**
//      * The set of all outputs.
//      */
//     readonly attribute OutputSet outputs;
// 
//     /**
//      * Creates a new Model. It is created with no parent and must be
//      * explicitly added.
//      */
//     Model createModel();
// 
//     /**
//      * Creates a new UniformTimeCourse. It is created with no parent and must be
//      * explicitly added.
//      */
//     UniformTimeCourse createUniformTimeCourse();
// 
//     /**
//      * Creates a new Task. It is created with no parent and must be
//      * explicitly added.
//      */
//     Task createTask();
// 
//     /**
//      * Creates a new DataGenerator. It is created with no parent and must be
//      * explicitly added.
//      */
//     DataGenerator createDataGenerator();
// 
//     /**
//      * Creates a new Plot2D. It is created with no parent and must be
//      * explicitly added.
//      */
//     Plot2D createPlot2D();
// 
//     /**
//      * Creates a new Plot3D. It is created with no parent and must be
//      * explicitly added.
//      */
//     Plot3D createPlot3D();
// 
//     /**
//      * Creates a new Report. It is created with no parent and must be
//      * explicitly added.
//      */
//     Report createReport();
// 
//     /**
//      * Creates a new ComputeChange. It is created with no parent and must be
//      * explicitly added.
//      */
//     ComputeChange createComputeChange();
// 
//     /**
//      * Creates a new ChangeAttribute. It is created with no parent and must be
//      * explicitly added.
//      */
//     ChangeAttribute createChangeAttribute();
// 
//     /**
//      * Creates a new AddXML. It is created with no parent and must be
//      * explicitly added.
//      */
//     AddXML createAddXML();
// 
//     /**
//      * Creates a new RemoveXML. It is created with no parent and must be
//      * explicitly added.
//      */
//     RemoveXML createRemoveXML();
// 
//     /**
//      * Creates a new ChangeXML. It is created with no parent and must be
//      * explicitly added.
//      */
//     ChangeXML createChangeXML();
// 
//     /**
//      * Creates a new Variable. It is created with no parent and must be
//      * explicitly added.
//      */
//     Variable createVariable();
// 
//     /**
//      * Creates a new Parameter. It is created with no parent and must be
//      * explicitly added.
//      */
//     Parameter createParameter();
// 
//     /**
//      * Creates a new Curve. It is created with no parent and must be
//      * explicitly added.
//      */
//     Curve createCurve();
// 
//     /**
//      * Creates a new Surface. It is created with no parent and must be
//      * explicitly added.
//      */
//     Surface createSurface();
// 
//     /**
//      * Creates a new DataSet. It is created with no parent and must be
//      * explicitly added.
//      */
//     DataSet createDataSet();
//   };
// 
//   /**
//    * The base interface for all elements with name attributes.
//    */
//   interface NamedElement
//     : Base
//   {
//     /**
//      * The human readable name of the element (not guaranteed to be unique).
//      */
//     attribute wstring name;
//   };
// 
//   /**
//    * The base type for all sets of elements with name attributes.
//    */
//   interface NamedElementSet
//     : BaseSet
//   {
//     /**
//      * Iterate through all named elements.
//      */
//     NamedElementIterator iterateNamedElement();
// 
//     /* Note: Names aren't guaranteed to be unique, they are for human use, so
//      * we don't provide a way to find by name.
//      */
//   };
// 
//   /**
//    * The base type of all iterators of elements with name attributes.
//    */
//   interface NamedElementIterator
//     : BaseIterator
//   {
//     /**
//      * Fetch the next named element, or null if there are no more elements.
//      */
//     NamedElement nextNamedElement();
//   };
// 
//   /**
//    * The base interface for all elements with name and id attributes.
//    */
//   interface NamedIdentifiedElement
//     : NamedElement
//   {
//     /**
//      * The unique identifier for the element.
//      */
//     attribute wstring id;
//   };
// 
//   /**
//    * The base type for all sets of named, identified elements.
//    */
//   interface NamedIdentifiedElementSet
//     : NamedElementSet
//   {
//     /**
//      * Iterates through all the named, identified elements...
//      */
//     NamedIdentifiedElementIterator iterateNamedIdentifiedElements();
// 
//     /**
//      * Finds an element by identifier
//      */
//     NamedIdentifiedElement getNamedIdentifiedElementByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * Allows named, identified elements to be iterated.
//    */
//   interface NamedIdentifiedElementIterator
//     : NamedElementIterator
//   {
//     /**
//      * Fetches the next named, identified element, or null if there are no more
//      * such elements.
//      */
//     NamedIdentifiedElement nextNamedIdentifiedElement();
//   };
// 
//   /**
//    * A SEDML reference to a particular model.
//    */
//   interface Model
//     : NamedIdentifiedElement
//   {
//     /** The language the model is in. */
//     attribute wstring language;
// 
//     /** The location of the model. */
//     attribute wstring source;
// 
//     /**
//      * The set of changes to make to the model.
//      */
//     readonly attribute ChangeSet changes;
//   };
// 
//   /**
//    * A set of models.
//    */
//   interface ModelSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterates through all models in the set.
//      */
//     ModelIterator iterateModels();
// 
//     /**
//      * Search for a model in the set by identifier.
//      */
//     Model getModelByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An iterator of models.
//    */
//   interface ModelIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Fetches the next model, or null if there are no more.
//      */
//     Model nextModel();
//   };
// 
//   /**
//    * A SEDML simulation.
//    */
//   interface Simulation
//     : NamedIdentifiedElement
//   {
//     /**
//      * The KISAO identifier for the corresponding algorithm.
//      */
//     attribute wstring algorithmKisaoID;
//   };
// 
//   /**
//    * A set of simulations.
//    */
//   interface SimulationSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterates through the simulations in the set.
//      */
//     SimulationIterator iterateSimulations();
// 
//     /**
//      * Finds a simulation in the set by identifier.
//      */
//     Simulation getSimulationByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An iterator of simulations.
//    */
//   interface SimulationIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Fetches the next Simulation, or null if there are no more.
//      */
//     Simulation nextSimulation();
//   };
// 
//   /**
//    * A Uniform Time Course simulation.
//    */
//   interface UniformTimeCourse
//     : Simulation
//   {
//     /**
//      * The time the simulation starts.
//      */
//     attribute double initialTime;
// 
//     /**
//      * The time of the first point to generate output for.
//      */
//     attribute double outputStartTime;
// 
//     /**
//      * The time of the last point to generate output for.
//      */
//     attribute double outputEndTime;
// 
//     /**
//      * The number of points of output to produce.
//      */
//     attribute unsigned long numberOfPoints;
//   };
// 
//   /**
//    * A SEDML task.
//    */
//   interface Task
//     : NamedIdentifiedElement
//   {
//     /**
//      * The referenced simulation, as an identifier.
//      */
//     attribute wstring simulationReferenceIdentifier;
// 
//     /**
//      * The referenced simulation object.
//      */
//     attribute Simulation simulationReference;
// 
//     /**
//      * The referenced model, as an identifier.
//      */
//     attribute wstring modelReferenceIdentifier;
// 
//     /**
//      * The referenced model object.
//      */
//     attribute Model modelReference;
//   };
// 
//   /**
//    * A set of SEDML tasks.
//    */
//   interface TaskSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterate through the tasks in this set.
//      */
//     TaskIterator iterateTasks();
// 
//     /**
//      * Find a task in the set by the identifier.
//      */
//     Task getTaskByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An iterator of SEDML tasks.
//    */
//   interface TaskIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Fetch the next task, or null if there are no more.
//      */
//     Task nextTask();
//   };
// 
//   /**
//    * A SEDML DataGenerator.
//    */
//   interface DataGenerator
//     : NamedIdentifiedElement, cellml_api::MathContainer
//   {
//     /**
//      * The set of parameters for this DataGenerator.
//      */
//     readonly attribute ParameterSet parameters;
// 
//     /**
//      * The set of variables for this DataGenerator.
//      */
//     readonly attribute VariableSet variables;
//   };
// 
//   /**
//    * A set of DataGenerators.
//    */
//   interface DataGeneratorSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterate through the DataGenerators in this set.
//      */
//     DataGeneratorIterator iterateDataGenerators();
// 
//     /**
//      * Find a DataGenerator by identfier.
//      */
//     DataGenerator getDataGeneratorByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An interator of DataGenerators.
//    */
//   interface DataGeneratorIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Fetches the next DataGenerator, or null if there are no more.
//      */
//     DataGenerator nextDataGenerator();
//   };
// 
//   /**
//    * A SEDML Output.
//    */
//   interface Output
//     : NamedIdentifiedElement
//   {
//   };
// 
//   /**
//    * A set of Outputs
//    */
//   interface OutputSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterates through all the outputs in this set.
//      */
//     OutputIterator iterateOutputs();
//     
//     /**
//      * Finds an output in this set by identifier.
//      */
//     Output getOutputByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An iterator of Outputs
//    */
//   interface OutputIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Fetches the next Output, or null if there are no more.
//      */
//     Output nextOutput();
//   };
// 
//   /**
//    * A SEDML 2D Plot output
//    */
//   interface Plot2D
//     : Output
//   {
//     /**
//      * Gets the set of all curves.
//      */
//     readonly attribute CurveSet curves;
//   };
// 
//   /**
//    * A SEDML 3D Plot output
//    */
//   interface Plot3D
//     : Output
//   {
//     /**
//      *  Gets the set of all surfaces.
//      */
//     readonly attribute SurfaceSet surfaces;
//   };
// 
//   /**
//    * A SEDML Report output
//    */
//   interface Report
//     : Output
//   {
//     /**
//      * Gets the set of all DataSets
//      */
//     readonly attribute DataSetSet datasets;
//   };
// 
//   /**
//    * A SEDML Change.
//    */
//   interface Change
//     : Base
//   {
//     attribute wstring target;
//   };
// 
//   /**
//    * A set of Changes
//    */
//   interface ChangeSet
//     : BaseSet
//   {
//     /**
//      * Iterate through all the Changes in the set.
//      */
//     ChangeIterator iterateChanges();
//   };
// 
//   /**
//    * An iterator of Changes
//    */
//   interface ChangeIterator
//     : BaseIterator
//   {
//     /**
//      * Fetch the next Change, or null if there are no more.
//      */
//     Change nextChange();
//   };
// 
//   /**
//    * A SEDML ComputeChange style change.
//    */
//   interface ComputeChange
//     : Change, cellml_api::MathContainer
//   {
//     /**
//      * All variables involved.
//      */
//     readonly attribute VariableSet variables;
// 
//     /**
//      * All parameters involved.
//      */
//     readonly attribute ParameterSet parameters;
//   };
// 
//   /**
//    * A SEDML attribute change.
//    */
//   interface ChangeAttribute
//     : Change
//   {
//     /**
//      * The new value of the attribute.
//      */
//     readonly attribute wstring newValue;
//   };
// 
//   /**
//    * A SEDML XML addition.
//    */
//   interface AddXML
//     : Change
//   {
//     /**
//      * The XML content to add.
//      */
//     readonly attribute dom::NodeList anyXML;
//   };
// 
//   /**
//    * A SEDML XML change.
//    */
//   interface ChangeXML
//     : AddXML
//   {
//   };
// 
//   /**
//    * A SEDML XML removal.
//    */
//   interface RemoveXML
//     : Change
//   {
//   };
// 
//   /**
//    * A SEDML Variable.
//    */
//   interface Variable
//     : NamedIdentifiedElement
//   {
//     attribute wstring target;
//     attribute wstring symbol;
//   };
// 
//   /**
//    * A set of Variables.
//    */
//   interface VariableSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterates through the variables.
//      */
//     VariableIterator iterateVariables();
// 
//     /**
//      * Finds a variable in the set by identifier.
//      */
//     Variable getVariableByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An iterator of Variables.
//    */
//   interface VariableIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Gets the next variable, or null if there are no more.
//      */
//     Variable nextVariable();
//   };
// 
//   /**
//    * A SEDML Parameter.
//    */
//   interface Parameter
//     : NamedIdentifiedElement
//   {
//     attribute double value;
//   };
// 
//   /**
//    * A set of Parameters.
//    */
//   interface ParameterSet
//     : NamedIdentifiedElementSet
//   {
//     /**
//      * Iterates through the parameters.
//      */
//     ParameterIterator iterateParameters();
// 
//     /**
//      * Finds a parameter in the set by identifier.
//      */
//     Parameter getParameterByIdentifier(in wstring idMatch);
//   };
// 
//   /**
//    * An iterator of Parameters.
//    */
//   interface ParameterIterator
//     : NamedIdentifiedElementIterator
//   {
//     /**
//      * Gets the next parameter, or null if there are no more.
//      */
//     Parameter nextParameter();
//   };
// 
//   /**
//    * A SEDML Curve.
//    */
//   interface Curve
//     : NamedElement
//   {
//     /**
//      * If true, log-transform the X axis.
//      */
//     attribute boolean logX;
// 
//     /**
//      * If true, log-transform the Y axis.
//      */
//     attribute boolean logY;
// 
//     /**
//      * The identifier of the X-axis data generator.
//      */
//     attribute wstring xDataGeneratorID;
// 
//     /**
//      * The identifier of the Y-axis data generator.
//      */
//     attribute wstring yDataGeneratorID;
// 
//     /**
//      * The X-axis data generator (if it can be found, otherwise
//      * null).
//      */
//     attribute DataGenerator xDataGenerator;
// 
//     /**
//      * The Y-axis data generator (if it can be found, otherwise
//      * null).
//      */
//     attribute DataGenerator yDataGenerator;
//   };
// 
//   /**
//    * A set of SEDML Curve.
//    */
//   interface CurveSet
//     : NamedElementSet
//   {
//     /**
//      * Iterates through the curves in the set.
//      */
//     CurveIterator iterateCurves();
//   };
// 
//   /**
//    * Allows SEDML Curves to be iterated.
//    */
//   interface CurveIterator
//     : NamedElementIterator
//   {
//     /**
//      * Fetches the next curve in the set, or null if there are no more.
//      */
//     Curve nextCurve();
//   };
// 
//   /**
//    * A SEDML Surface.
//    */
//   interface Surface
//     : Curve
//   {
//     /**
//      * If true, log the Z axis.
//      */
//     attribute boolean logZ;
// 
//     /**
//      * The identifier of the Z-axis data generator.
//      */
//     attribute wstring zDataGeneratorID;
// 
//     /**
//      * The Z-axis data generator (if it can be found, otherwise
//      * null).
//      */
//     attribute DataGenerator zDataGenerator;
//   };
// 
//   /**
//    * A set of SEDML Surfaces.
//    */
//   interface SurfaceSet
//     : CurveSet
//   {
//     /**
//      * Iterates the surfaces in this set.
//      */
//     SurfaceIterator iterateSurfaces();
//   };
// 
//   /**
//    * Allows SEDML Surfaces to be iterated.
//    */
//   interface SurfaceIterator
//     : CurveIterator
//   {
//     /**
//      * Fetches the next surface in the set, or null if there are no more.
//      */
//     Surface nextSurface();
//   };
// 
//   /**
//    * A SEDML DataSet.
//    */
//   interface DataSet
//     : NamedElement
//   {
//     /**
//      * The identifier of the data generator.
//      */
//     attribute wstring dataGeneratorID;
// 
//     /**
//      * The data generator (if it can be found, otherwise
//      * null). It must already be in the main set of
//      * DataGenerators and have an ID.
//      */
//     attribute DataGenerator dataGen;
//   };
// 
//   /**
//    * A set of SEDML DataSets.
//    */
//   interface DataSetSet
//     : NamedElementSet
//   {
//     /**
//      * Iterates through the DataSets in this set.
//      */
//     DataSetIterator iterateDataSets();
//   };
// 
//   /**
//    * Allows SEDML DataSets to be iterated.
//    */
//   interface DataSetIterator
//     : NamedElementIterator
//   {
//     /**
//      * Fetches the next DataSet, or null if there are no more.
//      */
//     DataSet nextDataSet();
//   };
// };


