#include "cda_compiler_support.h"
#include "CellMLTest.hpp"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

#define MATHML_NS L"http://www.w3.org/1998/Math/MathML"

CPPUNIT_TEST_SUITE_REGISTRATION( CellMLTest );

void
CellMLTest::setUp()
{
  mAchCascade = NULL;
  mBeelerReuter = NULL;
  mTenTusscher = NULL;
  mGlycolysis = NULL;
  mRelativeURI = NULL;
  mBootstrap = CreateCellMLBootstrap();
  mModelLoader = mBootstrap->modelLoader();
  mDOMImplementation = mBootstrap->domImplementation();
  mLocalURLLoader = mBootstrap->localURLLoader();
}

void
CellMLTest::tearDown()
{
  mBootstrap->release_ref();
  mModelLoader->release_ref();
  mDOMImplementation->release_ref();
  mLocalURLLoader->release_ref();
  if (mAchCascade)
    mAchCascade->release_ref();
  if (mBeelerReuter)
    mBeelerReuter->release_ref();
  if (mTenTusscher)
    mTenTusscher->release_ref();
  if (mGlycolysis)
    mGlycolysis->release_ref();
  if (mRelativeURI)
    mRelativeURI->release_ref();
}

void
CellMLTest::loadAchCascade()
{
  mAchCascade = mModelLoader->loadFromURL
    (BASE_DIRECTORY L"Ach_cascade_1995.xml");
}

void
CellMLTest::loadBeelerReuter()
{
  mBeelerReuter = mModelLoader->loadFromURL
    (BASE_DIRECTORY L"beeler_reuter_model_1977.xml");
}

void
CellMLTest::loadTenTusscher()
{
  mTenTusscher =
    mModelLoader->loadFromURL
    (L"http://www.cellml.org/Members/miller/andres_models/"
     L"2004_tenTusscher/experiments/periodic-stimulus-endocardial.xml");
}

void
CellMLTest::loadGlycolysis()
{
  mGlycolysis =
    mModelLoader->loadFromURL
    (BASE_DIRECTORY L"glycolysis_pathway_1997.xml");
}

void
CellMLTest::loadRelativeURIModel()
{
  mRelativeURI =
    mModelLoader->loadFromURL
    (BASE_DIRECTORY L"subdir1/subdir2/toplevel.xml");
  mRelativeURI->fullyInstantiateImports();
}

//   /**
//    * A DOM specific interface that provides everything an application needs to
//    * bootstrap the CellML API.
//    */
//   interface CellMLBootstrap
//     : XPCOM::IObject
//   {
void
CellMLTest::testCellMLBootstrap()
{
  CPPUNIT_ASSERT(mBootstrap);
//     /**
//      * The model loader used to load models.
//      */
//     readonly attribute DOMModelLoader modelLoader;
  CPPUNIT_ASSERT(mModelLoader);
//     /**
//      * The local DOMImplementation. This may not always be available, and when
//      * it isn't, nil should be returned.
//      */
//     readonly attribute dom::DOMImplementation domImplementation;
  CPPUNIT_ASSERT(mDOMImplementation);
//     /**
//      * The local URL loader. This may not always be available, and when it
//      * isn't, nil should be returned.
//      */
//     readonly attribute DOMURLLoader localURLLoader;
  CPPUNIT_ASSERT(mLocalURLLoader);

  // Now try creating a CellML model...
  iface::cellml_api::Model* m = mBootstrap->createModel(L"1.1");
  if (m != NULL)
    m->release_ref();
  CPPUNIT_ASSERT(m);
}
//   };

//   /**
//    * A DOM specific interface for loading a CellML model with more control.
//    */
//   interface DOMModelLoader
//     : ModelLoader
//   {
void
CellMLTest::testDOMModelLoader()
{
//     /**
//      * Loads the DOM for the model from the specified URL using the supplied
//      * DOMURLLoader. If further URLs need to be loaded(for example, to satisfy
//      * imports), the supplied loader will be used.
//      * @param url The URL from which to load.
//      * @param loader The URL loader to load the model and any imports needed.
//      * @return The loaded model.
//      * @raises CellMLException if there is an error loading the model or the
//      *           DOM document.
//      */
//     Model createFromDOM(in dom::DOMString url, in DOMURLLoader loader)
//       raises(CellMLException);
  iface::cellml_api::Model* m =
    mModelLoader->createFromDOM(BASE_DIRECTORY L"Ach_cascade_1995.xml",
                                mLocalURLLoader);
  CPPUNIT_ASSERT(m);
  // For now, just check the name to make sure we loaded the right model...
  std::wstring str = m->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade_1995"), str);
  m->release_ref();
}
//   };
//   /**
//    * An interface for loading a CellML model by URL.
//    */
//   interface ModelLoader
//     : XPCOM::IObject
//   {
void
CellMLTest::testModelLoader()
{
//     /**
//      * Loads a model from the given URL, using the local URL loader.
//      * Note that this operation may be unavailable(or restricted to certain
//      * URLs) due to security restrictions on some remotely hosted model
//      * loaders.
//      * @param URL The URL from which to load the model.
//      * @return The loaded CellML model.
//      * @raises CellMLException if the model cannot be loaded.
//      */
//     Model loadFromURL(in dom::DOMString URL)
//       raises(CellMLException);
  loadAchCascade();
  CPPUNIT_ASSERT(mAchCascade);
  std::wstring str = mAchCascade->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade_1995"), str);
  
//     /**
//      * An error message describing the cause of the last CellMLException.
//      * The error message is formatted code/param1/param2/.../paramn
//      * Valid error codes are...
//      * noperm (The user does not have permission to load URLs of this kind).
//      * badurl (The URL was malformed).
//      * servererror/info A server error occurred. Info is an optional, method
//      *   specific parameter, and for HTTP it should be the server error number.
//      * badxml/line/column/msg The XML was malformed. Line an column give the
//      *   location in the file at which the error occurred. msg is the message
//      *   from the processor stating what is wrong.
//      * notcellml The model did not have a cellml::model document element in
//      *   either the CellML 1.0 or CellML 1.1 namespace.
//      */
//     readonly attribute dom::DOMString lastErrorMessage;
  iface::cellml_api::Model* throwaway;
  CPPUNIT_ASSERT_THROW(throwaway=mModelLoader->loadFromURL(BASE_DIRECTORY L"dont_create_this_file.xml"),
                       iface::cellml_api::CellMLException);
  str = mModelLoader->lastErrorMessage();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"servererror"), str);


//    /**
//     * Creates a serialised XML document.
//     * @param xmlText A complete XML document, serialised into text.
//     * @return The loaded CellML model.
//     * @param CellMLException if the XML cannot be parsed into a model.
//     */
//    Model createFromText(in dom::DOMString xmlText)
//      raises(CellMLException);
  iface::cellml_api::Model* model = mModelLoader->createFromText
    (
L"<?xml version=\"1.0\"?>\n"
L"<model xmlns=\"http://www.cellml.org/cellml/1.0#\" />"
    );
  CPPUNIT_ASSERT(model);
  model->release_ref();
}
//   };
//   /**
//    * A DOM specific interface for loading a Document by URL.
//    */
//   interface DOMURLLoader
//     : XPCOM::IObject
//   {
void
CellMLTest::testDOMURLLoader()
{
//     /**
//      * Loads an XML formatted document from a given URL into a DOM. This may
//      * be restricted to certain URLs due to security restrictions on some
//      * remotely hosted URL loaders.
//      * @param URL The URL from which to load the document.
//      * @return The loaded document.
//      * @raises CellMLException if the model cannot be loaded.
//      */
//     dom::Document loadDocument(in dom::DOMString URL)
//       raises(CellMLException);
  iface::dom::Document* d =
    mLocalURLLoader->loadDocument(BASE_DIRECTORY L"Ach_cascade_1995.xml");
  CPPUNIT_ASSERT(d);
  iface::dom::Element* de = d->documentElement();
  CPPUNIT_ASSERT(de);
  std::wstring str = de->localName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"model"), str);
  de->release_ref();
  d->release_ref();
//     /**
//      * An error message describing the cause of the last CellMLException.
//      * The error message is formatted code/param1/param2/.../paramn
//      * Any error message the application desires may be placed in
//      * lastErrorMessage and it will be passed on to other API
//      * lastErrorMessage attributes.
//      *
//      * An API provided implementation may return the following error message
//      *  types:
//      * noperm (The user does not have permission to load URLs of this kind).
//      * badurl (The URL was malformed).
//      * servererror/info A server error occurred. Info is an optional, method
//      *   specific parameter, and for HTTP it should be the server error number.
//      * badxml/line/column/msg The XML was malformed. Line an column give the
//      *   location in the file at which the error occurred. msg is the message
//      *   from the processor stating what is wrong.
//      */
//     readonly attribute dom::DOMString lastErrorMessage;
  iface::dom::Document* throwaway;
  CPPUNIT_ASSERT_THROW(throwaway=mLocalURLLoader->loadDocument
                       (BASE_DIRECTORY L"dont_create_this_file.xml"),
                       iface::cellml_api::CellMLException);
  str = mLocalURLLoader->lastErrorMessage();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"servererror"), str);
}
//   };


//   interface RDFRepresentation : XPCOM::IObject
//   {
void
CellMLTest::testRDFRepresentation()
{
  loadAchCascade();
//     /**
//      * Returns the URN for the type of representation this is.
//      * An application may use the type to determine what specific instance of
//      * RDFRepresentation it may cast the object to.
//      */
//     readonly attribute wstring type;
  iface::cellml_api::RDFRepresentation* rr = NULL;
  CPPUNIT_ASSERT_NO_THROW
    (
     rr = mAchCascade->getRDFRepresentation(L"http://www.cellml.org/RDFXML/DOM")
    );
  CPPUNIT_ASSERT(rr);

  std::wstring str = rr->type();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.cellml.org/RDFXML/DOM"), str);
  rr->release_ref();

  CPPUNIT_ASSERT_NO_THROW
    (
     rr = mAchCascade->getRDFRepresentation(L"http://www.cellml.org/RDFXML/string")
    );
  CPPUNIT_ASSERT(rr);

  str = rr->type();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.cellml.org/RDFXML/string"), str);

  rr->release_ref();
}
//   };

//   /**
//    * The RDF representation for type="http://www.cellml.org/RDFXML/DOM"
//    */
//   interface RDFXMLDOMRepresentation : RDFRepresentation
//   {
void
CellMLTest::testRDFXMLDOMRepresentation()
{
  loadAchCascade();
//    /**
//     * A complete RDF/XML document (including a single top-level RDF:rdf
//     * element) as a DOM document. The document fetched is a copy of the RDF
//     * in the model (that is, it is not live). However, fetching the attribute
//     * again will update to the latest data. Assigning this attribute to
//     * a new document (or a changed version of the previous document) will
//     * replace all the RDF in the model with the contents of the new document.
//     */
//    attribute dom::Document data;

  iface::cellml_api::RDFRepresentation* rr = 0;
  CPPUNIT_ASSERT_NO_THROW
    (
     rr = mAchCascade->getRDFRepresentation(L"http://www.cellml.org/RDFXML/DOM")
    );
  CPPUNIT_ASSERT(rr);

  DECLARE_QUERY_INTERFACE_REPLACE(rrd, rr,
                                  cellml_api::RDFXMLDOMRepresentation);
  iface::dom::Document* doc = rrd->data();
  rrd->release_ref();

  iface::dom::Element* de = doc->documentElement();
  doc->release_ref();

  std::wstring str = de->localName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"RDF"), str);

  iface::dom::NodeList* nl = de->childNodes();
  de->release_ref();
  iface::dom::Node* n = nl->item(0);
  nl->release_ref();
  
  CPPUNIT_ASSERT(n);
  str = n->localName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Description"), str);

  DECLARE_QUERY_INTERFACE_REPLACE(el, n, dom::Element);

  str =
    el->getAttributeNS(L"http://www.w3.org/1999/02/22-rdf-syntax-ns#",
                       L"about");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L""), str);

  el->release_ref();
}
//   };

