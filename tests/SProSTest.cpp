#include "SProSTest.hpp"
#include "CellMLBootstrap.hpp"

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

#define SEDML_NS L"http://sed-ml.org/"

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
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
//   {
//     /**
//      * Creates an empty SEDML element in a new document.
//      */
//     SEDMLElement createEmptySEDML();
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement, sb->createEmptySEDML());
  CPPUNIT_ASSERT(es);
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, es->models());
  RETURN_INTO_OBJREF(msi, iface::SProS::BaseIterator, ms->iterateElements());
  CPPUNIT_ASSERT(NULL == msi->nextElement().getPointer());

//
//     /**
//      * Parses SEDML from a specified URL, using the specified relative URL.
//      */
//     SEDMLElement parseSEDMLFromURI(in wstring uri, in wstring relativeTo);
  es = already_AddRefd<iface::SProS::SEDMLElement>(sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
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
#define TRIVIAL_SEDML \
  L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"      \
  L"<sedML xmlns=\"http://www.biomodels.net/sed-ml\">\n"    \
  L"</sedML>\n"
  es = already_AddRefd<iface::SProS::SEDMLElement>(sb->parseSEDMLFromText(TRIVIAL_SEDML, L""));
  ms = already_AddRefd<iface::SProS::ModelSet>(es->models());
  msi = already_AddRefd<iface::SProS::BaseIterator>(ms->iterateElements());
  CPPUNIT_ASSERT(NULL == msi->nextElement().getPointer());

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
  RETURN_INTO_WSTRING(st, sb->sedmlToText(es));
  RETURN_INTO_OBJREF(cb, iface::cellml_api::CellMLBootstrap,
                     CreateCellMLBootstrap());
  RETURN_INTO_OBJREF(ul, iface::cellml_api::DOMURLLoader,
                     cb->localURLLoader());
  RETURN_INTO_OBJREF(doc, iface::dom::Document,
                     ul->loadDocumentFromText(st.c_str()));
  RETURN_INTO_OBJREF(de, iface::dom::Element,
                     doc->documentElement());

  es = already_AddRefd<iface::SProS::SEDMLElement>(sb->makeSEDMLFromElement(de));
  ms = already_AddRefd<iface::SProS::ModelSet>(es->models());
  msi = already_AddRefd<iface::SProS::BaseIterator>(ms->iterateElements());
  CPPUNIT_ASSERT(NULL == msi->nextElement().getPointer());
//   };
}
//
//   /**
//    * Base is implemented by all types of element in SEDML.
//    */
//   interface Base
//     : XPCOM::IObject
//   {

void
SProSTest::testSProSBase()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement, sb->createEmptySEDML());
  CPPUNIT_ASSERT(es);
//     /**
//      * The underlying DOM element.
//      */
//     readonly attribute dom::Element domElement;
  RETURN_INTO_OBJREF(domel, iface::dom::Element, es->domElement());
  CPPUNIT_ASSERT(NULL != domel);
  RETURN_INTO_WSTRING(domelln, domel->localName());
  CPPUNIT_ASSERT(domelln == L"sedML");

  RETURN_INTO_OBJREF(doc, iface::dom::Document, domel->ownerDocument());
  RETURN_INTO_OBJREF(noteEl, iface::dom::Element,
                     doc->createElementNS(SEDML_NS, L"notes"));
  RETURN_INTO_OBJREF(noteText, iface::dom::Text,
                     doc->createTextNode(L"Hello world"));
  noteEl->appendChild(noteText)->release_ref();
  domel->appendChild(noteEl)->release_ref();
  RETURN_INTO_OBJREF(annotationEl, iface::dom::Element,
                     doc->createElementNS(SEDML_NS, L"annotations"));
  RETURN_INTO_OBJREF(annotationText, iface::dom::Text,
                     doc->createTextNode(L"Hello world"));
  annotationEl->appendChild(annotationText)->release_ref();
  domel->appendChild(annotationEl)->release_ref();

//
//     /**
//      * The list of all note elements associated with this element.
//      */
//     readonly attribute dom::NodeList notes;
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList, es->notes());
  CPPUNIT_ASSERT_EQUAL(1, (int)nl->length());
  RETURN_INTO_OBJREF(item0, iface::dom::Node, nl->item(0));
  CPPUNIT_ASSERT(!CDA_objcmp(item0, noteText));

//
//     /**
//      * The list of all annotations associated with this element.
//      */
//     readonly attribute dom::NodeList annotations;
  nl = already_AddRefd<iface::dom::NodeList>(es->annotations());
  CPPUNIT_ASSERT_EQUAL(1, (int)nl->length());
  item0 = already_AddRefd<iface::dom::Node>(nl->item(0));
  CPPUNIT_ASSERT(!CDA_objcmp(item0, annotationText));
}
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

void
SProSTest::testSProSBaseSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet,
                     es->models());

//     /**
//      * Obtain an iterator for iterating through the elements.
//      */
//     BaseIterator iterateElements();
  RETURN_INTO_OBJREF(it1, iface::SProS::BaseIterator,
                     ms->iterateElements());
  RETURN_INTO_OBJREF(it2, iface::SProS::BaseIterator,
                     ms->iterateElements());
  CPPUNIT_ASSERT(it1);
  CPPUNIT_ASSERT(it2);

  RETURN_INTO_OBJREF(firstBase, iface::SProS::Base, it1->nextElement());
  CPPUNIT_ASSERT(firstBase);

//
//     /**
//      * Inserts an element into the set. Raises an exception if the element is
//      * already in another set, or was created for the wrong document.
//      */
//     void insert(in Base b) raises(SProSException);
  RETURN_INTO_OBJREF(newModel, iface::SProS::Model, es->createModel());
  ms->insert(newModel);

  RETURN_INTO_OBJREF(nextBase, iface::SProS::Base, it1->nextElement());
  CPPUNIT_ASSERT(!CDA_objcmp(nextBase, newModel));

//
//     /**
//      * Removes an element from the set.
//      */
//     void remove(in Base b);
  ms->remove(firstBase);
  RETURN_INTO_OBJREF(firstBase2, iface::SProS::Base, it2->nextElement());
  CPPUNIT_ASSERT(!CDA_objcmp(firstBase2, nextBase));
}
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
         // Tested above.
//   };
//
//   /**
//    * The top-level SEDML element.
//    */
//   interface SEDMLElement
//     : Base
//   {
void
SProSTest::testSProSSEDMLElement()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));

//     /**
//      * The SEDML level; defaults to the latest level supported by the
//      * implementation.
//      */
//     attribute unsigned long level;
  CPPUNIT_ASSERT_EQUAL(1, (int)es->level());
  es->level(2);
  CPPUNIT_ASSERT_EQUAL(2, (int)es->level());

//
//     /**
//      * The SEDML version; defaults to the latest version of SEDML supported by
//      * the implementation.
//      */
//     attribute unsigned long version;
  CPPUNIT_ASSERT_EQUAL(1, (int)es->version());
  es->version(2);
  CPPUNIT_ASSERT_EQUAL(2, (int)es->version());

//
//     /**
//      * The set of all models.
//      */
//     readonly attribute ModelSet models;
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet, es->models());
  CPPUNIT_ASSERT(ms);
  RETURN_INTO_OBJREF(msi, iface::SProS::ModelIterator, ms->iterateModels());
  RETURN_INTO_OBJREF(m, iface::SProS::Model, msi->nextModel());
  CPPUNIT_ASSERT(m);
  RETURN_INTO_WSTRING(modelId, m->id());
  CPPUNIT_ASSERT(modelId == L"model1");

//
//     /**
//      * The set of all tasks.
//      */
//     readonly attribute TaskSet tasks;
  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet, es->tasks());
  CPPUNIT_ASSERT(ts);
  RETURN_INTO_OBJREF(tsi, iface::SProS::TaskIterator, ts->iterateTasks());
  RETURN_INTO_OBJREF(t, iface::SProS::AbstractTask, tsi->nextTask());
  CPPUNIT_ASSERT(t);
  RETURN_INTO_WSTRING(taskId, t->id());
  CPPUNIT_ASSERT(taskId == L"task1");

