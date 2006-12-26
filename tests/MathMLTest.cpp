#include "MathMLTest.hpp"
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

CPPUNIT_TEST_SUITE_REGISTRATION( MathMLTest );

void
MathMLTest::setUp()
{
  cb = CreateCellMLBootstrap();
}

void
MathMLTest::tearDown()
{
  cb->release_ref();
}

void
MathMLTest::testCreateMathMLDocument()
{
  iface::dom::DOMImplementation* di;
  di = CreateDOMImplementation();
  CPPUNIT_ASSERT(di);
  
  CPPUNIT_ASSERT(di->hasFeature(L"org.w3c.dom.mathml", L"2.0"));

  DECLARE_QUERY_INTERFACE_REPLACE(mdi, di, mathml_dom::MathMLDOMImplementation);

  iface::mathml_dom::MathMLDocument* mdoc;
  mdoc = mdi->createMathMLDocument();
  CPPUNIT_ASSERT(mdoc);

  // Get the document element...
  iface::dom::Element* docel = mdoc->documentElement();
  CPPUNIT_ASSERT(docel);

  // Check it has the correct namespace URI and localName...
  wchar_t* nsURI = docel->namespaceURI();
  wchar_t* ln = docel->localName();
  CPPUNIT_ASSERT(!wcscmp(MATHML_NS, nsURI));
  CPPUNIT_ASSERT(!wcscmp(L"math", ln));
  free(nsURI);
  free(ln);

  // Check it has the correct type...
  CPPUNIT_ASSERT(hasInterface(docel, "mathml_dom::MathMLElement"));
  CPPUNIT_ASSERT(hasInterface(docel, "mathml_dom::MathMLMathElement"));

  docel->release_ref();
  mdoc->release_ref();
  mdi->release_ref();
}