//   /**
//    * The RDF representation for type="http://www.cellml.org/RDFXML/string"
//    */
//   interface RDFXMLStringRepresentation : RDFRepresentation
//   {
void
CellMLTest::testRDFXMLStringRepresentation()
{
  loadAchCascade();
//     /**
//      * The RDF/XML as a serialised string.
//      */
//     attribute wstring serialisedData;
  iface::cellml_api::RDFRepresentation* rr = NULL;
  CPPUNIT_ASSERT_NO_THROW
    (
     rr = mAchCascade->getRDFRepresentation(L"http://www.cellml.org/RDFXML/string")
    );
  CPPUNIT_ASSERT(rr);

  DECLARE_QUERY_INTERFACE_REPLACE(rrs, rr,
                                  cellml_api::RDFXMLStringRepresentation);
  const std::wstring correctValue =
L"<?xml version=\"1.0\"?>\n"
L"<RDF xmlns=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"><rdf:Description xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" rdf:about=\"\">\n"
L"      <!--\n"
L"        The Model Builder Metadata.  The Dublin Core \"creator\" element is used  \n"
L"        to indicate the person who translated the model into CellML.\n"
L"      -->\n"
L"      <dc:creator xmlns:dc=\"http://purl.org/dc/elements/1.1/\" rdf:parseType=\"Resource\">\n"
L"        <vCard:N xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\" rdf:parseType=\"Resource\">\n"
L"          <vCard:Family>Lloyd</vCard:Family>\n"
L"          <vCard:Given>Catherine</vCard:Given>\n"
L"          <vCard:Other>May</vCard:Other>\n"
L"        </vCard:N>\n"
L"        <vCard:EMAIL xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\" rdf:parseType=\"Resource\">\n"
L"          <rdf:value>c.lloyd@auckland.ac.nz</rdf:value>\n"
L"          <rdf:type rdf:resource=\"http://imc.org/vCard/3.0#internet\"/>\n"
L"        </vCard:EMAIL>\n"
L"        <vCard:ORG xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\" rdf:parseType=\"Resource\">\n"
L"          <vCard:Orgname>The University of Auckland</vCard:Orgname>\n"
L"          <vCard:Orgunit>The Bioengineering Institute</vCard:Orgunit>\n"
L"        </vCard:ORG>\n"
L"      </dc:creator>\n"
L"      \n"
L"      <!--\n"
L"        The Creation Date metadata. This is the date on which the model\n"
L"        was translated into CellML.\n"
L"      -->\n"
L"      <dcterms:created xmlns:dcterms=\"http://purl.org/dc/terms/\" rdf:parseType=\"Resource\">\n"
L"        <dcterms:W3CDTF>2002-10-30</dcterms:W3CDTF>\n"
L"      </dcterms:created>\n"
L"      \n"
L"      <!--\n"
L"        The Modification History metadata. This lists the changes that have been\n"
L"        made to the document, who made the changes, and when they were made.\n"
L"      -->\n"
L"      <cmeta:modification xmlns:cmeta=\"http://www.cellml.org/metadata/1.0#\" rdf:parseType=\"Resource\">\n"
L"        <rdf:value>\n"
L"          Changed the model name so the model loads in the database easier.\n"
L"        </rdf:value>\n"
L"        <cmeta:modifier rdf:parseType=\"Resource\">\n"
L"          <vCard:N xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\" rdf:parseType=\"Resource\">\n"
L"            <vCard:Family>Cuellar</vCard:Family>\n"
L"            <vCard:Given>Autumn</vCard:Given>\n"
L"            <vCard:Other>A</vCard:Other>\n"
L"          </vCard:N>\n"
L"        </cmeta:modifier>\n"
L"        <dcterms:modified xmlns:dcterms=\"http://purl.org/dc/terms/\" rdf:parseType=\"Resource\"> \n"
L"          <dcterms:W3CDTF>2003-04-05</dcterms:W3CDTF>\n"
L"        </dcterms:modified>\n"
L"      </cmeta:modification>\n"
L"      \n"
L"      \n"
L"      <!-- The Publisher metadata. -->\n"
L"      <dc:publisher xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
L"        The University of Auckland, Bioengineering Institute\n"
L"      </dc:publisher>\n"
L"    </rdf:Description><rdf:Description xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" rdf:about=\"#Ach_cascade\">\n"
L"      <!-- A human readable name for the model. -->\n"
L"      <dc:title xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
L"        Wang and Lipsius's 1995 kinetic analysis of an acetylcholine induced \n"
L"        signal transduction cascade resulting in ATP-sensitive K+ channel \n"
L"        activation.\n"
L"      </dc:title>\n"
L"      \n"
L"      <!-- A comment regarding the model. -->\n"
L"      <cmeta:comment xmlns:cmeta=\"http://www.cellml.org/metadata/1.0#\" rdf:parseType=\"Resource\">\n"
L"        <rdf:value>\n"
L"          This is the CellML descripition of Wang and Lipsius's 1995 kinetic \n"
L"          analysis of an acetylcholine induced signal transduction cascade \n"
L"          resulting in ATP-sensitive K+ channel activation.\n"
L"        </rdf:value>\n"
L"        <!-- The creator of the comment. -->\n"
L"        <dc:creator xmlns:dc=\"http://purl.org/dc/elements/1.1/\" rdf:parseType=\"Resource\">\n"
L"          <vCard:FN xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\">Catherine Lloyd</vCard:FN>\n"
L"        </dc:creator>\n"
L"      </cmeta:comment>\n"
L"      \n"
L"      <!--  Keyword(s) -->\n"
L"      <bqs:reference xmlns:bqs=\"http://www.cellml.org/bqs/1.0#\" rdf:parseType=\"Resource\">\n"
L"        <dc:subject xmlns:dc=\"http://purl.org/dc/elements/1.1/\" rdf:parseType=\"Resource\">\n"
L"          <bqs:subject_type>keyword</bqs:subject_type>\n"
L"          <rdf:value>signal transduction</rdf:value>\n"
L"        </dc:subject>\n"
L"      </bqs:reference>\n"
L"      \n"
L"      <!-- \n"
L"        The CellML Metadata Specification recommends that bibliographic metadata\n"
L"        is used to provide information about the original model reference.  The \n"
L"        \"identifier\" attribute on the \"BibliographicReference\" class provides an\n"
L"        elegant way to identify a cited reference using a database identifier   \n"
L"        such as Pubmed.  All associated data such as author, journal title,     \n"
L"        date, etc can be looked up on the database.\n"
L"      -->\n"
L"      <bqs:reference xmlns:bqs=\"http://www.cellml.org/bqs/1.0#\" rdf:parseType=\"Resource\">\n"
L"        <bqs:Pubmed_id>7641326</bqs:Pubmed_id>\n"
L"        <bqs:JournalArticle rdf:parseType=\"Resource\">\n"
L"          <dc:creator xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
L"            <rdf:Seq>\n"
L"              <rdf:li rdf:parseType=\"Resource\">\n"
L"                <bqs:Person rdf:parseType=\"Resource\">\n"
L"                  <vCard:N xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\" rdf:parseType=\"Resource\">\n"
L"                    <vCard:Family>Wang</vCard:Family>\n"
L"                    <vCard:Given>Yong Gao</vCard:Given>\n"
L"                  </vCard:N>\n"
L"                </bqs:Person>\n"
L"              </rdf:li>\n"
L"              <rdf:li rdf:parseType=\"Resource\">\n"
L"                <bqs:Person rdf:parseType=\"Resource\">\n"
L"                  <vCard:N xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\" rdf:parseType=\"Resource\">\n"
L"                    <vCard:Family>Lipsius</vCard:Family>\n"
L"                    <vCard:Given>Stephen</vCard:Given>\n"
L"                    <vCard:Other>L</vCard:Other>\n"
L"                  </vCard:N>\n"
L"                </bqs:Person>\n"
L"              </rdf:li>\n"
L"            </rdf:Seq>\n"
L"          </dc:creator>\n"
L"          <dc:title xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
L"            beta-Adrenergic Stimulation Induces Acetylcholine to Activate \n"
L"            ATP-Sensitive K+ Current in Cat Atrial Myocytes\n"
L"          </dc:title>\n"
L"          <dcterms:issued xmlns:dcterms=\"http://purl.org/dc/terms/\" rdf:parseType=\"Resource\">\n"
L"            <dcterms:W3CDTF>1995-09</dcterms:W3CDTF>\n"
L"          </dcterms:issued>\n"
L"          <bqs:Journal rdf:parseType=\"Resource\">\n"
L"            <dc:title xmlns:dc=\"http://purl.org/dc/elements/1.1/\">Circulation Research</dc:title>\n"
L"          </bqs:Journal>\n"
L"          <bqs:volume>77</bqs:volume>\n"
L"          <bqs:first_page>565</bqs:first_page>\n"
L"          <bqs:last_page>574</bqs:last_page>\n"
L"        </bqs:JournalArticle>\n"
L"      </bqs:reference>\n"
L"    </rdf:Description></RDF>";

  std::wstring str = rrs->serialisedData();  
  CPPUNIT_ASSERT_EQUAL(std::wstring(correctValue), str);

  rrs->release_ref();
}

//   };

//   /**
//    * Represents a uniform resource indicator, conforming to RFC2396
//    * (http://rfc.net/rfc2396.html)
//    */
//   interface URI : XPCOM::IObject
//   {
void
CellMLTest::testURI()
{
  loadAchCascade();
//     /**
//      * A representation of this string as text.
//      */
//     attribute wstring asText;
  iface::cellml_api::URI* uri = mAchCascade->xmlBase();
  CPPUNIT_ASSERT(uri);

  std::wstring str = uri->asText();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.example.org/repository/Ach_cascade_1995.xml"), str);

  uri->asText(L"http://www.example.org/repository/some_other_example.xml");
  str = uri->asText();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.example.org/repository/some_other_example.xml"), str);

  mAchCascade->clearXMLBase();
  str = uri->asText();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L""), str);
  
  uri->release_ref();
}
//   };

class MyUserData
  : public iface::cellml_api::UserData
{
public:
  MyUserData()
  {
    refcount = 1;
  }

  virtual ~MyUserData()
  {
  }

  void add_ref()
    throw(std::exception&)
  {
    refcount++;
  }

  void release_ref()
    throw(std::exception&)
  {
    refcount--;
    if (refcount == 0)
      delete this;
  }

  std::string objid()
    throw(std::exception&)
  {
    return "4d77f9ec-22b6-4329-a427-76e52de494da";
  }

  void*
  query_interface(const std::string& className)
    throw(std::exception&)
  {
    if (className == "XPCOM::IObject")
      return static_cast<iface::XPCOM::IObject*>(this);
    else if (className == "cellml_api::UserData")
      return static_cast<iface::cellml_api::UserData*>(this);
    return NULL;
  }

  std::vector<std::string> supported_interfaces() throw()
  {
    std::vector<std::string> ret;
    ret.push_back("XPCOM::IObject");
    ret.push_back("cellml_api::UserData");
    return ret;
  }
private:
  int refcount;
};

//   /**
//    * This is a general interface from which all CellML elements inherit.
//    */
//   interface CellMLElement : XPCOM::IObject
//   {
void
CellMLTest::testCellMLElement()
{
  loadAchCascade();

//     /**
//      * The CellML version that this element corresponds to.
//      * Can be the string "1.0" or "1.1".
//      * Other values are reserved for future use.
//      */
//     readonly attribute CellMLAttributeString cellmlVersion;
  std::wstring str = mAchCascade->cellmlVersion();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"1.0"), str);
  // TODO: Make a 1.1 model for testing things.

//     /**
//      * This element's cmeta id(which may be defined on any CellML element)
//      */
//     attribute CellMLAttributeString cmetaId;
  str = mAchCascade->cmetaId();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade"), str);
  mAchCascade->cmetaId(L"Ach_cascade_test");
  str = mAchCascade->cmetaId();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade_test"), str);
  // Just so we don't mess up other tests...
  mAchCascade->cmetaId(L"Ach_cascade");

//     /**
//      * The RDF metadata associated with this element. An element must have a
//      * cmeta:id for any RDF to be able to refer to it.
//      * @param type The URN describing the type of RDF metadata. Implementations
//      *             are free to add new types by creating new type names at URNs
//      *             under their jurisdiction. New URNs under http://www.cellml.org
//      *             are reserved for future versions of this specification.
//      * @return The object containing the RDF representation. If no arcs are
//      *         defined, an empty RDF representation is returned. The object may
//      *         be cast in an application defined manner depending on the type
//      *         returned.
//      * @raises CellMLException if type isn't supported.
//      * All implementations must implement the following types:
//      *             http://www.cellml.org/RDFXML/string
//      *             http://www.cellml.org/RDFXML/DOM
//      */
//     RDFRepresentation getRDFRepresentation(in wstring type) raises(CellMLException);
  // Tested in the RDFRepresentation tests.

//     /**
//      * The collection of extension elements associated with this CellML
//      * element. The list is "live", and so it will automatically update
//      * as changes to the underlying DOM representation are made.
//      */
//     readonly attribute ExtensionElementList extensionElements;
  iface::cellml_api::ExtensionElementList* eel = mAchCascade->extensionElements();
  CPPUNIT_ASSERT_EQUAL(5, (int)eel->length());

//     /**
//      * Insert an element into this collection without disturbing the order of
//      * the existing elements.
//      * @param marker The element after which to insert the element. If this is
//      *               nil, then the insertion will be at the start of the list.
//      * @param newEl The new element to insert.
//      */
//     void insertExtensionElementAfter(in ExtensionElement marker, in ExtensionElement newEl);
  iface::dom::Element* el1 = eel->getAt(1);
  iface::dom::Node* newn = el1->cloneNode(true);
  DECLARE_QUERY_INTERFACE_REPLACE(newel, newn, dom::Element);
  newel->setAttributeNS(L"", L"ismodified", L"true");
  mAchCascade->insertExtensionElementAfter(el1, newel);

  CPPUNIT_ASSERT_EQUAL(6, (int)eel->length());

  el1->release_ref();

  // Now check that the new extension element is in the right place...
  el1 = eel->getAt(2);
  str = el1->getAttributeNS(L"", L"ismodified");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"true"), str);

//     /**
//      * Remove an extension element. If the element is not found,
//      * do nothing.
//      * @param x The element to remove.
//      */
//     void removeExtensionElement(in ExtensionElement x);
  mAchCascade->removeExtensionElement(el1);
  el1->release_ref();
  CPPUNIT_ASSERT_EQUAL(5, (int)eel->length());

  // Now try an insert at the end...
  el1 = eel->getAt(4);
  mAchCascade->insertExtensionElementAfter(el1, newel);
  CPPUNIT_ASSERT_EQUAL(6, (int)eel->length());
  el1->release_ref();
  el1 = eel->getAt(5);
  str = el1->getAttributeNS(L"", L"ismodified");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"true"), str);
  mAchCascade->removeExtensionElement(el1);
  el1->release_ref();

//     /**
//      * Equivalent to insertAt(last extension element, x)
//      * @param x The element to append.
//      */
//     void appendExtensionElement(in ExtensionElement x);
  mAchCascade->appendExtensionElement(newel);
  CPPUNIT_ASSERT_EQUAL(6, (int)eel->length());
  // Now check that the new extension element is in the right place...
  el1 = eel->getAt(5);
  str = el1->getAttributeNS(L"", L"ismodified");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"true"), str);
  mAchCascade->removeExtensionElement(el1);
  el1->release_ref();
  CPPUNIT_ASSERT_EQUAL(5, (int)eel->length());

//     /**
//      * Equivalent to insertAt(nil, x)
//      * @param x The element to prepend.
//      */
//     void prependExtensionElement(in ExtensionElement x);
  mAchCascade->prependExtensionElement(newel);
  CPPUNIT_ASSERT_EQUAL(6, (int)eel->length());
  // Now check that the new extension element is in the right place...
  el1 = eel->getAt(0);
  str = el1->getAttributeNS(L"", L"ismodified");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"true"), str);
  CPPUNIT_ASSERT_NO_THROW(mAchCascade->removeExtensionElement(el1));
  el1->release_ref();
  CPPUNIT_ASSERT_EQUAL(5, (int)eel->length());

//     /**
//      * Find an extension element, and if it is found, replace it
//      * with another element.
//      * @param x The element to find.
//      * @param y The element to replace x with.
//      */
//     void replaceExtensionElement(in ExtensionElement x, in ExtensionElement y);
  el1 = eel->getAt(0);
  CPPUNIT_ASSERT_NO_THROW(mAchCascade->replaceExtensionElement(el1, newel));
  CPPUNIT_ASSERT_EQUAL(5, (int)eel->length());
  el1->release_ref();
  el1 = eel->getAt(0);
  str = el1->getAttributeNS(L"", L"ismodified");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"true"), str);
  el1->release_ref();

//     /**
//      * Remove all extension elements from this element.
//      */
//     void clearExtensionElements();
  CPPUNIT_ASSERT_NO_THROW(mAchCascade->clearExtensionElements());
  CPPUNIT_ASSERT_EQUAL(0, (int)eel->length());

  // Test prepend when the list is empty...
  mAchCascade->prependExtensionElement(newel);
  CPPUNIT_ASSERT_EQUAL(1, (int)eel->length());
  // Now check that the new extension element is in the right place...
  el1 = eel->getAt(0);
  str = el1->getAttributeNS(L"", L"ismodified");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"true"), str);
  el1->release_ref();
  newel->release_ref();
  eel->release_ref();

//     /**
//      * Get a list of all the child CellMLElements in this element.
//      */
//     readonly attribute CellMLElementSet childElements;
  iface::cellml_api::CellMLElementSet* els = NULL;
  CPPUNIT_ASSERT_NO_THROW(els = mAchCascade->childElements());
  CPPUNIT_ASSERT_EQUAL(113, (int)els->length());

//     /**
//      * Add a CellML element to this element. Checking is performed to ensure
//      * that it is a type of element that is allowed in this type of element,
//      * and if it is not, an exception is raised. Checking is also performed to
//      * ensure that the element belongs in this Model, and that it is not
//      * already inserted into the document.
//      * @param x The CellMLElement to add.
//      * @raises CellMLException if the element cannot be added to this Element.
//      */
//     void addElement(in CellMLElement x) raises(CellMLException);
  iface::cellml_api::Units* newunits = mAchCascade->createUnits();
  newunits->name(L"mynewunits");
  mAchCascade->addElement(newunits);
  // Check that it is there...
  iface::cellml_api::UnitsSet* us = mAchCascade->localUnits();
  iface::cellml_api::Units* newunits2 = us->getUnits(L"mynewunits");
  CPPUNIT_ASSERT(newunits2);
  newunits2->release_ref();
  CPPUNIT_ASSERT_EQUAL(114, (int)els->length());

