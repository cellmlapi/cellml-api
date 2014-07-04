#ifndef DOMTEST_H
#define DOMTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "DOMBootstrap.hxx"

#ifdef HAVE_STD_CODECVT
#include "CppUnitWstringSupport.hpp"
#endif // HAVE_STD_CODECVT

// Warning: The DOM is currently built on top of Gdome, which has its own tests.
// This code is not designed to test Gdome, but rather to find any defects in
// the way it has been wrapped. Therefore, it does not exercise every part of
// the DOM. If the DOM is replaced with our own ground-up implementation, we
// will need to write more tests.
// The serialiser, on the other hand, should be more thouroughly tested, as it
// is our own code.
class DOMTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( DOMTest );
  CPPUNIT_TEST(testCreateDOMImplementation);
  CPPUNIT_TEST(testCreateDocumentType);
  CPPUNIT_TEST(testCreateDocument);
  CPPUNIT_TEST(testCloneNode);
  CPPUNIT_TEST(testImportNode);
  CPPUNIT_TEST(testSerialiseBasicDocument);
  CPPUNIT_TEST(testSerialiseAttributes);
  CPPUNIT_TEST(testSerialiseChildElements);
  CPPUNIT_TEST(testSetAttributeNodeNS);
  CPPUNIT_TEST(testLoadDocument);
  CPPUNIT_TEST(testGetElementByTagName);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testCreateDOMImplementation();
  void testCreateDocumentType();
  void testCreateDocument();
  void testCloneNode();
  void testImportNode();
  void testSetAttributeNodeNS();

  void testSerialiseBasicDocument();
  void testSerialiseAttributes();
  void testSerialiseChildElements();

  void testLoadDocument();

  void testGetElementByTagName();
private:
  iface::dom::DOMImplementation* di;
  iface::dom::DocumentType* dt;
  iface::dom::Document *doc;
};

#endif  // DOMTEST_H