//
//     /**
//      * The set of all simulations.
//      */
//     readonly attribute SimulationSet simulations;
  RETURN_INTO_OBJREF(ss, iface::SProS::SimulationSet, es->simulations());
  CPPUNIT_ASSERT(ss);
  RETURN_INTO_OBJREF(ssi, iface::SProS::SimulationIterator, ss->iterateSimulations());
  RETURN_INTO_OBJREF(s, iface::SProS::Simulation, ssi->nextSimulation());
  CPPUNIT_ASSERT(s);
  RETURN_INTO_WSTRING(simulationId, s->id());
  CPPUNIT_ASSERT(simulationId == L"simulation1");

//
//     /**
//      * The set of all generators.
//      */
//     readonly attribute DataGeneratorSet generators;
  RETURN_INTO_OBJREF(gs, iface::SProS::DataGeneratorSet, es->generators());
  CPPUNIT_ASSERT(gs);
  RETURN_INTO_OBJREF(gsi, iface::SProS::DataGeneratorIterator, gs->iterateDataGenerators());
  RETURN_INTO_OBJREF(g, iface::SProS::DataGenerator, gsi->nextDataGenerator());
  CPPUNIT_ASSERT(g);
  RETURN_INTO_WSTRING(generatorId, g->id());
  CPPUNIT_ASSERT(generatorId == L"time");

//
//     /**
//      * The set of all outputs.
//      */
//     readonly attribute OutputSet outputs;
  RETURN_INTO_OBJREF(os, iface::SProS::OutputSet, es->outputs());
  CPPUNIT_ASSERT(os);
  RETURN_INTO_OBJREF(osi, iface::SProS::OutputIterator, os->iterateOutputs());
  RETURN_INTO_OBJREF(o, iface::SProS::Output, osi->nextOutput());
  CPPUNIT_ASSERT(o);
  RETURN_INTO_WSTRING(outputId, o->id());
  CPPUNIT_ASSERT(outputId == L"plot1");

//
//     /**
//      * Creates a new Model. It is created with no parent and must be
//      * explicitly added.
//      */
//     Model createModel();
  {
    RETURN_INTO_OBJREF(nm, iface::SProS::Model, es->createModel());
    CPPUNIT_ASSERT(nm);
  }

//
//     /**
//      * Creates a new UniformTimeCourse. It is created with no parent and must be
//      * explicitly added.
//      */
//     UniformTimeCourse createUniformTimeCourse();
  {
    RETURN_INTO_OBJREF(utc, iface::SProS::UniformTimeCourse,
                       es->createUniformTimeCourse());
    CPPUNIT_ASSERT(utc);
  }

//
//     /**
//      * Creates a new Task. It is created with no parent and must be
//      * explicitly added.
//      */
//     Task createTask();
  {
    RETURN_INTO_OBJREF(nt, iface::SProS::Task,
                       es->createTask());
    CPPUNIT_ASSERT(nt);
  }

//
//     /**
//      * Creates a new DataGenerator. It is created with no parent and must be
//      * explicitly added.
//      */
//     DataGenerator createDataGenerator();
  {
    RETURN_INTO_OBJREF(dg, iface::SProS::DataGenerator,
                       es->createDataGenerator());
    CPPUNIT_ASSERT(dg);
  }

//
//     /**
//      * Creates a new Plot2D. It is created with no parent and must be
//      * explicitly added.
//      */
//     Plot2D createPlot2D();
  {
    RETURN_INTO_OBJREF(p2d, iface::SProS::Plot2D,
                       es->createPlot2D());
    CPPUNIT_ASSERT(p2d);
  }

//
//     /**
//      * Creates a new Plot3D. It is created with no parent and must be
//      * explicitly added.
//      */
//     Plot3D createPlot3D();
  {
    RETURN_INTO_OBJREF(p3d, iface::SProS::Plot3D,
                       es->createPlot3D());
    CPPUNIT_ASSERT(p3d);
  }

//
//     /**
//      * Creates a new Report. It is created with no parent and must be
//      * explicitly added.
//      */
//     Report createReport();
  {
    RETURN_INTO_OBJREF(r, iface::SProS::Report,
                       es->createReport());
    CPPUNIT_ASSERT(r);
  }

//
//     /**
//      * Creates a new ComputeChange. It is created with no parent and must be
//      * explicitly added.
//      */
//     ComputeChange createComputeChange();
  {
    RETURN_INTO_OBJREF(cc, iface::SProS::ComputeChange,
                       es->createComputeChange());
    CPPUNIT_ASSERT(cc);
  }

//
//     /**
//      * Creates a new ChangeAttribute. It is created with no parent and must be
//      * explicitly added.
//      */
//     ChangeAttribute createChangeAttribute();
  {
    RETURN_INTO_OBJREF(ca, iface::SProS::ChangeAttribute,
                       es->createChangeAttribute());
    CPPUNIT_ASSERT(ca);
  }

//
//     /**
//      * Creates a new AddXML. It is created with no parent and must be
//      * explicitly added.
//      */
//     AddXML createAddXML();
  {
    RETURN_INTO_OBJREF(ax, iface::SProS::AddXML,
                       es->createAddXML());
    CPPUNIT_ASSERT(ax);
  }

//
//     /**
//      * Creates a new RemoveXML. It is created with no parent and must be
//      * explicitly added.
//      */
//     RemoveXML createRemoveXML();
  {
    RETURN_INTO_OBJREF(rx, iface::SProS::RemoveXML,
                       es->createRemoveXML());
    CPPUNIT_ASSERT(rx);
  }

//
//     /**
//      * Creates a new ChangeXML. It is created with no parent and must be
//      * explicitly added.
//      */
//     ChangeXML createChangeXML();
  {
    RETURN_INTO_OBJREF(cx, iface::SProS::ChangeXML,
                       es->createChangeXML());
    CPPUNIT_ASSERT(cx);
  }

//
//     /**
//      * Creates a new Variable. It is created with no parent and must be
//      * explicitly added.
//      */
//     Variable createVariable();
  {
    RETURN_INTO_OBJREF(var, iface::SProS::Variable,
                       es->createVariable());
    CPPUNIT_ASSERT(var);
  }

//
//     /**
//      * Creates a new Parameter. It is created with no parent and must be
//      * explicitly added.
//      */
//     Parameter createParameter();
  {
    RETURN_INTO_OBJREF(p, iface::SProS::Parameter,
                       es->createParameter());
    CPPUNIT_ASSERT(p);
  }

//
//     /**
//      * Creates a new Curve. It is created with no parent and must be
//      * explicitly added.
//      */
//     Curve createCurve();
  {
    RETURN_INTO_OBJREF(curve, iface::SProS::Curve,
                       es->createCurve());
    CPPUNIT_ASSERT(curve);
  }

//
//     /**
//      * Creates a new Surface. It is created with no parent and must be
//      * explicitly added.
//      */
//     Surface createSurface();
  {
    RETURN_INTO_OBJREF(surf, iface::SProS::Surface,
                       es->createSurface());
    CPPUNIT_ASSERT(surf);
  }

//
//     /**
//      * Creates a new DataSet. It is created with no parent and must be
//      * explicitly added.
//      */
//     DataSet createDataSet();
  {
    RETURN_INTO_OBJREF(ds, iface::SProS::DataSet,
                       es->createDataSet());
    CPPUNIT_ASSERT(ds);
  }
}
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
          // Tested below.
//   };
//
//   /**
//    * The base type for all sets of elements with name attributes.
//    */
//   interface NamedElementSet
//     : BaseSet
//   {

void
SProSTest::testSProSNamedElementSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(os, iface::SProS::OutputSet,
                     es->outputs());