//     /**
//      * Removes a CellML element from this element.
//      * @param x The CellMLElement to remove.
//      */
//     void removeElement(in CellMLElement x);
  mAchCascade->removeElement(newunits);
  // Check that it has gone...
  newunits2 = us->getUnits(L"mynewunits");
  CPPUNIT_ASSERT(!newunits2);
  CPPUNIT_ASSERT_EQUAL(113, (int)els->length());

//     /**
//      * Removes a CellML element from this element, and replaces it with another
//      * CellML element.
//      * @raises CellMLException if the new element is of a type not allowed in
//      *                         this Element type, or belongs to the wrong Model.
//      * @param x The CellMLElement to remove.
//      * @param y The CellMLElement to add.
//      */
//     void replaceElement(in CellMLElement x, in CellMLElement y)
//       raises(CellMLException);
  newunits2 = us->getUnits(L"micromolar");
  CPPUNIT_ASSERT(newunits2);
  CPPUNIT_ASSERT_NO_THROW(mAchCascade->replaceElement(newunits2, newunits));
  newunits2->release_ref();
  CPPUNIT_ASSERT_EQUAL(113, (int)els->length());

  // Now check that newunits is there...
  newunits2 = us->getUnits(L"mynewunits");
  CPPUNIT_ASSERT(newunits2);
  CPPUNIT_ASSERT(newunits2 == newunits);
  newunits2->release_ref();

  // and micromolar is gone...
  newunits2 = us->getUnits(L"micromolar");
  CPPUNIT_ASSERT(!newunits2);

  els->release_ref();
//     /**
//      * Perform validation according to the validation requirements
//      * specification. Some validation only makes sense once entire model is
//      * loaded.
//      */
//     boolean validate();
  // XXX validate hasn't been written yet, but it probably needs at least one
  // test of its own.

//     /**
//      * The parent element of this model...
//      */
//     readonly attribute CellMLElement parentElement;
  iface::cellml_api::CellMLElement* el = mAchCascade->parentElement();
  CPPUNIT_ASSERT(!el);

  el = newunits->parentElement();
  CPPUNIT_ASSERT(el);
  CPPUNIT_ASSERT(el == mAchCascade);
  el->release_ref();

//     /**
//      * The underlying model element.
//      */
//     readonly attribute Model modelElement;
  iface::cellml_api::Model* m = mAchCascade->modelElement();
  CPPUNIT_ASSERT(m == mAchCascade);
  m->release_ref();
  m = newunits->modelElement();
  CPPUNIT_ASSERT(m == mAchCascade);
  m->release_ref();

  iface::cellml_api::Unit* newunit = mAchCascade->createUnit();
  m = newunit->modelElement();
  CPPUNIT_ASSERT(!m);
  newunits->addElement(newunit);
  m = newunit->modelElement();
  CPPUNIT_ASSERT(m == mAchCascade);
  m->release_ref();

//     /**
//      * Remove a CellML element by name, or do nothing if that element is not
//      * found.
//      * @param type The type of element("variable", "component", etc...)
//      * @param name The name to remove.
//      */
//     void removeByName(in CellMLAttributeString type,
//                       in CellMLAttributeString name);

  // Now remove the entry we just added, by name...
  mAchCascade->removeByName(L"units", L"mynewunits");
  newunits2 = us->getUnits(L"mynewunits");
  CPPUNIT_ASSERT(!newunits2);
  us->release_ref();

  el = newunits->parentElement();
  CPPUNIT_ASSERT(!el);

//     /**
//      * Sets user-supplied annotations on this element. These annotations are
//      * never read from an XML file or written out, but are for the application
//      * to use for any purpose it likes.
//      * Implementations in languages in which the ability to do this on any
//      * object is built in may choose not to implement this, provided that the
//      * implementation does not offer the possibility of remote access to the
//      * object.
//      * If the key already exists, then the existing user-data should be removed
//      * and the new user-data added.
//      * @param key  A string to identify the annotation.
//      * @param data The data to set, or nil to clear the UserData for a key.
//      */
//     void setUserData(in wstring key, in UserData data);
  MyUserData* mud = new MyUserData();
  newunits->setUserData(L"CellMLTest::myUserData", mud);
  mud->release_ref();

//     /**
//      * Retrieves user-supplied annotations previously set on this element.
//      * @param key A string to identify the annotation.
//      * @return The user-data associated with the key.
//      * @raises CellMLException if no UserData is set for the given key.
//      */
//     UserData getUserData(in wstring key) raises(CellMLException);
  iface::cellml_api::UserData* ud =
    newunits->getUserData(L"CellMLTest::myUserData");
  CPPUNIT_ASSERT(ud);

  newunits->setUserData(L"CellMLTest::myUserData", NULL);
  iface::cellml_api::UserData* throwaway;
  CPPUNIT_ASSERT_THROW(throwaway=newunits->getUserData(L"CellMLTest::myUserData"),
                       iface::cellml_api::CellMLException);

  // Set the data again, so we know that refcounting is right when userdata is
  // automatically destroyed.
  newunits->setUserData(L"CellMLTest::otherUserData", ud);
  ud->release_ref();

  newunit->release_ref();

  //  /**
  //   * Clones a CellMLElement, and optionally all children.
  //   * This will not clone the contents of imported models.
  //   * @param deep If false, only clones the element. It will then have no children.
  //   *             If true, clones the element, and its children, which are added
  //   *               into the new element.
  //   */
  //  CellMLElement clone(in boolean deep);
  el = mAchCascade->clone(true);
  iface::cellml_api::CellMLElement* el2 = el->modelElement();
  CPPUNIT_ASSERT(el == el2);
  CPPUNIT_ASSERT(el != mAchCascade);
  el2->release_ref();

  els = el->childElements();
  CPPUNIT_ASSERT_EQUAL(112, (int)els->length());
  els->release_ref();

  str = el->cmetaId();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade"), str);
  el->release_ref();

  el = mAchCascade->clone(false);
  el2 = el->modelElement();
  CPPUNIT_ASSERT(el == el2);
  CPPUNIT_ASSERT(el != mAchCascade);
  el2->release_ref();

  els = el->childElements();
  CPPUNIT_ASSERT_EQUAL(0, (int)els->length());
  els->release_ref();

  str = el->cmetaId();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade"), str);
  el->release_ref();

  // There are two separate implementations of clone, one on Model and a base
  // implementation on CellMLElement, so test that too...
  el = newunits->clone(true);
  els = el->childElements();
  CPPUNIT_ASSERT_EQUAL(1, (int)els->length());
  els->release_ref();
  el->release_ref();
  el = newunits->clone(false);
  els = el->childElements();
  CPPUNIT_ASSERT_EQUAL(0, (int)els->length());
  els->release_ref();
  el->release_ref();

  newunits->release_ref();  
}
//   };

//  /**
//   * This is an optional additional interface which CellMLElements may
//   * implement. It provides a link between CellML elements and DOM elements.
//   *
//   * API users should be aware that not all API implementations will provide
//   * this interface.
//   */
//  interface CellMLDOMElement
//    : XPCOM::IObject
//  {
void
CellMLTest::testCellMLDOMElement()
{
  loadAchCascade();

  iface::cellml_api::CellMLComponentSet* cs;
  cs = mAchCascade->localComponents();
  iface::cellml_api::CellMLComponent* cc = cs->getComponent(L"reaction19");
  cs->release_ref();
  DECLARE_QUERY_INTERFACE_REPLACE(dcc, cc, cellml_api::CellMLDOMElement);
  CPPUNIT_ASSERT(dcc);

//    /**
//     * The DOM element corresponding to this CellML element.
//     */
//    readonly attribute dom::Element domElement;

  iface::dom::Element* el = dcc->domElement();
  CPPUNIT_ASSERT(el);
  std::wstring str = el->getAttribute(L"name");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"reaction19"), str);
  el->release_ref();

  dcc->release_ref();
//  };
}

//   /**
//    * This is a general interface from which all CellML elements which have a
//    * name attribute inherit.
//    */
//   interface NamedCellMLElement
//     : CellMLElement
//   {
void
CellMLTest::testNamedCellMLElement()
{
  loadAchCascade();
//     /**
//      * The name associated with this CellML element.
//      */
//     attribute CellMLAttributeString name;
  std::wstring str = mAchCascade->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Ach_cascade_1995"), str);

  mAchCascade->name(L"Test_cascade_1995");
  str = mAchCascade->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"Test_cascade_1995"), str);
}
//   };

//   /**
//    * This interface represents a <model> element in the XML.
//    */
//   interface Model
//     : NamedCellMLElement
//   {
void
CellMLTest::testModel()
{
  loadBeelerReuter();
  loadTenTusscher();
  //  Model getAlternateVersion(in wstring cellmlVersion) raises(CellMLException);
  iface::cellml_api::Model* m = NULL;
  CPPUNIT_ASSERT_NO_THROW(m = mBeelerReuter->getAlternateVersion(L"1.1"));
  std::wstring str;
  CPPUNIT_ASSERT_NO_THROW(str = m->cellmlVersion());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"1.1"), str);
  m->release_ref();
  CPPUNIT_ASSERT_NO_THROW(m = mTenTusscher->getAlternateVersion(L"1.0"));
  CPPUNIT_ASSERT_NO_THROW(str = m->cellmlVersion());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"1.0"), str);
  m->release_ref();

//     /**
//      * The collection of groups described in this model document.
//      */
//     readonly attribute GroupSet groups;
  // Get groups...

  iface::cellml_api::GroupSet* gs = NULL;
  CPPUNIT_ASSERT_NO_THROW(gs = mBeelerReuter->groups());
  int l;
  CPPUNIT_ASSERT_NO_THROW(l = (int)gs->length());
  CPPUNIT_ASSERT_EQUAL(2, l);

  // We are doing the GroupSet test while we are here...
//   /**
//    * An interface for accessing a set of Imports
//    */
//   interface GroupSet
//     : CellMLElementSet
//   {
//     /**
//      * Returns a CellMLElementIterator that can be used to iterate through the
//      * elements. The iteration order is undefined.
//      */
//     GroupIterator iterateGroups();

  iface::cellml_api::GroupIterator* gi = NULL;
  CPPUNIT_ASSERT_NO_THROW(gi = gs->iterateGroups());
  iface::cellml_api::Group* grp = NULL;
  CPPUNIT_ASSERT_NO_THROW(grp = gi->nextGroup());

  // Warning: This is order dependent, but the CellML API doesn't specify
  // iteration order, so it could fail on some valid API implementations.
  // Feel free to swap the order of the next two pairs of tests around to
  // make it pass on other implementations.
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(!grp->isEncapsulation()));
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(grp->isContainment()));
  grp->release_ref();

  CPPUNIT_ASSERT_NO_THROW(grp = gi->nextGroup());
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(grp->isEncapsulation()));
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(!grp->isContainment()));
  grp->release_ref();
  gi->release_ref();
//     /**
//      * Returns the subset of this set containing groups involving a particular
//      * relationship.
//      * @param relName The relationship groups must contain to be in the returned set.
//      * @return GroupSet The new set of groups.
//      */
//     GroupSet getSubsetInvolvingRelationship(in CellMLAttributeString relName);
  iface::cellml_api::GroupSet* gs2 = NULL;
  CPPUNIT_ASSERT_NO_THROW(gs2 = gs->getSubsetInvolvingRelationship
                          (L"encapsulation"));
  l = (int)gs2->length();
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL(1, l));
  CPPUNIT_ASSERT_NO_THROW(gi = gs2->iterateGroups());
  CPPUNIT_ASSERT_NO_THROW(grp = gi->nextGroup());
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(grp->isEncapsulation()));
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(!grp->isContainment()));
  grp->release_ref();
  gi->release_ref();
  gs2->release_ref();
//     /**
//      * The subset of all connections in this collection involving
//      * encapsulation.
//      */
//     readonly attribute GroupSet subsetInvolvingEncapsulation;
  CPPUNIT_ASSERT_NO_THROW(gs2 = gs->subsetInvolvingEncapsulation());
  l = (int)gs2->length();
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL(1, l));
  CPPUNIT_ASSERT_NO_THROW(gi = gs2->iterateGroups());
  CPPUNIT_ASSERT_NO_THROW(grp = gi->nextGroup());
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(grp->isEncapsulation()));
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(!grp->isContainment()));
  grp->release_ref();
  gi->release_ref();
  gs2->release_ref();

//     /**
//      * The subset of all connections in this collection involving
//      * containment.
//      */
//     readonly attribute GroupSet subsetInvolvingContainment;
  CPPUNIT_ASSERT_NO_THROW(gs2 = gs->subsetInvolvingContainment());
  l = (int)gs2->length();
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL(1, l));
  CPPUNIT_ASSERT_NO_THROW(gi = gs2->iterateGroups());
  CPPUNIT_ASSERT_NO_THROW(grp = gi->nextGroup());
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(grp->isContainment()));
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(!grp->isEncapsulation()));
  grp->release_ref();
  gi->release_ref();
  gs2->release_ref();

  gs->release_ref();

//   };
//     /**
//      * The collection of imports described in this model document.
//      */
//     readonly attribute CellMLImportSet imports;
  iface::cellml_api::CellMLImportSet* cis = NULL;
  CPPUNIT_ASSERT_NO_THROW(cis = mTenTusscher->imports());
  l = (int)cis->length();
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT_EQUAL(2, l));
  // We are doing the CellMLImportSet test while we are here...
//   /**
//    * An interface for accessing a set of Imports
//    */
//   interface CellMLImportSet
//     : NamedCellMLElementSet
//   {
//     /**
//      * Returns a CellMLElementIterator that can be used to iterate through the
//      * elements. The iteration order is undefined.
//      */
//     CellMLImportIterator iterateImports();
  iface::cellml_api::CellMLImportIterator* cii = NULL;
  CPPUNIT_ASSERT_NO_THROW(cii = cis->iterateImports());
  iface::cellml_api::CellMLImport* ci = NULL;
  CPPUNIT_ASSERT_NO_THROW(ci = cii->nextImport());
  iface::cellml_api::URI* xh = NULL;
  CPPUNIT_ASSERT_NO_THROW(xh = ci->xlinkHref());
  CPPUNIT_ASSERT_NO_THROW(str = xh->asText());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"../../common/units.xml"), str);
  xh->release_ref();
  ci->release_ref();
  CPPUNIT_ASSERT_NO_THROW(ci = cii->nextImport());
  CPPUNIT_ASSERT_NO_THROW(xh = ci->xlinkHref());
  CPPUNIT_ASSERT_NO_THROW(str = xh->asText());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"../2004_"
                                    L"tenTusscher_noble_noble_panfilov-endo.xml"),
                       str);
  xh->release_ref();
  ci->release_ref();
  cii->release_ref();

//   };
  cis->release_ref();

//     /**
//      * Models should really set an 'xml:base' attribute in the model element as
//      * we need to be able to unambiguously and uniquely reference a model
//      * independent of its location. If this is not set, the xmlBase could be
//      * set to the URI used to obtain it.
//      */
//     readonly attribute URI xmlBase;
  CPPUNIT_ASSERT_NO_THROW(xh = mBeelerReuter->xmlBase());
  CPPUNIT_ASSERT_NO_THROW(str = xh->asText());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.example.com/base/beelerreuter.xml"), str);
  xh->release_ref();

//     /**
//      * The collection of units defined within this model.
//      */
//     readonly attribute UnitsSet localUnits;
  iface::cellml_api::UnitsSet* us = NULL;
  CPPUNIT_ASSERT_NO_THROW(us = mBeelerReuter->localUnits());
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(10, l);
  us->release_ref();

  CPPUNIT_ASSERT_NO_THROW(us = mTenTusscher->localUnits());
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(0, l);
  us->release_ref();

