#ifndef URITEST_H
#define URITEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "cda_config.h"
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include "../sources/rdf/URITool.hpp"

class URITest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(URITest);
  CPPUNIT_TEST(testURIParse);
  CPPUNIT_TEST(testURIResolve);
  CPPUNIT_TEST(testURISerialise);
  CPPUNIT_TEST(testURISerialiseRelative);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testURIParse();
  void testURIResolve();
  void testURISerialise();
  void testURISerialiseRelative();
};

#endif // URITEST_H
