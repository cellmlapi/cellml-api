#include "CellMLEventsTest.hpp"
#include "CellMLBootstrap.hpp"
#include "Utilities.hxx"

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif
#include "IfaceCellML_events.hxx"

CPPUNIT_TEST_SUITE_REGISTRATION( CellMLEventsTest );

void
CellMLEventsTest::setUp()
{
  mGlycolysis = NULL;
  mBootstrap = CreateCellMLBootstrap();
  mModelLoader = mBootstrap->modelLoader();
  mGlycolysis =
    mModelLoader->loadFromURL
    (BASE_DIRECTORY L"glycolysis_pathway_1997.xml");
  QUERY_INTERFACE(mGlyET, mGlycolysis, events::EventTarget);
}

void
CellMLEventsTest::tearDown()
{
  mBootstrap->release_ref();
  mModelLoader->release_ref();
  mGlycolysis->release_ref();
  mGlyET->release_ref();
}

class TestEventHandler
  : public iface::events::EventListener
{
public:
  TestEventHandler()
    : mEventCount(0), mLastEvent(NULL)
  {
  }

  virtual ~TestEventHandler()
  {
    if (mLastEvent != NULL)
    {
      mLastEvent->release_ref();
    }
  }

  void handleEvent(iface::events::Event* aEvent)
    throw(std::exception&)
  {
    mEventCount++;
    if (mLastEvent == aEvent)
      return;
    if (mLastEvent)
      mLastEvent->release_ref();
    mLastEvent = aEvent;
    mLastEvent->add_ref();
  }

  uint32_t countEvents()
  {
    return mEventCount;
  }

  iface::events::Event*
  getLastEvent()
  {
    iface::events::Event* evt = mLastEvent;
    mLastEvent = NULL;
    return evt;
  }

  CDA_IMPL_REFCOUNT
  CDA_IMPL_QI1(events::EventListener)
  CDA_IMPL_ID
private:
  uint32_t mEventCount;
  iface::events::Event* mLastEvent;
};