//     /**
//      * The collection of units. This will fetch all the units retrieved by
//      * localUnits, but in addition will also fetch all the ImportUnits
//      * declared in this model.
//      */
//     readonly attribute UnitsSet modelUnits;
  CPPUNIT_ASSERT_NO_THROW(us = mBeelerReuter->modelUnits());
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(10, l);
  us->release_ref();

  CPPUNIT_ASSERT_NO_THROW(us = mTenTusscher->modelUnits());
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(13, l);
  iface::cellml_api::UnitsIterator* ui = NULL;
  CPPUNIT_ASSERT_NO_THROW(ui = us->iterateUnits());
  iface::cellml_api::Units* cu = NULL;
  CPPUNIT_ASSERT_NO_THROW(cu = ui->nextUnits());
  CPPUNIT_ASSERT(cu);
  DECLARE_QUERY_INTERFACE_REPLACE(icu, cu, cellml_api::ImportUnits);
  CPPUNIT_ASSERT(icu);
  CPPUNIT_ASSERT_NO_THROW(str = icu->name());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"ms"), str);

  CPPUNIT_ASSERT_NO_THROW(str = icu->unitsRef());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"ms"), str);

  icu->release_ref();
  ui->release_ref();
  us->release_ref();

//     /**
//      * The collection of units. Provided that the imports are instantiated,
//      * this will fetch units in both direct and indirect imports. If the
//      * import containing the unit definition has been instantiated, iterating
//      * the set will return the unit definition, otherwise it will return
//      * the ImportUnit. If an import containing unit definitions is instantiated
//      * after iterator creation but before unit retrieval, the implementation
//      * may choose to return either the ImportUnit or the unit definition.
//      * Applications must not rely upon the behaviour of any particular
//      * implementation in this case.
//      */
//     readonly attribute UnitsSet allUnits;

  us = mBeelerReuter->allUnits();
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(10, l);
  us->release_ref();

  us = mTenTusscher->allUnits();
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(13, l);

  CPPUNIT_ASSERT_NO_THROW(ui = us->iterateUnits());
  CPPUNIT_ASSERT_NO_THROW(cu = ui->nextUnits());
  CPPUNIT_ASSERT(cu);
  QUERY_INTERFACE_REPLACE(icu, cu, cellml_api::ImportUnits);
  CPPUNIT_ASSERT(icu);
  icu->release_ref();
  ui->release_ref();
  us->release_ref();

//     /**
//      * This fully instantiates the import definitions. It will then recursively
//      * instantiate all imports in the imported models.
//      */
//     void fullyInstantiateImports();
  CPPUNIT_ASSERT_NO_THROW(mTenTusscher->fullyInstantiateImports());

  CPPUNIT_ASSERT_NO_THROW(us = mTenTusscher->allUnits());
  CPPUNIT_ASSERT_NO_THROW(l = (int)us->length());
  CPPUNIT_ASSERT_EQUAL(1056, l);
  CPPUNIT_ASSERT_NO_THROW(ui = us->iterateUnits());
  CPPUNIT_ASSERT_NO_THROW(cu = ui->nextUnits());
  CPPUNIT_ASSERT(cu);
  QUERY_INTERFACE_REPLACE(icu, cu, cellml_api::ImportUnits);
  CPPUNIT_ASSERT(!icu);
  ui->release_ref();
  us->release_ref();

//     /**
//      * Returns a list of all components which are defined in this model.
//      */
//     readonly attribute CellMLComponentSet localComponents;
  iface::cellml_api::CellMLComponentSet* cs = NULL;
  CPPUNIT_ASSERT_NO_THROW(cs = mTenTusscher->localComponents());
  CPPUNIT_ASSERT_NO_THROW(l = (int)cs->length());
  CPPUNIT_ASSERT_EQUAL(4, l);
  cs->release_ref();
  CPPUNIT_ASSERT_NO_THROW(cs = mBeelerReuter->localComponents());
  CPPUNIT_ASSERT_NO_THROW(l = (int)cs->length());
  CPPUNIT_ASSERT_EQUAL(12, l);
  cs->release_ref();

//     /**
//      * The collection of components. This will fetch all the components retrieved by
//      * localComponents, but in addition will also fetch all the ImportComponents
//      * declared in this model.
//      */
//     readonly attribute CellMLComponentSet modelComponents;
  CPPUNIT_ASSERT_NO_THROW(cs = mTenTusscher->modelComponents());
  CPPUNIT_ASSERT_NO_THROW(l = (int)cs->length());
  CPPUNIT_ASSERT_EQUAL(5, l);
  cs->release_ref();
  CPPUNIT_ASSERT_NO_THROW(cs = mBeelerReuter->modelComponents());
  CPPUNIT_ASSERT_NO_THROW(l = (int)cs->length());
  CPPUNIT_ASSERT_EQUAL(12, l);
  cs->release_ref();

//     /**
//      * Returns all components with interface 'CellMLComponent' in the model.
//      * Provided that the imports are instantiated, this will fetch components
//      * in both direct and indirect imports. If the import containing the
//      * component definition has been instantiated, iterating the set will
//      * return the component definition, otherwise it will return the
//      * ImportComponent. If an import containing component definitions is
//      * instantiated after iterator creation but before component retrieval,
//      * the implementation may choose to return either the ImportComponent or
//      * the component definition. Applications must not rely upon the behaviour
//      * of any particular implementation in this case.
//      */
//     readonly attribute CellMLComponentSet allComponents;
  CPPUNIT_ASSERT_NO_THROW(cs = mBeelerReuter->allComponents());
  CPPUNIT_ASSERT_NO_THROW(l = (int)cs->length());
  CPPUNIT_ASSERT_EQUAL(12, l);
  cs->release_ref();
  CPPUNIT_ASSERT_NO_THROW(cs = mTenTusscher->allComponents());
  CPPUNIT_ASSERT_EQUAL(46, (int)cs->length());
  cs->release_ref();
//     /**
//      * The set of all connections defined in this model document.
//      */
//     readonly attribute ConnectionSet connections;
  iface::cellml_api::ConnectionSet* connset = NULL;
  CPPUNIT_ASSERT_NO_THROW(connset = mTenTusscher->connections());
  CPPUNIT_ASSERT_NO_THROW(l = (int)connset->length());
  CPPUNIT_ASSERT_EQUAL(6, l);
  connset->release_ref();

//     /**
//      * The following also searches user-defined relationships.
//      * @param name The name of the relationship to search for.
//      */
//     GroupSet findGroupsWithRelationshipRefName
//     (
//      in CellMLAttributeString name
//     );
  CPPUNIT_ASSERT_NO_THROW(gs = mBeelerReuter->findGroupsWithRelationshipRefName
                          (L"containment"));
  CPPUNIT_ASSERT_NO_THROW(l = (int)gs->length());
  CPPUNIT_ASSERT_EQUAL(1, l);
  CPPUNIT_ASSERT_NO_THROW(gi = gs->iterateGroups());
  iface::cellml_api::Group* g = NULL;
  CPPUNIT_ASSERT_NO_THROW(g = gi->nextGroup());
  gi->release_ref();
  iface::cellml_api::RelationshipRefSet* rrs = NULL;
  CPPUNIT_ASSERT_NO_THROW(rrs = g->relationshipRefs());
  g->release_ref();
  CPPUNIT_ASSERT_NO_THROW(l = (int)rrs->length());
  CPPUNIT_ASSERT_EQUAL(1, l);
  iface::cellml_api::RelationshipRefIterator* rri = NULL;
  CPPUNIT_ASSERT_NO_THROW(rri = rrs->iterateRelationshipRefs());
  iface::cellml_api::RelationshipRef* rr = NULL;
  CPPUNIT_ASSERT_NO_THROW(rr = rri->nextRelationshipRef());
  rri->release_ref();
  CPPUNIT_ASSERT_NO_THROW(str = rr->relationship());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"containment"), str);
  rr->release_ref();
  rrs->release_ref();
  gs->release_ref();
  
//     /**
//      * Return a flattened model, i.e. all imported component trees are promoted
//      * to model level and import objects are removed.
//      * @return The flattened model.
//      */
//     Model generateFlattenedModel();
  // XXX TODO not implemented yet.

//     /**
//      * Creates a new (local) CellMLComponent
//      */
//     CellMLComponent createComponent();
  iface::cellml_api::CellMLComponent* c = NULL;
  CPPUNIT_ASSERT_NO_THROW(c = mBeelerReuter->createComponent());
  CPPUNIT_ASSERT(c);
  c->release_ref();

//     /**
//      * Creates a new ImportComponent
//      */
//     ImportComponent createImportComponent();
  iface::cellml_api::ImportComponent* ic = NULL;
  CPPUNIT_ASSERT_NO_THROW(ic = mBeelerReuter->createImportComponent());
  CPPUNIT_ASSERT(ic);
  ic->release_ref();

//     /**
//      * Creates a new (local) Units
//      */
//     Units createUnits();
  CPPUNIT_ASSERT_NO_THROW(cu = mBeelerReuter->createUnits());
  CPPUNIT_ASSERT(cu);
  cu->release_ref();

//     /**
//      * Creates a new (import) Units
//      */
//     ImportUnits createImportUnits();
  CPPUNIT_ASSERT_NO_THROW(icu = mBeelerReuter->createImportUnits());
  CPPUNIT_ASSERT(icu);
  icu->release_ref();

//     /**
//      * Creates a new Unit
//      */
//     Unit createUnit();
  iface::cellml_api::Unit* unit = NULL;
  CPPUNIT_ASSERT_NO_THROW(unit = mBeelerReuter->createUnit());
  CPPUNIT_ASSERT(unit);
  unit->release_ref();

//     /**
//      * Creates a new CellMLImport
//      */
//     CellMLImport createCellMLImport();
  CPPUNIT_ASSERT_NO_THROW(ci = mBeelerReuter->createCellMLImport());
  CPPUNIT_ASSERT(ci);
  ci->release_ref();

//     /**
//      * Creates a new CellMLVariable
//      */
//     CellMLVariable createCellMLVariable();
  iface::cellml_api::CellMLVariable* cv = NULL;
  CPPUNIT_ASSERT_NO_THROW(cv = mBeelerReuter->createCellMLVariable());
  CPPUNIT_ASSERT(cv);
  cv->release_ref();

//     /**
//      * Creates a new ComponentRef.
//      */
//     ComponentRef createComponentRef();
  iface::cellml_api::ComponentRef* cr = NULL;
  CPPUNIT_ASSERT_NO_THROW(cr = mBeelerReuter->createComponentRef());
  CPPUNIT_ASSERT(cr);
  cr->release_ref();

//     /**
//      * Creates a new RelationshipRef.
//      */
//     RelationshipRef createRelationshipRef();
  CPPUNIT_ASSERT_NO_THROW(rr = mBeelerReuter->createRelationshipRef());
  CPPUNIT_ASSERT(rr);
  rr->release_ref();

//     /**
//      * Creates a new Group.
//      */
//     Group createGroup();
  CPPUNIT_ASSERT_NO_THROW(g = mBeelerReuter->createGroup());
  CPPUNIT_ASSERT(g);
  g->release_ref();

//     /**
//      * Creates a new Connection. This also creates the MapComponents.
//      */
//     Connection createConnection();
  iface::cellml_api::Connection* conn = NULL;
  CPPUNIT_ASSERT_NO_THROW(conn = mBeelerReuter->createConnection());
  CPPUNIT_ASSERT(conn);

  // Test implicit map component creation...
  iface::cellml_api::MapComponents* mc = NULL;
  mBeelerReuter->addElement(conn);
  CPPUNIT_ASSERT_NO_THROW(mc = conn->componentMapping());
  CPPUNIT_ASSERT(mc);
  mc->release_ref();
  conn->release_ref();

//     /**
//      * Creates a new MapVariables.
//      */
//     MapVariables createMapVariables();
  iface::cellml_api::MapVariables* mv = NULL;
  CPPUNIT_ASSERT_NO_THROW(mv = mBeelerReuter->createMapVariables());
  CPPUNIT_ASSERT(mv);
  mv->release_ref();

//     /**
//      * Creates a new Reaction
//      */
//     Reaction createReaction();
  iface::cellml_api::Reaction* rea = NULL;
  CPPUNIT_ASSERT_NO_THROW(rea = mBeelerReuter->createReaction());
  CPPUNIT_ASSERT(rea);
  rea->release_ref();

//       /**
//        * Creates a new VariableRef
//        */
//       VariableRef createVariableRef();
  iface::cellml_api::VariableRef* vr = NULL;
  CPPUNIT_ASSERT_NO_THROW(vr = mBeelerReuter->createVariableRef());
  CPPUNIT_ASSERT(vr);
  vr->release_ref();

//         /**
//          * Creates a new Role
//          */
//         Role createRole();
  iface::cellml_api::Role* role = NULL;
  CPPUNIT_ASSERT_NO_THROW(role = mBeelerReuter->createRole());
  CPPUNIT_ASSERT(role);
  role->release_ref();
}
//   };

//   interface MathContainer : XPCOM::IObject
//   {
void
CellMLTest::testMathContainer()
{
  loadBeelerReuter();
//     /**
//      * The set of all math defined here.
//      */
//     readonly attribute MathList math;
  iface::cellml_api::CellMLComponentSet* cs = mBeelerReuter->localComponents();
  iface::cellml_api::CellMLComponentIterator* ci = cs->iterateComponents();
  cs->release_ref();
  iface::cellml_api::CellMLComponent* c = ci->nextComponent();

  iface::cellml_api::MathList* ml = c->math();
  // We are testing MathList here...
//   /**
//    * A collection of math.
//    */
//   interface MathList : XPCOM::IObject
//   {
//     /**
//      * The length of the collection.
//      */
//     readonly attribute unsigned long length;
  // Note: order of component iterator is undefined, but this code assumes the
  //   order in the XML file. An implementation is still conforming if it is
  //   necessary to change the order in which components are fetched.
  CPPUNIT_ASSERT_EQUAL(0, (int)ml->length());
  ml->release_ref();
  c->release_ref();
  c = ci->nextComponent();
  ci->release_ref();
  ml = c->math();
  CPPUNIT_ASSERT_EQUAL(1, (int)ml->length());

//     /**
//      * Returns a CellMLElementIterator that can be used to iterate through the
//      * elements. The iteration order is undefined.
//      */
//     MathMLElementIterator iterate();
  iface::cellml_api::MathMLElementIterator* mi = ml->iterate();
  // We are testing the iterator while we are here...
//   /**
//    * An interface allowing MathMLElements to be iterated.
//    */
//   interface MathMLElementIterator : XPCOM::IObject
//   {
//     /**
//      * Fetches the next MathML element, and advances the iterator.
//      * @return The next MathML element, or nil if there are no more MathML
//      *         elements.
//      */
//     MathMLElement next();
  iface::mathml_dom::MathMLElement* me = mi->next();
  std::wstring str = me->className();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"testmath"), str);
//   };
  mi->release_ref();

//     /**
//      * Tests for the existance of an element in the set.
//      * @param x The element to test for.
//      * @return true if the element is present, or false otherwise.
//      */
//     boolean contains(in MathMLElement x);
  CPPUNIT_ASSERT(ml->contains(me));
  iface::dom::Document* dd = me->ownerDocument();
  iface::dom::Element* el = dd->createElementNS(MATHML_NS, L"math");
  dd->release_ref();
  DECLARE_QUERY_INTERFACE_REPLACE(me2, el, mathml_dom::MathMLElement);
  CPPUNIT_ASSERT(!ml->contains(me2));