void
MathMLTest::testMathMLDocumentElementContainer()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();

  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_simple_test.xml");

  // Check it has been identified as a MathML document...
  DECLARE_QUERY_INTERFACE_REPLACE(md, d, mathml_dom::MathMLDocument);
  CPPUNIT_ASSERT(md);

  iface::dom::Element* de = md->documentElement();
  DECLARE_QUERY_INTERFACE_REPLACE(me, de, mathml_dom::MathMLMathElement);
  CPPUNIT_ASSERT(me);

  wchar_t* str = me->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"foo"));
  free(str);

  str = me->mathElementStyle();
  CPPUNIT_ASSERT(!wcscmp(str, L"bar"));
  free(str);
  me->mathElementStyle(L"testElementStyle");
  str = me->mathElementStyle();
  CPPUNIT_ASSERT(!wcscmp(str, L"testElementStyle"));
  free(str);

  str = me->id();
  CPPUNIT_ASSERT(!wcscmp(str, L"baz"));
  free(str);
  me->id(L"testID");
  str = me->id();
  CPPUNIT_ASSERT(!wcscmp(str, L"testID"));
  free(str);

  str = me->xref();
  CPPUNIT_ASSERT(!wcscmp(str, L"blah"));
  free(str);
  me->xref(L"TestXref");
  str = me->xref();
  CPPUNIT_ASSERT(!wcscmp(str, L"TestXref"));
  free(str);

  str = me->href();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/maths/test"));
  free(str);
  me->xref(L"http://www.example.com/maths/testhref");
  str = me->xref();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/maths/testhref"));
  free(str);

  str = me->macros();
  CPPUNIT_ASSERT(!wcscmp(str, L""));
  free(str);
  me->macros(L"foobar");
  str = me->macros();
  CPPUNIT_ASSERT(!wcscmp(str, L"foobar"));
  free(str);

  str = me->display();
  CPPUNIT_ASSERT(!wcscmp(str, L""));
  free(str);
  me->display(L"block");
  str = me->display();
  CPPUNIT_ASSERT(!wcscmp(str, L"block"));
  free(str);

  CPPUNIT_ASSERT(!me->ownerMathElement());

  {
    CPPUNIT_ASSERT_EQUAL(1, (int)me->nArguments());
    
    iface::mathml_dom::MathMLElement* me2;
    CPPUNIT_ASSERT_NO_THROW(me2 = me->getArgument(1));
    CPPUNIT_ASSERT(me2);
    
    str = me2->localName();
    CPPUNIT_ASSERT(!wcscmp(str, L"apply"));
    free(str);
    
    iface::mathml_dom::MathMLNodeList* mnl = me->declarations();
    CPPUNIT_ASSERT_EQUAL(1, (int)mnl->length());
    mnl->release_ref();
    
    iface::dom::Node* ntmp = me2->cloneNode(true);
    DECLARE_QUERY_INTERFACE_REPLACE(me3, ntmp, mathml_dom::MathMLElement);
    
    me2->className(L"oldclass");
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    
    me3->className(L"newclass");
    
    iface::mathml_dom::MathMLElement* me4 = me->insertArgument(me3, 2);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    
    CPPUNIT_ASSERT_EQUAL(2, (int)me->nArguments());
    
    mnl = me->arguments();
    CPPUNIT_ASSERT(mnl);
    CPPUNIT_ASSERT_EQUAL(2, (int)mnl->length());
    
    ntmp = mnl->item(0);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    
    ntmp = mnl->item(1);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me2->release_ref();
    
    // Now try deleting an element...
    me->deleteArgument(2);
    ntmp = mnl->item(0);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    CPPUNIT_ASSERT_EQUAL(1, (int)mnl->length());
    mnl->release_ref();
    
    // Now re-add me3, this time at the start...
    me4 = me->insertArgument(me3, 1);
    me4->release_ref();
    
    // And check that the order is right...
    mnl = me->arguments();
    CPPUNIT_ASSERT(mnl);
    CPPUNIT_ASSERT_EQUAL(2, (int)mnl->length());
    
    ntmp = mnl->item(0);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me2->release_ref();
    
    ntmp = mnl->item(1);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    mnl->release_ref();
    
    // Now try calling removeArgument()...
    me4 = me->removeArgument(1);
    CPPUNIT_ASSERT(me4);
    // Check the return value...
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    // Now check the remaining value is correct...
    me4 = me->getArgument(1);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me4->release_ref();
    
    // Next call setArgument...
    me4 = me->setArgument(me3, 1);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    me4 = me->getArgument(1);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    me4 = me3->ownerMathElement();

    str = me4->localName();
    CPPUNIT_ASSERT(!wcscmp(L"math", str));
    free(str);
    me4->release_ref();
    me3->release_ref();
  }

  {
    // Now we repeat almost the same code, this time for declarations...
    iface::mathml_dom::MathMLDeclareElement* me2 = me->getDeclaration(1);
    iface::dom::Node* ntmp = me2->cloneNode(true);
    DECLARE_QUERY_INTERFACE_REPLACE(me3, ntmp, mathml_dom::MathMLDeclareElement);
    
    me2->className(L"oldclass");
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    
    me3->className(L"newclass");
    
    iface::mathml_dom::MathMLDeclareElement* me4 =
      me->insertDeclaration(me3, 2);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    
    iface::mathml_dom::MathMLNodeList* mnl = me->declarations();
    CPPUNIT_ASSERT(mnl);
    CPPUNIT_ASSERT_EQUAL(2, (int)mnl->length());
    
    ntmp = mnl->item(0);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLDeclareElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    
    ntmp = mnl->item(1);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLDeclareElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me2->release_ref();
    
    // Now try deleting an element...
    me->deleteDeclaration(2);
    ntmp = mnl->item(0);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLDeclareElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    CPPUNIT_ASSERT_EQUAL(1, (int)mnl->length());
    mnl->release_ref();
    
    // Now re-add me3, this time at the start...
    me4 = me->insertDeclaration(me3, 1);
    me4->release_ref();
    
    // And check that the order is right...
    mnl = me->declarations();
    CPPUNIT_ASSERT(mnl);
    CPPUNIT_ASSERT_EQUAL(2, (int)mnl->length());
    
    ntmp = mnl->item(0);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLDeclareElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me2->release_ref();
    
    ntmp = mnl->item(1);
    QUERY_INTERFACE_REPLACE(me2, ntmp, mathml_dom::MathMLDeclareElement);
    CPPUNIT_ASSERT(me2);
    str = me2->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me2->release_ref();
    mnl->release_ref();
    
    // Now try calling removeDeclaration()...
    me4 = me->removeDeclaration(1);
    CPPUNIT_ASSERT(me4);
    // Check the return value...
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    // Now check the remaining value is correct...
    me4 = me->getDeclaration(1);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"oldclass"));
    free(str);
    me4->release_ref();
    
    // Next call setDeclaration...
    me4 = me->setDeclaration(me3, 1);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    me4 = me->getDeclaration(1);
    str = me4->className();
    CPPUNIT_ASSERT(!wcscmp(str, L"newclass"));
    free(str);
    me4->release_ref();
    me3->release_ref();
  }

  me->release_ref();
  md->release_ref();
  ul->release_ref();
}

