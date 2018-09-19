#ifndef CUSESTEST_H
#define CUSESTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "cda_compiler_support.h"

#include "IfaceCellML_APISPEC.hxx"
#include "IfaceCUSES.hxx"
#include "IfaceAnnoTools.hxx"

class CUSESTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CUSESTest);
  CPPUNIT_TEST(testCUSESBootstrap);
  CPPUNIT_TEST(testCUSESObject);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testCUSESBootstrap();
  void testCUSESObject();

private:
  iface::cellml_services::CUSESBootstrap* mCUSESBootstrap;
  iface::cellml_api::CellMLBootstrap* mBootstrap;
  iface::cellml_api::DOMModelLoader* mModelLoader;
};

#endif // CUSESTEST_H