//   };

//     /**
//      * Add an element to this collection.
//      * @param x The element to add.
//      */
//     void addMath(in MathMLElement x);
  c->addMath(me2);
  CPPUNIT_ASSERT_EQUAL(2, (int)ml->length());
  CPPUNIT_ASSERT(ml->contains(me));
  CPPUNIT_ASSERT(ml->contains(me2));
//     /**
//      * Remove an element from this collection. If the element is not found,
//      * do nothing.
//      * @param x The element to remove.
//      */
//     void removeMath(in MathMLElement x);
  c->removeMath(me2);
  CPPUNIT_ASSERT_EQUAL(1, (int)ml->length());
  CPPUNIT_ASSERT(ml->contains(me));
  CPPUNIT_ASSERT(!ml->contains(me2));

//     /**
//      * Find an element in this collection, and if it is found, replace it
//      * with another element.
//      * @param x The element to find.
//      * @param y The element to replace x with.
//      * @raises CellMLException If y is not valid according to the constraints
//      *                         on this collection(check optional).
//      */
//     void replaceMath(in MathMLElement x, in MathMLElement y);
  c->replaceMath(me, me2);
  CPPUNIT_ASSERT_EQUAL(1, (int)ml->length());
  CPPUNIT_ASSERT(ml->contains(me2));
  CPPUNIT_ASSERT(!ml->contains(me));

//     /**
//      * Remove all elements in this collection.
//      */
//     void clearMath();
  c->clearMath();
  CPPUNIT_ASSERT_EQUAL(0, (int)ml->length());  
  me->release_ref();
  me2->release_ref();
  ml->release_ref();
  c->release_ref();
}
//   };

//   /**
//    * This interface represents a component element in the CellML document.
//    */
//   interface CellMLComponent
//     : NamedCellMLElement, MathContainer
//   {
void
CellMLTest::testCellMLComponent()
{
  loadBeelerReuter();
  loadTenTusscher();

  iface::cellml_api::CellMLComponentSet* cs = mBeelerReuter->localComponents();
  iface::cellml_api::CellMLComponentSet* cs2 = mTenTusscher->localComponents();
  iface::cellml_api::CellMLComponentIterator* ci = cs->iterateComponents();
  iface::cellml_api::CellMLComponentIterator* ci2 = cs2->iterateComponents();
  cs->release_ref();
  cs2->release_ref();
  iface::cellml_api::CellMLComponent* c = ci->nextComponent();
  iface::cellml_api::CellMLComponent* c2 = ci2->nextComponent();
  ci->release_ref();
  ci2->release_ref();

//     /**
//      * The set of all variables defined in this component.
//      */
//     readonly attribute CellMLVariableSet variables;
  iface::cellml_api::CellMLVariableSet* cvs = c->variables();
  iface::cellml_api::CellMLVariableSet* cvs2 = c2->variables();
  CPPUNIT_ASSERT_EQUAL(1, (int)cvs->length());
  CPPUNIT_ASSERT_EQUAL(1, (int)cvs2->length());
  iface::cellml_api::CellMLVariableIterator* cvi = cvs->iterateVariables();
  iface::cellml_api::CellMLVariableIterator* cvi2 = cvs2->iterateVariables();
  iface::cellml_api::CellMLVariable* v = cvi->nextVariable();
  iface::cellml_api::CellMLVariable* v2 = cvi2->nextVariable();
  cvi->release_ref();
  cvi2->release_ref();
  std::wstring str = v->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"footime"), str);
  str = v2->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"time"), str);
  v->release_ref();
  v2->release_ref();
  cvs->release_ref();
  cvs2->release_ref();

//     /**
//      * The set of all units defined in this component.
//      */
//     readonly attribute UnitsSet units;
  iface::cellml_api::UnitsSet* us = c->units();
  CPPUNIT_ASSERT_EQUAL(1, (int)us->length());
  us->release_ref();

//     /**
//      * The set of all connections which involve this component.
//      */
//     readonly attribute ConnectionSet connections;
  iface::cellml_api::ConnectionSet* conns = c->connections();
  // There are 5 connections to environment.
  CPPUNIT_ASSERT_EQUAL(5, (int)conns->length());
  conns->release_ref();

  mTenTusscher->fullyInstantiateImports();
  c2->release_ref();
  CPPUNIT_ASSERT_NO_THROW(cs2 = mTenTusscher->modelComponents());
  CPPUNIT_ASSERT_NO_THROW(c2 = cs2->getComponent(L"model"));
  cs2->release_ref();
  CPPUNIT_ASSERT_NO_THROW(conns = c2->connections());
  CPPUNIT_ASSERT_EQUAL(27, (int)conns->length());
  conns->release_ref();

  RETURN_INTO_OBJREF(ttac, iface::cellml_api::CellMLComponentSet, mTenTusscher->allComponents());
  RETURN_INTO_OBJREF(mpc, iface::cellml_api::CellMLComponent, ttac->getComponent(L"membrane_potential"));
  RETURN_INTO_OBJREF(mpccs, iface::cellml_api::ConnectionSet, mpc->connections());
  CPPUNIT_ASSERT(mpccs != NULL);

//     /**
//      * The component which, in the encapsulation hierarchy defined in this
//      * model, encapsulates this component. When this component is
//      * an imported component, an implementation must return
//      * any encapsulation parent defined in the importing model.
//      * When the same component is imported twice under different
//      * names, the implementation must return the correct encapsulation
//      * parent for each component instance.
//      * This attribute is nil if there is no encapsulation parent.
//      */
//     readonly attribute CellMLComponent encapsulationParent;
  iface::cellml_api::CellMLComponent* c3 = c2->encapsulationParent();
  CPPUNIT_ASSERT(!c3);

//     /**
//      * The set of all children of this component.
//      */
//     readonly attribute CellMLComponentSet encapsulationChildren;
  cs2 = c2->encapsulationChildren();
  CPPUNIT_ASSERT(cs2);
  CPPUNIT_ASSERT_EQUAL(23, (int)cs2->length());
  ci = cs2->iterateComponents();
  c3 = ci->nextComponent();
  ci->release_ref();
  CPPUNIT_ASSERT(c3);
  // Order is not specified by the CellML API specification. Implementators may
  // adjust this if their API returns elements in a different order...
  str = c3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"membrane_potential"), str);
  // We haven't really tested encapsulationParent() properly yet, so test...
  iface::cellml_api::CellMLComponent* c4 = NULL;
  CPPUNIT_ASSERT_NO_THROW(c4 = c3->encapsulationParent());
  str = c4->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"interface"), str);
  c4->release_ref();
  c3->release_ref();
  cs2->release_ref();

//     /**
//      * The containment parent for this component. This must work correctly
//      * across imports if imports have been instantiated.
//      */
//     readonly attribute CellMLComponent containmentParent;
  c->release_ref();
  cs = mBeelerReuter->localComponents();
  c = cs->getComponent(L"sodium_current");
  cs->release_ref();

  c3 = c->containmentParent();
  str = c3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"membrane"), str);

//     /**
//      * The containment parent for this component. This must work correctly
//      * across imports if imports have been instantiated.
//      */
//     readonly attribute CellMLComponentSet containmentChildren;
  cs = c3->containmentChildren();
  CPPUNIT_ASSERT(cs);
  CPPUNIT_ASSERT_EQUAL(4, (int)cs->length());
  cs->release_ref();
  cs = c->containmentChildren();
  CPPUNIT_ASSERT(cs);
  CPPUNIT_ASSERT_EQUAL(3, (int)cs->length());
  cs->release_ref();
  c3->release_ref();

//     /**
//      * Gets the 'import number' in which this component is contained.
//      * 0 means it is in the parent, and all other numbers are those allocated
//      * for each import. This must match the uniqueIdentifier on the import
//      * element which causes the importation of this model.
//      */
//     readonly attribute unsigned long importNumber;
  CPPUNIT_ASSERT_EQUAL(0, (int)c->importNumber());
  c2->release_ref();
  cs2 = mTenTusscher->modelComponents();
  c2 = cs2->getComponent(L"model");
  CPPUNIT_ASSERT(c2->importNumber() != 0);
  cs2->release_ref();

  c->release_ref();
  c2->release_ref();
}
//   };

//   interface Units
//     : NamedCellMLElement
//   {
void
CellMLTest::testUnits()
{
  loadBeelerReuter();

  iface::cellml_api::UnitsSet* us = mBeelerReuter->localUnits();
  iface::cellml_api::UnitsIterator* ui = us->iterateUnits();
  iface::cellml_api::Units* u1 = ui->nextUnits();
  iface::cellml_api::Units* u2 = ui->nextUnits();
  ui->release_ref();
  us->release_ref();

  // This is just here to make sure we got the units the test is referring to.
  // The CellML API specification does not specify the order in which iterators
  // return elements, so implementators or compliant API implementations may
  // need modify this logic to fetch the correct units.
  std::wstring str = u1->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"notarealunit"), str);
  str = u2->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millisecond"), str);

//     /**
//      * True if this units element defines a base unit, or false otherwise.
//      */
//     attribute boolean isBaseUnits;
  CPPUNIT_ASSERT(u1->isBaseUnits());
  CPPUNIT_ASSERT(!u2->isBaseUnits());
  u1->isBaseUnits(false);
  CPPUNIT_ASSERT(!u1->isBaseUnits());

//     /**
//      * A collection of all the <unit> elements making up this <units> element.
//      */
//     readonly attribute UnitSet unitCollection;
  iface::cellml_api::UnitSet* uns = u1->unitCollection();
  CPPUNIT_ASSERT_EQUAL(0, (int)uns->length());
  uns->release_ref();
  uns = u2->unitCollection();
  CPPUNIT_ASSERT_EQUAL(1, (int)uns->length());
  iface::cellml_api::UnitIterator* uni = uns->iterateUnits();
  iface::cellml_api::Unit* un = uni->nextUnit();
  
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(-3), un->prefix());
  un->prefix(-2);
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(-2), un->prefix());

  uni->release_ref();
  uns->release_ref();
  un->release_ref();

  u1->release_ref();
  u2->release_ref();
}

//   };

//   /**
//    * Interface to a <unit> element.
//    */
//   interface Unit
//     : CellMLElement
//   {
void
CellMLTest::testUnit()
{
  loadBeelerReuter();

  iface::cellml_api::UnitsSet* us = mBeelerReuter->localUnits();
  iface::cellml_api::UnitsIterator* ui = us->iterateUnits();
  ui->nextUnits()->release_ref();
  iface::cellml_api::Units* u1 = ui->nextUnits();
  iface::cellml_api::Units* u2 = ui->nextUnits();
  iface::cellml_api::Units* u3 = us->getUnits(L"per_millivolt_millisecond");
  ui->release_ref();
  us->release_ref();

  iface::cellml_api::UnitSet* uns = u1->unitCollection();
  iface::cellml_api::UnitIterator* uni = uns->iterateUnits();
  iface::cellml_api::Unit* un1 = uni->nextUnit();
  uni->release_ref();
  uns->release_ref();

  uns = u2->unitCollection();
  uni = uns->iterateUnits();
  iface::cellml_api::Unit* un2 = uni->nextUnit();
  uni->release_ref();
  uns->release_ref();

  uns = u3->unitCollection();
  uni = uns->iterateUnits();
  iface::cellml_api::Unit* un31 = uni->nextUnit();
  iface::cellml_api::Unit* un32 = uni->nextUnit();
  uni->release_ref();
  uns->release_ref();

//     /**
//      * The value of the prefix attribute in the unit element. This is
//      * automatically converted to or from the text name of the SI prefix when
//      * converting to a DOM view.
//      */
//     attribute long prefix;
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(-3), un1->prefix());
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(-3), un2->prefix());
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(0), un31->prefix());
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(0), un32->prefix());
  
//     /**
//      * The value of the multiplier attribute in the unit element.
//      */
//     attribute double multiplier;
  double x;
  CPPUNIT_ASSERT_NO_THROW(x = un1->multiplier());
  CPPUNIT_ASSERT_EQUAL(1.5, x);
  CPPUNIT_ASSERT_NO_THROW(x = un2->multiplier());
  CPPUNIT_ASSERT_EQUAL(1.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un31->multiplier());
  CPPUNIT_ASSERT_EQUAL(1.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un32->multiplier());
  CPPUNIT_ASSERT_EQUAL(1.0, x);
  CPPUNIT_ASSERT_NO_THROW(un1->multiplier(2.0));
  CPPUNIT_ASSERT_NO_THROW(x = un1->multiplier());
  CPPUNIT_ASSERT_EQUAL(2.0, x);

//     /**
//      * The value of the offset attribute in the unit element.
//      */
//     attribute double offset;
  CPPUNIT_ASSERT_NO_THROW(x = un1->offset());
  CPPUNIT_ASSERT_EQUAL(2.8, x);
  CPPUNIT_ASSERT_NO_THROW(x = un2->offset());
  CPPUNIT_ASSERT_EQUAL(0.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un31->offset());
  CPPUNIT_ASSERT_EQUAL(0.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un32->offset());
  CPPUNIT_ASSERT_EQUAL(0.0, x);
  CPPUNIT_ASSERT_NO_THROW(un1->offset(3.0));
  CPPUNIT_ASSERT_NO_THROW(x = un1->offset());
  CPPUNIT_ASSERT_EQUAL(3.0, x);

//     /**
//      * The value of the exponent attribute in unit element.
//      */
//     attribute double exponent;
  CPPUNIT_ASSERT_NO_THROW(x = un1->exponent());
  CPPUNIT_ASSERT_EQUAL(1.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un2->exponent());
  CPPUNIT_ASSERT_EQUAL(-1.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un31->exponent());
  CPPUNIT_ASSERT_EQUAL(-1.0, x);
  CPPUNIT_ASSERT_NO_THROW(x = un32->exponent());
  CPPUNIT_ASSERT_EQUAL(-1.0, x);
  CPPUNIT_ASSERT_NO_THROW(un1->exponent(4.0));
  CPPUNIT_ASSERT_NO_THROW(x = un1->exponent());
  CPPUNIT_ASSERT_EQUAL(4.0, x);

//     /**
//      * The name of the units attribute.
//      */
//     attribute CellMLAttributeString units;
  std::wstring str(un1->units());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"second"), str);

  un1->units(L"foobar");
  str = un1->units();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);

  str = un2->units();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"second"), str);

  str = un31->units();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millivolt"), str);

  str = un32->units();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millisecond"), str);

  un1->release_ref();
  un2->release_ref();
  un31->release_ref();
  un32->release_ref();
  u1->release_ref();
  u2->release_ref();
  u3->release_ref();
}
//   };

