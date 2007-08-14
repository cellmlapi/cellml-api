#include "DOMTest.hxx"
#include "DOMWriter.hxx"
#include "Utilities.hxx"
#include "IfaceCellML_APISPEC.hxx"
#include "CellMLBootstrap.hpp"

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

CPPUNIT_TEST_SUITE_REGISTRATION( DOMTest );

void
DOMTest::setUp()
{
  di = NULL;
  dt = NULL;
  doc = NULL;
  CPPUNIT_ASSERT_NO_THROW(di = CreateDOMImplementation());
  CPPUNIT_ASSERT_NO_THROW(dt = di->createDocumentType(L"foo", L"bar", L"baz"));
  CPPUNIT_ASSERT_NO_THROW(doc = di->createDocument(L"http://www.physiome.org/testsuite/1",
                                                   L"tests", dt));
}

void
DOMTest::tearDown()
{
  if (di != NULL)
    di->release_ref();
  if (dt != NULL)
    dt->release_ref();
  if (doc != NULL)
    doc->release_ref();
}

void
DOMTest::testCreateDOMImplementation()
{
  CPPUNIT_ASSERT(di);
}

void
DOMTest::testCreateDocumentType()
{
  CPPUNIT_ASSERT(dt);
  wchar_t* n;

  // Check the name...
  CPPUNIT_ASSERT_NO_THROW(n = dt->name());
  CPPUNIT_ASSERT_EQUAL(0, wcscmp(n, L"foo"));
  free(n);

  // Next check the publicId...
  CPPUNIT_ASSERT_NO_THROW(n = dt->publicId());
  CPPUNIT_ASSERT_EQUAL(0, wcscmp(n, L"bar"));
  free(n);

  // Next check the systemId...
  CPPUNIT_ASSERT_NO_THROW(n = dt->systemId());
  CPPUNIT_ASSERT_EQUAL(0, wcscmp(n, L"baz"));
  free(n);

  iface::dom::NamedNodeMap* nnm;
  CPPUNIT_ASSERT_NO_THROW(nnm = dt->entities());
  CPPUNIT_ASSERT(nnm);
  int32_t l;
  CPPUNIT_ASSERT_NO_THROW(l = nnm->length());
  CPPUNIT_ASSERT_EQUAL(0, l);
  nnm->release_ref();

  CPPUNIT_ASSERT_NO_THROW(nnm = dt->notations());
  CPPUNIT_ASSERT(nnm);
  CPPUNIT_ASSERT_NO_THROW(l = nnm->length());
  CPPUNIT_ASSERT_EQUAL(0, l);
  nnm->release_ref();

  // XXX Can we test internalSubset at all?

  int32_t nt;
  CPPUNIT_ASSERT_NO_THROW(nt = dt->nodeType());
  CPPUNIT_ASSERT_EQUAL((int32_t)iface::dom::Node::DOCUMENT_TYPE_NODE, nt);
}

void
DOMTest::testCreateDocument()
{
  CPPUNIT_ASSERT(doc);

  int32_t nt;
  CPPUNIT_ASSERT_NO_THROW(nt = doc->nodeType());
  CPPUNIT_ASSERT_EQUAL((int32_t)iface::dom::Node::DOCUMENT_NODE, nt);

  // Check there is a document element...
  iface::dom::Element* de;
  CPPUNIT_ASSERT_NO_THROW(de = doc->documentElement());
  CPPUNIT_ASSERT(de);

  CPPUNIT_ASSERT_NO_THROW(nt = de->nodeType());
  CPPUNIT_ASSERT_EQUAL((int32_t)iface::dom::Node::ELEMENT_NODE, nt);

  // Check the document element is in the right namespace...
  wchar_t* nsURI;
  CPPUNIT_ASSERT_NO_THROW(nsURI = de->namespaceURI());
  CPPUNIT_ASSERT(nsURI);
  CPPUNIT_ASSERT_EQUAL(0, wcscmp(nsURI, L"http://www.physiome.org/testsuite/1"));
  free(nsURI);

  // Check that the parent of the document element is the document...
  iface::dom::Node* pn;
  CPPUNIT_ASSERT(pn = de->parentNode());
  CPPUNIT_ASSERT_NO_THROW(nt = pn->nodeType());
  CPPUNIT_ASSERT_EQUAL((int32_t)iface::dom::Node::DOCUMENT_NODE, nt);

  // Check it is being correctly wrapped...
  DECLARE_QUERY_INTERFACE_REPLACE(docFromWrapped, pn, dom::Document);
  CPPUNIT_ASSERT(docFromWrapped);
  CPPUNIT_ASSERT_NO_THROW(nt = docFromWrapped->nodeType());
  CPPUNIT_ASSERT_EQUAL((int32_t)iface::dom::Node::DOCUMENT_NODE, nt);

  docFromWrapped->release_ref();
  de->release_ref();
}

