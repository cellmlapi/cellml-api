#ifndef CEVASTEST_H
#define CEVASTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "cda_compiler_support.h"

#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCeVAS.hxx"

class CeVASTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CeVASTest);
  CPPUNIT_TEST(testCeVASBootstrap);
  CPPUNIT_TEST(testCeVASCore);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testCeVASBootstrap();
  void testCeVASCore();

private:
  iface::cellml_services::CeVASBootstrap* mCeVASBootstrap;
  iface::cellml_api::CellMLBootstrap* mBootstrap;
  iface::cellml_api::DOMModelLoader* mModelLoader;
};

#endif // CEVASTEST_H