//   /**
//    * This interface represents a CellML import element.
//    */
//   interface CellMLImport
//     : CellMLElement
//   {
void
CellMLTest::testCellMLImport()
{
  loadTenTusscher();
  iface::cellml_api::CellMLImportSet* cis = mTenTusscher->imports();
  iface::cellml_api::CellMLImportIterator* cii = cis->iterateImports();
  
  iface::cellml_api::CellMLImport* ci1 = cii->nextImport();
  iface::cellml_api::CellMLImport* ci2 = cii->nextImport();
  cis->release_ref();
  cii->release_ref();

//     /**
//      * The URI in the xlink:href attribute. Changing the URL in the xlinkHref
//      * on an instantiated import will cause the import to become
//      * uninstantiated. If there are any iterators(of a kind which is required
//      * to return elements from more than one model) which were created from the
//      * sets prior to changing xlinkHref, an implementation may choose to
//      * continue to provide elements from the previously instantiated import.
//      * Applications must not rely upon the behaviour of any particular
//      * implementation in this case.
//      */
//     readonly attribute URI xlinkHref;
  iface::cellml_api::URI* u = ci1->xlinkHref();
  std::wstring str = u->asText();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"../../common/units.xml"), str);
  u->asText(L"http://www.example.org/models/test.xml");
  str = u->asText();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.example.org/models/test.xml"), str);
  u->asText(L"http://www.cellml.org/Members/miller/andres_models/"
            L"common/units.xml");
  u->release_ref();

//     /**
//      * The collection of all <component> elements inside this element.
//      */
//     readonly attribute ImportComponentSet components;
  iface::cellml_api::ImportComponentSet* ics = ci1->components();
  CPPUNIT_ASSERT_EQUAL(0, (int)ics->length());
  ics->release_ref();
  ics = ci2->components();
  CPPUNIT_ASSERT_EQUAL(1, (int)ics->length());
  ics->release_ref();

//     /**
//      * The collection of all <units> elements inside this element.
//      */
//     readonly attribute ImportUnitsSet units;
  iface::cellml_api::ImportUnitsSet* ius = ci1->units();
  CPPUNIT_ASSERT_EQUAL(13, (int)ius->length());
  ius->release_ref();
  ius = ci2->units();
  CPPUNIT_ASSERT_EQUAL(0, (int)ius->length());
  ius->release_ref();

//     /**
//      * Returns true if fullyInstantiate has previously been called on this
//      * import, or false otherwise. Note that a call to fullyInstantiateImports()
//      * on a model which is an ancestor of this one in the import tree shall be
//      * treated as if it were a call to fullyInstantiate() on all imports.
//      */
//     readonly attribute boolean wasInstantiated;
  CPPUNIT_ASSERT(!ci1->wasInstantiated());

//     /**
//      * Causes the model referenced by the href attribute to be loaded using the
//      * same model loader used to load this model.
//      */
//     void instantiate();
  mTenTusscher->fullyInstantiateImports();
  CPPUNIT_ASSERT(ci1->wasInstantiated());

//     /**
//      * The collection of all connections defined in the import or in components
//      * which are imported by the import(and so on) and which involve only
//      * the imported component or other components beneath the same imported
//      * component in the encapsulation hierarchy.
//      */
//     readonly attribute ConnectionSet importedConnections;
  iface::cellml_api::ConnectionSet* cs = NULL;
  CPPUNIT_ASSERT_NO_THROW(cs = ci2->importedConnections());
  int l;
  CPPUNIT_ASSERT_NO_THROW(l = (int)cs->length());
  CPPUNIT_ASSERT_EQUAL(83, l);
  cs->release_ref();

//    /**
//     * Returns NULL if imports have not been fully instantiated. Otherwise returns
//     * the imported Model corresponding to this CellMLImport.
//     */
//    readonly attribute Model importedModel;
  iface::cellml_api::Model* m = ci1->importedModel();
  str = m->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"units"), str);

  m->release_ref();

  CPPUNIT_ASSERT(ci1->wasInstantiated());
//    /**
//     * Causes the model to be uninstantiated, so that it can be re-instantiated
//     * in the future.
//     */
//    void uninstantiate();
  ci1->uninstantiate();
  CPPUNIT_ASSERT(!ci1->wasInstantiated());

//     /**
//      * A number that, within the parent model, uniquely represents the CellMLImport.
//      */
//     readonly attribute unsigned long uniqueIdentifier;
  ci1->release_ref();
  ci2->release_ref();
}
//   };

//   /**
//    * This interface represents a <component> element inside an <import>
//    * element.
//    */
//   interface ImportComponent
//     : CellMLComponent
//   {
void
CellMLTest::testImportComponent()
{
  loadTenTusscher();

  iface::cellml_api::CellMLComponentSet* cs =
    mTenTusscher->modelComponents();
  iface::cellml_api::CellMLComponent* cc = cs->getComponent(L"model");
  cs->release_ref();

  DECLARE_QUERY_INTERFACE_REPLACE(ic, cc, cellml_api::ImportComponent);

//     /**
//      * The name of the component referenced by this import component.
//      */
//     attribute CellMLAttributeString componentRef;
  std::wstring str = ic->componentRef();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"interface"), str);
  ic->componentRef(L"foobar");
  str = ic->componentRef();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);
  ic->componentRef(L"interface");

  mTenTusscher->fullyInstantiateImports();

  // Next, we retest the CellMLComponent methods, because import components are
  // supposed to transparently support them...

//     /**
//      * The set of all variables defined in this component.
//      */
//     readonly attribute CellMLVariableSet variables;
  iface::cellml_api::CellMLVariableSet* cvs = ic->variables();
  CPPUNIT_ASSERT_EQUAL(101, (int)cvs->length());
  cvs->release_ref();

//     /**
//      * The set of all units defined in this component.
//      */
//     readonly attribute UnitsSet units;
  iface::cellml_api::UnitsSet* us = ic->units();
  CPPUNIT_ASSERT_EQUAL(0, (int)us->length());
  us->release_ref();

//     /**
//      * The set of all connections which involve this component.
//      */
//     readonly attribute ConnectionSet connections;
  iface::cellml_api::ConnectionSet* conns = ic->connections();
  // There are 4+23=27 connections to model
  CPPUNIT_ASSERT_EQUAL(27, (int)conns->length());
  conns->release_ref();

//     /**
//      * The component which, in the encapsulation hierarchy defined in this
//      * model, encapsulates this component. When this component is
//      * an imported component, an implementation must return
//      * any encapsulation parent defined in the importing model.
//      * When the same component is imported twice under different
//      * names, the implementation must return the correct encapsulation
//      * parent for each component instance.
//      * This attribute is nil if there is no encapsulation parent.
//      */
//     readonly attribute CellMLComponent encapsulationParent;
  iface::cellml_api::CellMLComponent* c = ic->encapsulationParent();
  CPPUNIT_ASSERT(!c);

//     /**
//      * The set of all children of this component.
//      */
//     readonly attribute CellMLComponentSet encapsulationChildren;
  cs = ic->encapsulationChildren();
  CPPUNIT_ASSERT(cs);
  CPPUNIT_ASSERT_EQUAL(23, (int)cs->length());
  cs->release_ref();

  ic->release_ref();
}
//   };

//   /**
//    * This interface represents a <units> found in <import>
//    */
//   interface ImportUnits
//     : Units
//   {
void
CellMLTest::testImportUnits()
{
  loadTenTusscher();

  iface::cellml_api::UnitsSet* us =
    mTenTusscher->modelUnits();
  iface::cellml_api::Units* cu = us->getUnits(L"per_ms");
  us->release_ref();

  DECLARE_QUERY_INTERFACE_REPLACE(iu, cu, cellml_api::ImportUnits);

//     /**
//      * The name of the units referenced by this import component.
//      */
//     attribute CellMLAttributeString unitsRef;
  std::wstring str = iu->unitsRef();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"per_ms"), str);
  iu->unitsRef(L"foobar");
  str = iu->unitsRef();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);
  iu->unitsRef(L"per_ms");

  mTenTusscher->fullyInstantiateImports();

  CPPUNIT_ASSERT(!iu->isBaseUnits());
  iu->isBaseUnits(true);
  CPPUNIT_ASSERT(iu->isBaseUnits());  

  iface::cellml_api::UnitSet* uns = iu->unitCollection();
  CPPUNIT_ASSERT_EQUAL(1, (int)uns->length());
  uns->release_ref();

  iu->release_ref();
}
//   };

//   /**
//    * Represents a <variable> element in a CellML document.
//    */
//   interface CellMLVariable
//     : NamedCellMLElement
//   {
void
CellMLTest::testCellMLVariable()
{
  loadBeelerReuter();

  iface::cellml_api::CellMLComponentSet* cs = mBeelerReuter->localComponents();
  iface::cellml_api::CellMLComponent* c1 = cs->getComponent(L"membrane");
  iface::cellml_api::CellMLComponent* c2 = cs->getComponent(L"sodium_current");
  cs->release_ref();

  iface::cellml_api::CellMLVariableSet* vs = c1->variables();
  iface::cellml_api::CellMLVariable* v1 = vs->getVariable(L"V");
  vs->release_ref();
  vs = c2->variables();
  iface::cellml_api::CellMLVariable* v2 = vs->getVariable(L"time");
  iface::cellml_api::CellMLVariable* v3 = vs->getVariable(L"m");
  vs->release_ref();
  c1->release_ref();
  c2->release_ref();

//     /**
//      * The initial value of this variable.
//      */
//     attribute CellMLAttributeString initialValue;
  std::wstring str = v1->initialValue();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"-84.624"), str);
  v2->initialValue(L"123.45");
  str = v2->initialValue();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"123.45"), str);

//     /**
//      * The private interface direction of this variable.
//      */
//     attribute VariableInterface privateInterface;
  CPPUNIT_ASSERT(v1->privateInterface() == iface::cellml_api::INTERFACE_NONE);
  CPPUNIT_ASSERT(v2->privateInterface() == iface::cellml_api::INTERFACE_OUT);
  CPPUNIT_ASSERT(v3->privateInterface() == iface::cellml_api::INTERFACE_IN);
  v1->privateInterface(iface::cellml_api::INTERFACE_OUT);
  CPPUNIT_ASSERT(v1->privateInterface() == iface::cellml_api::INTERFACE_OUT);

//     /**
//      * The public interface direction of this variable.
//      */
//     attribute VariableInterface publicInterface;
  CPPUNIT_ASSERT(v1->publicInterface() == iface::cellml_api::INTERFACE_OUT);
  CPPUNIT_ASSERT(v2->publicInterface() == iface::cellml_api::INTERFACE_IN);
  CPPUNIT_ASSERT(v3->publicInterface() == iface::cellml_api::INTERFACE_NONE);
  v3->publicInterface(iface::cellml_api::INTERFACE_IN);
  CPPUNIT_ASSERT(v3->publicInterface() == iface::cellml_api::INTERFACE_IN);

//     /**
//      * The collection of all variables connected to this variable.
//      */
//     readonly attribute CellMLVariableSet connectedVariables;
  iface::cellml_api::CellMLVariableSet* cvs = v1->connectedVariables();
  CPPUNIT_ASSERT_EQUAL(10, (int)cvs->length());
  cvs->release_ref();

//     /**
//      * The source variable is always a variable on which f(v) holds, and is
//      * this variable, or a variable which is(directly or indirectly) connected
//      * to this variable. If no such variable exists, sourceVariable is nil.
//      * This must work correctly across import boundaries, if those imports
//      * have been fully instantiated.
//      * f(v) is defined as meaning
//      *   (v.public_interface == INTERFACE_OUT ||
//      *    v.public_interface == INTERFACE_NONE) &&
//      *   (v.private_interface == INTERFACE_OUT ||
//      *    v.private_interface == INTERFACE_NONE)
//      * 
//      */
//     readonly attribute CellMLVariable sourceVariable;
  iface::cellml_api::CellMLVariable* v4 = v1->sourceVariable();
  // Should be the same...
  CPPUNIT_ASSERT(CDA_objcmp(v4, v1) == 0);
  v4->release_ref();
  v4 = v2->sourceVariable();
  iface::cellml_api::CellMLElement* el = v4->parentElement();
  DECLARE_QUERY_INTERFACE_REPLACE(c3, el, cellml_api::CellMLComponent);
  str = c3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"environment"), str);
  c3->release_ref();
  v4->release_ref();

//     /**
//      * The name of the component containing this CellMLVariable.
//      */
//     readonly attribute CellMLAttributeString componentName;
  str = v1->componentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"membrane"), str);

  str = v1->unitsName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millivolt"), str);

  str = v2->unitsName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millisecond"), str);

  str = v3->unitsName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"dimensionless"), str);

  v3->unitsName(L"millivolt");
  str = v3->unitsName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millivolt"), str);
  v3->unitsName(L"dimensionless");

  iface::cellml_api::Units* u1 = v1->unitsElement();
  str = u1->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millivolt"), str);

  iface::cellml_api::Units* u2 = v2->unitsElement();
  str = u2->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millisecond"), str);
  u2->release_ref();

  v2->unitsElement(u1);

  u2 = v2->unitsElement();
  str = u2->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"millivolt"), str);
  u2->release_ref();

  u1->release_ref();

  v1->release_ref();
  v2->release_ref();
  v3->release_ref();
}
//   };

//   /**
//    * Represents a <group> element.
//    */
//   interface Group : CellMLElement
//   {
void
CellMLTest::testGroup()
{
  loadBeelerReuter();
  iface::cellml_api::GroupSet* gs = mBeelerReuter->groups();
  iface::cellml_api::GroupSet* egs = gs->subsetInvolvingEncapsulation();
  gs->release_ref();
  iface::cellml_api::GroupIterator* gi = egs->iterateGroups();
  iface::cellml_api::Group* g = gi->nextGroup();
  gi->release_ref();
  egs->release_ref();

//     /**
//      * The collection of relationship references.
//      */
//     readonly attribute RelationshipRefSet relationshipRefs;
  iface::cellml_api::RelationshipRefSet* rrs = g->relationshipRefs();
  CPPUNIT_ASSERT_EQUAL(1, (int)rrs->length());
  rrs->release_ref();

//     /**
//      * The collection of component references.
//      */
//     readonly attribute ComponentRefSet componentRefs;
  iface::cellml_api::ComponentRefSet* crs = g->componentRefs();
  CPPUNIT_ASSERT_EQUAL(3, (int)crs->length());
  crs->release_ref();

//     /**
//      * True if this is an encapsulation relationship
//      * (with relationship on relationship_ref in the CellML namespace)
//      * and false otherwise.
//      */
//     readonly attribute boolean isEncapsulation;
  CPPUNIT_ASSERT(g->isEncapsulation());

//     /**
//      * True if this is a containment relationship
//      * (with relationship on relationship_ref in the CellML namespace)
//      * and false otherwise.
//      */
//     readonly attribute boolean isContainment;
  CPPUNIT_ASSERT(!g->isContainment());

  g->release_ref();
}
//   };

//   /**
//    * Represents a <component_ref> element in a group element or in another
//    * <component_ref> element.
//    */
//   interface ComponentRef
//     : CellMLElement

//   {
void
CellMLTest::testComponentRef()
{
  loadBeelerReuter();
  iface::cellml_api::GroupSet* gs = mBeelerReuter->groups();
  iface::cellml_api::GroupSet* egs = gs->subsetInvolvingEncapsulation();
  gs->release_ref();
  iface::cellml_api::GroupIterator* gi = egs->iterateGroups();
  iface::cellml_api::Group* g = gi->nextGroup();
  gi->release_ref();
  egs->release_ref();
  iface::cellml_api::ComponentRefSet* crs = g->componentRefs();
  iface::cellml_api::ComponentRefIterator* cri = crs->iterateComponentRefs();
  crs->release_ref();
  iface::cellml_api::ComponentRef* cr = cri->nextComponentRef();
  cri->release_ref();

//     /**
//      * The name of the component being referenced.
//      */
//     attribute CellMLAttributeString componentName;
  std::wstring str = cr->componentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"sodium_current"), str);
  cr->componentName(L"foobar");
  str = cr->componentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);