void
MathMLTest::testMathMLContentToken()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_simple_test.xml");
  ul->release_ref();

  // Check it has been identified as a MathML document...
  DECLARE_QUERY_INTERFACE_REPLACE(md, d, mathml_dom::MathMLDocument);
  CPPUNIT_ASSERT(md);

  iface::dom::Element* de = md->getElementById(L"cnel");
  CPPUNIT_ASSERT(de);

  DECLARE_QUERY_INTERFACE_REPLACE(mce, de, mathml_dom::MathMLCnElement);

  CPPUNIT_ASSERT(mce);

  iface::mathml_dom::MathMLNodeList* mnl = mce->arguments();
  CPPUNIT_ASSERT(mnl);
  CPPUNIT_ASSERT_EQUAL(3, (int)mnl->length());
  iface::dom::Node*n = mnl->item(1);
  DECLARE_QUERY_INTERFACE_REPLACE(tn, n, dom::Text);
  wchar_t* txt = tn->data();
  CPPUNIT_ASSERT(!wcscmp(txt, L"1"));
  free(txt);
  tn->release_ref();
  mnl->release_ref();

  txt = mce->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(txt, L"http://www.example.com/definitionURL/"));
  free(txt);

  txt = mce->encoding();
  CPPUNIT_ASSERT(!wcscmp(txt, L"test123"));
  free(txt);

  CPPUNIT_ASSERT_NO_THROW(n = mce->getArgument(3));
  QUERY_INTERFACE_REPLACE(tn, n, dom::Text);
  txt = tn->data();
  CPPUNIT_ASSERT(!wcscmp(txt, L"2"));
  free(txt);
  tn->release_ref();

  tn = md->createTextNode(L"123");
  mce->insertArgument(tn, 1)->release_ref();
  n = mce->getArgument(1);
  DECLARE_QUERY_INTERFACE_REPLACE(tn2, n, dom::Text);
  txt = tn2->data();
  tn2->release_ref();
  CPPUNIT_ASSERT(!wcscmp(txt, L"123"));
  free(txt);
  mce->deleteArgument(1);
  n = mce->getArgument(1);
  QUERY_INTERFACE_REPLACE(tn2, n, dom::Text);
  txt = tn2->data();
  tn2->release_ref();
  CPPUNIT_ASSERT(!wcscmp(txt, L"0"));
  free(txt);

  mce->insertArgument(tn, 2)->release_ref();
  n = mce->getArgument(2);
  QUERY_INTERFACE_REPLACE(tn2, n, dom::Text);
  txt = tn2->data();
  tn2->release_ref();
  CPPUNIT_ASSERT(!wcscmp(txt, L"123"));
  free(txt);
  iface::dom::Node* n2 = mce->removeArgument(2);
  QUERY_INTERFACE_REPLACE(tn2, n2, dom::Text);
  txt = tn2->data();
  CPPUNIT_ASSERT(!wcscmp(txt, L"123"));
  free(txt);
  tn2->release_ref();
  n = mce->getArgument(2);
  QUERY_INTERFACE_REPLACE(tn2, n, dom::Text);
  txt = tn2->data();
  tn2->release_ref();
  CPPUNIT_ASSERT(!wcscmp(txt, L"1"));
  free(txt);

  mce->insertArgument(tn, 4)->release_ref();
  n = mce->getArgument(4);
  QUERY_INTERFACE_REPLACE(tn2, n, dom::Text);
  txt = tn2->data();
  tn2->release_ref();
  CPPUNIT_ASSERT(!wcscmp(txt, L"123"));
  free(txt);
  n2 = mce->removeArgument(4);
  QUERY_INTERFACE_REPLACE(tn2, n2, dom::Text);
  txt = tn2->data();
  CPPUNIT_ASSERT(!wcscmp(txt, L"123"));
  free(txt);
  tn2->release_ref();
  CPPUNIT_ASSERT_EQUAL(3, (int)mce->nargs());

  mce->setArgument(tn, 1)->release_ref();  
  CPPUNIT_ASSERT_EQUAL(3, (int)mce->nargs());
  n = mce->getArgument(1);
  QUERY_INTERFACE_REPLACE(tn2, n, dom::Text);
  txt = tn2->data();
  tn2->release_ref();
  CPPUNIT_ASSERT(!wcscmp(txt, L"123"));
  free(txt);

  txt = mce->base();
  CPPUNIT_ASSERT(!wcscmp(txt, L"10"));
  free(txt);
  mce->base(L"16");
  txt = mce->base();
  CPPUNIT_ASSERT(!wcscmp(txt, L"16"));
  free(txt);

  txt = mce->type();
  CPPUNIT_ASSERT(!wcscmp(txt, L"vector"));
  free(txt);
  mce->type(L"matrix");
  txt = mce->type();
  CPPUNIT_ASSERT(!wcscmp(txt, L"matrix"));
  free(txt);

  tn->release_ref();
  mce->release_ref();

  de = md->getElementById(L"ciel");
  CPPUNIT_ASSERT(de);

  DECLARE_QUERY_INTERFACE_REPLACE(cie, de, mathml_dom::MathMLCiElement);

  txt = cie->type();
  CPPUNIT_ASSERT(!wcscmp(txt, L"real"));
  free(txt);

  cie->type(L"complex");
  txt = cie->type();
  CPPUNIT_ASSERT(!wcscmp(txt, L"complex"));
  free(txt);

  cie->release_ref();
  md->release_ref();
}

