#include "cellml-api-cxx-support.hpp"
#include "XPathTest.hpp"
#include "CellMLBootstrap.hpp"
#include <iostream>

#ifndef BASE_DIRECTORY
#ifdef WIN32
#define BASE_DIRECTORY L"file:///" TESTDIR L"/test_xml/"
#else
#define BASE_DIRECTORY L"file://" TESTDIR L"/test_xml/"
#endif
#endif

void
XPathTest::setUp()
{
  mBootstrap = CreateCellMLBootstrap();
  mLocalURLLoader = mBootstrap->localURLLoader();
  mTestDoc = mLocalURLLoader->loadDocument(BASE_DIRECTORY L"xpath-source.xml");
  mXPEval = CreateXPathEvaluator();
  mXPResolv = mXPEval->createNSResolver(mTestDoc);
}

void
XPathTest::tearDown()
{
  mBootstrap->release_ref();
  mLocalURLLoader->release_ref();
  mTestDoc->release_ref();
  mXPEval->release_ref();
  mXPResolv->release_ref();
}

void
XPathTest::testBasicXPath()
{
  {
    RETURN_INTO_OBJREF(ex, iface::xpath::XPathExpression,
                       mXPEval->createExpression(L"self::node()", mXPResolv));
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       ex->evaluate(mTestDoc, iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE, NULL));
    CPPUNIT_ASSERT_EQUAL(1, (int)r->snapshotLength());
  }
  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test1/child::para/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::UNORDERED_NODE_SNAPSHOT_TYPE, NULL
                        ));
    CPPUNIT_ASSERT_EQUAL(2, (int)r->snapshotLength());
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->snapshotItem(0));
    RETURN_INTO_WSTRING(o1id, o1->nodeValue());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->snapshotItem(1));
    RETURN_INTO_WSTRING(o2id, o2->nodeValue());
    CPPUNIT_ASSERT(o1id == L"a1");
    CPPUNIT_ASSERT(o2id == L"a3");
  }
  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test2/child::*/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o3, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o4, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(o4);
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);

    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"b1");

    RETURN_INTO_WSTRING(o2v, o2->nodeValue());
    CPPUNIT_ASSERT(o2v == L"b2");

    RETURN_INTO_WSTRING(o3v, o3->nodeValue());
    CPPUNIT_ASSERT(o3v == L"b3");

    RETURN_INTO_WSTRING(o4v, o4->nodeValue());
    CPPUNIT_ASSERT(o4v == L"b4");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test2/child::*/node()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o3, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o4, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o5, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(o5);
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);

    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"b1");

    RETURN_INTO_WSTRING(o2v, o2->nodeValue());
    CPPUNIT_ASSERT(o2v == L"b2");

    RETURN_INTO_WSTRING(o3v, o3->nodeValue());
    CPPUNIT_ASSERT(o3v == L"b3");

    RETURN_INTO_WSTRING(o4v, o4->nodeValue());
    CPPUNIT_ASSERT(o4v == L"b4");

    RETURN_INTO_WSTRING(nn, o5->nodeName());
    CPPUNIT_ASSERT(nn == L"a");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test3/attribute::hello",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(av, o1->nodeValue());
    CPPUNIT_ASSERT(av == L"world");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test3/attribute::*",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(av1, o1->nodeValue());
    CPPUNIT_ASSERT(av1 == L"bar");
    RETURN_INTO_WSTRING(av2, o2->nodeValue());
    CPPUNIT_ASSERT(av2 == L"world");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test4/para/descendant::para/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(av1, o1->nodeValue());
    CPPUNIT_ASSERT(av1 == L"Hello");
    RETURN_INTO_WSTRING(av2, o2->nodeValue());
    CPPUNIT_ASSERT(av2 == L"World");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//div[@id='test5inner']/ancestor::div/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(av1, o1->nodeValue());
    RETURN_INTO_WSTRING(av2, o2->nodeValue());
    CPPUNIT_ASSERT(av1 == L"Hello");
    CPPUNIT_ASSERT(av2 == L"World");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//div[@id='test6inner']/ancestor-or-self::div/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(av1, o1->nodeValue());
    RETURN_INTO_WSTRING(av2, o2->nodeValue());
    CPPUNIT_ASSERT(av1 == L"Hello");
    CPPUNIT_ASSERT(av2 == L"World");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//div[@mark='test7']/descendant-or-self::div/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o3, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o4, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(av1, o1->nodeValue());
    RETURN_INTO_WSTRING(av2, o2->nodeValue());
    RETURN_INTO_WSTRING(av3, o3->nodeValue());
    RETURN_INTO_WSTRING(av4, o4->nodeValue());
    CPPUNIT_ASSERT(av1 == L"Hello");
    CPPUNIT_ASSERT(av2 == L"World");
    CPPUNIT_ASSERT(av3 == L"This Is");
    CPPUNIT_ASSERT(av4 == L"A Test");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test7/self::test7",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1n, o1->nodeName());
    CPPUNIT_ASSERT(o1n == L"test7");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test8/child::chapter/descendant::para/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o3, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    RETURN_INTO_WSTRING(o2v, o2->nodeValue());
    RETURN_INTO_WSTRING(o3v, o3->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Hello");
    CPPUNIT_ASSERT(o2v == L"World");
    CPPUNIT_ASSERT(o3v == L"Foo");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test8/child::*/para/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Foo");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"/",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    DECLARE_QUERY_INTERFACE_OBJREF(o1d, o1, dom::Document);
    CPPUNIT_ASSERT(o1d != NULL);
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test9/entry/para[position()=1]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Good 1");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test9/entry/para[position()=last()]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Bad 3");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test9/entry/para[position()=last()-1]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Bad 2");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test9/entry/para[position()>1]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o3, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Bad 1");
    RETURN_INTO_WSTRING(o2v, o2->nodeValue());
    CPPUNIT_ASSERT(o2v == L"Bad 2");
    RETURN_INTO_WSTRING(o3v, o3->nodeValue());
    CPPUNIT_ASSERT(o3v == L"Bad 3");
  }
  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test10inner/following-sibling::chapter[position()=1]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Foo");
  }
  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test10inner/preceding-sibling::chapter[position()=1]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Baz");
  }
  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test11/chapter[position()=5]/child::section[position()=2]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"5s2");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test12/child::para[attribute::type=\"warning\"]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Selected text");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test13/child::para[attribute::type=\"warning\"][position()=5]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Selected text");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test13/child::para[position()=6][attribute::type=\"warning\"]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"The sixth");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test14/child::chapter[child::title='Introduction']/title/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Introduction");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test14/child::chapter[child::title]/title/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o2, iface::dom::Node, r->iterateNext());
    RETURN_INTO_OBJREF(o3, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    RETURN_INTO_WSTRING(o2v, o2->nodeValue());
    RETURN_INTO_WSTRING(o3v, o3->nodeValue());
    CPPUNIT_ASSERT(o1v == L"Preface");
    CPPUNIT_ASSERT(o2v == L"Introduction");
    CPPUNIT_ASSERT(o3v == L"XPath");
  }

  {
    RETURN_INTO_OBJREF(r, iface::xpath::XPathResult,
                       mXPEval->evaluate
                       (L"//test15/child::*[self::chapter or self::appendix][position()=last()]/text()",
                        mTestDoc, mXPResolv,
                        iface::xpath::XPathResult::ORDERED_NODE_ITERATOR_TYPE, NULL));
    RETURN_INTO_OBJREF(o1, iface::dom::Node, r->iterateNext());
    CPPUNIT_ASSERT(r->iterateNext().getPointer() == NULL);
    RETURN_INTO_WSTRING(o1v, o1->nodeValue());
    CPPUNIT_ASSERT(o1v == L"XPath");
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION( XPathTest );