//     /**
//      * Iterate through all named elements.
//      */
//     NamedElementIterator iterateNamedElement();
  RETURN_INTO_OBJREF(nei, iface::SProS::NamedElementIterator,
                     os->iterateNamedElement());
  CPPUNIT_ASSERT(nei);

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
  RETURN_INTO_OBJREF(ne1, iface::SProS::NamedElement, nei->nextNamedElement());
  CPPUNIT_ASSERT(ne1);
  {
    RETURN_INTO_WSTRING(n, ne1->name());
    CPPUNIT_ASSERT(n == L"BioModel 3");
  }
  ne1->name(L"BioModel 4");
  {
    RETURN_INTO_WSTRING(n, ne1->name());
    CPPUNIT_ASSERT(n == L"BioModel 4");
  }

  nei->nextNamedElement()->release_ref();
  nei->nextNamedElement()->release_ref();
  CPPUNIT_ASSERT(nei->nextNamedElement().getPointer() == NULL);
//   };
}

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
         // Tested below...
//   };
//
//   /**
//    * The base type for all sets of named, identified elements.
//    */
//   interface NamedIdentifiedElementSet
//     : NamedElementSet
//   {
void
SProSTest::testSProSNamedIdentifiedElementSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(dgs, iface::SProS::NamedIdentifiedElementSet,
                     es->generators());

//     /**
//      * Iterates through all the named, identified elements...
//      */
//     NamedIdentifiedElementIterator iterateNamedIdentifiedElements();
  RETURN_INTO_OBJREF(niei, iface::SProS::NamedIdentifiedElementIterator,
                     dgs->iterateNamedIdentifiedElements());

//
//     /**
//      * Finds an element by identifier
//      */
//     NamedIdentifiedElement getNamedIdentifiedElementByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(nie1, iface::SProS::NamedIdentifiedElement,
                     dgs->getNamedIdentifiedElementByIdentifier(L"time"));
  CPPUNIT_ASSERT(nie1);
  {
    RETURN_INTO_WSTRING(id1, nie1->id());
    CPPUNIT_ASSERT(id1 == L"time");
  }
  nie1->id(L"time2");
  {
    RETURN_INTO_WSTRING(id1, nie1->id());
    CPPUNIT_ASSERT(id1 == L"time2");
  }

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
  RETURN_INTO_OBJREF(nie2, iface::SProS::NamedIdentifiedElement,
                     niei->nextNamedIdentifiedElement());
  CPPUNIT_ASSERT(nie2);
  RETURN_INTO_WSTRING(id2, nie2->id());
  CPPUNIT_ASSERT(id2 == L"time2");
  RETURN_INTO_OBJREF(nie3, iface::SProS::NamedIdentifiedElement,
                     niei->nextNamedIdentifiedElement());
  CPPUNIT_ASSERT(nie3);
  RETURN_INTO_WSTRING(id3, nie3->id());
  CPPUNIT_ASSERT(id3 == L"C1");

}
//   };
//
//   /**
//    * A set of models.
//    */
//   interface ModelSet
//     : NamedIdentifiedElementSet
//   {
void
SProSTest::testSProSModel()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet,
                     es->models());

//     /**
//      * Iterates through all models in the set.
//      */
//     ModelIterator iterateModels();
  RETURN_INTO_OBJREF(mi, iface::SProS::ModelIterator, ms->iterateModels());
  CPPUNIT_ASSERT(mi);

//
//     /**
//      * Search for a model in the set by identifier.
//      */
//     Model getModelByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(m1, iface::SProS::Model, ms->getModelByIdentifier(L"model1"));
  CPPUNIT_ASSERT(m1);

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
  RETURN_INTO_OBJREF(m2, iface::SProS::Model, mi->nextModel());
  CPPUNIT_ASSERT(!CDA_objcmp(m1, m2));

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
  {
    RETURN_INTO_WSTRING(lang, m1->language());
    CPPUNIT_ASSERT(lang == L"SBML");
  }
  m1->language(L"CellML");
  {
    RETURN_INTO_WSTRING(lang, m1->language());
    CPPUNIT_ASSERT(lang == L"CellML");
  }

//
//     /** The location of the model. */
//     attribute wstring source;
  {
    RETURN_INTO_WSTRING(src, m1->source());
    CPPUNIT_ASSERT(src == L"urn:miriam:biomodels.db:BIOMD0000000003");
  }
  m1->source(L"http://example.org/mymodel.xml");
  {
    RETURN_INTO_WSTRING(src, m1->source());
    CPPUNIT_ASSERT(src == L"http://example.org/mymodel.xml");
  }

//
//     /**
//      * The set of changes to make to the model.
//      */
//     readonly attribute ChangeSet changes;
  RETURN_INTO_OBJREF(cs, iface::SProS::ChangeSet,
                     m1->changes());
  CPPUNIT_ASSERT(cs);
//   };
}
//
//   /**
//    * A set of simulations.
//    */
//   interface SimulationSet
//     : NamedIdentifiedElementSet
//   {
void
SProSTest::testSProSSimulationSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(ss, iface::SProS::SimulationSet,
                     es->simulations());

//     /**
//      * Iterates through the simulations in the set.
//      */
//     SimulationIterator iterateSimulations();
  RETURN_INTO_OBJREF(ssi, iface::SProS::SimulationIterator, ss->iterateSimulations());
  CPPUNIT_ASSERT(ssi);

//
//     /**
//      * Finds a simulation in the set by identifier.
//      */
//     Simulation getSimulationByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(s1, iface::SProS::Simulation, ss->getSimulationByIdentifier(L"simulation1"));
  CPPUNIT_ASSERT(s1);

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
  RETURN_INTO_OBJREF(s2, iface::SProS::Simulation, ssi->nextSimulation());
  CPPUNIT_ASSERT(!CDA_objcmp(s1, s2));

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
  {
    RETURN_INTO_WSTRING(ksid, s1->algorithmKisaoID());
    CPPUNIT_ASSERT(ksid == L"KISAO:0000019");
  }
  s1->algorithmKisaoID(L"KISAO:12345");
  {
    RETURN_INTO_WSTRING(ksid, s1->algorithmKisaoID());
    CPPUNIT_ASSERT(ksid == L"KISAO:12345");
  }

//    /**
//     * The list of algorithm parameters.
//     */
//    readonly attribute AlgorithmParameterSet algorithmParameters;
  ObjRef<iface::SProS::AlgorithmParameterSet> params(s1->algorithmParameters());
  CPPUNIT_ASSERT(params != NULL);

  ObjRef<iface::SProS::AlgorithmParameterIterator> paramIt
    (params->iterateAlgorithmParameters());
  ObjRef<iface::SProS::AlgorithmParameter> firstParam(paramIt->nextAlgorithmParameter());
  CPPUNIT_ASSERT(firstParam != NULL);
  ObjRef<iface::SProS::AlgorithmParameter> nullParam(paramIt->nextAlgorithmParameter());
  CPPUNIT_ASSERT(nullParam == NULL);

//   };

//  /**
//   * An algorithm parameter
//   */
//  interface AlgorithmParameter
//    : Base
//  {
//    /**
//     * The KISAO ID of the parameter.
//     */
//    attribute wstring kisaoID;
  CPPUNIT_ASSERT(firstParam->kisaoID() == L"KISAO:0000211");
  firstParam->kisaoID(L"KISAO:0000212");
  CPPUNIT_ASSERT(firstParam->kisaoID() == L"KISAO:0000212");

//
//    /**
//     * The value of the parameter.
//     */
//    attribute wstring value;
  CPPUNIT_ASSERT(firstParam->value() == L"23");
  firstParam->value(L"24");
  CPPUNIT_ASSERT(firstParam->value() == L"24");

//  };

//
//   /**
//    * A Uniform Time Course simulation.
//    */
//   interface UniformTimeCourse
//     : Simulation
//   {
  DECLARE_QUERY_INTERFACE_OBJREF(utc, s1, SProS::UniformTimeCourse);
  CPPUNIT_ASSERT(utc);

//     /**
//      * The time the simulation starts.
//      */
//     attribute double initialTime;
  CPPUNIT_ASSERT_EQUAL(0.0, utc->initialTime());
  utc->initialTime(1.5);
  CPPUNIT_ASSERT_EQUAL(1.5, utc->initialTime());

