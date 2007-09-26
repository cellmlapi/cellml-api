#ifndef VACSSSTEST_H
#define VACSSTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "cda_config.h"

#ifdef HAVE_INTTYPES
#include <inttypes.h>
#endif

#include "IfaceCellML_APISPEC.hxx"
#include "IfaceVACSS.hxx"

class VACSSTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(VACSSTest);
  CPPUNIT_TEST(testVACSService);
  CPPUNIT_TEST(testGetPositionInXML);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testVACSService();
  void testGetPositionInXML();

private:
  iface::cellml_services::VACSService* mVACSService;
};

#endif // VACSSTEST_H