void
DOMTest::testCloneNode()
{
  iface::dom::Element* de;
  CPPUNIT_ASSERT_NO_THROW(de = doc->documentElement());
  CPPUNIT_ASSERT(de);

  wchar_t* ln = de->localName();
  de->release_ref();
  CPPUNIT_ASSERT_EQUAL(0, wcscmp(ln, L"tests"));
  free(ln);

  iface::dom::Node* cn;
  CPPUNIT_ASSERT_NO_THROW(cn = doc->cloneNode(true));
  CPPUNIT_ASSERT(cn);

  DECLARE_QUERY_INTERFACE_REPLACE(doc2, cn, dom::Document);

  CPPUNIT_ASSERT_NO_THROW(de = doc2->documentElement());
  CPPUNIT_ASSERT(de);

  ln = de->localName();
  de->release_ref();
  CPPUNIT_ASSERT_EQUAL(0, wcscmp(ln, L"tests"));
  free(ln);

  doc2->release_ref();
}

void
DOMTest::testImportNode()
{
  iface::dom::Document* doc2;
  iface::dom::DocumentType* dtype;
  CPPUNIT_ASSERT_NO_THROW(doc2 = di->createDocument(L"http://www.physiome.org/testsuite/1",
                                                    L"tests", NULL));

  iface::dom::Element* de = doc->documentElement();
  CPPUNIT_ASSERT(de);

  iface::dom::Node* in;
  CPPUNIT_ASSERT_NO_THROW(in = doc2->importNode(de, true));
  iface::dom::Element* de2 = doc2->documentElement();
  de2->appendChild(in)->release_ref();

  in->release_ref();
  de2->release_ref();
  de->release_ref();
  doc2->release_ref();
}


void
DOMTest::testSerialiseBasicDocument()
{
  DOMWriter dw;
  std::wstring str;
  dw.writeDocument(NULL, doc, str);
  CPPUNIT_ASSERT(str ==
                 L"<?xml version=\"1.0\"?>\n"
                 L"<tests xmlns=\"http://www.physiome.org/testsuite/1\"/>");
}

void
DOMTest::testSerialiseAttributes()
{
  DOMWriter dw;
  std::wstring str;

  iface::dom::Node* dn;
  iface::dom::Element* de = doc->documentElement();
  CPPUNIT_ASSERT_NO_THROW(dn = de->cloneNode(true));
  de->release_ref();

  QUERY_INTERFACE_REPLACE(de, dn, dom::Element);

  CPPUNIT_ASSERT_NO_THROW(de->setAttribute(L"hello", L"world"));
  CPPUNIT_ASSERT_NO_THROW(de->setAttributeNS(L"", L"foo", L"bar"));
  CPPUNIT_ASSERT_NO_THROW
    (
     de->setAttributeNS(L"http://www.example.org/testsuite/attrns/1", L"baz",
                        L"test")
    );

  CPPUNIT_ASSERT_NO_THROW
    (
     de->setAttributeNS(L"http://www.example.org/testsuite/attrns/2", L"anotherns:baz",
                        L"test2")
    );

  dw.writeNode(NULL, de, str);
  de->release_ref();

  // XXX this is ugly as attribute order isn't defined.
  CPPUNIT_ASSERT(str ==
                 L"<tests xmlns=\"http://www.physiome.org/testsuite/1\" "
                 L"xmlns:anotherns=\"http://www.example.org/testsuite/attrns/2"
                 L"\" xmlns:ns=\"http://www.example.org/testsuite/attrns/1\" "
                 L"foo=\"bar\" hello=\"world\" "
                 L"ns:baz=\"test\" anotherns:baz=\"test2\"/>" ||
                 str ==
                 L"<tests xmlns=\"http://www.physiome.org/testsuite/1\" "
                 L"xmlns:anotherns=\"http://www.example.org/testsuite/attrns/2"
                 L"\" xmlns:ns=\"http://www.example.org/testsuite/attrns/1\" "
                 L"ns:baz=\"test\" anotherns:baz=\"test2\" foo=\"bar\" hello=\"world\"/>"
                );
}

void
DOMTest::testSerialiseChildElements()
{
  DOMWriter dw;
  std::wstring str;

  iface::dom::Node* dn;
  iface::dom::Element* de = doc->documentElement();
  CPPUNIT_ASSERT_NO_THROW(dn = de->cloneNode(true));
  de->release_ref();
  QUERY_INTERFACE_REPLACE(de, dn, dom::Element);

  iface::dom::Element* el;
  CPPUNIT_ASSERT_NO_THROW(el = doc->createElementNS
                          (L"http://www.example.org/testsuite/subelns",
                           L"foo:test"));

  iface::dom::Element* el2;
  CPPUNIT_ASSERT_NO_THROW(el2 = doc->createElementNS
                          (L"http://www.example.org/testsuite/subelns",
                           L"bar"));
  el->appendChild(el2)->release_ref();
  el2->release_ref();
  de->appendChild(el)->release_ref();
  el->release_ref();
  
  CPPUNIT_ASSERT_NO_THROW(el = doc->createElementNS
                          (L"http://www.physiome.org/testsuite/1",
                           L"test2"));
  de->appendChild(el)->release_ref();
  el->release_ref();


  dw.writeNode(NULL, de, str);
  de->release_ref();

  CPPUNIT_ASSERT(str ==
                 L"<tests xmlns=\"http://www.physiome.org/testsuite/1\">"
                 L"<foo:test xmlns:foo=\"http://www.example.org/testsuite"
                 L"/subelns\"><foo:bar/></foo:test><test2/></tests>");
}