void
CellMLEventsTest::testMathModified()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh1, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  iface::cellml_api::CellMLComponentSet* ccs =
    mGlycolysis->localComponents();
  // Find F6P...
  iface::cellml_api::CellMLComponent* c =
    ccs->getComponent(L"F6P");

  // Find its maths...
  iface::cellml_api::MathList* ml = c->math();
  iface::cellml_api::MathMLElementIterator* mei = ml->iterate();
  ml->release_ref();
  iface::mathml_dom::MathMLElement* me = mei->next();
  mei->release_ref();

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  me->className(L"foobar");

  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  iface::events::Event* evt = teh1->getLastEvent();

  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  std::wstring str = mevt->type();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"MathModified"), str);
  // Should be the maths node...
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, me));
  et->release_ref();
  et = mevt->currentTarget();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, mGlycolysis));
  et->release_ref();
  CPPUNIT_ASSERT(iface::events::Event::BUBBLING_PHASE == mevt->eventPhase());
  CPPUNIT_ASSERT(mevt->bubbles());
  CPPUNIT_ASSERT(!mevt->cancelable());
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, c));
  ce->release_ref();
  mevt->release_ref();

  // Now try some slightly different changes to the maths...
  DECLARE_QUERY_INTERFACE_REPLACE(mme, me, mathml_dom::MathMLMathElement);
  me = mme->getArgument(1);
  DECLARE_QUERY_INTERFACE_REPLACE(mae, me, mathml_dom::MathMLApplyElement);
  me = mae->getArgument(2);
  mae->release_ref();
  QUERY_INTERFACE_REPLACE(mae, me, mathml_dom::MathMLApplyElement);
  me = mae->getArgument(2);
  mae->release_ref();
  DECLARE_QUERY_INTERFACE_REPLACE(mce, me, mathml_dom::MathMLCiElement);

  iface::dom::Node* n = mce->getArgument(1);
  CPPUNIT_ASSERT(iface::dom::Node::TEXT_NODE == n->nodeType());
  mce->release_ref();
  mme->release_ref();

  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  n->nodeValue(L"foobar");
  n->release_ref();
  CPPUNIT_ASSERT_EQUAL(2, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  evt = teh1->getLastEvent();
  QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  mevt->release_ref();

  c->release_ref();
  ccs->release_ref();
  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testMathInserted()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh1, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  iface::cellml_api::CellMLComponentSet* ccs =
    mGlycolysis->localComponents();
  // Find F6P...
  iface::cellml_api::CellMLComponent* c =
    ccs->getComponent(L"F6P");
  ccs->release_ref();

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  iface::mathml_dom::MathMLMathElement* mme = mGlycolysis->createMathElement();
  c->addMath(mme);

  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, mme));
  mme->release_ref();
  et->release_ref();
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, c));
  ce->release_ref();
  mevt->release_ref();

  c->release_ref();

  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testMathRemoved()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh1, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  iface::cellml_api::CellMLComponentSet* ccs =
    mGlycolysis->localComponents();
  // Find F6P...
  iface::cellml_api::CellMLComponent* c =
    ccs->getComponent(L"F6P");
  ccs->release_ref();
  // Find its maths...
  iface::cellml_api::MathList* ml = c->math();
  iface::cellml_api::MathMLElementIterator* mei = ml->iterate();
  ml->release_ref();
  iface::mathml_dom::MathMLElement* me = mei->next();
  mei->release_ref();

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  c->removeMath(me);

  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  c->release_ref();

  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, me));
  et->release_ref();
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, c));
  ce->release_ref();
  mevt->release_ref();

  me->release_ref();
  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testCellMLElementInserted()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh1, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  iface::cellml_api::CellMLComponent* cc = mGlycolysis->createComponent();
  mGlycolysis->addElement(cc);
  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, cc));
  et->release_ref();
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, mGlycolysis));
  ce->release_ref();
  mevt->release_ref();
  cc->release_ref();

  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testCellMLElementRemoved()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh1, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  iface::cellml_api::CellMLComponentSet* ccs =
    mGlycolysis->localComponents();
  // Find F6P...
  iface::cellml_api::CellMLComponent* c =
    ccs->getComponent(L"F6P");
  ccs->release_ref();

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  mGlycolysis->removeElement(c);
  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, c));
  et->release_ref();
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, mGlycolysis));
  ce->release_ref();
  c->release_ref();
  mevt->release_ref();

  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testCellMLAttributeChanged()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh1, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  iface::cellml_api::CellMLComponentSet* ccs =
    mGlycolysis->localComponents();
  iface::cellml_api::CellMLComponent* c =
    ccs->getComponent(L"F6P");
  ccs->release_ref();

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  c->cmetaId(L"foobar");
  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, c));
  et->release_ref();
  CPPUNIT_ASSERT(
                 iface::cellml_events::MutationEvent::MODIFICATION ==
                 mevt->attrChange()
                );
  std::wstring str = mevt->prevValue();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"F6P"), str);
  str = mevt->newValue();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"foobar"), str);
  str = mevt->attrLocalName();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"id"), str);
  str = mevt->attrNamespaceURI();
  CPPUNIT_ASSERT_EQUAL(std::wstring(L"http://www.cellml.org/metadata/1.0#"), str);

  mevt->release_ref();
  c->release_ref();

  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testExtensionElementInserted()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh1, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh2, false);

  iface::cellml_api::CellMLComponentSet* ccs =
    mGlycolysis->localComponents();
  // Find F6P...
  iface::cellml_api::CellMLComponent* c =
    ccs->getComponent(L"F6P");
  ccs->release_ref();

  // Create a new extension element...
  iface::dom::Element* el =
    mGlycolysis->createExtensionElement(L"http://www.example.org/extension/test",
                                        L"thisisatest");
  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  c->appendExtensionElement(el);
  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, el));
  et->release_ref();
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, c));
  ce->release_ref();
  mevt->release_ref();

  el->release_ref();
  c->release_ref();
  teh1->release_ref();
  teh2->release_ref();
}

void
CellMLEventsTest::testExtensionElementRemoved()
{
  TestEventHandler* teh1 = new TestEventHandler();
  TestEventHandler* teh2 = new TestEventHandler();
  mGlyET->addEventListener(L"MathModified", teh2, false);
  mGlyET->addEventListener(L"MathInserted", teh2, false);
  mGlyET->addEventListener(L"MathRemoved", teh2, false);
  mGlyET->addEventListener(L"CellMLAttributeChanged", teh2, false);
  mGlyET->addEventListener(L"CellMLElementInserted", teh2, false);
  mGlyET->addEventListener(L"CellMLElementRemoved", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementInserted", teh2, false);
  mGlyET->addEventListener(L"ExtensionElementRemoved", teh1, false);

  iface::cellml_api::ExtensionElementList* eel =
    mGlycolysis->extensionElements();
  iface::cellml_api::ExtensionElement ee = eel->getAt(0);
  eel->release_ref();

  CPPUNIT_ASSERT_EQUAL(0, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());
  mGlycolysis->removeExtensionElement(ee);
  CPPUNIT_ASSERT_EQUAL(1, (int)teh1->countEvents());
  CPPUNIT_ASSERT_EQUAL(0, (int)teh2->countEvents());

  iface::events::Event* evt = teh1->getLastEvent();
  DECLARE_QUERY_INTERFACE_REPLACE(mevt, evt, cellml_events::MutationEvent);
  iface::events::EventTarget* et = mevt->target();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(et, ee));
  et->release_ref();
  iface::cellml_api::CellMLElement* ce = mevt->relatedElement();
  CPPUNIT_ASSERT_EQUAL(0, CDA_objcmp(ce, mGlycolysis));
  ce->release_ref();
  mevt->release_ref();

  ee->release_ref();
  teh1->release_ref();
  teh2->release_ref();
}
