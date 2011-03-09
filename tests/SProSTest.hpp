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
  CPPUNIT_TEST(testSProSSEDMLElement);
  CPPUNIT_TEST(testSProSNamedElementSet);
  CPPUNIT_TEST(testSProSNamedIdentifiedElementSet);
  CPPUNIT_TEST(testSProSModel);
  CPPUNIT_TEST(testSProSSimulationSet);
  CPPUNIT_TEST(testSProSTaskSet);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSProSBootstrap();
  void testSProSBase();
  void testSProSBaseSet();
  void testSProSSEDMLElement();
  void testSProSNamedElementSet();
  void testSProSNamedIdentifiedElementSet();
  void testSProSModel();
  void testSProSSimulationSet();
  void testSProSTaskSet();
};

#endif // SPROSTEST_H