void
MathMLTest::testMathMLContentContainer()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_simple_test.xml");
  ul->release_ref();

  // Check it has been identified as a MathML document...
  DECLARE_QUERY_INTERFACE_REPLACE(md, d, mathml_dom::MathMLDocument);
  CPPUNIT_ASSERT(md);

  iface::dom::Element* de = md->getElementById(L"applyel");
  CPPUNIT_ASSERT(de);
  DECLARE_QUERY_INTERFACE_REPLACE(mae, de, mathml_dom::MathMLApplyElement);

  CPPUNIT_ASSERT_EQUAL(1, (int)mae->nBoundVariables());

  iface::mathml_dom::MathMLConditionElement* condition = mae->condition();
  CPPUNIT_ASSERT(condition);
  wchar_t* str = condition->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"testcondition"));
  free(str);
  condition->release_ref();

  // Now make a replacement condition element...
  iface::dom::Element* eltmp =
    md->createElementNS(MATHML_NS, L"condition");
  QUERY_INTERFACE_REPLACE(condition, eltmp, mathml_dom::MathMLConditionElement);
  mae->condition(condition);
  condition->className(L"newcondition");
  condition->release_ref();

  condition = mae->condition();
  CPPUNIT_ASSERT(condition);
  str = condition->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcondition"));
  free(str);
  condition->release_ref();

  // Next test the opDegree operations...
  iface::mathml_dom::MathMLElement* odel = mae->opDegree();
  CPPUNIT_ASSERT(odel);
  str = odel->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"degreeel"));
  free(str);
  odel->release_ref();

  // Now make a replacement degree element...
  eltmp = md->createElementNS(MATHML_NS, L"degree");
  QUERY_INTERFACE_REPLACE(odel, eltmp, mathml_dom::MathMLElement);
  mae->opDegree(odel);
  odel->className(L"newdegreeel");
  odel->release_ref();

  odel = mae->opDegree();
  CPPUNIT_ASSERT(odel);
  str = odel->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newdegreeel"));
  free(str);
  odel->release_ref();

  // Now get a domainOfApplication...
  iface::mathml_dom::MathMLElement* doa = mae->domainOfApplication();
  CPPUNIT_ASSERT(doa);
  str = doa->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"doael"));
  free(str);  
  doa->release_ref();

  eltmp = md->createElementNS(MATHML_NS, L"domainofapplication");
  QUERY_INTERFACE_REPLACE(doa, eltmp, mathml_dom::MathMLElement);
  mae->domainOfApplication(doa);
  doa->className(L"newdoael");
  doa->release_ref();

  doa = mae->domainOfApplication();
  CPPUNIT_ASSERT(doa);
  str = doa->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newdoael"));
  free(str);
  doa->release_ref();

  // Now get a momentAbout...
  iface::mathml_dom::MathMLElement* ma = mae->momentAbout();
  CPPUNIT_ASSERT(ma);
  str = ma->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"momentaboutel"));
  free(str);  
  ma->release_ref();

  eltmp = md->createElementNS(MATHML_NS,
                              L"momentabout");
  QUERY_INTERFACE_REPLACE(ma, eltmp, mathml_dom::MathMLElement);
  mae->momentAbout(ma);
  ma->className(L"newmomentabout");
  ma->release_ref();

  ma = mae->momentAbout();
  CPPUNIT_ASSERT(ma);
  str = ma->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newmomentabout"));
  free(str);
  ma->release_ref();
  
  iface::mathml_dom::MathMLBvarElement* bvar = mae->getBoundVariable(1);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"bvarel"));
  free(str);
  bvar->release_ref();

  eltmp = md->createElementNS(MATHML_NS, L"bvar");
  DECLARE_QUERY_INTERFACE_REPLACE(newbvar, eltmp, mathml_dom::MathMLBvarElement);
  newbvar->className(L"newbvar");
  bvar = mae->insertBoundVariable(newbvar, 1);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();
  bvar = mae->getBoundVariable(1);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();

  mae->deleteBoundVariable(1);
  bvar = mae->getBoundVariable(1);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"bvarel"));
  free(str);
  bvar->release_ref();

  // Now try an insert at the end...

  bvar = mae->insertBoundVariable(newbvar, 0);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();
  bvar = mae->getBoundVariable(2);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();

  bvar = mae->removeBoundVariable(2);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();
  bvar = mae->getBoundVariable(1);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"bvarel"));
  free(str);
  bvar->release_ref();

  // Try a set...
  bvar = mae->setBoundVariable(newbvar, 1);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();
  bvar = mae->getBoundVariable(1);
  CPPUNIT_ASSERT(bvar);
  str = bvar->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newbvar"));
  free(str);
  bvar->release_ref();

  newbvar->release_ref();

  // Now test the attributes/operations on the MathMLApplyElement interface...
  iface::mathml_dom::MathMLElement* op = mae->_cxx_operator();
  str = op->localName();
  CPPUNIT_ASSERT(!wcscmp(str, L"eq"));
  free(str);
  op->release_ref();

  eltmp = md->createElementNS(MATHML_NS, L"plus");
  DECLARE_QUERY_INTERFACE_REPLACE(newop, eltmp, mathml_dom::MathMLElement);
  mae->_cxx_operator(newop);
  newop->release_ref();

  op = mae->_cxx_operator();
  str = op->localName();
  CPPUNIT_ASSERT(!wcscmp(str, L"plus"));
  free(str);
  op->release_ref();

  // Check the lowlimit and uplimit...
  
  iface::mathml_dom::MathMLElement* ll = mae->lowLimit();
  str = ll->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"llel"));
  free(str);
  ll->release_ref();

  eltmp = md->createElementNS(MATHML_NS, L"lowlimit");
  DECLARE_QUERY_INTERFACE_REPLACE(newll, eltmp, mathml_dom::MathMLElement);
  newll->className(L"newllel");
  mae->lowLimit(newll);
  newll->release_ref();

  ll = mae->lowLimit();
  str = ll->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newllel"));
  free(str);
  ll->release_ref();

  // Now the uplimit...
  iface::mathml_dom::MathMLElement* upl = mae->upLimit();
  str = upl->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"ulel"));
  free(str);
  upl->release_ref();

  eltmp = md->createElementNS(MATHML_NS, L"uplimit");
  DECLARE_QUERY_INTERFACE_REPLACE(newul, eltmp, mathml_dom::MathMLElement);

  newul->className(L"newulel");
  mae->upLimit(newul);
  newul->release_ref();

  upl = mae->upLimit();
  str = upl->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newulel"));
  free(str);
  upl->release_ref();

  mae->release_ref();
  md->release_ref();
}