//
//     /**
//      * The time of the first point to generate output for.
//      */
//     attribute double outputStartTime;
  CPPUNIT_ASSERT_EQUAL(0.0, utc->outputStartTime());
  utc->outputStartTime(1.5);
  CPPUNIT_ASSERT_EQUAL(1.5, utc->outputStartTime());

//
//     /**
//      * The time of the last point to generate output for.
//      */
//     attribute double outputEndTime;
  CPPUNIT_ASSERT_EQUAL(200.0, utc->outputEndTime());
  utc->outputEndTime(300.0);
  CPPUNIT_ASSERT_EQUAL(300.0, utc->outputEndTime());

//
//     /**
//      * The number of points of output to produce.
//      */
//     attribute unsigned long numberOfPoints;
  CPPUNIT_ASSERT_EQUAL(1000, (int)utc->numberOfPoints());
  utc->numberOfPoints(500);
  CPPUNIT_ASSERT_EQUAL(500, (int)utc->numberOfPoints());
}
//   };
//
//   /**
//    * A set of SEDML tasks.
//    */
//   interface TaskSet
//     : NamedIdentifiedElementSet
//   {
void
SProSTest::testSProSTaskSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(ts, iface::SProS::TaskSet,
                     es->tasks());

//     /**
//      * Iterate through the tasks in this set.
//      */
//     TaskIterator iterateTasks();
  RETURN_INTO_OBJREF(ti, iface::SProS::TaskIterator, ts->iterateTasks());
  CPPUNIT_ASSERT(ti != NULL);

//
//     /**
//      * Find a task in the set by the identifier.
//      */
//     AbstractTask getTaskByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(at1, iface::SProS::AbstractTask, ts->getTaskByIdentifier(L"task1"));
  CPPUNIT_ASSERT(at1 != NULL);

  RETURN_INTO_OBJREF(at2, iface::SProS::AbstractTask, ts->getTaskByIdentifier(L"task2"));
  CPPUNIT_ASSERT(at2 != NULL);

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
//     AbstractTask nextTask();
  RETURN_INTO_OBJREF(t3, iface::SProS::AbstractTask, ti->nextTask());
  RETURN_INTO_OBJREF(t4, iface::SProS::AbstractTask, ti->nextTask());
  CPPUNIT_ASSERT(!CDA_objcmp(at1, t3));
  CPPUNIT_ASSERT(!CDA_objcmp(at2, t4));

  CPPUNIT_ASSERT(ti->nextTask().getPointer() == NULL);

//   };
//
//   /**
//    * A SEDML task.
//    */
//   interface Task
//     : AbstractTask
  ObjRef<iface::SProS::Task> t1(QueryInterface(at1));
//   {
//     /**
//      * The referenced simulation, as an identifier.
//      */
//     attribute wstring simulationReferenceIdentifier;
  RETURN_INTO_WSTRING(sri, t1->simulationReferenceIdentifier());
  CPPUNIT_ASSERT(sri == L"simulation1");

//
//     /**
//      * The referenced simulation object.
//      */
//     attribute Simulation simulationReference;
  RETURN_INTO_OBJREF(sr, iface::SProS::Simulation, t1->simulationReference());
  CPPUNIT_ASSERT(sr);
  RETURN_INTO_WSTRING(sri2, sr->id());
  CPPUNIT_ASSERT(sri2 == L"simulation1");

  t1->simulationReferenceIdentifier(L"simulation2");
  CPPUNIT_ASSERT(!t1->simulationReference().getPointer());
  t1->simulationReference(sr);
  RETURN_INTO_WSTRING(sri3, t1->simulationReferenceIdentifier());
  CPPUNIT_ASSERT(sri3 == L"simulation1");

//
//     /**
//      * The referenced model, as an identifier.
//      */
//     attribute wstring modelReferenceIdentifier;
  RETURN_INTO_WSTRING(mri, t1->modelReferenceIdentifier());
  CPPUNIT_ASSERT(mri == L"model1");

//
//     /**
//      * The referenced model object.
//      */
//     attribute Model modelReference;
  RETURN_INTO_OBJREF(mr, iface::SProS::Model, t1->modelReference());
  CPPUNIT_ASSERT(mr);
  RETURN_INTO_WSTRING(mri2, mr->id());
  CPPUNIT_ASSERT(mri2 == L"model1");

  t1->modelReferenceIdentifier(L"model2");
  CPPUNIT_ASSERT(!t1->modelReference().getPointer());
  t1->modelReference(mr);
  RETURN_INTO_WSTRING(mri3, t1->modelReferenceIdentifier());
  CPPUNIT_ASSERT(mri3 == L"model1");

//   };

  ObjRef<iface::SProS::RepeatedTask> t2(QueryInterface(at2));
  CPPUNIT_ASSERT(t2 != NULL);
//  /**
//   * A SEDML RepeatedTask.
//   */
//  interface RepeatedTask
//    : AbstractTask
//  {
//    /**
//     * The referenced range, as an identifier.
//     */
//    attribute wstring rangeIdentifier;
  CPPUNIT_ASSERT(t2->rangeIdentifier() == L"index");

//
//    /**
//     * The referenced range object.
//     */
//    attribute Range rangeReference;
  ObjRef<iface::SProS::Range> indexRange(t2->rangeReference());
  CPPUNIT_ASSERT(indexRange != NULL);
  CPPUNIT_ASSERT(indexRange->id() == L"index");

  t2->rangeIdentifier(L"current");
  CPPUNIT_ASSERT(t2->rangeIdentifier() == L"current");
  ObjRef<iface::SProS::Range> funRange(t2->rangeReference());
  CPPUNIT_ASSERT(funRange->id() == L"current");
  t2->rangeIdentifier(L"invalid");
  CPPUNIT_ASSERT(t2->rangeIdentifier() == L"invalid");
  ObjRef<iface::SProS::Range> shouldBeNull(t2->rangeReference());
  CPPUNIT_ASSERT(shouldBeNull == NULL);
  t2->rangeReference(indexRange);
  CPPUNIT_ASSERT(t2->rangeIdentifier() == L"index");

//
//    /**
//     * Whether or not to reset the model.
//     */
//    attribute boolean resetModel;
  CPPUNIT_ASSERT_EQUAL(false, t2->resetModel());
  t2->resetModel(true);
  CPPUNIT_ASSERT_EQUAL(true, t2->resetModel());
  t2->resetModel(false);
  CPPUNIT_ASSERT_EQUAL(false, t2->resetModel());

//
//    /**
//     * The list of ranges over which to repeat.
//     */
//    readonly attribute RangeSet ranges;
  ObjRef<iface::SProS::RangeSet> allRanges(t2->ranges());
  CPPUNIT_ASSERT(allRanges != NULL);

  ObjRef<iface::SProS::Range> lookedUpRange(allRanges->getRangeByIdentifier(L"current"));
  CPPUNIT_ASSERT(!CDA_objcmp(lookedUpRange, funRange));

  ObjRef<iface::SProS::RangeIterator> rangeIt(allRanges->iterateRanges());
  ObjRef<iface::SProS::Range> range1(rangeIt->nextRange());
  CPPUNIT_ASSERT(range1 != NULL);
  ObjRef<iface::SProS::Range> range2(rangeIt->nextRange());
  CPPUNIT_ASSERT(range2 != NULL);
  ObjRef<iface::SProS::Range> range3(rangeIt->nextRange());
  CPPUNIT_ASSERT(range3 != NULL);
  ObjRef<iface::SProS::Range> shouldBeNullRange(rangeIt->nextRange());
  CPPUNIT_ASSERT(shouldBeNullRange == NULL);
  CPPUNIT_ASSERT(!CDA_objcmp(range1, indexRange));
  CPPUNIT_ASSERT(!CDA_objcmp(range2, funRange));

//
//    /**
//     * The list of subTasks to perform.
//     */
//    readonly attribute SubTaskSet subTasks;
  ObjRef<iface::SProS::SubTaskSet> subTasks(t2->subTasks());
  ObjRef<iface::SProS::SubTaskIterator> subTaskIt(subTasks->iterateSubTasks());
  ObjRef<iface::SProS::SubTask> firstSubTask(subTaskIt->nextSubTask());
  CPPUNIT_ASSERT(firstSubTask != NULL);
  ObjRef<iface::SProS::SubTask> shouldBeNullSubTask(subTaskIt->nextSubTask());
  CPPUNIT_ASSERT(shouldBeNullSubTask == NULL);

  CPPUNIT_ASSERT(firstSubTask->taskIdentifier() == L"task1");

