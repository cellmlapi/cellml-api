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
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSProSBootstrap();
};

#endif // SPROSTEST_H