void
MathMLTest::testSpecificMathMLContentContainers()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_contentcontainers.xml");
  ul->release_ref();

  iface::dom::Element* el = d->getElementById(L"functionel");
  DECLARE_QUERY_INTERFACE_REPLACE(fnel, el, mathml_dom::MathMLFnElement);

  wchar_t* str = fnel->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/defnurl/function"));
  free(str);
  fnel->definitionURL(L"http://www.example.com/defnurl/anotherfunction");
  str = fnel->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/defnurl/anotherfunction"));
  free(str);

  str = fnel->encoding();
  CPPUNIT_ASSERT(!wcscmp(str, L"some-encoding"));
  free(str);
  fnel->encoding(L"a-different-encoding");
  str = fnel->encoding();
  CPPUNIT_ASSERT(!wcscmp(str, L"a-different-encoding"));
  free(str);
  fnel->release_ref();

  // Find the lambda...
  el = d->getElementById(L"lambdael");
  DECLARE_QUERY_INTERFACE_REPLACE(lambdael, el, mathml_dom::MathMLLambdaElement);

  iface::mathml_dom::MathMLElement* exp = lambdael->expression();

  str = exp->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"lambdaexpr"));
  free(str);
  exp->release_ref();

  // Now produce a new expression...
  iface::dom::Element* eltmp =
    d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(exp, eltmp, mathml_dom::MathMLElement);
  exp->className(L"newexpression");
  lambdael->expression(exp);
  exp->release_ref();

  exp = lambdael->expression();
  str = exp->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newexpression"));
  free(str);
  exp->release_ref();

  lambdael->release_ref();

  el = d->getElementById(L"implicitset");
  DECLARE_QUERY_INTERFACE_REPLACE(setel, el, mathml_dom::MathMLSetElement);
  CPPUNIT_ASSERT(!setel->isExplicit());
  setel->release_ref();

  el = d->getElementById(L"explicitset");
  QUERY_INTERFACE_REPLACE(setel, el, mathml_dom::MathMLSetElement);
  CPPUNIT_ASSERT(setel->isExplicit());

  str = setel->type();
  CPPUNIT_ASSERT(!wcscmp(str, L"normal"));
  free(str);

  setel->type(L"multiset");
  str = setel->type();
  CPPUNIT_ASSERT(!wcscmp(str, L"multiset"));
  free(str);
  setel->release_ref();

  el = d->getElementById(L"implicitlist");

  DECLARE_QUERY_INTERFACE_REPLACE(listel, el, mathml_dom::MathMLListElement);
  CPPUNIT_ASSERT(!listel->isExplicit());
  listel->release_ref();

  el = d->getElementById(L"explicitlist");
  QUERY_INTERFACE_REPLACE(listel, el, mathml_dom::MathMLListElement);
  CPPUNIT_ASSERT(listel->isExplicit());

  str = listel->ordering();
  CPPUNIT_ASSERT(!wcscmp(str, L"lexicographic"));
  free(str);

  listel->ordering(L"numeric");
  str = listel->ordering();
  CPPUNIT_ASSERT(!wcscmp(str, L"numeric"));
  free(str);
  listel->release_ref();

  d->release_ref();
}

void
MathMLTest::testMathMLPredefinedSymbol()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_simple_test.xml");
  ul->release_ref();

  iface::dom::Element* el = d->getElementById(L"eqel");
  DECLARE_QUERY_INTERFACE_REPLACE(ps, el, mathml_dom::MathMLPredefinedSymbol);

  wchar_t* str = ps->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/defnurl/myequals"));
  free(str);
  ps->definitionURL(L"http://www.example.com/defnurl/anotherequals");
  str = ps->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/defnurl/anotherequals"));
  free(str);

  str = ps->encoding();
  CPPUNIT_ASSERT(!wcscmp(str, L"myencoding"));
  free(str);
  ps->encoding(L"anotherencoding");
  str = ps->encoding();
  CPPUNIT_ASSERT(!wcscmp(str, L"anotherencoding"));
  free(str);

  str = ps->arity();
  CPPUNIT_ASSERT(!wcscmp(str, L"variable"));
  free(str);

  str = ps->symbolName();
  CPPUNIT_ASSERT(!wcscmp(str, L"eq"));
  free(str);

  ps->release_ref();

  // Find a tendsto...
  el = d->getElementById(L"ttel");
  DECLARE_QUERY_INTERFACE_REPLACE(tt, el, mathml_dom::MathMLTendsToElement);

  str = tt->type();
  CPPUNIT_ASSERT(!wcscmp(str, L"above"));
  free(str);
  tt->type(L"below");
  str = tt->type();
  CPPUNIT_ASSERT(!wcscmp(str, L"below"));
  free(str);

  tt->release_ref();
  d->release_ref();
}

