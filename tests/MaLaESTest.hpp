#ifndef MALAESTEST_H
#define MALAESTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "cda_compiler_support.h"

#include "IfaceCellML_APISPEC.hxx"
#include "IfaceMaLaES.hxx"
#include "IfaceCUSES.hxx"
#include "IfaceAnnoTools.hxx"

class MaLaESTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MaLaESTest);
  CPPUNIT_TEST(testMaLaESBootstrap);
  CPPUNIT_TEST(testMaLaESObject);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testMaLaESBootstrap();
  void testMaLaESObject();

private:
  iface::cellml_services::MaLaESBootstrap* mMaLaESBootstrap;
};

#endif // MALAESTEST_H