//
//    /**
//     * The list of SetValue changes to apply.
//     */
//    readonly attribute SetValueSet changes;
  ObjRef<iface::SProS::SetValueSet> taskChanges(t2->changes());
  ObjRef<iface::SProS::SetValueIterator> setValIt(taskChanges->iterateSetValues());
  ObjRef<iface::SProS::SetValue> firstSetValue(setValIt->nextSetValue());
  CPPUNIT_ASSERT(firstSetValue != NULL);
  CPPUNIT_ASSERT(firstSetValue->rangeIdentifier() == L"current");
  ObjRef<iface::SProS::SetValue> shouldBeNullSetValue(setValIt->nextSetValue());
  CPPUNIT_ASSERT(shouldBeNullSetValue == NULL);

//  };

//  /**
//   * A SED-ML functional range.
//   */
//  interface FunctionalRange
//    : Range
//  {
//    /**
//     * The name to make available in the MathML as the index.
//     */
//    attribute wstring indexName;
  ObjRef<iface::SProS::FunctionalRange> funcRange(QueryInterface(funRange));
  CPPUNIT_ASSERT(funcRange->indexName() == L"index");
  funcRange->indexName(L"test");
  CPPUNIT_ASSERT(funcRange->indexName() == L"test");

//
//    /**
//     * The list of all variables to make available to the MathML.
//     */
//    readonly attribute VariableSet variables;
  ObjRef<iface::SProS::VariableSet> funRangeVars(funcRange->variables());
  ObjRef<iface::SProS::VariableIterator> funRangeVarIt(funRangeVars->iterateVariables());
  ObjRef<iface::SProS::Variable> funRangeVar(funRangeVarIt->nextVariable());
  CPPUNIT_ASSERT(funRangeVar != NULL);
  ObjRef<iface::SProS::Variable> shouldBeNullVar(funRangeVarIt->nextVariable());
  CPPUNIT_ASSERT(shouldBeNullVar == NULL);

  CPPUNIT_ASSERT(funRangeVar->id() == L"val");

//
//    /**
//     * The MathML math element defining the next value from the range.
//     */
//    attribute mathml_dom::MathMLMathElement function;
  ObjRef<iface::mathml_dom::MathMLMathElement> frFunction(funcRange->function());
  CPPUNIT_ASSERT(frFunction != NULL);
  ObjRef<iface::mathml_dom::MathMLElement> frFuncContents(frFunction->getArgument(1));
  CPPUNIT_ASSERT(frFuncContents != NULL);
  CPPUNIT_ASSERT(frFuncContents->localName() == L"piecewise");
//  };
//
//  /**
//   * A SED-ML vector range.
//   */
//  interface VectorRange
//    : Range
  ObjRef<iface::SProS::VectorRange> vecRange(QueryInterface(range3));
//  {
//    /**
//     * Retrieves the number of values in the range.
//     */
//    readonly attribute long numberOfValues;
  CPPUNIT_ASSERT_EQUAL(3, vecRange->numberOfValues());

//
//    /**
//     * Gets the value of the VectorRange at the specified index.
//     * Indices start at zero. Returns 0 if index out of range.
//     */
//    double valueAt(in long index);
  CPPUNIT_ASSERT_EQUAL(1.0, vecRange->valueAt(0));
  CPPUNIT_ASSERT_EQUAL(4.0, vecRange->valueAt(1));
  CPPUNIT_ASSERT_EQUAL(10.0, vecRange->valueAt(2));

//
//    /**
//     * Sets the value of the VectorRange at the specified index.
//     * Indices start at zero.
//     */
//    void setValueAt(in long index, in double value);
  vecRange->setValueAt(0, 2.0);
  CPPUNIT_ASSERT_EQUAL(2.0, vecRange->valueAt(0));
  CPPUNIT_ASSERT_EQUAL(3, vecRange->numberOfValues());

//
//    /**
//     * Inserts a value before the index specified. If index is <= 0, inserts at
//     * the beginning; if index is >= numberOfValues, inserts at the end of the
//     * list.
//     */
//    void insertValueBefore(in long index, in double value);
  vecRange->insertValueBefore(0, 0.0);
  CPPUNIT_ASSERT_EQUAL(4, vecRange->numberOfValues());
  CPPUNIT_ASSERT_EQUAL(0.0, vecRange->valueAt(0));
  CPPUNIT_ASSERT_EQUAL(2.0, vecRange->valueAt(1));
  vecRange->insertValueBefore(1, 0.5);
  vecRange->insertValueBefore(100, 100.0);
  CPPUNIT_ASSERT_EQUAL(6, vecRange->numberOfValues());
  CPPUNIT_ASSERT_EQUAL(0.0, vecRange->valueAt(0));
  CPPUNIT_ASSERT_EQUAL(0.5, vecRange->valueAt(1));
  CPPUNIT_ASSERT_EQUAL(2.0, vecRange->valueAt(2));
  CPPUNIT_ASSERT_EQUAL(4.0, vecRange->valueAt(3));
  CPPUNIT_ASSERT_EQUAL(10.0, vecRange->valueAt(4));
  CPPUNIT_ASSERT_EQUAL(100.0, vecRange->valueAt(5));

//
//    /**
//     * Removes the value at the index specified, or does nothing if the index is
//     * out of range.
//     */
//    void removeValueAt(in long index);
  vecRange->removeValueAt(0);
  vecRange->removeValueAt(2);
  CPPUNIT_ASSERT_EQUAL(4, vecRange->numberOfValues());
  CPPUNIT_ASSERT_EQUAL(0.5, vecRange->valueAt(0));
  CPPUNIT_ASSERT_EQUAL(2.0, vecRange->valueAt(1));
  CPPUNIT_ASSERT_EQUAL(10.0, vecRange->valueAt(2));
  CPPUNIT_ASSERT_EQUAL(100.0, vecRange->valueAt(3));
//  };
//

//  interface UniformRange
  ObjRef<iface::SProS::UniformRange> uniRange(QueryInterface(range1));
  CPPUNIT_ASSERT(uniRange != NULL);
//    : Range
//  {
//    /**
//     * The point at which to start the simulation.
//     */
//    attribute double start;
  CPPUNIT_ASSERT_EQUAL(0.0, uniRange->start());
  uniRange->start(1.0);
  CPPUNIT_ASSERT_EQUAL(1.0, uniRange->start());

//
//    /**
//     * The point at which to end the simulation.
//     */
//    attribute double end;
  CPPUNIT_ASSERT_EQUAL(10.0, uniRange->end());
  uniRange->end(9.5);
  CPPUNIT_ASSERT_EQUAL(9.5, uniRange->end());

//
//    /**
//     * The number of points between start and end.
//     */
//    attribute long numberOfPoints;
  CPPUNIT_ASSERT_EQUAL(100, uniRange->numberOfPoints());
  uniRange->numberOfPoints(101);
  CPPUNIT_ASSERT_EQUAL(101, uniRange->numberOfPoints());
//  };
//
//  /**
//   * A SED-ML SetValue element.
//   */
//  interface SetValue
//    : Base
//  {
//    /**
//     * The XPath expression describing the variable target.
//     */
//    attribute wstring target;
  CPPUNIT_ASSERT(L"/sbml:sbml/sbml:model/sbml:listOfParameters/sbml:parameter[@id='J0_v0']" ==
                       firstSetValue->target());
  firstSetValue->target(L"/sbml:sbml/sbml:model/sbml:listOfParameters/sbml:parameter[@id='J0_v1']");
  CPPUNIT_ASSERT(L"/sbml:sbml/sbml:model/sbml:listOfParameters/sbml:parameter[@id='J0_v1']" ==
                       firstSetValue->target());