void
MathMLTest::testMathMLInterval()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_othercontent.xml");
  ul->release_ref();

  iface::dom::Element* el = d->getElementById(L"intervalel");
  DECLARE_QUERY_INTERFACE_REPLACE(mie, el, mathml_dom::MathMLIntervalElement);

  wchar_t* str = mie->closure();
  CPPUNIT_ASSERT(!wcscmp(str, L"open"));
  free(str);

  mie->closure(L"closed");
  str = mie->closure();
  CPPUNIT_ASSERT(!wcscmp(str, L"closed"));
  free(str);

  iface::dom::Element* el2 =
    d->createElementNS(MATHML_NS, L"interval");
  DECLARE_QUERY_INTERFACE_REPLACE(mie2, el2,
                                  mathml_dom::MathMLIntervalElement);
  str = mie2->closure();
  CPPUNIT_ASSERT(!wcscmp(str, L"closed"));
  free(str);

  iface::mathml_dom::MathMLContentElement* mc = mie->start();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"intervalapply1"));
  free(str);
  mc->release_ref();

  mc = mie->end();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"intervalapply2"));
  free(str);
  mc->release_ref();

  el2 = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(mc, el2, mathml_dom::MathMLContentElement);
  mc->className(L"newintervalapply1");
  mie->start(mc);
  mc->release_ref();
  mc = mie->start();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newintervalapply1"));
  free(str);
  mc->release_ref();

  el2 = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(mc, el2, mathml_dom::MathMLContentElement);
  mc->className(L"newintervalapply2");
  mie->end(mc);
  mc->release_ref();
  mc = mie->end();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newintervalapply2"));
  free(str);
  mc->release_ref();

  // Now check that setting when not present works...
  el2 = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(mc, el2, mathml_dom::MathMLContentElement);
  mc->className(L"newintervalapply1");
  mie2->end(mc);
  mc->release_ref();
  mc = mie2->end();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newintervalapply1"));
  free(str);
  mc->release_ref();
  mc = mie2->start();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newintervalapply1"));
  free(str);
  mc->release_ref();
  el2 = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(mc, el2, mathml_dom::MathMLContentElement);
  mc->className(L"newintervalapply2");
  mie2->start(mc);
  mc->release_ref();
  mc = mie2->start();
  str = mc->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newintervalapply2"));
  free(str);
  mc->release_ref();

  mie2->release_ref();
  mie->release_ref();
  d->release_ref();
}

void
MathMLTest::testMathMLCondition()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_othercontent.xml");
  ul->release_ref();

  iface::dom::Element* condel = d->getElementById(L"conditionel");
  CPPUNIT_ASSERT(condel);
  iface::mathml_dom::MathMLConditionElement* mce =
    dynamic_cast<iface::mathml_dom::MathMLConditionElement*>
    (condel);

  iface::mathml_dom::MathMLApplyElement* cond =
    mce->condition();
  wchar_t* str = cond->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"conditionapply"));
  free(str);
  cond->release_ref();
  mce->release_ref();

  // Now check that setting when not present works...
  condel = d->createElementNS(MATHML_NS, L"condition");
  QUERY_INTERFACE_REPLACE(mce, condel, mathml_dom::MathMLConditionElement);
  
  iface::dom::Element* el2 = d->createElementNS(MATHML_NS,
                              L"apply");
  QUERY_INTERFACE_REPLACE(cond, el2, mathml_dom::MathMLApplyElement);
  cond->className(L"newconditionapply");
  mce->condition(cond);
  cond->release_ref();

  cond = mce->condition();
  str = cond->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newconditionapply"));
  free(str);
  cond->release_ref();
  mce->release_ref();

  d->release_ref();
}

void
MathMLTest::testMathMLDeclare()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_othercontent.xml");
  ul->release_ref();

  iface::dom::Element* declel = d->getElementById(L"declareel");
  DECLARE_QUERY_INTERFACE_REPLACE(decl, declel,
                                  mathml_dom::MathMLDeclareElement);

  wchar_t* str = decl->type();
  CPPUNIT_ASSERT(!wcscmp(str, L"vector"));
  free(str);

  decl->type(L"matrix");
  str = decl->type();
  CPPUNIT_ASSERT(!wcscmp(str, L"matrix"));
  free(str);
  
  CPPUNIT_ASSERT_EQUAL(5, (int)decl->nargs());
  decl->nargs(4);
  CPPUNIT_ASSERT_EQUAL(4, (int)decl->nargs());

  str = decl->occurrence();
  CPPUNIT_ASSERT(!wcscmp(str, L"prefix"));
  free(str);
  decl->occurrence(L"infix");
  str = decl->occurrence();
  CPPUNIT_ASSERT(!wcscmp(str, L"infix"));
  free(str);

  str = decl->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/defnurl/old"));
  free(str);
  decl->definitionURL(L"http://www.example.com/defnurl/new");
  str = decl->definitionURL();
  CPPUNIT_ASSERT(!wcscmp(str, L"http://www.example.com/defnurl/new"));
  free(str);

  str = decl->encoding();
  CPPUNIT_ASSERT(!wcscmp(str, L"some-encoding"));
  free(str);
  decl->encoding(L"some-other-encoding");
  str = decl->encoding();
  CPPUNIT_ASSERT(!wcscmp(str, L"some-other-encoding"));
  free(str);

  iface::mathml_dom::MathMLCiElement* cie = decl->identifier();
  str = cie->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"theci"));
  free(str);
  cie->release_ref();

  iface::dom::Element* elci =
    d->createElementNS(MATHML_NS, L"ci");
  QUERY_INTERFACE_REPLACE(cie, elci, mathml_dom::MathMLCiElement);
  cie->className(L"newci");
  decl->identifier(cie);
  cie->release_ref();
  cie = decl->identifier();
  str = cie->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newci"));
  free(str);
  cie->release_ref();

  iface::mathml_dom::MathMLElement* construct = decl->constructor();
  str = construct->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"theconstructor"));
  free(str);
  construct->release_ref();

  elci =
    d->createElementNS(MATHML_NS, L"vector");
  QUERY_INTERFACE_REPLACE(construct, elci, mathml_dom::MathMLElement);
  construct->className(L"newconstruct");
  decl->constructor(construct);
  construct->release_ref();
  construct = decl->constructor();
  str = construct->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newconstruct"));
  free(str);
  construct->release_ref();

  decl->release_ref();
  d->release_ref();
}