//     /**
//      * A collection of component references which are beneath this one in the
//      * group tree.
//      */
//     readonly attribute ComponentRefSet componentRefs;
  crs = cr->componentRefs();
  CPPUNIT_ASSERT_EQUAL(3, (int)crs->length());
  cri = crs->iterateComponentRefs();
  crs->release_ref();
  iface::cellml_api::ComponentRef* cr2 = cri->nextComponentRef();
  cri->release_ref();


//     /**
//      * The parent component reference, or nil if this is the toplevel
//      * component reference.
//      */
//     readonly attribute ComponentRef parentComponentRef;
  CPPUNIT_ASSERT(!cr->parentComponentRef().getPointer());
  iface::cellml_api::ComponentRef* cr3 = cr2->parentComponentRef();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(cr, cr3));
  cr3->release_ref();

//     /**
//      * The parent group in which the component reference is found.
//      */
//     readonly attribute Group parentGroup;
  iface::cellml_api::Group* g2 = cr->parentGroup();
  CPPUNIT_ASSERT(CDA_objcmp(g, g2) == 0);
  g2->release_ref();
  g2 = cr2->parentGroup();
  CPPUNIT_ASSERT(CDA_objcmp(g, g2) == 0);
  g2->release_ref();

  g->release_ref();
  cr->release_ref();
  cr2->release_ref();
}
//   };

//   /**
//    * Represents a relationship_ref element in a group element.
//    */
//   interface RelationshipRef
//     : CellMLElement
//   {
void
CellMLTest::testRelationshipRef()
{
  loadBeelerReuter();
  iface::cellml_api::GroupSet* gs = mBeelerReuter->groups();
  iface::cellml_api::GroupSet* egs = gs->subsetInvolvingEncapsulation();
  gs->release_ref();
  iface::cellml_api::GroupIterator* gi = egs->iterateGroups();
  iface::cellml_api::Group* g = gi->nextGroup();
  gi->release_ref();
  egs->release_ref();
  iface::cellml_api::RelationshipRefSet* rrs = g->relationshipRefs();
  g->release_ref();
  iface::cellml_api::RelationshipRefIterator* rri = rrs->iterateRelationshipRefs();
  rrs->release_ref();
  iface::cellml_api::RelationshipRef* rr = rri->nextRelationshipRef();
  rri->release_ref();

//     /**
//      * The name of this relationship_ref, or the empty string if no such name
//      * is defined.
//      */
//     attribute CellMLAttributeString name;
  std::wstring str = rr->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);
  rr->name(L"barfoo");
  str = rr->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"barfoo"), str);

//     /**
//      * The name of the relationship referenced by this element.
//      */
//     readonly attribute CellMLAttributeString relationship;
  CPPUNIT_ASSERT_NO_THROW(str = rr->relationship());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"encapsulation"), str);

//     /**
//      * The namespace in which the relationship attribute belongs.
//      */
//     readonly attribute CellMLAttributeString relationshipNamespace;
  CPPUNIT_ASSERT_NO_THROW(str = rr->relationshipNamespace());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L""), str);

//     /**
//      * Sets the namespace and relationship name. This will remove any other
//      * relationship attributes in any namespace.
//      * @param namespaceURI The URI of the namespace 
//      * @param name The name of the relationship.
//      */
//     void setRelationshipName(in CellMLAttributeString namespaceURI,
//                              in CellMLAttributeString name);
  CPPUNIT_ASSERT_NO_THROW
    (rr->setRelationshipName(L"http://www.example.org/examplerelationship/"
                             L"namespace", L"testrelationship"));
  CPPUNIT_ASSERT_NO_THROW(str = rr->relationship());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"testrelationship"), str);
  CPPUNIT_ASSERT_NO_THROW(str = rr->relationshipNamespace());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.example.org/examplerelationship/"
                                    L"namespace"), str);

  rr->release_ref();
}
//   };

//   /**
//    * Represents a <connection> element.
//    */
//   interface Connection
//     : CellMLElement
//   {
void
CellMLTest::testConnection()
{
  loadBeelerReuter();
  iface::cellml_api::ConnectionSet* cs = mBeelerReuter->connections();
  iface::cellml_api::ConnectionIterator* ci = cs->iterateConnections();
  cs->release_ref();
  iface::cellml_api::Connection* c = ci->nextConnection();
  ci->release_ref();

//     /**
//      * The map_components element in this connection.
//      */
//     readonly attribute MapComponents componentMapping;
  iface::cellml_api::MapComponents* mc = c->componentMapping();
  // Might as well test MapComponents now too...
//   /**
//    * An interface the map_components element.
//    */
//   interface MapComponents
//     : CellMLElement
//   {
//     /**
//      * The name of the first component. The returned name shall be the same
//      * name as found in the component_1 attribute(that is, the implementation
//      * shall not translate the name of imported components).
//      */
//     attribute CellMLAttributeString firstComponentName;
  std::wstring str = mc->firstComponentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"membrane"), str);
  mc->firstComponentName(L"foobar");
  str = mc->firstComponentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);
  mc->firstComponentName(L"membrane");

//     /**
//      * The name of the second component. The returned name shall be the same
//      * name as found in the component_2 attribute(that is, the implementation
//      * shall not translate the name of imported components).
//      */
//     attribute CellMLAttributeString secondComponentName;
  str = mc->secondComponentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"environment"), str);
  mc->secondComponentName(L"barfoo");
  str = mc->secondComponentName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"barfoo"), str);
  mc->secondComponentName(L"environment");

//     /**
//      * The first component. This component shall be the component definition if
//      * imports have been instantiated, and otherwise shall be the ImportComponent.
//      */
//     attribute CellMLComponent firstComponent;
  iface::cellml_api::CellMLComponent* comp = mc->firstComponent();
  str = comp->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"membrane"), str);
//     /**
//      * The second component. This component shall be the component definition if
//      * imports have been instantiated, and otherwise shall be the ImportComponent.
//      */
//     attribute CellMLComponent secondComponent;
  iface::cellml_api::CellMLComponent* comp2 = mc->secondComponent();
  str = comp2->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"environment"), str);

  // Switch the components...
  CPPUNIT_ASSERT_NO_THROW(mc->firstComponent(comp2));
  CPPUNIT_ASSERT_NO_THROW(mc->secondComponent(comp));

  iface::cellml_api::CellMLComponent* comp3 = NULL;
  CPPUNIT_ASSERT_NO_THROW(comp3 = mc->firstComponent());
  str = comp3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"environment"), str);
  comp3->release_ref();

  CPPUNIT_ASSERT_NO_THROW(comp3 = mc->secondComponent());
  str = comp3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"membrane"), str);
  comp3->release_ref();
//   }; 
  // Switch them back so next tests work...

  CPPUNIT_ASSERT_NO_THROW(mc->firstComponent(comp));
  CPPUNIT_ASSERT_NO_THROW(mc->secondComponent(comp2));

  mc->release_ref();

//     /**
//      * A collection of the map_variables elements in this connection.
//      */
//     readonly attribute MapVariablesSet variableMappings;
  iface::cellml_api::MapVariablesSet* mvs = c->variableMappings();
  iface::cellml_api::MapVariablesIterator* mvi  = mvs->iterateMapVariables();
  mvs->release_ref();
  iface::cellml_api::MapVariables* mv = mvi->nextMapVariable();
  mvi->release_ref();

  // Might as well test MapVariables now too...

//   /**
//    * An interface the map_variables element.
//    */
//   interface MapVariables
//     : CellMLElement
//   {
//     /**
//      * The name of the first variable. The variable name is interpreted only in
//      * the context of MapComponents, and so may only be read or set after the
//      * MapVariables is added to a Connection, and the Connection has in turn
//      * been added to a model. It also requires that the component name has
//      * already been set.
//      * @raises CellMLException if the MapVariables has not been added to a
//      *                         Connection, or the Connection has not been added
//      *                         to the Model.
//      */
//     attribute CellMLAttributeString firstVariableName;
  str = mv->firstVariableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"time"), str);
  mv->firstVariableName(L"time2");
  str = mv->firstVariableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"time2"), str);
  mv->firstVariableName(L"time");

//     /**
//      * The name of the second variable. The variable name is interpreted only in
//      * the context of MapComponents, and so may only be read or set after the
//      * MapVariables is added to a Connection, and the Connection has in turn
//      * been added to a model. It also requires that the component name has
//      * already been set.
//      * @raises CellMLException if the MapVariables has not been added to a
//      *                         Connection, or the Connection has not been added
//      *                         to the Model.
//      */
//     attribute CellMLAttributeString secondVariableName;
  str = mv->secondVariableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"footime"), str);
  mv->secondVariableName(L"bartime");
  str = mv->secondVariableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"bartime"), str);
  mv->secondVariableName(L"footime");

//     /**
//      * The first variable. If this has already been added to a Connection, it
//      * must refer to the same Component as firstComponent(if it is set). If the
//      * component in the MapComponents has not been set, this implicitly sets it.
//      * If this has not been added to the Connection, the checks are performed
//      * at the time it is added.
//      */
//     attribute CellMLVariable firstVariable;
  iface::cellml_api::CellMLVariable* v = mv->firstVariable();
  str = v->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"time"), str);

//     /**
//      * The second variable. If this has already been added to a Connection, it
//      * must refer to the same Component as secondComponent(if it is set). If the
//      * component in the MapComponents has not been set, this implicitly sets it.
//      * If this has not been added to the Connection, the checks are performed
//      * at the time it is added.
//      */
//     attribute CellMLVariable secondVariable;
  iface::cellml_api::CellMLVariable* v2 = mv->secondVariable();
  str = v2->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"footime"), str);

  // Switch the components...
  CPPUNIT_ASSERT_NO_THROW(mc->firstComponent(comp2));
  CPPUNIT_ASSERT_NO_THROW(mc->secondComponent(comp));
  comp->release_ref();
  comp2->release_ref();

  // Switch the variables...
  CPPUNIT_ASSERT_NO_THROW(mv->firstVariable(v2));
  CPPUNIT_ASSERT_NO_THROW(mv->secondVariable(v));
  v->release_ref();
  v2->release_ref();

  iface::cellml_api::CellMLVariable* v3 = NULL;
  CPPUNIT_ASSERT_NO_THROW(v3 = mv->firstVariable());
  str = v3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"footime"), str);
  v3->release_ref();

  v3 = mv->secondVariable();
  str = v3->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"time"), str);
  v3->release_ref();

  mv->release_ref();
//   };

  c->release_ref();
}
//   };


//  /**
//   * An interface to a <reaction> element.
//   */
//  interface Reaction
//    : CellMLElement
//  {
void
CellMLTest::testReaction()
{
  loadGlycolysis();

  iface::cellml_api::CellMLComponentSet* cs = mGlycolysis->localComponents();
  iface::cellml_api::CellMLComponent* c =
    cs->getComponent(L"glucose_transport_system");
  cs->release_ref();

  iface::cellml_api::ReactionSet* rs = c->reactions();
  iface::cellml_api::ReactionIterator* ri = rs->iterateReactions();
  rs->release_ref();
  iface::cellml_api::Reaction* r = ri->nextReaction();
  ri->release_ref();

//    /**
//     * A collection of all the reactants involved in this reaction.
//     */
//    readonly attribute VariableRefSet variableReferences;
  iface::cellml_api::VariableRefSet* vrs = NULL;
  CPPUNIT_ASSERT_NO_THROW(vrs = r->variableReferences());
  int l;
  CPPUNIT_ASSERT_NO_THROW(l = (int)vrs->length());
  CPPUNIT_ASSERT_EQUAL(5, l);
  vrs->release_ref();

//
//    /**
//      * Whether or not the reaction is reversible. May be yes, no,
//      * or the empty string(if the attribute is absent).
//      */
//     attribute boolean reversible;
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(r->reversible()));
  CPPUNIT_ASSERT_NO_THROW(r->reversible(false));
  CPPUNIT_ASSERT_NO_THROW(CPPUNIT_ASSERT(!r->reversible()));

//     /**
//      * Fetches a VariableRef by the variable name.
//      * @param varName The name of the variable, within this component.
//      * @param create Whether or not to create a VariableRef if not found.
//      * @return The variable reference corresponding to this variable.
//      *   If no such VariableRef exists, then:
//      *     i) If create is true, a new one is created and added to this
//      *        reaction.
//      *    ii) If create is false, NULL is returned.
//      */
//     VariableRef getVariableRef(in wstring varName, in boolean create);
  iface::cellml_api::VariableRef* vr = NULL;
  CPPUNIT_ASSERT_NO_THROW(vr = r->getVariableRef(L"G6P", false));

  // Now test VariableRef...
//   /**
//    * An interface to the <variable_ref> element.
//    */
//   interface VariableRef
//     : CellMLElement
//   {
//     /**
//      * The name of the variable being referenced.
//      */
//     attribute wstring variableName;
  std::wstring str = vr->variableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"G6P"), str);

//     /**
//      * The variable being referenced. This may be assigned to a variable in the
//      * same component.
//      */
//     attribute CellMLVariable variable;
  iface::cellml_api::CellMLVariable* v = vr->variable();
  str = v->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"G6P"), str);
  v->release_ref();
  iface::cellml_api::CellMLVariableSet* vs = c->variables();
  c->release_ref();
  CPPUNIT_ASSERT_NO_THROW(v = vs->getVariable(L"PERM"));
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_NO_THROW(vr->variable(v));
  v->release_ref();
  CPPUNIT_ASSERT_NO_THROW(v = vr->variable());
  CPPUNIT_ASSERT_NO_THROW(str = v->name());
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"PERM"), str);
  v->release_ref();
  str = vr->variableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"PERM"), str);

  // Now try setting the variable name...
  vr->variableName(L"test123");
  str = vr->variableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"test123"), str);

  // Now try setting the VariableRef

//     /**
//      * A collection of all the Roles that this VariableRef plays in this reaction.
//      */
//     readonly attribute RoleSet roles;
  iface::cellml_api::RoleSet* rls = vr->roles();
  CPPUNIT_ASSERT_EQUAL(1, (int)rls->length());
  iface::cellml_api::RoleIterator* rli =  rls->iterateRoles();
  rls->release_ref();
  iface::cellml_api::Role* rl = rli->nextRole();
  rli->release_ref();

//   /**
//    * An interface to the <role> element.
//    */
//   interface Role
//     : CellMLElement, MathContainer
//   {
//     enum RoleType
//     {
//       REACTANT, PRODUCT, RATE, CATALYST, ACTIVATOR, INHIBITOR, MODIFIER
//     };

//     enum DirectionType
//     {
//       FORWARD, REVERSE, BOTH
//     };

//     /**
//      * The role this variable plays.
//      */
//     attribute RoleType variableRole;
  CPPUNIT_ASSERT(rl->variableRole() == iface::cellml_api::Role::INHIBITOR);
  rl->variableRole(iface::cellml_api::Role::ACTIVATOR);
  CPPUNIT_ASSERT(rl->variableRole() == iface::cellml_api::Role::ACTIVATOR);

//     /**
//      * The direction of the reaction for which this role applies.
//      */
//     attribute DirectionType direction;
  CPPUNIT_ASSERT(rl->direction() == iface::cellml_api::Role::FORWARD);
  rl->direction(iface::cellml_api::Role::REVERSE);
  CPPUNIT_ASSERT(rl->direction() == iface::cellml_api::Role::REVERSE);