void
DOMTest::testSetAttributeNodeNS()
{
  iface::dom::Element* el = doc->documentElement();
  iface::dom::Attr* at =
    doc->createAttributeNS(L"http://www.example.org/foons", L"foo:bar");
  CPPUNIT_ASSERT(at);
  iface::dom::Attr* at2 = el->setAttributeNodeNS(at);
  CPPUNIT_ASSERT(!at2);
  at->release_ref();

  at = doc->createAttributeNS(L"", L"bar");
  CPPUNIT_ASSERT(at);
  at2 = el->setAttributeNodeNS(at);
  CPPUNIT_ASSERT(!at2);
  at->release_ref();

  el->release_ref();
}

void
DOMTest::testLoadDocument()
{
  iface::cellml_api::CellMLBootstrap* cb = CreateCellMLBootstrap();
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  cb->release_ref();

  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"simple_test.xml");
  ul->release_ref();

  iface::dom::Element* de = d->documentElement();

  wchar_t* str = de->localName();
  CPPUNIT_ASSERT(!wcscmp(str, L"foo"));
  free(str);

  str = de->namespaceURI();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.org/bar/"));
  free(str);

  iface::dom::Element* e2 = d->createElementNS(L"http://www.example.org/baz",
                                               L"mybaz");
  de->appendChild(e2)->release_ref();
  e2->release_ref();

  iface::dom::Text* t = d->createTextNode(L"hello world");
  de->appendChild(t)->release_ref();
  t->release_ref();

  de->release_ref();
  d->release_ref();
}

void
DOMTest::testGetElementByTagName()
{
  iface::cellml_api::CellMLBootstrap* cb = CreateCellMLBootstrap();
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  cb->release_ref();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"simple_test.xml");
  ul->release_ref();

  iface::dom::NodeList* nl =
    d->getElementsByTagName(L"baz");
  CPPUNIT_ASSERT_EQUAL(0, (int)nl->length());
  nl->release_ref();

  nl = d->getElementsByTagName(L"bar");
  CPPUNIT_ASSERT_EQUAL(1, (int)nl->length());
  iface::dom::Node* n = nl->item(0);
  CPPUNIT_ASSERT(n);
  wchar_t* ln = n->nodeName();
  CPPUNIT_ASSERT(!wcscmp(ln, L"bar"));
  free(ln);
  n->release_ref();
  nl->release_ref();

  nl = d->getElementsByTagName(L"boo");
  CPPUNIT_ASSERT_EQUAL(2, (int)nl->length());
  n = nl->item(0);
  CPPUNIT_ASSERT(n);
  wchar_t* nu = n->namespaceURI();
  CPPUNIT_ASSERT(!wcscmp(nu, L"http://www.example.org/test/"));
  free(nu);
  n->release_ref();

  n = nl->item(1);
  CPPUNIT_ASSERT(n);
  nu = n->namespaceURI();
  CPPUNIT_ASSERT(!wcscmp(nu, L"http://www.example.org/bar/"));
  free(nu);
  n->release_ref();

  nl->release_ref();

  nl = d->getElementsByTagNameNS(L"http://www.example.org/bar/", L"baz");
  CPPUNIT_ASSERT_EQUAL(0, (int)nl->length());
  nl->release_ref();

  nl = d->getElementsByTagNameNS(L"http://www.example.org/baz/", L"bar");
  CPPUNIT_ASSERT_EQUAL(0, (int)nl->length());
  nl->release_ref();

  nl = d->getElementsByTagNameNS(L"http://www.example.org/bar/", L"bar");
  CPPUNIT_ASSERT_EQUAL(1, (int)nl->length());
  n = nl->item(0);
  CPPUNIT_ASSERT(n);
  ln = n->nodeName();
  CPPUNIT_ASSERT(!wcscmp(ln, L"bar"));
  free(ln);
  n->release_ref();
  nl->release_ref();

  nl = d->getElementsByTagNameNS(L"http://www.example.org/test/", L"boo");
  CPPUNIT_ASSERT_EQUAL(1, (int)nl->length());
  n = nl->item(0);
  CPPUNIT_ASSERT(n);
  nu = n->namespaceURI();
  CPPUNIT_ASSERT(!wcscmp(nu, L"http://www.example.org/test/"));
  free(nu);
  n->release_ref();

  nl->release_ref();

  d->release_ref();
}