void
MathMLTest::testMathMLVector()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_othercontent.xml");
  ul->release_ref();

  iface::dom::Element* vecel = d->getElementById(L"vectorel");
  DECLARE_QUERY_INTERFACE_REPLACE(mve, vecel, mathml_dom::MathMLVectorElement);
  
  CPPUNIT_ASSERT_EQUAL(3, (int)mve->ncomponents());
  iface::mathml_dom::MathMLContentElement* c1 = mve->getComponent(1);
  wchar_t* str = c1->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"content1"));
  free(str);
  c1->release_ref();

  vecel = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(c1, vecel, mathml_dom::MathMLContentElement);
  c1->className(L"newcontent1");

  iface::mathml_dom::MathMLContentElement* c2 = mve->insertComponent(c1, 1);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  c2 = mve->getComponent(1);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  c2 = mve->removeComponent(1);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  c2 = mve->getComponent(1);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"content1"));
  free(str);
  c2->release_ref();
  
  c2 = mve->insertComponent(c1, 0);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  CPPUNIT_ASSERT_EQUAL(4, (int)mve->ncomponents());
  c2 = mve->getComponent(4);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  mve->deleteComponent(4);
  CPPUNIT_ASSERT_EQUAL(3, (int)mve->ncomponents());

  c2 = mve->setComponent(c1, 1);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  c2 = mve->getComponent(1);
  str = c2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcontent1"));
  free(str);
  c2->release_ref();
  c1->release_ref();

  mve->release_ref();
  d->release_ref();
}

void
MathMLTest::testMathMLMatrix()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_othercontent.xml");
  ul->release_ref();

  iface::dom::Element* matel = d->getElementById(L"matrixel");
  DECLARE_QUERY_INTERFACE_REPLACE(mme, matel, mathml_dom::MathMLMatrixElement);

  CPPUNIT_ASSERT_EQUAL(4, (int)mme->nrows());
  CPPUNIT_ASSERT_EQUAL(3, (int)mme->ncols());
  
  iface::mathml_dom::MathMLNodeList* mnl = mme->rows();
  CPPUNIT_ASSERT_EQUAL(4, (int)mnl->length());
  iface::dom::Node* n = mnl->item(0);
  DECLARE_QUERY_INTERFACE_REPLACE(mrow, n, mathml_dom::MathMLMatrixrowElement);
  // We now test MathMLMatrixRow while we have it...
  CPPUNIT_ASSERT_EQUAL(3, (int)mrow->nEntries());

  iface::mathml_dom::MathMLContentElement* ce = mrow->getEntry(1);
  wchar_t* str = ce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"entry1"));
  free(str);
  ce->release_ref();

  n = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(ce, n, mathml_dom::MathMLContentElement);
  ce->className(L"newentry1");
  iface::mathml_dom::MathMLContentElement* ce2 = mrow->insertEntry(ce, 1);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  CPPUNIT_ASSERT_EQUAL(4, (int)mrow->nEntries());
  ce2 = mrow->getEntry(1);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  ce2 = mrow->removeEntry(1);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  
  ce2 = mrow->insertEntry(ce, 4);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  CPPUNIT_ASSERT_EQUAL(4, (int)mrow->nEntries());
  ce2 = mrow->getEntry(4);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  mrow->deleteEntry(4);

  ce2 = mrow->setEntry(ce, 2);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  CPPUNIT_ASSERT_EQUAL(3, (int)mrow->nEntries());
  ce2 = mrow->getEntry(2);
  str = ce2->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newentry1"));
  free(str);
  ce2->release_ref();
  ce->release_ref();

  // And now continue testing MathMLMatrix...
  mrow->release_ref();
  mnl->release_ref();

  iface::mathml_dom::MathMLMatrixrowElement* mr =
    mme->getRow(1);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"firstrow"));
  free(str);
  mr->release_ref();

  n = d->createElementNS(MATHML_NS, L"matrixrow");
  DECLARE_QUERY_INTERFACE_REPLACE(mre, n, mathml_dom::MathMLMatrixrowElement);
  mre->className(L"newrow1");

  mr = mme->insertRow(mre, 1);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();
  CPPUNIT_ASSERT_EQUAL(5, (int)mme->nrows());
  mr = mme->getRow(1);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();
  mr = mme->removeRow(1);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();
  CPPUNIT_ASSERT_EQUAL(4, (int)mme->nrows());

  mr = mme->insertRow(mre, 5);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();
  CPPUNIT_ASSERT_EQUAL(5, (int)mme->nrows());
  mr = mme->getRow(5);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();
  mme->deleteRow(1);
  CPPUNIT_ASSERT_EQUAL(4, (int)mme->nrows());
  
  mr = mme->setRow(mre, 4);
  CPPUNIT_ASSERT_EQUAL(4, (int)mme->nrows());
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();
  mr = mme->getRow(4);
  str = mr->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newrow1"));
  free(str);
  mr->release_ref();

  mre->release_ref();
  mme->release_ref();
  d->release_ref();
}