//
//    /**
//     * The referenced model, as an identifier.
//     */
//    attribute wstring modelReferenceIdentifier;
  CPPUNIT_ASSERT(firstSetValue->modelReferenceIdentifier() == L"");
  firstSetValue->modelReferenceIdentifier(L"model1");
  CPPUNIT_ASSERT(firstSetValue->modelReferenceIdentifier() == L"model1");

//
//    /**
//     * The referenced model object.
//     */
//    attribute Model modelReference;
  ObjRef<iface::SProS::Model> refModel(firstSetValue->modelReference());
  CPPUNIT_ASSERT(refModel != NULL);
  CPPUNIT_ASSERT(refModel->id() == L"model1");
  firstSetValue->modelReferenceIdentifier(L"");
  CPPUNIT_ASSERT(firstSetValue->modelReferenceIdentifier() == L"");
  ObjRef<iface::SProS::Model> nullRefModel(firstSetValue->modelReference());
  CPPUNIT_ASSERT(nullRefModel == NULL);
  firstSetValue->modelReference(refModel);
  ObjRef<iface::SProS::Model> refModel2(firstSetValue->modelReference());
  CPPUNIT_ASSERT(!CDA_objcmp(refModel, refModel2));

//
//    /**
//     * The referenced range, as an identifier.
//     */
//    attribute wstring rangeIdentifier;
  CPPUNIT_ASSERT(firstSetValue->rangeIdentifier() == L"current");
  firstSetValue->rangeIdentifier(L"index");
  CPPUNIT_ASSERT(firstSetValue->rangeIdentifier() == L"index");

//
//    /**
//     * The referenced range object.
//     */
//    attribute Range rangeReference;
  ObjRef<iface::SProS::Range> referencedRange(firstSetValue->rangeReference());
  CPPUNIT_ASSERT(!CDA_objcmp(referencedRange, uniRange));
  firstSetValue->rangeReference(vecRange);
  CPPUNIT_ASSERT(firstSetValue->rangeIdentifier() == L"vecrange");

//  };
//
//  /**
//   * A SED-ML SubTask
//   */
//  interface SubTask
//    : Base
//  {
//    /**
//     * The referenced task, as an identifier string.
//     */
//    attribute wstring taskIdentifier;

  CPPUNIT_ASSERT(firstSubTask->taskIdentifier() == L"task1");
  firstSubTask->taskIdentifier(L"task2");
  CPPUNIT_ASSERT(firstSubTask->taskIdentifier() == L"task2");
  firstSubTask->taskIdentifier(L"task1");

//
//    /**
//     * The referenced task.
//     */
//    attribute AbstractTask taskReference;

  ObjRef<iface::SProS::AbstractTask> refTask(firstSubTask->taskReference());
  CPPUNIT_ASSERT(refTask != NULL);
  CPPUNIT_ASSERT(refTask->id() == L"task1");
  firstSubTask->taskIdentifier(L"");
  ObjRef<iface::SProS::AbstractTask> nullRefTask(firstSubTask->taskReference());
  CPPUNIT_ASSERT(nullRefTask == NULL);
  firstSubTask->taskReference(refTask);
  ObjRef<iface::SProS::AbstractTask> refTask2(firstSubTask->taskReference());
  CPPUNIT_ASSERT(!CDA_objcmp(refTask, refTask2));

//
//    /**
//     * The order value of this SubTask.
//     */
//    attribute long order;
  firstSubTask->order(10);
  CPPUNIT_ASSERT_EQUAL(10, firstSubTask->order());
//  };
}
//   /**
//    * A set of DataGenerators.
//    */
//   interface DataGeneratorSet
//     : NamedIdentifiedElementSet
//   {
void
SProSTest::testSProSDataGeneratorSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(dgs, iface::SProS::DataGeneratorSet,
                     es->generators());

//     /**
//      * Iterate through the DataGenerators in this set.
//      */
//     DataGeneratorIterator iterateDataGenerators();
  RETURN_INTO_OBJREF(dgi, iface::SProS::DataGeneratorIterator,
                     dgs->iterateDataGenerators());
  CPPUNIT_ASSERT(dgi);

//
//     /**
//      * Find a DataGenerator by identfier.
//      */
//     DataGenerator getDataGeneratorByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(dg1, iface::SProS::DataGenerator,
                     dgs->getDataGeneratorByIdentifier(L"time"));
  CPPUNIT_ASSERT(dg1);

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
  RETURN_INTO_OBJREF(dg2, iface::SProS::DataGenerator,
                     dgi->nextDataGenerator());
  CPPUNIT_ASSERT(!CDA_objcmp(dg1, dg2));
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
  RETURN_INTO_OBJREF(pars, iface::SProS::ParameterSet, dg1->parameters());
  CPPUNIT_ASSERT(pars);

//
//     /**
//      * The set of variables for this DataGenerator.
//      */
//     readonly attribute VariableSet variables;
  RETURN_INTO_OBJREF(vars, iface::SProS::VariableSet, dg1->variables());
  CPPUNIT_ASSERT(vars);
}
//   };
//
//   /**
//    * A set of Outputs
//    */
//   interface OutputSet
//     : NamedIdentifiedElementSet
//   {
void
SProSTest::testSProSOutputSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(os, iface::SProS::OutputSet,
                     es->outputs());
//     /**
//      * Iterates through all the outputs in this set.
//      */
//     OutputIterator iterateOutputs();
  RETURN_INTO_OBJREF(oi, iface::SProS::OutputIterator, os->iterateOutputs());

//
//     /**
//      * Finds an output in this set by identifier.
//      */
//     Output getOutputByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(o1, iface::SProS::Output, os->getOutputByIdentifier(L"plot1"));
  CPPUNIT_ASSERT(o1);
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
  RETURN_INTO_OBJREF(o2, iface::SProS::Output, oi->nextOutput());
  CPPUNIT_ASSERT(!CDA_objcmp(o1, o2));

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
//    * A SEDML 2D Plot output
//    */
//   interface Plot2D
//     : Output
//   {
  DECLARE_QUERY_INTERFACE_OBJREF(p2d, o1, SProS::Plot2D);
  CPPUNIT_ASSERT(p2d);

//     /**
//      * Gets the set of all curves.
//      */
//     readonly attribute CurveSet curves;
  RETURN_INTO_OBJREF(curveSet, iface::SProS::CurveSet, p2d->curves());
//   };
//
//   /**
//    * A SEDML 3D Plot output
//    */
//   interface Plot3D
//     : Output
//   {
  RETURN_INTO_OBJREF(o3, iface::SProS::Output, oi->nextOutput());
  DECLARE_QUERY_INTERFACE_OBJREF(p3d, o3, SProS::Plot3D);
  CPPUNIT_ASSERT(p3d);

//     /**
//      *  Gets the set of all surfaces.
//      */
//     readonly attribute SurfaceSet surfaces;
  RETURN_INTO_OBJREF(ss, iface::SProS::SurfaceSet, p3d->surfaces());
  CPPUNIT_ASSERT(ss);

//   };
//
//   /**
//    * A SEDML Report output
//    */
//   interface Report
//     : Output
//   {
  RETURN_INTO_OBJREF(o4, iface::SProS::Output, oi->nextOutput());
  DECLARE_QUERY_INTERFACE_OBJREF(rep, o4, SProS::Report);
  CPPUNIT_ASSERT(rep);

//     /**
//      * Gets the set of all DataSets
//      */
//     readonly attribute DataSetSet datasets;
  RETURN_INTO_OBJREF(ds, iface::SProS::DataSetSet, rep->datasets());
  CPPUNIT_ASSERT(ds);
//   };
}

//
//   /**
//    * A set of Changes
//    */
//   interface ChangeSet
//     : BaseSet
//   {
void
SProSTest::testSProSChangeSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(ms, iface::SProS::ModelSet,
                     es->models());
  RETURN_INTO_OBJREF(m, iface::SProS::Model,
                     ms->getModelByIdentifier(L"model1"));
  RETURN_INTO_OBJREF(cs, iface::SProS::ChangeSet,
                     m->changes());

