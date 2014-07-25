#ifndef RDFTEST_H
#define RDFTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "cda_compiler_support.h"

#include "IfaceRDF_APISPEC.hxx"

class RDFTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RDFTest);
  CPPUNIT_TEST(testW3CSuite);
  CPPUNIT_TEST(testContainerLibrary);
  CPPUNIT_TEST(testRDFAPIImplementation);
  CPPUNIT_TEST_SUITE_END();

  void assertContainerContents(iface::rdf_api::Container*, const wchar_t** aExpect);

public:
  void setUp();
  void tearDown();
  void testRDFAPIImplementation();
  void testW3CSuite();
  void testContainerLibrary();
};

#endif // RDFTEST_H
