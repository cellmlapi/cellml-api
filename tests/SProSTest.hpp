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
  CPPUNIT_TEST(testSProSDataGeneratorSet);
  CPPUNIT_TEST(testSProSOutputSet);
  CPPUNIT_TEST(testSProSChangeSet);
  CPPUNIT_TEST(testSProSVariable);
  CPPUNIT_TEST(testSProSCurve);
  CPPUNIT_TEST(testSProSSurface);
  CPPUNIT_TEST(testSProSDataSet);
  CPPUNIT_TEST(testSProSOneStepSteadyState);
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
  void testSProSDataGeneratorSet();
  void testSProSOutputSet();
  void testSProSChangeSet();
  void testSProSVariable();
  void testSProSParameter();
  void testSProSCurve();
  void testSProSSurface();
  void testSProSDataSet();
  void testSProSOneStepSteadyState();
};

#endif // SPROSTEST_H