//     /**
//      * Iterate through all the Changes in the set.
//      */
//     ChangeIterator iterateChanges();
  RETURN_INTO_OBJREF(ci, iface::SProS::ChangeIterator,
                     cs->iterateChanges());
  CPPUNIT_ASSERT(ci);
//   };
//
//   /**
//    * An iterator of Changes
//    */
//
//   interface ChangeIterator
//     : BaseIterator
//   {
//     /**
//      * Fetch the next Change, or null if there are no more.
//      */
//     Change nextChange();
  RETURN_INTO_OBJREF(c1, iface::SProS::Change, ci->nextChange());
//   };
//   /**
//    * A SEDML Change.
//    */
//   interface Change
//     : Base
//   {
//     attribute wstring target;

  {
    RETURN_INTO_WSTRING(t1, c1->target());
    CPPUNIT_ASSERT(t1 == L"foo");
  }

  c1->target(L"bar");

  {
    RETURN_INTO_WSTRING(t1, c1->target());
    CPPUNIT_ASSERT(t1 == L"bar");
  }

  RETURN_INTO_OBJREF(c2, iface::SProS::Change, ci->nextChange());
  RETURN_INTO_OBJREF(c3, iface::SProS::Change, ci->nextChange());
  RETURN_INTO_OBJREF(c4, iface::SProS::Change, ci->nextChange());
  RETURN_INTO_OBJREF(c5, iface::SProS::Change, ci->nextChange());

//   };
//
//   /**
//    * A SEDML ComputeChange style change.
//    */
//   interface ComputeChange
//     : Change, cellml_api::MathContainer
//   {
  DECLARE_QUERY_INTERFACE_OBJREF(cc, c5, SProS::ComputeChange);
  CPPUNIT_ASSERT(cc);

//     /**
//      * All variables involved.
//      */
//     readonly attribute VariableSet variables;
  RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet, cc->variables());
  CPPUNIT_ASSERT(vs);
  RETURN_INTO_OBJREF(v, iface::SProS::Variable,
                     vs->getVariableByIdentifier(L"time"));
  CPPUNIT_ASSERT(v);

//
//     /**
//      * All parameters involved.
//      */
//     readonly attribute ParameterSet parameters;
  RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, cc->parameters());
  CPPUNIT_ASSERT(ps);
  RETURN_INTO_OBJREF(p, iface::SProS::Parameter,
                     ps->getParameterByIdentifier(L"time"));
  CPPUNIT_ASSERT(p);

//   };
//
//   /**
//    * A SEDML attribute change.
//    */
//   interface ChangeAttribute
//     : Change
//   {
  DECLARE_QUERY_INTERFACE_OBJREF(ca, c1, SProS::ChangeAttribute);
  CPPUNIT_ASSERT(cc);

//     /**
//      * The new value of the attribute.
//      */
//     readonly attribute wstring newValue;
  RETURN_INTO_WSTRING(canv, ca->newValue());
  CPPUNIT_ASSERT(canv == L"bar");
//   };
//
//   /**
//    * A SEDML XML addition.
//    */
//   interface AddXML
//     : Change
//   {
  DECLARE_QUERY_INTERFACE_OBJREF(ax, c3, SProS::AddXML);
  CPPUNIT_ASSERT(ax);
//     /**
//      * The XML content to add.
//      */
//     readonly attribute dom::NodeList anyXML;
  RETURN_INTO_OBJREF(nl, iface::dom::NodeList, ax->anyXML());
  RETURN_INTO_OBJREF(it1, iface::dom::Node, nl->item(0));
  CPPUNIT_ASSERT(it1);
  DECLARE_QUERY_INTERFACE_OBJREF(tn1, it1, dom::Text);
  CPPUNIT_ASSERT(tn1);
  RETURN_INTO_WSTRING(dtn1d, tn1->data());
  CPPUNIT_ASSERT(dtn1d == L"Hello World");

//   };
//
//   /**
//    * A SEDML XML change.
//    */
//   interface ChangeXML
//     : AddXML
  DECLARE_QUERY_INTERFACE_OBJREF(cx, c2, SProS::ChangeXML);
  CPPUNIT_ASSERT(cx);
//   {
//   };
//
//   /**
//    * A SEDML XML removal.
//    */
//   interface RemoveXML
//     : Change
  DECLARE_QUERY_INTERFACE_OBJREF(rx, c4, SProS::RemoveXML);
  CPPUNIT_ASSERT(rx);
//   {
//   };
}

//
//   /**
//    * A SEDML Variable.
//    */
void
SProSTest::testSProSVariable()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(dgs, iface::SProS::DataGeneratorSet,
                     es->generators());
  RETURN_INTO_OBJREF(dgi, iface::SProS::DataGeneratorIterator, dgs->iterateDataGenerators());
  RETURN_INTO_OBJREF(dg, iface::SProS::DataGenerator, dgi->nextDataGenerator());
  RETURN_INTO_OBJREF(vs, iface::SProS::VariableSet, dg->variables());

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
  RETURN_INTO_OBJREF(vi, iface::SProS::VariableIterator, vs->iterateVariables());

//
//     /**
//      * Finds a variable in the set by identifier.
//      */
//     Variable getVariableByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(v_by_id, iface::SProS::Variable,
                     vs->getVariableByIdentifier(L"time"));
  CPPUNIT_ASSERT(v_by_id);

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
  RETURN_INTO_OBJREF(v, iface::SProS::Variable, vi->nextVariable());
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(v, v_by_id));
//   };

//   interface Variable
//     : NamedIdentifiedElement
//   {
//     attribute wstring target;
  {
    RETURN_INTO_WSTRING(t1, v->target());
    CPPUNIT_ASSERT(t1 == L"time");
  }

  v->target(L"time2");
  {
    RETURN_INTO_WSTRING(t1, v->target());
    CPPUNIT_ASSERT(t1 == L"time2");
  }

//     attribute wstring symbol;
  {
    RETURN_INTO_WSTRING(s1, v->symbol());
    CPPUNIT_ASSERT(s1 == L"timeSym");
  }
  v->symbol(L"timeSym2");
  {
    RETURN_INTO_WSTRING(s1, v->symbol());
    CPPUNIT_ASSERT(s1 == L"timeSym2");
  }
//   };
}
//
void
SProSTest::testSProSParameter()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(dgs, iface::SProS::DataGeneratorSet,
                     es->generators());
  RETURN_INTO_OBJREF(dgi, iface::SProS::DataGeneratorIterator, dgs->iterateDataGenerators());
  RETURN_INTO_OBJREF(dg, iface::SProS::DataGenerator, dgi->nextDataGenerator());
  RETURN_INTO_OBJREF(ps, iface::SProS::ParameterSet, dg->parameters());

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
  RETURN_INTO_OBJREF(pi, iface::SProS::ParameterIterator, ps->iterateParameters());

//
//     /**
//      * Finds a parameter in the set by identifier.
//      */
//     Parameter getParameterByIdentifier(in wstring idMatch);
  RETURN_INTO_OBJREF(p1, iface::SProS::Parameter, ps->getParameterByIdentifier(L"param1"));
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
  RETURN_INTO_OBJREF(p2, iface::SProS::Parameter, pi->nextParameter());
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(p1, p2));
//   };
//
//   /**
//    * A SEDML Parameter.
//    */
//   interface Parameter
//     : NamedIdentifiedElement
//   {
//     attribute double value;
  CPPUNIT_ASSERT(p1->value() == 1.234);
//   };
}
//

void
SProSTest::testSProSCurve()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(os, iface::SProS::OutputSet,
                     es->outputs());
  RETURN_INTO_OBJREF(o1, iface::SProS::Output,
                     os->getOutputByIdentifier(L"plot1"));
  DECLARE_QUERY_INTERFACE_OBJREF(p2d, o1, SProS::Plot2D);
  RETURN_INTO_OBJREF(cs, iface::SProS::CurveSet, p2d->curves());
  CPPUNIT_ASSERT(cs);

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
  RETURN_INTO_OBJREF(ci, iface::SProS::CurveIterator, cs->iterateCurves());
  CPPUNIT_ASSERT(ci);

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
  RETURN_INTO_OBJREF(c, iface::SProS::Curve, ci->nextCurve());
  CPPUNIT_ASSERT(c);

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
  CPPUNIT_ASSERT_EQUAL(true, c->logX());
  c->logX(false);
  CPPUNIT_ASSERT_EQUAL(false, c->logX());