void
MathMLTest::testMathMLPiecewise()
{
  iface::cellml_api::DOMURLLoader* ul = cb->localURLLoader();
  iface::dom::Document* d =
    ul->loadDocument(BASE_DIRECTORY L"mathml_othercontent.xml");
  ul->release_ref();

  iface::dom::Element* pwel = d->getElementById(L"piecewise1");
  DECLARE_QUERY_INTERFACE_REPLACE(mpw, pwel, mathml_dom::MathMLPiecewiseElement);

  iface::mathml_dom::MathMLContentElement* mce = mpw->otherwise();
  CPPUNIT_ASSERT(mce);
  wchar_t* str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"otherwisevalue"));
  free(str);
  mce->release_ref();

  iface::mathml_dom::MathMLNodeList* mnl = mpw->pieces();
  CPPUNIT_ASSERT_EQUAL(3, (int)mnl->length());
  iface::dom::Node* n = mnl->item(0);
  DECLARE_QUERY_INTERFACE_REPLACE(mcase, n, mathml_dom::MathMLCaseElement);
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1"));
  free(str);

  // Test the MathMLCaseElement now, while we have it...
  mce = mcase->caseCondition();
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1cond"));
  free(str);
  mce->release_ref();

  mce = mcase->caseValue();
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1value"));
  free(str);
  mce->release_ref();

  // Go back to testing the MathMLPiecewiseElement...
  mcase->release_ref();

  mce = mpw->getCaseValue(1);
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1value"));
  free(str);
  mce->release_ref();

  pwel = d->createElementNS(MATHML_NS, L"apply");
  DECLARE_QUERY_INTERFACE_REPLACE(mcenew, pwel,
                                  mathml_dom::MathMLContentElement);
  mcenew->className(L"newpwpvalue1");
  mce = mpw->setCaseValue(1, mcenew);
  mcenew->release_ref();
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpwpvalue1"));
  free(str);
  mce->release_ref();
  mce = mpw->getCaseValue(1);
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpwpvalue1"));
  free(str);
  mce->release_ref();

  mce = mpw->getCaseCondition(1);
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1cond"));
  free(str);
  mce->release_ref();

  pwel = d->createElementNS(MATHML_NS, L"apply");
  QUERY_INTERFACE_REPLACE(mcenew, pwel,
                                  mathml_dom::MathMLContentElement);
  mcenew->className(L"newpwpcond1");
  mce = mpw->setCaseCondition(1, mcenew);
  mcenew->release_ref();  
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpwpcond1"));
  free(str);
  mce->release_ref();
  mce = mpw->getCaseCondition(1);
  str = mce->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpwpcond1"));
  free(str);
  mce->release_ref();


  CPPUNIT_ASSERT_EQUAL(3, (int)mnl->length());
  mcase = mpw->getCase(1);
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1"));
  free(str);
  mcase->release_ref();

  pwel = d->createElementNS(MATHML_NS, L"piece");
  DECLARE_QUERY_INTERFACE_REPLACE(newpiece, pwel,
                                  mathml_dom::MathMLCaseElement);
  newpiece->className(L"newpiece");

  
  pwel = d->createElementNS(MATHML_NS, L"apply");
  DECLARE_QUERY_INTERFACE_REPLACE(el1, pwel,
                                  mathml_dom::MathMLContentElement);
  el1->className(L"newcaseval");
  pwel = d->createElementNS(MATHML_NS, L"apply");
  DECLARE_QUERY_INTERFACE_REPLACE(el2, pwel,
                                  mathml_dom::MathMLContentElement);
  el2->className(L"newcasecond");
  CPPUNIT_ASSERT_NO_THROW(newpiece->caseValue(el1));
  CPPUNIT_ASSERT_NO_THROW(newpiece->caseCondition(el2));
  el1->release_ref();
  el2->release_ref();

  mcase = mpw->insertCase(1, newpiece);
  CPPUNIT_ASSERT_EQUAL(4, (int)mnl->length());
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpiece"));
  free(str);
  mcase->release_ref();
  mcase = mpw->getCase(1);
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpiece"));
  free(str);
  mcase->release_ref();

  mcase = mpw->removeCase(1);
  CPPUNIT_ASSERT_EQUAL(3, (int)mnl->length());
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpiece"));
  free(str);
  el1 = mcase->caseCondition();
  str = el1->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcasecond"));
  free(str);
  el1->release_ref();
  el1 = mcase->caseValue();
  str = el1->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newcaseval"));
  free(str);
  el1->release_ref();
  mcase->release_ref();
  mcase = mpw->getCase(1);
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"pwp1"));
  free(str);
  mcase->release_ref();

  mcase = mpw->insertCase(0, newpiece);
  CPPUNIT_ASSERT_EQUAL(4, (int)mnl->length());
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpiece"));
  free(str);
  mcase->release_ref();
  mcase = mpw->getCase(4);
  str = mcase->className();
  CPPUNIT_ASSERT(!wcscmp(str, L"newpiece"));
  free(str);
  mcase->release_ref();
  mpw->deleteCase(4);
  CPPUNIT_ASSERT_EQUAL(3, (int)mnl->length());

  newpiece->release_ref();
  mnl->release_ref();
  mpw->release_ref();
  d->release_ref();
}
