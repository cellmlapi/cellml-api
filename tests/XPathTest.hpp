#ifndef XPATHTEST_H
#define XPATHTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "XPathBootstrap.hpp"
#include "cda_compiler_support.h"
#include "Ifacexpath.hxx"
#include "IfaceCellML_APISPEC.hxx"

class XPathTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XPathTest);
  CPPUNIT_TEST(testBasicXPath);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void tearDown();

  void testBasicXPath();
private:
  iface::cellml_api::CellMLBootstrap* mBootstrap;
  iface::cellml_api::DOMURLLoader* mLocalURLLoader;
  iface::dom::Document* mTestDoc;
  iface::xpath::XPathEvaluator* mXPEval;
  iface::xpath::XPathNSResolver* mXPResolv;
};

#endif // XPATHTEST_H
