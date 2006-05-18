#ifndef CELLMLEVENTSTEST_H
#define CELLMLEVENTSTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include <inttypes.h>
#include "IfaceCellML_events.hxx"

class CellMLEventsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CellMLEventsTest);
  CPPUNIT_TEST(testMathModified);
  CPPUNIT_TEST(testMathInserted);
  CPPUNIT_TEST(testMathRemoved);
  CPPUNIT_TEST(testCellMLElementInserted);
  CPPUNIT_TEST(testCellMLElementRemoved);
  CPPUNIT_TEST(testCellMLAttributeChanged);
  CPPUNIT_TEST(testExtensionElementInserted);
  CPPUNIT_TEST(testExtensionElementRemoved);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void tearDown();

  void testMathModified();
  void testMathInserted();
  void testMathRemoved();
  void testCellMLElementInserted();
  void testCellMLElementRemoved();
  void testCellMLAttributeChanged();
  void testExtensionElementInserted();
  void testExtensionElementRemoved();

private:
  iface::cellml_api::CellMLBootstrap* mBootstrap;
  iface::cellml_api::DOMModelLoader* mModelLoader;
  iface::cellml_api::Model* mGlycolysis;
  iface::events::EventTarget* mGlyET;
};

#endif // CELLMLEVENTSTEST_H