//
//     /**
//      * If true, log-transform the Y axis.
//      */
//     attribute boolean logY;
  CPPUNIT_ASSERT_EQUAL(true, c->logY());
  c->logY(false);
  CPPUNIT_ASSERT_EQUAL(false, c->logY());

//
//     /**
//      * The identifier of the X-axis data generator.
//      */
//     attribute wstring xDataGeneratorID;
  {
    RETURN_INTO_WSTRING(xdgid, c->xDataGeneratorID());
    CPPUNIT_ASSERT(xdgid == L"time");
  }
  c->xDataGeneratorID(L"X1");
  {
    RETURN_INTO_WSTRING(xdgid, c->xDataGeneratorID());
    CPPUNIT_ASSERT(xdgid == L"X1");
  }

//
//     /**
//      * The identifier of the Y-axis data generator.
//      */
//     attribute wstring yDataGeneratorID;
  {
    RETURN_INTO_WSTRING(ydgid, c->yDataGeneratorID());
    CPPUNIT_ASSERT(ydgid == L"C1");
  }
  c->yDataGeneratorID(L"M1");
  {
    RETURN_INTO_WSTRING(ydgid, c->yDataGeneratorID());
    CPPUNIT_ASSERT(ydgid == L"M1");
  }

//
//     /**
//      * The X-axis data generator (if it can be found, otherwise
//      * null).
//      */
//     attribute DataGenerator xDataGenerator;
  RETURN_INTO_OBJREF(x1dg, iface::SProS::DataGenerator, c->xDataGenerator());
  RETURN_INTO_WSTRING(x1dgn, x1dg->name());
  CPPUNIT_ASSERT(x1dgn == L"X1");

//
//     /**
//      * The Y-axis data generator (if it can be found, otherwise
//      * null).
//      */
//     attribute DataGenerator yDataGenerator;
  RETURN_INTO_OBJREF(m1dg, iface::SProS::DataGenerator, c->yDataGenerator());
  RETURN_INTO_WSTRING(m1dgn, m1dg->name());
  CPPUNIT_ASSERT(m1dgn == L"M1");

  c->xDataGenerator(m1dg);
  c->yDataGenerator(x1dg);

  RETURN_INTO_OBJREF(m1dg2, iface::SProS::DataGenerator, c->xDataGenerator());
  RETURN_INTO_WSTRING(m1dg2n, m1dg2->name());
  CPPUNIT_ASSERT(m1dg2n == L"M1");

  RETURN_INTO_OBJREF(x1dg2, iface::SProS::DataGenerator, c->yDataGenerator());
  RETURN_INTO_WSTRING(x1dg2n, x1dg2->name());
  CPPUNIT_ASSERT(x1dg2n == L"X1");

//   };
}

void
SProSTest::testSProSSurface()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(os, iface::SProS::OutputSet,
                     es->outputs());
  RETURN_INTO_OBJREF(o1, iface::SProS::Output,
                     os->getOutputByIdentifier(L"plot2"));
  DECLARE_QUERY_INTERFACE_OBJREF(p3d, o1, SProS::Plot3D);
  CPPUNIT_ASSERT(p3d);
  RETURN_INTO_OBJREF(ss, iface::SProS::SurfaceSet, p3d->surfaces());

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
  RETURN_INTO_OBJREF(si, iface::SProS::SurfaceIterator, ss->iterateSurfaces());
  CPPUNIT_ASSERT(si);

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
  RETURN_INTO_OBJREF(s, iface::SProS::Surface, si->nextSurface());
  CPPUNIT_ASSERT(s);
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
  CPPUNIT_ASSERT_EQUAL(true, s->logZ());
  s->logZ(false);
  CPPUNIT_ASSERT_EQUAL(false, s->logZ());

//
//     /**
//      * The identifier of the Z-axis data generator.
//      */
//     attribute wstring zDataGeneratorID;
  RETURN_INTO_OBJREF(zdg1, iface::SProS::DataGenerator,
                     s->zDataGenerator());
  {
    RETURN_INTO_WSTRING(zdgid, s->zDataGeneratorID());
    CPPUNIT_ASSERT(zdgid == L"M1");
  }
  s->zDataGeneratorID(L"X1");
  {
    RETURN_INTO_WSTRING(zdgid, s->zDataGeneratorID());
    CPPUNIT_ASSERT(zdgid == L"X1");
  }

//
//     /**
//      * The Z-axis data generator (if it can be found, otherwise
//      * null).
//      */
//     attribute DataGenerator zDataGenerator;
  RETURN_INTO_OBJREF(zdg2, iface::SProS::DataGenerator, s->zDataGenerator());
  RETURN_INTO_WSTRING(zdg2n, zdg2->name());
  CPPUNIT_ASSERT(zdg2n == L"X1");

  s->zDataGenerator(zdg1);

  RETURN_INTO_OBJREF(zdg3, iface::SProS::DataGenerator, s->zDataGenerator());
  RETURN_INTO_WSTRING(zdg3n, zdg3->name());
  CPPUNIT_ASSERT(zdg3n == L"M1");

//   };
}

void
SProSTest::testSProSDataSet()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(es, iface::SProS::SEDMLElement,
                     sb->parseSEDMLFromURI(L"sedml-example-1.xml", BASE_DIRECTORY));
  RETURN_INTO_OBJREF(os, iface::SProS::OutputSet,
                     es->outputs());
  RETURN_INTO_OBJREF(o1, iface::SProS::Output,
                     os->getOutputByIdentifier(L"report1"));
  DECLARE_QUERY_INTERFACE_OBJREF(r1, o1, SProS::Report);
  CPPUNIT_ASSERT(r1);

  RETURN_INTO_OBJREF(dss, iface::SProS::DataSetSet, r1->datasets());

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
  RETURN_INTO_OBJREF(dsi, iface::SProS::DataSetIterator, dss->iterateDataSets());
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
  RETURN_INTO_OBJREF(ds, iface::SProS::DataSet, dsi->nextDataSet());
//   };
// };
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
  RETURN_INTO_WSTRING(dgid1, ds->dataGeneratorID());
  CPPUNIT_ASSERT(dgid1 == L"time");

  RETURN_INTO_OBJREF(dgorig, iface::SProS::DataGenerator, ds->dataGen());

  ds->dataGeneratorID(L"X1");

  RETURN_INTO_WSTRING(dgid2, ds->dataGeneratorID());
  CPPUNIT_ASSERT(dgid2 == L"X1");

//
//     /**
//      * The data generator (if it can be found, otherwise
//      * null). It must already be in the main set of
//      * DataGenerators and have an ID.
//      */
//     attribute DataGenerator dataGen;

  RETURN_INTO_OBJREF(dgx1, iface::SProS::DataGenerator, ds->dataGen());
  RETURN_INTO_WSTRING(dgx1n, dgx1->name());
  CPPUNIT_ASSERT(dgx1n == L"X1");

//   };
//
}

void
SProSTest::testSProSOneStepSteadyState()
{
  RETURN_INTO_OBJREF(sb, iface::SProS::Bootstrap,
                     CreateSProSBootstrap());
  RETURN_INTO_OBJREF(sedml, iface::SProS::SEDMLElement,
                     sb->createEmptySEDML());

  RETURN_INTO_OBJREF(oneStep, iface::SProS::OneStep,
                     sedml->createOneStep());
  oneStep->step(1.0);
  CPPUNIT_ASSERT_EQUAL(1.0, oneStep->step());
  oneStep->step(2.5);
  CPPUNIT_ASSERT_EQUAL(2.5, oneStep->step());
  CPPUNIT_ASSERT(oneStep != NULL);

  RETURN_INTO_OBJREF(steadyState, iface::SProS::SteadyState,
                     sedml->createSteadyState());
  CPPUNIT_ASSERT(steadyState != NULL);
}
