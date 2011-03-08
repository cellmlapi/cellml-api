#ifndef SPROSTEST_H
#define SPROSTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "sources/Utilities.hxx"
#include "SProSBootstrap.hpp"
#include "IfaceSProS.hxx"

class SProSTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SProSTest);
  CPPUNIT_TEST(testSProSBootstrap);
  CPPUNIT_TEST(testSProSBase);
  CPPUNIT_TEST(testSProSBaseSet);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSProSBootstrap();
  void testSProSBase();
  void testSProSBaseSet();
};

#endif // SPROSTEST_H