//     /**
//      * The stoichiometry for this role.
//      */
//     attribute double stoichiometry;
  rl->stoichiometry(4.5);
  CPPUNIT_ASSERT_EQUAL(4.5, rl->stoichiometry());

//     /**
//      * The variable represented by the delta_variable attribute.
//      */
//     attribute CellMLVariable deltaVariable;
  // Tested below...

//     /**
//      * The name of the variable represented by the delta_variable attribute.
//      */
//     attribute wstring deltaVariableName;
  // Tested below...
//   };

  rl->release_ref();

//   };
  CPPUNIT_ASSERT(vr);
  vr->release_ref();

  CPPUNIT_ASSERT_NO_THROW(vr = r->getVariableRef(L"foo", false));
  CPPUNIT_ASSERT(!vr);

  CPPUNIT_ASSERT_NO_THROW(vr = r->getVariableRef(L"foo", true));
  CPPUNIT_ASSERT(vr);
  vr->release_ref();

  CPPUNIT_ASSERT_NO_THROW(vr = r->getVariableRef(L"foo", false));
  CPPUNIT_ASSERT(vr);
  vr->release_ref();

//     /**
//      * Fetches a Role by the delta variable name.
//      * @param varName The name of the delta variable, within this component.
//      * @return The Role corresponding to this delta variable.
//      * If no such Role exists, then NULL is returned.
//      */
//     Role getRoleByDeltaVariable(in wstring varName);
  CPPUNIT_ASSERT_NO_THROW(rl = r->getRoleByDeltaVariable
                          (L"delta_Glc_C_rxn1"));
  str = rl->deltaVariableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"delta_Glc_C_rxn1"), str);
  v = rl->deltaVariable();
  str = v->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"delta_Glc_C_rxn1"), str);
  v->release_ref();

  v = vs->getVariable(L"delta_Glc_rxn1");
  vs->release_ref();
  CPPUNIT_ASSERT_NO_THROW(rl->deltaVariable(v));
  v->release_ref();

  str = rl->deltaVariableName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"delta_Glc_rxn1"), str);
  v = rl->deltaVariable();
  str = v->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"delta_Glc_rxn1"), str);
  v->release_ref();

  rl->release_ref();
  r->release_ref();
}
//   };

//   /**
//    * A collection of extension elements.
//    */
//   interface ExtensionElementList : XPCOM::IObject
//   {
void
CellMLTest::testExtensionElementList()
{
  loadAchCascade();
  iface::cellml_api::ExtensionElementList* eel = mAchCascade->extensionElements();
//     /**
//      * The length of the collection.
//      */
//     readonly attribute unsigned long length;
  CPPUNIT_ASSERT_EQUAL(5, (int)eel->length());
//     /**
//      * Fetches the extension element at a certain index(starting from 0).
//      * @param index The index at which to fetch the extension element.
//      */
//     ExtensionElement getAt(in unsigned long index);
  iface::dom::Element* el = eel->getAt(3);
  std::wstring str = el->localName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"abc"), str);

//     /**
//      * Finds the index of the given extension element.
//      * @param x The extension element to find.
//      * @return The index(first is 0) of the extension element, or -1 if the
//      *         element is not found.
//      */
//     long getIndexOf(in ExtensionElement x);
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(3), eel->getIndexOf(el));
  iface::dom::Node* notinlist = el->parentNode();
  DECLARE_QUERY_INTERFACE_REPLACE(elnotinlist, notinlist, dom::Element);
  CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(-1), eel->getIndexOf(elnotinlist));
//     /**
//      * Tests for the existance of an element in the set.
//      * @param x The element to test for.
//      * @return true if the element is present, or false otherwise.
//      */
//     boolean contains(in ExtensionElement x);
  CPPUNIT_ASSERT(eel->contains(el));
  CPPUNIT_ASSERT(!eel->contains(elnotinlist));

  el->release_ref();
  elnotinlist->release_ref();
  eel->release_ref();
}
//   };

void
CellMLTest::testExtensionAttributeSet()
{
  loadAchCascade();

//    /**
//     * Fetches an extension attribute.
//     * @param ns The namespace of the extension attribute to fetch.
//     * @param localName The local name of the attribute to fetch.
//     */
//    wstring getExtensionAttributeNS(in wstring ns, in wstring localName);
  std::wstring str =
    mAchCascade->getExtensionAttributeNS(L"http://example.org/mytest", L"hello");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"world"), str);

  str =
    mAchCascade->getExtensionAttributeNS(L"http://example.org/yourtest", L"hello");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L""), str);

//    /**
//     * Sets an extension attribute (adding it if it doesn't already exist,
//     * otherwise replacing it).
//     * @param ns The namespace of the extension attribute to set.
//     * @param qualifiedName The qualified name of the attribute to set.
//     * @param value The value to set the attribute to.
//     */
//    void setExtensionAttributeNS(in wstring ns, in wstring qualifiedName, in wstring value);
  mAchCascade->setExtensionAttributeNS(L"http://example.org/newtest", L"something:thistest", L"123");
  mAchCascade->setExtensionAttributeNS(L"http://example.org/thetest", L"nsblah:atest", L"456");
  str =
    mAchCascade->getExtensionAttributeNS(L"http://example.org/newtest", L"thistest");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"123"), str);

//    /**
//     * Removes an extension attribute. No action is taken if the attribute is
//     * not already present.
//     * @param ns The namespace of the extension attribute to remove.
//     * @param localName The local name of the attribute to remove.
//     */
//    void removeExtensionAttributeNS(in wstring ns, in wstring localName);
  mAchCascade->removeExtensionAttributeNS(L"http://example.org/newtest", L"thistest");
  str =
    mAchCascade->getExtensionAttributeNS(L"http://example.org/newtest", L"thistest");
  CPPUNIT_ASSERT_EQUAL(std::wstring(L""), str);

//
//    /**
//     * Fetches the set of all extension attributes, which can be used to iterate
//     * through the extension attributes.
//     */
//    readonly attribute ExtensionAttributeSet extensionAttributes;
  iface::cellml_api::ExtensionAttributeSet* eas = mAchCascade->extensionAttributes();
  CPPUNIT_ASSERT(eas != NULL);

//  /**
//   * A set of extension attributes
//   */
//  interface ExtensionAttributeSet : XPCOM::IObject
//  {
//    /**
//     * Returns a CellMLElementIterator that can be used to iterate through the
//     * attributes. The iteration order is undefined.
//     */
//    ExtensionAttributeIterator iterate();
  iface::cellml_api::ExtensionAttributeIterator* eai = eas->iterate();
  eas->release_ref();

//  };
//  /**
//   * An interface for iterating through all extension attributes.
//   */
//  interface ExtensionAttributeIterator : XPCOM::IObject
//  {
//    /**
//     * Fetches the next extension attribute node.
//     */
//    dom::Attr nextAttribute();
//  };
  // Test that the iterator is live...
  mAchCascade->setExtensionAttributeNS(L"http://example.org/livetest", L"shouldbe", L"live");

  iface::dom::Attr* attr1 = eai->nextAttribute();
  CPPUNIT_ASSERT(attr1 != NULL);
  iface::dom::Attr* attr2 = eai->nextAttribute();
  CPPUNIT_ASSERT(attr2 != NULL);
  iface::dom::Attr* attr3 = eai->nextAttribute();
  CPPUNIT_ASSERT(attr3 != NULL);
  // And three more because of the xmlns attributes.
  iface::dom::Attr* attr4 = eai->nextAttribute();
  CPPUNIT_ASSERT(attr4 != NULL);
  iface::dom::Attr* attr5 = eai->nextAttribute();
  CPPUNIT_ASSERT(attr5 != NULL);
  iface::dom::Attr* attr6 = eai->nextAttribute();
  CPPUNIT_ASSERT(attr6 != NULL);

  CPPUNIT_ASSERT(!eai->nextAttribute().getPointer());

  std::wstring attr1ns = attr1->namespaceURI(), attr2ns = attr2->namespaceURI(), attr3ns = attr3->namespaceURI(),
    attr4ns = attr4->namespaceURI(), attr5ns = attr5->namespaceURI(), attr6ns = attr6->namespaceURI();
  std::wstring attr1ln = attr1->localName(), attr2ln = attr2->localName(), attr3ln = attr3->localName(),
    attr4ln = attr4->localName(), attr5ln = attr5->localName(), attr6ln = attr6->localName();

  attr1->release_ref();
  attr2->release_ref();
  attr3->release_ref();
  attr4->release_ref();
  attr5->release_ref();
  attr6->release_ref();

  if (attr2ns == L"http://example.org/thetest")
  {
    std::wstring tmp = attr2ns;
    attr2ns = attr1ns;
    attr1ns = tmp;
    tmp = attr2ln;
    attr2ln = attr1ln;
    attr1ln = tmp;
  }
  else if (attr3ns == L"http://example.org/thetest")
  {
    std::wstring tmp = attr3ns;
    attr3ns = attr1ns;
    attr1ns = tmp;
    tmp = attr3ln;
    attr3ln = attr1ln;
    attr1ln = tmp;
  }
  if (attr3ns == L"http://example.org/hello")
  {
    std::wstring tmp = attr3ns;
    attr3ns = attr2ns;
    attr2ns = tmp;
    tmp = attr3ln;
    attr3ln = attr2ln;
    attr2ln = tmp;
  }

  // printf("%S %S %S %S %S %S %S %S %S %S %S %S\n", attr1ns, attr1ln,
  //        attr2ns, attr2ln, attr3ns, attr3ln, attr4ns, attr4ln, attr5ns,
  //        attr5ln, attr6ns, attr6ln);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://example.org/thetest"), attr1ns);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"atest"), attr1ln);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.w3.org/2000/xmlns/"), attr2ns);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"cellml"), attr2ln);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.w3.org/2000/xmlns/"), attr3ns);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"cmeta"), attr3ln);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://example.org/mytest"), attr4ns);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"hello"), attr4ln);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.w3.org/2000/xmlns/"), attr5ns);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"mytest"), attr5ln);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://example.org/livetest"), attr6ns);
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"shouldbe"), attr6ln);

  eai->release_ref();
}

void
CellMLTest::testCellMLElementSet()
{
  loadBeelerReuter();
  iface::cellml_api::UnitsSet* us = mBeelerReuter->localUnits();
//   /**
//    * An immutable collection of CellML elements.
//    */
//   interface CellMLElementSet : XPCOM::IObject
//   {
//     /**
//      * The length of the collection.
//      */
//     readonly attribute unsigned long length;
  CPPUNIT_ASSERT_EQUAL(10, (int)us->length());

//     /**
//      * Tests for the existance of an element in the set.
//      * @param x The element to test for.
//      * @return true if the element is present, or false otherwise.
//      */
//     boolean contains(in CellMLElement x);
  iface::cellml_api::Units* u = us->getUnits(L"microF_per_cm2");
  CPPUNIT_ASSERT(us->contains(u));
  u->release_ref();

//     /**
//      * Returns a CellMLElementIterator that can be used to iterate through the
//      * elements. The iteration order is undefined.
//      */
//     CellMLElementIterator iterate();
  // This is tested in many places.
//   };
  us->release_ref();
}

#define CELLML_1_1_NS L"http://www.cellml.org/cellml/1.1#"

void
CellMLTest::testIteratorLiveness()
{
  loadBeelerReuter();
  // Note: In the University of Auckland CellML API implementation, the
  // iterator code responsible for liveness is shared between all types of
  // iterators. If this is not the case for a different implementation you are
  // testing, you should probably write more testcases.

  iface::cellml_api::ExtensionElementList* eel =
    mBeelerReuter->extensionElements();
  iface::dom::Element* el = eel->getAt(0);
  eel->release_ref();
  iface::dom::Node* n = el->parentNode();
  el->release_ref();
  QUERY_INTERFACE_REPLACE(el, n, dom::Element);

  iface::dom::Document* doc = el->ownerDocument();
  
  iface::cellml_api::UnitsSet* us = mBeelerReuter->localUnits();
  iface::cellml_api::UnitsIterator* ui = us->iterateUnits();

  // Add a new element right at the beginning...
  iface::dom::Node* refnode = el->firstChild();
  iface::dom::Element* newel = doc->createElementNS(CELLML_1_1_NS, L"units");
  el->insertBefore(newel, refnode)->release_ref();
  newel->setAttributeNS(L"", L"name", L"newunits");
  refnode->release_ref();

  iface::cellml_api::Units* u = ui->nextUnits();
  std::wstring str = u->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"newunits"), str);
  u->release_ref();

  n = newel->nextSibling();

  iface::dom::Element* newel2 =
    doc->createElementNS(CELLML_1_1_NS, L"units");
  newel2->setAttributeNS(L"", L"name", L"newunits2");
  el->insertBefore(newel2, n)->release_ref();
  n->release_ref();
  newel2->release_ref();

  u = ui->nextUnits();
  str = u->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"newunits2"), str);
  mBeelerReuter->removeElement(u);
  u->release_ref();

  u = ui->nextUnits();
  str = u->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"notarealunit"), str);
  mBeelerReuter->removeElement(u);
  u->release_ref();

  n = newel->nextSibling();
  newel->release_ref();
  iface::dom::Node* np = n->parentNode();
  bool firstHit = false;
  while (true)
  {
    iface::dom::Node* n2 = n->nextSibling();
    np->removeChild(n)->release_ref();
    n->release_ref();
    n = n2;

    str = n->nodeName();
    if (str == L"units")
    {
      if (!firstHit)
      {
        firstHit = true;
      }
      else
        break;
    }
  }

  newel2 =
    doc->createElementNS(CELLML_1_1_NS, L"units");
  newel2->setAttributeNS(L"", L"name", L"newunits3");
  el->insertBefore(newel2, n)->release_ref();
  np->removeChild(n)->release_ref();
  np->release_ref();
  n->release_ref();
  newel2->release_ref();

  u = ui->nextUnits();
  str = u->name();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"newunits3"), str);
  mBeelerReuter->removeElement(u);
  u->release_ref();

  us->release_ref();
  ui->release_ref();
  doc->release_ref();
  el->release_ref();
}

void
CellMLTest::testRelativeImports()
{
  loadRelativeURIModel();
  iface::cellml_api::CellMLComponentSet* ccs = mRelativeURI->allComponents();
  CPPUNIT_ASSERT_EQUAL(1, (int)ccs->length());
  iface::cellml_api::CellMLComponentIterator* cci = ccs->iterateComponents();
  ccs->release_ref();
  iface::cellml_api::CellMLComponent* c = cci->nextComponent();
  cci->release_ref();

  std::wstring ret = c->cmetaId();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"level2_component"), ret);
  c->release_ref();
}

void
CellMLTest::testImportClone()
{
  loadRelativeURIModel();

  //    /**
  //     * Clones a Model, including all of its imports.
  //     * Will cause the model to be fully instantiated.
  //     * @raises CellMLException if problems occur instantiating imports.
  //     */
  //    CellMLElement cloneAcrossImports() raises(CellMLException);

  iface::cellml_api::Model* m = mRelativeURI->cloneAcrossImports();
  iface::cellml_api::CellMLImportSet* is = m->imports();
  m->release_ref();
  iface::cellml_api::CellMLImportIterator* ii = is->iterateImports();
  is->release_ref();
  iface::cellml_api::CellMLImport* imp = ii->nextImport();
  ii->release_ref();
  iface::cellml_api::Model* mod = imp->importedModel();
  imp->release_ref();
  std::wstring name = mod->name();
  mod->release_ref();

  CPPUNIT_ASSERT_EQUAL(std::wstring(L"level1"), name);
}
