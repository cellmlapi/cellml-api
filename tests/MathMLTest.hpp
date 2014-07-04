#ifndef MATHMLTEST_H
#define MATHMLTEST_H

#include <cppunit/extensions/HelperMacros.h>
#ifdef HAVE_STD_CODECVT
#include "CppUnitWstringSupport.hpp"
#endif // HAVE_STD_CODECVT
#include "DOMBootstrap.hxx"
#include "IfaceMathML_content_APISPEC.hxx"
#include "IfaceCellML_APISPEC.hxx"

class MathMLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MathMLTest);
  CPPUNIT_TEST(testCreateMathMLDocument);
  CPPUNIT_TEST(testMathMLDocumentElementContainer);
  CPPUNIT_TEST(testMathMLContentToken);
  CPPUNIT_TEST(testMathMLContentContainer);
  CPPUNIT_TEST(testSpecificMathMLContentContainers);
  CPPUNIT_TEST(testMathMLPredefinedSymbol);
  CPPUNIT_TEST(testMathMLInterval);
  CPPUNIT_TEST(testMathMLCondition);
  CPPUNIT_TEST(testMathMLDeclare);
  CPPUNIT_TEST(testMathMLVector);
  CPPUNIT_TEST(testMathMLMatrix);
  CPPUNIT_TEST(testMathMLPiecewise);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testCreateMathMLDocument();
  void testMathMLDocumentElementContainer();
  void testMathMLContentToken();
  void testMathMLContentContainer();
  void testSpecificMathMLContentContainers();
  void testMathMLPredefinedSymbol();
  void testMathMLInterval();
  void testMathMLCondition();
  void testMathMLDeclare();
  void testMathMLVector();
  void testMathMLMatrix();
  void testMathMLPiecewise();
private:
  iface::cellml_api::CellMLBootstrap* cb;
};

#endif  // DOMTEST_H
